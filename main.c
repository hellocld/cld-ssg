#include "cmark.h"
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

int md_filter(const struct dirent *);

struct post {
	char *title;
	char *url;
	char *content;
	struct tm *time;
};

struct post *create_post(const char *path);
char *get_title(struct cmark_node *root);
int create_all_posts(struct post *posts);
int create_index(struct post *posts, int totalPosts);
int create_archive(struct post *posts, int totalPosts);
int create_rss(struct post *posts, int totalPosts);

char *read_text(const char *path, int maxLength);

char buf[MAX_POST_CHARS];

int main()
{
	printf("*** cmark testing ***\n\n");
	char *md = "This is a *test* of some `markdown`";

	char *html = cmark_markdown_to_html(md, strlen(md), CMARK_OPT_DEFAULT);
	printf("%s\n", html);
	free(html);

	printf("*** dirent testing ***\n\n");
	DIR *testdir = opendir("./testdir");
	struct dirent *tde;
	while((tde = readdir(testdir)) != NULL)
		printf("%ld\t%s\n", tde->d_ino, tde->d_name);
	printf("\n\n");
	struct dirent **files;
	int filecount = 0;
	filecount = scandir("./testdir", &files, md_filter, alphasort);
	printf("Found %d files\n", filecount);

	struct dirent **tf = files;
	for( ; filecount > 0 ; filecount--) {
		printf("%ld\t%s\n", (*tf)->d_ino, (*tf)->d_name);
		tf++;
	}

	printf("*** read_file testing ***\n\n");
	char file[MAX_URL_CHARS] = "";
	strcat(file, "./testdir/");
	strcat(file, (*files)->d_name);
	struct post *tp = create_post(file);
	printf("File Path:\t%s\nPost Title:\t%s\n%s\n", file, tp->title, tp->content);
	free(tp->content);
	free(tp->title);
	free(tp);
	free(testdir);
	while(filecount-- > 0)
		free(*files++);
	free(files);
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
struct post *create_post(const char *path)
{
	struct post *p = malloc(sizeof(struct post));


	/* read in the post text file */
	char *tmp = read_text(path, MAX_POST_CHARS);
	struct cmark_node *t_root = cmark_parse_document(tmp, strlen(tmp), CMARK_OPT_DEFAULT);
	free(tmp);
	
	/* convert the markdown to html */
	p->content = cmark_render_html(t_root, CMARK_OPT_DEFAULT);
	
	p->title = get_title(t_root);

	free(t_root);
	return p;
}

/* Gets the post title from the first HEADER in the cmark tree */
char *get_title(struct cmark_node *root)
{
	cmark_iter *t_iter = cmark_iter_new(root);
	while(cmark_iter_next(t_iter) != CMARK_EVENT_DONE)
		if(cmark_node_get_type(cmark_iter_get_node(t_iter)) == CMARK_NODE_HEADING) {
			cmark_iter_next(t_iter);
			break;
		}
	cmark_node *t_title = cmark_iter_get_node(t_iter);
	cmark_iter_free(t_iter);
	return cmark_node_get_literal(t_title);
}

/* Generates all posts found in ./_posts; returns total number of posts */
int create_all_posts(struct post *posts)
{

}

/* Generates index.html using the latest posts */
int create_index(struct post *posts, int totalPosts)
{

}

/* Generates archive.html using all posts */
int create_arcive(struct post *posts, int totalPosts)
{

}

/* Creates an rss feed from all posts */
int create_rss(struct post *posts, int totalPosts)
{

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
