#ifndef HISTORY_STACK_H
#define HISTORY_STACK_H

#include <vector>
#include <string>
#include "Track.h"

using namespace std;

const int MAX_HISTORY = 100;

class HistoryStack {
private:
    Track items[MAX_HISTORY];
    int top;  // index of top element, -1 means empty

public:
    HistoryStack() : top(-1) {}

    bool push(Track t) {
        if (top >= MAX_HISTORY - 1) return false;  // stack full
        items[++top] = t;
        return true;
    }

    bool pop(Track& outTrack) {
        if (isEmpty()) return false;
        outTrack = items[top--];
        return true;
    }

    bool peek(Track& outTrack) const {
        if (isEmpty()) return false;
        outTrack = items[top];
        return true;
    }

    bool isEmpty() const { return top == -1; }

    int size() const { return top + 1; }

    void clear() { top = -1; }

    vector<Track> toVector() const {
        vector<Track> result;
        for (int i = top; i >= 0; i--) {
            result.push_back(items[i]);
        }
        return result;
    }

};

#endif
