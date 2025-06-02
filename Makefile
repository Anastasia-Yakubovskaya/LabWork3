CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Iinclude -O3
GTEST_FLAGS = -lgtest -lgtest_main -pthread

SRC = src/skip_list.cpp
TEST_SRC = tests/test_skip_list.cpp
HEADERS = include/skip_list.h

LIB = libskip_list.a
TEST_EXE = test_skip_list

all: $(LIB) tests

$(LIB): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $(SRC) -o skip_list.o
	ar rcs $(LIB) skip_list.o
	rm -f skip_list.o

tests: $(TEST_EXE)

$(TEST_EXE): $(TEST_SRC) $(LIB)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -L. -lskip_list $(GTEST_FLAGS) -o $(TEST_EXE)

docs:
	doxygen docs/Doxyfile

clean:
	rm -f $(LIB) $(TEST_EXE) *.o
	rm -rf docs/html

.PHONY: all tests docs clean
