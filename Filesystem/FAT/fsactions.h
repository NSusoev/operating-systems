#ifndef FSACTIONS_H
#define FSACTIONS_H
#include "struct.h"

#define FS_FILE_NAME "filesystem"
#define BLOCK_SIZE 4096
#define BLOCK_COUNT 256
#define BLOCK_NEXT_NUMBER_OFFSET 0
#define BLOCK_STAT_OFFSET (BLOCK_NEXT_NUMBER_OFFSET + sizeof(int))
#define BLOCK_DATA_OFFSET (BLOCK_STAT_OFFSET + sizeof(stat_t))
#define BLOCK_DATA_PART_SIZE (BLOCK_SIZE - (sizeof(int) + sizeof(stat_t)))

#define FAT_NAME_OFFSET 0
#define FAT_STATUS_OFFSET (sizeof(char) * FAT_NAME_MAX_SIZE)
#define FAT_BLOCK_NUMBER (FAT_STAT_OFFSET + sizeof(stat_t))

int init_file_system();
int create_fat();
int write_fat_item(fat_block_t *fat_item, unsigned int number);
int read_fat_item(fat_block_t *fat_item, unsigned int number);
int search_free_block();
int search_fat_item_of_block(unsigned int block_number);


extern int filesystem_fd;

#endif
