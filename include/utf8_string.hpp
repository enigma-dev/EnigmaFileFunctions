/**
 * @file  utf8_string.hpp
 * @brief A UTF-8 implementation of std::string, atop std::string.
 *
 * Declares an interface giving methods for listing, opening, and creating
 * files and directories.
 *
 * @section License
 * Copyright (C) 2014 Josh Ventura
 * This file is part of ENIGMA.
 * 
 * ENIGMA is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License, or (at your option) any later version.
 * 
 * ENIGMA is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * ENIGMA. If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef e_UTF8_STRING_H
#define e_UTF8_STRING_H

#include <string>
#include <iostream>
#include <stdexcept>

#define UTF8S_NOEXCEPT
#define UTF8S_CPP11 0


namespace utf8 {

template<size_t sz> struct po2log2;
template<> struct po2log2<0> { enum { v = 0 }; };
template<> struct po2log2<1> { enum { v = 0 }; };
template<size_t sz> struct po2log2 {
  enum {
    v = sz < 2? 0 : po2log2<((sz + 1) >> 1)>::v + 1
  };
};

inline bool utf8_is_fragment(char c) {
  return (c & 0xC0) == 0x80;
}

class utf8_string {
  std::string data;
  std::basic_string<size_t> nthcharat;
  size_t utf8length;
  
  enum {
    SHIFTBY     = po2log2<sizeof(size_t)>::v,
    CHARSPERIND = 1 << SHIFTBY,
    LOSTBITS    = CHARSPERIND - 1
  };
  
  static inline char maskfor(char c) {
    return 
        "\x1F\x1F\x1F\x1F\x1F\x1F\x1F\x1F" "\x1F\x1F\x1F\x1F\x1F\x1F\x1F\x1F"
        "\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F" "\x07\x07\x07\x07\x03\x03\x01"
        [(c & 0x3E) >> 1]; // 0x3E = 0b00111110
  }
  static inline int length_of(char c) {
    return
        "\2\2\2\2\2\2\2\2" "\2\2\2\2\2\2\2\2"
        "\3\3\3\3\3\3\3\3" "\4\4\4\4\5\5\6\1"
        [(c & 0x3E) >> 1]; // 0x3E = 0b00111110
  }
  
  inline size_t byte_of_unsafe(size_t n) const {
    size_t closest = n & ~LOSTBITS;
    size_t bat = nthcharat[n >> SHIFTBY];
    while (closest < n) {
      ++closest;
      while (++bat < data.length() && utf8_is_fragment(data.at(bat)));
    }
    return bat;
  }
  
  inline void build_index(size_t from = 0) {
    const size_t sz = data.length();
    nthcharat.reserve((sz >> SHIFTBY) + 1);
    utf8length = from;
    if (from)
      nthcharat.resize(from >> SHIFTBY);
    for (size_t i = from? byte_of_unsafe(from) : 0; i < sz; ) {
      if (!(utf8length & LOSTBITS))
        nthcharat.append(1, i);
      while (++i < sz && utf8_is_fragment(data.at(i)));
      ++utf8length;
    }
  }
  
  inline int utf8char_at_byte(size_t n, char c, int len) const {
    int accum = c & maskfor(c); // Pull the first bits.
    for (int lenof = len; lenof > 1; --lenof) {
      ++n;
      if (n > data.length())
        throw "truncated char";
      c = data.at(n);
      if ((c & 0xC0) != 0x80)
        throw "lrn2mask";
      accum = (accum << 6) | (c & 0x3F); // 0x3F = 0b00111111
    }
    return accum;
  }
  
  
  inline int char_and_length_at_byte(size_t n, int &length_out) const {
    char c = data.at(n);
    if (c & 0x80) { // If the high bit isn't set, this is an ASCII char.
      if (!(c & 0x40)) // If the second bit is zero, this is supposed to be
        throw "lrn2utf8"; // part of another character. Throw.
      return utf8char_at_byte(n, c, length_out = length_of(c));
    }
    length_out = 1;
    return c;
  }
  
  inline int char_at_byte(size_t n) const {
    int ignore;
    return char_and_length_at_byte(n, ignore);
  }
  
  inline int length_at_byte(size_t n) const {
    int len;
    char_and_length_at_byte(n, len);
    return len;
  }
  
  inline void shrink_to(size_t n) {
    size_t bl = byte_of_unsafe(n);
    data.resize(bl);
    nthcharat.resize((n + LOSTBITS) >> SHIFTBY);
  }
  
  
public:
  
  size_t size()            const UTF8S_NOEXCEPT { return data.size(); }
  size_t max_size()        const UTF8S_NOEXCEPT { return data.max_size(); }
  size_t capacity()        const UTF8S_NOEXCEPT { return data.capacity(); }
  bool empty()             const UTF8S_NOEXCEPT { return data.empty(); }
  const char *c_str()      const UTF8S_NOEXCEPT { return data.c_str(); }
  const std::string &str() const UTF8S_NOEXCEPT { return data; }
  
  bool operator==(const utf8_string &x) const { return data == x.data; }
  bool operator!=(const utf8_string &x) const { return data == x.data; }
  
  void reserve(size_t n = 0) {
    data.reserve(n);
    nthcharat.reserve(n >> SHIFTBY);
  }
  
  #if UTF8S_CPP11
  void shrink_to_fit() {
    data.shrink_to_fit();
    nthcharat.shrink_to_fit();
  }
  #endif
  
  size_t length() const UTF8S_NOEXCEPT {
    return utf8length;
  }
  void resize(size_t n) {
    if (n > utf8length) {
      size_t szo = data.size();
      data.resize(szo + n - utf8length);
      size_t leno = utf8length;
      for (utf8length = n; leno < utf8length; ++szo) {
        if (!(leno++ & LOSTBITS))
          nthcharat.append(1, szo);
      }
    }
    else if (n < utf8length)
      shrink_to(n);
  }
  void resize(size_t n, int c) {
    
  }
  
  utf8_string(const char* x): data(x), nthcharat() {
    build_index();
  }
  utf8_string(const std::string &x): data(x), nthcharat() {
    build_index();
  }
  utf8_string(): data(), nthcharat(), utf8length(0) {
  }
  
  size_t byte_of(size_t n) const {
    if (n < 0 || n > utf8length)
      throw "a fit";
    return byte_of_unsafe(n);
  }
  
  int at(size_t n) const {
    if (n < 0 || n > utf8length)
      throw "a fit";
    return char_at_byte(byte_of_unsafe(n));
  }
  
  inline int operator[](size_t n) {
    return at(n);
  }
  
  std::basic_string<size_t> debug() {
    return nthcharat;
  }
  
  std::string substdstr(size_t pos, size_t len = std::string::npos) const {
    if (pos > utf8length)
      throw std::range_error("utf8::string::substdstr(): index out of bounds");
    if (len == std::string::npos)
      return data.substr(byte_of_unsafe(pos));
    size_t last = pos + len - 1;
    if (last > utf8length)
      throw std::range_error("utf8::string::substdstr(): length out of bounds");
    size_t from = byte_of_unsafe(pos), to = byte_of_unsafe(last);
    return data.substr(from, to + length_at_byte(to) - from);
  }
  
  utf8_string &operator+=(const utf8_string& app) {
    data.operator+=((std::string)app);
    if ((utf8length & LOSTBITS) == LOSTBITS) {
      nthcharat += app.nthcharat;
      return *this;
    }
    build_index(utf8length);
    return *this;
  }
  
  operator const std::string&() const { return data; }
  operator std::string() { return data; }
};

static std::ostream &operator<<(std::ostream &os, const utf8_string &s) { 
    return os << s.str();
}

}

#endif
