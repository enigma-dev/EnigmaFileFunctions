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

#include <map>
#include <string>
#include <iostream>

#include "unit_testing.hpp"

unit_test_registrar::test_collection &unit_test_registrar::all_tests() {
  static test_collection *all_tests_ = new test_collection();
  return *all_tests_;
}

static void run_test(const std::pair<std::string, void(*)()> &test) {
  std::cout << test.first << "... " << std::flush;
  test.second();
  std::cout << "PASS" << std::endl;
}

template<class some>
void failed(some reason) {
  std::cout << "FAIL" << std::endl << "  " << reason << std::endl;
}

int main() {
  bool had_failures = false;
  for (auto test_pair : unit_test_registrar::all_tests()) {
    try { run_test(test_pair); }
    catch (std::exception &e)   { had_failures = true; failed(e.what()); }
    catch (std::string reason)  { had_failures = true; failed(reason); }
    catch (const char* reason)  { had_failures = true; failed(reason); }
    catch (...)                 { had_failures = true; failed("Reason not understood"); }
  }
  delete &unit_test_registrar::all_tests();
  return had_failures;
}
