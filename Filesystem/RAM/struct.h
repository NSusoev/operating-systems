typedef struct inode
{
	char* name;
	char* content;
	struct inode *parent;
	struct inode *childs;
	size_t childs_c;
} inode_t;

