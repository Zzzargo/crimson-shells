#include "include/DLinkList.h"

CDLLNode* initList(void *data) {
    CDLLNode *head = calloc(1, sizeof(CDLLNode));
    if (!head) {
        fprintf(stderr, "Failed to allocate memory for list head node\n");
        exit(EXIT_FAILURE);
    }
    head->data = data;
    head->next = head;
    head->prev = head;
    return head;
}

void CDLLInsertLast(CDLLNode *head, void *data) {
    if (!head || !data) {
        fprintf(stderr, "Cannot insert into list: head or data is NULL\n");
        return;
    }

    CDLLNode *newNode = calloc(1, sizeof(CDLLNode));
    if (!newNode) {
        fprintf(stderr, "Failed to allocate memory for new list node\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;

    // Insert newNode before the head (at the end of the list)
    newNode->prev = head->prev;
    newNode->next = head;
    head->prev->next = newNode;
    head->prev = newNode;
}

void CDLLRemoveLast(CDLLNode *head) {
    if (!head) {
        fprintf(stderr, "Cannot remove from list: head is NULL\n");
        return;
    }

    CDLLNode *lastNode = head->prev;
    if (lastNode == head) {
        freeList(&head);
        return; // List is now empty
    }
    lastNode->prev->next = head;
    head->prev = lastNode->prev;

    free(lastNode->data);
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
    // Could have freed the data but in my case it's better to do it outside
    free(*head);
    *head = NULL;  // The double pointer was to prevent this dangling pointer
}
