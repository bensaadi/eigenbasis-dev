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

#include <book/plugin.h>
#include <book/exceptions.h>
#include <book/types.h>
#include <book/plugins/trackers/user_id_tracker.h>

namespace book {
namespace plugins {

enum SelfTradePolicy : uint8_t {
  stp_cancel_taker = 1,
  stp_cancel_maker = 2,
  stp_cancel_both = 3
};

template <class OrderPtr>
struct SelfTradePolicyTracker :
public virtual UserIDTracker<OrderPtr>
{
  SelfTradePolicyTracker(const OrderPtr& order) : stp_(order->stp()) {
    UserIDTracker<OrderPtr>::set_user_id(order->user_id());
  }

  SelfTradePolicy stp() const {
    return stp_;
  }

private:
  SelfTradePolicy stp_;
};


struct SelfTradePolicyOrder {
  virtual uint32_t user_id() const = 0;
  virtual SelfTradePolicy stp() const = 0;
};

template <class Tracker>
class SelfTradePolicyPlugin :
public Plugin<Tracker>
{
protected:
  typedef typename Tracker::OrderPtr OrderPtr;
  typedef Tracker PluginTracker;

  void should_trade(
    Tracker& taker,
    Tracker& maker,
    CancelReasons& taker_reason,
    CancelReasons& maker_reason)
  {
    if(taker.user_id() != maker.user_id()) return;

    SelfTradePolicy combined_stp =
      (SelfTradePolicy)(taker.stp() | maker.stp());

    if(combined_stp & stp_cancel_taker) taker_reason = self_trade;
    if(combined_stp & stp_cancel_maker) maker_reason = self_trade;
  }
};

}
}