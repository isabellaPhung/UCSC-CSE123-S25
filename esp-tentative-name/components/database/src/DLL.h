#ifndef DLL_H
#define DLL_H

#include <stddef.h>

// Define the node structure
typedef struct Node
{
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

// Define the doubly linked list structure
typedef struct DoublyLinkedList
{
    Node *head;
    Node *tail;
    size_t size;
} DoublyLinkedList;

// Function prototypes
DoublyLinkedList *create_list();
int destroy_list(DoublyLinkedList *list, void (*free_data)(void *));
int insert_front(DoublyLinkedList *list, void *data);
int insert_back(DoublyLinkedList *list, void *data);
int delete_node(DoublyLinkedList *list, Node *node, void (*free_data)(void *));
Node *find_node(DoublyLinkedList *list, void *data, int (*cmp)(void *, void *));
int traverse_forward(DoublyLinkedList *list, void (*func)(void *));
int traverse_backward(DoublyLinkedList *list, void (*func)(void *));

#endif // DLL_H
