#include "fsactions.h"

int main()
{
    printf("%s\n", "vse ok!"); 
    init_file_system(); 
    TRACE

    data_block_t *block = (data_block_t *)malloc(sizeof(data_block_t));
    strcpy(block->name, "somefolder");
    block->status = BLOCK_FOLDER;
    if (write_block(block, 1) != 0)
        printf("%s\n", "write error");
    
    TRACE
    printf("writed item: %s\n", block->name);
    printf("%d\n", block->status);

    data_block_t *block2 = (data_block_t *)malloc(sizeof(data_block_t));
    if (read_block(block2, 1) != 0)
        printf("%s\n", "read error");

    printf("readed 1 item: %s\n", block2->name);
    printf("%d\n", block2->status);

    printf("%d\n",sizeof(data_block_t));
    printf("%d\n",BLOCK_DATA_PART_SIZE);
    free(block);
    free(block2);
    return 0;
}
