#define TESTING_ENABLED

#include "main.cpp" 
#include <gtest/gtest.h>

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
};

TEST_F(OrderBookTest, AddSingleBid) {
    MBOEvent event{};
    event.action = 'A';
    event.side = 'B';
    event.price = 150 * PRICE_SCALE;
    event.size = 100;
    event.order_id = 12345;

    book.process_event(event);

    std::vector<std::pair<int64_t, PriceLevel>> top_bids, top_asks;
    book.get_top_levels(top_bids, top_asks);

    ASSERT_EQ(top_bids.size(), 1);
    ASSERT_TRUE(top_asks.empty());
    ASSERT_EQ(top_bids[0].first, 150 * PRICE_SCALE);
    ASSERT_EQ(top_bids[0].second.size, 100);
    ASSERT_EQ(top_bids[0].second.count, 1);
}

TEST_F(OrderBookTest, FullCancelRemovesLevel) {
    MBOEvent add_event{};
    add_event.action = 'A';
    add_event.side = 'B';
    add_event.price = 150 * PRICE_SCALE;
    add_event.size = 100;
    add_event.order_id = 12345;
    book.process_event(add_event);

    MBOEvent cancel_event = add_event;
    cancel_event.action = 'C';
    book.process_event(cancel_event);

    std::vector<std::pair<int64_t, PriceLevel>> top_bids, top_asks;
    book.get_top_levels(top_bids, top_asks);
    ASSERT_TRUE(top_bids.empty());
}