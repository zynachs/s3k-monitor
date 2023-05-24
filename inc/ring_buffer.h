#pragma once
#include <stdbool.h>
#include <stdint.h>

struct ring_buffer {
	void **buffer;
	volatile uint64_t head, tail, size;
};

static inline void rb_init(struct ring_buffer *rb, void **buffer, int size)
{
	rb->buffer = buffer;
	rb->head = 0;
	rb->tail = 0;
	rb->size = size;
}

static inline bool rb_push(struct ring_buffer *rb, void *data)
{
	if (rb->tail - rb->head >= rb->size)
		return false;
	rb->buffer[rb->tail % rb->size] = data;
	__sync_synchronize();
	rb->tail++;
	return true;
}

static inline bool rb_pop(struct ring_buffer *rb, void **data)
{
	if (rb->tail == rb->head)
		return false;
	*data = rb->buffer[rb->head % rb->size];
	__sync_synchronize();
	rb->head++;
	return true;
}
