#ifndef FSACTIONS
#define FSACTIONS

#pragma pack(1)

//#define DEBUG

#ifdef DEBUG
#define TRACE printf("[DEBUG] FILE:%s LINE:%d\n", __FILE__, __LINE__);

#else

#define TRACE
#endif

#define FILESYSTEM "filesystem"
#define NUMBER_OF_ROOT_BLOCK 0
#define SIZE_OF_BLOCK 4096
#define NODE_NAME_MAX_SIZE 256
#define BLOCK_STATUS_OFFSET 0
#define NODE_NAME_OFFSET (BLOCK_STATUS_OFFSET + sizeof(char))
#define NODE_STAT_OFFSET (NODE_NAME_OFFSET + NODE_NAME_MAX_SIZE)
#define NODE_CONTENT_OFFSET (NODE_STAT_OFFSET + sizeof(stat_t))
#define NODE_CONTENT_MAX_SIZE (size_of_block - NODE_CONTENT_OFFSET)

// информация о файле
typedef struct stat stat_t;
// файл или папка
typedef struct node node_t;
// список блоков
typedef struct list list_t;
// блок с данными
typedef struct data data_t;
// логический тип
typedef char bool;

enum boolean
{
    FALSE = 0,
    TRUE = 1,
};

enum block_status
{
    BLOCK_STATUS_FREE = 0,
    BLOCK_STATUS_FOLDER = 1,
    BLOCK_STATUS_FILE = 2,
};

struct node
{
    char status;
    char name[NODE_NAME_MAX_SIZE];
    stat_t stat;
    char content[0];
};

// search
// искать первый свободный блок
int seek_free_block();
// найти узел
int seek_node(int node_number, char **node_names);
// поиск узла в папке
int seek_node_in_folder(int folder_number, const char *node_name);

//create
// загрузить данные из файла в ОЗУ и инициализировать глобальные переменные
int init();
// создать корень
int create_root();
//  создать блок
void *create_block();
// создать папку
int create_folder(const char *name, mode_t mode);
// создать файл
int create_file(const char *name, mode_t mode, dev_t dev);
// создать копию имени
char *create_name(const char *name);
// создать пустое имя
char *create_empty_name();

// parse group
char **create_node_names(const char *path);
// исключить имя последнего узла
char *exclude_last_node_name(char **node_names);

//read group
//  читать блок
int read_block(int number, void *block);

//write group
// записать блок
int write_block(int number, void *block);

//remove & destroy & clear group
// удалить файл
int remove_file(int number);
// удалить папку
int remove_folder(int number);
// удалить блок
int remove_block(int number);
//  уничтожить блок
void destroy_block(void *block);
// освободить память
void destroy_name(char *name);
// освободить память
void destroy_node_names(char **node_names);
// стереть блок
int clear_block(int number);
// удалить узел из папки
int remove_node_from_folder(int folder_number, int node_number);

//get group
// получить блок
void *get_block(int number);
// получить состояние блока
int get_block_status(int number);
// получить имя узла
int get_node_name(int number, char *buf);
// получить атрибуты узла
int get_node_stat(int number, stat_t *stbuf);

//set group
// задать состояние блока
int set_block_status(int number, char status);
// задать имя узла
int set_node_name(int number, char *buf);
// задать атрибуты узла
int set_node_stat(int number, stat_t *buf);

//add group
// добавить узел в папку
int add_node_to_folder(int folder_number, int node_number);

const int size_of_block = SIZE_OF_BLOCK;
int filesystem_fd = -1;
const int number_of_root_block = NUMBER_OF_ROOT_BLOCK;

#endif


