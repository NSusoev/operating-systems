#ifndef FSOPERATIONS
#define FSOPERATIONS

#include "struct.h"

#ifdef BEBUG
#define TRACE printf("[DEBUG] FILE:%s LINE:%d\n", __FILE__, __LINE__);
#else
#define TRACE
#endif

void log_action(char* action);
inode_t *init_file_system();
void add_inode(inode_t *parent, inode_t *node);
inode_t *create_inode(char* name, char* content);
void delete_inode(inode_t *node);
char** split_path(char* path);
inode_t *search_inode(inode_t *root,char *path, int mode);

#endif







