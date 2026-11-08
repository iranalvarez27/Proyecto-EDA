CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS =
EXE =

ifeq ($(OS),Windows_NT)
LDFLAGS = -static
EXE = .exe
endif

HEADERS = $(wildcard include/*.h)
VECINOS = src/vecinos.cpp
MC = src/celdas_superficie.cpp src/mc_compartido.cpp
OCTREE = src/octree_lineal.cpp $(VECINOS) $(MC)

PROGRAMAS = bin/test_vecinos$(EXE) bin/bench_vecinos$(EXE) \
            bin/test_octree$(EXE) bin/bench_octree$(EXE) bin/test_malla$(EXE) bin/mc$(EXE)

.PHONY: all test

all: $(PROGRAMAS)

bin/test_vecinos$(EXE): tests/test_vecinos.cpp $(VECINOS) $(HEADERS)
	$(CXX) $(CXXFLAGS) tests/test_vecinos.cpp $(VECINOS) -o $@ $(LDFLAGS)

bin/bench_vecinos$(EXE): src/bench_vecinos.cpp $(VECINOS) $(HEADERS)
	$(CXX) $(CXXFLAGS) src/bench_vecinos.cpp $(VECINOS) -o $@ $(LDFLAGS)

bin/test_octree$(EXE): tests/test_octree.cpp $(OCTREE) $(HEADERS)
	$(CXX) $(CXXFLAGS) tests/test_octree.cpp $(OCTREE) -o $@ $(LDFLAGS)

bin/bench_octree$(EXE): src/bench_octree.cpp $(OCTREE) $(HEADERS)
	$(CXX) $(CXXFLAGS) src/bench_octree.cpp $(OCTREE) -o $@ $(LDFLAGS)

bin/mc$(EXE): src/main_mc.cpp src/mc_baseline.cpp $(MC) $(VECINOS) $(HEADERS)
	$(CXX) $(CXXFLAGS) src/main_mc.cpp src/mc_baseline.cpp $(MC) $(VECINOS) -o $@ $(LDFLAGS)

bin/test_malla$(EXE): tests/test_malla.cpp src/mc_baseline.cpp $(MC) $(VECINOS) $(HEADERS)
	$(CXX) $(CXXFLAGS) tests/test_malla.cpp src/mc_baseline.cpp $(MC) $(VECINOS) -o $@ $(LDFLAGS)

test: bin/test_vecinos$(EXE) bin/test_octree$(EXE) bin/test_malla$(EXE)
	bin/test_vecinos$(EXE)
	bin/test_octree$(EXE)
	bin/test_malla$(EXE)
