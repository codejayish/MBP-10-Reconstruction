# Makefile for the MBP-10 Orderbook Reconstruction Project

CXX = g++
# -std=c++17: Use the C++17 standard
# -O3: Enable maximum optimization level
# -Wall -Wextra: Enable all common warnings for cleaner code
# -march=native: Optimize for the specific architecture of the machine it's compiled on
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native

TARGET = reconstruction_gemini

SOURCES = main.cpp

# Default target: build the executable
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Command to run the reconstruction on the sample data
run: all
	./$(TARGET) mbo.csv

# Clean up build artifacts and output files
clean:
	rm -f $(TARGET) *.o mbp.csv

.PHONY: all clean run