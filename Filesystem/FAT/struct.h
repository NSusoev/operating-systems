#ifndef STRUCT_H
#define STRUCT_H
#define FAT_NAME_MAX_SIZE 256

typedef struct stat stat_t;

typedef enum status_block {

    BLOCK_FREE = 0,
    BLOCK_FILE = 1,
    BLOCK_FOLDER = 2

} status_block_t;

typedef struct fat_block {

    char name[FAT_NAME_MAX_SIZE];
    status_block_t status;
    unsigned int first_block_number;

} fat_block_t;

typedef struct data_block {

    unsigned int next_block_number;
    stat_t stats;
    unsigned int data[BLOCK_DATA_PART_SIZE];

} data_block_t;

#endif
