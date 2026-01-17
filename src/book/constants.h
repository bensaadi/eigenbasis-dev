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

#include <unordered_map>

namespace book {

const double EPSILON = 1e-14;
const double MIN_ORDER_QTY = 1e-6;

/* qty at which a market order will be considered filled
 if only this much or less funds is remaining  */
const double MIN_ORDER_FUNDS = 0.01;


/* tracker.tradable_qty() returns an amount that can
  exceed the funds. we round down to its nearest TRADE_QTY_INCREMENT */
const double TRADE_QTY_INCREMENT = 1e-7;

const double TAKER_FEE_RATE = 0.01;
const double MAKER_FEE_RATE = 0.005;

const size_t DEFAULT_DEPTH_SIZE = 30;

}