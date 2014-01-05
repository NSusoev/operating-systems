#ifndef FSOPERATIONS
#define FSOPERATIONS

#include "struct.h"

#ifdef BEBUG
#define TRACE printf("[DEBUG] FILE:%s LINE:%d\n", __FILE__, __LINE__);
#else
#define TRACE
#endif

inode_t *init();
inode_t *search_inode(inode_t *root,char *path);
inode_t *create_new_inode(char *name, char *content);
void add_inode(inode_t *parent, inode_t *newnode);
void delete_inode(inode_t *node);
void log_action(char *action);
char **split(char *path);

#endif
