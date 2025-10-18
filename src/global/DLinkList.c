#include <global.h>  // For the macros

CDLLNode* initList(GenericData data, GenericDataType dataType) {
    CDLLNode *head = calloc(1, sizeof(CDLLNode));
    if (!head) THROW_ERROR_AND_EXIT("Failed to allocate memory for list head");
    head->data = data;
    head->dataType = dataType;
    head->next = head;
    head->prev = head;
    return head;
}

void CDLLInsertLast(CDLLNode *head, GenericData data, GenericDataType dataType) {
    if (!head) THROW_ERROR_AND_RETURN_VOID("Cannot insert into list: head is NULL");

    CDLLNode *newNode = calloc(1, sizeof(CDLLNode));
    if (!newNode) THROW_ERROR_AND_EXIT("Failed to allocate memory for new list node");
    newNode->data = data;
    newNode->dataType = dataType;

    // Insert newNode before the head (at the end of the list)
    newNode->prev = head->prev;
    newNode->next = head;
    head->prev->next = newNode;
    head->prev = newNode;
}

void CDLLRemoveLast(CDLLNode *head) {
    if (!head) THROW_ERROR_AND_RETURN_VOID("Cannot remove from list: head is NULL");

    CDLLNode *lastNode = head->prev;
    if (lastNode == head) {
        freeList(&head);
        return; // List is now empty
    }
    lastNode->prev->next = head;
    head->prev = lastNode->prev;

    free(lastNode);
}

void freeList(CDLLNode **head) {
    if (!head || !*head) return;

    CDLLNode *current = (*head)->next;
    while (current != *head) {
        CDLLNode *next = current->next;
        free(current);
        current = next;
    }
    free(*head);
    *head = NULL;  // The double pointer was to prevent this dangling pointer
}
