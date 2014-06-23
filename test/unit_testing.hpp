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

#ifndef __TESTING_HPP__
#define __TESTING_HPP__

#include <deque>
#include <string>
#include <sstream>
#include <utility>
#include <exception>

#define concatenate_name(x,y) x ## y
#define make_function_name(line) concatenate_name(unit_test_, line)
#define make_registrar_name(line) concatenate_name(register_unit_test_, line)
#define RUN_TEST(name) \
  static void make_function_name(__LINE__) (); \
  static unit_test_registrar make_registrar_name(__LINE__) (name, make_function_name(__LINE__)); \
  static void make_function_name(__LINE__) ()

struct unit_test_registrar {
  typedef void(*test_function)();
  typedef std::pair<std::string, test_function> test_pair;
  typedef std::deque<test_pair> test_collection; 
  static test_collection &all_tests();
  unit_test_registrar(std::string name, void(*function)()) {
    all_tests().push_back(test_pair(name, function));
  }
};

struct assertion_failure: std::exception {
  std::string message;
  const char* what() const throw() { return message.c_str(); }
  assertion_failure(std::string file, int line, std::string exc) {
    std::stringstream ext;
    ext << file << ":" << line << ": " << exc;
    message = ext.str();
  }
};

template<class type1, class type2>
void assert_equals(std::string message, type1 x, type2 y, std::string file, int line) {
  if (!(x == y)) {
    std::stringstream ext;
    ext << message << " Expected <[" << x << "]> but was <[" << y << "]>" << std::endl;
    throw assertion_failure(file, line, ext.str());
  }
}
template<class type1, class type2>
void assert_equals(type1 x, type2 y, std::string file, int line) {
  assert_equals("", x, y, file, line);
}

static inline void assert_true(std::string message, bool exp, std::string file, int line) {
  if (!exp)
    throw assertion_failure(file, line, message + " Expression was expected to be true but was false");
}
static inline void assert_true(bool exp, std::string file, int line) {
  if (!exp) throw assertion_failure(file, line, "Expression was expected to be true but was false");
}

static inline void assert_false(std::string message, bool exp, std::string file, int line) {
  if (exp)
    throw assertion_failure(file, line, message + " Expression was expected to be false but was true");
}
static inline void assert_false(bool exp, std::string file, int line) {
  if (exp) throw assertion_failure(file, line, "Expression was expected to be false but was true");
}

#define assert_equals(x...) assert_equals(x, __FILE__, __LINE__)
#define assert_true(x...)   assert_true  (x, __FILE__, __LINE__)
#define assert_false(x...)  assert_false (x, __FILE__, __LINE__)

#endif
