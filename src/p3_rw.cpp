#include <pthread.h>
#include <vector>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstdlib>

constexpr int NBUCKET = 1024;


struct Node {
    int k, v;
    Node* next;
};


struct Map {
    Node* b[NBUCKET]{};
    pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
};

inline int h(int k) { return k % NBUCKET; }


int map_get_rw(Map* m, int k) {
    pthread_rwlock_rdlock(&m->rw);
    Node* n = m->b[h(k)];
    int v = -1;
    while (n) { if (n->k == k) { v = n->v; break; } n = n->next; }
    pthread_rwlock_unlock(&m->rw);
    return v;
}

void map_put_rw(Map* m, int k, int v) {
    pthread_rwlock_wrlock(&m->rw);
    Node* n = new Node{k,v,m->b[h(k)]};
    m->b[h(k)] = n;
    pthread_rwlock_unlock(&m->rw);
}


int map_get_mtx(Map* m, int k) {
    pthread_mutex_lock(&m->mtx);
    Node* n = m->b[h(k)];
    int v = -1;
    while (n) { if (n->k == k) { v = n->v; break; } n = n->next; }
    pthread_mutex_unlock(&m->mtx);
    return v;
}


void map_put_mtx(Map* m, int k, int v) {
    pthread_mutex_lock(&m->mtx);
    Node* n = new Node{k,v,m->b[h(k)]};
    m->b[h(k)] = n;
    pthread_mutex_unlock(&m->mtx);
}


struct Args {
    Map* m;
    int ops;
    double write_ratio; 
    bool use_rw;
};

void* worker(void* p) {
    auto* a = static_cast<Args*>(p);
    std::mt19937 rng(pthread_self());
    std::uniform_int_distribution<int> keydist(0,100000);
    std::uniform_real_distribution<double> opdist(0,1);

    for (int i=0; i<a->ops; i++) {
        int k = keydist(rng);
        if (opdist(rng) < a->write_ratio) {
            if (a->use_rw) map_put_rw(a->m,k,i);
            else map_put_mtx(a->m,k,i);
        } else {
            if (a->use_rw) map_get_rw(a->m,k);
            else map_get_mtx(a->m,k);
        }
    }
    return nullptr;
}

int main(int argc, char** argv) {
    int T = (argc>1)? std::atoi(argv[1]) : 4; // hilos
    int ops = (argc>2)? std::atoi(argv[2]) : 100000;
    double write_ratio = (argc>3)? std::atof(argv[3]) : 0.1; //10% writes
    int mode = (argc>4)? std::atoi(argv[4]) : 0; // 0=mutex, 1=rwlock

    Map m;
    std::vector<pthread_t> th(T);
    Args a{&m, ops, write_ratio, mode==1};

    auto start = std::chrono::high_resolution_clock::now();
    for (int i=0;i<T;i++) pthread_create(&th[i],nullptr,worker,&a);
    for (int i=0;i<T;i++) pthread_join(th[i],nullptr);
    auto end = std::chrono::high_resolution_clock::now();

    double t = std::chrono::duration<double>(end-start).count();
    printf("T=%d, ops=%d, writes=%.2f, mode=%s, tiempo=%.6f s, throughput=%.2f ops/s\n",
           T, ops, write_ratio,
           mode==1?"rwlock":"mutex",
           t, (double)T*ops/t);

    return 0;
}