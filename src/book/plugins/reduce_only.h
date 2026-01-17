/*
 * Copyright (c) 2026 Lyes Bensaadi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <list>
#include <cassert>
#include <map>

#include <book/tracker.h>
#include "positions.h"

#include <book/plugins/trackers/user_id_tracker.h>

namespace book {
namespace plugins {

template <class OrderPtr>
struct ReduceOnlyTracker : public virtual UserIDTracker<OrderPtr>,
public virtual BaseTracker<OrderPtr> {
  ReduceOnlyTracker(const OrderPtr& order) :
    BaseTracker<OrderPtr>(order),
    reduce_only_(order->reduce_only()),
    is_bid_(order->is_bid()) {
      UserIDTracker<OrderPtr>::set_user_id(order->user_id());
    }
  
  bool reduce_only() const { return reduce_only_; };
  bool is_bid() const { return is_bid_; }

  private:
    const bool reduce_only_;
    const bool is_bid_;
};

struct ReduceOnlyOrder {
  virtual bool reduce_only() const = 0;
};


template <class Tracker>
class ReduceOnlyPlugin : public virtual PositionsInterface,
public Plugin<Tracker> {
protected:
  typedef typename Tracker::OrderPtr OrderPtr;
  typedef Callback<OrderPtr> TypedCallback;

  void should_trade(
    Tracker& taker,
    Tracker& maker,
    CancelReasons& taker_reason,
    CancelReasons& maker_reason)
  {
    /* only test maker. taker is handled in pre_add */
    if(try_reduce(maker)) maker_reason = reduce_only_match;
  }

  bool try_reduce(Tracker& tracker) {
    if(!tracker.reduce_only())
      return false;

    Position position;
    bool found = this->get_position(tracker.user_id(), position);

    /* otherwise it wouldnt have been added */
    assert(found);
    assert((position.qty > 0) != tracker.is_bid());

    if(tracker.open_qty() > fabs(position.qty))
      this->do_replace(tracker.ptr(), fabs(position.qty) - tracker.open_qty());
  
    return false;
  }

  void should_add(const Tracker& taker, InsertRejectReasons& reason) {
    if(!taker.reduce_only()) return;

    Position position;
    bool found = this->get_position(taker.user_id(), position);

    /* if order would either increase the current position or
       open an opposite position, then reject it */
 
    if(!found || (found && (position.qty > 0) == taker.is_bid()))
      reason = reduce_only_increase;
    else if(taker.open_qty() > fabs(position.qty))
      reason = reduce_only_reverse;
    else
      reduce_only_orders_.emplace(taker.user_id(), taker.ptr());
  }

  void on_position_close(uint64_t user_id) {
    /* cancel all reduce only orders */
    for(auto it = reduce_only_orders_.find(user_id);
      it != reduce_only_orders_.end(); ++it)
    {
      if(it->first != user_id) return;
      this->do_cancel(it->second, reduce_only_close);
    }

    reduce_only_orders_.erase(user_id);
  }

private:
  std::multimap<uint64_t, OrderPtr> reduce_only_orders_;

};

}
}