// Fitting an island (a function of one value, sampled by the profiling run; docs/S1CC.md §6).
// Domain: t = ta V + tb in [-1, 1] (V = the root's value); models, cheapest first that meets the tolerance
// (99.5th percentile of |error| / output span; the maximum is reported too):
//   poly   P(t), degree 1..maxDeg (odd P when the domain and the function are symmetric)
//   exp2   sign 2^Q(t), Q of degree 1..4 (exponential curves: increments, cutoffs)
// with the output limited to the plateaus the samples show at either end (saturation: CLIP, table ends).
// Least squares in the Chebyshev basis, converted to powers of t.
#include <algorithm>
#include <cmath>

#include "s1cc.h"

namespace s1 {

namespace {

std::vector<double> solve(std::vector<std::vector<double>> A, std::vector<double> b) {
    int n = (int)b.size();
    for (int c = 0; c < n; c++) {
        int p = c;
        for (int r = c + 1; r < n; r++)
            if (std::fabs(A[r][c]) > std::fabs(A[p][c]))
                p = r;
        std::swap(A[c], A[p]);
        std::swap(b[c], b[p]);
        if (std::fabs(A[c][c]) < 1e-300)
            continue;
        for (int r = c + 1; r < n; r++) {
            double f = A[r][c] / A[c][c];
            for (int k = c; k < n; k++)
                A[r][k] -= f * A[c][k];
            b[r] -= f * b[c];
        }
    }
    std::vector<double> x(n, 0);
    for (int r = n - 1; r >= 0; r--) {
        double s = b[r];
        for (int k = r + 1; k < n; k++)
            s -= A[r][k] * x[k];
        x[r] = std::fabs(A[r][r]) < 1e-300 ? 0 : s / A[r][r];
    }
    return x;
}

// Chebyshev polynomials T_0..T_deg as power-basis coefficient vectors
std::vector<std::vector<double>> chebPow(int deg) {
    std::vector<std::vector<double>> T(deg + 1, std::vector<double>(deg + 1, 0));
    T[0][0] = 1;
    if (deg >= 1)
        T[1][1] = 1;
    for (int k = 2; k <= deg; k++)
        for (int j = 0; j <= deg; j++)
            T[k][j] = (j ? 2 * T[k - 1][j - 1] : 0) - T[k - 2][j];
    return T;
}

// least squares with the Chebyshev polynomials of the given degrees; returns power-basis coefficients (deg+1)
std::vector<double> lsq(const std::vector<double> &t, const std::vector<double> &y, int deg,
                        const std::vector<int> &which) {
    int m = (int)which.size();
    std::vector<std::vector<double>> A(m, std::vector<double>(m, 0));
    std::vector<double> b(m, 0);
    std::vector<double> tk(deg + 1);
    for (size_t i = 0; i < t.size(); i++) {
        tk[0] = 1;
        if (deg >= 1)
            tk[1] = t[i];
        for (int k = 2; k <= deg; k++)
            tk[k] = 2 * t[i] * tk[k - 1] - tk[k - 2];
        for (int a = 0; a < m; a++) {
            b[a] += tk[which[a]] * y[i];
            for (int c = 0; c < m; c++)
                A[a][c] += tk[which[a]] * tk[which[c]];
        }
    }
    std::vector<double> cc = solve(A, b);
    auto T = chebPow(deg);
    std::vector<double> p(deg + 1, 0);
    for (int a = 0; a < m; a++)
        for (int j = 0; j <= deg; j++)
            p[j] += cc[a] * T[which[a]][j];
    return p;
}

double horner(const std::vector<double> &c, double t) {
    double s = 0;
    for (int k = (int)c.size() - 1; k >= 0; k--)
        s = s * t + c[k];
    return s;
}

}  // namespace

Fit fitIsland(const std::vector<std::pair<double, double>> &xy, double tolSpan, double tolLog, int maxDeg) {
    Fit best;
    best.err = 1e300;
    double tol = tolSpan;
    if (xy.empty())
        fail("island without samples (the profiling run never reached it)");
    double xl = 1e300, xh = -1e300, yl = 1e300, yh = -1e300;
    for (auto &p : xy) {
        xl = std::min(xl, p.first);
        xh = std::max(xh, p.first);
        yl = std::min(yl, p.second);
        yh = std::max(yh, p.second);
    }
    double span = yh - yl;
    if (span <= 0 || xh <= xl) {
        best.model = "poly";
        best.c = {xy[0].second};
        best.err = 0;
        best.cost = 0;
        return best;
    }
    double ta = 2 / (xh - xl), tb = -(xh + xl) / (xh - xl);
    bool symmetric = std::fabs(tb) < 0.02;
    if (symmetric)
        tb = 0;
    // plateaus at the ends (saturation)
    std::vector<std::pair<double, double>> s = xy;
    std::sort(s.begin(), s.end());
    double eps = 1e-9 * span;
    size_t nlo = 0, nhi = 0;
    auto endRun = [&](bool fromStart, double level) {
        size_t n = 0;
        if (fromStart) {
            while (n < s.size() && std::fabs(s[n].second - level) <= eps)
                n++;
        } else {
            while (n < s.size() && std::fabs(s[s.size() - 1 - n].second - level) <= eps)
                n++;
        }
        return n;
    };
    // a plateau = >= 2 % of the samples at the extreme value, at one end
    size_t minRun = std::max<size_t>(3, s.size() / 50);
    bool lowAtStart = std::fabs(s.front().second - yl) <= eps, lowAtEnd = std::fabs(s.back().second - yl) <= eps;
    bool highAtStart = std::fabs(s.front().second - yh) <= eps, highAtEnd = std::fabs(s.back().second - yh) <= eps;
    size_t cutStart = 0, cutEnd = 0;
    bool clampLo = false, clampHi = false;
    if (lowAtStart && (nlo = endRun(true, yl)) >= minRun) { clampLo = true; cutStart = nlo - 1; }
    if (highAtEnd && (nhi = endRun(false, yh)) >= minRun) { clampHi = true; cutEnd = nhi - 1; }
    if (highAtStart && !clampLo && (nhi = endRun(true, yh)) >= minRun) { clampHi = true; cutStart = nhi - 1; }
    if (lowAtEnd && !clampHi && (nlo = endRun(false, yl)) >= minRun) { clampLo = true; cutEnd = nlo - 1; }
    std::vector<double> t, y;
    for (size_t i = cutStart; i + cutEnd < s.size(); i++) {
        t.push_back(ta * s[i].first + tb);
        y.push_back(s[i].second);
    }
    if (clampLo || clampHi)
        symmetric = false;
    // positive and over more than 3 octaves (a coefficient, a frequency): judged in octaves, above 1/64 of the
    // maximum (below, the integer steps of a table dominate)
    bool logMetric = yl > 0 && yh / yl > 8;
    best.metric = logMetric ? "log2" : "span";
    if (logMetric)
        tol = tolLog;
    auto evaluate = [&](Fit &f) {
        std::vector<double> es;
        es.reserve(xy.size());
        f.metric = logMetric ? "log2" : "span";
        for (auto &p : xy) {
            double tt = ta * p.first + tb, v;
            if (f.model == "poly")
                v = horner(f.c, tt);
            else
                v = f.sign * std::exp2(horner(f.c, tt));
            if (f.clampLo)
                v = std::max(v, f.lo);
            if (f.clampHi)
                v = std::min(v, f.hi);
            if (!logMetric)
                es.push_back(std::fabs(v - p.second) / span);
            else if (p.second >= yh / 64)
                es.push_back(v > 0 ? std::fabs(std::log2(v / p.second)) : 1e9);
        }
        std::sort(es.begin(), es.end());
        f.errMax = es.back();
        return es[(size_t)(0.995 * (es.size() - 1))];      // 99.5th percentile: a corner of a table does not decide
    };
    int tcost = (ta != 1 ? 1 : 0) + (tb != 0 ? 1 : 0);
    int ccost = (clampLo ? 1 : 0) + (clampHi ? 1 : 0);
    auto consider = [&](Fit f) {
        f.ta = ta;
        f.tb = tb;
        f.clampLo = clampLo;
        f.clampHi = clampHi;
        f.lo = yl;
        f.hi = yh;
        f.err = evaluate(f);
        bool okF = f.err <= tol, okB = best.err <= tol;
        if ((okF && !okB) || (okF && okB && f.cost < best.cost) || (!okF && !okB && f.err < best.err))
            best = f;
    };
    for (int d = 1; d <= maxDeg; d++) {
        std::vector<int> all;
        for (int k = 0; k <= d; k++)
            all.push_back(k);
        Fit f;
        f.model = "poly";
        f.c = lsq(t, y, d, all);
        f.cost = tcost + ccost + d + 1;
        consider(f);
        if (symmetric && (d & 1)) {
            std::vector<int> odd;
            for (int k = 1; k <= d; k += 2)
                odd.push_back(k);
            Fit g;
            g.model = "poly";
            g.c = lsq(t, y, d, odd);
            g.cost = tcost + ccost + 1 + (d - 1) / 2 + 2;   // t^2, Horner in t^2, x t
            consider(g);
        }
        if (best.err <= tol && best.cost <= tcost + ccost + d + 1)
            break;
    }
    bool pos = yl > 0, neg = yh < 0;
    if (pos || neg) {
        std::vector<double> ly;
        for (double v : y)
            ly.push_back(std::log2(std::fabs(v)));
        for (int d = 1; d <= 6; d++) {
            std::vector<int> all;
            for (int k = 0; k <= d; k++)
                all.push_back(k);
            Fit f;
            f.model = "exp2";
            f.sign = pos ? 1 : -1;
            f.c = lsq(t, ly, d, all);
            f.cost = tcost + ccost + (d + 1) + 10;
            consider(f);
        }
    }
    return best;
}

}  // namespace s1
