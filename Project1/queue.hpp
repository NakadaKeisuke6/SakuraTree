#include <iostream>

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

class Queue {
private:
    queue* q;
    float lifetime;
    bool (*f)(float, float*, float, float*);

    void initQueue();
public:
    Queue();
    Queue(float lifetime, bool (*isDeletable)(float lifetime, float loc[3], float rotate, float v[3]));
    ~Queue();
    int isEmpty();
    onode* getData();
    float getHeadLocY();
    void autoDequeue();
    void enqueue(float loc[3], float r, float v[3]);
    void dequeue();
    void freeQueue();
};