#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <fstream>
#include <vector>
#include "Track.h"

using namespace std;

// Simple helper for reading Track data from a .txt file.
// Only used to load the fixed, read-only master catalog at startup — per
// the proposal's "No data persistence" limitation, the playlist, queue, and
// history are in-memory only and are never written to disk, so no write
// helper is needed here.
namespace FileManager {

    inline vector<Track> readTracks(const string& path) {
        vector<Track> tracks;
        ifstream file(path);
        if (!file.is_open()) return tracks;  // file may not exist yet, that's ok

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            Track t = Track::fromLine(line);
            if (t.id != -1) tracks.push_back(t);
        }
        file.close();
        return tracks;
    }
}

#endif
