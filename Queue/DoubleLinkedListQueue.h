//
// Created by Denis on 12/26/2025.
//

#ifndef DOUBLELINKEDLISTQUEUE_H
#define DOUBLELINKEDLISTQUEUE_H

typedef struct node * node_t;

typedef struct queue * queue_t;

void queue_init(queue_t *queue);

void enqueue(queue_t queue, void *data);

void *dequeue(queue_t queue);

void *peek(const struct queue* queue);

void print_queue(const struct queue* queue, void (*print)(void *));

int get_size(const struct queue* queuee);

int is_empty(const struct queue* queue);
#endif //DOUBLELINKEDLISTQUEUE_H
