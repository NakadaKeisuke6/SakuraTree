#pragma once
#include "queue.hpp"

/*
* キュー関連
*/

/*
* privateメソッド
*/
void Queue::initQueue() {
    this->q = new queue;
    q->head = NULL;
    q->tail = NULL;
}

/*
* publicメソッド
*/
// コンストラクタ
Queue::Queue() {
    this->initQueue();
    this->lifetime = NULL;
    this->f = NULL;
}

Queue::Queue(float lifetime, bool (*isDeletable)(float lifetime, float loc[3], float rotate, float v[3])) {
    this->initQueue();
    this->lifetime = lifetime;
    this->f = isDeletable;
}

// デストラクタ
Queue::~Queue() {
    this->freeQueue();
    delete this->q;
}

onode* Queue::getData() {
    return this->q->head;
}

float Queue::getHeadLocY() {
    return this->q->head->data->loc[1];
}

// キューが空か
int Queue::isEmpty() {
    return (q->head == NULL) && (q->tail == NULL);
}

void Queue::enqueue(float loc[3], float r, float v[3]) {
    onode* node = new onode;
    object* obj = new object;
    if (node == NULL || obj == NULL) { // メモリが確保できないとき
        std::cerr << "out of memory" << std::endl;
        exit(1);
    }

    for (int i = 0; i < 3; i++) {
        obj->loc[i] = loc[i];
        obj->v[i] = v[i];
    }
    obj->rotate = r;
    node->data = obj;
    node->next = NULL;

    if (this->isEmpty()) {
        q->head = node;
        q->tail = node;
    }
    else {
        q->tail->next = node;
        q->tail = node;
    }
}

void Queue::dequeue() {
    if (this->isEmpty()) {
        std::cerr << "The queue is empty" << std::endl;
        return;
    }

    onode* op;

    if (q->head == q->tail) {
        delete q->head;
        q->head = NULL;
        q->tail = NULL;
    }
    else {
        op = q->head;
        q->head = q->head->next;
        delete op;
    }
    return;
}

void Queue::autoDequeue() {
    if (this->isEmpty()) return;

    if (f(lifetime, q->head->data->loc, q->head->data->rotate, q->head->data->v)) {
        this->dequeue();
    }
}

void Queue::freeQueue() {
    onode* n, * p;

    if (this->isEmpty()) return;

    p = q->head;
    while (p != NULL) {
        n = p->next;
        delete p->data;
        delete p;
        p = n;
    }
    std::cout << "Delete Queue" << std::endl;
}