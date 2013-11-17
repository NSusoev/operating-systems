typedef struct http_protocol_t
{
	char *header;
	char *body;
} http_protocol_t;

void answer(int fd);
