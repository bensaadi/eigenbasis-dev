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

namespace book {

enum InsertRejectReasons : uint8_t {
  dont_reject,
  insert_reject_no_reason,
  reduce_only_increase,
  reduce_only_reverse,
  insufficient_funds,
  qty_too_small,
  funds_too_small,
  duplicate_client_order_id
};

enum CancelRejectReasons : uint8_t {
  dont_cancel_reject,
  cancel_reject_not_found
};

enum ReplaceRejectReasons : uint8_t {
  dont_replace_reject,
  replace_reject_not_found,
  replace_reject_no_qty,
  replace_insufficient_funds
};

enum CancelReasons : uint8_t {
  dont_cancel,
  user_cancel,
  temporary_cancel,
  no_liquidity,
  self_trade,
  engine_shutdown,
  replaced_all_qty,
  post_only,
  reduce_only_match,
  reduce_only_close,
  mm_routed,
  routing_failure,  
};


}