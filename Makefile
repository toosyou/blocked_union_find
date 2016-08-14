CXXFLAGS += -std=c++11
CXXFLAGS += -fopenmp 
CXXFLAGS += -Iprogressbar/include -Lprogressbar -lprogressbar -lncurses

all: blocked_union

blocked_union: main.cpp block.h block.cpp block_coordinate.h
	g++-5 $(CXXFLAGS) main.cpp block.cpp -o $@

clean: blocked_union
	rm -rf blocked_union
