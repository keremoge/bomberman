CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra
LDLIBS = -lncurses

bomberman: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f bomberman

.PHONY: clean
