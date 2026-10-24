"""Port against reference: the numbers port-check prints (docs/MUTABLE_SDK.md §6)."""
import numpy as np

FS = 96000.0


def f0(x, fs=FS):
    """fundamental by rising zero crossings of the signal minus its mean"""
    x = np.asarray(x) - np.mean(x)
    c = np.nonzero((x[:-1] < 0) & (x[1:] >= 0))[0]
    return (len(c) - 1) * fs / (c[-1] - c[0]) if len(c) > 2 else 0.0


def snr_db(port, ref):
    """reference power over error power, sample by sample"""
    p, r = np.asarray(port), np.asarray(ref)
    e = np.mean((p - r) ** 2)
    return float('inf') if e == 0 else 10 * np.log10(np.mean(r ** 2) / e)


def spectrum_db(x, fs=FS):
    x = np.asarray(x) - np.mean(x)
    w = np.hanning(len(x))
    s = np.abs(np.fft.rfft(x * w)) / (np.sum(w) / 2)
    return np.fft.rfftfreq(len(x), 1 / fs), 20 * np.log10(np.maximum(s, 1e-9))


def band_levels(x, fs=FS, edges=(20, 200, 1000, 4000, 12000, 40000)):
    """power in each band (dB), for a spectral comparison that does not depend on the phases"""
    f, s = spectrum_db(x, fs)
    p = 10 ** (s / 10)
    return [10 * np.log10(np.sum(p[(f >= lo) & (f < hi)]) + 1e-18) for lo, hi in zip(edges, edges[1:])]


def report(port, ref, n_sample=None):
    port, ref = np.asarray(port), np.asarray(ref)
    m = n_sample or len(ref)
    bp, br = band_levels(port), band_levels(ref)
    return {'snr_db': snr_db(port[:m], ref[:m]), 'f0_port': f0(port), 'f0_ref': f0(ref),
            'rms_port': float(np.sqrt(np.mean(port ** 2))), 'rms_ref': float(np.sqrt(np.mean(ref ** 2))),
            'peak_port': float(np.max(np.abs(port))), 'peak_ref': float(np.max(np.abs(ref))),
            'bands_db': [round(a - b, 2) for a, b in zip(bp, br)]}
