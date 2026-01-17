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

namespace book {
namespace plugins {

template <class OrderPtr>
struct PostOnlyTracker {
  PostOnlyTracker(const OrderPtr& order) :
    post_only_(order->post_only()) {}
  
  bool post_only() const { return post_only_; };

  private:
    bool post_only_;
};

struct PostOnlyOrder {
  virtual bool post_only() const = 0;
};

template <class Tracker>
class PostOnlyPlugin :
public Plugin<Tracker>
{
protected:
  typedef typename Tracker::OrderPtr OrderPtr;

  void should_trade(
    Tracker& taker,
    Tracker& maker,
    CancelReasons& taker_reason,
    CancelReasons& maker_reason)
  {
    /* only check taker  */
    if(taker.post_only()) taker_reason = CancelReasons::post_only;
  }
};

}
}