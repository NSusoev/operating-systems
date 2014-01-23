#include "fsactions.h"

int main()
{
    if (init_file_system() != 0)
    {
        printf("%s\n","init fs error");
        exit(EXIT_FAILURE); 
    }
    
    fat_block_t *fat_item = (fat_block_t *)malloc(sizeof(fat_block_t));
    strcpy(fat_item->name,"somefolder");
    fat_item->status = BLOCK_FOLDER;
    fat_item->first_block_number = 4;

    printf("writed item: %s\n", fat_item->name);
    printf("%d\n", fat_item->status);
    printf("%d\n", fat_item->first_block_number);
    
    if (write_fat_item(fat_item, 5) != 0)
    {
        printf("%s\n","write oper was failed");
        exit(EXIT_FAILURE);
    }
    
    if (read_fat_item(fat_item, 5) != 0)
    {
        printf("%s\n","read oper was failed");
        exit(EXIT_FAILURE);
    }

    printf("readed item: %s\n", fat_item->name);
    printf("%d\n", fat_item->status);
    printf("%d\n", fat_item->first_block_number);

    set_fat_item_name("somefile", 5);
    set_fat_item_status(BLOCK_FILE, 5);
    set_fat_item_first_block(20,5);

    if (read_fat_item(fat_item, 5) != 0)
    {
        printf("%s\n","read oper was failed");
        exit(EXIT_FAILURE);
    }

    printf("readed item: %s\n", fat_item->name);
    printf("%d\n", fat_item->status);
    printf("%d\n", fat_item->first_block_number);

    int free_block = search_free_block();
    printf("free block %d\n", free_block);

    free(fat_item);
    return 0;
}
