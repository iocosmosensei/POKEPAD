const API = "http://localhost:8080/api";

let currentTrack = null;   // the track object currently loaded in the audio player
let currentGen = 0;        // 0 = all generations

// ---------- small helpers ----------

// Safely embeds a JSON blob inside a single-quoted HTML attribute
// (e.g. onclick='...'). JSON.stringify only escapes double quotes, not
// apostrophes, so any string value containing one (like a track title —
// "Tarragon's Garage") would otherwise break out of the attribute early
// and corrupt the inline handler. HTML-encoding the apostrophe keeps the
// attribute intact; the browser decodes it back to ' before JS parses it.
function jsonForAttr(obj) {
  return JSON.stringify(obj).replace(/'/g, "&#39;");
}

async function apiGet(path) {
  const res = await fetch(API + path);
  if (!res.ok) return null;
  return res.json();
}

async function apiPost(path, body) {
  const res = await fetch(API + path, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body || {})
  });
  if (!res.ok) return null;
  return res.json();
}

function trackMetaLine(t) {
  return `Gen ${t.generation} · ${t.game} · ${t.duration} · ${t.composer}`;
}

// ---------- CATALOG ----------

async function loadCatalog() {
  const path = currentGen === 0 ? "/catalog" : `/catalog?generation=${currentGen}`;
  const tracks = await apiGet(path);
  renderCatalog(tracks || []);
}

function renderCatalog(tracks) {
  const list = document.getElementById("catalogList");
  document.getElementById("catalogCount").textContent = `${tracks.length} tracks`;

  if (tracks.length === 0) {
    list.innerHTML = `<div class="empty-note">No tracks found for this generation.</div>`;
    return;
  }

  list.innerHTML = tracks.map(t => `
    <div class="track-card">
      <div class="track-info">
        <div class="track-title">${t.title}</div>
        <div class="track-meta">${trackMetaLine(t)}</div>
      </div>
      <div class="track-actions">
        <button class="icon-btn" onclick='playTrackDirectly(${jsonForAttr(t)})'>&#9654;</button>
        <button class="icon-btn" onclick="addToQueue(${t.id})">+Queue</button>
        <button class="icon-btn" onclick="addToPlaylist(${t.id}, 'back')">+Playlist</button>
      </div>
    </div>
  `).join("");
}

// ---------- GENERATION DIAL ----------

document.getElementById("genDial").addEventListener("click", (e) => {
  const btn = e.target.closest(".dial-btn");
  if (!btn) return;
  document.querySelectorAll(".dial-btn").forEach(b => b.classList.remove("active"));
  btn.classList.add("active");
  currentGen = parseInt(btn.dataset.gen, 10);
  loadCatalog();
});

// ---------- TABS ----------

document.getElementById("sideTabs").addEventListener("click", (e) => {
  const btn = e.target.closest(".tab-btn");
  if (!btn) return;
  document.querySelectorAll(".tab-btn").forEach(b => b.classList.remove("active"));
  document.querySelectorAll(".tab-panel").forEach(p => p.classList.remove("active"));
  btn.classList.add("active");
  document.getElementById("tab-" + btn.dataset.tab).classList.add("active");
});

// ---------- PLAYLIST (doubly linked list) ----------

async function addToPlaylist(trackId, position) {
  await apiPost("/playlist/add", { trackId, position });
  loadPlaylist();
}

async function removeFromPlaylist(title) {
  await apiPost("/playlist/remove", { title });
  loadPlaylist();
}

async function loadPlaylist() {
  const tracks = await apiGet("/playlist");
  renderLcdList("playlistList", tracks || [], (t) => `
    <button onclick='playTrackDirectly(${jsonForAttr(t)})'>Play</button>
    <button onclick="removeFromPlaylist('${t.title.replace(/'/g, "\\'")}')">Remove</button>
  `);
}

document.getElementById("playlistClearBtn").addEventListener("click", async () => {
  await apiPost("/playlist/clear");
  loadPlaylist();
});

document.getElementById("playlistNextBtn").addEventListener("click", async () => {
  const t = await apiPost("/playlist/next");
  if (t && !t.error) loadTrack(t, true);
});

document.getElementById("playlistPrevBtn").addEventListener("click", async () => {
  const t = await apiPost("/playlist/previous");
  if (t && !t.error) loadTrack(t, true);
});

// ---------- QUEUE (FIFO) ----------

async function addToQueue(trackId) {
  await apiPost("/queue/enqueue", { trackId });
  loadQueue();
}

async function loadQueue() {
  const tracks = await apiGet("/queue");
  renderLcdList("queueList", tracks || [], () => "");
}

document.getElementById("queuePlayBtn").addEventListener("click", async () => {
  const t = await apiPost("/queue/play");
  if (t && !t.error) {
    loadTrack(t, true);
    loadQueue();
    loadHistory();
  }
});

// ---------- HISTORY (stack) ----------

async function loadHistory() {
  const tracks = await apiGet("/history");
  renderLcdList("historyList", tracks || [], () => "");
}

document.getElementById("historyUndoBtn").addEventListener("click", async () => {
  const t = await apiPost("/history/undo");
  if (t && !t.error) {
    loadTrack(t, true);
    loadHistory();
  }
});

// ---------- shared LCD list renderer ----------

function renderLcdList(elementId, tracks, actionsHtml) {
  const el = document.getElementById(elementId);
  if (tracks.length === 0) {
    el.innerHTML = `<div class="empty-note">Empty.</div>`;
    return;
  }
  el.innerHTML = tracks.map(t => `
    <div class="lcd-row">
      <div>
        <div class="lcd-row-title">${t.title}</div>
        <div>Gen ${t.generation} · ${t.duration}</div>
      </div>
      <div class="lcd-row-actions">${actionsHtml(t)}</div>
    </div>
  `).join("");
}

// ---------- PLAYBACK ----------

const audio = document.getElementById("audioPlayer");

function playTrackDirectly(t) {
  loadTrack(t, true);
}

function loadTrack(t, autoplay) {
  currentTrack = t;
  document.getElementById("transportTitle").textContent = t.title;
  document.getElementById("transportSub").textContent = trackMetaLine(t);
  document.getElementById("nowPlayingInfo").innerHTML = `
    <div class="np-title">${t.title}</div>
    <div class="np-meta">${trackMetaLine(t)}</div>
  `;

  // filenames can contain spaces, &, :, etc. - encode each path segment
  // (gen folder / actual filename) separately so the URL is valid
  audio.src = "audio/" + t.filename.split("/").map(encodeURIComponent).join("/");
  audio.currentTime = 0;

  if (autoplay) {
    audio.play().catch(() => {
      // audio file probably isn't in frontend/audio/ yet - not fatal
      document.getElementById("transportSub").textContent =
        trackMetaLine(t) + "  (audio file not found in /audio)";
    });
  }
  updatePlayPauseIcon();
}

document.getElementById("playPauseBtn").addEventListener("click", () => {
  if (!currentTrack) return;
  if (audio.paused) audio.play().catch(() => {});
  else audio.pause();
  updatePlayPauseIcon();
});

document.getElementById("nextBtn").addEventListener("click", () => {
  document.getElementById("playlistNextBtn").click();
});
document.getElementById("prevBtn").addEventListener("click", () => {
  document.getElementById("playlistPrevBtn").click();
});

function updatePlayPauseIcon() {
  document.getElementById("playPauseBtn").innerHTML = audio.paused ? "&#9654;" : "&#10074;&#10074;";
}

audio.addEventListener("play", updatePlayPauseIcon);
audio.addEventListener("pause", updatePlayPauseIcon);

audio.addEventListener("timeupdate", () => {
  if (!audio.duration) return;
  document.getElementById("seekBar").value = (audio.currentTime / audio.duration) * 100;
  document.getElementById("curTime").textContent = formatTime(audio.currentTime);
  document.getElementById("durTime").textContent = formatTime(audio.duration);
});

document.getElementById("seekBar").addEventListener("input", (e) => {
  if (!audio.duration) return;
  audio.currentTime = (e.target.value / 100) * audio.duration;
});

function formatTime(sec) {
  sec = Math.floor(sec || 0);
  const m = Math.floor(sec / 60);
  const s = sec % 60;
  return `${m}:${s.toString().padStart(2, "0")}`;
}

// ---------- INITIAL LOAD ----------

loadCatalog();
loadPlaylist();
loadQueue();
loadHistory();

apiGet("/now-playing").then(t => {
  if (t && !t.error) loadTrack(t, false);
});
