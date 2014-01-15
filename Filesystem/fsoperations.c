#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsoperations.h"

void log_action(char* action)
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

inode_t *init_file_system()
{
	inode_t *tree = (inode_t *)malloc(sizeof(inode_t));
	tree->name = "/";
	tree->content = NULL;
	tree->parent = NULL;
	tree->childs = NULL;
	tree->childs_c = 0;
	return tree;
}

void add_inode(inode_t *parent, inode_t *newnode)
{	
	newnode->parent = parent;
	parent->childs_c++;
	parent->childs = (inode_t *)realloc(parent->childs, sizeof(inode_t) * parent->childs_c);
	parent->childs[parent->childs_c - 1] = *newnode;
}

inode_t *create_inode(char* name, char* content)
{
	inode_t *node = (inode_t *)malloc(sizeof(inode_t));
	node->name = name;
	node->content = content;
	node->parent = NULL;
	node->childs = NULL;
	node->childs_c = 0;
	return node;
}

void delete_inode(inode_t *delnode)
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

char** split_path(char* path)
{
	char** array;	
	if (strlen(path) > 1)
	{	
		int count = 0;	
		int i = 0;	
		while(path[i] != 0)
		{
			if (path[i] == '/') count++;
			i++;
		}
		array = (char**)malloc(sizeof(char*)*(count + 2));
		array[count + 1] = 0;
		array[0] = "/";

		int n = 1;
		i = 1;
		while(path[i] != 0)
		{
			int c = 1;			
			int j = i;			
			while((path[j] != '/') && (path[j] != 0))
			{
				c++;j++;i++;
			}
			if (path[i] != 0) i++;
			array[n] = (char *)malloc(sizeof(char)*c);
			array[n][c - 1] = 0;
			n++;	
		}

		n = 1;
		i = 1;
		while(path[i] != 0)
		{			
			int j = i;
			int tmp = i;			
			while((path[j] != '/') && (path[j] != 0))
			{
				array[n][j - tmp] = path[j];
				j++;i++;
			}
			if (path[i] != 0) i++;
			n++;	
		}
	} 
    else
	{
		array = (char**)malloc(sizeof(char*)*2);
		array[1] = 0;
		array[0] = "/";
	}
	return array;

/*	int  nesting_level = 1;
	int  i = 0;
	char **result;
    char *copy_path_for_parse;

	if(strlen(path) > 1)
	{
		while(path[i] != 0)
		{	
			if(path[i] != '/')
			{
				nesting_level++;
			}
			i++;
		}

		result = (char **)malloc(sizeof(char *) * nesting_level + 2);
        copy_path_for_parse = strdup(path);
        copy_path_for_parse++;
		result[0] = "/";
        result[nesting_level + 1] = 0;
		result[1] = strtok(copy_path_for_parse, "/");

		for(i = 2; i < nesting_level + 1; i++)
		{
			result[i] = strtok(NULL, "/");
		}
	}
	else
	{
		result = (char**)malloc(sizeof(char *) * 2); 
        result[1] = 0;
        result[0] = "/";
	}

	return result;*/
}

inode_t *search_inode(inode_t *root,char *path, int mode)
{
	char **splited = split_path(path);
	int parts_count = 0;
	int i = 0;

	while(splited[i++] != 0) parts_count++;

	if(parts_count == 1) return root;

	inode_t *curnode = root;
	i = 1;

	while(splited[i] != 0)
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

    if(mode == 0)
    {
        if(strcmp(curnode->name, splited[parts_count - 1]) != 0)
            return NULL;
    }
    else
    {
        if(strcmp(curnode->name, splited[parts_count - 2]) != 0)
            return NULL;
    }

	return curnode;
} 
