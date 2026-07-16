#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <sstream>
#include <vector>

using namespace std;

// Represents one Pokemon OST entry in the catalog.
struct Track {
    int id;
    string title;
    string game;
    int generation;
    string composer;
    string duration;  // stored as "mm:ss"
    string filename;  // audio file inside frontend/audio/

    Track() : id(-1), generation(0) {}

    Track(int id, string title, string game, int generation,
          string composer, string duration, string filename)
        : id(id), title(title), game(game), generation(generation),
          composer(composer), duration(duration), filename(filename) {}

    // Turns a Track into one pipe-delimited line for saving to a .txt file
    string toLine() const {
        stringstream ss;
        ss << id << "|" << title << "|" << game << "|" << generation << "|"
           << composer << "|" << duration << "|" << filename;
        return ss.str();
    }

    // Turns a Track into a small JSON object (built by hand, no library)
    string toJson() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"title\":\"" << escape(title) << "\","
           << "\"game\":\"" << escape(game) << "\","
           << "\"generation\":" << generation << ","
           << "\"composer\":\"" << escape(composer) << "\","
           << "\"duration\":\"" << duration << "\","
           << "\"filename\":\"" << filename << "\""
           << "}";
        return ss.str();
    }

    static string escape(const string& s) {
        string out;
        for (char c : s) {
            if (c == '"' || c == '\\') out += '\\';
            out += c;
        }
        return out;
    }

    // Splits a line like "1|Route 1|Red and Blue|1|Junichi Masuda|1:32|route1.mp3"
    // back into a Track. Returns a Track with id -1 if the line is malformed.
    static Track fromLine(const string& line) {
        vector<string> parts;
        stringstream ss(line);
        string field;
        while (getline(ss, field, '|')) {
            parts.push_back(field);
        }
        if (parts.size() < 7) return Track();

        Track t;
        t.id = stoi(parts[0]);
        t.title = parts[1];
        t.game = parts[2];
        t.generation = stoi(parts[3]);
        t.composer = parts[4];
        t.duration = parts[5];
        t.filename = parts[6];
        return t;
    }
};

// Turns a vector of Tracks into a JSON array string, e.g. [ {...}, {...} ]
inline string tracksToJsonArray(const vector<Track>& tracks) {
    stringstream ss;
    ss << "[";
    for (size_t i = 0; i < tracks.size(); i++) {
        ss << tracks[i].toJson();
        if (i != tracks.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

#endif
