#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "ringbuffer.h"

int ringbuffer_init(ringbuffer_t *ringbuffer, int max_node_num)
{
	node_t *node;

	if (ringbuffer == NULL)
		return -1;

	if (max_node_num < 2)
		return -2;

	ringbuffer->max_node_num = max_node_num;
	ringbuffer->node_num = 0;

	if (pthread_mutex_init(&ringbuffer->readlock, NULL) != 0)
	{
		return -3;
	}

	if (pthread_mutex_init(&ringbuffer->writelock, NULL) != 0)
	{
		pthread_mutex_destroy(&ringbuffer->readlock);
		return -4;
	}

	if ((node=malloc(sizeof(node_t))) == NULL)
	{
		pthread_mutex_destroy(&ringbuffer->readlock);
		pthread_mutex_destroy(&ringbuffer->writelock);
		return -5;
	}

	node->next = node;
	ringbuffer->read = ringbuffer->write = node;
	ringbuffer->node_num++;
#if 1
	int i;
	for (i=0; i<max_node_num; i++)
	{
		if ((node=malloc(sizeof(node_t))) != NULL)
		{
			node->next = ringbuffer->read->next;
			ringbuffer->read->next = node;
			ringbuffer->node_num++;
		}
		else
		{
			ringbuffer_clean(ringbuffer);
			return -6;
		}
	}
#endif

	return 0;
}

int ringbuffer_clean(ringbuffer_t *ringbuffer)
{
	node_t *head, *node;

	if (ringbuffer == NULL)
		return -1;

	if (ringbuffer->read == NULL)
		return -2;

	pthread_mutex_destroy(&ringbuffer->writelock);
	pthread_mutex_destroy(&ringbuffer->readlock);

	head = ringbuffer->read;
	do
	{
		node = ringbuffer->read->next;
		free(ringbuffer->read);
		ringbuffer->node_num--;
		ringbuffer->read = node;
	}
	while (ringbuffer->read != head);

	ringbuffer->read = ringbuffer->write = NULL;

	return 0;
}

int ringbuffer_read(ringbuffer_t *ringbuffer, item_t *item)
{
	if (ringbuffer->read == ringbuffer->write)
	{
		return -1;
	}
	else
	{
		memcpy((char *)item, (char *)&(ringbuffer->read->item), sizeof(item_t));
		ringbuffer->read = ringbuffer->read->next;

		return 0;
	}
}

int ringbuffer_write(ringbuffer_t *ringbuffer, const item_t *item)
{
	node_t *node;

	pthread_mutex_lock(&ringbuffer->writelock);

	if (ringbuffer->write->next == ringbuffer->read)
	{
		if (ringbuffer->node_num >= ringbuffer->max_node_num)
		{
			pthread_mutex_unlock(&ringbuffer->writelock);
			return -1;
		}

		if ((node=malloc(sizeof(node_t))) == NULL)
		{	
			pthread_mutex_unlock(&ringbuffer->writelock);
			return -2;
		}

		ringbuffer->node_num++;

		memcpy((char *)&(ringbuffer->write->item), (char *)item, sizeof(item_t));
		node->next = ringbuffer->write->next;
		ringbuffer->write->next = node;
		ringbuffer->write = ringbuffer->write->next;

		pthread_mutex_unlock(&ringbuffer->writelock);

		return 0;
	}
	else
	{
		memcpy((char *)&(ringbuffer->write->item), (char *)item, sizeof(item_t));
		ringbuffer->write = ringbuffer->write->next;
		
		pthread_mutex_unlock(&ringbuffer->writelock);

		return 0;
	}
}
