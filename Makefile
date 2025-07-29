# Makefile for the MBP-10 Orderbook Reconstruction Project
CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -pthread
LDFLAGS_GTEST = -L./googletest/lib -lgtest -lgtest_main
INCLUDES_GTEST = -I./googletest/googletest/include

# Targets
TARGET = reconstruction_gemini
TEST_TARGET = orderbook_tester
PERF_TARGET = perf_tester

# Source files
SOURCES = main.cpp
TEST_SOURCES = test_orderbook.cpp
PERF_SOURCES = performance_test.cpp

# Build main reconstruction executable
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Run reconstruction on mbo.csv
run: $(TARGET)
	./$(TARGET) mbo.csv

# Build and run unit tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES)
	$(CXX) $(CXXFLAGS) -DTESTING_ENABLED $(TEST_SOURCES) $(INCLUDES_GTEST) $(LDFLAGS_GTEST) -o $(TEST_TARGET)

# Build and run performance benchmark
perf_test: $(PERF_TARGET)
	./$(PERF_TARGET)

$(PERF_TARGET): $(PERF_SOURCES)
	$(CXX) $(CXXFLAGS) -DTESTING_ENABLED -o $(PERF_TARGET) $(PERF_SOURCES)

# Clean all build artifacts and output
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(PERF_TARGET) *.o mbp.csv

.PHONY: all run test perf_test clean
