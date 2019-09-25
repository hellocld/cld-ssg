/* 
 * config.h
 *
 * Configuration for cld-ssg. Define stuff like directory paths, max posts on
 * index.html, char* lengths, etc.
 */

/* Root directory for the generated website */
#define HTMLDIR "./testhtml/"
/* Directory where .md posts are saved */
#define POSTDIR	"./testdir/"
#define INDEX_POSTS 10

/* I should probably use some system defaults for these */
#define MAX_POST_CHARS	30000 /* The biggest post I've written thus */
#define MAX_TITLE_CHARS	1000  /* far is 18459 bytes as html */
#define MAX_URL_CHARS	1000  /* hellocld.com/2019/06/11/bitcrushed-5.html */

