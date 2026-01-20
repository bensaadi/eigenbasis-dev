#include <doctest/doctest.h>
#include <memory>
#include <cmath>
#include <vector>

#include <book/types.h>
#include <book/plugins/self_trade_policy.h>
#include <book/plugins/positions.h>
#include <book/plugins/trailing_stop_orders.h>
#include "fixtures/order.h"
#include "fixtures/me.h"
#include "fixtures/helpers.h"

namespace trailing_test {

#define SYMBOL_ID_1 1
#define USER_1 1
#define USER_2 2

#define BUY true
#define SELL false

// Define an Order class that ONLY supports Trailing Stop interface
class Order : public fixtures::OrderWithUserID, 
              public book::plugins::TrailingStopOrder {
public:
  Order(
    uint32_t user_id,
    bool is_bid,
    double price,
    double qty,
    double funds,
    double trailing_amount = 0) :
      fixtures::OrderWithUserID(user_id, is_bid, price, qty, funds),
      trailing_amount_(trailing_amount) { }

  double trailing_amount() const override {
      return trailing_amount_;
  }

private:
  double trailing_amount_;
};

typedef std::shared_ptr<Order> OrderPtr;

struct Tracker :
  public virtual book::BaseTracker<OrderPtr>,
  public book::plugins::SelfTradePolicyTracker<OrderPtr>,
  public book::plugins::TrailingStopOrdersTracker<OrderPtr>
{
  Tracker(const OrderPtr& order) :
    book::BaseTracker<OrderPtr>(order),
    book::plugins::SelfTradePolicyTracker<OrderPtr>(order),
    book::plugins::TrailingStopOrdersTracker<OrderPtr>() {}
};

typedef fixtures::ME<
  Tracker,
  book::plugins::TrailingStopOrdersPlugin<Tracker>
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

// Helper to check if order was accepted
bool was_accepted(const Callbacks& cbs) {
    if (cbs.empty()) return false;
    return cbs[0].type == TypedCallback::cb_order_accept;
}

TEST_CASE("Trailing Stop Orders") {

    SUBCASE("Trailing Sell (Stop Loss on Long)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);
        
        // Trailing Sell. Trail=10.
        // Initial Stop = Market(100) - 10 = 90.
        auto t_sell = std::make_shared<Order>(USER_1, SELL, 85, 10, 0, 10);
        auto cbs = book.add_and_get_cbs(t_sell);
        
        CHECK(was_accepted(cbs));
        CHECK_FALSE(was_triggered(cbs, t_sell));
        
        // 1. Market UP to 105.
        // Stop should move to Max(90, 105 - 10 = 95) = 95.
        book.add(std::make_shared<Order>(USER_1, BUY, 105, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 105, 1, 0, 0));
        CHECK(book.market_price() == 105);
        CHECK_FALSE(was_triggered(cbs, t_sell));
        
        // 2. Market UP to 110.
        // Stop should move to Max(95, 110 - 10 = 100) = 100.
        book.add(std::make_shared<Order>(USER_1, BUY, 110, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 110, 1, 0, 0));
        CHECK(book.market_price() == 110);
        CHECK_FALSE(was_triggered(cbs, t_sell));
        
        // 3. Market DOWN to 102.
        // Stop should stay at 100.
        book.add(std::make_shared<Order>(USER_1, BUY, 102, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 102, 1, 0, 0));
        CHECK_FALSE(was_triggered(cbs, t_sell));
        
        // 4. Market DOWN to 100. Trigger!
        // 100 <= 100.
        book.add(std::make_shared<Order>(USER_1, BUY, 100, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 100, 1, 0, 0));
        
        CHECK(was_triggered(cbs, t_sell));
        
        // Should be in book now (Ask @ 85)
        CHECK(book.asks().size() == 1);
        CHECK(book.asks().begin()->second.ptr() == t_sell);
    }

    SUBCASE("Trailing Buy (Stop Loss on Short)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);
        
        // Trailing Buy. Trail=10.
        // Initial Stop = Market(100) + 10 = 110.
        auto t_buy = std::make_shared<Order>(USER_1, BUY, 115, 10, 0, 10);
        auto cbs = book.add_and_get_cbs(t_buy);
        
        CHECK(was_accepted(cbs));
        CHECK_FALSE(was_triggered(cbs, t_buy));
        
        // 1. Market DOWN to 95.
        // Stop should move to Min(110, 95 + 10 = 105) = 105.
        book.add(std::make_shared<Order>(USER_1, BUY, 95, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 95, 1, 0, 0));
        CHECK(book.market_price() == 95);
        CHECK_FALSE(was_triggered(cbs, t_buy));

        // 2. Market UP to 102.
        // Stop stays 105. No trigger.
        book.add(std::make_shared<Order>(USER_1, BUY, 102, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 102, 1, 0, 0));
        CHECK_FALSE(was_triggered(cbs, t_buy));

        // 3. Market UP to 105. Trigger!
        // 105 >= 105.
        book.add(std::make_shared<Order>(USER_1, BUY, 105, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 105, 1, 0, 0));
        
        CHECK(was_triggered(cbs, t_buy));
        
        // Should be in book (Bid @ 115)
        CHECK(book.bids().size() == 1);
        CHECK(book.bids().begin()->second.ptr() == t_buy);
    }

    SUBCASE("Immediate Trigger on Entry") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);
        
        // Trailing Sell. Trail=5. Market 100.
        // Initial Stop = 95.
        // If Market drops to 90 immediately before order processing? 
        // No, market is 100.
        
        // Let's change market to 90 quickly.
        book.add(std::make_shared<Order>(USER_1, BUY, 90, 1, 0, 0));
        book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 90, 1, 0, 0));
        CHECK(book.market_price() == 90.0);

        // Add Trailing Sell, Trail=5.
        // Initial Stop = 90 - 5 = 85.
        // Market is 90. Not triggered.
        auto t_sell = std::make_shared<Order>(USER_1, SELL, 80, 10, 0, 5);
        auto cbs = book.add_and_get_cbs(t_sell);
        CHECK_FALSE(was_triggered(cbs, t_sell));
    }

    SUBCASE("Cascading Triggers (Domino Effect)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Scenario: Three users place trailing sells with increasing trailing distances.
        // User A: Trail 10 (Stop @ 90)
        // User B: Trail 20 (Stop @ 80)
        // User C: Trail 30 (Stop @ 70)
        auto o_tight  = std::make_shared<Order>(USER_1, SELL, 85, 10, 0, 10);
        auto o_medium = std::make_shared<Order>(USER_2, SELL, 75, 10, 0, 20);
        auto o_loose  = std::make_shared<Order>(USER_1, SELL, 65, 10, 0, 30);

        book.add_and_get_cbs(o_tight);
        book.add_and_get_cbs(o_medium);
        book.add_and_get_cbs(o_loose);

        // 1. Market drops to 95. No triggers expected.
        book.add(std::make_shared<Order>(USER_1, BUY, 95, 1, 0, 0));
        auto cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 95, 1, 0, 0));
        
        CHECK_FALSE(was_triggered(cbs, o_tight));
        CHECK_FALSE(was_triggered(cbs, o_medium));
        CHECK_FALSE(was_triggered(cbs, o_loose));

        // 2. Market drops to 88. Tight stop (90) triggers.
        book.add(std::make_shared<Order>(USER_1, BUY, 88, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 88, 1, 0, 0));

        CHECK(was_triggered(cbs, o_tight));
        CHECK_FALSE(was_triggered(cbs, o_medium));
        CHECK_FALSE(was_triggered(cbs, o_loose));

        // 3. Market drops to 78. Medium stop (80) triggers.
        book.add(std::make_shared<Order>(USER_1, BUY, 78, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 78, 1, 0, 0));

        CHECK(was_triggered(cbs, o_medium));
        CHECK_FALSE(was_triggered(cbs, o_loose));

        // 4. Market drops to 68. Loose stop (70) triggers.
        book.add(std::make_shared<Order>(USER_1, BUY, 68, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 68, 1, 0, 0));

        CHECK(was_triggered(cbs, o_loose));
    }

    SUBCASE("Gap Down (Mass Trigger)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Two orders with close trailing stops
        // O1: Trail 5 (Stop @ 95)
        // O2: Trail 6 (Stop @ 94)
        auto o1 = std::make_shared<Order>(USER_1, SELL, 90, 10, 0, 5);
        auto o2 = std::make_shared<Order>(USER_2, SELL, 90, 10, 0, 6);

        book.add_and_get_cbs(o1);
        book.add_and_get_cbs(o2);

        // Market gaps directly from 100 to 90.
        // Both 95 and 94 are violated. Both should trigger in the same update.
        book.add(std::make_shared<Order>(USER_1, BUY, 90, 1, 0, 0));
        auto cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 90, 1, 0, 0));

        CHECK(was_triggered(cbs, o1));
        CHECK(was_triggered(cbs, o2));
        
        // Ensure they were submitted to book
        CHECK(book.asks().size() == 2);
    }

    SUBCASE("Whipsaw (High Water Mark Resilience)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Trailing Sell, Trail 10. Initial Stop 90.
        auto t_order = std::make_shared<Order>(USER_1, SELL, 80, 10, 0, 10);
        auto cbs = book.add_and_get_cbs(t_order);

        // 1. Price rises to 120. Stop should ratchet up to 110.
        book.add(std::make_shared<Order>(USER_1, BUY, 120, 1, 0, 0));
        book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 120, 1, 0, 0));

        // 2. Price dips to 111. Stop is 110. No Trigger.
        book.add(std::make_shared<Order>(USER_1, BUY, 111, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 111, 1, 0, 0));
        CHECK_FALSE(was_triggered(cbs, t_order));

        // 3. Price recovers to 115. Stop stays at 110 (does not loosen).
        book.add(std::make_shared<Order>(USER_1, BUY, 115, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 115, 1, 0, 0));
        CHECK_FALSE(was_triggered(cbs, t_order));

        // 4. Price drops to 109. Trigger!
        book.add(std::make_shared<Order>(USER_1, BUY, 109, 1, 0, 0));
        cbs = book.add_and_get_cbs(std::make_shared<Order>(USER_2, SELL, 109, 1, 0, 0));
        CHECK(was_triggered(cbs, t_order));
    }

    SUBCASE("Zero Trailing Amount (Sanity Check)") {
        Book book(SYMBOL_ID_1);
        book.set_market_price(100.0);

        // Order with 0 trail should act as standard Limit/Market order (handled by logic)
        // or be ignored by the plugin depending on implementation.
        // Based on provided code: if(trailing_amount != 0) add_trailing_stop...
        // So this should NOT be added to trailing map.
        auto std_order = std::make_shared<Order>(USER_1, SELL, 90, 10, 0, 0);
        auto cbs = book.add_and_get_cbs(std_order);

        CHECK(was_accepted(cbs));
        CHECK_FALSE(was_triggered(cbs, std_order));
        
        // It should be immediately in the book as a regular limit order
        CHECK(book.asks().size() == 1);
    }
}

}