#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <string>

using namespace std;

// Very small helper for pulling one field out of a flat JSON object body,
// e.g. body = {"trackId": 5, "position": "front"}
// We don't need a full JSON library since our frontend only ever sends
// simple, flat objects like this.
namespace JsonHelper {

    inline bool findField(const string& body, const string& key, string& rawValue) {
        string needle = "\"" + key + "\"";
        size_t keyPos = body.find(needle);
        if (keyPos == string::npos) return false;

        size_t colonPos = body.find(':', keyPos + needle.size());
        if (colonPos == string::npos) return false;

        size_t i = colonPos + 1;
        while (i < body.size() && (body[i] == ' ' || body[i] == '\t')) i++;

        if (i < body.size() && body[i] == '"') {
            // quoted string value
            size_t start = i + 1;
            size_t end = start;
            string value;
            while (end < body.size() && body[end] != '"') {
                if (body[end] == '\\' && end + 1 < body.size()) end++;  // skip escaped char
                value += body[end];
                end++;
            }
            rawValue = value;
            return true;
        } else {
            // number, bool, or null - read until , or } or whitespace
            size_t end = i;
            while (end < body.size() && body[end] != ',' && body[end] != '}' &&
                   body[end] != ' ' && body[end] != '\n' && body[end] != '\r') {
                end++;
            }
            rawValue = body.substr(i, end - i);
            return true;
        }
    }

    inline int getInt(const string& body, const string& key, int fallback = -1) {
        string raw;
        if (!findField(body, key, raw)) return fallback;
        try {
            return stoi(raw);
        } catch (...) {
            return fallback;
        }
    }

    inline string getString(const string& body, const string& key, string fallback = "") {
        string raw;
        if (!findField(body, key, raw)) return fallback;
        return raw;
    }
}

#endif
