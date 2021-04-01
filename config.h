/* 
 * config.h
 *
 * Configuration for cld-ssg. Define stuff like directory paths, max posts on
 * index.html, char* lengths, etc.
 */

#define WEBSITE "https://hellocld.com/"
/* Root directory for the generated website */
#define HTMLDIR "html/"
/* Directory where .md posts are saved */
#define POSTDIR	"posts/"
/* Directory containing anything you just need copied to HTMLDIR */
#define RESOURCEDIR "files/"
/* Header and footer files */
#define HEADER_HTML "assets/header.html"
#define FOOTER_HTML "assets/footer.html"
#define HEADER_RSS  "assets/headerRSS.xml"
#define FOOTER_RSS  "assets/footerRSS.xml"

#define INDEX_POSTS 10
#define MAX_RSS_POSTS 10 /* seems like a good idea to cap the feed post count */

/* I should probably use some system defaults for these */
#define MAX_POST_CHARS	90000 /* The biggest post I've written thus */
#define MAX_TITLE_CHARS	1000  /* far is 18459 bytes as html */
#define MAX_URL_CHARS	1000  /* hellocld.com/2019/06/11/bitcrushed-5.html */

#define STATIC_PAGE "static"
