#define _XOPEN_SOURCE 700 /* eliminates gcc warning about strptime */

#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "config.h"

#include "cmark.h"

int md_filter(const struct dirent *);

struct post {
	char *title;
	char *url;
	char *content;
	struct tm *time;
};

/* Post generation functions */
struct post *create_post(const char *file);
char *get_post_title(struct cmark_node *root);
struct tm *get_post_time(const char *file);
char *create_post_url(struct tm *time, char *title);
int create_all_posts(struct post *posts);

int write_index(struct post *posts, int totalPosts);
int write_archive(struct post *posts, int totalPosts);
int write_rss(struct post *posts, int totalPosts);
int write_post(struct post *post);

int create_directory(const char *path);

char *read_text(const char *path, int maxLength);

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
	char file[MAX_URL_CHARS] = "2019-09-24-20-24-real-test.md";
	struct post *tp = create_post(file);
	printf("File Path:\t%s\nPost Title:\t%s\nPost URL:\t%s\n%s\n", 
			file, tp->title, tp->url, tp->content);

	write_post(tp);

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

	char *path = malloc(MAX_URL_CHARS);
	strcpy(path, POSTDIR);
	strcat(path, file);
	/* read in the post text file */
	char *tmp = read_text(path, MAX_POST_CHARS);
	struct cmark_node *t_root = cmark_parse_document(
			tmp, strlen(tmp), CMARK_OPT_DEFAULT);
	free(tmp);
	
	/* convert the markdown to html */
	p->content = cmark_render_html(t_root, CMARK_OPT_DEFAULT);
	
	p->title = get_post_title(t_root);

	p->time = get_post_time(file);

	p->url = create_post_url(p->time, p->title);
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
	char t_time[16] = "";
	strncpy(t_time, file, 16);
	if(strptime(t_time, "%Y-%m-%d-%H-%M", time) == NULL)
		printf("ERROR: Failed to convert time\n");
	return time;
}

char *create_post_url(struct tm *time, char *title)
{
	char *t_url = malloc(MAX_URL_CHARS);
		sprintf(t_url, "%d/%02d/%02d/%s.html", 
			time->tm_year + 1900,
			time->tm_mon,
			time->tm_mday,
			title);
	char *c;
	while((c = strchr(t_url, ' ')))
		*c = '-';
	return t_url;
}

int write_post(struct post *post)
{
	char *dir = malloc(MAX_URL_CHARS);
	sprintf(dir, "%s%s", HTMLDIR, post->url);

	printf("DEBUG: %s\n", dir);

	printf("%d\n", create_directory(dir));

	FILE *f = fopen(post->url, "w");
	fprintf(f, post->content);
	fclose(f);
}


/* Reads a text file into a char* */
char *read_text(const char *path, int maxLength)
{
	char *o = malloc(maxLength);
	FILE *f = fopen(path, "r");
	int c;
	char *t = o;
	while((c = fgetc(f)) != EOF && --maxLength > 0)
		*(t++) = (char)c;
	*t = '\0';
	fclose(f);
	return o;
}

/* Creates a directory structure based on the path provided */
int create_directory(const char *path)
{
	int status;
	mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	status = errno;
	/*
	char t_buf[MAX_URL_CHARS] = "";
	char *dir = strtok(path, "/");
	do {
		strcat(t_buf, dir);
		strcat(t_buf, "/");
		mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		status = errno;
		printf("%s\t%d\n", t_buf, status);
		dir = strtok(NULL, "/");
	} while (dir != NULL);
	*/

	return status;
}

