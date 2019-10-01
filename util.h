/*
 * Some utility functions I might use in future projects
 */

#ifndef UTIL_H
#define UTIL_H

/* Creates a directory structure based on the path provided */
int create_directory(const char *);

/* Error checking for create_directory */
int dir_error(int, const char *);

/* Reads a text file into a char* */
char *read_text(const char *, int);

/* Copies a file from one location to another */
int copy_file(const char *, const char *);

/* A uniform error message printing thing */
void errprintf(const char *, int);

#endif
