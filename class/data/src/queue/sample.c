#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <queue.h>

int main(int argc, char **argv)
{
    int data[3] = {100, 200, 300};
    int *tmp = 0;
    queue_t *q = create_queue();

    q->enqueue(q, (void *)&data[0]);
    q->enqueue(q, (void *)&data[2]);
    q->enqueue(q, (void *)&data[1]);
    q->remove(q, (void *)&data[2], NULL);
    q->dequeue(q, (void **)&tmp);
    printf("tmp: %d\n", *(int *)tmp);
    q->dequeue(q, (void **)&tmp);
    printf("tmp: %d\n", *(int *)tmp);

    q->clear(q);
    q->destroy(q);
    return 0;
}
