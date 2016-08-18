CXXFLAGS += -std=c++11
CXXFLAGS += -fopenmp
CXXFLAGS += -Iprogressbar/include -Lprogressbar -lprogressbar -lncurses
CXXFLAGS += -Icpp-lru-cache4file/include

all: blocked_union

blocked_union: main.cpp block.h block.cpp block_coordinate.h cpp-lru-cache4file/include/lrucache.hpp
	g++-6 $(CXXFLAGS) main.cpp block.cpp -o $@

clean: blocked_union
	rm -rf blocked_union
