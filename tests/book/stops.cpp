#include <doctest/doctest.h>
#include <memory>
#include <cmath>
#include <vector>

#include <book/types.h>
#include <book/plugins/self_trade_policy.h>
#include <book/plugins/positions.h>
#include <book/plugins/stop_orders.h>
#include "fixtures/order.h"
#include "fixtures/me.h"
#include "fixtures/helpers.h"

namespace matching_test {

#define SYMBOL_ID_1 1
#define USER_1 1
#define USER_2 2

#define BUY true
#define SELL false

typedef fixtures::OrderWithStopPrice Order;
typedef std::shared_ptr<Order> OrderPtr;

struct Tracker :
  public virtual book::BaseTracker<OrderPtr>,
  public book::plugins::SelfTradePolicyTracker<OrderPtr>
{
  Tracker(const OrderPtr& order) :
    book::BaseTracker<OrderPtr>(order),
    book::plugins::SelfTradePolicyTracker<OrderPtr>(order) {}
};

typedef fixtures::ME<
  Tracker,
  book::plugins::StopOrdersPlugin<Tracker>
> Book;

using Callbacks = Book::Callbacks;
using TypedCallback = Book::TypedCallback;

// Helper to check if a stop_trigger callback was received for a specific order
bool was_triggered(const Callbacks& cbs, const OrderPtr& order) {
  for (const auto& cb : cbs) {
    if (cb.type == TypedCallback::cb_order_stop_trigger && cb.order == order) {
      return true;
    }
  }
  return false;
}

// Helper to check if an order was matched (trade callback)
bool was_matched(const Callbacks& cbs, const OrderPtr& order) {
    for (const auto& cb : cbs) {
        if (cb.type == TypedCallback::cb_trade && (cb.order == order || cb.maker_order == order)) {
            return true;
        }
    }
    return false;
}

// Helper to check if order was accepted (into the book or triggered)
bool was_accepted(const Callbacks& cbs) {
    if (cbs.empty()) return false;
    return cbs[0].type == TypedCallback::cb_order_accept;
}

TEST_CASE("Stop Orders") {

    SUBCASE("Stop Buy Limit (Trigger & Post)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);
        
        // Stop Buy @ 110. Limit 111.
        auto stop_buy = std::make_shared<Order>(USER_1, BUY, 111, 10, 0, 110);
        auto cbs = book.add_and_get_cbs(stop_buy);
        
        // Should be accepted but NOT triggered
        CHECK(was_accepted(cbs));
        CHECK_FALSE(was_triggered(cbs, stop_buy));
        CHECK(book.bids().size() == 0); // Not in main book

        // Move Price to 105 -> No Trigger
        book.add(std::make_shared<Order>(USER_1, BUY, 105, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 105, 1, 0, 0));
        CHECK(book.market_price() == 105);
        CHECK_FALSE(was_triggered(cbs, stop_buy));

        // Move Price to 110 -> Trigger
        book.add(std::make_shared<Order>(USER_1, BUY, 110, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 110, 1, 0, 0));
        CHECK(book.market_price() == 110);
        
        CHECK(was_triggered(cbs, stop_buy));
        
        // After trigger, it becomes a limit buy @ 111.
        // Current market 110. No asks < 111 (except the one just filled at 110).
        // It should be posted to book.
        CHECK(book.bids().size() == 1);
        CHECK(book.bids().begin()->second.ptr() == stop_buy);
    }

    SUBCASE("Stop Sell Limit (Trigger & Post)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Stop Sell @ 90. Limit 89.
        auto stop_sell = std::make_shared<Order>(USER_1, SELL, 89, 10, 0, 90);
        auto cbs = book.add_and_get_cbs(stop_sell);

        CHECK(was_accepted(cbs));
        CHECK_FALSE(was_triggered(cbs, stop_sell));
        CHECK(book.asks().size() == 0);

        // Move to 95 -> No Trigger
        book.add(std::make_shared<Order>(USER_1, BUY, 95, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 95, 1, 0, 0));
        CHECK(book.market_price() == 95);
        CHECK_FALSE(was_triggered(cbs, stop_sell));

        // Move to 90 -> Trigger
        book.add(std::make_shared<Order>(USER_1, BUY, 90, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 90, 1, 0, 0));
        CHECK(book.market_price() == 90);
        CHECK(was_triggered(cbs, stop_sell));

        // Posted to book (Limit 89)
        CHECK(book.asks().size() == 1);
        CHECK(book.asks().begin()->second.ptr() == stop_sell);
    }

    SUBCASE("Stop Buy Limit (Trigger & Match)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Stop Buy @ 110. Limit 115.
        // There is already an Ask at 112.
        book.add(std::make_shared<Order>(USER_2, SELL, 112, 5, 0, 0));

        auto stop_buy = std::make_shared<Order>(USER_1, BUY, 115, 10, 0, 110);
        book.add(stop_buy);

        // Move Price to 110 -> Trigger
        book.add(std::make_shared<Order>(USER_1, BUY, 110, 1, 0, 0));
        auto cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 110, 1, 0, 0));
        CHECK(book.market_price() == 112.0);
        
        CHECK(was_triggered(cbs, stop_buy));
        
        // It triggers and becomes Limit Buy @ 115.
        // It should match against Ask @ 112.
        CHECK(was_matched(cbs, stop_buy));
        
        // Remaining qty (5) posted at 115
        CHECK(book.bids().size() == 1);
        // The ask @ 112 is gone
        CHECK(book.asks().size() == 0);
    }

    SUBCASE("Stop Market (Trigger & Match)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Liquidity for the market order
        book.add(std::make_shared<Order>(USER_2, SELL, 112, 10, 0, 0));

        // Stop Buy Market @ 110. (Price=0, Funds=10000 or Qty)
        auto stop_market = std::make_shared<Order>(USER_1, BUY, 0, 5, 0, 110); 
        
        book.add(stop_market);

        // Trigger at 110
        book.add(std::make_shared<Order>(USER_1, BUY, 110, 1, 0, 0));
        auto cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 110, 1, 0, 0));
        
        CHECK(was_triggered(cbs, stop_market));
        CHECK(was_matched(cbs, stop_market));
        
        // Market order should fill completely against Ask @ 112
        // Ask 112 had 10, matched 5. Remaining 5.
        CHECK(book.asks().size() == 1);
        CHECK(book.asks().begin()->second.qty_on_book() == 5);
    }

    SUBCASE("Immediate Trigger (Buy)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Stop Buy @ 90. Market 100. 100 >= 90. Immediate trigger.
        auto stop_buy = std::make_shared<Order>(USER_1, BUY, 105, 10, 0, 90);
        auto cbs = book.add_and_get_cbs(stop_buy);

        // Should NOT be triggered via callback because it happens synchronously during add()
        CHECK_FALSE(was_triggered(cbs, stop_buy));
        
        // It should be in the book (or matched)
        CHECK(book.bids().size() == 1);
        CHECK(book.bids().begin()->second.ptr() == stop_buy);
    }

    SUBCASE("Cascading Triggers") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);
        
        // Setup Liquidity
        book.add(std::make_shared<Order>(USER_2, SELL, 105, 1, 0, 0));
        book.add(std::make_shared<Order>(USER_2, SELL, 110, 1, 0, 0));
        
        // Stop Buy A: Trigger 102. Limit 115.
        auto stop_a = std::make_shared<Order>(USER_1, BUY, 115, 10, 0, 102);
        book.add(stop_a);
        
        // Stop Buy B: Trigger 108. Limit 115.
        auto stop_b = std::make_shared<Order>(USER_1, BUY, 115, 10, 0, 108);
        book.add(stop_b);
        
        // Trigger Trade: Buy @ 102.
        book.add(std::make_shared<Order>(USER_1, BUY, 102, 1, 0, 0));
        auto cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 102, 1, 0, 0));
        
        // Expectation:
        // 1. Trade @ 102 (Trigger Trade). Price -> 102.
        // 2. Stop A triggers (102>=102). Becomes Limit Buy @ 115.
        // 3. Stop A matches Ask @ 105. Price -> 105.
        // 4. Stop A matches Ask @ 110. Price -> 110.
        // 5. Stop B triggers (110>=108). Becomes Limit Buy @ 115.
        // 6. Stop B posts (or matches if more liquidity).
        
        CHECK(book.market_price() == 110.0);
        CHECK(was_triggered(cbs, stop_a));
        CHECK(was_triggered(cbs, stop_b));
        
        // Check triggers executed trades
        CHECK(was_matched(cbs, stop_a));
        // Stop B triggered but posted (no liquidity left at 110+ except implied empty?)
        // Limit 115. Ask 110 filled. No asks left. Stop B posts.
        CHECK(book.bids().size() == 2); // Stop A remainder + Stop B full
    }
}
}
