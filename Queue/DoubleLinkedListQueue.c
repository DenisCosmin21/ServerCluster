//
// Created by Denis on 12/26/2025.
//

#include "DoubleLinkedListQueue.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct node {
    void *data;
    node_t next;
};

struct queue {
    node_t front;
    node_t back;
    size_t size;
};

static struct node *create_node(void *data) {
    node_t node = malloc(sizeof(struct node));

    if(node == NULL) {
        perror("Eroare alocare");
        exit(-1);
    }

    node->data = data;
    node->next = NULL;

    return node;
}

void queue_init(queue_t *queue) {
    *queue = malloc(sizeof(struct queue));

    (*queue)->front = NULL;

    (*queue)->back = NULL;

    (*queue)->size = 0;
}

void enqueue(queue_t queue, void *data) {
    node_t new_node = create_node(data);

    if(queue->front == NULL)
        queue->front = new_node;

    if(queue->back == NULL)
        queue->back = new_node;
    else {
        queue->back->next = new_node;
        queue->back = new_node;
    }

    queue->size++;
}

void *dequeue(queue_t queue) {
    if(queue->front == NULL)
        return NULL;

    node_t front_node = queue->front;

    queue->front = front_node->next;

    if(queue->front == NULL)
        queue->back = NULL;

    void *data = front_node->data;

    free(front_node);

    queue->size--;

    return data;
}

int is_empty(const struct queue* queue) {
    return queue->size == 0;
}

int get_size(const struct queue* queue) {
    return queue->size;
}

void *peek(const struct queue* queue) {
    if(queue->front == NULL)
        return NULL;

    return queue->front->data;
}

void print_queue(const struct queue* queue, void (*print)(void *)) {
    node_t front_node = queue->front;

    while(front_node != NULL) {
        print(front_node->data);
        front_node = front_node->next;
    }

    printf("\n");
}