#pragma once

#include <book/plugin.h>
#include <map>
#include <set>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>

namespace book {
namespace plugins {

struct TrailingStopOrder {
public:
  virtual ~TrailingStopOrder() = default;
  virtual double trailing_amount() const = 0;
  
  /* this is used to find the order in case user wants to cancel or replace it.
    storing a separate hashmap by ID would require heap allocation and extra cleanup */
  double trailing_stop_key() const {
    return trailing_stop_key_;
  };

  void trailing_stop_key(double k) {
    trailing_stop_key_ = k;
  }

private:
  double trailing_stop_key_;
};

template <class OrderPtr>
struct TrailingStopOrdersTracker {
};


template <class Tracker>
class TrailingStopOrdersPlugin : public Plugin<Tracker> {
public:
  using OrderPtr = typename Plugin<Tracker>::OrderPtr;
  using TrackerVec = typename Plugin<Tracker>::TrackerVec;
  using TypedCallback = typename Plugin<Tracker>::TypedCallback;
  using TrailingMap = std::multimap<double, Tracker>;

protected:
  bool should_add_tracker(const Tracker& taker) override {
    double trailing_amount = taker.ptr()->trailing_amount();
    if(trailing_amount != 0) {
      add_trailing_stop(taker);
      return false;
    }

    return true;
  }

  void on_market_price_change(double prev_price, double new_price) override {
    const double dP = std::abs(new_price - prev_price);

    if(new_price > prev_price) {
      if(!trailStopAsks_.empty()) {
        const double smallestAskTrail =
          trailStopAsks_.begin()->first
            - trailStopAsks_.begin()->second.ptr()->trailing_amount();
        if(ask_cursor_ > smallestAskTrail + dP)
          ask_cursor_ -= dP;
        else
          ask_cursor_ = smallestAskTrail;
      }
      
      if(!trailStopBids_.empty()) {
        bid_cursor_ += dP;
        check_trailing_stops(trailStopBids_, bid_cursor_);
      }
    }

    else if(new_price < prev_price) {
      if(!trailStopBids_.empty()) {
        const double smallestBidTrail = 
          trailStopBids_.begin()->first
            - trailStopBids_.begin()->second.ptr()->trailing_amount();


        if(bid_cursor_ > smallestBidTrail + dP)
          bid_cursor_ -= dP;
        else
          bid_cursor_ = smallestBidTrail;
      }

      if(!trailStopAsks_.empty()) {
        ask_cursor_ += dP;
        check_trailing_stops(trailStopAsks_, ask_cursor_);
      }
    }
  }


  void after_add_tracker(const Tracker& taker) override {
    while(!pending_orders_.empty())
      submit_pending_orders();
  }

  void cancel(const OrderPtr& order, CancelReasons reason) override {
    /* TODO
      find in the trailStopBids_ and trailStopAsks_ using order->trailing_amount() as a hint
      then remove the order from those containers. 
    */
  }


private:
  double bid_cursor_ = 0;
  double ask_cursor_ = 0;
  TrailingMap trailStopBids_;
  TrailingMap trailStopAsks_;
  TrackerVec pending_orders_;

  void add_trailing_stop(const Tracker& taker) {
    const OrderPtr& order = taker.ptr();
    bool isBuy = taker.is_bid();
    typename TrailingMap::iterator pos;

    if(isBuy) {
      double key = order->trailing_amount() + bid_cursor_;
      order->trailing_stop_key(key);
      pos = trailStopBids_.emplace(key, std::move(taker));
    }
    else {
      double key = order->trailing_amount() + ask_cursor_;
      order->trailing_stop_key(key);
      pos = trailStopAsks_.emplace(key, std::move(taker));
    }
  }

  void check_trailing_stops(TrailingMap& stops, double trail) {
    auto pos = stops.begin();
    while(pos != stops.end()) {
      auto here = pos++;
      const Tracker& tracker = here->second;

      if(trail < here->first) {
        break;
      }

      pending_orders_.push_back(std::move(tracker));
      stops.erase(here);
    }
  }


  void submit_pending_orders() {
    TrackerVec pending;
    pending.swap(pending_orders_);
    for(auto pos = pending.begin(); pos != pending.end(); ++pos) {
      Tracker& tracker = *pos;
      this->add_tracker(tracker);
      this->callbacks().push_back(TypedCallback::stop_trigger(tracker.ptr()));
    }
  }




};

} // namespace plugins
} // namespace book
