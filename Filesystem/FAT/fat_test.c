#include "fsactions.h"

int main()
{
    printf("%s\n", "vse ok!"); 
    init_file_system(); 
    TRACE

    data_block_t *block = (data_block_t *)malloc(sizeof(data_block_t));
    strcpy(block->name, "somefolder");
    strcpy(block->data, "dsddsdssomefolder");
    block->status = BLOCK_FOLDER;
    if (write_block(block, 1) != 0)
        printf("%s\n", "write error");
    
    TRACE
    printf("writed item: %s\n", block->name);
    printf("%d\n", block->status);
    printf("%s\n", block->data);

    data_block_t *block2 = (data_block_t *)malloc(sizeof(data_block_t));
    if (read_block(block2, 1) != 0)
        printf("%s\n", "read error");

    printf("readed 1 item: %s\n", block2->name);
    printf("%d\n", block2->status);
    printf("%s\n", block2->data);

    set_block_name("somefile", 1);
    set_block_status(BLOCK_FILE, 1);
    char newdata[BLOCK_DATA_PART_SIZE];
    strcpy(newdata, "somedata");
    set_block_data(newdata, 1);
    data_block_t *block3 = (data_block_t *)malloc(sizeof(data_block_t));
    read_block(block3, 1);

    printf("after sets: %s\n", block3->name);
    printf("%d\n", block3->status);
    printf("%s\n", block3->data);

    TRACE
    int free_block = search_free_block();
    printf("free block: %d\n", free_block);

    TRACE
    int fat_item = search_fat_item_of_block(1);
    printf("fat item of 1 block: %d\n", fat_item);

    TRACE
    printf("%d\n",sizeof(data_block_t));
    printf("%d\n",BLOCK_DATA_PART_SIZE + BLOCK_NAME_SIZE + sizeof(status_block_t) + sizeof(stat_t));
    printf("%d\n",sizeof(int));
    free(block);
    free(block2);
    free(block3);
    free(newdata);
    return 0;
}
