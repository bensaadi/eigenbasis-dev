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

class BookPrice {
public:
  BookPrice(bool is_bid, double price) : is_bid_(is_bid), price_(price) {}

  bool matches(double rhs) const {
    if(price_ == rhs)
      return true;
    if(is_bid_)
      return rhs < price_ || price_ == 0;
    return price_ < rhs || rhs == 0;
  }

  bool operator <(double rhs) const {
    if(price_ == 0)
      return rhs != 0;
    else if(rhs == 0)
      return false;
    else if(is_bid_)
      return rhs < price_ ;
    else
      return price_ < rhs;
  }

  bool operator ==(double rhs) const {
    return price_ == rhs;
  }

  bool operator !=(double rhs) const {
    return !(price_ == rhs);
  }

  bool operator > (double rhs) const {
    return price_!= 0 && ((rhs == 0) || (is_bid_ ? (rhs > price_) : (price_ > rhs)));
  }

  bool operator <=(double rhs) const {
    return *this < rhs || *this == rhs;
  }

  bool operator >=(double rhs) const {
    return *this > rhs || *this == rhs;
  }

  bool operator <(const BookPrice & rhs) const {
    return *this < rhs.price_;
  }

  bool operator ==(const BookPrice & rhs) const {
    return *this == rhs.price_;
  }

  bool operator !=(const BookPrice & rhs) const {
    return *this != rhs.price_;
  }

  bool operator >(const BookPrice & rhs) const {
    return *this > rhs.price_;
  }

  double price() const {
    return price_;
  }

  bool is_bid() const {
    return is_bid_;
  }

  bool is_market() const {
    return price_ == 0;
  }

private:
  bool is_bid_;
  double price_;
};

inline bool operator < (double price, const BookPrice & key) {
  return key > price;
}

inline bool operator > (double price, const BookPrice & key) {
  return key < price;
}

inline bool operator == (double price, const BookPrice & key) {
  return key == price;
}

inline bool operator != (double price, const BookPrice & key) {
  return key != price;
}

inline bool operator <= (double price, const BookPrice & key) {
  return key >= price;
}

inline bool operator >= (double price, const BookPrice & key) {
  return key <= price;
}

}
