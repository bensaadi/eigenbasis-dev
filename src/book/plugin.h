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

#include <vector>
#include <map>
#include <book/callback.h>
#include <book/tracker.h>
#include <book/book_price.h>

namespace book {

template <class Tracker>
class Plugin {
protected:
  using OrderPtr = typename Tracker::OrderPtr;
  typedef std::multimap<BookPrice, Tracker> TrackerMap;
  typedef std::vector<Tracker> TrackerVec;
  typedef Callback<OrderPtr> TypedCallback;

  virtual std::vector<TypedCallback>& callbacks() = 0;
  virtual void emit_callback(const TypedCallback& callback) = 0;
  virtual void emit_cancel_callback(
    const Tracker& tracker, CancelReasons reason) = 0;

  virtual void cancel(const OrderPtr& order, CancelReasons reason) = 0;
  virtual void do_cancel(const OrderPtr& order, CancelReasons reason) = 0;
  virtual void do_replace(const OrderPtr& order, double delta) = 0;
  virtual bool add_tracker(Tracker& taker) = 0;
  virtual bool add(const OrderPtr& order) = 0;
  virtual double market_price() const = 0;

  virtual void process_callbacks() = 0;
  virtual uint32_t symbol_id() const = 0;

  virtual const TrackerMap& bids() const = 0;
  virtual const TrackerMap& asks() const = 0;

  virtual void should_add(
    const Tracker& taker,
    InsertRejectReasons& reason) { }

  virtual bool should_add_tracker(
    const Tracker& taker) { return true; }

  virtual void after_add_tracker(
    const Tracker& taker) {}

  virtual void should_trade(
    Tracker& taker,
    Tracker& maker,
    CancelReasons& taker_reason,
    CancelReasons& maker_reason) { }

  virtual void after_trade(
    Tracker& taker,
    Tracker& maker,
    bool maker_is_bid,
    double qty,
    double price) {}

  virtual void on_market_price_change(
    double prev_price,
    double new_price) {}

};

}