#include "httplib.h"
#include "Track.h"
#include "Database.h"
#include "Catalog.h"
#include "Playlist.h"
#include "SongQueue.h"
#include "HistoryStack.h"
#include "JsonHelper.h"
#include <iostream>

using namespace std;
using namespace httplib;

const string DB_HOST = "localhost";
const string DB_USER = "root";
const string DB_PASSWORD = "";
const string DB_NAME = "pokepad";
const unsigned int DB_PORT = 3306;

Database db;
Catalog catalog;
Playlist playlist;
SongQueue songQueue;
HistoryStack history;
Track nowPlaying;
bool hasNowPlaying = false;

void setJson(Response& res, const string& body) {
    res.set_content(body, "application/json");
}

void setError(Response& res, int code, const string& message) {
    res.status = code;
    setJson(res, "{\"error\":\"" + Track::escape(message) + "\"}");
}

int main() {
    if (!db.connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, DB_PORT)) {
        cerr << "\nCould not connect to MySQL.\n"
             << "Make sure MySQL is running and you've run schema.sql first:\n"
             << "  mysql -u root -p < schema.sql\n"
             << "Then check the DB_HOST/DB_USER/DB_PASSWORD/DB_NAME constants "
                "at the top of main.cpp match your setup.\n\n";
        return 1;
    }

    Server svr;

    catalog.loadFromDatabase(db);

    cout << "Connected to MySQL. Loaded " << catalog.size() << " tracks into the catalog.\n";

    svr.set_mount_point("/", "../frontend");

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

    svr.Get("/api/history", [](const Request&, Response& res) {
        setJson(res, tracksToJsonArray(history.toVector()));
    });

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
