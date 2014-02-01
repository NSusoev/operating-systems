#ifndef FSACTIONS_H
#define FSACTIONS_H

#define BEBUG

#ifdef BEBUG 
#define TRACE printf("[BEBUG] FILE:%s LINE:%d\n", __FILE__, __LINE__);
#else
#define TRACE
#endif

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26
#include <fuse.h>

#pragma pack(1)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "struct.h"

#define FS_FILE_NAME "filesystem"
#define BLOCK_NAME_OFFSET 0
#define BLOCK_STATUS_OFFSET (BLOCK_NAME_OFFSET + (sizeof(char) * BLOCK_NAME_MAX_SIZE))
#define BLOCK_STATS_OFFSET (BLOCK_STATUS_OFFSET + sizeof(status_block_t))
#define BLOCK_DATA_OFFSET (BLOCK_STATS_OFFSET + sizeof(stat_t))
#define BLOCK_NAME_SIZE sizeof(char) * BLOCK_NAME_MAX_SIZE

#define TO_START_DATA_BLOCK_OFFSET (sizeof(fat_block_t) * BLOCK_COUNT)

int init_file_system();

int create_fat();

int write_fat_item(fat_block_t *fat_item, size_t number);

int read_fat_item(fat_block_t *fat_item, size_t number);

int search_free_block();

int search_fat_item_of_block(size_t block_number);

int write_block(data_block_t *block, size_t number);

int read_block(data_block_t *block, size_t number);

int create_data_blocks();

int set_block_status(status_block_t newstatus, size_t number);

int set_block_name(char *name, size_t number);

int set_block_stats(stat_t *newstats, size_t number);

int set_block_data(char *data, size_t number);

extern int filesystem_fd;

#endif
