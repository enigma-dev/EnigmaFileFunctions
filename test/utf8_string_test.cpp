/** Copyright (C) 2014 Josh Ventura
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

#include "unit_testing.hpp"
#include <utf8_string.hpp>
#include <string>

RUN_TEST("Verify utf8::utf8_string.at() returns correct character value") {
  const utf8::utf8_string str = "\xF0\x9F\x98\x80\xF0\x9F\x98\x81\xF0\x9F\x98\x82\xF0\x9F\x98\x84\xF0\x9F\x98\x85";
  assert_equals(0x01F602, str.at(2));
  
  const utf8::utf8_string str2 = "γειά, κόσμο!";
  assert_equals(0x03BA, str2.at(6));
  assert_equals(0x03CC, str2.at(7));
  assert_equals(0x03C3, str2.at(8));
  assert_equals(0x03BC, str2.at(9));
  assert_equals(0x03BF, str2.at(10));
}

RUN_TEST("Test utf8::utf8_string::substr") {
  const utf8::utf8_string str = "γειά, κόσμο!";
  std::string sub = str.substdstr(6, 5);
  assert_equals("Substring is not accurate;", "κόσμο", sub);
  
  utf8::utf8_string str2 = "\xF0\x9F\x98\x80\xF0\x9F\x98\x81\xF0\x9F\x98\x82\xF0\x9F\x98\x84\xF0\x9F\x98\x85";
  sub = str2.substdstr(2, 2);
  assert_equals("Substring is not accurate;", "\xF0\x9F\x98\x82\xF0\x9F\x98\x84", sub);
}

RUN_TEST("Test utf8::utf8_string::length") {
  const utf8::utf8_string str = "γειά, κόσμο! \xF0\x9F\x98\x80!";
  assert_equals("Length is not accurate;", 15, str.length());
}

RUN_TEST("Test utf8::utf8_string::size") {
  const utf8::utf8_string str = "γειά, κόσμο! \xF0\x9F\x98\x80!";
  assert_equals("Size is not accurate;", 27, str.size());
}

RUN_TEST("Test utf8::utf8_string per-character assembly and disassembly using substdstr") {
  const utf8::utf8_string input = "γειά, κόσμο! \xF0\x9F\x98\x80!";
  utf8::utf8_string output;
  for (size_t i = 0; i < input.length(); ++i)
    output += input.substdstr(i, 1);
  assert_equals("String assembled from per-character substrings does not match original;", input, output);
}
