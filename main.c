#include "cmark.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

int md_filter(const struct dirent *);

struct post {
	char *title;
	char *url;
	char * content;
	struct tm *time;
};

struct post *create_post(const char *path);
int create_all_posts(struct post *posts);
int create_index(struct post *posts, int totalPosts);
int create_archive(struct post *posts, int totalPosts);
int create_rss(struct post *posts, int totalPosts);

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
	
	for( ; filecount > 0 ; filecount--) {
		printf("%ld\t%s\n", (*files)->d_ino, (*files)->d_name);
		files++;
	}
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

/* Returns a pointer to a post struct from a .md file */
struct post *create_post(const char *path)
{

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
