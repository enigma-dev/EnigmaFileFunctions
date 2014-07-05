#include "gdir.hpp"
#include <iostream>
using namespace std;

static void print_tree(eff::directory dir, string indent = "") {
  size_t lastfile = dir.file_count() - 1, lastdir = lastfile == size_t(-1)? dir.directory_count() - 1 : size_t(-1);
  size_t cur = 0;
  for (string dn = dir.first_directory(); !dn.empty(); dn = dir.next_directory()) {
    bool lastent = (cur++ == lastdir);
    cout << indent + (lastent? "╰ " : "├ ") << dn << endl;
    eff::directory d = dir.enter_new(dn);
    if (d.good())
      print_tree(d, indent + (lastent? "  " : "│ "));
    else
      cout << "Could not enter" << endl;
  }
  cur = 0;
  for (string fn = dir.first_file(); !fn.empty(); fn = dir.next_file()) {
    bool lastent = (cur++ == lastfile);
    cout << indent << (lastent? "╰ " : "├ ") << fn << endl;
  }
}


#include "utf8_string.hpp"
using utf8::utf8_string;

static inline void test_utf8_string() {
  try {
    utf8_string mystr = "hi there and γειά, κόσμο, too \360\237\230\201\360\237\230\201\360\237\230\201 :) YES";
    for (size_t i = 0; i < mystr.debug().length(); ++i)
      cout << mystr.debug()[i] << ", "; cout << "end" << endl;
    
    cout << "\"" << mystr.c_str() << "\".length = " << mystr.length() << endl;
    cout << "ans[37]: " << mystr[37] << endl;
    
    for (size_t i = 0; i < mystr.length(); ++i)
      cout << "mystr[" << i << "] = " << mystr[i] << " (", wcout << (wchar_t)mystr[i], cout << "; byte " << mystr.byte_of(i) << ")" << endl;
    
    mystr += " cool beans, mister";
    cout << mystr.c_str() << endl;
    cout << mystr.substdstr(49) << endl;
  }
  catch (const char* x) {
    cout << "Fail: " << x << endl;
  }
  catch (...) { cerr << "Some serious bullshit happened" << endl; }
}


int main() {
  cout << "Testing directory read" << endl;
  eff::directory dir = eff::dirent_zip("/home/josh/Desktop/coolio.zip");
  if (dir.is_open()) {
    cout << "Read directory" << endl;
    print_tree(dir, "");
  }
  else cout << "Failure." << endl;
  
  cout << "Testing directory read" << endl;
  dir = eff::dirent("/home/josh/Desktop/");
  if (dir.is_open()) {
    cout << "Read directory" << endl;
    print_tree(dir, "");
  }
  else cout << "Failure." << endl;
  
  cout << endl;
  string tus; char tusb[256]; *tusb = 0;
  cout << "Test UTF-8 Strings? [y/N]: "; cin.getline(tusb, 256);
  tus = tusb;
  if (tus.length() && (tus[0] == 'Y' || tus[0] == 'y'))
    test_utf8_string();
  
  return 0;
}
