#include "httplib.h"
#include "Track.h"
#include "Catalog.h"
#include "Playlist.h"
#include "SongQueue.h"
#include "HistoryStack.h"
#include "JsonHelper.h"
#include <iostream>

using namespace std;
using namespace httplib;

// Global data structures - one instance of each, shared across all requests.
// (This is a single-session app per the proposal's limitations, so no
// per-user state is needed.)
Catalog catalog;
Playlist playlist;
SongQueue songQueue;
HistoryStack history;
Track nowPlaying;
bool hasNowPlaying = false;

const string DATA_DIR = "data/";

void setJson(Response& res, const string& body) {
    res.set_content(body, "application/json");
}

void setError(Response& res, int code, const string& message) {
    res.status = code;
    setJson(res, "{\"error\":\"" + Track::escape(message) + "\"}");
}

int main() {
    Server svr;

    // ---- Load only the fixed master catalog from disk on startup ----
    // Per the proposal's limitations (4.0 "No data persistence" / "No Playlist
    // Persistence"), the playlist, queue, and history are session-only and
    // live purely in memory below. They are intentionally never read from or
    // written to disk, so they reset every time the server restarts.
    catalog.loadFromFile(DATA_DIR + "catalog.txt");

    cout << "Loaded " << catalog.size() << " tracks into the catalog.\n";

    // Serve the frontend (index.html, style.css, script.js) as static files
    svr.set_mount_point("/", "../frontend");

    // ---------------- CATALOG ----------------
    svr.Get("/api/catalog", [](const Request& req, Response& res) {
        vector<Track> result;
        if (req.has_param("generation")) {
            int gen = stoi(req.get_param_value("generation"));
            result = catalog.filterByGeneration(gen);
        } else if (req.has_param("game")) {
            result = catalog.filterByGame(req.get_param_value("game"));
        } else {
            result = catalog.getAll();
        }
        setJson(res, tracksToJsonArray(result));
    });

    // ---------------- PLAYLIST (doubly linked list) ----------------
    svr.Get("/api/playlist", [](const Request&, Response& res) {
        setJson(res, tracksToJsonArray(playlist.toVector()));
    });

    svr.Post("/api/playlist/add", [](const Request& req, Response& res) {
        int trackId = JsonHelper::getInt(req.body, "trackId");
        string position = JsonHelper::getString(req.body, "position", "back");

        Track t;
        if (!catalog.findById(trackId, t)) {
            setError(res, 404, "Track not found in catalog");
            return;
        }

        if (position == "front") playlist.addFront(t);
        else playlist.addBack(t);

        setJson(res, tracksToJsonArray(playlist.toVector()));
    });

    svr.Post("/api/playlist/remove", [](const Request& req, Response& res) {
        string title = JsonHelper::getString(req.body, "title");
        bool removed = playlist.removeByTitle(title);
        setJson(res, string("{\"removed\":") + (removed ? "true" : "false") + "}");
    });

    svr.Post("/api/playlist/clear", [](const Request&, Response& res) {
        playlist.clear();
        setJson(res, "{\"cleared\":true}");
    });

    svr.Post("/api/playlist/next", [](const Request&, Response& res) {
        Track t;
        if (!playlist.next(t)) {
            setError(res, 400, "No next song in playlist");
            return;
        }
        setJson(res, t.toJson());
    });

    svr.Post("/api/playlist/previous", [](const Request&, Response& res) {
        Track t;
        if (!playlist.previous(t)) {
            setError(res, 400, "No previous song in playlist");
            return;
        }
        setJson(res, t.toJson());
    });

    svr.Get("/api/playlist/current", [](const Request&, Response& res) {
        Track t;
        if (!playlist.getCurrent(t)) {
            setError(res, 404, "Playlist is empty");
            return;
        }
        setJson(res, t.toJson());
    });

    // ---------------- SONG REQUEST QUEUE (FIFO) ----------------
    svr.Get("/api/queue", [](const Request&, Response& res) {
        setJson(res, tracksToJsonArray(songQueue.toVector()));
    });

    svr.Post("/api/queue/enqueue", [](const Request& req, Response& res) {
        int trackId = JsonHelper::getInt(req.body, "trackId");
        Track t;
        if (!catalog.findById(trackId, t)) {
            setError(res, 404, "Track not found in catalog");
            return;
        }
        songQueue.enqueue(t);
        setJson(res, tracksToJsonArray(songQueue.toVector()));
    });

    // Dequeues the front of the queue, makes it the now-playing track,
    // and pushes it onto the playback history stack.
    svr.Post("/api/queue/play", [](const Request&, Response& res) {
        Track t;
        if (!songQueue.dequeue(t)) {
            setError(res, 400, "Queue is empty");
            return;
        }
        nowPlaying = t;
        hasNowPlaying = true;
        history.push(t);

        setJson(res, t.toJson());
    });

    // ---------------- PLAYBACK HISTORY (array-based stack) ----------------
    svr.Get("/api/history", [](const Request&, Response& res) {
        setJson(res, tracksToJsonArray(history.toVector()));
    });

    // Pops the most recent history entry and replays it as now-playing,
    // i.e. "undo a skip"
    svr.Post("/api/history/undo", [](const Request&, Response& res) {
        Track t;
        if (!history.pop(t)) {
            setError(res, 400, "History is empty");
            return;
        }
        nowPlaying = t;
        hasNowPlaying = true;
        setJson(res, t.toJson());
    });

    // ---------------- NOW PLAYING ----------------
    svr.Get("/api/now-playing", [](const Request&, Response& res) {
        if (!hasNowPlaying) {
            setError(res, 404, "Nothing is playing");
            return;
        }
        setJson(res, nowPlaying.toJson());
    });

    cout << "PokePad server running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}
