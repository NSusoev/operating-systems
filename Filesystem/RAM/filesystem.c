#define FUSE_USE_VERSION 26

#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fsoperations.h"

FILE *file;
static inode_t *tree;
static char* cur_file = "";
static char* cur_content = ""; 

static int my_getattr(const char *path, struct stat *stbuf)
{	
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	inode_t *node = search_inode(tree, path, 0);
	if (node == 0) return -ENOENT;	
	if (node->content == 0)
	{
		stbuf->st_mode = S_IFDIR | 0666;
		stbuf->st_nlink = 2;
	} else
	{
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(node->content);
	}
	return 0; 
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
		
	inode_t *node = search_inode(tree, path, 0);	
	int i = 0;
	for(; i < node->childs_c; i++)
	{
		filler(buf, node->childs[i].name, NULL, 0);
	}
	return 0;	
}

static int my_open(const char *path, struct fuse_file_info *fi)
{
	
	return 0;
}

static int my_truncate(const char *path, off_t newsize)
{
    return 0;
}


static int my_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{		
	inode_t *node = search_inode(tree, path, 0);
	int len = strlen(node->content);
	if (offset < len)
	{
		if (offset + size > len) size = len - offset;
		memcpy(buf, node->content + offset, size);
	} else	size = 0;
	return size;
}

static int my_write(const char *path, const char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{	
	//cur_file = path;
	//cur_content = memcpu(cur_content,buf + offset);
	//return strlen(buf+offset);
    
	inode_t *node = search_inode(tree, path, 0);
	int max_len = 255;
	if (offset < max_len) 
	{
		if (offset + size > max_len) size = max_len - offset;
		memcpy(node->content + offset, buf, size);
	} else	size = max_len;
	return size;
}

static int my_mkdir(const char* path, mode_t mode)
{	
	char** array = split_path(path);
	int ct = 0;	

	while(array[ct] != 0) ct++;
	inode_t *node = create_inode(array[ct - 1], 0);
	inode_t *parent = search_inode(tree, path, 1);
	add_inode(parent, node);
	fprintf(file, "%s\n", "NEW FOLDER WAS CREATED");
	return 0;		
}

static int my_mknod(const char* path, mode_t mode, dev_t dev)
{
	char** array = split_path(path);
	int ct = 0;	

	while(array[ct] != 0) ct++;
	inode_t *node = create_inode(array[ct - 1], "");
	inode_t *parent = search_inode(tree, path, 1);
	add_inode(parent, node);
	fprintf(file, "%s\n", "NEW NODE WAS CREATED");

	return 0;
}

static int my_rename(const char* old, const char* new)
{	
	if (strcmp(old,cur_file) == 0)
	{					
		inode_t *node = search_inode(tree, new, 0);
		node->content = cur_content;
		cur_content = "";
		return 0;
	}

	inode_t *node = search_inode(tree, old, 0);
	char** array = split_path(new);
	int i = 0;
	while(array[i] != 0) i++;
	node->name = array[i - 1];
	return 0;
}

static int my_rmdir(char* path)
{
	inode_t *node = search_inode(tree, path, 0);
	delete_inode(node);
	fprintf(file, "%s\n", "DIRECTORY WAS DELETED");
	return 0;
}

static int my_unlink(char* path)
{
	inode_t *node = search_inode(tree, path, 0);
	delete_inode(node);
	fprintf(file, "%s\n", "FILE WAS DELETED");
	return 0;
}

static struct fuse_operations my_oper = 
{
	.getattr	= my_getattr,
	.readdir	= my_readdir,
	.open		= my_open,
	.read		= my_read,
	.write		= my_write,	
	.mkdir 		= my_mkdir,
	.mknod		= my_mknod,
	.rename 	= my_rename,
	.rmdir		= my_rmdir,
	.unlink 	= my_unlink,
    .truncate   = my_truncate,
};

int main(int argc, char *argv[])
{	
	tree = init_file_system();

	file = fopen("log.txt", "a+");
	if(file == 0)
	{
		perror("log.txt");
		exit(EXIT_FAILURE);
	}

	fprintf(file, "%s\n", "\nFILE SYSTEM WAS CREATED");
	return fuse_main(argc, argv, &my_oper, NULL);
}
