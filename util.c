#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

#define MAX_CHARS 512

char buf[MAX_CHARS];

/* Creates a directory structure based on the path provided */
int create_directory(const char *path)
{
	char c;
	memcpy(buf, path, MAX_CHARS);

	char *p = buf;

	while((c = *(++p)))
		if(c == '/') {
			*p = '\0';
			if(mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
				if(dir_error(errno, buf) < 0) return -1;
			*p = '/';
		}
	if(mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
		if(dir_error(errno, buf) < 0) return -1;
	return 0;
}

/* Error checking for create_directory */
int dir_error(int status, const char* dir)
{
	switch(status) {
		case 0:
		case EEXIST:
			break;
		case EACCES:
			printf("create_directory error: %s permission denied\n", dir);
			return -1;
			break;
		case ENAMETOOLONG:
			printf("create_directory error: path %s too long\n", dir);
			return -1;
			break;
		case ENOTDIR:
			printf("create_directory error: non-directory %s exists\n", dir);
			return -1;
			break;
	}
	return 0;
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

/* Copies a file from source to destination */
int copy_file(const char *source, const char *dest)
{
	FILE *s = fopen(source, "r");
	if(s == NULL) {
		errprintf("copy_file", errno);
		return -1;
	}
	FILE *d = fopen(dest, "w");
	if(s == NULL) {
		errprintf("copy_file", errno);
		return -1;
	}
	int c = fgetc(s);
	while(c != EOF) {
		if(fputc(c, d) == EOF) {
			errprintf("copy_file", errno);
			return -1;
		}
		c = fgetc(s);
	}
	fclose(s);
	fclose(d);
	return 0;
}

/* A uniform error message printing thing */
void errprintf(const char *function, int error)
{
	printf("*** %s ERROR: %d\n", function, error);
}

