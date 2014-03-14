
#ifndef _PATH_H
#define _PATH_H

/* Creates NULL terminater list of paths to search for game data files, allocates memory! */
char **get_search_paths();

/* Free alocated memory */
void destroy_search_paths(char** path);

/* Returns a string with found path, allocates memory */
char *find_file(char** path, char* suffix);

#endif
