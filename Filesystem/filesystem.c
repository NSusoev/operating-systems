/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fsoperations.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static inode_t *root;

static int fs_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	inode_t *node = search_inode(root, path);
	
    if (node == 0) return -ENOENT;  

    if (node->content == 0)
    {
        stbuf->st_mode = S_IFDIR | 0666;
        stbuf->st_nlink = 2;
    } 
    else
    {
        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(node->content);
    }

	return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
	/*if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES; */

	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;
}

static int fs_mkdir(const char* path, mode_t mode)
{
	char **splited = split(path);
	int nesting_level = 0;

	while(splited[nesting_level++] != 0)
		nesting_level++;

	inode_t *newnode = create_new_inode(splited[nesting_level - 1], 0);
	inode_t *parent = search_inode(root, path);
	add_inode(parent, newnode);

	return 0;
}

static struct fuse_operations fs_oper = {
	.getattr	= fs_getattr,
	.readdir	= fs_readdir,
	.open		= fs_open,
	.read		= fs_read,
	.mkdir      = fs_mkdir,
};

int main(int argc, char *argv[])
{
	root = init();
	log_action("FILE SYSTEM WAS CREATED\n");

	return fuse_main(argc, argv, &fs_oper, NULL);
}
