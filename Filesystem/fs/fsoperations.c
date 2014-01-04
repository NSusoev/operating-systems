#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsoperations.h"

inode_t *init()
{
	inode_t *root = (inode_t *)malloc(sizeof(inode_t));
	root->name = "/";
	root->content = 0;
	root->parent = NULL;
	root->childs = NULL;
	root->childs_c = 0;
	return root;
}

inode_t *create_new_node(char *name, char *content);
{
	inode_t *newnode = (inode_t *)malloc(sizeof(inode_t));
	newnode->name = name;
	newnode->content = content;
	newnode->parent = NULL;
	newnode->childs = NULL;
	newnode->childs_c = 0;
	return newnode;
}

void add_inode(inode_t *parent, inode_t *newnode)
{
	newnode->parent = parent;
	parent->childs_c++;
	parent->childs = (inode_t *)realloc(parent->childs, sizeof(inode_t) * parent->childs);
	parent->childs[childs_c - 1] = newnode;
}

void delete_inode(inode_t *delnode)
{
	delnode->parent->childs_c--;
	delnode->parent->childs = (inode_t *)realloc(parent->childs, sizeof(inode_t) * parent->childs_c);
}