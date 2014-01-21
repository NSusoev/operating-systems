#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "fsactions.h"

// получаем атрибуты файла
int my_getattr(const char *path, struct stat *stbuf);
// получаем сдержимое папки
int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
// определяем опции открытия файла
int my_open(const char *path, struct fuse_file_info *fi);
// читаем данные из открытого файла
int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
// предоставляет возможность записать в открытый файл
int my_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
// создаём папку
int my_mkdir(const char *path, mode_t mode);
// создаём файл
int my_mknod(const char *path, mode_t mode, dev_t dev);
// переименование
int my_rename(const char *old_path, const char *new_path);
// удалям папку
int my_rmdir(const char *path);
// удаляем файл
int my_unlink(const char *path);
// изменить размер файла
int my_truncate(const char *path, off_t size);

int my_getattr(const char *path, struct stat *stbuf)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        int number = search_inode(number_of_root_block, node_names);
        if (number >= 0 && get_inode_stat(number, stbuf) == 0)
        {
            result = 0;
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        int number = search_inode(number_of_root_block, node_names);
        if (number >= 0)
        {
            inode_t *folder = (inode_t *)get_block(number);
            if (folder != NULL)
            {
                if (folder->status == BLOCK_STATUS_FOLDER)
                {
                    result = 0;
                    filler(buf, ".", NULL, 0);
                    filler(buf, "..", NULL, 0);
                    char name[NODE_NAME_MAX_SIZE];
                    stat_t stat;
                    int *start = (int *)folder->content;
                    int *end = (int *)((void *)folder + size_of_block);
                    while (start < end)
                    {
                        if (*start > 0 && get_inode_name(*start, name) == 0 && get_inode_stat(*start, &stat) == 0)
                        {
                            if (filler(buf, name, &stat, 0) != 0)
                            {
                                break;
                            }
                        }
                        start++;
                    }
                }
                destroy_block(folder);
            }
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_open(const char *path, struct fuse_file_info *fi)
{
	return 0;
}


int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        int number = search_inode(number_of_root_block, node_names);
        if (number >= 0)
        {
            inode_t *file = (inode_t *)get_block(number);
            if (file != NULL)
            {
                if (file->status == BLOCK_STATUS_FILE)
                {
                    if (offset < NODE_CONTENT_MAX_SIZE)
                    {
                        if (offset + size > NODE_CONTENT_MAX_SIZE)
                        {
                            size = NODE_CONTENT_MAX_SIZE - offset;
                        }
                        memcpy(buf, file->content + offset, size);
                        result = size;
                    }
                    else
                    {
                        result = 0;
                    }
                }
                destroy_block(file);
            }
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        int number = search_inode(number_of_root_block, node_names);
        if (number >= 0)
        {
            inode_t *file = (inode_t *)get_block(number);
            if (file != NULL)
            {
                if (file->status == BLOCK_STATUS_FILE)
                {
                    if (offset < NODE_CONTENT_MAX_SIZE)
                    {
                        if (offset + size > NODE_CONTENT_MAX_SIZE)
                        {
                            size = NODE_CONTENT_MAX_SIZE - offset;
                        }
                        memcpy(file->content + offset, buf, size);
                        if (file->stat.st_size < offset + size)
                        {
                            file->stat.st_size = offset + size;
                        }
                        if (write_block(number, file) == 0)
                        {
                            result = size;
                        }
                    }
                    else
                    {
                        result = 0;
                    }
                }
                destroy_block(file);
            }
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_mkdir(const char *path, mode_t mode)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        char *name = exclude_last_node_name(node_names);
        if (name != NULL)
        {
            int folder_number = search_inode(number_of_root_block, node_names);
            if (folder_number >= 0)
            {
                int new_folder = create_folder(name, mode);
                if (new_folder >= 0 && add_inode_to_folder(folder_number, new_folder) == 0)
                {
                    result = 0;
                }
            }
            destroy_name(name);
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_mknod(const char *path, mode_t mode, dev_t dev)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        char *name = exclude_last_node_name(node_names);
        if (name != NULL)
        {
            int folder_number = search_inode(number_of_root_block, node_names);
            if (folder_number >= 0)
            {
                int new_file = create_file(name, mode, dev);
                if (new_file >= 0 && add_inode_to_folder(folder_number, new_file) == 0)
                {
                    result = 0;
                }
            }
            destroy_name(name);
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_rename(const char *old_path, const char *new_path)
{
    int result = -ENOENT;
    char **old_node_names = split_path(old_path);
    if (old_node_names != NULL)
    {
        char **new_node_names = split_path(new_path);
        if (new_node_names != NULL)
        {
            char *old_name = exclude_last_node_name(old_node_names);
            if (old_name != NULL)
            {
                char *new_name = exclude_last_node_name(new_node_names);
                if (new_name != NULL)
                {
                    int old_folder_number = search_inode(number_of_root_block, old_node_names);
                    int new_folder_number = search_inode(number_of_root_block, new_node_names);
                    int node_number = search_inode_in_folder(old_folder_number, old_name);
                    remove_node_from_folder(old_folder_number, node_number);
                    add_inode_to_folder(new_folder_number, node_number);
                    set_inode_name(node_number, new_name);
                    result = 0;
                    destroy_name(new_name);
                }
                destroy_name(old_name);
            }
            destroy_node_names(new_node_names);
        }
        destroy_node_names(old_node_names);
    }
    return result;
}


int my_rmdir(const char *path)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        char *name = exclude_last_node_name(node_names);
        if (name != NULL)
        {
            int folder_number = search_inode(number_of_root_block, node_names);
            if (folder_number >= 0)
            {
                int node_number = search_inode_in_folder(folder_number, name);
                if (node_number >= 0)
                {
                    if (remove_node_from_folder(folder_number, node_number) == 0 && remove_block(node_number) == 0)
                    {
                        result = 0;
                    }
                }
            }
            destroy_name(name);
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_unlink(const char *path)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        char *name = exclude_last_node_name(node_names);
        if (name != NULL)
        {
            int folder_number = search_inode(number_of_root_block, node_names);
            if (folder_number >= 0)
            {
                int node_number = search_inode_in_folder(folder_number, name);
                if (node_number >= 0)
                {
                    if (remove_node_from_folder(folder_number, node_number) == 0 && remove_block(node_number) == 0)
                    {
                        result = 0;
                    }
                }
            }
            destroy_name(name);
        }
        destroy_node_names(node_names);
    }
    return result;
}


int my_truncate(const char *path, off_t size)
{
    int result = -ENOENT;
    char **node_names = split_path(path);
    if (node_names != NULL)
    {
        int number = search_inode(number_of_root_block, node_names);
        if (number >= 0)
        {
            stat_t stat;
            if (get_inode_stat(number, &stat) == 0)
            {
                if (size <= NODE_CONTENT_MAX_SIZE)
                {
                    stat.st_size = size;
                    if (set_inode_stat(number, &stat) == 0)
                    {
                        result = 0;
                    }
                }
            }
        }
        destroy_node_names(node_names);
    }
    return result;
}


// структура определенных мной операций
struct fuse_operations my_oper =
{
    .getattr    = my_getattr,
    .readdir    = my_readdir,
    .open       = my_open,
    .read       = my_read,
    .write      = my_write,
    .mkdir      = my_mkdir,
    .mknod      = my_mknod,
    .rename     = my_rename,
    .rmdir      = my_rmdir,
    .unlink     = my_unlink,
    .truncate   = my_truncate,
};

int main(int argc, char *argv[])
{
    if (init() != 0)
    {
        printf("initialization error\n");
        return -1;
    }
    return fuse_main(argc, argv, &my_oper, NULL);
}
