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
            TRACE
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
    fat_block_t *fat_item = (fat_block_t *)malloc(sizeof(fat_block_t));
    int result = 0;
    memset(fat_item->blocks, 0, sizeof(size_t) * BLOCK_COUNT);    

    for(i = 0; i < BLOCK_COUNT; i++)
    {
        switch(i)
        {
            case 0:
                {
                    fat_item->blocks[0] = 0;
                    break;
                }
            default:
                {
                    fat_item->blocks[0] = i;
                    break;
                }
        }

        if (write_fat_item(fat_item, i) < 0)
        {
            perror("FAT ITEM WRITE OPERATION WAS FAILED");
            result = -1;
            break;
        }
    }
    TRACE
    free(fat_item);
    return result;
}

int write_fat_item(fat_block_t *fat_item, size_t number)
{
    if (lseek(filesystem_fd, sizeof(fat_block_t) * number, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, fat_item, sizeof(fat_block_t)) == sizeof(fat_block_t))
            return 0;
    }
    return -1;
}

int read_fat_item(fat_block_t *fat_item, size_t number)
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
    fat_block_t *fat_item = (fat_block_t *)malloc(sizeof(fat_block_t));
    data_block_t *block = (data_block_t *)malloc(sizeof(data_block_t));

    for(i = 0; i < BLOCK_COUNT; i++)
    {
        if (read_fat_item(fat_item, i) != 0)
        {
            perror("READ FAT ITEM OPERATION FAILED");
            break;
        }

        if (read_block(block, fat_item->blocks[0]) != 0)
        {
            perror("READ BLOCK OPERATION FAILED");
            break;
        }
        else
        {
            if (block->status == BLOCK_FREE)
            {
                free_block_number = i;
                break;
            }
        }
    }

    free(fat_item);
    free(block);
    return free_block_number;
}

int search_fat_item_of_block(size_t block_number)
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
            if (fat_item->blocks[0] == block_number)
            {
                result_block_number = i;
                break;
            }
        }
    }

    free(fat_item);
    return result_block_number;
}

int create_data_blocks()
{
    int i;
    int result = 0;
    data_block_t *block = (data_block_t *)malloc(sizeof(data_block_t));

    memset(block->name, '\0', sizeof(BLOCK_NAME_SIZE));
    memset(block->data, '\0', sizeof(BLOCK_DATA_PART_SIZE));

    TRACE
    for(i = 0; i < BLOCK_COUNT; i++)
    {
        if (i == 0)
            block->status = BLOCK_FOLDER;
        else
            block->status = BLOCK_FREE;

        if (write_block(block, i) != 0)
        {
            perror("WRITE BLOCK OPERATION FAILED");
            result = -1;
            break;
        }
    }
    TRACE

    free(block);
    TRACE
    return result;
}

int write_block(data_block_t *block, size_t number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(data_block_t) * number, SEEK_SET) >= 0)
    {
        if (write(filesystem_fd, block, sizeof(data_block_t)) == sizeof(data_block_t))
        {
            return 0;
        }
    }
    return -1;
}

int read_block(data_block_t *block, size_t number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(data_block_t) * number, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, block, sizeof(data_block_t)) == sizeof(data_block_t))
        {
            return 0;
        }
    }
    return -1;
}

int set_block_name(char *name, size_t number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(fat_block_t) * number + BLOCK_NAME_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, name, BLOCK_NAME_SIZE) == BLOCK_NAME_SIZE)
                return 0;
    }
    return -1;
}

int set_block_status(status_block_t newstatus, size_t number)
{
    if (lseek(filesystem_fd,TO_START_DATA_BLOCK_OFFSET + sizeof(fat_block_t) * number + BLOCK_STATUS_OFFSET, SEEK_SET) >= 0)
    {
            if (write(filesystem_fd, &newstatus, sizeof(status_block_t)) == sizeof(status_block_t))
                return 0;
    }
    return -1;
}

int set_block_stats(stat_t *newstats, size_t number)
{
    if (lseek(filesystem_fd, TO_START_DATA_BLOCK_OFFSET + sizeof(BLOCK_SIZE) * number + BLOCK_STATS_OFFSET, SEEK_SET) >= 0)
    {
        if (read(filesystem_fd, newstats, sizeof(stat_t)) == sizeof(stat_t))
        {
            return 0;
        }
    }
    return -1;
}

int set_block_data(size_t *data, size_t number)
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
