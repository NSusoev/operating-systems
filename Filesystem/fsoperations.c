#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsoperations.h"

extern inode_t *init()
{
	inode_t *root = (inode_t *)malloc(sizeof(inode_t));
	root->name = "/";
	root->content = 0;
	root->parent = NULL;
	root->childs = NULL;
	root->childs_c = 0;
	return root;
}

extern inode_t *create_new_inode(char *name, char *content)
{
	inode_t *newnode = (inode_t *)malloc(sizeof(inode_t));
	newnode->name = name;
	newnode->content = content;
	newnode->parent = NULL;
	newnode->childs = NULL;
	newnode->childs_c = 0;
	return newnode;
}

extern void add_inode(inode_t *parent, inode_t *newnode)
{
	newnode->parent = parent;
	parent->childs_c++;
	parent->childs = (inode_t *)realloc(parent->childs, sizeof(inode_t) * parent->childs_c);
	parent->childs[parent->childs_c - 1] = *newnode;
}

extern void delete_inode(inode_t *delnode)
{
	int i = 0;

	for(; i < delnode->parent->childs_c; i++)
	{	
		if(strcmp(delnode->parent->childs[i].name, delnode->name) == 0)
			break;
	}

	delnode->parent->childs_c--;

	if (i < delnode->parent->childs_c - 1)
	{
		delnode->parent->childs[i] = delnode->parent->childs[delnode->parent->childs_c];
		delnode->parent->childs = (inode_t *)realloc(delnode->parent->childs, sizeof(inode_t) * delnode->parent->childs_c);
    }
    else
    {
    	delnode->parent->childs = (inode_t *)realloc(delnode->parent->childs, sizeof(inode_t) * delnode->parent->childs_c);
    }
} 

extern void log_action(char *action)
{
	FILE *file;
	file = fopen("log.txt", "a+");
	if(file == 0)
	{
		perror("log.txt");
		exit(EXIT_FAILURE);
	}

	fprintf(file, "%s\n", action);
	fclose(file);
}

extern char **split(char *path)
{
	char *ppath = path;
	int  nesting_level = 1;
	int  i = 2;
	char **result;

	if(strlen(path) > 1)
	{
		while(*ppath != 0)
		{	
			if(*ppath == '/')
			{
				nesting_level++;
			}
			ppath++;
		}

		result = (char **)malloc(sizeof(char) * nesting_level);
		path++;
		result[0] = "/";
		result[1] = strtok(path, "/");

		for(; i < nesting_level; i++)
		{
			result[i] = strtok(NULL, "/");
		}
	}
	else
	{
		result = (char**)malloc(sizeof(char) * 2); 
        result[1] = 0;
        result[0] = "/";
	}

	return result;
}

extern inode_t *search_inode(inode_t *root,char *path)
{
	char **splited = split(path);
	int parts_count = 0;
	int i = 0;

	while(splited[i++] != 0) parts_count++;

	if(parts_count == 1) return root;

	inode_t *curnode = root;
	i = 1;

	while(splited[i++] != 0)
	{
		int j = 0;
		for(; j < curnode->childs_c; j++)
		{
			if(strcmp(curnode->childs[j].name, splited[i]) == 0)
			{
				curnode = &curnode->childs[j];
				break;
			}
		}
		i++;
	}		

	if(strcmp(curnode->name, splited[parts_count - 1]) != 0)
		return NULL;

	return curnode;
} 