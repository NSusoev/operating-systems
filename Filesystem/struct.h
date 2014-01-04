typedef struct inode
{
    char *name;
    char *content;
    struct inode *parent;
    struct inode *childs;
    unsigned int childs_c;

} inode_t;

