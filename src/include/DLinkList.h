#ifndef D_LINKED_LIST_H
#define D_LINKED_LIST_H

#include <stdlib.h>
#include <stdio.h>

// Generic (Circular) Doubly Linked List implementation

typedef struct CDLLNode {
    void *data;
    struct CDLLNode *next;
    struct CDLLNode *prev;
} CDLLNode;

/**
 * Initializes a circular doubly linked list with some data
 * @return pointer to the head node
 */
CDLLNode* initList(void *data);

/**
 * Inserts a new node with the given data at the end of the list
 * @param head pointer to the head node of the list
 * @param data pointer to the data to be stored in the new node
 */
void CDLLInsertLast(CDLLNode *head, void *data);

/**
 * Removes the last node from the list
 * @param head pointer to the head node of the list
 */
void CDLLRemoveLast(CDLLNode *head);

/**
 * Frees the memory allocated for the list and its nodes
 * @param head double pointer to the head node of the list
 */
void freeList(CDLLNode **head);

#endif // D_LINKED_LIST_H