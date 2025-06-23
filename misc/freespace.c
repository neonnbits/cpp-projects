#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
typedef struct {
    int size;
    struct node *next;
} node;

int main() {
    node *head =
        mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    head->size = 4096 - sizeof(node);
    head->next = NULL;
}