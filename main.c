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
	char *desc;
	struct tm *time;
	int is_static;
};

/* Post generation functions */
struct post *create_post(const char *file);
char *get_post_title(struct cmark_node *root);
char *get_post_desc(struct cmark_node *root);
struct tm *get_post_time(const char *file);
void free_post(struct post *p);
int insert_post_time(struct cmark_node *root, struct tm *time);

int write_index(struct post *posts[], int totalPosts);
int write_archive(struct post *posts[], int totalPosts);
int write_rss(struct post *posts[], int totalPosts);
int write_post(struct post *post);
int is_post_static(char *file);

void copy_resources(char *);

void errprintf(const char *, int);

char bufPost[MAX_POST_CHARS];
char *header;
char *footer;
char *rssHeader;
char *rssFooter;

int main()
{
	printf("---- Beginning website generation...\n");
	/* Load header and footer html */
	header = read_text(HEADER_HTML, MAX_POST_CHARS);
	footer = read_text(FOOTER_HTML, MAX_POST_CHARS);
	rssHeader = read_text(HEADER_RSS, MAX_POST_CHARS);
	rssFooter = read_text(FOOTER_RSS, MAX_POST_CHARS);

	struct dirent **t_mds;
	int t_postcount = scandir(POSTDIR, &t_mds, md_filter, alphasort);
	if(t_postcount <= 0) {
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

	/* write the rss feed */
	write_rss(posts, t_postcount);

	/* Copy images/videos/audio/static pages */
	copy_resources(RESOURCEDIR);

	/* free up all the posts on the heap */
	i = t_postcount;
	while(i--)
		free_post(posts[i]);

	while(t_postcount--)
		free(t_mds[t_postcount]);
	free(t_mds);

	printf("--- Website generated.\n");
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

	sprintf(bufPost, "%s%s", POSTDIR, p->fsource);
	printf("Loading post %s ... \n", bufPost);
	/* read in the post text file */
	char *tmp = read_text(bufPost, MAX_POST_CHARS);
	/* Check if it's a post or a static (other) page */
	p->is_static = is_post_static(p->fsource);
	if(p->is_static)
		printf("** Page %s is static\n", p->fsource);
	printf("Parsing post to cmark_node...\n");
	struct cmark_node *t_root = cmark_parse_document(
			tmp, strlen(tmp), CMARK_OPT_UNSAFE);
	free(tmp);
	printf("Extracting title...\n");
	/* extract the post title from the first header in the post */
	p->title = get_post_title(t_root);
	printf("Extracting description...\n");
	p->desc = get_post_desc(t_root);
	printf("Generating post time...\n");
	/* generate the struct tm representing the post date */
	p->time = get_post_time(p->fsource);
	printf("Inserting post time into cmark tree...\n");
	if(!p->is_static)
		insert_post_time(t_root, p->time);
	printf("Rendering HTML...\n");
	/* convert the markdown to html */
	p->content = cmark_render_html(t_root, CMARK_OPT_UNSAFE);
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
	printf("Generating html web directory...\n");
	/* generate the base directory of the post */
	p->dir = malloc(MAX_URL_CHARS);
	if(!p->is_static) {
		sprintf(p->dir, "%d/%02d/%02d/", 
		p->time->tm_year + 1900,
		p->time->tm_mon + 1,
		p->time->tm_mday);

	} else 
		sprintf(p->dir, "");
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
	bufPost[0] = '\0';
	while(cmark_iter_next(t_titleIter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_titleIter)) == CMARK_NODE_TEXT) 
			strcat(bufPost, cmark_node_get_literal(cmark_iter_get_node(t_titleIter)));
	cmark_iter_free(t_iter);
	cmark_iter_free(t_titleIter);
	char *title = malloc(strlen(bufPost)+1);
	char *t = title;
	int i = 0;
	while((*t++ = bufPost[i++]))
		;
	return title;
}

/* Gets the first paragraph (or make a blank one) for the RSS description */
char *get_post_desc(struct cmark_node *root)
{
	cmark_iter *t_iter = cmark_iter_new(root);
	while(cmark_iter_next(t_iter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_iter)) == CMARK_NODE_PARAGRAPH)
			break;
	cmark_node *t_desc = cmark_iter_get_node(t_iter);
	cmark_iter *t_descIter = cmark_iter_new(t_desc);
	bufPost[0] = '\0';
	while(cmark_iter_next(t_descIter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_descIter)) == CMARK_NODE_TEXT)
			strcat(bufPost, cmark_node_get_literal(cmark_iter_get_node(t_descIter)));
	cmark_iter_free(t_iter);
	cmark_iter_free(t_descIter);
	char *desc = malloc(strlen(bufPost)+1);
	char *t = desc;
	int i = 0;
	while((*t++ = bufPost[i++]))
		;
	return desc;
}

/* Generates a tm struct based on the time in the post filename */
struct tm *get_post_time(const char *file)
{
	struct tm *time = malloc(sizeof(struct tm));
	strncpy(bufPost, file, 16);
	if(strptime(bufPost, "%Y-%m-%d-%H-%M", time) == NULL) {
		printf("ERROR: Failed to convert time\n");
		free(time);
		return NULL;
	}
	return time;
}

/* Inserts a new text node containing the date and time of the post */
int insert_post_time(struct cmark_node *root, struct tm *time)
{
	if(time == NULL)
		return 1;
	printf("-- Inserting date into post...\n");
	cmark_iter *t_iter = cmark_iter_new(root);
	while(cmark_iter_next(t_iter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_iter)) == CMARK_NODE_HEADING)
			break;
	
	/* Store the parent HEADING node */
	cmark_node *t_title = cmark_iter_get_node(t_iter);
	
	/* Create the new node for the date */
	cmark_node *t_date_block = cmark_node_new(CMARK_NODE_HTML_BLOCK);
	/* Create the date string */
	char timebufPost[MAX_URL_CHARS];
	strftime(timebufPost, MAX_URL_CHARS, "%A, %B %e, %Y", time);
	sprintf(bufPost, "<p class=\"postdate\">%s</p>", timebufPost);
	if(!cmark_node_set_literal(t_date_block, bufPost))
		return -1;
	cmark_node_insert_after(t_title, t_date_block);
	printf("-- Date inserted.\n");
	return 0;
}

/* Frees a post from the heap */
void free_post(struct post *p)
{
	free(p->title);
	free(p->dir);
	free(p->fsource);
	free(p->fhtml);
	free(p->content);
	if(p->time != NULL)
		free(p->time);
	free(p);
}

/* Writes a post to an html file */
int write_post(struct post *post)
{
	printf("-- Writing post %s...\n", post->title);
	/* Generate the directory structure */
	sprintf(bufPost, "%s%s", HTMLDIR, post->dir);

	if(create_directory(bufPost) < 0)
		return -1;

	sprintf(bufPost, "%s%s%s", HTMLDIR, post->dir, post->fhtml);
	FILE *f = fopen(bufPost, "w");
	sprintf(bufPost, "%s<article>%s</article>%s", header, post->content, footer);
	fprintf(f, bufPost);
	fclose(f);
	printf("-- Post complete.\n");
	return 0;
}

/* Writes the index.html file */
int write_index(struct post *posts[], int totalPosts)
{
	printf("-- Writing index.html...\n");
	sprintf(bufPost, "%s%s", HTMLDIR, "index.html");
	FILE *f = fopen(bufPost, "w");
	if(f == NULL) {
		errprintf("write_index", errno);
		return -1;
	}
	if(fprintf(f, header) < 0) {
		errprintf("write_index", errno);
		return -1;
	}
	int c = 0;
	while(totalPosts-- > 0 && c < INDEX_POSTS) {
		if(posts[totalPosts]->is_static)
			continue;
		fprintf(f, "<article>\n");
		if(fprintf(f, posts[totalPosts]->content) < 0) {
			errprintf("write_index", errno);
			fclose(f);
			return -1;
		}
		fprintf(f, "</article>\n");
		if(totalPosts > 0) 
			fprintf(f, "<hr>\n");
		c++;
	}
	if(fprintf(f, footer) < 0) {
		errprintf("write_index", errno);
		fclose(f);
		return -1;
	}
	fclose(f);
	printf("-- index.html complete.\n");
	return 0;
}

/* Writes the archive.html file */
int write_archive(struct post *posts[], int totalPosts)
{
	printf("-- Writing archive.html...\n");
	sprintf(bufPost, "%s%s", HTMLDIR, "archive.html");
	FILE *f = fopen(bufPost, "w");
	fprintf(f, header);
	fprintf(f, "<article class=\"archive\">\n<ul>\n");
	while(totalPosts-- > 0) {
		if(posts[totalPosts]->is_static)
			continue;
		strftime(bufPost, MAX_URL_CHARS, "%Y-%m-%d", posts[totalPosts]->time);
		fprintf(f, "<li><a href=\"%s%s\">%s - %s</a></li>\n",
				posts[totalPosts]->dir,
				posts[totalPosts]->fhtml,
				bufPost,
				posts[totalPosts]->title);
	}
	fprintf(f, "</ul>\n</article>\n<hl>\n");
	fprintf(f, footer);
	fclose(f);
	printf("-- archive.html complete.\n");
	return 0;
}

int write_rss(struct post *posts[], int totalPosts)
{
	printf("-- Writing RSS feed...\n");
	sprintf(bufPost, "%s%s", HTMLDIR, "feed.xml");
	FILE *f = fopen(bufPost, "w");
	fprintf(f, rssHeader);
	int oldestPostIdx = totalPosts - MAX_RSS_POSTS;
	while(totalPosts-- > oldestPostIdx) {
		if(posts[totalPosts]->is_static) {
			oldestPostIdx++;
			continue;
		}
		fprintf(f, "<item>\n");
		fprintf(f, "<title>%s</title>\n", posts[totalPosts]->title);
		fprintf(f, "<link>%s%s%s</link>\n",
				WEBSITE,
				posts[totalPosts]->dir,
				posts[totalPosts]->fhtml);
		printf("-- DEBUG: Post Time: %d\n", posts[totalPosts]->time->tm_mon);
		strftime(bufPost, MAX_URL_CHARS, "%a, %d %b %y %T %z", posts[totalPosts]->time);
		fprintf(f, "<pubDate>%s</pubDate>\n", bufPost);
		fprintf(f, "<description>");
		int i;
		char *c = posts[totalPosts]->content;
		for(i = 0; i < MAX_POST_CHARS && *c != '\0'; ++i) {
			fprintf(f, "%c", *c++);
		}
		fprintf(f, "</description>\n");
		fprintf(f, "</item>\n");
	}
	fprintf(f, rssFooter);
	fclose(f);
	printf("-- feed.xml complete.\n");
	return 0;
}

/* Recursively copies all the static files in RESOURCEDIR to HTMLDIR */
void copy_resources(char *dir)
{
	printf("-- Copying resources...\n");
	printf("Creating directory %s\n", dir);
	sprintf(bufPost, "%s%s", HTMLDIR, dir + strlen(RESOURCEDIR));
	create_directory(bufPost);
	char t_dest[MAX_URL_CHARS];
	struct stat t_stat;
	/* find all files in resource directory */
	struct dirent **t_files;
	printf("Scanning directory %s\n", dir);
	int t_count = scandir(dir, &t_files, NULL, NULL);
	int t_cleanup = t_count;
	while(t_count-- > 0) {
		if(t_files[t_count]->d_name[0] == '.')
			continue;
		printf("Found %s%s\n", dir, t_files[t_count]->d_name);
		sprintf(bufPost, "%s%s", dir, t_files[t_count]->d_name);
		stat(bufPost, &t_stat);
		if(S_ISDIR(t_stat.st_mode)) {
			char t_dir[MAX_URL_CHARS];
			sprintf(t_dir, "%s%s/", dir, t_files[t_count]->d_name);
			printf("Directory found: %s\n", t_dir);
			copy_resources(t_dir);
		}
		if(S_ISREG(t_stat.st_mode)) {	
			sprintf(t_dest, "%s%s", HTMLDIR, bufPost + strlen(RESOURCEDIR));
			printf("Copying %s to %s\n", bufPost, t_dest);
			copy_file(bufPost, t_dest);
		}
	}
	while(t_cleanup-- > 0)
		free(t_files[t_cleanup]);
	free(t_files);
	printf("-- Finished copying resources.\n");
}

/* Checks if a file is a blog post or a static page */
int is_post_static(char *file)
{
	printf("-- Checking if post %s is static...\n", file);
	if (strncmp(file, STATIC_PAGE, sizeof(STATIC_PAGE) - 1) == 0) {
		printf("Yep!\n");
		return 1;
	}
	printf("Nope!\n");
	return 0;
}
