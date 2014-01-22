#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "fsactions.h"

int filesystem_fd = -1;

int init_file_system()
{
    filesystem_fd = open(FS_FILE_NAME, O_RDWR, 0666);
    if (filesystem_fd < 0)
    {
        filesystem_fd = open(FS_FILE_NAME, O_CREAT | O_RDWR, 0666);
        if (filesystem_fd >= 0)
        {
            if (create_root_catalog() != 0)
            {
                perror("CREATE FAT OPERATION WAS FAILED");
                return -1;
            }
        }
    } 
    return 0;
}

int create_fat()
{
    int i;

    for(i = 0; i < BLOCK_COUNT; i++)
    {
        fat_block_t *fat_item = (fat_block_t *)malloc(sizeof(fat_block_t));
            
        switch(i)
        {
            case 0:
                {
                    fat_item->name[0] = '/';
                    fat_item->name[1] = NULL;
                    fat_item->status = BLOCK_FOLDER;
                    break;
                }
            default:
                {
                    fat_item->name[0] = NULL;
                    fat_item->status = BLOCK_FREE;
                    break;
                }
        }
        fat_item->first_block_number = i;

        if (write_fat_item(fat_item, i) < 0)
        {
            perror("FAT ITEM WRITE OPERATION WAS FAILED");
            return -1;
        }
        free(fat_item);
    }
    return 0;
}

int write_fat_item(fat_block_t *fat_item, unsigned int number)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, fat_item, sizeof(fat_block_t)) == sizeof(fat_block_t))
            return 0;
    }
    return -1;
}

int read_fat_item(fat_block_t *fat_item, unsigned int number)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, fat_item, sizeof(fat_block_t)) == sizeof(fat_block_t))
            return 0;
    }
    return -1;
}

int search_free_block()
{
    int i;
    int free_block_number = -1;

    for(i = 0; i < BLOCK_COUNT; i++)
    {
        fat_block_t *fat_item = (fat_block_t *)malloc(sizeof(fat_block_t));
        if (read_fat_item(fat_item, i) != 0)
        {
            perror("READ FAT ITEM OPERATION FAILED");
            break;
        }

        if (fat_item->status == 0)
        {
            free_block_number = fat_item->first_block_number;
            break;
        }
    }
    free(fat_item);
    return free_block_number;
}

int search_fat_item_of_block(unsigned int block_number)
{
    int i;
    int block_number = -1;

    fat_block_t *fat_item = (fat_block_t *)malloc(sizeof(fat_block_t));
    for(i = 0; i < BLOCK_COUNT; i++)
    {
        if (read_fat_item(fat_item, i) != 0)
        {
            perror("READ FAT ITEM OPERATION FAILED");
            break;
        }
        else
        {
            if (fat_item->first_block_number == block_number)
            {
                block_number = i;
                break;
            }
        }
    }
    free(fat_item);
    return block_number;
}

int set_fat_item_name(unsigned int number, char *name)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number + FAT_NAME_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, name, FAT_NAME_MAX_SIZE) == FAT_NAME_MAX_SIZE)
                return 0;
    }
    return -1;
}

int set_fat_item_status(unsigned int number, status_block_t newstatus)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number + FAT_STATUS_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, &newstat, sizeof(status_block_t)) == sizeof(status_block_t))
                return 0;
    }
    return -1;
}

int set_fat_item_first_block(unsigned int number, unsigned int new_first_block)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number + FAT_STATUS_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, &new_first_block, sizeof(int)) == sizeof(int))
                return 0;
    }
    return -1;
}


