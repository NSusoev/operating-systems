#ifndef STRUCT_H
#define STRUCT_H
#define BLOCK_NAME_MAX_SIZE 256

typedef struct stat stat_t;

typedef enum status_block {

    BLOCK_FREE = 0,
    BLOCK_FILE = 1,
    BLOCK_FOLDER = 2

} status_block_t;

typedef struct fat_block {

    unsigned int blocks[BLOCK_COUNT];

} fat_block_t;

typedef struct data_block {

    char name[BLOCK_NAME_MAX_SIZE];
    status_block_t status;
    stat_t stats;
    unsigned int data[BLOCK_DATA_PART_SIZE];

} data_block_t;

#endif
