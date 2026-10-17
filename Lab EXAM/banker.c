#include <stdio.h>
int n, m, alloc[10][10], max[10][10], need[10][10];
int avail[10], work[10], finish[10], seq[10], count = 0;

void search(int depth)
{
    if (depth == n) {
        printf("Safe sequence %d: ", ++count);
        for (int i = 0; i < n; i++) printf("P%d ", seq[i]);
        printf("\n");
        return;
    }
    for (int i = 0; i < n; i++) {
        if (finish[i]) continue;
        int ok = 1;
        for (int j = 0; j < m; j++)
            if (need[i][j] > work[j]) ok = 0;
        if (!ok) continue;
        for (int j = 0; j < m; j++) work[j] += alloc[i][j];
        finish[i] = 1;
        seq[depth] = i;
        search(depth + 1);
        finish[i] = 0;
        for (int j = 0; j < m; j++) work[j] -= alloc[i][j];
    }
}

int main(void)
{
    printf("Enter process count and resource count: ");
    scanf("%d%d", &n, &m);
    printf("Enter allocation matrix:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) scanf("%d", &alloc[i][j]);
    printf("Enter maximum matrix:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) scanf("%d", &max[i][j]);
    printf("Enter available resources:\n");
    for (int j = 0; j < m; j++) scanf("%d", &avail[j]);
    for (int i = 0; i < n; i++) {
        finish[i] = 0;
        for (int j = 0; j < m; j++) need[i][j] = max[i][j] - alloc[i][j];
    }
    for (int j = 0; j < m; j++) work[j] = avail[j];
    search(0);
    if (count) printf("System is safe.\n");
    else printf("System is unsafe; no safe sequence exists.\n");
    return 0;
}
