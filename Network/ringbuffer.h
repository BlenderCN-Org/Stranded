#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H 1

#include "XXXXXXX.h"

typedef struct 
{
	TagItem data;
	time_t timestamp;
}item_t;

struct node
{
	item_t item;
	struct node *next;
};
typedef struct node node_t;

typedef struct
{
	node_t *read;
	node_t *write;
	pthread_mutex_t readlock;
	pthread_mutex_t writelock;
	int node_num;
	int max_node_num;
}ringbuffer_t;

int ringbuffer_init(ringbuffer_t *ringbuffer, int max_node_num);
int ringbuffer_clean(ringbuffer_t *ringbuffer);
int ringbuffer_read(ringbuffer_t *ringbuffer, item_t *item);
int ringbuffer_write(ringbuffer_t *ringbuffer, const item_t *item);

#endif
