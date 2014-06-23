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

#include <set>
#include <string>
#include "unit_testing.hpp"

#include <gdir.hpp>

using std::set;
using std::string;

#define dir_count 3

const char* dirs_root[dir_count] = {
  "alpha", "beta", "gamma"
};

const char* files_alpha[] = { "apple.txt", NULL };
const char* files_beta [] = { "banana.txt", "blueberry.txt", NULL };
const char* files_gamma[] = { "grape.txt", NULL };

const char** dir_files[dir_count] = {
  files_alpha, files_beta, files_gamma
};

static void test_file_structure(eff::directory &dir) {
  set<string> all_dirs, read_dirs, all_files, read_files;
  for (string dirname : dirs_root)
    all_dirs.insert(dirname);
  
  assert_equals("Directory count mismatch;", all_dirs.size(), dir.directory_count());
  assert_equals("File count error;", 0, dir.file_count());
  
  assert_equals("As there are no files in the directory, first_file should return empty;", "", dir.first_file());
  for (string dirname = dir.first_directory(); !dirname.empty(); dirname = dir.next_directory()) {
    assert_true("Returned directory names should be in our set; `" + dirname + "' is not", all_dirs.find(dirname) != all_dirs.end());
    read_dirs.insert(dirname);
  }
  assert_equals("Not all expected directories were read;", all_dirs.size(), read_dirs.size());
  
  for (size_t i = 0; i < dir_count; ++i) {
    string dirname = dirs_root[i];
    assert_true("Entering directory `" + dirname + "' should have returned success;", dir.enter(dirname));
    
    all_files.clear();
    read_files.clear();
    for (size_t j = 0; dir_files[i][j]; ++j)
      all_files.insert(dir_files[i][j]);
    
    assert_equals("File count for `" + dirname + "' incorrect;", all_files.size(), dir.file_count());
    for (string fname = dir.first_file(); !fname.empty(); fname = dir.next_file()) {
      assert_true("Unexpected file `" + fname + "' encountered in `" + dirname + "';", all_files.find(fname) != all_files.end());
      read_files.insert(fname);
    }
    assert_equals("Not all expected files were read;", all_files.size(), read_files.size());
    assert_true("Leaving the freshly-entered directory must return success;", dir.leave());
  }
  
  for (size_t i = 0; i < dir_count; ++i) {
    string dirname = dirs_root[i];
    eff::directory ndir = dir.enter_new(dirname);
    assert_true("Entering directory `" + dirname + "' should have returned a good directory;", ndir.good());
    
    all_files.clear();
    read_files.clear();
    for (size_t j = 0; dir_files[i][j]; ++j)
      all_files.insert(dir_files[i][j]);
    
    assert_equals("File count for `" + dirname + "' incorrect;", all_files.size(), ndir.file_count());
    for (string fname = ndir.first_file(); !fname.empty(); fname = ndir.next_file()) {
      assert_true("Unexpected file `" + fname + "' encountered in `" + dirname + "';", all_files.find(fname) != all_files.end());
      read_files.insert(fname);
    }
    assert_equals("Not all expected files were read;", all_files.size(), read_files.size());
    // TODO: This is inconsistent between APIs. Works for zip, fails for directory.
    // assert_false("Leaving this newly-allocated directory doesn't make sense;", ndir.leave());
  }
}

RUN_TEST("Verify directory iteration works as expected") {
  eff::directory dir = eff::dirent("data/testfolder");
  assert_true("Couldn't open directory for iteration", dir.is_open());
  test_file_structure(dir);
}

RUN_TEST("Verify zip file iteration works as expected") {
  eff::directory dir = eff::dirent_zip("data/testfolder.zip");
  assert_true("Couldn't open directory for iteration", dir.is_open());
  test_file_structure(dir);
}
