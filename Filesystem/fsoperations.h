#include "struct.h"

inode_t *init();
inode_t *search_inode(inode_t *root, char *name);
inode_t *create_new_node(char *name, char *content);
void add_inode(inode_t *parent, inode_t newnode);
void delete_inode(inode_t *node);
void log_action(char *action);
char **split(char *path);


