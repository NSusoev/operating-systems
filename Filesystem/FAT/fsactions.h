#ifndef FSACTIONS_H
#define FSACTIONS_H

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26
#include <fuse.h>

#define FS_FILE_NAME "filesystem"
#define BLOCK_SIZE 4096
#define BLOCK_COUNT 256
#define BLOCK_NEXT_NUMBER_OFFSET 0
#define BLOCK_STAT_OFFSET (BLOCK_NEXT_NUMBER_OFFSET + sizeof(int))
#define BLOCK_DATA_OFFSET (BLOCK_STAT_OFFSET + sizeof(stat_t))
#define BLOCK_DATA_PART_SIZE (BLOCK_SIZE - ((sizeof(int) + sizeof(stat_t))))

#define FAT_NAME_OFFSET 0
#define FAT_STATUS_OFFSET (sizeof(char) * FAT_NAME_MAX_SIZE)
#define FAT_FIRST_BLOCK_NUMBER_OFFSET (FAT_STATUS_OFFSET + sizeof(status_block_t))
#define TO_START_DATA_BLOCK_OFFSET (sizeof(fat_block_t) * BLOCK_COUNT)

#include "struct.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int init_file_system();
int create_fat();
int write_fat_item(fat_block_t *fat_item, unsigned int number);
int read_fat_item(fat_block_t *fat_item, unsigned int number);
int search_free_block();
int search_fat_item_of_block(unsigned int block_number);
int set_fat_item_first_block(unsigned int new_first_block, unsigned int number);
int set_fat_item_status(status_block_t newstatus, unsigned int number);
int set_fat_item_name(char *name, unsigned int number);
int write_block(data_block_t *block, unsigned int number);
int read_block(data_block_t *block, unsigned int number);
int create_data_blocks();
int set_block_next_number(unsigned int new_next_number, unsigned int number);
int set_block_stats(stat_t newstats, unsigned int number);
int set_block_data(unsigned int *data, unsigned int number);

extern int filesystem_fd;

#endif
