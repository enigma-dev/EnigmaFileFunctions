/**
 * @file  gdir.h
 * @brief Generic Directory access.
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
 
#include "gdir.hpp"
#include <zip.h>
#include <vector>
#include <deque>
#include <map>

using std::deque;
using std::vector;
using std::map;

// Determine whether we're on a Windows or POSIX machine.
// It is assumed that everything that isn't Windows is POSIX-compliant or else not going to run this.
#if !defined(EFF_WINDOWS) && !defined(EFF_POSIX)
#  if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__WIN64__)
#    define EFF_WINDOWS
#  else
#    define EFF_POSIX
#  endif
#endif

#ifdef EFF_WINDOWS
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <dirent.h>
#  include <unistd.h>
#endif // EFF_WINDOWS

namespace eff {
  
  /* ******************************************************************************************* *\
  |* Internal structure to represent a hierarchy when there isn't one, or there's no API for it. *|
  \* ******************************************************************************************* */
  
  struct parsed_directory {
    typedef map<string, parsed_directory> dirmap;
    typedef map<string, size_t> filemap;
    typedef dirmap::iterator dirit;
    typedef filemap::iterator fileit;
    
    parsed_directory *parent;
    dirmap subdirs;
    filemap files;
    
    parsed_directory(parsed_directory *p = NULL): parent(p), subdirs(), files() {}
    parsed_directory(const parsed_directory &d): parent(NULL), subdirs(d.subdirs), files(d.files) {
      if (d.parent != NULL)
        throw "new shitHappensException()";
    }
    parsed_directory& operator=(const parsed_directory&) { return *this; }
    
    void add_file(const char* path, size_t index, parsed_directory *p) {
      if (!parent)
        parent = p;
      const char *s;
      for (s = path; *s && *s != '/'; ++s);
      if (*s == '/') {
        if (s == path) return add_file(s + 1, index, parent);
        return subdirs[string(path, s)].add_file(s + 1, index, this);
      }
      if (s > path)
      files[string(path, s)] = index;
    }
  };
  
  /* ******************************************************************************************* *\
  |* Filesystem directory traversal. Platform-specific, but otherwise self-contained. ********** *|
  \* ******************************************************************************************* */
  
  struct directory_filesystem: public eff::directory {
    struct kernel_filesystem: directory_kernel {
      typedef deque<string> filelist;
      
      class whole_directory {
        whole_directory* parent;
        size_t refs;
        
        inline void unref_parent() {
          if (parent && !--parent->refs)
            delete parent;
        }
        
        ~whole_directory() {
          unref_parent();
        }
        
        public:
        string path;
        
        filelist files;
        filelist dirs;
        
        whole_directory(const whole_directory& d): parent(d.parent), refs(1), path(d.path), files(d.files), dirs(d.dirs) {
          if (d.parent)
            ++ d.parent->refs;
        }
        
        whole_directory& operator=(const whole_directory& d) {
          path = d.path;
          files = d.files;
          dirs = d.dirs;
          return *this;
        }
        
        inline void set_parent(whole_directory *new_parent) {
          unref_parent();
          parent = new_parent;
        }
        inline whole_directory *get_parent() const {
          return parent;
        }
        
        static void unref(whole_directory* ref) {
          if (!--ref->refs)
            delete ref;
        }
        
        
#     ifdef EFF_WINDOWS
#         define PATH_CHAR "\\"
          
          static inline bool is_directory(HANDLE file) {
            // TODO: write
            return false;
          }
          
          static inline whole_directory* cache(string dirname) {
            // TODO: write
            return NULL;
          }
          
        private:
          whole_directory() {
            // TODO: write
          }
#       else
#         define PATH_CHAR "/"
          
          static inline bool is_directory(::dirent *file) {
#           ifdef _DIRENT_HAVE_D_TYPE // Not standard POSIX; ask GLIBC if this system supports file->d_type
              return file->d_type == DT_DIR;
#           else // In the event that it does not, we have to call stat...
              struct stat sb;
              stat(file->d_name, &sb);
              return sb.st_mode & S_IFDIR;
#           endif
          }
          
          static inline whole_directory* cache(whole_directory* parent, string dirname) {
            DIR* dir_open = ::opendir(dirname.c_str());
            if (!dir_open) return NULL;
            whole_directory *res = new whole_directory(parent, dirname, dir_open);
            closedir(dir_open);
            return res;
          }
          
        private:
          whole_directory(whole_directory *prnt, string dirname, DIR* dir): parent(prnt), refs(1), path(dirname), files(), dirs() {
            if (parent)
              parent->refs++;
            for (::dirent* rd; (rd = readdir(dir)); ) {
              if (is_directory(rd)) {
                string name = rd->d_name;
                if (name != "." and name != "..")
                  dirs.push_back(name);
              }
              else files.push_back(rd->d_name);
            }
          }
#       endif
      };
      
      whole_directory *current_root;
      filelist::iterator curfile;
      filelist::iterator curdir;
      
      
      virtual string first_file() {
        curfile = current_root->files.begin();
        return next_file();
      }
      virtual string first_directory() {
        curdir = current_root->dirs.begin();
        return next_directory();
      }
      
      virtual string next_file() {
        if (curfile == current_root->files.end())
          return "";
        return *curfile++;
      }
      
      virtual string next_directory() {
        if (curdir == current_root->dirs.end())
          return "";
        return *curdir++;
      }
      
      virtual size_t file_count() const { return current_root->files.size(); }
      virtual size_t directory_count() const { return current_root->dirs.size(); }
      
      virtual bool enter(string dname) {
        whole_directory* newroot = whole_directory::cache(current_root, current_root->path + PATH_CHAR + dname);
        if (!newroot) return false;
        current_root = newroot;
        return true;
      }
      virtual directory_kernel *enter_new(string dname) const {
        whole_directory* root = whole_directory::cache(current_root, current_root->path + PATH_CHAR + dname);
        return root? new kernel_filesystem(root) : NULL;
      }
      virtual bool leave() {
        if (!current_root->get_parent())
          return false;
        whole_directory *old_root = current_root;
        current_root = current_root->get_parent();
        whole_directory::unref(old_root);
        return true;
      }
      ;
      
      static directory_kernel *enter_directory(string dname) {
        whole_directory* root = whole_directory::cache(NULL, dname);
        return root? new kernel_filesystem(root) : NULL;
      }
      
      ~kernel_filesystem() { whole_directory::unref(current_root); }
      kernel_filesystem(whole_directory* dir): current_root(dir), curfile(), curdir() {}
      
      private:
        kernel_filesystem(const kernel_filesystem&);
        kernel_filesystem& operator=(const kernel_filesystem&);
    };
    
    static inline directory enter(string dir) {
      return ctor(kernel_filesystem::enter_directory(dir));
    }
  };
  
  /* ******************************************************************************************* *\
  |* Zip file iteration: Call enter() on a zip file. ******************************************* *|
  \* ******************************************************************************************* */
  
  struct directory_zip: public eff::directory {
    struct kernel_zip: directory_kernel {
      zip *zfile;
      size_t *refs;
      parsed_directory *tree;
      parsed_directory *curdir;
      parsed_directory::fileit file_at;
      parsed_directory::dirit dir_at;
      
      virtual string first_file() {
        file_at = curdir->files.begin();
        return next_file();
      }
      virtual string first_directory() {
        dir_at = curdir->subdirs.begin();
        return next_directory();
      }
      virtual string next_file() {
        if (file_at == curdir->files.end())
          return "";
        return (file_at++)->first;
      }
      virtual string next_directory() {
        if (dir_at == curdir->subdirs.end())
          return "";
        return (dir_at++)->first;
      }
      
      virtual size_t file_count() const { return curdir->files.size(); }
      virtual size_t directory_count() const { return curdir->subdirs.size(); }
      
      virtual bool enter(string dname) {
        parsed_directory::dirit i = curdir->subdirs.find(dname);
        if (i == curdir->subdirs.end()) return false;
        curdir = &i->second;
        return true;
      }
      virtual directory_kernel *enter_new(string dname) const {
        parsed_directory::dirit i = curdir->subdirs.find(dname);
        if (i == curdir->subdirs.end()) return NULL;
        kernel_zip *res = new kernel_zip(this);
        res->curdir = &i->second;
        return res;
      }
      virtual bool leave() {
        if (!curdir->parent) return false;
        curdir = curdir->parent;
        return true;
      }
      
      kernel_zip(zip *zf): zfile(zf), refs(new size_t(1)), tree(new parsed_directory), curdir(tree), file_at(), dir_at() {
        vector<string> filepaths(zip_get_num_entries(zf, 0));
        for (size_t i = 0; i < filepaths.size(); ++i) {
          const char *fn = zip_get_name(zfile, i, 0);
          if (fn) filepaths[i] = fn;
          tree->add_file(fn, i, NULL);
        }
      }
      ~kernel_zip() {
        if (!--*refs) {
          zip_close(zfile);
          delete tree;
          delete refs;
        }
      }
      
      kernel_zip(const kernel_zip* dz): zfile(dz->zfile), refs(dz->refs), tree(dz->tree),
          curdir(dz->curdir), file_at(dz->file_at), dir_at(dz->dir_at) {
        ++*refs;
      }
      
      private:
        kernel_zip(const kernel_zip&);
        kernel_zip& operator=(const kernel_zip&);
    };
    
    directory_zip(): directory(NULL) {}
    
    static inline directory enter(string zipfile) {
      zip *zf = zip_open(zipfile.c_str(), ZIP_CHECKCONS, 0);
      if (!zf) return ctor(NULL);
      return ctor(new directory_zip::kernel_zip(zf));
    }
  };
  
  directory dirent_zip(string zipfile) {
    return directory_zip::enter(zipfile);
  }
  
  directory dirent(string dname) {
    return directory_filesystem::enter(dname);
  }
}
