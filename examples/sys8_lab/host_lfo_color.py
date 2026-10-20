"""Host-side LFO on COLOR: a triangle 0 -> 255 -> 0 sent as DT1 to OSC1 COLOR while a note plays (S8LAB slot 1).

The sweep is written from the PC in one open session (no reconnect per step). The recording is split into
windows and the spectral centroid is printed next to the COLOR value sent in that window.

    python3 host_lfo_color.py [period_s] [steps_per_s]
"""
import sys, threading, time
import numpy as np
import color_test as C
import plugout as P
import plugout_tool as T
import dspaudio as da

PERIOD = float(sys.argv[1]) if len(sys.argv) > 1 else 4.0
RATE = float(sys.argv[2]) if len(sys.argv) > 2 else 20.0


def tri(t):
    ph = (t / PERIOD) % 1.0
    return int(round(255 * (2 * ph if ph < 0.5 else 2 * (1 - ph))))


def main():
    a = C.args()
    s, port = T.open_session(a)
    sent = []
    stop = threading.Event()

    def sweep():
        t0 = time.monotonic()
        while not stop.is_set():
            t = time.monotonic() - t0
            v = tri(t)
            s.send(P.build(P.DT1, C.COLOR, bytes([(v >> 4) & 0xF, v & 0xF]), a.device_id))
            sent.append((t, v))
            time.sleep(1.0 / RATE)

    try:
        s._pump()
        th = threading.Thread(target=sweep); th.start()
        p = da.record('host_lfo', note=45, secs=PERIOD * 2 + 1, on=0.2, hold=PERIOD * 2 + 0.5)
        stop.set(); th.join()
    finally:
        port.close()
    x = np.fromfile(p, dtype='<f4').reshape(-1, 2).mean(1)
    W = int(0.25 * 96000)
    print('window  time   COLOR  centroid(Hz)  level')
    for i in range(len(x) // W):
        seg = x[i * W:(i + 1) * W]
        t = (i + 0.5) * 0.25
        if t < 0.5 or t > PERIOD * 2 - 0.2:
            continue
        S = np.abs(np.fft.rfft(seg * np.hanning(W))); f = np.fft.rfftfreq(W, 1 / 96000)
        cen = float((S * f).sum() / max(S.sum(), 1e-12))
        near = min(sent, key=lambda e: abs(e[0] - t)) if sent else (0, -1)
        print(f'{i:3d}  {t:5.2f}s  {near[1]:5d}  {cen:10.1f}   {float(np.sqrt((seg**2).mean())):.5f}')


if __name__ == '__main__':
    main()
