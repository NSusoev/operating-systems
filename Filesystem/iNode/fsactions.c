#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "fsactions.h"

const int size_of_block = SIZE_OF_BLOCK;
int filesystem_fd = -1;
const int number_of_root_block = NUMBER_OF_ROOT_BLOCK;

int init()
{
    int result = 0;
    // пытаемся открыть существующий файл с файловой системой
    filesystem_fd = open(FILESYSTEM, O_RDWR, 0666);
    if (filesystem_fd < 0)
    {
        // создаем новый
        filesystem_fd = open(FILESYSTEM, O_CREAT | O_RDWR, 0666);
        if (filesystem_fd < 0 || create_root() != 0)
        {
            result = -1;
        }
    }
    return result;
}


int create_root()
{
    int result = -1;
    inode_t *root = (inode_t *)create_block();
    if (root != NULL)
    {
        root->status = BLOCK_STATUS_FOLDER;
        root->name[0] = '\0';
        root->stat.st_mode = S_IFDIR | 0777;
        root->stat.st_nlink = 2;
        if (write_block(number_of_root_block, root) == 0)
        {
            result = 0;
        }
        destroy_block(root);
    }
    return result;
}


void *create_block()
{
    return calloc(size_of_block, sizeof(char));
}


void destroy_block(void *block)
{
    free(block);
}


int read_block(int number, void *block)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, block, size_of_block) >= 0)
        {
            result = 0;
        }
    }
    return result;
}


int write_block(int number, void *block)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, block, size_of_block) == size_of_block)
        {
            result = 0;
        }
    }
    return result;
}


int search_free_block()
{
    int number = number_of_root_block + 1;
    char status;
    int read_result;
    while (TRUE)
    {
        if (lseek(filesystem_fd,  size_of_block * number, SEEK_SET) < 0)
        {
            number = -1;
            break;
        }
        read_result = read(filesystem_fd, &status, sizeof(char));
        if (read_result < 0)
        {
            number = -1;
            break;
        }
        if (read_result == 0 || status == BLOCK_STATUS_FREE)
        {
            break;
        }
        number++;
    }
    return number;
}


void *get_block(int number)
{
    void *block = NULL;
    if (number >= 0)
    {
        block = create_block();
        if (block != NULL && read_block(number, block) != 0)
        {
            destroy_block(block);
            block = NULL;
        }
    }
    return block;
}


int set_block_status(int number, char status)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number + BLOCK_STATUS_OFFSET, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, &status, sizeof(char)) == sizeof(char))
        {
            result = 0;
        }
    }
    return result;
}


int remove_block(int number)
{
    int result = 0;
    int status = get_block_status(number);
    switch (status)
    {
        case BLOCK_STATUS_FREE:
            break;
        case BLOCK_STATUS_FOLDER:
            remove_folder(number);
            break;
        case BLOCK_STATUS_FILE:
            remove_file(number);
            break;
        default:
            result = -1;
            break;
    }
    return result;
}


int search_inode(int node_number, char **node_names)
{
    int result = -1;
    if (node_number >= 0 && node_names != NULL)
    {
        if (*node_names == NULL)
        {
            result = node_number;
        }
        else
        {
            int next_node_number = search_inode_in_folder(node_number, *node_names);
            if (next_node_number > 0)
            {
                result = search_inode(next_node_number, node_names + 1);
            }
        }
    }
    return result;
}


int remove_file(int number)
{
    return set_block_status(number, BLOCK_STATUS_FREE);
}


int remove_folder(int number)
{
    int result = -1;
    inode_t *folder = (inode_t *)get_block(number);
    if (folder != NULL)
    {
        int *start = (int *)folder->content;
        int *end = (int *)((void *)folder + size_of_block);
        while (start < end)
        {
            if (*start > 0)
            {
                 remove_block(*start);
            }
            start++;
        }
        destroy_block(folder);
        result = set_block_status(number, BLOCK_STATUS_FREE);
    }
    return result;
}


int create_folder(const char *name, mode_t mode)
{
    int number = search_free_block();
    if (number >= 0)
    {
        inode_t *folder = (inode_t *)create_block();
        if (folder != NULL)
        {
            int name_size = strlen(name) + 1;
            if (name_size > NODE_NAME_MAX_SIZE)
            {
                name_size = NODE_NAME_MAX_SIZE;
            }
            folder->status = BLOCK_STATUS_FOLDER;
            memcpy(folder->name, name, name_size);
            folder->stat.st_mode = S_IFDIR | mode;
            folder->stat.st_nlink = 2;
            if (write_block(number, folder) != 0)
            {
                number = -1;
            }
            destroy_block(folder);
        }
    }
    return number;
}


int create_file(const char *name, mode_t mode, dev_t dev)
{
    int number = search_free_block();
    if (number >= 0)
    {
        inode_t *file = (inode_t *)create_block();
        if (file != NULL)
        {
            int name_size = strlen(name) + 1;
            if (name_size > NODE_NAME_MAX_SIZE)
            {
                name_size = NODE_NAME_MAX_SIZE;
            }
            file->status = BLOCK_STATUS_FILE;
            memcpy(file->name, name, name_size);
            file->stat.st_mode = S_IFREG | mode;
            file->stat.st_rdev = dev;
            file->stat.st_nlink = 1;
            if (write_block(number, file) != 0)
            {
                number = -1;
            }
            destroy_block(file);
        }
    }
    return number;
}


char **split_path(const char *path)
{
    char **result = NULL;
    int path_size = strlen(path) + 1;
    char *copy_path = (char *)malloc(path_size);
    if (copy_path != NULL)
    {
        memcpy(copy_path, path, path_size);
        int depth = 0;
        int i = 0;
        while (copy_path[i] != '\0')
        {
            if (copy_path[i] == '/')
            {
                depth++;
                copy_path[i] = '\0';
            }
            i++;
        }
        if (copy_path[i - 1] == '\0')
        {
            depth--;
        }
        result = (char **)malloc(sizeof(char **) * (depth + 1));
        if (result != NULL)
        {
            i = 0;
            int j = 0;
            while (j < depth)
            {
                while (copy_path[i++] != '\0');
                result[j++] = create_name(copy_path + i);
            }
            result[j] = NULL;
        }
        free(copy_path);
    }
    return result;
}


int get_block_status(int number)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number + BLOCK_STATUS_OFFSET, SEEK_SET) >= 0)
    {
        char status;
        result = read(filesystem_fd, &status, sizeof(char));
        if (result < 0)
        {
            result = -1;
        }
        else if (result == 0)
        {
            result = BLOCK_STATUS_FREE;
        }
        else
        {
            result = status;
        }
    }
    return result;
}


int get_inode_name(int number, char *buf)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number + NODE_NAME_OFFSET, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, buf, NODE_NAME_MAX_SIZE) == NODE_NAME_MAX_SIZE)
        {
            result = 0;
        }
    }
    return result;
}


int get_inode_stat(int number, stat_t *stbuf)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number + NODE_STAT_OFFSET, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, stbuf, sizeof(stat_t)) == sizeof(stat_t))
        {
            result = 0;
        }
    }
    return result;
}


int set_inode_name(int number, char *buf)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number + NODE_NAME_OFFSET, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, buf, NODE_NAME_MAX_SIZE) == NODE_NAME_MAX_SIZE)
        {
            result = 0;
        }
    }
    return result;
}


int set_inode_stat(int number, stat_t *buf)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number + NODE_STAT_OFFSET, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, buf, sizeof(stat_t)) == sizeof(stat_t))
        {
            result = 0;
        }
    }
    return result;
}


char *create_name(const char *name)
{
    char *result = (char *)calloc(NODE_NAME_MAX_SIZE, sizeof(char));
    if (result != NULL)
    {
        int size = strlen(name) + 1;
        if (size > NODE_NAME_MAX_SIZE)
        {
            size = NODE_NAME_MAX_SIZE;
        }
        memcpy(result, name, size);
    }
    return result;
}


char *create_empty_name()
{
    return (char *)calloc(NODE_NAME_MAX_SIZE, sizeof(char));
}


void destroy_name(char *name)
{
    free(name);
}


char *exclude_last_node_name(char **node_names)
{
    char *result = NULL;
    if (node_names != NULL && *node_names != NULL)
    {
        while (node_names[1] != NULL)
        {
            node_names++;
        }
        result = node_names[0];
        node_names[0] = NULL;
    }
    return result;
}


void destroy_node_names(char **node_names)
{
    if (node_names != NULL)
    {
        char **tmp = node_names;
        while (*tmp != NULL)
        {
            destroy_name(*tmp);
            tmp++;
        }
        free(node_names);
    }
}


int clear_block(int number)
{
    int result = -1;
    if (number >= 0 && lseek(filesystem_fd, size_of_block * number, SEEK_SET) >= 0)
    {
        void *block = create_block();
        if (block != NULL)
        {
            if (write_block(number, block) == 0)
            {
                result = 0;
            }
            destroy_block(block);
        }
    }
    return result;
}


int add_inode_to_folder(int folder_number, int node_number)
{
    int result = -1;
    if (folder_number >= 0 && node_number > 0)
    {
        inode_t *folder = (inode_t *)get_block(folder_number);
        if (folder != NULL)
        {
            if (folder->status == BLOCK_STATUS_FOLDER)
            {
                int *start = (int *)folder->content;
                int *end = (int *)((void *)folder + size_of_block);
                while (start < end)
                {
                    if (*start <= 0)
                    {
                        *start = node_number;
                        break;
                    }
                    start++;
                }
                if (start < end)
                {
                    result = write_block(folder_number, folder);
                }
            }
            destroy_block(folder);
        }
    }
    return result;
}


int remove_node_from_folder(int folder_number, int node_number)
{
    int result = -1;
    if (folder_number >= 0 && node_number > 0)
    {
        inode_t *folder = (inode_t *)get_block(folder_number);
        if (folder != NULL)
        {
            if (folder->status == BLOCK_STATUS_FOLDER)
            {
                int *start = (int *)folder->content;
                int *end = (int *)((void *)folder + size_of_block);
                while (start < end)
                {
                    if (*start == node_number)
                    {
                        *start = 0;
                        break;
                    }
                    start++;
                }
                if (start < end)
                {
                    result = write_block(folder_number, folder);
                }
                else
                {
                    result = 0;
                }
            }
            destroy_block(folder);
        }
    }
    return result;}


int search_inode_in_folder(int folder_number, const char *node_name)
{
    int result = -1;
    if (folder_number >= 0 && node_name != NULL)
    {
        inode_t *folder = (inode_t *)get_block(folder_number);
        if (folder != NULL)
        {
            if (folder->status == BLOCK_STATUS_FOLDER)
            {
                char name[NODE_NAME_MAX_SIZE];
                int *start = (int *)folder->content;
                int *end = (int *)((void *)folder + size_of_block);
                while (start < end)
                {
                    if (*start > 0 && get_inode_name(*start, name) == 0 && strcmp(node_name, name) == 0)
                    {
                        result = *start;
                        break;
                    }
                    start++;
                }
            }
            destroy_block(folder);
        }
    }
    return result;
}
