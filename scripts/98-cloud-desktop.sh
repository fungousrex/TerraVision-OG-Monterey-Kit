#!/usr/bin/env bash
set -euo pipefail

# Avvia un desktop X11 virtuale e lo espone via noVNC sul port-forward Codespaces.
# Il server VNC resta in ascolto solo su localhost; Codespaces inoltra la porta 6080.

LOGDIR="$HOME/.tv-cloud"
mkdir -p "$LOGDIR"
export DISPLAY=:0
export LIBGL_ALWAYS_SOFTWARE=1

if ! pgrep -x Xvfb >/dev/null 2>&1; then
  Xvfb :0 -screen 0 1600x1000x24 +extension GLX +render -noreset \
    >"$LOGDIR/xvfb.log" 2>&1 &
  echo $! > "$LOGDIR/xvfb.pid"
  for _ in $(seq 1 30); do
    if xdpyinfo -display :0 >/dev/null 2>&1; then break; fi
    sleep 1
  done
fi
xdpyinfo -display :0 >/dev/null 2>&1 || {
  echo "Xvfb non è partito. Log: $LOGDIR/xvfb.log" >&2; exit 1;
}

if ! pgrep -f 'x11vnc.*rfbport 5900' >/dev/null 2>&1; then
  x11vnc -display :0 -forever -shared -nopw -localhost -rfbport 5900 \
    >"$LOGDIR/x11vnc.log" 2>&1 &
  echo $! > "$LOGDIR/x11vnc.pid"
fi

if ! pgrep -f 'websockify.*6080' >/dev/null 2>&1; then
  websockify --web=/usr/share/novnc 0.0.0.0:6080 localhost:5900 \
    >"$LOGDIR/websockify.log" 2>&1 &
  echo $! > "$LOGDIR/websockify.pid"
fi

if ! pgrep -u "$(id -u)" -f 'xfce4-session' >/dev/null 2>&1; then
  nohup dbus-launch --exit-with-session startxfce4 \
    >"$LOGDIR/xfce.log" 2>&1 </dev/null &
  echo $! > "$LOGDIR/xfce.pid"
fi

sleep 2
cat <<'MSG'
Desktop TerraVision pronto.
In Codespaces apri la scheda PORTS, individua 6080 e seleziona Open in Browser.
URL noVNC: /vnc.html?autoconnect=true&resize=remote
Se il port-forward è privato, apri l'URL autenticato generato da Codespaces.
MSG
