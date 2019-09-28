#define _XOPEN_SOURCE 700 /* eliminates gcc warning about strptime */

#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
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
int create_all_posts(struct post *posts);

int write_index(struct post *posts, int totalPosts);
int write_archive(struct post *posts, int totalPosts);
int write_rss(struct post *posts, int totalPosts);
int write_post(struct post *post);

char buf[MAX_POST_CHARS];

int main()
{

	/* Load header and footer html */

	/* Create website directories */
	
	/* Load all posts */
	
	/* Write post HTML pages */

	/* Write index.html */

	/* Write archive.html */

	/* Copy images/videos/audio/static pages */

	printf("*** read_file testing ***\n\n");
	struct post *tp = create_post("2019-09-24-20-24-real-test.md");

	printf("Title:\t%s\n", tp->title);
	printf("Dir:\t%s\n", tp->dir);
	printf("Source:\t%s\n", tp->fsource);
	printf("HTML:\t%s\n", tp->fhtml);
	printf("Content:\n%s\n", tp->content);

//	write_post(tp);

	free_post(tp);
	free(tp->content);
	free(tp->title);
	free(tp);
	return 0;
}

/* returns non-zero if filename ends with ".md" */
int md_filter(const struct dirent *d)
{
	/* if d->d_name doesn't end with ".md" we don't want it */
	if(strstr(d->d_name, ".md") != NULL)
		return 1;
	return 0;
}

/* Returns a pointer to a malloc'd post struct from a .md file */
struct post *create_post(const char *file)
{
	struct post *p = malloc(sizeof(struct post));
	p->fsource = malloc(MAX_URL_CHARS);
	memcpy(p->fsource, buf, MAX_URL_CHARS);

	sprintf(buf, "%s%s", POSTDIR, p->fsource);

	/* read in the post text file */
	char *tmp = read_text(buf, MAX_POST_CHARS);
	struct cmark_node *t_root = cmark_parse_document(
			tmp, strlen(tmp), CMARK_OPT_DEFAULT);
	free(tmp);
	
	/* convert the markdown to html */
	p->content = cmark_render_html(t_root, CMARK_OPT_DEFAULT);
	
	/* extract the post title from the first header in the post */
	p->title = get_post_title(t_root);

	/* generate the html file name */
	p->fhtml = malloc(MAX_URL_CHARS);
	while((*(p->fhtml++) = *(p->title++)))
		if(*(p->fhtml) == ' ')
			*(p->fhtml) = '-';
	strncat(p->fhtml, ".html", 6);

	/* generate the struct tm representing the post date */
	p->time = get_post_time(file);

	/* generate the base directory of the post */
	p->dir = malloc(MAX_URL_CHARS);
	sprintf(p->dir, "%d/%02d/%02d/", 
		p->time->tm_year + 1900,
		p->time->tm_mon,
		p->time->tm_mday);

	free(t_root);
	return p;
}

/* Gets the post title from the first HEADER in the cmark tree */
char *get_post_title(struct cmark_node *root)
{
	cmark_iter *t_iter = cmark_iter_new(root);
	while(cmark_iter_next(t_iter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_iter)) == CMARK_NODE_HEADING)
			break;
	cmark_node *t_title = cmark_iter_get_node(t_iter);
	cmark_iter_free(t_iter);
	t_iter = cmark_iter_new(t_title);
	char *title = malloc(MAX_URL_CHARS);
	while(cmark_iter_next(t_iter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_iter)) == CMARK_NODE_TEXT) 
			strcat(title, cmark_node_get_literal(cmark_iter_get_node(t_iter)));
	free(t_iter);
	free(t_title);

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

int write_post(struct post *post)
{
	sprintf(buf, "%s%s", HTMLDIR, post->dir);

	printf("DEBUG: %s\n", buf);

	printf("%d\n", create_directory(buf));

	sprintf(buf, "%s%s%s", HTMLDIR, post->dir, post->fhtml);
	FILE *f = fopen(buf, "w");
	fprintf(f, post->content);
	fclose(f);
}



