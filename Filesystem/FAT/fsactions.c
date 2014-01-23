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
            if (create_fat() != 0 || create_data_blocks() != 0)
            {
                perror("CREATE NEW FILE SYSTEM OPERATION WAS FAILED");
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
        memset(fat_item->name, '\0', 256);
            
        switch(i)
        {
            case 0:
                {
                    fat_item->name[0] = '/';
                    fat_item->status = BLOCK_FOLDER;
                    break;
                }
            default:
                {
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
    fat_block_t *fat_item;

    for(i = 0; i < BLOCK_COUNT; i++)
    {
        fat_item = (fat_block_t *)malloc(sizeof(fat_block_t));
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
    int result_block_number = -1;

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
                result_block_number = i;
                break;
            }
        }
    }
    free(fat_item);
    return result_block_number;
}

int set_fat_item_name(char *name, unsigned int number)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number + FAT_NAME_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, name, FAT_NAME_MAX_SIZE) == FAT_NAME_MAX_SIZE)
                return 0;
    }
    return -1;
}

int set_fat_item_status(status_block_t newstatus, unsigned int number)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number + FAT_STATUS_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, &newstatus, sizeof(status_block_t)) == sizeof(status_block_t))
                return 0;
    }
    return -1;
}

int set_fat_item_first_block(unsigned int new_first_block, unsigned int number)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number + FAT_FIRST_BLOCK_NUMBER_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, &new_first_block, sizeof(int)) == sizeof(int))
                return 0;
    }
    return -1;
}

int create_data_blocks()
{
    int i;
    data_block_t *block;
    int result = 0;

    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET, SEEK_SET) >= 0)
    {
        for(i = 1; i < BLOCK_COUNT; i++)
        {
            block = (data_block_t *)malloc(sizeof(BLOCK_SIZE));
            block->next_block_number = 0; 
            memset(block->data, '\0', BLOCK_DATA_PART_SIZE); 
            if (write_block(block, i) != 0)
            {
                perror("WRITE BLOCK OPERATION FAILED");
                result = -1;
                break;
            }
            free(block);
        }
    }

    free(block);
    return result;
}

int write_block(data_block_t *block, unsigned int number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(BLOCK_SIZE) * number, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, block, sizeof(BLOCK_SIZE)) == sizeof(BLOCK_SIZE))
        {
            return 0;
        }
    }
    return -1;
}

int read_block(data_block_t *block, unsigned int number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(BLOCK_SIZE) * number, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, block, sizeof(BLOCK_SIZE)) == sizeof(BLOCK_SIZE))
        {
            return 0;
        }
    }
    return -1;
}

int set_block_next_number(unsigned int new_next_number, unsigned int number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(BLOCK_SIZE) * number + BLOCK_NEXT_NUMBER_OFFSET, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, new_next_number, sizeof(int)) == sizeof(int))
        {
            return 0;
        }
    }
    return -1;
}

int set_block_stats(stat_t newstats, unsigned int number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(BLOCK_SIZE) * number + BLOCK_STAT_OFFSET, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, new_next_number, sizeof(stat_t)) == sizeof(stat_t))
        {
            return 0;
        }
    }
    return -1;
}

int set_block_data(int *data, unsigned int number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(BLOCK_SIZE) * number + BLOCK_DATA_OFFSET, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, data, sizeof(BLOCK_DATA_PART_SIZE)) == sizeof(BLOCK_DATA_PART_SIZE))
        {
            return 0;
        }
    }
    return -1;
}




