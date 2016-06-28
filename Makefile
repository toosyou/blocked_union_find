CXXFLAGS += -fopenmp 
CXXFLAGS += -Iprogressbar/include -Lprogressbar -lprogressbar -lncurses

all: blocked_union

blocked_union: main.cpp block.h block.cpp
	g++-5 $(CXXFLAGS) main.cpp block.cpp -o $@
