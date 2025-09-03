#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <chrono>


struct Ring {
    std::vector<int> buf;
    size_t head = 0, tail = 0, count = 0;
    bool stop = false;

    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
    pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

    Ring(size_t Q) : buf(Q) {}
};

void ring_push(Ring* r, int v) {
    pthread_mutex_lock(&r->m);
    while (r->count == r->buf.size() && !r->stop) {
        pthread_cond_wait(&r->not_full, &r->m);
    }
    if (!r->stop) {
        r->buf[r->head] = v;
        r->head = (r->head + 1) % r->buf.size();
        r->count++;
        pthread_cond_signal(&r->not_empty);
    }
    pthread_mutex_unlock(&r->m);
}

bool ring_pop(Ring* r, int* out) {
    pthread_mutex_lock(&r->m);
    while (r->count == 0 && !r->stop) {
        pthread_cond_wait(&r->not_empty, &r->m);
    }
    if (r->count == 0 && r->stop) {
        pthread_mutex_unlock(&r->m);
        return false;
    }
    *out = r->buf[r->tail];
    r->tail = (r->tail + 1) % r->buf.size();
    r->count--;
    pthread_cond_signal(&r->not_full);
    pthread_mutex_unlock(&r->m);
    return true;
}


struct Args {
    Ring* ring;
    int items;
};

void* producer(void* p) {
    auto* a = static_cast<Args*>(p);
    for (int i = 0; i < a->items; i++) {
        ring_push(a->ring, i);
    }
    return nullptr;
}

void* consumer(void* p) {
    auto* a = static_cast<Args*>(p);
    int value;
    int consumed = 0;
    while (ring_pop(a->ring, &value)) {
        consumed++;
    }
    printf("Consumidor terminó: %d items\n", consumed);
    return nullptr;
}

int main(int argc, char** argv) {
    int P = (argc > 1) ? std::atoi(argv[1]) : 2;  // productores
    int C = (argc > 2) ? std::atoi(argv[2]) : 2;  // consumidores
    int Q = (argc > 3) ? std::atoi(argv[3]) : 1024; // tamaño de cola
    int items = (argc > 4) ? std::atoi(argv[4]) : 100000; // items por productor

    Ring ring(Q);
    std::vector<pthread_t> prod(P), cons(C);
    Args a{&ring, items};

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < P; i++) pthread_create(&prod[i], nullptr, producer, &a);
    for (int i = 0; i < C; i++) pthread_create(&cons[i], nullptr, consumer, &a);

    for (int i = 0; i < P; i++) pthread_join(prod[i], nullptr);

    // Señalar a consumidores que no habrá más trabajo
    pthread_mutex_lock(&ring.m);
    ring.stop = true;
    pthread_cond_broadcast(&ring.not_empty);
    pthread_mutex_unlock(&ring.m);

    for (int i = 0; i < C; i++) pthread_join(cons[i], nullptr);

    auto end = std::chrono::high_resolution_clock::now();
    double t = std::chrono::duration<double>(end - start).count();

    long total = (long)P * items;
    printf("P=%d, C=%d, Q=%d, items=%ld -> tiempo=%.6f s, throughput=%.2f ops/s\n",
       P, C, Q, total, t, total / t);


    return 0;
}
