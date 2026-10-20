"""Generate the morph wavetable of the C-MORPH PLUG-OUT (2026-10-20).

Six frames of a WAV (K = 6, N = 72 points: 6 x 73 = 438 words = bank 5 0x048..0x1FD, see re/wt5.py) go into a table
the engine writes into bank 5 of DSP2/DSP3 when the plugout starts (engine.c -DMORPH). One DSP program (wt5's player,
--scan) reads the table with a triangle LFO: frame 0 -> frame 5 -> frame 0, one period every 1/SCAN_HZ seconds.

    python3 make.py [WAV]     # writes build/morph_table.h (for the engine) and prog/morph_dco1.txt (DCO1 program)
"""
import os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
RE = os.path.join(HERE, '..', 'from_scratch', 're')
sys.path.insert(0, RE)
import wt5          # noqa: E402
import dspasm as A  # noqa: E402

WAV = os.environ.get('MORPH_WAV', '/home/emilio/PRODUCCION/samples/THE_REST/wavetables/Echo Sound Works Core Tables/'
                     'Analog/80\'s PWM.wav')
K, N = 6, 72
SCAN_HZ = float(os.environ.get('MORPH_HZ', '0.25'))
ENGINE_BUILD = os.path.join(HERE, '..', 'from_scratch', 'engine', 'build')


def main():
    wav = sys.argv[1] if len(sys.argv) > 1 else WAV
    frames, picks, F, fs = wt5.convert(wav, K, N)
    table = wt5.words_of_table(frames)
    assert len(table) == K * (N + 1) == wt5.LIMIT - wt5.BASE, len(table)
    ints = [max(-(1 << 17) + 1, min((1 << 17) - 1, round(v * (1 << 17)))) for v in table]
    os.makedirs(ENGINE_BUILD, exist_ok=True)
    with open(os.path.join(ENGINE_BUILD, 'morph_table.h'), 'w') as f:
        f.write(f'/* {os.path.basename(wav)}: frames {picks} of {F}, {N} points each, 18-bit (value * 2^17) */\n')
        f.write(f'#define MORPH_BASE {wt5.BASE:#x}\n#define MORPH_WORDS {len(ints)}\n')
        f.write('static const int32_t morph_tab[MORPH_WORDS] = {\n')
        for i in range(0, len(ints), 8):
            f.write('    ' + ', '.join(str(v) for v in ints[i:i + 8]) + ',\n')
        f.write('};\n')
    P = wt5.program(table, N, K, 0.0, SCAN_HZ, slot='dco1')
    os.makedirs(os.path.join(HERE, 'prog'), exist_ok=True)
    out = os.path.join(HERE, 'prog', 'morph_dco1.txt')
    open(out, 'w').write(A.text(P, f'morph {os.path.basename(wav)} K={K} N={N} scan={SCAN_HZ} Hz frames {picks}'))
    print(f'frames {picks} of {F}; table {len(ints)} words; program {len(P)} instructions -> {out}')


if __name__ == '__main__':
    main()
