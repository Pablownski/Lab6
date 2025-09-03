#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <atomic>
#include <chrono>


struct Args {
    long iters;
    long* global;
    pthread_mutex_t* mtx;
};


void* worker_naive(void* p) {
    auto* a = static_cast<Args*>(p);
    for (long i = 0; i < a->iters; i++) {
        (*a->global)++; // RACE
    }
    return nullptr;
}


void* worker_mutex(void* p) {
    auto* a = static_cast<Args*>(p);
    for (long i = 0; i < a->iters; i++) {
        pthread_mutex_lock(a->mtx);
        (*a->global)++;
        pthread_mutex_unlock(a->mtx);
    }
    return nullptr;
}


void* worker_sharded(void* p) {
    long* local = static_cast<long*>(p);
    long iters = *local;
    long* result = new long(0);
    for (long i = 0; i < iters; i++) (*result)++;
    return result; 
}

void* worker_atomic(void* p) {
    auto* a = static_cast<std::pair<long, std::atomic<long>*>*>(p);
    for (long i = 0; i < a->first; i++) {
        (*a->second)++;
    }
    return nullptr;
}

int main(int argc, char** argv) {
    int T = (argc > 1) ? std::atoi(argv[1]) : 4;
    long it = (argc > 2) ? std::atol(argv[2]) : 1000000;


    {
        long global = 0;
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
        std::vector<pthread_t> th(T);
        Args a{it, &global, &mtx};

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < T; i++) pthread_create(&th[i], nullptr, worker_naive, &a);
        for (int i = 0; i < T; i++) pthread_join(th[i], nullptr);
        auto end = std::chrono::high_resolution_clock::now();

        printf("NAIVE total=%ld (esperado=%ld), tiempo=%.6f s\n",
               global, (long)T * it,
               std::chrono::duration<double>(end - start).count());
    }

  
    {
        long global = 0;
        pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
        std::vector<pthread_t> th(T);
        Args a{it, &global, &mtx};

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < T; i++) pthread_create(&th[i], nullptr, worker_mutex, &a);
        for (int i = 0; i < T; i++) pthread_join(th[i], nullptr);
        auto end = std::chrono::high_resolution_clock::now();

        printf("MUTEX total=%ld (esperado=%ld), tiempo=%.6f s\n",
               global, (long)T * it,
               std::chrono::duration<double>(end - start).count());
    }

    {
        std::vector<pthread_t> th(T);
        std::vector<long> args(T, it);

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < T; i++) pthread_create(&th[i], nullptr, worker_sharded, &args[i]);

        long global = 0;
        for (int i = 0; i < T; i++) {
            void* ret;
            pthread_join(th[i], &ret);
            global += *(long*)ret;
            delete (long*)ret;
        }
        auto end = std::chrono::high_resolution_clock::now();

        printf("SHARDED total=%ld (esperado=%ld), tiempo=%.6f s\n",
               global, (long)T * it,
               std::chrono::duration<double>(end - start).count());
    }

    
    {
        std::atomic<long> global{0};
        std::vector<pthread_t> th(T);
        std::pair<long, std::atomic<long>*> arg{it, &global};

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < T; i++) pthread_create(&th[i], nullptr, worker_atomic, &arg);
        for (int i = 0; i < T; i++) pthread_join(th[i], nullptr);
        auto end = std::chrono::high_resolution_clock::now();

        printf("ATOMIC total=%ld (esperado=%ld), tiempo=%.6f s\n",
               global.load(), (long)T * it,
               std::chrono::duration<double>(end - start).count());
    }

    return 0;
}
