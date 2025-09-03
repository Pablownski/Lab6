#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <chrono>

constexpr int n = 100000;   
constexpr int STAGES = 3;   

pthread_barrier_t barrier;
pthread_once_t once_control = PTHREAD_ONCE_INIT;

void init_barrier() {
    pthread_barrier_init(&barrier, nullptr, STAGES);
}


void* stage1(void*) {
    pthread_once(&once_control, init_barrier);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        
        if (i % (n/10) == 0) printf("Stage1 generó item %d\n", i);
    }
    pthread_barrier_wait(&barrier);
    auto end = std::chrono::high_resolution_clock::now();

    double t = std::chrono::duration<double>(end-start).count();
    printf("Stage1 listo en %.6f s\n", t);
    return nullptr;
}


void* stage2(void*) {
    pthread_once(&once_control, init_barrier);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
    
        if (i % (n/10) == 0) printf("Stage2 procesó item %d\n", i);
    }
    pthread_barrier_wait(&barrier);
    auto end = std::chrono::high_resolution_clock::now();

    double t = std::chrono::duration<double>(end-start).count();
    printf("Stage2 listo en %.6f s\n", t);
    return nullptr;
}


void* stage3(void*) {
    pthread_once(&once_control, init_barrier);

    auto start = std::chrono::high_resolution_clock::now();
    long acc = 0;
    for (int i = 0; i < n; i++) {
        acc += i; // simular reduce
        if (i % ( n/10) == 0) printf("Stage3 acumuló hasta %d\n", i);
    }
    pthread_barrier_wait(&barrier);
    auto end = std::chrono::high_resolution_clock::now();

    double t = std::chrono::duration<double>(end-start).count();
    printf("Stage3 listo en %.6f s, resultado=%ld\n", t, acc);
    return nullptr;
}


int main(int argc, char** argv) {
    int n = (argc > 1) ? std::atoi(argv[1]) : n;

    pthread_t s1, s2, s3;

    pthread_create(&s1, nullptr, stage1, nullptr);
    pthread_create(&s2, nullptr, stage2, nullptr);
    pthread_create(&s3, nullptr, stage3, nullptr);

    pthread_join(s1, nullptr);
    pthread_join(s2, nullptr);
    pthread_join(s3, nullptr);

    pthread_barrier_destroy(&barrier);

    return 0;
}
