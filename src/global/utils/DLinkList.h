#ifndef D_LINKED_LIST_H
#define D_LINKED_LIST_H

#include <stdlib.h>
#include <stdio.h>

// Generic (Circular) Doubly Linked List implementation

typedef enum {
    DATA_PTR,
    DATA_U64,
    DATA_I64,
    DATA_DOUBLE,
    DATA_FLOAT,
    DATA_U32,
    DATA_I32,
    DATA_U16,
    DATA_I16,
    DATA_BOOLEAN,
    DATA_I8
} GenericDataType;

typedef union {
    void *ptr;  // For bigger data
    Uint64 u64;
    Sint64 i64;
    double_t d;
    float f;
    Uint32 u32;
    Sint32 i32;
    Uint16 u16;
    Sint16 i16;
    Uint8 boolean;
    Sint8 i8;
} GenericData;

typedef struct CDLLNode {
    GenericData data;
    GenericDataType dataType;
    struct CDLLNode *next;
    struct CDLLNode *prev;
} CDLLNode;

/**
 * Initializes a circular doubly linked list with some data
 * @param data generic data to be stored in the head node
 * @param dataType type of the data being stored
 * @return pointer to the head node
 */
CDLLNode* initList(GenericData data, GenericDataType dataType);

/**
 * Inserts a new node with the given data at the end of the list
 * @param head pointer to the head node of the list
 * @param data generic data to be stored in the new node
 */
void CDLLInsertLast(CDLLNode *head, GenericData data, GenericDataType dataType);

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