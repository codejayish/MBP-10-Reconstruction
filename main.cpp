#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iomanip>
#include <functional>
#include <cstdint> 


constexpr int MBP_LEVELS = 10;
constexpr int64_t PRICE_SCALE = 10000;


struct PriceLevel {
    uint64_t size = 0;
    uint64_t count = 0;
};

struct MBOEvent {
    std::string ts_recv;
    std::string ts_event;
    int rtype;
    int publisher_id;
    int instrument_id;
    int flags;
    int64_t ts_in_delta;
    uint64_t sequence;
    std::string symbol;
    uint64_t order_id;
    char action;
    char side;
    int64_t price;
    uint64_t size;
};

class OrderBook {
private:
    std::map<int64_t, PriceLevel, std::greater<int64_t>> bids;
    std::map<int64_t, PriceLevel> asks;
    std::unordered_map<uint64_t, std::pair<int64_t, uint64_t>> order_map; 

public:
    void clear() {
        bids.clear();
        asks.clear();
        order_map.clear();
    }

    void add_order(const MBOEvent& event) {
        order_map[event.order_id] = {event.price, event.size};
        if (event.side == 'B') {
            bids[event.price].size += event.size;
            bids[event.price].count++;
        } else if (event.side == 'A') {
            asks[event.price].size += event.size;
            asks[event.price].count++;
        }
    }

    void cancel_order(const MBOEvent& event) {
        auto it = order_map.find(event.order_id);
        if (it == order_map.end()) return;

        int64_t price = it->second.first;
        uint64_t original_size = it->second.second;

        if (event.side == 'B') {
            auto level_it = bids.find(price);
            if (level_it != bids.end()) {
                level_it->second.size -= event.size;
                if (event.size == original_size && level_it->second.count > 0) {
                    level_it->second.count--;
                }
                if (level_it->second.size == 0 || level_it->second.count == 0) {
                    bids.erase(level_it);
                }
            }
        } else if (event.side == 'A') {
            auto level_it = asks.find(price);
            if (level_it != asks.end()) {
                level_it->second.size -= event.size;
                if (event.size == original_size && level_it->second.count > 0) {
                   level_it->second.count--;
                }
                if (level_it->second.size == 0 || level_it->second.count == 0) {
                    asks.erase(level_it);
                }
            }
        }
        
        if (event.size >= original_size) {
            order_map.erase(it);
        }
    }

    void process_event(const MBOEvent& event) {
        switch (event.action) {
            case 'A': add_order(event); break;
            case 'C': 
            case 'M': cancel_order(event); break;
            case 'R': clear(); break;
        }
    }

    void get_top_levels(std::vector<std::pair<int64_t, PriceLevel>>& top_bids, std::vector<std::pair<int64_t, PriceLevel>>& top_asks) const {
        top_bids.clear();
        top_asks.clear();
        
        auto bid_it = bids.begin();
        for(int i = 0; i < MBP_LEVELS && bid_it != bids.end(); ++i, ++bid_it) {
            top_bids.push_back(*bid_it);
        }

        auto ask_it = asks.begin();
        for(int i = 0; i < MBP_LEVELS && ask_it != asks.end(); ++i, ++ask_it) {
            top_asks.push_back(*ask_it);
        }
    }
};

void write_header(std::ofstream& out) {
    out << ",ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence";
    for(int i = 0; i < MBP_LEVELS; ++i) {
        out << ",bid_px_" << std::setfill('0') << std::setw(2) << i
            << ",bid_sz_" << std::setfill('0') << std::setw(2) << i
            << ",bid_ct_" << std::setfill('0') << std::setw(2) << i
            << ",ask_px_" << std::setfill('0') << std::setw(2) << i
            << ",ask_sz_" << std::setfill('0') << std::setw(2) << i
            << ",ask_ct_" << std::setfill('0') << std::setw(2) << i;
    }
    out << ",symbol,order_id\n";
}

void write_mbp_row(std::ofstream& out, const MBOEvent& event, int row_index, const OrderBook& book) {
    out << row_index << ',' << event.ts_event << ',' << event.ts_event << ','
        << 10 << ',' << event.publisher_id << ',' << event.instrument_id << ','
        << event.action << ',' << event.side << ',' << 0 << ',';

    if (event.action == 'R') {
        out << "," << event.size << ',';
    } else {
        out << std::fixed << std::setprecision(4) << static_cast<double>(event.price) / PRICE_SCALE << ',' << event.size << ',';
    }

    out << event.flags << ',' << event.ts_in_delta << ',' << event.sequence;

    std::vector<std::pair<int64_t, PriceLevel>> top_bids, top_asks;
    book.get_top_levels(top_bids, top_asks);

    for (std::size_t i = 0; i < MBP_LEVELS; ++i) {
        if (i < top_bids.size()) {
            out << ',' << std::fixed << std::setprecision(4) << static_cast<double>(top_bids[i].first) / PRICE_SCALE
                << ',' << top_bids[i].second.size << ',' << top_bids[i].second.count;
        } else {
            out << ",,0,0";
        }
        if (i < top_asks.size()) {
            out << ',' << std::fixed << std::setprecision(4) << static_cast<double>(top_asks[i].first) / PRICE_SCALE
                << ',' << top_asks[i].second.size << ',' << top_asks[i].second.count;
        } else {
            out << ",,0,0";
        }
    }
    out << ',' << event.symbol << ',' << event.order_id << '\n';
}

bool parse_line(const std::string& line, MBOEvent& event) {
    std::stringstream ss(line);
    std::string token;
    try {
        std::getline(ss, event.ts_recv, ',');
        std::getline(ss, event.ts_event, ',');
        std::getline(ss, token, ','); event.rtype = std::stoi(token);
        std::getline(ss, token, ','); event.publisher_id = std::stoi(token);
        std::getline(ss, token, ','); event.instrument_id = std::stoi(token);
        std::getline(ss, token, ','); event.action = token.empty() ? ' ' : token[0];
        std::getline(ss, token, ','); event.side = token.empty() ? ' ' : token[0];
        std::getline(ss, token, ',');
        event.price = token.empty() ? 0 : static_cast<int64_t>(std::stod(token) * PRICE_SCALE);
        std::getline(ss, token, ','); event.size = std::stoull(token);
        std::getline(ss, token, ','); 
        std::getline(ss, token, ','); event.order_id = std::stoull(token);
        std::getline(ss, token, ','); event.flags = std::stoi(token);
        std::getline(ss, token, ','); event.ts_in_delta = std::stoll(token);
        std::getline(ss, token, ','); event.sequence = std::stoull(token);
        std::getline(ss, event.symbol);
        if (!event.symbol.empty() && event.symbol.back() == '\r') {
            event.symbol.pop_back();
        }
    } catch (const std::exception& e) {
        std::cerr << "\n[!] PARSING FAILED ON LINE: " << line << '\n';
        std::cerr << "    ERROR: " << e.what() << "\n\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    if (argc < 2) {
        std::cerr << "Error: Missing input file.\nUsage: " << argv[0] << " <mbo_file.csv>\n";
        return 1;
    }

    std::ifstream input_file(argv[1]);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open input file " << argv[1] << "\n";
        return 1;
    }

    std::ofstream output_file("mbp.csv");
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not create output file mbp.csv\n";
        return 1;
    }

    write_header(output_file);
    OrderBook book;
    std::string line;
    int row_index = 0;

    std::getline(input_file, line);

    MBOEvent event;
    while (std::getline(input_file, line)) {
        if (line.empty()) continue;

        if (parse_line(line, event)) {
            book.process_event(event);
            write_mbp_row(output_file, event, row_index, book);
            row_index++;
        }
    }

    std::cout << "Reconstruction complete. Processed " << row_index << " records.\n";
    std::cout << "Output written to mbp.csv\n";

    return 0;
}