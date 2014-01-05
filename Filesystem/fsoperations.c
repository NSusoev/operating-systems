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

void log_action(char *action)
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

char **split(char *path)
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

		for(i; i < nesting_level; i++)
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

inode_t *search_inode(inode_t *root, char *name)
{
	char **splited = split(path);
	int parts_count = 0;
	int i = 0;

	while(splited[i++] != 0) count++;

	if(count == 1) return root;

	inode_t *curnode = root;
	i = 1;

	while(splited[i++] != 0)
	{
		int j = 0;
		for(; j < curnode->childs_c; j++)
		{
			if(strcmp(curnode->childs[j], splited[i]) == 0)
			{
				curnode = curnode->childs[j];
				break;
			}
		}
		i++;
	}		

	return curnode;
} 