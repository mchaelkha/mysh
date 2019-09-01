/// file: queue.c
/// purpose: Queue data type that uses abstract data types.
/// Contains standard methods for queues.
/// @author Michael Kha, mxk5025@rit.edu
#include <stdlib.h>     // memory allocation
#include <assert.h>
#include "queue.h"

/// Reallocate memory for the given queue. The memory
/// is doubled for its contents and capacity.
/// @param queue The queue to resize its capacity for
void requeue(Queue queue) {
    size_t old_cap = queue->capacity;
    size_t new_cap = old_cap * 2;
    void **contents = (void **) calloc(new_cap, sizeof(void *));
    void **old_contents = queue->contents;
    for (unsigned int i = 0; i < old_cap; i++) {
        contents[i] = old_contents[i];
    }
    free(old_contents);
    queue->contents = contents;
}

/// Create a queue with a given starting capacity.
/// Memory is allocated for the struct and its contents.
/// @param capacity The size of the queue
Queue create_queue(size_t capacity) {
    Queue queue = malloc(sizeof(struct Queue_S));
    void **contents = (void **) calloc(capacity, sizeof(void *));
    queue->contents = contents;
    queue->size = 0;
    queue->capacity = capacity;
    //queue->oldest = 0;
    return queue;
}

/// Deallocate the memory of the queue struct and its contents.
/// @param queue The queue to deallocate
void destroy_queue(Queue queue) {
    assert(queue != 0);
    for (size_t i = 0; i < queue->size; i++) {
        free(queue->contents[i]);
    }
    free(queue->contents);
    free(queue);
}

/// Add to the end of the queue the item. If the capacity
/// is reached, requeue the queue struct.
/// @param queue The queue to add the item to
/// @param item The item to add
void enqueue(Queue queue, void *item) {
    if (queue->size == queue->capacity) {
        void *item = dequeue(queue);
        free(item);
        //queue->oldest = queue->oldest + 1;
    }
    assert(queue != 0);
    void **contents = queue->contents;
    contents[queue->size] = item;
    queue->size = queue->size + 1;
    /*
    if (queue->size == queue->capacity) {
        requeue(queue);
    }*/
}

/// Remove from the front of the queue and return that item.
/// The queue must be non-empty.
/// @param queue The queue to dequeue from
/// @return The item from the front of the queue
void *dequeue(Queue queue) {
    assert(queue != 0);
    assert(queue->size > 0);
    void **contents = queue->contents;
    void *item = contents[0];
    size_t size = queue->size;
    queue->size = queue->size - 1;
    // Shift array down
    for (unsigned int i = 0; i < size - 1; i++) {
        contents[i] = contents[i + 1];
    }
    contents[size - 1] = NULL;
    return item;
}

