//
// Created by Denis on 12/26/2025.
//

#ifndef DOUBLELINKEDLISTQUEUE_H
#define DOUBLELINKEDLISTQUEUE_H

typedef struct node * node_t;

typedef struct queue * queue_t;

void queue_init(queue_t *queue);

void enqueue(queue_t queue, char *data);

char *dequeue(queue_t queue);

void print_queue(queue_t queue);
#endif //DOUBLELINKEDLISTQUEUE_H
