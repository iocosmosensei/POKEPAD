#ifndef CATALOG_H
#define CATALOG_H

#include <vector>
#include <string>
#include "Track.h"
#include "Database.h"

using namespace std;

const int MAX_CATALOG_SIZE = 400;

// Master music catalog - a fixed-size array of all Pokemon OST tracks.
// This is read-only from the user's point of view (see proposal section 4.0).
// The array itself still does the actual storage/search work - the database
// is just where it's loaded from at startup.
class Catalog {
private:
    Track tracks[MAX_CATALOG_SIZE];
    int count;

public:
    Catalog() : count(0) {}

    void loadFromDatabase(Database& db) {
        vector<Track> loaded = db.loadCatalog();
        count = 0;
        for (size_t i = 0; i < loaded.size() && count < MAX_CATALOG_SIZE; i++) {
            tracks[count++] = loaded[i];
        }
    }

    int size() const {
        return count;
    }

    vector<Track> getAll() const {
        vector<Track> result;
        for (int i = 0; i < count; i++) {
            result.push_back(tracks[i]);
        }
        return result;
    }

    vector<Track> filterByGeneration(int gen) const {
        vector<Track> result;
        for (int i = 0; i < count; i++) {
            if (tracks[i].generation == gen) result.push_back(tracks[i]);
        }
        return result;
    }

    vector<Track> filterByGame(const string& game) const {
        vector<Track> result;
        for (int i = 0; i < count; i++) {
            if (tracks[i].game.find(game) != string::npos) result.push_back(tracks[i]);
        }
        return result;
    }

    // Linear search through the array by id
    bool findById(int id, Track& outTrack) const {
        for (int i = 0; i < count; i++) {
            if (tracks[i].id == id) {
                outTrack = tracks[i];
                return true;
            }
        }
        return false;
    }
};

#endif
