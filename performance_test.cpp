#define TESTING_ENABLED

#include "main.cpp" 
#include <iostream>
#include <chrono>

void run_performance_test() {
    OrderBook book;
    MBOEvent event{};
    event.action = 'A';
    event.side = 'B';
    event.size = 100;

    const int num_operations = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        event.price = (15000 + (i % 100)) * PRICE_SCALE;
        event.order_id = i + 1;
        book.process_event(event);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> duration = end - start;
    
    std::cout << "--- Performance Test ---" << std::endl;
    std::cout << "Processed " << num_operations << " add operations.\n";
    std::cout << "Total time: " << duration.count() << " microseconds.\n";
    std::cout << "Average time per operation: " << duration.count() / num_operations << " microseconds." << std::endl;
}

int main() {
    run_performance_test();
    return 0;
}