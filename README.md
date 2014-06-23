# ENIGMA File Functions

This is a library for abstract file interface. It includes a unicode string class, a directory and archive iteration interface, and a data storage abstraction layer for object serialization.

As of yet, it is incomplete.

### Currently implemented:
* **utf8::utf8_string**: An implementation of std::string for UTF-8 strings.
 * `length()`: Returns the length, in unicode characters, of this string.
 * `size()`: Returns the size, in bytes, of this string.
 * `substdstr()`: Returns an std::string between the given indices.
 * `at()`: Returns the unicode value of the character at the given index.
 * `operator[]`: Returns the unicode value of the character at the given index.
* **eff::directory**: A common interface for reading zip files and directories.
 * `first_file()`/`next_file()`: Retrieve successive filenames, or empty string if no more.
 * `first_directory()`/`next_directory()`: Retrieve successive directories, or empty string if no more.
 * `file_count()`/`directory_count()`: Retrieve the number of files/directories contained.
 * `enter()`: Enter a subdirectory by its name.
 * `enter_new()`: Enter a subdirectory by its name, returning a new directory object.
 * `leave()`: Leave a previously entered subdirectory.
 * `good()`/`is_open()`: Return whether this directory was successfully opened.

### To be done:
* **utf8::utf8_string**
 * `substr()`: Should return the utf8::string between the given indices.
 * `operator[]`: Should allow an (expensive) assignment to a character
* **eff::directory**
 * Needs coding and testing for Windows
 * Needs methods to actually open files for read/write
 * Needs methods to get and set file attributes/permissions
* **Data storage model**
 * A common mechanism for reading and writing various types of data.
