CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude
SAN      = -fsanitize=address,undefined
HEADERS  = $(wildcard include/*/*.h)

all: realSearch

realSearch: src/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp

test_structures: tests/test_structures.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ tests/test_structures.cpp

test_structures_san: tests/test_structures.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SAN) -o $@ tests/test_structures.cpp

test_sorting: tests/test_sorting.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ tests/test_sorting.cpp

test_sorting_san: tests/test_sorting.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SAN) -o $@ tests/test_sorting.cpp

test: test_structures test_structures_san test_sorting test_sorting_san
	./test_structures
	./test_structures_san
	./test_sorting
	./test_sorting_san

clean:
	rm -f realSearch test_structures test_structures_san test_sorting test_sorting_san
