#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <sstream>
#include <vector>
#include <cstdio>

using namespace std;

struct Track {
    int id;
    string title;
    string game;
    int generation;
    string composer;
    string duration;  
    string filename;  

    Track() : id(-1), generation(0) {}

    Track(int id, string title, string game, int generation,
          string composer, string duration, string filename)
        : id(id), title(title), game(game), generation(generation),
          composer(composer), duration(duration), filename(filename) {}

    string toLine() const {
        stringstream ss;
        ss << id << "|" << title << "|" << game << "|" << generation << "|"
           << composer << "|" << duration << "|" << filename;
        return ss.str();
    }

    string toJson() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"title\":\"" << escape(title) << "\","
           << "\"game\":\"" << escape(game) << "\","
           << "\"generation\":" << generation << ","
           << "\"composer\":\"" << escape(composer) << "\","
           << "\"duration\":\"" << escape(duration) << "\","
           << "\"filename\":\"" << escape(filename) << "\""
           << "}";
        return ss.str();
    }

    static string escape(const string& s) {
        string out;
        for (unsigned char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if (c < 0x20) {
                        char buf[7];
                        snprintf(buf, sizeof(buf), "\\u%04x", c);
                        out += buf;
                    } else {
                        out += c;
                    }
            }
        }
        return out;
    }

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
