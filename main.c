#include "cmark.h"
#include <stdlib.h>
#include <string.h>

int main()
{
	char *md = "This is a *test* of some `markdown`";

	char *html = cmark_markdown_to_html(md, strlen(md), CMARK_OPT_DEFAULT);
	printf("%s\n", html);
	free(html);

	return 0;
}
