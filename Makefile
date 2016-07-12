CXXFLAGS += -fopenmp 
CXXFLAGS += -Iprogressbar/include -Lprogressbar -lprogressbar -lncurses

all: blocked_union set2ipt reindex

blocked_union: main.cpp block.h block.cpp block_coordinate.h
	g++-5 $(CXXFLAGS) main.cpp block.cpp -o $@

set2ipt: set2ipt.cpp
	g++-5 $(CXXFLAGS) set2ipt.cpp -o $@

reindex: reindex.cpp
	g++-5 $(CXXFLAGS) reindex.cpp -o $@
