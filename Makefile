CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra

SRCS := src/ast.cpp src/lexer.cpp src/parser.cpp src/eval.cpp src/main.cpp
OBJS := $(patsubst src/%.cpp, bin/%.o, $(SRCS))
TARGET := lamb

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/%.o: src/%.cpp | bin
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bin:
	mkdir -p bin

clean:
	rm -f $(OBJS) $(TARGET)
