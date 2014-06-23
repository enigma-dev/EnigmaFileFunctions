/**
 * @file  gdir.hpp
 * @brief Abstract directory access.
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
 
#ifndef e_GDIR_H
#define e_GDIR_H

#include <string>

/// The ENIGMA File Functions namespace
namespace eff {
  using std::string;
  
  /* ******************************************************************************************* *\
  |* This part's a little ugly. We declare an interface, then just go ahead and write functions  *|
  |* that delegate to it. The idea is to keep the interface's workings internal; you don't have  *|
  |* to allocate or delete anything, because this directory object handles it for you.           *|
  \* ******************************************************************************************* */
  
  class directory {
    protected:
    struct directory_kernel {
      virtual string first_file() = 0;
      virtual string first_directory() = 0;
      virtual string next_file() = 0;
      virtual string next_directory() = 0;
      virtual size_t file_count() const = 0;
      virtual size_t directory_count() const = 0;
      virtual bool enter(string dname) = 0;
      virtual directory_kernel *enter_new(string dname) const = 0;
      virtual bool leave() = 0;
      virtual ~directory_kernel() {}
    } *kernel;
    size_t *krefs;
    
    /// Construct from a kernel; this is only available to our children
    inline directory(directory_kernel* k): kernel(k), krefs(new size_t(1)) {}
    inline static directory ctor(directory_kernel* k) { return k; }
    
    inline void unref() {
      if (!--*krefs) {
        delete kernel;
        delete krefs;
      }
    }
    
    public:
      
      /// Construct from another directory, maintaining reference count
      inline directory(const directory &d):
          kernel(d.kernel), krefs(d.krefs) {
        ++*d.krefs;
      }
      
      inline string first_file()      { return kernel->first_file(); }
      inline string first_directory() { return kernel->first_directory(); }
      inline string next_file()       { return kernel->next_file(); }
      inline string next_directory()  { return kernel->next_directory(); }
      inline size_t file_count()      { return kernel->file_count(); }
      inline size_t directory_count() { return kernel->directory_count(); }
      
      /// Enter the subdirectory with the given name.
      /// @return Returns true if successful, false otherwise.
      inline bool enter(string dname) { return kernel->enter(dname); }
      
      inline bool is_open() const { return kernel; }
      inline bool good()    const { return kernel; }
      
      inline directory enter_new(string dname) { return kernel->enter_new(dname); }
      
      inline bool leave() { return kernel->leave(); }
      inline ~directory() { unref(); }
      
      inline directory& operator= (const directory& dir) {
        if (kernel)
          unref();
        ++*dir.krefs;
        kernel = dir.kernel;
        krefs = dir.krefs;
        return *this;
      }
  };

  directory dirent_zip(string zipfile);
  directory dirent(string dname);
}

#endif
