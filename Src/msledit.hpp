#ifndef MXPSQL_MSLedit_HPP
/**
 * @file msledit.hpp
 * @author MXPSQL
 * @brief C++ 11 String Builder and Editor Library
 * @version 0.1
 * @date 2022-06-29
 * 
 * @copyright
 * 
 * Copyright (c) 2022 MXPSQL
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
 * 
 */


/**
 * @brief Include guard
 * 
 */
#define MXPSQL_MSLedit_HPP

#if (!defined(__cplusplus))
    #error This project can only be compiled as C++ code
#endif

#if defined(_MSC_VER)
    #if (_MSC_VER < 1800)
        #error This project needs atleast Visual Studio 2013
    #endif
#elif (defined(__cplusplus) && (__cplusplus <= 199711L))
    #error This project can only be compiled with a compiler that supports C++11
#endif

#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <map>

#include <initializer_list>

#include <exception>
#include <stdexcept>

#include <string>
#include <utility>
#include <locale>
#include <algorithm>
#include <functional>
#include <iterator>

#include <mutex>
#include <thread>
#include <atomic>

#include <cstdlib>
#include <cstdio>
#include <cstdint>

namespace MXPSQL{
    namespace MSLedit{

        /**
         * @brief String builder and line editor
         * 
         * @note This class has mutexes for almost all functions for thread safety (and possibly reeterant).
         * @warning Due to use of recursive_mutex for thread safety, this class may throw std::system_error, researching online shows that the mutex used here is hiding a bad design (but no, I will not replace that with wrapped functions becuase of big code). Some functions may use 0 based indexing while others use 1 based indexing, for best results experiment with this class and browse the source and make a not about it. May not be async-signal safe and cancel-safe.
         */
        class MSLedit{
            private:

            protected:
            /**
             * @brief The internal buffer
             * 
             */
            std::vector<std::string> buffer;
            /**
             * @brief The last file that is opened/writtened
             * 
             */
            std::string file = "";
            /**
             * @brief Syncrhonization mutex, you can say this is the GIL (if this is a language interpreter)
             * 
             * @note This mutex is mentioned in the note of this class, this synchronizes the class
             */
            std::recursive_mutex lock_mutex;
            /**
             * @brief Config to modify the runtime behaviour of the string builder and editor
             * 
             */
            std::map<std::string, std::string> strEditorConfig;

            public:

            /**
             * @brief The key for the prompt config
             * 
             */
            const std::string nprompt = "prompt";
            /**
             * @brief The no system config key
             * 
             */
            const std::string nosystem = "nosystem";
            /**
             * @brief Say goodbye to the nice warn greeting of the banner confik gey
             * 
             */
            const std::string nobanner = "nobanner";

            /**
             * @brief Handles other operations in the MSLedit REPL
             * 
             * @details
             * 
             * If empty, will do nothing
             * 
             * Arguments for handlers: begin, args, arglen, out, input, error
             * 
             */
            std::function<int(std::string, std::vector<std::string>, size_t, std::ostream&, std::istream&, std::ostream&)> replBeginHandler;

            /**
             * @brief Construct a new MSLedit object with initializations
             * 
             */
            MSLedit();
            /**
             * @brief Construct a new MSLedit object by copying another object
             * 
             * @param ledit the other object
             */
            MSLedit(MSLedit& ledit);
            /**
             * @brief Construct a new MSLedit object by copying another object, but dynamically allocated
             * 
             * @param ledit the other dynamically allocated object
             */
            MSLedit(MSLedit* ledit);
            /**
             * @brief Construct a new MSLedit object from a string
             * 
             * @param content the string
             */
            MSLedit(std::string content);
            /**
             * @brief Construct a new MSLedit object from a C-Style string
             * 
             * @param cstr the C-Style string
             */
            MSLedit(char* cstr);
            /**
             * @brief Construct a new MSLedit object from a constant C-Style string
             * 
             * @param ccstr the constant C-Style string
             */
            MSLedit(const char* ccstr);
            /**
             * @brief Construct a new MSLedit object from a vector of strings
             * 
             * @param buffer the vector of strings (buffer)
             */
            MSLedit(std::vector<std::string> buffer);
            /**
             * @brief Construct a new MSLedit object from an initializer list of strings
             * 
             * @param ilbuf the initializer list of strings
             */
            MSLedit(std::initializer_list<std::string> ilbuf);
            /**
             * @brief Construct a new MSLedit object from a vector of characters
             * 
             * @param cbuffer the vector of characters (buffer)
             */
            MSLedit(std::vector<char> cbuffer);
            /**
             * @brief Construct a new MSLedit object from an initializer list of characters
             * 
             * @param cilbuf the initializer list of characters
             */
            MSLedit(std::initializer_list<char> cilbuf);
            /**
             * @brief Construct a new MSLedit object from a string or a file path
             * 
             * @param fileinsteadofcontent accept as file path
             * @param obj the file path or the string
             */
            MSLedit(bool fileinsteadofcontent, std::string obj);

            /**
             * @brief Set the Instance object to another MSLedit instance
             * 
             * @details
             * 
             * Copies everything except the file
             * 
             * @param other other instance
             */
            void setInstance(MSLedit& other);
            /**
             * @brief Set the Instance object to another MSLedit instance, constant edition
             * 
             * @param cother other instance, constant edition
             */
            void setInstance(const MSLedit& cother);
            /**
             * @brief Set the Instance object, pointer edition
             * 
             * @param other other instance, pointer edition
             */
            void setInstance(MSLedit* other);

            /**
             * @brief Line counts
             * 
             * @return size_t number of line
             */
            size_t lineNums();
            /**
             * @brief Length of string
             * 
             * @return size_t the length of the string
             */
            size_t length();
            /**
             * @brief get size of string
             *
             * @return size_t the size of the string
             */
            size_t size();
            /**
             * @brief Get a string from a line
             * 
             * @param line the line
             * @return std::string the string
             */
            std::string stringAtLine(size_t line);
            /**
             * @brief Get the line number and the index of the vector grid from the string index
             * 
             * @param strindex the index of the string
             * @return std::pair<size_t, size_t> the grid index. First is the line number (starts from 1), second is the index of the line.
             * 
             * @note the arguents begins with 1 instead of 0
             * 
             * @warning default implementation (the one in the header file) is SLOW AS A SLOTH (SLOTHY) and broken.
             */
            std::pair<size_t, size_t> getGridIndexFromStringIndex(size_t strindex);
            /**
             * @brief Get a character from an index
             * 
             * @param index the index
             * @return char the character at that index
             * 
             * @note begin with 1 instead of 0
             */
            char charAtPosition(size_t index);
            /**
             * @brief Get the character from the vector of string
             * 
             * @param line the line number
             * @param index the index of the string in that line
             * @return char the character
             * 
             * @note begin with 1 instead of 0
             */
            char charAtGrid(size_t line, size_t index);

            /**
             * @brief Open a file for content
             * 
             * @param path the file path
             */
            void readFile(std::string path);
            /**
             * @brief Write to a file
             * 
             * @param path the file path
             */
            void writeFile(std::string path);
            /**
             * @brief Write to a file, locked edition (safer). Uses writeFile internally.
             * 
             * @details 
             * Copying mechanism:
             * 
             * Lock copy move unlock copying.
             * 
             * If that fails, then use the c++ fstreams and std::remove
             * 
             * If that fails too, an exception is thrown
             * 
             * @param path the file path
             * 
             * @see writeFile
             * 
             * @note Throws exception when a problem occurs (copy failure, file mismatch, etc), you can get the temporary file by splitting the message using the ';' as the delimiter (format: 'Message;TempFile').
             */
            void writeFileLocked(std::string path);
            /**
             * @brief Set the internal buffer from a string
             * 
             * @param text the text
             */ 
            void setText(std::string text);
            /**
             * @brief Get the Text object as a formatted string
             * 
             * @return std::string the string
             */
            std::string getText();
            /**
             * @brief Get the Text object as an unformatted string
             * 
             * @return std::string that big string
             */
            std::string str();
            /**
             * @brief Get the Text object
             * 
             * @param formatted should be formatted with a line numbering?
             * @param begin which line should be the starting point? -1 or below for the very beginning
             * @param end which line should be the ending point? -1 or below for the very end (all)
             * @return std::string the text
             * 
             * @note the begin and end arguments should begin with 1
             */
            std::string getText(bool formatted, long int begin, long int end);
            /**
             * @brief Set the current text to the other instance
             * 
             * @param other_miss_ledit_instance that other instance
             */
            void setTextOfInstance(MSLedit& other_miss_ledit_instance);
            /**
             * @brief Like setTextOfInstance, but swaps the content instead of copying
             * 
             * @param other_miss_ledit_instance that other instance
             * 
             * @see setTextOfInstance
             */
            void swap(MSLedit& other_miss_ledit_instance);
            /**
             * @brief Set the internal buffer from a C-Style string
             * 
             * @param cstr 
             */
            void setCString(char* cstr);
            /**
             * @brief Set the internal buffer from a constant C-Style string
             * 
             * @param ccstr 
             */
            void setCString(const char* ccstr);
            /**
             * @brief Get a C-Style string
             * 
             * @param formatted should be formatted with a line numbering?
             * @param begin which line should be the starting point? -1 or below for the very beginning
             * @param end which line should be the ending point? -1 or below for the very end (all)
             * @return char* C-Style string
             * 
             * @see getConstCString
             * 
             * @note See the note in getConstCString, this string should be deallocated when done
             */
            char* getCString(bool formatted, long int begin, long int end);
            /**
             * @brief Get a constant C-Style string
             * 
             * @param formatted should be formatted with a line numbering?
             * @param begin which line should be the starting point? -1 or below for the very beginning
             * @param end which line should be the ending point? -1 or below for the very end (all)
             * @return const char* constant C-Style string
             * 
             * @see getText
             * 
             * @note You shoudld deallocate this string when done (unique_ptr is helpful). This allocates a new string (to get around clang lmao)
             */
            const char* getConstCString(bool formatted, long int begin, long int end);
            /**
             * @brief Set the internal buffer from a vector of string
             * 
             * @param buffer the string buffer to set the internal buffer to
             */
            void setBuffer(std::vector<std::string> buffer);
            /**
             * @brief Get the internal buffer as a vector of string
             * 
             * @return std::vector<std::string> the internal buffer as a vector of string
             */
            std::vector<std::string> getBuffer();
            /**
             * @brief Set the internal buffer from a vector of characters
             * 
             * @param cbuffer the character buffer to set the internal buffer to
             */
            void setBuffer(std::vector<char> cbuffer);
            /**
             * @brief Get the internal buffer as a vector of string
             * 
             * @return std::vector<char> the internal buffer as a vector of string
             */
            std::vector<char> getCBuffer();

            /**
             * @brief Print the current text to std::cout, internally uses the ostream version
             * 
             * @param formatted should it be formatted with line numberings? (like getText())
             * @param begin the beginning line number, see getText()
             * @param end see getText()
             * 
             * @see getText
             */
            void print(bool formatted, long int begin, long int end);
            /**
             * @brief Print the current text to ostream
             * 
             * @param formatted should it be formatted with line numberings? (like getText())
             * @param stream the ostream to print to
             * @param begin the beginning line number, see getText()
             * @param end see getText()
             * 
             * @see getText
             */
            void print(bool formatted, std::ostream& stream, long int begin, long int end);
            /*
             * @brief Print the current text to stringstream, internally uses the ostream version
             * 
             * @param formatted should it be formatted with line numberings? (like getText())
             * @param stream the stringstream to print to
             * @param begin the beginning line number, see getText()
             * @param end see getText()
             * 
             * @see getText
             *\/
            void print(bool formatted, std::stringstream& stream, long int begin, long int end);*/

            /**
             * @brief Append a line at the end (newlines are handeled)
             * 
             * @param line the line
             */
            void appendAtEnd(std::string line);
            /**
             * @brief Append a line in an existing line
             * 
             * @param linenum the line number to append to
             * @param line the line
             */
            void appendAtLine(int linenum, std::string line);
            /**
             * @brief Insert before a line
             * 
             * @param linenum the line number to insert before
             * @param line the line
             */
            void insertAtLine(int linenum, std::string line);
            /**
             * @brief Search a string
             * 
             * @param text2search the search string
             * @return std::pair<size_t, size_t> the first found instance of a string, else both is std::string::npos. first is line, second is the index of the needle in that line
             */
            std::pair<size_t, size_t> search(std::string text2search);
            /**
             * @brief Search a string from a starting line
             * 
             * @param text2search the string to search
             * @param begin_line the starting line
             * @return std::pair<size_t, size_t> the first found instance of a string, else both is std::string::npos. first is line, second is the index of the needle in that line
             */
            std::pair<size_t, size_t> search(std::string text2search, size_t begin_line);
            /**
             * @brief Search for multiple instances
             * 
             * @param text2search the string to search
             * @param begin_line the starting line
             * @param counts how much instance to look for, use std::string::npos for as much as possible
             * @return std::vector<std::pair<size_t, size_t>> all instances of the searched string. Element pairs: first is line, second is the index of the needle in that line
             * 
             * @note begin with 1 instead of 0
             */
            std::vector<std::pair<size_t, size_t>> search(std::string text2search, size_t begin_line, size_t counts);
            /**
             * @brief Edit a line
             * 
             * @param linenum the line number to edit
             * @param line the line
             */
            void editLine(int linenum, std::string line);
            /**
             * @brief Edit a character
             * 
             * @param index the character index
             * @param character the character
             * 
             * @note begin with 1 for the linenum instead of 0
             */
            void editChar(int index, char character);
            /**
             * @brief Delete a line
             * 
             * @param linenum the line number to delete
             * 
             * @note See the note in editline
             * 
             * @see editLine
             */
            void deleteAtLine(int linenum);
            /**
             * @brief Clear and empty the buffer
             * 
             */
            void clear();

            /**
             * @brief Append a character
             * 
             * @param c the character
             */
            void append(char c);
            /**
             * @brief Append a C-Style string
             * 
             * @param cstr the C-Style string
             */
            void append(char* cstr);
            /**
             * @brief Append a constant C-Style string
             * 
             * @param ccstr the constant C-Style string
             */
            void append(const char* ccstr);
            /**
             * @brief Append a double
             * 
             * @param d the double
             */
            void append(double d);
            /**
             * @brief Append the floating point number
             * 
             * @param f the floating point number
             */
            void append(float f);
            /**
             * @brief Append an integer
             * 
             * @param i the integer
             */
            void append(int i);
            /**
             * @brief Append a long integer
             * 
             * @param li the long integer
             */
            void append(long int li);
            /**
             * @brief Append a size_t typedef integer
             * 
             * @param s the size_t typedef integer
             */
            void append(size_t s);
            /**
             * @brief Append a boolean
             * 
             * @param boolean the boolean
             */
            void append(bool boolean);
            /**
             * @brief Append a normal string
             * 
             * @param str the string
             */
            void append(std::string str);
            /**
             * @brief Append another instance of MSLedit
             * 
             * @param miss the other instance
             */
            void append(MSLedit miss);
            /**
             * @brief Append a pointer
             *
             * @param ptr pointer
             */
            void append(void* ptr);
            /**
             * @brief Append nullptr
             * 
             * @param nptr the "steward pid"/stewpid/stupid nullptr
             */
            void append(std::nullptr_t nptr);
            /**
             * @brief Append a long integer pointer (for NULL)
             * 
             * @param liptr long int pointer
             */
            void append(long int* liptr);
            /**
             * @brief Append a newine
             * 
             */
            void appendNewLine();

            /**
             * @brief Insert a boolean at a specified position
             * 
             * @param position the position
             * @param boolean the boolean
             */
            void insert(size_t position, bool boolean);
            /**
             * @brief Insert a C-Style string at a specified position
             * 
             * @param position the position
             * @param cstr the C-Style string
             */
            void insert(size_t position, char* cstr);
            /**
             * @brief Insert a constant C-Style string at a specified position
             * 
             * @param position the position
             * @param ccstr the constant C-Style string
             */
            void insert(size_t position, const char* ccstr);
            /**
             * @brief Insert an integer at a specified position
             * 
             * @param position the position
             * @param i integer
             */
            void insert(size_t position, int i);
            /**
             * @brief Insert a long integer at a specified position
             * 
             * @param position the position
             * @param li the long integer
             */
            void insert(size_t position, long int li);
            /**
             * @brief Insert a size_t typedef integer at a specified position
             * 
             * @param position the position
             * @param s the size_t typedef integer
             */
            void insert(size_t position, size_t s);
            /**
             * @brief Insert a string at a specified position
             * 
             * @param position the position
             * @param str the string
             */
            void insert(size_t position, std::string str);
            /**
             * @brief Insert another instance of MSLedit at a specified position
             * 
             * @param position the position
             * @param miss the other MSLedit instance
             */
            void insert(size_t position, MSLedit miss);
            /**
             * @brief Insert a pointer at a specified position
             * 
             * @param position the position
             * @param ptr the pointer
             */
            void insert(size_t position, void* ptr);
            /**
             * @brief Insert nullptr at a specified position
             * 
             * @param position the position
             * @param nptr the nullptr thing
             */
            void insert(size_t position, std::nullptr_t nptr);
            /**
             * @brief Insert a long integer pointer (for NULL) at a specified position
             * 
             * @param position the posirtion
             * @param liptr the long integer pointer
             */
            void insert(size_t position, long int* liptr);
            /**
             * @brief Insert a new line at a specified position
             * 
             * @param position the position
             */
            void insertnewline(size_t position);

            /**
             * @brief Delete a substring between 2 certain index
             * 
             * @param begin the beginning index
             * @param end the last index
             */
            void deleteAt(size_t begin, size_t end);
            /**
             * @brief Delete a character at a certain index
             * 
             * @param index the index to delete a character
             */
            void deleteCharAt(size_t index);

            /**
             * @brief Get the index of a string
             * 
             * @param str the string to search for
             * @return size_t the position of the string, std::string::npos if none
             */
            size_t indexOf(std::string str);
            /**
             * @brief Get the index of a string from a starting position
             * 
             * @param str the string to search for
             * @param begin the starting index
             * @return size_t the position of the string, std::string::npos if none
             */
            size_t indexOf(std::string str, size_t begin);

            /**
             * @brief Tokenize a string by a character
             * 
             * @param delimiter the delimiter
             * @return std::vector<std::string> the tokenized string
             */
            std::vector<std::string> split(char delimiter);
            /**
             * @brief Tokenize a string by a string
             * 
             * @param delimiter the delimiter
             * @return std::vector<std::string> the tokenized string
             */
            std::vector<std::string> split(std::string delimiter);

            /**
             * @brief Get a substring to the end
             * 
             * @param pos the beginning position of the substring
             * @return std::string the substring
             */
            std::string substring(size_t pos);
            /**
             * @brief Get a substring from the beginning to a user defined ending point
             * 
             * @param pos the beginning of the substring
             * @param end the ending point of the substring
             * @return std::string the substring
             */
            std::string substring(size_t pos, size_t end);

            /**
             * @brief Reverse a string
             * 
             */
            void reverse();

            /**
             * @brief Set the Config from a map
             * 
             * @param c2 the map
             */
            void setConfig(std::map<std::string, std::string> c2);
            /**
             * @brief Set the config from a config file
             * 
             * @details
             * 
             * The format is the following:
             * 
             * # Comment
             * Key1=Val1
             * A=Out
             * 
             * @param configPath the path to the config file
             */
            void parseConfig(std::string configPath);
            /**
             * @brief Build a config file from the config
             * 
             * @details
             * 
             * The format is the following:
             * 
             * # Comment
             * Key1=Val1
             * A=Out
             * 
             * @return std::string that config file
             */
            std::string buildConfig();
            /**
             * @brief Get the Config as a map
             * 
             * @return std::map<std::string, std::string> the config map
             */
            std::map<std::string, std::string> getConfig();
            /**
             * @brief Set the key of the config
             * 
             * @param key the config key
             * @param value the value
             */
            void setKey(std::string key, std::string value);
            /**
             * @brief Get the key of the config
             * 
             * @param key the config key
             * @return std::string the value of the key
             */
            std::string getKey(std::string key);
            /**
             * @brief Does the key exist in the config?
             * 
             * @param key the config key
             * @return true it exists
             * @return false it doesn't want to exist
             */
            bool keyExists(std::string key);

            /**
             * @brief Start the REPL using the default options
             * 
             * @details
             * 
             * Start the REPL using the default prompt from the prompt key in the config and use std::cout, std::cin, std::cerr.
             * 
             * @return int the status code of the REPL
             */
            int repl();
            /**
             * @brief Start the REPL using the default streams.
             * 
             * @details
             * 
             * Start the REPL using std::cout, std::cin, std::cerr.
             * 
             * @param prompt the prompt
             * @return int the status code of the REPL
             */
            int repl(std::string prompt);
            /**
             * @brief Start the REPL
             * 
             * @param prompt the prompt
             * @param out output stream
             * @param in input stream
             * @param err error stream
             * @return int the status code of the REPL
             * 
             * @note No readline for portability and licensing purposes and do not even suggest the idea of readline or equivalent here.
             */
            int repl(std::string prompt, std::ostream& out, std::istream& in, std::ostream& err);

            /**
             * @brief Compare this class instance to a string
             * 
             * @param mystr the string
             * @return int the important thing is that if this returns 0, equal
             */
            int compare(std::string mystr);
            /**
             * @brief Compare this class instance to another instance
             * 
             * @param miss that other stupid instance
             * @return int the important thing is that if this returns 0, equal
             */
            int compare(MSLedit& miss);
            /**
             * @brief Compare this class instance to a C-Style string
             * 
             * @param cstr the C-Style string
             * @return int the important thing is that if this returns 0, equal
             */
            int compare(char* cstr);
            /**
             * @brief Compare this class instance to a constant C-Style string
             * 
             * @param ccstr the constant C-Style string
             * @return int the important thing is that if this returns 0, equal
             */
            int compare(const char* ccstr);

            /**
             * @brief Am I empty?
             * 
             * @return true yes you are
             * @return false no you are full
             */
            operator bool() const { return !file.empty() || !buffer.empty(); }

            /**
             * @brief Am equal
             * 
             * @param miss this class
             * @param str other string
             * @return true equal
             * @return false no sad
             */
            friend bool operator==(MSLedit& miss, std::string& str) {return (miss.compare(str) == 0);}
            /**
             * @brief Am not equal
             * 
             * @param miss this class
             * @param str other string
             * @return true ye me no equal
             * @return false no sad me equal
             */
            friend bool operator!=(MSLedit& miss, std::string& str) {return !(miss == str);}
            /**
             * @brief Am equal
             * 
             * @param miss this class
             * @param miss2 other stupid bingus of an MSLedit instance
             * @return true equal
             * @return false no sad
             */
            friend bool operator==(MSLedit& miss, MSLedit& miss2) {
                return (
                    ((miss.compare(miss2)) == 0) &&
                    (miss.getConfig() == miss2.getConfig())
                );
            }
            /**
             * @brief Am not equal
             * 
             * @param miss this class
             * @param miss2 other stupid bingus of an MSLedit instance
             * @return true ye me no equal
             * @return false no sad me equal
             */
            friend bool operator!=(MSLedit& miss, MSLedit& miss2) {return !(miss == miss2);}
            /**
             * @brief Am equal
             * 
             * @param miss this class
             * @param cstr other C String
             * @return true equal
             * @return false no sad
             */
            friend bool operator==(MSLedit& miss, char* cstr) {return (miss.compare(cstr) == 0);}
            /**
             * @brief Am not equal
             * 
             * @param miss this class
             * @param cstr other C String
             * @return true ye me no equal
             * @return false no sad me equal
             */
            friend bool operator!=(MSLedit& miss, char* cstr) {return !(miss == cstr);};
            /**
             * @brief Am equal
             * 
             * @param miss this class
             * @param ccstr other frozen chunk/block of icy/ice C String
             * @return true equal
             * @return false no sad
             */
            friend bool operator==(MSLedit& miss, const char* ccstr) {return (miss.compare(ccstr) == 0);};
            /**
             * @brief Am not equal
             * 
             * @param miss this class
             * @param ccstr other frozen chunk/block of icy/ice C String
             * @return true ye me no equal
             * @return false no sad me equal
             */
            friend bool operator!=(MSLedit& miss, const char* ccstr) {return !(miss == ccstr);}

            /**
             * @brief Output the formated content of an instance to the stream
             * 
             * @param os output stream
             * @param miss the said instance
             * @return std::ostream& os
             */
            friend std::ostream& operator<<(std::ostream& os, MSLedit& miss);
            /**
             * @brief Set the content of an instance from the stream
             * 
             * @param is input stream
             * @param miss the said instance
             * @return std::istream& is
             */
            friend std::istream& operator>>(std::istream& is, MSLedit& miss);

            /**
             * @brief Get a character from an index
             * 
             * @param index the index
             * @return char the character
             * 
             * @note You cannot modify the character from here
             */
            char operator[](int index);

            /**
             * @brief No self assignment errors
             * 
             * @param miss that one
             * @return MSLedit& the other new one
             */
            MSLedit& operator=(MSLedit& miss){
                return operator=(const_cast<const MSLedit&>(miss));
            }
            /**
             * @brief Also no more self assignment errors
             * 
             * @param cmiss that constant one
             * @return MSLedit& the other new one
             */
            MSLedit& operator=(const MSLedit& cmiss){
                if(this != &cmiss) {
                    setInstance(cmiss);
                }

                return *this;
            }
            /**
             * @brief Also no more self assignment errors, pointer edition
             * 
             * @param miss that one, pointer edition
             * @return MSLedit& the other boring old same new one
             */
            MSLedit& operator=(MSLedit* miss){
                return operator=(*miss);
            }

            /**
             * @brief Try operator this, genius implementation with stringstream right?
             * 
             * @param ccstr the string
             * @return MSLedit the instance that was rudely created with a stringstream
             */
            friend MSLedit operator""_MSLedit(const char* ccstr){
                std::stringstream ss;
                ss << ccstr;
                MSLedit missledit(ss.str());
                return missledit;
            }
        };

        #ifndef MXPSQL_MSLedit_No_Implementation

        MSLedit::MSLedit(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            setKey(nprompt, "MSLedit> ");
            setKey(nosystem, "false");
            setKey(nobanner, "false");
        }

        MSLedit::MSLedit(MSLedit& ledit) : MSLedit() {
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            setInstance(ledit);
        }

        MSLedit::MSLedit(MSLedit* ledit) : MSLedit(*ledit) {}

        MSLedit::MSLedit(std::string content) : MSLedit(false, content) {}

        MSLedit::MSLedit(char* cstr) : MSLedit(std::string(cstr)) {}
        MSLedit::MSLedit(const char* ccstr) : MSLedit(std::string(ccstr)) {}

        MSLedit::MSLedit(std::vector<std::string> buffer) : MSLedit() {
            setBuffer(buffer);
        }

        MSLedit::MSLedit(std::initializer_list<std::string> ilbuf) : MSLedit(std::vector<std::string>(ilbuf)) {}

        MSLedit::MSLedit(std::vector<char> cbuffer) : MSLedit(){
            std::string s(cbuffer.begin(), cbuffer.end());
            setText(s);
        }

        MSLedit::MSLedit(std::initializer_list<char> cilbuf) : MSLedit(std::vector<char>(cilbuf)) {}

        MSLedit::MSLedit(bool fileinsteadofcontent, std::string obj) : MSLedit() {
            if(fileinsteadofcontent){
                readFile(obj);
            }
            else{
                setText(obj);
            }
        }


        void MSLedit::setInstance(MSLedit& other){
            this->setText(other.str());
            this->setConfig(other.getConfig());
        }

        void MSLedit::setInstance(const MSLedit& cother){
            this->setInstance(const_cast<MSLedit&>(cother));
        }

        void MSLedit::setInstance(MSLedit* other){
            this->setInstance(*other);
        }


        size_t MSLedit::lineNums(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return buffer.size();
        }

        size_t MSLedit::length(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().length();
        }

        size_t MSLedit::size(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().size();
        }

        std::string MSLedit::stringAtLine(size_t line){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(line > lineNums() || line < 1){
                throw std::out_of_range("Attempting to access beyond array bounds");
            }

            return buffer.at(line-1);
        }

        std::pair<size_t, size_t> MSLedit::getGridIndexFromStringIndex(size_t strindex){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            size_t line = 1;
            size_t index = 1;

            if(strindex > length() || strindex < 1){
                throw std::out_of_range("Attempting to access beyond array bounds");
            }

            char c = ' ';
            for(size_t i = 0; i < strindex-1; i++, index++){
                c = charAtPosition(i+1);
                if(c == '\n'){
                    line++;
                    index = 1;
                    // std::cout << "yeah";
                }
            }


            return std::make_pair(line, index+1);
        }

        char MSLedit::charAtPosition(size_t index){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            if(index < 1 || index > length()){
                throw std::out_of_range("Attempting to access beyond array bounds");
            }

            return t.at(index);
        }

        char MSLedit::charAtGrid(size_t line, size_t index){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            char c = ' ';
            if((line < 1 || line > lineNums())){
                throw std::out_of_range("Attempting to access beyond array bounds");
            }

            std::string t = stringAtLine(line);
            if((line+1) <= lineNums()){
                t += '\n';
            }
            if(index < 1 || index > t.size()){
                throw std::out_of_range("Attempting to access beyond string bounds");
            }

            c = t.at(index-1);

            return c;
        }


        void MSLedit::readFile(std::string path){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(path.empty() || path.find_first_not_of(' ') == std::string::npos || path.length() < 1) path = file;
            if(path.empty() || path.find_first_not_of(' ') == std::string::npos || path.length() < 1) throw std::runtime_error("File not set");
            std::ifstream fstream(path, std::ios::binary);
            if(!fstream.is_open() || !fstream.good()){
                if(fstream.is_open()) fstream.close();

                throw std::runtime_error("Unable to open '" + path + "'");
                return;
            }
            else{
                std::ostringstream ss;
                ss << fstream.rdbuf();
                setText(ss.str());
                fstream.close();
                file = path;
            }
        }

        void MSLedit::writeFile(std::string path){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(path.empty() || path.find_first_not_of(' ') == std::string::npos || path.length() < 1) path = file;
            if(path.empty() || path.find_first_not_of(' ') == std::string::npos || path.length() < 1) throw std::runtime_error("File not set");
            std::ofstream fstream(path, std::ios::binary);
            if(!fstream.is_open() || !fstream.good()){
                if(fstream.is_open()) fstream.close();

                throw std::runtime_error("Unable to write to '" + path + "'");
                return;
            }
            else{
                std::string text = str();
                fstream.write(text.c_str(), length()-1);
                file = path;
            }
        }

        void MSLedit::writeFileLocked(std::string path){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string tmpfile = "";
            bool isTaken = true;
            while(isTaken){
                {
                    // make temp file
                    char tmpfile_tmp[L_tmpnam];
                    if(!std::tmpnam(tmpfile_tmp)){
                        throw std::runtime_error("Unable to generate random file name for locking");
                    }
                    tmpfile = std::string(tmpfile_tmp);
                }
                {
                    // check if it is taken
                    std::ifstream chk_in(tmpfile, std::ios::binary);
                    isTaken = chk_in.is_open() && chk_in.good();
                    if(chk_in.is_open()){
                        chk_in.close();
                    }
                }
            }
            writeFile(tmpfile);
            // attempt to use good old c style functions
            if(std::rename(tmpfile.c_str(), path.c_str()) != 0) 
            {
                // We rely on fallback of the normal boring ifstream and ofstream c++ io for copying if there is problem with std::rename
                // Something goes (maybe horribly or even catastrophically) wrong with it or you tried to copy between different filesystems or drive and we run fallback
                {
                    // copy file
                    std::ifstream in(tmpfile, std::ios::binary);
                    std::ofstream out(path, std::ios::binary);

                    bool is_in_good = in.is_open() && in.good() && !in.fail() && in;
                    bool is_out_good = out.is_open() && out.good() && !out.fail() && in;

                    if(!is_in_good || !is_out_good){
                        if(in.is_open()){
                            in.close();
                        }

                        if(out.is_open()){
                            out.close();
                        }
                        throw std::runtime_error((std::string("Unable to copy file from temporary file;") + tmpfile));
                    }

                    out << in.rdbuf();

                    if(in.is_open()){
                        in.close();
                    }

                    if(out.is_open()){
                        out.close();
                    }
                }
                {
                    // compare file
                    std::ifstream in_cmp_1(tmpfile, std::ios::binary);
                    std::ifstream in_cmp_2(path, std::ios::binary);

                    bool is_f1_good = in_cmp_1.is_open() && in_cmp_1.good() && !in_cmp_1.fail() && in_cmp_1;
                    bool is_f2_good = in_cmp_2.is_open() && in_cmp_2.good() && !in_cmp_2.fail() && in_cmp_2;

                    if(!is_f1_good || !is_f2_good){
                        if(in_cmp_1.is_open()){
                            in_cmp_1.close();
                        }

                        if(in_cmp_2.is_open()){
                            in_cmp_2.close();
                        }
                        throw std::runtime_error((std::string("Unable to compare temporary file to destionation file path due to IO Error;") + tmpfile));
                    }

                    if(in_cmp_1.tellg() != in_cmp_2.tellg()){
                        // size mismatch
                        if(in_cmp_1.is_open()){
                            in_cmp_1.close();
                        }

                        if(in_cmp_2.is_open()){
                            in_cmp_2.close();
                        }
                        throw std::runtime_error((std::string("Size mismatch between temporary file to destionation file path;") + tmpfile));
                    }

                    if(!in_cmp_1.seekg(0, std::ifstream::beg) || !in_cmp_2.seekg(0, std::ifstream::beg)){
                        if(in_cmp_1.is_open()){
                            in_cmp_1.close();
                        }

                        if(in_cmp_2.is_open()){
                            in_cmp_2.close();
                        }
                        throw std::runtime_error((std::string("Unable to set reading point thingy (?) to original position. failbit is broken lmao;") + tmpfile));
                    }

                    in_cmp_1.clear();
                    in_cmp_2.clear();

                    bool fequal = true;
                    
                    {
                        // compare with char one by one and a checksum (simple char to long summing)

                        bool simplesum_status = true;
                        {
                            // simple char to long summing checksum
                            typedef unsigned long long int lsize_t; // yeah long
                            lsize_t simplesum_1 = 0;
                            lsize_t simplesum_2 = 0;

                            char c = 0;
                            char c2 = 0;

                            while(in_cmp_1.get(c)){
                                simplesum_1 += c;
                            }
                            
                            while(in_cmp_2.get(c2)){
                                simplesum_2 += c2;
                            }

                            simplesum_status = simplesum_1 == simplesum_2;
                        }

                        if(!in_cmp_1.seekg(0, std::ifstream::beg) || !in_cmp_2.seekg(0, std::ifstream::beg)){
                            if(in_cmp_1.is_open()){
                                in_cmp_1.close();
                            }

                            if(in_cmp_2.is_open()){
                                in_cmp_2.close();
                            }
                            throw std::runtime_error((std::string("Unable to set reading point thingy (?) to original position. failbit is broken lmao;") + tmpfile));
                        }

                        in_cmp_1.clear();
                        in_cmp_2.clear();


                        // fequal checks that uses char comparison
                        fequal = std::equal(
                        std::istreambuf_iterator<char>(in_cmp_1.rdbuf()),
                        std::istreambuf_iterator<char>(),
                        std::istreambuf_iterator<char>(in_cmp_2.rdbuf())
                        ) && simplesum_status;

                        if(in_cmp_1.is_open()){
                            in_cmp_1.close();
                        }

                        if(in_cmp_2.is_open()){
                            in_cmp_2.close();
                        }
                    }

                    if(!fequal){
                        if(in_cmp_1.is_open()){
                            in_cmp_1.close();
                        }

                        if(in_cmp_2.is_open()){
                            in_cmp_2.close();
                        }
                        throw std::runtime_error((std::string("Content mismatch between temporary file to destionation file path;") + tmpfile));
                    }
                }
                // remove temporary file
                if(std::remove(tmpfile.c_str()) != 0){
                    throw std::runtime_error((std::string("Unable to remove temporary file;") + tmpfile));
                }
            }
        }

        void MSLedit::setText(std::string text){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::istringstream iss(text);
            std::string token;
            std::vector<std::string> vectr;

            while(std::getline(iss, token, '\n')){
                vectr.push_back(token);
            }

            setBuffer(vectr);
        }

        std::string MSLedit::getText(bool formatted, long int begin, long int end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::stringstream ss;
            std::string str = "";
            std::vector<std::string> nw(buffer);
            size_t lsize = lineNums();

            if(begin > end){
                throw std::out_of_range("Attempted to begin beyond end");
            }

            for(size_t i = 0; i < lsize; i++){
                if(begin > 0 || end > 0){
                    if(end > (long int) lsize) throw std::out_of_range("Attemting to edit beyond array bound");

                    long int cbegin = begin - 1;
                    long int cend = end - 1;
                    long int si = (long int) i;
                    if(si < cbegin){
                        continue;
                    }
                    else if(si > cend){
                        break;
                    }
                }

                if(formatted) {
                    size_t ione = (i+1);
                    std::string ioneAsStr = std::to_string(ione);
                    std::string lsizeAsStr = std::to_string(lsize);
                    std::string space = std::string(lsizeAsStr.length() - ioneAsStr.length(), ' ');
                    ss << space << ione << "|" << nw[i] << "\n";
                }
                else {
                    ss << nw[i] << "\n";
                }
            }

            str = ss.str();
            return str;
        }

        void MSLedit::setTextOfInstance(MSLedit& other_miss_ledit_instance){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            setText(other_miss_ledit_instance.str());
        }

        void MSLedit::swap(MSLedit& other_miss_ledit_instance){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string tmp = other_miss_ledit_instance.str();
            other_miss_ledit_instance.setText(str());
            setText(tmp);
        }

        std::string MSLedit::getText(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return getText(true, -1, -1);
        }

        std::string MSLedit::str(){
            return getText(false, -1, -1);
        }

        void MSLedit::setCString(char* cstr){
            setText(std::string(cstr));
        }

        void MSLedit::setCString(const char* ccstr){
            setCString((char*) ccstr);
        }

        char* MSLedit::getCString(bool formatted, long int begin, long int end){
            return (char*) getConstCString(formatted, begin, end);
        }

        const char* MSLedit::getConstCString(bool formatted, long int begin, long int end){
            std::string cp = getText(formatted, begin, end);
            char* cstr = new char[cp.length() + 1];
            cp.copy(cstr, cp.length() + 1);
            return (const char*) cstr;
        }

        void MSLedit::setBuffer(std::vector<std::string> buffer){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::vector<std::string> vectr(buffer);
            std::string tmpbuf;
            this->buffer.clear();
            {
                std::stringstream ss;
                for(size_t i = 0; i < vectr.size(); i++){
                    ss << vectr.at(i);
                    if(i+1 < vectr.size()){
                        ss << std::endl;
                    }
                }
                tmpbuf = ss.str();
            }
            {
                std::istringstream iss(tmpbuf);
                for(std::string token = "";std::getline(iss, token, '\n');this->buffer.push_back(token));
            }
        }

        std::vector<std::string> MSLedit::getBuffer(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return buffer;
        }

        void MSLedit::setBuffer(std::vector<char> cbuffer){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::vector<char> tmpcbuffer(cbuffer);
            std::string s(tmpcbuffer.begin(), tmpcbuffer.end());
            setText(s);
        }


        void MSLedit::print(bool formatted, long int begin, long int end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            print(formatted, std::cout, begin, end);
        }

        void MSLedit::print(bool formatted, std::ostream& stream, long int begin, long int end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            stream << getText(formatted, begin, end);
        }

        /* void MSLedit::print(bool formatted, std::stringstream& stream, long int begin, long int end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            print(formatted, stream, begin, end);
        } */


        void MSLedit::appendAtEnd(std::string line){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::istringstream iss(line);
            std::string token;
            while(std::getline(iss, token, '\n')) buffer.push_back(token);
        }

        void MSLedit::appendAtLine(int linenum, std::string line){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string l = stringAtLine((size_t) linenum) + line;

            std::istringstream iss(l);
            std::string token;
            for(int i = 0, nln = linenum; std::getline(iss, token, '\n');i++, nln++){
                if(i == 0){
                    editLine(linenum, token);
                }
                else{
                    insertAtLine(nln, token);
                }
            }
        }

        void MSLedit::insertAtLine(int linenum, std::string line){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(linenum > (int) lineNums() || linenum < 1) throw std::out_of_range("Attempting to edit beyond array bound");
            std::vector<std::string> tmpbuffer(buffer);
            std::istringstream iss(line);
            std::string token;
            for(auto it = tmpbuffer.begin() + linenum - 1; std::getline(iss, token, '\n'); it++){
                tmpbuffer.insert(it, token);
            }

            setBuffer(tmpbuffer);
        }

        std::pair<size_t, size_t> MSLedit::search(std::string text2search){
            return search(text2search, 1);
        }

        std::pair<size_t, size_t> MSLedit::search(std::string text2search, size_t begin_line){
            std::vector<std::pair<size_t, size_t>> s = search(text2search, begin_line, 1);
            if(s.size() < 1) return std::make_pair(std::string::npos, std::string::npos);
            else return s[0];
        }

        std::vector<std::pair<size_t, size_t>> MSLedit::search(std::string text2search, size_t begin_line, size_t count){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::vector<std::pair<size_t, size_t>> poses;
            std::vector<std::string> vectr(buffer);

            for(size_t i = begin_line, c = 0; i < vectr.size() && (count == std::string::npos || c < count); i++){
                std::string line = vectr.at(i);
                size_t pos = line.find(text2search);
                while(pos != std::string::npos){
                    poses.push_back(std::make_pair(i, pos));
                    c++;
                    pos = line.find(text2search, pos);
                }
            }

            return poses;
        }

        void MSLedit::editLine(int linenum, std::string line){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(linenum > (int) lineNums() || linenum < 1) throw std::out_of_range("Attempting to edit beyond array bound");
            std::istringstream iss(line);
            std::string token;
            for(int i = 0, nln = linenum; std::getline(iss, token, '\n');i++, nln++)
            {
                if(i == 0){
                    buffer[linenum-1] = token;
                }
                else{
                    insertAtLine(nln, token);
                }
            }
        }

        void MSLedit::editChar(int index, char character){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            if(index > (int) length() || index < 1) throw std::out_of_range("Attempting to edit beyond array beyond");
            t[index-1] = character;
            setText(t);
        }

        void MSLedit::deleteAtLine(int linenum){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(linenum > (int) lineNums() || linenum < 1) throw std::out_of_range("Attempting to print beyond array bound");
            buffer.erase(buffer.begin() + linenum - 1);
        }

        void MSLedit::clear(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            buffer.clear();
        }


        void MSLedit::append(char c){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string s = "";
            s += c;
            append(s);
        }

        void MSLedit::append(char* cstr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(cstr == NULL && cstr == nullptr){
                append("");
                return;
            }
            append(const_cast<const char*>(cstr));
        }

        void MSLedit::append(const char* ccstr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string s = "";
            if(ccstr == NULL || ccstr == nullptr){
                s = "";
            }
            else{
                // stringstream way
                std::stringstream ss;
                ss << ccstr;
                s = ss.str();
            }
            append(s);
        }

        void MSLedit::append(double d){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(d));
        }

        void MSLedit::append(float f){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(f));
        }

        void MSLedit::append(int i){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(i));
        }

        void MSLedit::append(long int li){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(li));
        }

        void MSLedit::append(size_t s){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(s));
        }

        void MSLedit::append(bool boolean){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(boolean ? "true" : "false");
        }

        void MSLedit::append(std::string str){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            /* if(str.find_first_not_of('\n') != std::string::npos){
                std::istringstream iss(str);
                std::string token;
                while(std::getline(iss, token, '\n')) appendAtEnd(token);
            }
            else{
                for(size_t i = 0; i < str.length(); i++){
                    appendAtEnd(std::string(1, str[i]));
                }
            } */
            insert(size(), str);
        }

        void MSLedit::append(MSLedit miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(miss.str());
        }

        void MSLedit::append(void* ptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string ssstr;
            int size = std::snprintf(nullptr, 0, "%p", ptr);
            std::vector<char> autocstr(size + 1);
            if(std::snprintf(&autocstr[0], autocstr.size(), "%p", ptr) < 0){
                throw std::length_error("Unable to write pointer to temporary buffer");
                return;
            }
            for(char c : autocstr){
                ssstr += c;
            }
            append(ssstr);
        }

        void MSLedit::append(std::nullptr_t nptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(((void*) nptr));
        }

        void MSLedit::append(long int* liptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(((void*) liptr));
        }

        void MSLedit::appendNewLine(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append('\n');
        }


        void MSLedit::insert(size_t position, bool boolean){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, boolean ? "true" : "false");
        }

        void MSLedit::insert(size_t position, char* cstr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::string(cstr));
        }

        void MSLedit::insert(size_t position, const char* ccstr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::string(ccstr));
        }

        void MSLedit::insert(size_t position, int i){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::to_string(i));
        }

        void MSLedit::insert(size_t position, long int li){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::to_string(li));
        }

        void MSLedit::insert(size_t position, size_t s){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::to_string(s));
        }

        void MSLedit::insert(size_t position, std::string estr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string tstr = str();
            tstr.insert(position, estr);

            setText(tstr);
        }

        void MSLedit::insert(size_t position, MSLedit miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, miss.getText());
        }

        void MSLedit::insert(size_t position, void* ptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string ssstr;
            int size = std::snprintf(nullptr, 0, "%p", ptr);
            std::vector<char> autocstr(size + 1);
            if(std::snprintf(&autocstr[0], autocstr.size(), "%p", ptr) < 0){
                throw std::length_error("Unable to write pointer to temporary buffer");
                return;
            }
            for(char c : autocstr){
                ssstr += c;
            }
            append(ssstr);
            insert(position, ssstr);
        }

        void MSLedit::insert(size_t position, std::nullptr_t nptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, ((void*) nptr));
        }

        void MSLedit::insert(size_t position, long int* liptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, ((void*) liptr));
        }

        void MSLedit::insertnewline(size_t position){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert('\n', position);
        }


        void MSLedit::deleteAt(size_t begin, size_t end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            t.erase(begin, end);
            setText(t);
        }

        void MSLedit::deleteCharAt(size_t index){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            t.erase(index, 1);
            setText(t);
        }


        size_t MSLedit::indexOf(std::string text){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            return t.find(text);
        }

        size_t MSLedit::indexOf(std::string text, size_t begin){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().find(text, begin);
        }


        std::vector<std::string> MSLedit::split(char delimiter){
            std::vector<std::string> misses;
            std::string t = str();
            std::string token;
            std::istringstream iss(t);
            for(token="";std::getline(iss, token, delimiter);misses.push_back(token));
            return misses;
        }

        std::vector<std::string> MSLedit::split(std::string delimiter){
            std::vector<std::string> misses;
            std::string t = str();
            size_t start = 0;
            size_t end = t.find(delimiter);
            while (end != std::string::npos) {
                std::string tt = t.substr(start, end - start);
                start = end + delimiter.size();
                end = t.find(delimiter, start);
                misses.push_back(tt);
            }
            misses.push_back(t.substr(start, end - start));
            return misses;
        }

        
        std::string MSLedit::substring(size_t pos){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().substr(pos);
        }

        std::string MSLedit::substring(size_t pos, size_t end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().substr(pos, end);
        }


        void MSLedit::reverse(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            std::string rt(t.rbegin(), t.rend());
            setText(rt);
        }


        void MSLedit::setConfig(std::map<std::string, std::string> c2){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            strEditorConfig = c2;
        }

        void MSLedit::parseConfig(std::string configPath){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::map<std::string, std::string>& data = strEditorConfig;
            std::ifstream cFile(configPath);
            if (cFile.is_open() && cFile.good()) {
                std::string line;
                while (getline(cFile, line)) {
                    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
                    if (line[0] == '#' || line.empty()) {
                        continue;
                    } else if (line.find('#')) {
                        line = line.substr(0, line.find('#'));
                    }
                    std::istringstream iss(line);
                    std::string strr;
                    while (std::getline(iss, strr, ',')) {
                        auto delimiterPos = strr.find("=");
                        auto name         = strr.substr(0, delimiterPos);
                        std::string value      = strr.substr(delimiterPos + 1);
                        // std::cout << name << " " << value << '\n';
                        data[name] = value;
                    }
                }
            } else {
                if(cFile.is_open()) cFile.close();
                throw std::runtime_error("File '" + configPath + "' not found");
            }
        }

        std::string MSLedit::buildConfig(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::map<std::string, std::string>& data = strEditorConfig;
            std::stringstream ss;
            for(auto const& item : data){
                ss << item.first << "=" << item.second << std::endl;
            }
            return ss.str();
        }

        std::map<std::string, std::string> MSLedit::getConfig(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return strEditorConfig;
        }

        void MSLedit::setKey(std::string key, std::string value){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            strEditorConfig[key] = value;
        }

        std::string MSLedit::getKey(std::string key){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return strEditorConfig[key];
        }

        bool MSLedit::keyExists(std::string key){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            auto it = strEditorConfig.find(key);
            return (it != strEditorConfig.end());
        }


        int MSLedit::repl(){
            std::string prompt = "> ";
            {
                std::unique_lock<std::recursive_mutex> lock(lock_mutex);
                if(keyExists(nprompt)) prompt = getKey(nprompt);
            }

            return repl(prompt);
        }

        int MSLedit::repl(std::string prompt){
            return repl(prompt, std::cout, std::cin, std::cerr);
        }

        int MSLedit::repl(std::string prompt, std::ostream& out, std::istream& in, std::ostream& err){
            int status = EXIT_SUCCESS;
            bool run = true;
            std::string l = "";
            std::vector<std::string> ls;

            {
            bool allowBanner = true;
            {
                std::unique_lock<std::recursive_mutex> lock(lock_mutex);
                if(keyExists(nobanner)) allowBanner = (getKey(nobanner) != "true");
            }

                if(allowBanner){
                    size_t s = 0;
                    std::vector<std::string> banners{"MSLedit", "Written by MXPSQL", "Entering REPL", "Type 'h' for help"};
                    std::string banner = "";
                    for(std::string bnr : banners){
                        size_t s2 = bnr.length() + 5;
                        if(s2 > s){
                            s = s2;
                        }
                        out << bnr << std::endl;
                    }
                    out << std::string(s, '=') << std::endl;
                }
            }

            while(run){
                try{
                    std::string begin = "";
                    ls.clear();
                    out << prompt;
                    if(!std::getline(in, l, '\n')){
                        err << "Error getting input from user." << std::endl;
                        status = EXIT_FAILURE;
                        run = false;
                    }


                    if(l.find_first_not_of(" ") == std::string::npos || l.length() < 1) continue;
                    out << std::endl; // for the betterness with pipes when piping

                    {
                        std::istringstream iss(l);
                        std::string token = "";
                        while(getline(iss, token, ' ')) ls.push_back(token);
                    }

                    begin = ls.at(0);
                    size_t arglen = ls.size() - 1;
                    std::vector<std::string> args(ls);
                    args.erase(args.begin());
                    
                    // # and <# are comments
                    // for scripting purposes
                    {
                        auto chk_comment = [](std::string line) -> bool {


                            bool shbegin = ((line.find("#") != std::string::npos) && (line.find("#") == 0));
                            bool pwshbegin = ((line.find("<#") != std::string::npos) && (line.find("<#") == 0));

                            return (shbegin || pwshbegin);
                        };

                        if(chk_comment(l)){
                            continue;
                        }
                    }

                    if(begin == "q" || begin == "quit" || begin == "exit"){
                        std::locale loc;
                        std::string opt = "";
                        while(!(opt == "y" || opt == "n")){
                            opt = "";
                            out << "Are you sure you want to quit? You may have unsaved documents, just save or don't quit to be sure. (Y/N/y/n) ";
                            getline(in, opt, '\n');
                            {
                                std::string nopt = opt;
                                for(char& c : nopt){
                                    c = std::tolower(c, loc);
                                }

                                opt = nopt;
                            }

                            if (in.rdbuf()->in_avail() > 0) {
                                while(in.get() != EOF);
                            }
                        }

                        if(opt != "y"){
                            continue;
                        }
                        run = false;
                        status = EXIT_SUCCESS;
                    }
                    else if(begin == "h" || begin == "help"){
                        {
                            std::string helpbanner = "MSLEdit REPL Help";
                            out << helpbanner << std::endl << std::string(helpbanner.length(), '=') << std::endl;
                        }

                        out 
                        << "q, quit, exit: Exits the REPL. Usage example: 'q'" << std::endl
                        << "h, help: Prints this help message. Usage example: 'help'" << std::endl
                        << "v, view, p, print: Prints the file. Usage example: 'view' or 'view [line begin] [line end]" << std::endl
                        << "aae, appendAtEnd: Append a line at the end of document. Usage example: 'aae [your text here]'" << std::endl
                        << "anlae, appendNewLineAtEnd: Append newline at the end. Usage example: 'anlae'" << std::endl
                        << "aal, appendAtLine: Append text to a line. Usage example: 'aal [your line] [your text here]'" << std::endl
                        << "ial, insertAtLine: Insert text before a line. Usage example: 'ial [your line] [your text here]'" << std::endl
                        << "inlal, insertNewLineAtLine: Insert a newline before a line. Usage example: 'inlal [your line]'" << std::endl
                        << "s, search: Search for text. Usage example: 'search [text]" << std::endl
                        << "ss, ssearch: Search for text from a line and beyond. Usage example: 'search [line] [text]" << std::endl
                        << "el, editLine: Edit a line. Usage example: 'el [line] [your text]" << std::endl
                        << "del, delete: Delete a line. Usage example; 'del [your line]" << std::endl
                        << "open, read: Open a file. Usage example: 'open [not funny]'" << std::endl
                        << "save, write: Save a file. Usage example: 'save [your file]'" << std::endl
                        << "prompt: Set prompt. Usage example: '> ee your not so funny prompt maybe> '" << std::endl
                        << "sh, shell, exec, run, cmd, system: Run a command. Usage example: 'exec echo monke'" << std::endl
                        << "ed: ed, you know what ed is. (runs 'ed' on posix environments, dumb on windows) Usage: 'ed [your ed arguments]'" << std::endl
                        << std::endl;
                    }
                    else if(begin == "v" || begin == "view" || begin == "p" || begin == "print"){
                        if(arglen < 2){
                            if(arglen == 2) {
                                err << "Missing argument to command '" << begin << "'" << std::endl;
                                continue;
                            }
                            try{
                                print(true, -1, -1);
                            }
                            catch(std::runtime_error& re){
                                err << "There is a problem viewing the current document. "
                                << std::endl << "The message is '" << re.what() << "'"
                                << std::endl;
                            }
                        }
                        else{
                            try{
                                long int beg = std::stol(args[0]);
                                long int end = std::stol(args[1]);
                                print(true, out, beg, end);
                            }
                            catch(std::invalid_argument& ia){
                                err << "Invalid arguments provided, message: '" << ia.what() << "'" << std::endl;
                            }
                            catch(std::out_of_range& ooa){
                                err << "Arguments is out of range, did you try to view beyond the limit? message: " << ooa.what() << "'" << std::endl;
                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured."
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }
                    }
                    else if(begin == "aae" || begin == "appendAtEnd"){
                        if(arglen < 1){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            std::stringstream ss;
                            for(size_t i = 0; i < arglen; i++){
                                if((i + 1) >= arglen){
                                    ss << args[i];
                                }
                                else{
                                    ss << args[i] << " ";
                                }
                            }

                            appendAtEnd(ss.str());
                        }
                    }
                    else if(begin == "anlae" || begin == "appendNewLineAtEnd"){
                        appendNewLine();
                    }
                    else if(begin == "aal" || begin == "appendAtLine"){
                        if(arglen < 2){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                std::vector<std::string> slicedargs(args);
                                int line = stoi(slicedargs[0]);
                                slicedargs.erase(slicedargs.begin());
                                std::stringstream ss;
                                for(size_t i = 0; i < slicedargs.size(); i++){
                                    if((i + 1) >= slicedargs.size()){
                                        ss << slicedargs[i];
                                    }
                                    else{
                                        ss << slicedargs[i] << " ";
                                    }
                                }

                                appendAtLine(line, ss.str());
                            }
                            catch(std::invalid_argument& ia){
                                err << "Second argument provided is invalid, message: '" << ia.what() << "'" <<
                                std::endl << ". If you see stoi on the message, that second input is wrong" << std::endl;
                            }
                            catch(std::out_of_range& ooa){
                                err << "Second argument is out of range or some problem occured, did you try to edit beyond the limit? message: " << ooa.what() << "'" << std::endl;
                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured."
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }
                    }
                    else if(begin == "ial" || begin == "insertAtLine"){
                        if(arglen < 2){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                std::vector<std::string> slicedargs(args);
                                int line = stoi(slicedargs[0]);
                                slicedargs.erase(slicedargs.begin());
                                std::stringstream ss;
                                for(size_t i = 0; i < slicedargs.size(); i++){
                                    if((i + 1) >= slicedargs.size()){
                                        ss << slicedargs[i];
                                    }
                                    else{
                                        ss << slicedargs[i] << " ";
                                    }
                                }

                                insertAtLine(line, ss.str());
                            }
                            catch(std::invalid_argument& ia){
                                err << "Second argument provided is invalid, message: '" << ia.what() << "'" <<
                                std::endl << ". If you see stoi on the message, that second input is wrong" << std::endl;
                            }
                            catch(std::out_of_range& ooa){
                                err << "Second argument is out of range or some problem occured, did you try to edit beyond the limit? message: " << ooa.what() << "'" << std::endl;
                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured."
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }
                    }
                    else if(begin == "inlal" || begin == "insertNewLineAtLine"){
                        if(arglen < 1){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                std::vector<std::string> slicedargs(args);
                                int line = stoi(slicedargs[0]);
                                slicedargs.erase(slicedargs.begin());

                                insertAtLine(line, std::string("\n"));
                            }
                            catch(std::invalid_argument& ia){
                                err << "Second argument provided is invalid, message: '" << ia.what() << "'" <<
                                std::endl << ". If you see stoi on the message, that second input is wrong" << std::endl;
                            }
                            catch(std::out_of_range& ooa){
                                err << "Second argument is out of range or some problem occured, did you try to edit beyond the limit? message: " << ooa.what() << "'" << std::endl;
                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured. "
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }
                    }
                    else if(begin == "search" || begin == "s"){
                        if(arglen < 1){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            std::stringstream ss;
                            for(size_t i = 0; i < arglen; i++){
                                if((i + 1) >= arglen){
                                    ss << args[i];
                                }
                                else{
                                    ss << args[i] << " ";
                                }
                            }
                            std::pair<size_t, size_t> location = search(ss.str());
                            if(location.first == std::string::npos){
                                err << "'" << ss.str() << "' not found." << std::endl;
                            }
                            else{
                                std::string marker = std::to_string(location.first+1) + "| ";
                                out << marker << stringAtLine(location.first+1) << std::endl
                                << std::string(location.second+marker.size(), ' ') << std::string((int) ss.str().size(), '^') << std::endl;
                            }
                        }
                    }
                    else if(begin == "ssearch" || begin == "ss"){
                        if(arglen < 2){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                int begin_line = std::stoi(args[0]);
                                std::stringstream ss;
                                for(size_t i = 1; i < arglen; i++){
                                    if((i + 1) >= arglen){
                                        ss << args[i];
                                    }
                                    else{
                                        ss << args[i] << " ";
                                    }
                                }
                                std::pair<size_t, size_t> s = search(ss.str(), begin_line);
                                if(s.first == std::string::npos){
                                    err << "'" << ss.str() << "' not found." << std::endl;
                                }
                                else{
                                    std::string marker = std::to_string(s.first+1) + "| ";
                                    out << marker << stringAtLine(s.first+1) << std::endl
                                    << std::string(s.second+marker.size(), ' ') << std::string((int) ss.str().size(), '^') << std::endl;
                                }
                            }
                            catch(std::invalid_argument& ia){
                                err << "Second argument provided is invalid, message: '" << ia.what() << "'" <<
                                std::endl << ". If you see stoi on the message, that second input is wrong" << std::endl;
                            }
                            catch(std::out_of_range& ooa){
                                err << "Second argument is out of range or some problem occured, message: " << ooa.what() << "'" << std::endl;
                            }
                        }
                    }
                    else if(begin == "is" || begin == "isearch"){

                    }
                    else if(begin == "el" || begin == "editLine"){
                        if(arglen < 2){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                std::vector<std::string> slicedargs(args);
                                int line = stoi(slicedargs[0]);
                                slicedargs.erase(slicedargs.begin());
                                std::stringstream ss;
                                for(size_t i = 0; i < slicedargs.size(); i++){
                                    if((i + 1) >= slicedargs.size()){
                                        ss << slicedargs[i];
                                    }
                                    else{
                                        ss << slicedargs[i] << " ";
                                    }
                                }

                                editLine(line, ss.str());
                            }
                            catch(std::invalid_argument& ia){
                                err << "Second argument provided is invalid, message: '" << ia.what() << "'" <<
                                std::endl << ". If you see stoi on the message, that second input is wrong" << std::endl;
                            }
                            catch(std::out_of_range& ooa){
                                err << "Second argument is out of range or some problem occured, did you try to edit beyond the limit? message: " << ooa.what() << "'" << std::endl;
                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured. "
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }
                    }
                    else if(begin == "del" || begin == "delete"){
                        if(arglen < 1){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                int line = stoi(args[0]);

                                deleteAtLine(line);
                            }
                            catch(std::invalid_argument& ia){
                                err << "Second argument provided is invalid, message: '" << ia.what() << "'" <<
                                std::endl << ". If you see stoi on the message, that second input is wrong" << std::endl;
                            }
                            catch(std::out_of_range& ooa){
                                err << "Second argument is out of range or some problem occured, did you try to edit beyond the limit, message: " << ooa.what() << "'" << std::endl;
                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured. "
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }
                    }
                    else if(begin == "open" || begin == "read"){
                        if(arglen < 1){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                std::locale loc;
                                std::stringstream ss;
                                for(size_t i = 0; i < arglen; i++){
                                    if((i + 1) >= arglen){
                                        ss << args[i];
                                    }
                                    else{
                                        ss << args[i] << " ";
                                    }
                                }

                                {
                                    std::ifstream ifs(ss.str(), std::ios::binary);
                                    if(ifs.is_open()){
                                        std::string opt = "";
                                        while(!(opt == "y" || opt == "n")){
                                            opt = "";
                                            out << "You want to open '" << ss.str() << "', Do you want to overwrite your current document (if it exists)? (Y/N/y/n) ";
                                            getline(in, opt, '\n');
                                            {
                                                std::string nopt = opt;
                                                for(char& c : nopt){
                                                    c = std::tolower(c, loc);
                                                }

                                                opt = nopt;
                                            }

                                            if (in.rdbuf()->in_avail() > 0) {
                                                while(in.get() != EOF);
                                            }
                                        }

                                        if(opt != "y"){
                                            continue;
                                        }
                                    }

                                    out << "File '" << ss.str() << "' was opened." << std::endl;
                                    readFile(ss.str());

                                    if(ifs.is_open()) ifs.close();
                                }
                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured, did you try to open a non-existent file? "
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }   
                    }
                    else if(begin == "save" || begin == "write"){
                        if(arglen < 1){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            try{
                                std::locale loc;
                                std::stringstream ss;
                                for(size_t i = 0; i < arglen; i++){
                                    if((i + 1) >= arglen){
                                        ss << args[i];
                                    }
                                    else{
                                        ss << args[i] << " ";
                                    }
                                }

                                {
                                    std::ifstream ifs(ss.str(), std::ios::binary);
                                    if(ifs.is_open()){
                                        std::string opt = "";
                                        while(!(opt == "y" || opt == "n")){
                                            opt = "";
                                            out << "The file called '" << ss.str() << "' exists. Do you want to overwrite it? (Y/N/y/n) ";
                                            getline(in, opt, '\n');
                                            {
                                                std::string nopt = opt;
                                                for(char& c : nopt){
                                                    c = std::tolower(c, loc);
                                                }

                                                opt = nopt;
                                            }

                                            if (in.rdbuf()->in_avail() > 0) {
                                                while(in.get() != EOF);
                                            }
                                        }

                                        if(opt != "y"){
                                            continue;
                                        }
                                    }

                                    out << "File '" << ss.str() << "' was written." << std::endl;
                                    writeFileLocked(ss.str());

                                    if(ifs.is_open()) ifs.close();
                                }

                            }
                            catch(std::runtime_error& re){
                                err << "An error had occured. Something may possibly gone wrong when writing the file. "
                                << std::endl << "Here is the message btw: '" << re.what() << "'"
                                << std::endl;
                            }
                        }   
                    }
                    else if(begin == "prompt"){
                        if(arglen < 1){
                            err << "Missing arguments to '" << begin << "'" << std::endl;
                        }
                        else{
                            std::stringstream ss;
                            for(size_t i = 0; i < arglen; i++){
                                if((i + 1) >= arglen){
                                    ss << args[i];
                                }
                                else{
                                    ss << args[i] << " ";
                                }
                            }
                            prompt = ss.str();
                        }
                    }
                    else if(begin == "sh" || begin == "shell" || begin == "exec" || begin == "run" || begin == "cmd" || begin == "system"){
                        bool allow = true;
                        {
                            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
                            if(keyExists(nosystem)) allow = (getKey(nosystem) != "true");
                        }
                        if(!allow){
                            err << "This command uses the function 'System' and it is disabled. Not sorry at all you cannot do this." << std::endl;
                            continue;
                        }
                        {
                            if(std::system(NULL) == 0){
                                err << "This command uses the function 'System' and it needs a command prepecessor. You don't have one so sad." << std::endl;
                                continue;
                            }
                        }
                        {
                            std::stringstream ss;
                            for(size_t i = 0; i < arglen; i++){
                                if((i + 1) >= arglen){
                                    ss << args[i];
                                }
                                else{
                                    ss << args[i] << " ";
                                }
                            }
                            out << std::endl; // flush 4 system
                            std::system(ss.str().c_str());
                        }
                    }
                    else if(begin == "ed"){ // ed
                        // this command does not want to exist, but is forced to exist except on posix environments
                        #if (defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))) || (defined(__CYGWIN__)) || ((defined(__cplusplus) && __cplusplus >= 201703L) && __has_include(<unistd.h>))
                        bool allow = true;
                        {
                            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
                            if(keyExists(nosystem)) allow = (getKey(nosystem) != "true");
                        }
                        if(!allow){
                            err << "This command uses the function 'System' and it is disabled. Not sorry at all you cannot do this." << std::endl;
                            continue;
                        }
                        {
                            if(std::system(NULL) == 0){
                                err << "This command uses the function 'System' and it needs a command prepecessor. You don't have one so sad." << std::endl;
                                continue;
                            }
                        }
                        {
                            std::string scmd = "ed ";
                            std::stringstream ss;
                            for(size_t i = 0; i < arglen; i++){
                                if((i + 1) >= arglen){
                                    ss << args[i];
                                }
                                else{
                                    ss << args[i] << " ";
                                }
                            }
                            scmd += ss.str();
                            out << "ed time" << std::endl;
                            std::system(scmd.c_str());
                        }
                        #else
                        out << "?" << std::endl;
                        #endif
                    }
                    else{
                        int hstatus = EXIT_FAILURE;

                        if(((replBeginHandler) && (replBeginHandler != nullptr))){
                            hstatus = replBeginHandler(begin, args, arglen, out, in, err);
                        }

                        if(hstatus != EXIT_SUCCESS) {
                            err << "A command called '" + begin + "' does not want to exist. (Not found)" << std::endl;
                        }
                    }
                }
                catch(std::exception& e){
                    err << "Exception caught: '" + std::string(e.what()) << "'" << std::endl;
                    status = EXIT_FAILURE;
                    run = false;
                }

                ls.clear();
            }

            return status;
        }


        int MSLedit::compare(std::string mystr){
            return str().compare(mystr);
        }

        int MSLedit::compare(MSLedit& miss){
            return compare(miss.str());
        }

        int MSLedit::compare(char* cstr){
            return compare(std::string(cstr));
        }

        int MSLedit::compare(const char* ccstr){
            return compare(std::string(ccstr));
        }


        std::ostream& operator<<(std::ostream& os, MSLedit& miss){
            os << miss.getText();
            return os;
        }

        std::istream& operator>>(std::istream& is, MSLedit& miss){
            std::stringstream ss;
            std::string token;
            while(std::getline(is, token, '\n')) ss << token << '\n';
            miss.setText(ss.str());
            return is;
        }


        char MSLedit::operator[](int index){
            return str()[index];
        }
        #endif
    };
};

#endif
