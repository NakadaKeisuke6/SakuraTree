#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
/*
* キュー関連
*/

void initQueue(queue* q) {
    q->head = NULL;
    q->tail = NULL;
}
int isEmpty(queue* q) {
    return (q->head == NULL) && (q->tail == NULL);
}
void enqueue(queue* q, float loc[3], float r, float v[3]) {
    onode* node = (onode*)malloc(sizeof(onode));
    object* obj = (object*)malloc(sizeof(object));
    if (node == NULL || obj == NULL) { // メモリが確保できないとき
        printf("out of memory\n"); exit(1);
    }

    for (int i = 0; i < 3; i++) {
        obj->loc[i] = loc[i];
        obj->v[i] = v[i];
    }
    obj->rotate = r;
    node->data = obj;
    node->next = NULL;

    if (isEmpty(q)) {
        q->head = node;
        q->tail = node;
    }
    else {
        q->tail->next = node;
        q->tail = node;
    }
}
object* dequeue(queue* q) {
    object* result = NULL;
    onode* op;

    if (isEmpty(q)) {
        printf("The queue is empty\n"); return result;
    }

    result = q->head->data;

    if (q->head == q->tail) {
        free(q->head);
        q->head = NULL;
        q->tail = NULL;
    }
    else {
        op = q->head;
        q->head = q->head->next;
        free(op);
    }
    return result;
}
void freeQueue(queue* q) {
    onode* n, * p;
    p = q->head;
    while (p != NULL) {
        n = p->next;
        free(p->data);
        free(p);
        p = n;
    }
    printf("Delete Queue\n");
}