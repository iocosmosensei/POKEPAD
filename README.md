# PokePad — Music Player & Playlist Management System
CS104 Data Structures and Algorithms — Group 4

This project ships in **two backend versions** that are functionally identical
and share the same frontend and API:

- **`backend/`** — file handling (`.txt` files), no external database needed.
- **`backend-mysql/`** — the original proposal's MySQL storage.

Pick whichever matches what your professor expects. Both were built and tested
end-to-end.

## How it maps to the proposal

| Feature | Data structure | File (both versions) |
|---|---|---|
| Master music catalog | Array | `Catalog.h` |
| Playlist (next/previous navigation) | Doubly linked list | `Playlist.h` |
| Song request queue | Queue (FIFO, linked list) | `SongQueue.h` |
| Playback history / undo skip | Stack (LIFO, array-based) | `HistoryStack.h` |

The four data structures themselves are **identical** in both versions — only
how the master catalog gets loaded changes. In `backend/`, `FileManager.h`
reads the pipe-delimited `backend/data/catalog.txt` once at startup. In
`backend-mysql/`, `Database.h` runs the equivalent query against a MySQL
database defined in `backend-mysql/schema.sql`. The catalog is read-only
after that, per the proposal's limitations section.

The playlist, queue, and history are **session-only and in-memory**, per the
proposal's "No data persistence" / "No Playlist Persistence" limitations —
they are never written to a file or the database, so they reset every time
the server restarts.

Both versions run the same small local HTTP server (using the single-header
`httplib.h` library) that serves the `frontend/` folder as static files and
exposes a REST-style API under `/api/...` for the JS frontend to call — so the
frontend code doesn't change at all between versions.

## Folder structure

```
PokePad/
├── backend/               file-handling version
│   ├── main.cpp
│   ├── Track.h
│   ├── Catalog.h
│   ├── Playlist.h
│   ├── SongQueue.h
│   ├── HistoryStack.h
│   ├── FileManager.h      generic file read helpers (catalog only)
│   ├── JsonHelper.h
│   ├── httplib.h
│   └── data/
│       └── catalog.txt    seed data (302 Pokemon OST tracks, Gen 1-9)
├── backend-mysql/         MySQL version
│   ├── main.cpp
│   ├── Track.h
│   ├── Catalog.h
│   ├── Playlist.h
│   ├── SongQueue.h
│   ├── HistoryStack.h
│   ├── Database.h         MySQL C API connection + query helpers (catalog only)
│   ├── JsonHelper.h
│   ├── httplib.h
│   └── schema.sql         CREATE TABLE catalog + seed data (same 302 tracks)
└── frontend/               shared by both backends
    ├── index.html
    ├── style.css
    ├── script.js
    └── audio/             put your own .mp3 files here (see below)
```

## Building and running — file-handling version

You need a C++17 compiler (g++ or similar).

```
cd backend
g++ -std=c++17 -o pokepad_server main.cpp -lpthread
./pokepad_server
```

Then open **http://localhost:8080** in your browser. The C++ server serves the
frontend itself, so you don't need a separate web server.

On Windows with MinGW, the command is the same; if `-lpthread` isn't found,
try `g++ -std=c++17 -o pokepad_server.exe main.cpp -static`.

## Building and running — MySQL version

You need MySQL Server running locally and the MySQL C client dev headers.

**1. Install MySQL client dev headers** (needed to compile, separate from the
server itself):
- Ubuntu/Debian: `sudo apt install libmysqlclient-dev`
- Windows: install MySQL Server from mysql.com, which bundles the client
  library and headers (usually under `C:\Program Files\MySQL\MySQL Server
  8.0\include` and `...\lib`)

**2. Load the schema and seed data:**
```
cd backend-mysql
mysql -u root -p --default-character-set=utf8mb4 < schema.sql
```
The `--default-character-set=utf8mb4` part matters — without it, accented
characters like the é in "Pokémon" get corrupted on import even though the
table itself is UTF-8. This creates the `pokepad` database with a single
`catalog` table, seeded with the full 302-track library.

**3. Check the connection settings** at the top of `main.cpp` match your
MySQL setup:
```cpp
const string DB_HOST = "localhost";
const string DB_USER = "root";
const string DB_PASSWORD = "";      // set this if your root user has a password
const string DB_NAME = "pokepad";
const unsigned int DB_PORT = 3306;
```

**4. Compile and run:**
```
g++ -std=c++17 -o pokepad_server main.cpp -lpthread $(mysql_config --cflags --libs)
./pokepad_server
```
On Windows, replace `$(mysql_config --cflags --libs)` with explicit
`-IC:\path\to\mysql\include -LC:\path\to\mysql\lib -llibmysql` flags instead
(`mysql_config` is a Linux/Mac tool), and make sure `libmysql.dll` is next to
your `.exe` or on your PATH.

Then open **http://localhost:8080**, same as the file-handling version.

## Audio files

The catalog is built from your own soundtrack collection — 302 real tracks
across all 9 generations. Each track's `filename` includes its generation
folder and a game subfolder, e.g.
`Generation 1/Pokémon Red-Blue-Yellow/Pokemon BlueRed - Pallet Town.mp3`, so
the player looks for it at
`frontend/audio/Generation 1/Pokémon Red-Blue-Yellow/Pokemon BlueRed - Pallet Town.mp3`.

Drop your `.mp3` files into the matching generation + game folder:
```
frontend/audio/
├── Generation 1/Pokémon Red-Blue-Yellow/
├── Generation 2/Pokémon Gold-Silver & Crystal/
├── Generation 3/Pokémon FireRed-LeafGreen/
├── Generation 3/Pokémon Ruby-Sapphire & Emerald/
├── Generation 4/Pokémon Diamond-Pearl & Platinum/
├── Generation 4/Pokémon HeartGold-SoulSilver/
├── Generation 5/Pokémon Black-White/
├── Generation 5/Pokémon Black 2-White 2/
├── Generation 6/Pokémon X-Y/
├── Generation 6/Pokémon Omega Ruby-Alpha Sapphire/
├── Generation 7/Pokémon Sun-Moon/
├── Generation 7/Pokémon Let_s go Pikachu-Eevee/
├── Generation 8/Pokémon Sword-Shield/
├── Generation 8/Pokémon Legends_ Arceus/
├── Generation 8/Pokémon Brilliant Diamond- Shining Pearl/
├── Generation 9/Pokémon Scarlett-Violet/
└── Generation 9/Pokémon Legends_ Z-A/
```
No renaming needed — just make sure each file lands in the folder for its
generation and game, using its original filename. `script.js` builds the
audio path by splitting `filename` on `/` and encoding each segment, so it
supports any folder depth automatically — no code changes needed if you
reorganize further.

**Note on durations:** the `duration` field in `catalog.txt`/`schema.sql` is
a placeholder estimate for each track (generated per-track so UI elements
have something to display), not the real length of the official recording.
Replace it with the actual runtime once you've dropped in the real `.mp3`
files, if accuracy matters for grading.

3 tracks from the original list were excluded because their names were cut
off with "..." and couldn't be matched reliably (the Sword/Shield trio that
had the same issue — Trainer Encountered (Lass), Trainer Encountered
(Youngster), and Trainer Battle Theme — were later added as ids 300–302 once
their full filenames were confirmed):
- Trainers: Eyes Meet (Youngster) — Ruby/Sapphire/Emerald
- Trainers: Eyes Meet (Lass) — Ruby/Sapphire/Emerald
- Rustboro City, Mauville City & Mossdeep City — Omega Ruby/Alpha Sapphire

If you want these added, send the exact filenames and they can be inserted
the same way as the rest.

**If you're using phpMyAdmin to import `schema.sql`:** on the Import tab,
set "Character set of the file" to `utf-8` before clicking Go, or the
accented characters (Pokémon, é, etc.) will come out corrupted the same way
the command-line import does without `--default-character-set=utf8mb4`.

## API quick reference

- `GET /api/catalog?generation=N` or `?game=NAME` — browse catalog
- `GET/POST /api/playlist`, `/api/playlist/add`, `/remove`, `/clear`, `/next`, `/previous`
- `GET/POST /api/queue`, `/api/queue/enqueue`, `/api/queue/play`
- `GET/POST /api/history`, `/api/history/undo`
- `GET /api/now-playing`
