"""Live peek / poke of master memory (incl. the master's DSPs) through our region 0 (model.c -DPEEK).

    python3 dsppeek.py ping
    python3 dsppeek.py read 0x4188EFDC 16            # words (hex + DSP RAW value)
    python3 dsppeek.py write 0x41804400 0x30000000 ...

Built into the DSPLIVE plugin (dsplive.py install). Transport: pitch bend = 14 data bits, then
CC1 = seq<<6 | 3<<4 | cmd on channel 1; region 0 runs the command on the OS CPU (master BMC
core 0) and answers reads / pings with SysEx F0 7D 02|03 ... F7 on SYSTEM-8 MIDI 1.
The slaves never see these CC1s (region 0 does not forward them), so the slave DSPs cannot be
read this way: only master memory (DSP n of the master: program 0x41480000 + n*0x100000,
bank 0 RAW view 0x41404000 + n*0x100000).
"""
import argparse, os, select, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspnum   # noqa: E402

DELAY = 0.004


def raw_value(w):
    """DSP RAW word -> float (NOTES.md: sign-magnitude, value = +-M / 2^(32 - 4 s))"""
    s = (w >> 29) & 3
    v = (w & 0x1FFFFFFF) / 2 ** (32 - 4 * s)
    return -v if w >> 31 else v


class Peek:
    def __init__(self, delay=DELAY):
        dev = None
        for line in open('/proc/asound/cards'):
            w = line.split()
            if len(w) > 1 and w[1] == '[SYSTEM8':
                dev = f'/dev/snd/midiC{w[0]}D0'
        if dev is None:
            raise SystemExit('SYSTEM-8 not found')
        self.fd = os.open(dev, os.O_RDWR | os.O_NONBLOCK)
        self.delay, self.seq, self.rx = delay, 0, b''
        self.drain()

    def drain(self):
        end = time.time() + 0.2                      # the port may carry clock / active sensing
        while time.time() < end and select.select([self.fd], [], [], 0.05)[0]:
            try:
                os.read(self.fd, 4096)
            except BlockingIOError:
                break

    def cmd(self, cmd, p=0):
        assert 0 <= p < 0x4000
        self.seq ^= 1
        os.write(self.fd, bytes([0xE0, p & 0x7F, p >> 7, 0xB0, 1, self.seq << 6 | 0x30 | cmd]))
        time.sleep(self.delay)

    def reply(self, kind, timeout=1.0):
        end = time.time() + timeout
        while time.time() < end:
            if select.select([self.fd], [], [], 0.05)[0]:
                try:
                    self.rx += os.read(self.fd, 4096)
                except BlockingIOError:
                    pass
            i = self.rx.find(bytes([0xF0, 0x7D, kind]))
            j = self.rx.find(b'\xf7', i + 1) if i >= 0 else -1
            if j > 0:
                body, self.rx = self.rx[i + 3:j], self.rx[j + 1:]
                return body
        raise TimeoutError(f'no reply F0 7D {kind:02x}')

    def ping(self, p=0x1234):
        self.cmd(6, p)
        body = self.reply(3)
        return int(''.join(f'{x:x}' for x in body), 16)

    def address(self, a):
        assert 0x40000000 <= a < 0x80000000 and a % 4 == 0
        w = (a - 0x40000000) >> 2
        self.cmd(0, w & 0x3FFF)
        self.cmd(1, w >> 14)

    def read(self, a, n):
        out = []
        while n > 0:
            k = min(n, 16)
            self.address(a)
            self.cmd(5, k)
            body = self.reply(2)
            nib = ''.join(f'{x:x}' for x in body)
            got = int(nib[:8], 16)
            if got != a:
                raise IOError(f'reply for {got:#x}, expected {a:#x}')
            out += [int(nib[8 + 8 * i:16 + 8 * i], 16) for i in range(k)]
            a += 4 * k
            n -= k
        return out

    def write(self, a, words):
        self.address(a)
        for v in words:
            self.cmd(2, v & 0x3FFF)
            self.cmd(3, (v >> 14) & 0x3FFF)
            self.cmd(4, v >> 28)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['ping', 'read', 'write'])
    ap.add_argument('args', nargs='*')
    a = ap.parse_args()
    pk = Peek()
    if a.cmd == 'ping':
        print(f'pong {pk.ping():#06x}')
    elif a.cmd == 'read':
        addr, n = int(a.args[0], 0), int(a.args[1], 0) if len(a.args) > 1 else 1
        for i, w in enumerate(pk.read(addr, n)):
            print(f'{addr + 4 * i:08x}: {w:08x}  {raw_value(w):+.9g}')
    else:
        pk.write(int(a.args[0], 0), [int(x, 0) for x in a.args[1:]])


if __name__ == '__main__':
    main()
