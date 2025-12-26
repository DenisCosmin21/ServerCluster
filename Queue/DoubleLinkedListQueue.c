//
// Created by Denis on 12/26/2025.
//

#include "DoubleLinkedListQueue.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct node {
    char *data;
    struct node *next;
};

struct queue {
    struct node *front;
    struct node *back;
    size_t size;
};

static struct node *create_node(char *data) {
    node_t node = malloc(sizeof(struct node));
    node->data = data;
    node->next = NULL;
}

void queue_init(queue_t *queue) {
    *queue = malloc(sizeof(struct queue));

    (*queue)->front = NULL;

    (*queue)->back = NULL;

    (*queue)->size = 0;
}

void enqueue(queue_t queue, char *data) {
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

char *dequeue(queue_t queue) {
    if(queue->front == NULL)
        return NULL;

    node_t front_node = queue->front;

    queue->front = front_node->next;

    char *data = front_node->data;

    free(front_node);

    queue->size--;

    return data;
}

void print_queue(queue_t queue) {
    node_t front_node = queue->front;

    while(front_node != NULL) {
        printf("%s ", front_node->data);
        front_node = front_node->next;
    }

    printf("\n");
}