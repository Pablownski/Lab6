CXX = g++
CXXFLAGS = -O2 -std=gnu++17 -Wall -Wextra -pthread

SRC = $(wildcard src/*.cpp)
BIN = $(patsubst src/%.cpp,bin/%,$(SRC))

all: $(BIN)

bin/%: src/%.cpp | bin
	$(CXX) $(CXXFLAGS) $< -o $@

bin:
	mkdir -p bin


p1: bin/p1_counter
	./bin/p1_counter 4 1000000

p2: bin/p2_ring
	./bin/p2_ring 2 2 1024 100000

p3: bin/p3_rw
	# 90% lecturas (0.1 escrituras)
	./bin/p3_rw 4 100000 0.1 0
	./bin/p3_rw 4 100000 0.1 1
	# 70% lecturas (0.3 escrituras)
	./bin/p3_rw 4 100000 0.3 0
	./bin/p3_rw 4 100000 0.3 1
	# 50% lecturas (0.5 escrituras)
	./bin/p3_rw 4 100000 0.5 0
	./bin/p3_rw 4 100000 0.5 1


p4: bin/p4_deadlock_fixed
	./bin/p4_deadlock_fixed

p5: bin/p5_pipeline
	./bin/p5_pipeline 100000


bench: bench_p1 bench_p2 bench_p3 bench_p4 bench_p5

bench_p1: bin/p1_counter
	./scripts/bench_p1.sh

bench_p2: bin/p2_ring
	./scripts/bench_p2.sh

bench_p3: bin/p3_rw
	./scripts/bench_p3.sh



bin/p4_deadlock_fixed: src/p4_deadlock_fixed.cpp | bin
	$(CXX) $(CXXFLAGS) $< -o $@

bench_p5: bin/p5_pipeline
	./scripts/bench_p5.sh


clean:
	rm -rf bin/* results/*.csv
