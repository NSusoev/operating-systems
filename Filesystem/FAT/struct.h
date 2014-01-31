#ifndef STRUCT_H
#define STRUCT_H
#define BLOCK_NAME_MAX_SIZE 256
#define BLOCK_COUNT 256
#define BLOCK_SIZE 4096

typedef struct stat stat_t;

typedef enum status_block {

    BLOCK_FREE = 0,
    BLOCK_FILE = 1,
    BLOCK_FOLDER = 2

} status_block_t;

typedef struct fat_block {

    size_t blocks[BLOCK_COUNT];

} fat_block_t;

#define BLOCK_DATA_PART_SIZE (BLOCK_SIZE - ((sizeof(char) * BLOCK_NAME_MAX_SIZE + sizeof(status_block_t) + sizeof(stat_t))))

typedef struct data_block {

    char name[BLOCK_NAME_MAX_SIZE];
    status_block_t status;
    stat_t stats;
    char data[BLOCK_DATA_PART_SIZE];

} data_block_t;

#endif
