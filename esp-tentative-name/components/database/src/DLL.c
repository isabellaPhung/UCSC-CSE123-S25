#include <stdlib.h>
#include "DLL.h"

/// @brief Create a new doubly linked list
/// @return Returns list or NULL if malloc failed
DoublyLinkedList *create_list()
{
    DoublyLinkedList *list = (DoublyLinkedList *)malloc(sizeof(DoublyLinkedList));
    if (list)
    {
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;
    }
    return list;
}

/// @brief Destroy the doubly linked list
/// @param list Doubly linked list in question
/// @param free_data Function to handle freed data (WILL NOT FREE DATA AUTOMATICALLY)
/// @return 0 on success, 1 if list is invalid
int destroy_list(DoublyLinkedList *list, void (*free_data)(void *))
{
    if (!list)
        return 1;
    Node *current = list->head;
    while (current)
    {
        Node *next = current->next;
        if (free_data)
        {
            free_data(current->data);
        }
        free(current);
        current = next;
    }
    free(list);
    return 0;
}

/// @brief Insert a node at the front of the list
/// @param list List in question
/// @param data Data to insert
/// @return 0 on success, 1 on invalid inputs
int insert_front(DoublyLinkedList *list, void *data)
{
    if (!list)
        return 1;
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node)
        return 1;
    new_node->data = data;
    new_node->next = list->head;
    new_node->prev = NULL;
    if (list->head)
    {
        list->head->prev = new_node;
    }
    else
    {
        list->tail = new_node;
    }
    list->head = new_node;
    list->size++;
    return 0;
}

/// @brief Insert a node at the back of the list
/// @param list The list in question
/// @param data Data to add
/// @return 0 on success, 1 if inputs are invalid
int insert_back(DoublyLinkedList *list, void *data)
{
    if (!list)
        return 1;
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node)
        return 1;
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = list->tail;
    if (list->tail)
    {
        list->tail->next = new_node;
    }
    else
    {
        list->head = new_node;
    }
    list->tail = new_node;
    list->size++;
    return 0;
}

/// @brief Delete a node from the list
/// @param list The list in question
/// @param node The node to remove (use find_node)
/// @param free_data Function to handle freed data (WILL NOT FREE DATA AUTOMATICALLY)
/// @return 
int delete_node(DoublyLinkedList *list, Node *node, void (*free_data)(void *))
{
    if (!list || !node)
        return 1;

    // Close gap
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        list->head = node->next;
    }
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        list->tail = node->prev;
    }

    // Free the data
    if (free_data)
    {
        free_data(node->data);
    }
    free(node);
    list->size--;
    return 0;
}

// Find a node in the list
Node *find_node(DoublyLinkedList *list, void *data, int (*cmp)(void *, void *))
{
    if (!list)
        return NULL;
    Node *current = list->head;
    while (current)
    {
        if (cmp(current->data, data) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Traverse the list forward
int traverse_forward(DoublyLinkedList *list, void (*func)(void *))
{
    if (!list)
        return 1;
    Node *current = list->head;
    while (current)
    {
        func(current->data);
        current = current->next;
    }
    return 0;
}

// Traverse the list backward
int traverse_backward(DoublyLinkedList *list, void (*func)(void *))
{
    if (!list)
        return 1;
    Node *current = list->tail;
    while (current)
    {
        func(current->data);
        current = current->prev;
    }
    return 0;
}