#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "Track.h"

using namespace std;

class Database {
private:
    MYSQL* conn;

public:
    Database() : conn(nullptr) {}

    ~Database() {
        if (conn) mysql_close(conn);
    }

    bool connect(const string& host, const string& user, const string& password,
                 const string& dbName, unsigned int port = 3306) {
        conn = mysql_init(nullptr);
        if (!conn) return false;

        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                                 dbName.c_str(), port, nullptr, 0)) {
            cerr << "MySQL connection failed: " << mysql_error(conn) << "\n";
            conn = nullptr;
            return false;
        }

        mysql_set_character_set(conn, "utf8mb4");

        return true;
    }

    bool isConnected() const { return conn != nullptr; }

    // Escapes a string for safe use inside a query (prevents SQL injection)
    string escape(const string& s) const {
        vector<char> buf(s.size() * 2 + 1);
        unsigned long len = mysql_real_escape_string(conn, buf.data(), s.c_str(), s.size());
        return string(buf.data(), len);
    }

    bool runUpdate(const string& sql) {
        if (mysql_query(conn, sql.c_str()) != 0) {
            cerr << "MySQL query failed: " << mysql_error(conn) << "\n  SQL: " << sql << "\n";
            return false;
        }
        return true;
    }

    vector<Track> loadCatalog() {
        vector<Track> tracks;
        if (!runUpdate("SELECT id, title, game, generation, composer, duration, filename "
                        "FROM catalog ORDER BY id ASC")) {
            return tracks;
        }
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) return tracks;

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Track t;
            t.id = row[0] ? stoi(row[0]) : -1;
            t.title = row[1] ? row[1] : "";
            t.game = row[2] ? row[2] : "";
            t.generation = row[3] ? stoi(row[3]) : 0;
            t.composer = row[4] ? row[4] : "";
            t.duration = row[5] ? row[5] : "";
            t.filename = row[6] ? row[6] : "";
            tracks.push_back(t);
        }
        mysql_free_result(result);
        return tracks;
    }
};

#endif
