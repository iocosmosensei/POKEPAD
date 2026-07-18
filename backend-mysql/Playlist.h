#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <vector>
#include <string>
#include "Track.h"

using namespace std;

struct PlaylistNode {
    Track track;
    PlaylistNode* prev;
    PlaylistNode* next;

    PlaylistNode(Track t) : track(t), prev(nullptr), next(nullptr) {}
};

class Playlist {
private:
    PlaylistNode* head;
    PlaylistNode* tail;
    PlaylistNode* current;
    int count;

public:
    Playlist() : head(nullptr), tail(nullptr), current(nullptr), count(0) {}

    ~Playlist() { clear(); }

    void addFront(Track t) {
        PlaylistNode* node = new PlaylistNode(t);
        if (head == nullptr) {
            head = tail = current = node;
        } else {
            node->next = head;
            head->prev = node;
            head = node;
        }
        count++;
    }

    void addBack(Track t) {
        PlaylistNode* node = new PlaylistNode(t);
        if (tail == nullptr) {
            head = tail = current = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
        count++;
    }

    bool removeByTitle(const string& title) {
        PlaylistNode* node = head;
        while (node != nullptr) {
            if (node->track.title == title) {
                if (node->prev) node->prev->next = node->next;
                else head = node->next;

                if (node->next) node->next->prev = node->prev;
                else tail = node->prev;

                if (current == node) current = node->next ? node->next : node->prev;

                delete node;
                count--;
                return true;
            }
            node = node->next;
        }
        return false;
    }

    void clear() {
        PlaylistNode* node = head;
        while (node != nullptr) {
            PlaylistNode* next = node->next;
            delete node;
            node = next;
        }
        head = tail = current = nullptr;
        count = 0;
    }

    bool next(Track& outTrack) {
        if (current == nullptr || current->next == nullptr) return false;
        current = current->next;
        outTrack = current->track;
        return true;
    }

    bool previous(Track& outTrack) {
        if (current == nullptr || current->prev == nullptr) return false;
        current = current->prev;
        outTrack = current->track;
        return true;
    }

    bool getCurrent(Track& outTrack) const {
        if (current == nullptr) return false;
        outTrack = current->track;
        return true;
    }

    int size() const { return count; }

    vector<Track> toVector() const {
        vector<Track> result;
        PlaylistNode* node = head;
        while (node != nullptr) {
            result.push_back(node->track);
            node = node->next;
        }
        return result;
    }

};

#endif
