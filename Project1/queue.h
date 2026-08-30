#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float loc[3];
    float rotate;
    float v[3];
} object;
typedef struct objects {
    object* data;
    struct objects* next;
} onode;
typedef struct {
    onode* head, * tail;
} queue;

#ifdef __cplusplus
extern "C" {
#endif
    void initQueue(queue* q);
    int isEmpty(queue* q);
    void enqueue(queue* q, float loc[3], float r, float v[3]);
    object* dequeue(queue* q);
    void freeQueue(queue* q);
#ifdef __cplusplus
}
#endif