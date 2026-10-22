# Project Report - Virtual Memory Page Replacement Visualizer

**Team:** TechTitans_OSSP
**Members:** Billakanti Siri (2520030601), Bhavana Kondakrindi (2520030082), T. HimaBindu Sree (2520030484)

## 1. Problem Statement
Build a simulator that visually compares FIFO, LRU and Optimal page replacement
algorithms based on the number of page faults.

## 2. Background
- Virtual memory lets processes use more memory than physical RAM by keeping pages on disk.
- A page fault occurs when the referenced page is not in a physical frame. The OS must load it,
  and if all frames are full it must evict a victim page.
- The replacement algorithm chooses the victim. A better choice means fewer faults.
- A cold-start fault is the first load of a page into an empty frame; it needs no eviction.

## 3. Algorithms
| Algorithm | Victim choice | Implementation | Notes |
|-----------|---------------|----------------|-------|
| FIFO | Page resident the longest | loaded[] timestamp, minimum wins | Simple; suffers Belady's anomaly |
| LRU | Page unused for the longest time | used[] timestamp updated on every hit | Approximates OPT; no anomaly |
| OPT | Page whose next use is farthest in future | next_use() scans rest of string | Needs future knowledge; benchmark only |

## 4. System Design
- Input: frame count and reference string (default, flags, interactive -i, or random).
- Engine: simulate(algo, frames, &Result) runs one algorithm and records, per step, the frame
  snapshot, hit or fault, changed slot and evicted page.
- Visualizer: ANSI-colored terminal output with a step table (changed frame in red, HIT in green),
  a comparison table with hit-rate, a bar chart of faults and a tie-aware summary line.
- Extras: animation delay (-d), Belady's anomaly demo (-b), input validation.

## 5. Implementation
- Language: C (gcc, POSIX getopt and usleep) on Linux, built with make.
- Data: fixed arrays (max 200 references, 10 frames) and one Result struct per algorithm.
- Complexity: FIFO and LRU are O(n*f); OPT is O(n*f*n) in the worst case.

## 6. Testing and Results
Run with: make test (12 tests, all passing).

| Test | Frames | Reference string | FIFO | LRU | OPT |
|------|--------|------------------|------|-----|-----|
| T1 | 3 | 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1 | 15 | 12 | 9 |
| T2 | 3 | 1 2 3 4 1 2 5 1 2 3 4 5 | 9 | 10 | 7 |
| T3 | 4 | 1 2 3 4 1 2 5 1 2 3 4 5 | 10 | 8 | 6 |
| T4 | 2 | 1 1 1 1 1 | 1 | 1 | 1 |
| T5 | 3 | 1 2 3 1 2 3 1 2 3 | 3 | 3 | 3 |
| T6 | 1 | 1 2 3 4 | 4 | 4 | 4 |
| T7 | 1 | 1 1 2 2 3 3 | 3 | 3 | 3 |

Also tested: comma-separated input, OPT <= FIFO and LRU on random runs, and rejection of
invalid frame counts and empty reference strings.

Hit rates for T1: FIFO 25%, LRU 40%, OPT 55%.

Observations:
- OPT always gives the fewest faults.
- LRU beats FIFO on inputs with locality.
- FIFO shows Belady's anomaly: 9 faults with 3 frames but 10 with 4 frames.
- When frames >= distinct pages, no eviction happens and all algorithms tie.

## 7. Impact and Applications
- Teaching tool for OS memory management.
- Helps reason about replacement policy choice (Linux uses LRU approximations).
- Basis for extensions such as Clock, LFU and the working-set model.

## 8. Limitations and Future Work
- Fixed array limits (200 references, 10 frames) and terminal-only visualization.
- Future: Clock (second chance), a web front end, reading real traces.

## 9. Team Contributions
| Member | Role |
|--------|------|
| Billakanti Siri | FIFO + LRU implementation |
| Bhavana Kondakrindi | Optimal algorithm + testing |
| T. HimaBindu Sree | Visualization + documentation |

## 10. References
- Silberschatz, Galvin, Gagne - Operating System Concepts, Virtual Memory chapter.
- Belady, Nelson, Shedler (1969) - an anomaly in space-time characteristics of paging algorithms.
