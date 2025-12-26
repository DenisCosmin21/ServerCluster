#include <stdio.h>
#include <mpi.h>
#include "Queue/DoubleLinkedListQueue.h"

queue_t queue;

int main(void) {
    queue_init(&queue);

    char one[100] = "Test";

    char two[100] = "Test2";

    char three[100] = "Test3";

    enqueue(queue, one);

    print_queue(queue);

    enqueue(queue, two);

    print_queue(queue);

    enqueue(queue, three);

    print_queue(queue);

    char *result = dequeue(queue);

    print_queue(queue);

    printf("%s\n", result);

    result = dequeue(queue);

    print_queue(queue);

    printf("%s\n", result);

    result = dequeue(queue);

    print_queue(queue);

    printf("%s\n", result);
}
