#ifndef SONG_QUEUE_H
#define SONG_QUEUE_H

#include <vector>
#include <string>
#include "Track.h"

using namespace std;

struct QueueNode {
    Track track;
    QueueNode* next;
    QueueNode(Track t) : track(t), next(nullptr) {}
};

class SongQueue {
private:
    QueueNode* front;
    QueueNode* rear;
    int count;

public:
    SongQueue() : front(nullptr), rear(nullptr), count(0) {}

    ~SongQueue() { clear(); }

    void enqueue(Track t) {
        QueueNode* node = new QueueNode(t);
        if (rear == nullptr) {
            front = rear = node;
        } else {
            rear->next = node;
            rear = node;
        }
        count++;
    }

    bool dequeue(Track& outTrack) {
        if (front == nullptr) return false;
        QueueNode* node = front;
        outTrack = node->track;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        delete node;
        count--;
        return true;
    }

    bool peekFront(Track& outTrack) const {
        if (front == nullptr) return false;
        outTrack = front->track;
        return true;
    }

    bool isEmpty() const { return front == nullptr; }

    int size() const { return count; }

    void clear() {
        while (front != nullptr) {
            QueueNode* next = front->next;
            delete front;
            front = next;
        }
        rear = nullptr;
        count = 0;
    }

    vector<Track> toVector() const {
        vector<Track> result;
        QueueNode* node = front;
        while (node != nullptr) {
            result.push_back(node->track);
            node = node->next;
        }
        return result;
    }
};

#endif
