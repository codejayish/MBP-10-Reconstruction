# Blockhouse Quant Dev Work Trial: MBP-10 Reconstruction

## 1. Overview

This program reconstructs a 10-level Market-by-Price (MBP-10) order book from a Market-by-Order (MBO) data feed. It reads an `mbo.csv` file, processes each event in a streaming fashion, and writes the resulting order book state for each event to `mbp.csv`.

The implementation prioritizes high performance (speed), low memory usage, and correctness, as required for high-frequency trading systems.

## 2. Build Instructions

A `Makefile` is provided for easy compilation. With a C++ compiler like `g++` and `make` installed, navigate to the project directory and run:

make

This will create an executable file named `reconstruction_gemini`. The `-O3` and `-march=native` flags are used to ensure the compiler generates highly optimized machine code.

## 3. Run Instructions

To run the program, use the following command, passing the input MBO file as an argument:

./reconstruction_gemini mbo.csv

The program will process the input file and generate an output file named `mbp.csv` in the same directory. A confirmation message will be printed to the console upon completion.

## 4. Design and Optimization Strategy

Performance and correctness are the core principles of this design.

### a. Streaming Architecture
The program processes the input file line-by-line, immediately writing a corresponding output line. This streaming approach ensures a constant, low memory footprint (O(1) space complexity relative to file size) and low latency. It can handle files of any size without running out of RAM, which is critical for market data. This is in contrast to reading the entire file into memory first, which is not scalable.

### b. Data Structures for the Order Book
- **`std::map` for Price Levels:** Two `std::map` containers are used for the bid and ask sides. This choice is deliberate: `std::map` is a balanced binary search tree that automatically keeps price levels sorted. This makes retrieving the top 10 levels extremely fast (O(log N) to find the start, then O(1) for each subsequent level). `std::greater<>` is used for the bid map to keep the highest bids at the beginning.
- **Combined `PriceLevel` Struct:** Instead of using separate maps for price, size, and order count, a `struct PriceLevel { size, count }` is used as the map's value. This improves data locality and eliminates the risk of synchronization bugs between multiple containers.
- **`std::unordered_map` for Order Tracking:** An `unordered_map` (`order_map`) tracks individual orders by their ID. This provides O(1) average-time lookup, which is essential for processing cancellations (`C`) and modifications (`M`) efficiently without searching the entire book.

### c. Scaled Integer Arithmetic for Prices
Floating-point numbers (`double`, `float`) are avoided for price representation due to potential precision errors and slower performance. Instead, prices are read as doubles, multiplied by a scaling factor (e.g., 10000), and stored as 64-bit integers (`int64_t`). All book logic is performed using fast and exact integer arithmetic.

### d. Efficient I/O
- `std::ios_base::sync_with_stdio(false)` and `std::cin.tie(NULL)` are used to decouple C++ streams from C standard streams, providing a significant speed boost for I/O operations.
- A robust `stringstream`-based parser is used to correctly handle the specific CSV format, including string-based timestamps and empty fields.
