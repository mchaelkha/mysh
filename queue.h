/// file: queue.h
/// purpose: Declare an interface for a queue data type.
/// @author Michael Kha, mxk5025@rit.edu
#ifndef QUEUE_H
#define QUEUE_H

/// Queue struct using only arrays
struct Queue_S {
    void **contents;
    size_t size;
    size_t capacity;
    size_t oldest;
};

/// Pointer declaration to the queue struct
typedef struct Queue_S *Queue;

/// Reallocate memory to the queue to increase its capacity
void requeue(Queue queue);

/// Create an empty queue with a capacity
Queue create_queue(size_t capacity);

/// Destroy the queue by deallocating all memory
void destroy_queue(Queue queue);

/// Add to the back of the queue
void enqueue(Queue queue, void *item);

/// Remove from the front of the queue
void *dequeue(Queue queue);

#endif

