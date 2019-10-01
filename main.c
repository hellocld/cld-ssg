#define _XOPEN_SOURCE 700 /* eliminates gcc warning about strptime */

#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "util.h"
#include "cmark.h"

#include "config.h"

int md_filter(const struct dirent *);

struct post {
	char *title;
	char *dir;
	char *fsource;
	char *fhtml;
	char *content;
	struct tm *time;
};

/* Post generation functions */
struct post *create_post(const char *file);
char *get_post_title(struct cmark_node *root);
struct tm *get_post_time(const char *file);
void free_post(struct post *p);

int write_index(struct post *posts[], int totalPosts);
int write_archive(struct post *posts[], int totalPosts);
int write_rss(struct post *posts, int totalPosts);
int write_post(struct post *post);

void copy_resources(char *);

void errprintf(const char *, int);

char buf[MAX_POST_CHARS];
char *header;
char *footer;

int main()
{

	/* Load header and footer html */
	header = read_text(HEADER_HTML, MAX_POST_CHARS);
	footer = read_text(FOOTER_HTML, MAX_POST_CHARS);

	/* Load all posts */
	struct dirent **t_mds;
	int t_postcount = scandir(POSTDIR, &t_mds, md_filter, alphasort);
	if(t_postcount < 0) {
		printf("ERROR: Failed to load posts from %s\n", POSTDIR);
		return -1;
	}

	int i = t_postcount;
	struct post *posts[t_postcount];
	while(i--) {
		posts[i] = create_post(t_mds[i]->d_name);
		/* Since we're already iterating through, write the post now */
		write_post(posts[i]);
	}

	/* Write index.html */
	write_index(posts, t_postcount);

	/* Write archive.html */
	write_archive(posts, t_postcount);

	/* Copy images/videos/audio/static pages */
	copy_resources(RESOURCEDIR);

	/* free up all the posts on the heap */
	i = t_postcount;
	while(i--)
		free_post(posts[i]);
	
	return 0;
}

/* returns non-zero if filename ends with ".md" */
int md_filter(const struct dirent *d)
{
	/* if d->d_name doesn't end with ".md" we don't want it */
	if(strcmp(d->d_name + (strlen(d->d_name) - 3), ".md") == 0)
		return 1;
	return 0;
}

/* Returns a pointer to a malloc'd post struct from a .md file */
struct post *create_post(const char *file)
{
	struct post *p = malloc(sizeof(struct post));
	p->fsource = malloc(MAX_URL_CHARS);
	memcpy(p->fsource, file, MAX_URL_CHARS);

	sprintf(buf, "%s%s", POSTDIR, p->fsource);
	printf("Loading post %s ... \n", buf);
	/* read in the post text file */
	char *tmp = read_text(buf, MAX_POST_CHARS);
	printf("Parsing post to cmark_node...\n");
	struct cmark_node *t_root = cmark_parse_document(
			tmp, strlen(tmp), CMARK_OPT_DEFAULT);
	free(tmp);
	printf("Rendering HTML...\n");
	/* convert the markdown to html */
	p->content = cmark_render_html(t_root, CMARK_OPT_DEFAULT);
	printf("Extracting title...\n");
	/* extract the post title from the first header in the post */
	p->title = get_post_title(t_root);
	printf("Generating html filename...\n");
	/* generate the html file name */
	p->fhtml = malloc(MAX_URL_CHARS);
	char *t_html = p->fhtml;
	char *t_title = p->title;
	while((*t_html++ = *t_title++))
		;
	t_html = p->fhtml;
	while((*t_html++))
		if(*t_html == ' ')
			*t_html = '-';
	strncat(p->fhtml, ".html", 6);
	printf("Generating post time...\n");
	/* generate the struct tm representing the post date */
	p->time = get_post_time(file);
	printf("Generating html web directory...\n");
	/* generate the base directory of the post */
	p->dir = malloc(MAX_URL_CHARS);
	sprintf(p->dir, "%d/%02d/%02d/", 
		p->time->tm_year + 1900,
		p->time->tm_mon,
		p->time->tm_mday);
	printf("Generation complete. Freeing node...\n");
	cmark_node_free(t_root);
	return p;
}

/* Gets the post title from the first HEADER in the cmark tree */
char *get_post_title(struct cmark_node *root)
{
	/* Loop through the tree to find the first header */
	cmark_iter *t_iter = cmark_iter_new(root);
	while(cmark_iter_next(t_iter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_iter)) == CMARK_NODE_HEADING)
			break;
	
	/* Store the parent HEADING node */
	cmark_node *t_title = cmark_iter_get_node(t_iter);
	cmark_iter *t_titleIter = cmark_iter_new(t_title);
	buf[0] = '\0';
	while(cmark_iter_next(t_titleIter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_titleIter)) == CMARK_NODE_TEXT) 
			strcat(buf, cmark_node_get_literal(cmark_iter_get_node(t_titleIter)));
	cmark_iter_free(t_iter);
	cmark_iter_free(t_titleIter);
	char *title = malloc(strlen(buf)+1);
	char *t = title;
	int i = 0;
	while((*t++ = buf[i++]))
		;
	return title;
}

/* Generates a tm struct based on the time in the post filename */
struct tm *get_post_time(const char *file)
{
	struct tm *time = malloc(sizeof(struct tm));
	strncpy(buf, file, 16);
	if(strptime(buf, "%Y-%m-%d-%H-%M", time) == NULL)
		printf("ERROR: Failed to convert time\n");
	return time;
}

/* Frees a post from the heap */
void free_post(struct post *p)
{
	free(p->title);
	free(p->dir);
	free(p->fsource);
	free(p->fhtml);
	free(p->content);
	free(p->time);
	free(p);
}

/* Writes a post to an html file */
int write_post(struct post *post)
{
	/* Generate the directory structure */
	sprintf(buf, "%s%s", HTMLDIR, post->dir);
	printf("DEBUG: %s\n", buf);

	if(create_directory(buf) < 0)
		return -1;

	sprintf(buf, "%s%s%s", HTMLDIR, post->dir, post->fhtml);
	FILE *f = fopen(buf, "w");
	sprintf(buf, "%s<article>%s</article>%s", header, post->content, footer);
	fprintf(f, buf);
	fclose(f);
	return 0;
}

int write_index(struct post *posts[], int totalPosts)
{
	sprintf(buf, "%s%s", HTMLDIR, "index.html");
	FILE *f = fopen(buf, "w");
	if(f == NULL) {
		errprintf("write_index", errno);
		return -1;
	}
	if(fprintf(f, header) < 0) {
		errprintf("write_index", errno);
		return -1;
	}
	int c = 0;
	while(totalPosts-- > 0 && c++ < INDEX_POSTS) {
		fprintf(f, "<article>\n");
		if(fprintf(f, posts[totalPosts]->content) < 0) {
			errprintf("write_index", errno);
			fclose(f);
			return -1;
		}
		fprintf(f, "</article>\n");
		if(totalPosts > 0) 
			fprintf(f, "<hr>\n");
	}
	if(fprintf(f, footer) < 0) {
		errprintf("write_index", errno);
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

int write_archive(struct post *posts[], int totalPosts)
{
	sprintf(buf, "%s%s", HTMLDIR, "archive.html");
	FILE *f = fopen(buf, "w");
	fprintf(f, header);
	fprintf(f, "<article class=\"archive\">\n<ul>\n");
	while(totalPosts-- > 0) {
		strftime(buf, MAX_URL_CHARS, "%Y-%m-%d", posts[totalPosts]->time);
		fprintf(f, "<li><a href=\"%s%s\">%s - %s</a></li>\n",
				posts[totalPosts]->dir,
				posts[totalPosts]->fhtml,
				buf,
				posts[totalPosts]->title);
	}
	fprintf(f, "</ul>\n</article>\n<hl>\n");
	fprintf(f, footer);
	fclose(f);
	return 0;
}

void errprintf(const char *function, int error)
{
	printf("%s ERROR: %d\n", function, error);
}

void copy_resources(char *dir)
{
	printf("DEBUG: Creating directory %s\n", dir);
	sprintf(buf, "%s%s", HTMLDIR, dir + strlen(RESOURCEDIR));
	create_directory(buf);
	char t_dest[MAX_URL_CHARS];
	struct stat t_stat;
	/* find all files in resource directory */
	struct dirent **t_files;
	printf("Scanning directory %s\n", dir);
	int t_count = scandir(dir, &t_files, NULL, NULL);
	while(t_count-- > 0) {
		if(t_files[t_count]->d_name[0] == '.')
			continue;
		printf("Found %s%s\n", dir, t_files[t_count]->d_name);
		sprintf(buf, "%s%s", dir, t_files[t_count]->d_name);
		stat(buf, &t_stat);
		if(S_ISDIR(t_stat.st_mode)) {
			char t_dir[MAX_URL_CHARS];
			sprintf(t_dir, "%s%s/", dir, t_files[t_count]->d_name);
			printf("Directory found: %s\n", t_dir);
			copy_resources(t_dir);
		}
		if(S_ISREG(t_stat.st_mode)) {	
			sprintf(t_dest, "%s%s", HTMLDIR, buf + strlen(RESOURCEDIR));
			printf("DEBUG: copying %s to %s\n", buf, t_dest);
			copy_file(buf, t_dest);
		}
	}

}
