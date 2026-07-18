#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <fstream>
#include <vector>
#include "Track.h"

using namespace std;

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
