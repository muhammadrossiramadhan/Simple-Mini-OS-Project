#include <stdio.h>
#include <time.h>

long long fib(int n) { if (n<=1) return n; return fib(n-1)+fib(n-2); }

int main() {
    clock_t start = clock();
    int n = fib(6); // Samakan angka dengan input di MiniOS
    printf("%d\n", n);
    printf("Time: %f ms\n", (double)(clock()-start)/CLOCKS_PER_SEC*1000);
    return 0;
}