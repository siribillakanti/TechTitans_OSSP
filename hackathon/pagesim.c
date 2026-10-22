/*
 * pagesim.c - Virtual Memory Page Replacement Visualizer
 * Compares FIFO, LRU and Optimal (Belady's OPT) by page faults.
 *
 * Build : gcc -Wall -Wextra -O2 -o pagesim pagesim.c
 * Usage : ./pagesim [-f frames] [-r "7 0 1 2 ..."] [-n count -m maxpage]
 *                   [-d delay_ms] [-b] [-q]
 *   -f  number of frames (default 3)
 *   -r  reference string, space/comma separated
 *   -n  generate N random references (pages 0..m-1), -m range (default 10)
 *   -d  delay per step in ms for animation (default 0)
 *   -b  Belady's anomaly demo (FIFO, frames 1..7)
 *   -q  quiet: only summary + chart (no step-by-step)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#define MAXN 200
#define MAXF 10

#define RST "\033[0m"
#define RED "\033[1;31m"
#define GRN "\033[1;32m"
#define YEL "\033[1;33m"
#define CYN "\033[1;36m"
#define BLD "\033[1m"

enum { FIFO, LRU, OPT };
static const char *NAME[] = {"FIFO", "LRU", "OPTIMAL"};

typedef struct {
    int faults;
    int hit[MAXN];
    int slot[MAXN];            /* frame index changed, -1 on hit */
    int victim[MAXN];          /* page evicted, -1 if none       */
    int snap[MAXN][MAXF];      /* frame contents after each step */
} Result;

static int n = 0, nf = 3, ref[MAXN];
static int delay_ms = 0, quiet = 0;

static int next_use(int page, int from)
{
    for (int k = from; k < n; k++)
        if (ref[k] == page) return k;
    return INT_MAX;
}

static void simulate(int algo, int frames, Result *r)
{
    int fr[MAXF], loaded[MAXF], used[MAXF];
    for (int j = 0; j < frames; j++) fr[j] = -1, loaded[j] = used[j] = 0;
    r->faults = 0;

    for (int i = 0; i < n; i++) {
        int p = ref[i], f = -1;
        for (int j = 0; j < frames; j++) if (fr[j] == p) { f = j; break; }

        if (f >= 0) {                       /* HIT */
            used[f] = i;
            r->hit[i] = 1; r->slot[i] = -1; r->victim[i] = -1;
        } else {                            /* FAULT */
            int v = -1;
            for (int j = 0; j < frames; j++) if (fr[j] == -1) { v = j; break; }
            if (v < 0) {
                v = 0;
                if (algo == FIFO) {
                    for (int j = 1; j < frames; j++) if (loaded[j] < loaded[v]) v = j;
                } else if (algo == LRU) {
                    for (int j = 1; j < frames; j++) if (used[j] < used[v]) v = j;
                } else {
                    int far = -1;
                    for (int j = 0; j < frames; j++) {
                        int nu = next_use(fr[j], i + 1);
                        if (nu > far) { far = nu; v = j; }
                    }
                }
            }
            r->victim[i] = fr[v];
            fr[v] = p; loaded[v] = used[v] = i;
            r->hit[i] = 0; r->slot[i] = v; r->faults++;
        }
        for (int j = 0; j < frames; j++) r->snap[i][j] = fr[j];
    }
}

static void print_steps(int algo, int frames, const Result *r)
{
    printf("\n" CYN "=== %s (frames = %d) ===" RST "\n", NAME[algo], frames);
    printf(BLD "Step  Page  Frames%*s Result" RST "\n", frames * 4 - 4, "");
    for (int i = 0; i < n; i++) {
        printf("%3d   %3d   ", i + 1, ref[i]);
        for (int j = 0; j < frames; j++) {
            int v = r->snap[i][j];
            if (v < 0) printf("[ -]");
            else if (j == r->slot[i]) printf(RED "[%2d]" RST, v);
            else printf("[%2d]", v);
        }
        if (r->hit[i]) printf("  " GRN "HIT" RST);
        else if (r->victim[i] >= 0) printf("  " RED "FAULT" RST " (evict %d)", r->victim[i]);
        else printf("  " YEL "FAULT" RST " (cold start)");
        printf("\n");
        if (delay_ms > 0) { fflush(stdout); usleep(delay_ms * 1000); }
    }
}

static void bar(int faults, const char *color)
{
    int w = n ? faults * 40 / n : 0;
    if (faults > 0 && w == 0) w = 1;
    printf("%s", color);
    for (int k = 0; k < w; k++) printf("#");
    printf(RST);
}

static void belady(void)
{
    int saved_n = n, saved_ref[MAXN];
    memcpy(saved_ref, ref, sizeof ref);
    int demo[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    n = 12; memcpy(ref, demo, sizeof demo);
    printf(BLD "\nBelady's Anomaly demo - FIFO on 1 2 3 4 1 2 5 1 2 3 4 5" RST "\n");
    printf("Frames  Faults\n");
    for (int f = 1; f <= 7; f++) {
        static Result r;
        simulate(FIFO, f, &r);
        printf("%4d    %3d  ", f, r.faults);
        bar(r.faults, r.faults > 0 ? YEL : GRN);
        printf("%s\n", (f == 4 && r.faults > 9) ? "  <-- more frames, MORE faults!" : "");
    }
    n = saved_n; memcpy(ref, saved_ref, sizeof ref);
}

static void parse_ref(const char *s)
{
    n = 0;
    const char *p = s;
    while (*p && n < MAXN) {
        while (*p && !isdigit((unsigned char)*p)) p++;
        if (!*p) break;
        ref[n++] = (int)strtol(p, (char **)&p, 10);
    }
}

int main(int argc, char **argv)
{
    int opt, gen = 0, maxpage = 10, do_belady = 0;
    int interactive = 0;
    static char line[2048];
    const char *rs = "7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1";

    while ((opt = getopt(argc, argv, "f:r:n:m:d:bqi")) != -1) {
        switch (opt) {
        case 'f': nf = atoi(optarg); break;
        case 'r': rs = optarg; break;
        case 'n': gen = atoi(optarg); break;
        case 'm': maxpage = atoi(optarg); break;
        case 'd': delay_ms = atoi(optarg); break;
        case 'b': do_belady = 1; break;
        case 'q': quiet = 1; break;
        case 'i': interactive = 1; break;
        default:
            fprintf(stderr, "Usage: %s [-f frames] [-r \"refs\"] [-n count -m maxpage] [-d ms] [-b] [-q]\n", argv[0]);
            return 1;
        }
    }
    if (interactive) {
        printf("Enter number of frames: ");
        if (scanf("%d", &nf) != 1) { fprintf(stderr, "invalid frames\n"); return 1; }
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        printf("Enter reference string (e.g. 7 0 1 2 0 3): ");
        if (!fgets(line, sizeof line, stdin)) return 1;
        rs = line;
    }
    if (nf < 1 || nf > MAXF) { fprintf(stderr, "frames must be 1..%d\n", MAXF); return 1; }

    if (gen > 0) {
        if (gen > MAXN) gen = MAXN;
        if (maxpage < 1) maxpage = 10;
        srand((unsigned)time(NULL));
        n = gen;
        for (int i = 0; i < n; i++) ref[i] = rand() % maxpage;
    } else {
        parse_ref(rs);
    }
    if (n == 0) { fprintf(stderr, "empty reference string\n"); return 1; }

    printf(BLD "Virtual Memory Page Replacement Visualizer" RST "\n");
    printf("Frames: %d | References (%d): ", nf, n);
    for (int i = 0; i < n; i++) printf("%d ", ref[i]);
    printf("\n");

    static Result res[3];
    for (int a = 0; a < 3; a++) {
        simulate(a, nf, &res[a]);
        if (!quiet) print_steps(a, nf, &res[a]);
    }

    const char *col[] = {RED, YEL, GRN};
    printf("\n" BLD "=== COMPARISON ===" RST "\n");
    printf("%-8s %7s %6s %9s  Faults\n", "Algo", "Faults", "Hits", "Hit-rate");
    int best = 0;
    for (int a = 0; a < 3; a++) {
        int h = n - res[a].faults;
        printf("%-8s %7d %6d %8.1f%%  ", NAME[a], res[a].faults, h, 100.0 * h / n);
        bar(res[a].faults, col[a]);
        printf("\n");
        if (res[a].faults < res[best].faults) best = a;
    }
    int mn = res[best].faults, cnt = 0;
    for (int a = 0; a < 3; a++) if (res[a].faults == mn) cnt++;
    if (cnt == 3) {
        printf("\nResult: " GRN "All algorithms tied" RST " with %d page faults (no difference for this input).\n", mn);
    } else {
        printf("\nFewest page faults: " GRN);
        int first = 1;
        for (int a = 0; a < 3; a++)
            if (res[a].faults == mn) { printf("%s%s", first ? "" : " / ", NAME[a]); first = 0; }
        printf(RST " (%d)\n", mn);
    }
    printf("Note: OPTIMAL gives the minimum possible faults for any input.\n");

    if (do_belady) belady();
    return 0;
}
