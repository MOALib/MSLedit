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
#include <memory>

#include <mutex>
#include <thread>
#include <atomic>

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cstdarg>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#endif

namespace MXPSQL{
    namespace MSLedit{

        /**
         * @brief String builder and line editor. Base class for all variants.
         * 
         * @tparam StringType a std::string-like object (basically any object that is based of std::basic_string) for manipulation
         * 
         * @note This class has mutexes for almost all functions for thread safety (and possibly reeterant).
         * @warning Due to use of recursive_mutex for thread safety, this class may throw std::system_error, researching online shows that the mutex used here is hiding a bad design (but no, I will not replace that with wrapped functions becuase of big code). However, despite the thread safety checks, this class is not atomic.
         */
        template<typename StringType>
        class MSLedit{
            public: // again
            /* None, at all! none_t */

            protected: // again for hashing

            /**
             * @brief SHA1 checksum comparer. We do not need to know the hash
             * 
             * @details
             * 
             * from https://github.com/stbrumme/hash-library/blob/master/sha1.h and https://github.com/stbrumme/hash-library/blob/master/sha1.cpp, spliced together
             * 
             * The copyright notice for the source
             * 
             * zlib License
             * 
             * Copyright (c) 2014,2015 Stephan Brumme
             * 
             * This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
             * Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
             * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
             *    If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
             * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
             * 3. This notice may not be removed or altered from any source distribution.
             * 
             * 
             * @param c1 string 1
             * @param c2 string 2
             * @return true same checksum
             * @return false bad chksum
             */
            bool compareStringSHA1(std::string c1, std::string c2){
                #ifdef _MSC_VER
                // Windows
                typedef unsigned __int8  uint8_t;
                typedef unsigned __int32 uint32_t;
                typedef unsigned __int64 uint64_t;
                #endif

                  // mix functions for processBlock()
                  static auto f1 = [](uint32_t b, uint32_t c, uint32_t d) -> uint32_t
                  {
                    return d ^ (b & (c ^ d)); // original: f = (b & c) | ((~b) & d);
                  };
                
                  static auto f2 = [](uint32_t b, uint32_t c, uint32_t d) -> uint32_t
                  {
                    return b ^ c ^ d;
                  };
                
                  static auto f3 = [](uint32_t b, uint32_t c, uint32_t d) -> uint32_t
                  {
                    return (b & c) | (b & d) | (c & d);
                  };
                
                  static auto rotate = [](uint32_t a, uint32_t c) -> uint32_t
                  {
                    return (a << c) | (a >> (32 - c));
                  };
                
                  auto static swap = [](uint32_t x) -> uint32_t
                  {
                #if defined(__GNUC__) || defined(__clang__)
                    return __builtin_bswap32(x);
                #elif defined(MSC_VER)
                    return _byteswap_ulong(x);
                #else
                
                    return (x >> 24) |
                          ((x >>  8) & 0x0000FF00) |
                          ((x <<  8) & 0x00FF0000) |
                           (x << 24);
                #endif
                  };


                /// compute SHA1 hash
                /** Usage:
                    SHA1 sha1;
                    std::string myHash  = sha1("Hello World");     // std::string
                    std::string myHash2 = sha1("How are you", 11); // arbitrary data, 11 bytes
                    // or in a streaming fashion:
                    SHA1 sha1;
                    while (more data available)
                      sha1.add(pointer to fresh data, number of new bytes);
                    std::string myHash3 = sha1.getHash();
                  */
                class SHA1 //: public Hash
                {
                public:
                  /// split into 64 byte blocks (=> 512 bits), hash is 20 bytes long
                  enum { BlockSize = 512 / 8, HashBytes = 20 };

                  /// same as reset()
                  SHA1(){
                    reset();
                  }

                  /// compute SHA1 of a memory block
                  std::string operator()(const void* data, size_t numBytes){
                    reset();
                    add(data, numBytes);
                    return getHash();
                  }
                  /// compute SHA1 of a string, excluding final zero
                  std::string operator()(const std::string& text){
                    reset();
                    add(text.c_str(), text.size());
                    return getHash();
                  }

                  /// add arbitrary number of bytes
                  void add(const void* data, size_t numBytes){
                        const uint8_t* current = (const uint8_t*) data;
                        
                        if (m_bufferSize > 0)
                        {
                          while (numBytes > 0 && m_bufferSize < BlockSize)
                          {
                            m_buffer[m_bufferSize++] = *current++;
                            numBytes--;
                          }
                        }
                        
                        // full buffer
                        if (m_bufferSize == BlockSize)
                        {
                          processBlock((void*)m_buffer);
                          m_numBytes  += BlockSize;
                          m_bufferSize = 0;
                        }
                        
                        // no more data ?
                        if (numBytes == 0)
                          return;
                        
                        // process full blocks
                        while (numBytes >= BlockSize)
                        {
                          processBlock(current);
                          current    += BlockSize;
                          m_numBytes += BlockSize;
                          numBytes   -= BlockSize;
                        }
                        
                        // keep remaining bytes in buffer
                        while (numBytes > 0)
                        {
                          m_buffer[m_bufferSize++] = *current++;
                          numBytes--;
                        }
                  }

                  /// return latest hash as 40 hex characters
                  std::string getHash(){
                    // compute hash (as raw bytes)
                    unsigned char rawHash[HashBytes];
                    getHash(rawHash);

                    // convert to hex string
                    std::string result;
                    result.reserve(2 * HashBytes);
                    for (int i = 0; i < HashBytes; i++)
                    {
                      static const char dec2hex[16+1] = "0123456789abcdef";
                      result += dec2hex[(rawHash[i] >> 4) & 15];
                      result += dec2hex[ rawHash[i]       & 15];
                    }

                    return result;
                  }
                  /// return latest hash as bytes
                  void        getHash(unsigned char buffer[HashBytes]){
                    // save old hash if buffer is partially filled
                    uint32_t oldHash[HashValues];
                    for (int i = 0; i < HashValues; i++)
                      oldHash[i] = m_hash[i];

                    // process remaining bytes
                    processBuffer();

                    unsigned char* current = buffer;
                    for (int i = 0; i < HashValues; i++)
                    {
                      *current++ = (m_hash[i] >> 24) & 0xFF;
                      *current++ = (m_hash[i] >> 16) & 0xFF;
                      *current++ = (m_hash[i] >>  8) & 0xFF;
                      *current++ =  m_hash[i]        & 0xFF;

                      // restore old hash
                      m_hash[i] = oldHash[i];
                    }
                  }

                  /// restart
                  void reset(){
                    m_numBytes   = 0;
                    m_bufferSize = 0;

                    // according to RFC 1321
                    m_hash[0] = 0x67452301;
                    m_hash[1] = 0xefcdab89;
                    m_hash[2] = 0x98badcfe;
                    m_hash[3] = 0x10325476;
                    m_hash[4] = 0xc3d2e1f0;
                  }

                private:
                  /// process 64 bytes
                  void processBlock(const void* data){
                      // get last hash
                      uint32_t a = m_hash[0];
                      uint32_t b = m_hash[1];
                      uint32_t c = m_hash[2];
                      uint32_t d = m_hash[3];
                      uint32_t e = m_hash[4];

                      // data represented as 16x 32-bit words
                      const uint32_t* input = (uint32_t*) data;
                      // convert to big endian
                      uint32_t words[80];
                      for (int i = 0; i < 16; i++)
                    #if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
                        words[i] = input[i];
                    #else
                        words[i] = swap(input[i]);
                    #endif

                      // extend to 80 words
                      for (int i = 16; i < 80; i++)
                        words[i] = rotate(words[i-3] ^ words[i-8] ^ words[i-14] ^ words[i-16], 1);

                      // first round
                      for (int i = 0; i < 4; i++)
                      {
                        int offset = 5*i;
                        e += rotate(a,5) + f1(b,c,d) + words[offset  ] + 0x5a827999; b = rotate(b,30);
                        d += rotate(e,5) + f1(a,b,c) + words[offset+1] + 0x5a827999; a = rotate(a,30);
                        c += rotate(d,5) + f1(e,a,b) + words[offset+2] + 0x5a827999; e = rotate(e,30);
                        b += rotate(c,5) + f1(d,e,a) + words[offset+3] + 0x5a827999; d = rotate(d,30);
                        a += rotate(b,5) + f1(c,d,e) + words[offset+4] + 0x5a827999; c = rotate(c,30);
                      }

                      // second round
                      for (int i = 4; i < 8; i++)
                      {
                        int offset = 5*i;
                        e += rotate(a,5) + f2(b,c,d) + words[offset  ] + 0x6ed9eba1; b = rotate(b,30);
                        d += rotate(e,5) + f2(a,b,c) + words[offset+1] + 0x6ed9eba1; a = rotate(a,30);
                        c += rotate(d,5) + f2(e,a,b) + words[offset+2] + 0x6ed9eba1; e = rotate(e,30);
                        b += rotate(c,5) + f2(d,e,a) + words[offset+3] + 0x6ed9eba1; d = rotate(d,30);
                        a += rotate(b,5) + f2(c,d,e) + words[offset+4] + 0x6ed9eba1; c = rotate(c,30);
                      }

                      // third round
                      for (int i = 8; i < 12; i++)
                      {
                        int offset = 5*i;
                        e += rotate(a,5) + f3(b,c,d) + words[offset  ] + 0x8f1bbcdc; b = rotate(b,30);
                        d += rotate(e,5) + f3(a,b,c) + words[offset+1] + 0x8f1bbcdc; a = rotate(a,30);
                        c += rotate(d,5) + f3(e,a,b) + words[offset+2] + 0x8f1bbcdc; e = rotate(e,30);
                        b += rotate(c,5) + f3(d,e,a) + words[offset+3] + 0x8f1bbcdc; d = rotate(d,30);
                        a += rotate(b,5) + f3(c,d,e) + words[offset+4] + 0x8f1bbcdc; c = rotate(c,30);
                      }

                      // fourth round
                      for (int i = 12; i < 16; i++)
                      {
                        int offset = 5*i;
                        e += rotate(a,5) + f2(b,c,d) + words[offset  ] + 0xca62c1d6; b = rotate(b,30);
                        d += rotate(e,5) + f2(a,b,c) + words[offset+1] + 0xca62c1d6; a = rotate(a,30);
                        c += rotate(d,5) + f2(e,a,b) + words[offset+2] + 0xca62c1d6; e = rotate(e,30);
                        b += rotate(c,5) + f2(d,e,a) + words[offset+3] + 0xca62c1d6; d = rotate(d,30);
                        a += rotate(b,5) + f2(c,d,e) + words[offset+4] + 0xca62c1d6; c = rotate(c,30);
                      }

                      // update hash
                      m_hash[0] += a;
                      m_hash[1] += b;
                      m_hash[2] += c;
                      m_hash[3] += d;
                      m_hash[4] += e;
                  }
                  /// process everything left in the internal buffer
                  void processBuffer(){
                      // the input bytes are considered as bits strings, where the first bit is the most significant bit of the byte

                      // - append "1" bit to message
                      // - append "0" bits until message length in bit mod 512 is 448
                      // - append length as 64 bit integer

                      // number of bits
                      size_t paddedLength = m_bufferSize * 8;

                      // plus one bit set to 1 (always appended)
                      paddedLength++;

                      // number of bits must be (numBits % 512) = 448
                      size_t lower11Bits = paddedLength & 511;
                      if (lower11Bits <= 448)
                        paddedLength +=       448 - lower11Bits;
                      else
                        paddedLength += 512 + 448 - lower11Bits;
                      // convert from bits to bytes
                      paddedLength /= 8;

                      // only needed if additional data flows over into a second block
                      unsigned char extra[BlockSize];

                      // append a "1" bit, 128 => binary 10000000
                      if (m_bufferSize < BlockSize)
                        m_buffer[m_bufferSize] = 128;
                      else
                        extra[0] = 128;

                      size_t i;
                      for (i = m_bufferSize + 1; i < BlockSize; i++)
                        m_buffer[i] = 0;
                      for (; i < paddedLength; i++)
                        extra[i - BlockSize] = 0;

                      // add message length in bits as 64 bit number
                      uint64_t msgBits = 8 * (m_numBytes + m_bufferSize);
                      // find right position
                      unsigned char* addLength;
                      if (paddedLength < BlockSize)
                        addLength = m_buffer + paddedLength;
                      else
                        addLength = extra + paddedLength - BlockSize;

                      // must be big endian
                      *addLength++ = (unsigned char)((msgBits >> 56) & 0xFF);
                      *addLength++ = (unsigned char)((msgBits >> 48) & 0xFF);
                      *addLength++ = (unsigned char)((msgBits >> 40) & 0xFF);
                      *addLength++ = (unsigned char)((msgBits >> 32) & 0xFF);
                      *addLength++ = (unsigned char)((msgBits >> 24) & 0xFF);
                      *addLength++ = (unsigned char)((msgBits >> 16) & 0xFF);
                      *addLength++ = (unsigned char)((msgBits >>  8) & 0xFF);
                      *addLength   = (unsigned char)( msgBits        & 0xFF);

                      // process blocks
                      processBlock(m_buffer);
                      // flowed over into a second block ?
                      if (paddedLength > BlockSize)
                        processBlock(extra);
                  }

                  /// size of processed data in bytes
                  uint64_t m_numBytes;
                  /// valid bytes in m_buffer
                  size_t   m_bufferSize;
                  /// bytes not processed yet
                  uint8_t  m_buffer[BlockSize];

                  enum { HashValues = HashBytes / 4 };
                  /// hash, stored as integers
                  uint32_t m_hash[HashValues];
                };

                {
                    SHA1 sha1_checksum;

                    std::string hash1 = "";
                    std::string hash2 = "";

                    hash1 = sha1_checksum(c1);
                    hash2 = sha1_checksum(c2);

                    return (hash1.compare(hash2) == 0) && (hash1 == hash2);
                }
            }

            /**
             * @brief Stupid checksum comparer.
             * 
             * @details
             * 
             * SSS means Stupid Simple Summing.
             * 
             * It is called that because it uses stupid summing of char to size_t
             * 
             * @param c1 string 1
             * @param c2 string 2
             * @return true match
             * @return false no
             */
            bool compareStringSSS(std::string c1, std::string c2){
                bool simplesum_status = true;
                {
                    std::istringstream in_cmp_1(c1);
                    std::istringstream in_cmp_2(c2);

                    // simple char to long summing checksum
                    typedef size_t lsize_t; // yeah
                    lsize_t simplesum_1 = 0;
                    lsize_t simplesum_2 = 0;

                    char ch1 = 0;
                    char ch2 = 0;

                    while(in_cmp_1.get(ch1)){
                        simplesum_1 += ch1;
                    }
                    
                    while(in_cmp_2.get(ch2)){
                        simplesum_2 += ch2;
                    }

                    simplesum_status = simplesum_1 == simplesum_2;
                }
                return simplesum_status;
            }

            /**
             * @brief MD5 checksum comparer. We do not need to know the hash
             * 
             * @details
             * 
             * from https://github.com/stbrumme/hash-library/blob/master/md5.hpp and https://github.com/stbrumme/hash-library/blob/master/md5.cpp, spliced together
             * 
             * The copyright notice for the source
             * 
             * zlib License
             * 
             * Copyright (c) 2014,2015 Stephan Brumme
             * 
             * This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
             * Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
             * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
             *    If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
             * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
             * 3. This notice may not be removed or altered from any source distribution.
             * 
             * 
             * @param c1 string 1
             * @param c2 string 2
             * @return true same checksum
             * @return false bad chksum
             */
            bool compareStringMD5(std::string c1, std::string c2){
                            // full blown md5 checksum
                            // from https://github.com/stbrumme/hash-library/blob/master/md5.hpp and https://github.com/stbrumme/hash-library/blob/master/md5.cpp, spliced together
                            // The copyright notice for the source
                            // zlib License
                            // 
                            // Copyright (c) 2014,2015 Stephan Brumme
                            // 
                            // This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
                            // Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
                            // 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
                            //    If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
                            // 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
                            // 3. This notice may not be removed or altered from any source distribution.


                            #ifdef _MSC_VER
                            // Windows
                            typedef unsigned __int8  uint8_t;
                            typedef unsigned __int32 uint32_t;
                            typedef unsigned __int64 uint64_t;
                            #endif

                              // mix functions for processBlock()
                              static auto f1 = [](uint32_t b, uint32_t c, uint32_t d) -> uint32_t
                              {
                                return d ^ (b & (c ^ d)); // original: f = (b & c) | ((~b) & d);
                              };

                              static auto f2 = [](uint32_t b, uint32_t c, uint32_t d) -> uint32_t
                              {
                                return c ^ (d & (b ^ c)); // original: f = (b & d) | (c & (~d));
                              };

                              static auto f3 = [](uint32_t b, uint32_t c, uint32_t d) -> uint32_t
                              {
                                return b ^ c ^ d;
                              };

                              static auto f4 = [](uint32_t b, uint32_t c, uint32_t d) -> uint32_t
                              {
                                return c ^ (b | ~d);
                              };

                              static auto rotate = [](uint32_t a, uint32_t c) -> uint32_t
                              {
                                return (a << c) | (a >> (32 - c));
                              };

                            #if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
                              static auto swap = [](uint32_t x) -> uint32
                              {
                                #if defined(__GNUC__) || defined(__clang__)
                                    return __builtin_bswap32(x);
                                #elif defined(MSC_VER)
                                    return _byteswap_ulong(x);
                                #else

                                return (x >> 24) |
                                      ((x >>  8) & 0x0000FF00) |
                                      ((x <<  8) & 0x00FF0000) |
                                       (x << 24);
                                };
                                #endif
                            #endif

                            class MD5 //: public Hash
                            {
                            public:
                              // split into 64 byte blocks (=> 512 bits), hash is 16 bytes long
                              enum { BlockSize = 512 / 8, HashBytes = 16 };

                              // same as reset()
                              MD5(){
                                reset();
                              }

                              // compute MD5 of a memory block
                              std::string operator()(const void* data, size_t numBytes) {
                              reset();
                              add(data, numBytes);
                              return getHash();
                            }
                              // compute MD5 of a string, excluding final zero
                              std::string operator()(const std::string& text) {
                              reset();
                              add(text.c_str(), text.size());
                              return getHash();
                            }

                              // add arbitrary number of bytes
                              void add(const void* data, size_t numBytes){
                                const uint8_t* current = (const uint8_t*) data;

                              if (m_bufferSize > 0)
                              {
                                while (numBytes > 0 && m_bufferSize < BlockSize)
                                {
                                  m_buffer[m_bufferSize++] = *current++;
                                  numBytes--;
                                }
                              }

                              // full buffer
                              if (m_bufferSize == BlockSize)
                              {
                                processBlock(m_buffer);
                                m_numBytes  += BlockSize;
                                m_bufferSize = 0;
                              }

                              // no more data ?
                              if (numBytes == 0)
                                return;

                              // process full blocks
                              while (numBytes >= BlockSize)
                              {
                                processBlock(current);
                                current    += BlockSize;
                                m_numBytes += BlockSize;
                                numBytes   -= BlockSize;
                              }

                              // keep remaining bytes in buffer
                              while (numBytes > 0)
                              {
                                m_buffer[m_bufferSize++] = *current++;
                                numBytes--;
                              }
                              }

                              // return latest hash as 32 hex characters
                              std::string getHash(){
                            // compute hash (as raw bytes)
                              unsigned char rawHash[HashBytes];
                              getHash(rawHash);

                              // convert to hex string
                              std::string result;
                              result.reserve(2 * HashBytes);
                              for (int i = 0; i < HashBytes; i++)
                              {
                                static const char dec2hex[16+1] = "0123456789abcdef";
                                result += dec2hex[(rawHash[i] >> 4) & 15];
                                result += dec2hex[ rawHash[i]       & 15];
                              }

                              return result;
                              }
                              // return latest hash as bytes
                              void        getHash(unsigned char buffer[HashBytes]){
                              // save old hash if buffer is partially filled
                              uint32_t oldHash[HashValues];
                              for (int i = 0; i < HashValues; i++)
                                oldHash[i] = m_hash[i];

                              // process remaining bytes
                              processBuffer();

                              unsigned char* current = buffer;
                              for (int i = 0; i < HashValues; i++)
                              {
                                *current++ =  m_hash[i]        & 0xFF;
                                *current++ = (m_hash[i] >>  8) & 0xFF;
                                *current++ = (m_hash[i] >> 16) & 0xFF;
                                *current++ = (m_hash[i] >> 24) & 0xFF;

                                // restore old hash
                                m_hash[i] = oldHash[i];
                              }
                              }

                              // restart
                              void reset(){
                                m_numBytes   = 0;
                                m_bufferSize = 0;

                                // according to RFC 1321
                                m_hash[0] = 0x67452301;
                                m_hash[1] = 0xefcdab89;
                                m_hash[2] = 0x98badcfe;
                                m_hash[3] = 0x10325476;
                              }

                            private:
                              // process 64 bytes
                              void processBlock(const void* data){
                              // get last hash
                              uint32_t a = m_hash[0];
                              uint32_t b = m_hash[1];
                              uint32_t c = m_hash[2];
                              uint32_t d = m_hash[3];

                              // data represented as 16x 32-bit words
                              const uint32_t* words = (uint32_t*) data;

                              // computations are little endian, swap data if necessary
                              static auto LITTLEENDIAN = [](uint32_t x) -> uint32_t{
                                #if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
                                return swap(x);
                                #else
                                return x;
                                #endif
                              };

                              // first round
                              uint32_t word0  = LITTLEENDIAN(words[ 0]);
                              a = rotate(a + f1(b,c,d) + word0  + 0xd76aa478,  7) + b;
                              uint32_t word1  = LITTLEENDIAN(words[ 1]);
                              d = rotate(d + f1(a,b,c) + word1  + 0xe8c7b756, 12) + a;
                              uint32_t word2  = LITTLEENDIAN(words[ 2]);
                              c = rotate(c + f1(d,a,b) + word2  + 0x242070db, 17) + d;
                              uint32_t word3  = LITTLEENDIAN(words[ 3]);
                              b = rotate(b + f1(c,d,a) + word3  + 0xc1bdceee, 22) + c;

                              uint32_t word4  = LITTLEENDIAN(words[ 4]);
                              a = rotate(a + f1(b,c,d) + word4  + 0xf57c0faf,  7) + b;
                              uint32_t word5  = LITTLEENDIAN(words[ 5]);
                              d = rotate(d + f1(a,b,c) + word5  + 0x4787c62a, 12) + a;
                              uint32_t word6  = LITTLEENDIAN(words[ 6]);
                              c = rotate(c + f1(d,a,b) + word6  + 0xa8304613, 17) + d;
                              uint32_t word7  = LITTLEENDIAN(words[ 7]);
                              b = rotate(b + f1(c,d,a) + word7  + 0xfd469501, 22) + c;

                              uint32_t word8  = LITTLEENDIAN(words[ 8]);
                              a = rotate(a + f1(b,c,d) + word8  + 0x698098d8,  7) + b;
                              uint32_t word9  = LITTLEENDIAN(words[ 9]);
                              d = rotate(d + f1(a,b,c) + word9  + 0x8b44f7af, 12) + a;
                              uint32_t word10 = LITTLEENDIAN(words[10]);
                              c = rotate(c + f1(d,a,b) + word10 + 0xffff5bb1, 17) + d;
                              uint32_t word11 = LITTLEENDIAN(words[11]);
                              b = rotate(b + f1(c,d,a) + word11 + 0x895cd7be, 22) + c;

                              uint32_t word12 = LITTLEENDIAN(words[12]);
                              a = rotate(a + f1(b,c,d) + word12 + 0x6b901122,  7) + b;
                              uint32_t word13 = LITTLEENDIAN(words[13]);
                              d = rotate(d + f1(a,b,c) + word13 + 0xfd987193, 12) + a;
                              uint32_t word14 = LITTLEENDIAN(words[14]);
                              c = rotate(c + f1(d,a,b) + word14 + 0xa679438e, 17) + d;
                              uint32_t word15 = LITTLEENDIAN(words[15]);
                              b = rotate(b + f1(c,d,a) + word15 + 0x49b40821, 22) + c;

                              // second round
                              a = rotate(a + f2(b,c,d) + word1  + 0xf61e2562,  5) + b;
                              d = rotate(d + f2(a,b,c) + word6  + 0xc040b340,  9) + a;
                              c = rotate(c + f2(d,a,b) + word11 + 0x265e5a51, 14) + d;
                              b = rotate(b + f2(c,d,a) + word0  + 0xe9b6c7aa, 20) + c;

                              a = rotate(a + f2(b,c,d) + word5  + 0xd62f105d,  5) + b;
                              d = rotate(d + f2(a,b,c) + word10 + 0x02441453,  9) + a;
                              c = rotate(c + f2(d,a,b) + word15 + 0xd8a1e681, 14) + d;
                              b = rotate(b + f2(c,d,a) + word4  + 0xe7d3fbc8, 20) + c;

                              a = rotate(a + f2(b,c,d) + word9  + 0x21e1cde6,  5) + b;
                              d = rotate(d + f2(a,b,c) + word14 + 0xc33707d6,  9) + a;
                              c = rotate(c + f2(d,a,b) + word3  + 0xf4d50d87, 14) + d;
                              b = rotate(b + f2(c,d,a) + word8  + 0x455a14ed, 20) + c;

                              a = rotate(a + f2(b,c,d) + word13 + 0xa9e3e905,  5) + b;
                              d = rotate(d + f2(a,b,c) + word2  + 0xfcefa3f8,  9) + a;
                              c = rotate(c + f2(d,a,b) + word7  + 0x676f02d9, 14) + d;
                              b = rotate(b + f2(c,d,a) + word12 + 0x8d2a4c8a, 20) + c;

                              // third round
                              a = rotate(a + f3(b,c,d) + word5  + 0xfffa3942,  4) + b;
                              d = rotate(d + f3(a,b,c) + word8  + 0x8771f681, 11) + a;
                              c = rotate(c + f3(d,a,b) + word11 + 0x6d9d6122, 16) + d;
                              b = rotate(b + f3(c,d,a) + word14 + 0xfde5380c, 23) + c;

                              a = rotate(a + f3(b,c,d) + word1  + 0xa4beea44,  4) + b;
                              d = rotate(d + f3(a,b,c) + word4  + 0x4bdecfa9, 11) + a;
                              c = rotate(c + f3(d,a,b) + word7  + 0xf6bb4b60, 16) + d;
                              b = rotate(b + f3(c,d,a) + word10 + 0xbebfbc70, 23) + c;

                              a = rotate(a + f3(b,c,d) + word13 + 0x289b7ec6,  4) + b;
                              d = rotate(d + f3(a,b,c) + word0  + 0xeaa127fa, 11) + a;
                              c = rotate(c + f3(d,a,b) + word3  + 0xd4ef3085, 16) + d;
                              b = rotate(b + f3(c,d,a) + word6  + 0x04881d05, 23) + c;

                              a = rotate(a + f3(b,c,d) + word9  + 0xd9d4d039,  4) + b;
                              d = rotate(d + f3(a,b,c) + word12 + 0xe6db99e5, 11) + a;
                              c = rotate(c + f3(d,a,b) + word15 + 0x1fa27cf8, 16) + d;
                              b = rotate(b + f3(c,d,a) + word2  + 0xc4ac5665, 23) + c;

                              // fourth round
                              a = rotate(a + f4(b,c,d) + word0  + 0xf4292244,  6) + b;
                              d = rotate(d + f4(a,b,c) + word7  + 0x432aff97, 10) + a;
                              c = rotate(c + f4(d,a,b) + word14 + 0xab9423a7, 15) + d;
                              b = rotate(b + f4(c,d,a) + word5  + 0xfc93a039, 21) + c;

                              a = rotate(a + f4(b,c,d) + word12 + 0x655b59c3,  6) + b;
                              d = rotate(d + f4(a,b,c) + word3  + 0x8f0ccc92, 10) + a;
                              c = rotate(c + f4(d,a,b) + word10 + 0xffeff47d, 15) + d;
                              b = rotate(b + f4(c,d,a) + word1  + 0x85845dd1, 21) + c;

                              a = rotate(a + f4(b,c,d) + word8  + 0x6fa87e4f,  6) + b;
                              d = rotate(d + f4(a,b,c) + word15 + 0xfe2ce6e0, 10) + a;
                              c = rotate(c + f4(d,a,b) + word6  + 0xa3014314, 15) + d;
                              b = rotate(b + f4(c,d,a) + word13 + 0x4e0811a1, 21) + c;

                              a = rotate(a + f4(b,c,d) + word4  + 0xf7537e82,  6) + b;
                              d = rotate(d + f4(a,b,c) + word11 + 0xbd3af235, 10) + a;
                              c = rotate(c + f4(d,a,b) + word2  + 0x2ad7d2bb, 15) + d;
                              b = rotate(b + f4(c,d,a) + word9  + 0xeb86d391, 21) + c;

                              // update hash
                              m_hash[0] += a;
                              m_hash[1] += b;
                              m_hash[2] += c;
                              m_hash[3] += d;
                            }

                              // process everything left in the internal buffer
                              void processBuffer(){
                                                              // the input bytes are considered as bits strings, where the first bit is the most significant bit of the byte

                              // - append "1" bit to message
                              // - append "0" bits until message length in bit mod 512 is 448
                              // - append length as 64 bit integer

                              // number of bits
                              size_t paddedLength = m_bufferSize * 8;

                              // plus one bit set to 1 (always appended)
                              paddedLength++;

                              // number of bits must be (numBits % 512) = 448
                              size_t lower11Bits = paddedLength & 511;
                              if (lower11Bits <= 448)
                                paddedLength +=       448 - lower11Bits;
                              else
                                paddedLength += 512 + 448 - lower11Bits;
                              // convert from bits to bytes
                              paddedLength /= 8;

                              // only needed if additional data flows over into a second block
                              unsigned char extra[BlockSize];

                              // append a "1" bit, 128 => binary 10000000
                              if (m_bufferSize < BlockSize)
                                m_buffer[m_bufferSize] = 128;
                              else
                                extra[0] = 128;

                              size_t i;
                              for (i = m_bufferSize + 1; i < BlockSize; i++)
                                m_buffer[i] = 0;
                              for (; i < paddedLength; i++)
                                extra[i - BlockSize] = 0;

                              // add message length in bits as 64 bit number
                              uint64_t msgBits = 8 * (m_numBytes + m_bufferSize);
                              // find right position
                              unsigned char* addLength;
                              if (paddedLength < BlockSize)
                                addLength = m_buffer + paddedLength;
                              else
                                addLength = extra + paddedLength - BlockSize;

                              // must be little endian
                              *addLength++ = msgBits & 0xFF; msgBits >>= 8;
                              *addLength++ = msgBits & 0xFF; msgBits >>= 8;
                              *addLength++ = msgBits & 0xFF; msgBits >>= 8;
                              *addLength++ = msgBits & 0xFF; msgBits >>= 8;
                              *addLength++ = msgBits & 0xFF; msgBits >>= 8;
                              *addLength++ = msgBits & 0xFF; msgBits >>= 8;
                              *addLength++ = msgBits & 0xFF; msgBits >>= 8;
                              *addLength++ = msgBits & 0xFF;

                              // process blocks
                              processBlock(m_buffer);
                              // flowed over into a second block ?
                              if (paddedLength > BlockSize)
                                processBlock(extra);
                              }

                              // size of processed data in bytes
                              uint64_t m_numBytes;
                              // valid bytes in m_buffer
                              size_t   m_bufferSize;
                              // bytes not processed yet
                              uint8_t  m_buffer[BlockSize];

                              enum { HashValues = HashBytes / 4 };
                              // hash, stored as integers
                              uint32_t m_hash[HashValues];
                            };

                            // do not mind the code above ok



                            {
                                // original code that is not copied from that source above
                                MD5 md5_checksum_hash;

                                std::string hash_1 = md5_checksum_hash(c1);
                                std::string hash_2 = md5_checksum_hash(c2);

                                return (hash_1.compare(hash_2) == 0) && (hash_1 == hash_2);
                            }
            }

            /**
             * @brief Compare string with CRC32. Also don't need to know crc32 result
             * 
             * @details
             * 
             * generated by pycrc using the crc-32 model, the table driven algorithm and std is c99
             * 
             * @param c1 string 1
             * @param c2 string 2
             * @return true yes same
             * @return false no same
             */
            bool compareStringCRC32(std::string c1, std::string c2){
                // generated by pycrc using the crc-32 model, the table driven algorithm and std is c99
                typedef uint_fast32_t crc_t;

                static auto crc_init = []() -> crc_t {
                    return (crc_t) 0xffffffff;
                };

                static auto crc_finalize = [](crc_t crc) -> crc_t {
                    return crc ^ 0xffffffff;
                };

                static const crc_t crc_table[256] = {
                    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
                    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
                    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
                    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
                    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
                    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
                    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
                    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
                    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
                    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
                    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
                    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
                    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
                    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
                    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
                    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
                    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
                    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
                    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
                    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
                    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
                    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
                    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
                    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
                    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
                    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
                    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
                    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
                    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
                    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
                    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
                    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
                };

                static auto crc_update = [](crc_t crc, const void *data, size_t data_len) -> crc_t {
                    const unsigned char *d = (const unsigned char *)data;
                    unsigned int tbl_idx;

                    while (data_len--) {
                        tbl_idx = (crc ^ *d) & 0xff;
                        crc = (crc_table[tbl_idx] ^ (crc >> 8)) & 0xffffffff;
                        d++;
                    }
                    return crc & 0xffffffff;
                };

                {
                    crc_t crc1 = crc_init();
                    crc_t crc2 = crc_init();

                    crc1 = crc_update(crc1, (unsigned char*) c1.c_str(), std::strlen(c1.c_str()));
                    crc2 = crc_update(crc1, (unsigned char*) c2.c_str(), std::strlen(c2.c_str()));

                    size_t crc1_f = crc_finalize(crc1);
                    size_t crc2_f = crc_finalize(crc2);

                    return crc1_f == crc2_f;
                }
            }

            private:
            /**
             * @brief Stupid typedef
             * 
             */
            typedef unsigned long long int tapdef_t; // why not

            /**
             * @brief Why not a va_list
             * 
             */
            std::va_list stupid_vaargs;
            /**
             * @brief More wasted memory!
             * 
             */
            std::va_list vaargs_list;

            protected: // first
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
             * @brief Syncrhonization mutex, you can say this is the GIL (if this is a language interpreter, but yes i know if yu read after this) if this was a language interpreter.
             * 
             * @note This mutex is mentioned in the note of this class, this synchronizes the class
             */
            std::recursive_mutex lock_mutex;
            /**
             * @brief Config to modify the runtime behaviour of the string builder and editor
             * 
             */
            std::map<std::string, std::string> strEditorConfig;

            public: // first

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
             * @brief Should help if displaying garbage when using the repl by disabling color. Normally has color when supported.
             * 
             */
            const std::string nocolor = "nocolor";

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
            MSLedit(MSLedit<StringType>& ledit);
            /**
             * @brief Construct a new MSLedit object by copying another object, but dynamically allocated
             * 
             * @param ledit the other dynamically allocated object
             */
            MSLedit(MSLedit<StringType>* ledit);
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
            void setInstance(MSLedit<StringType>& other);
            /**
             * @brief Set the Instance object to another MSLedit instance, constant edition
             * 
             * @param cother other instance, constant edition
             */
            void setInstance(const MSLedit<StringType>& cother);
            /**
             * @brief Set the Instance object, pointer edition
             * 
             * @param other other instance, pointer edition
             */
            void setInstance(MSLedit<StringType>* other);

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
             * @warning default implementation (the one in the header file) is SLOW AS A SLOTH (SLOTHY) and broken. Don't even use this.
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
             * @brief Obtain iterator from buffer
             * 
             * @return std::vector<std::string>::iterator iterator
             */
            std::vector<std::string>::iterator getIterator();
            /**
             * @brief Get the end marker of the iterator
             * 
             * @return std::vector<std::string>::iterator ending point
             */
            std::vector<std::string>::iterator getIteratorEnd();

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
             * @brief Get the Text object as a raw, unformatted string
             * 
             * @return std::string the string
             */
            std::string getRawText();
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
            void setTextOfInstance(MSLedit<StringType>& other_miss_ledit_instance);
            /**
             * @brief Like setTextOfInstance, but swaps the content instead of copying
             * 
             * @param other_miss_ledit_instance that other instance
             * 
             * @see setTextOfInstance
             */
            void swap(MSLedit<StringType>& other_miss_ledit_instance);
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
             * @see getText
             * @see getUniqueCString
             * @see getSharedCString
             * 
             * @note You shoudld deallocate this string when done (unique_ptr is helpful, in fact there is that function to help you called getUniqueCString and getSharedCString). This allocates a new string (to get around clang lmao)
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
             * @see getCString
             * @note See the note in getCString, this string should be deallocated when done. You should have done it.
             */
            const char* getConstCString(bool formatted, long int begin, long int end);
            /**
             * @brief Get the C String in a smart pointer (unique_ptr). Uses getCString internally.
             * 
             * @param formatted should be formatted with a line numbering?
             * @param begin which line should be the starting point? -1 or below for the very beginning
             * @param end which line should be the ending point? -1 or below for the very end (all)
             * @return std::unique_ptr<char[]> smart string (unique)
             * 
             * @see getCString
             */
            std::unique_ptr<char[]> getUniqueCString(bool formatted, long int begin, long int end);
            /**
             * @brief Get the C String in a smart pointer (shared_ptr). Uses getCString internally.
             * 
             * @param formatted should be formatted with a line numbering?
             * @param begin which line should be the starting point? -1 or below for the very beginning
             * @param end which line should be the ending point? -1 or below for the very end (all)
             * @return std::shared_ptr<char[]> smart string (shared)
             * 
             @see getCString
             */
            std::shared_ptr<char[]> getSharedCString(bool formatted, long int begin, long int end);
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
             * @brief Dump the internal state of this instance to std::cout, internally uses the ostream version
             * 
             * @param formatted format the output as json if true
             */
            void dump(bool formatted);
            /**
             * @brief Dump the internal state of this instance to the stream, internally uses dumpstr
             * 
             * @param formatted format the output as json if true
             * @param stream the stream
             */
            void dump(bool formatted, std::ostream& stream);
            /**
             * @brief Dump the internal state of this instance as a return of type string
             * 
             * @param formatted format the output as json if true
             * @return std::string the dumped state
             */
            std::string dumpstr(bool formatted);

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
             * @brief Append a pointer. Uses append_printf
             *
             * @param ptr pointer
             * 
             * @see append_printf
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
             * @brief C Style printf to append. Uses append_vprintf to avoid duplication
             * 
             * @param format printf format
             * @param ... extra arguments
             * @return int status of the printf
             * 
             * @see append_vprintf
             */
            int append_printf(std::string format, ...);
            /**
             * @brief C Style printf to append, but accepts va_list instead
             * 
             * @param format 
             * @param vaargs 
             * @return int 
             * 
             * @note You need to va_start and va_end the va_list manually
             */
            int append_vprintf(std::string format, va_list vaargs);

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
             * @brief C-Style printf to insert at. Uses insert_vprintf to avoid duplication
             * 
             * @param position the position to append the string at
             * @param format printf format
             * @param ... your arguments if you know printf
             * @return int the status return from printf
             */
            int insert_printf(size_t position, std::string format, ...);
            /**
             * @brief C-Style printf to insert at, but accepts va_list.
             * 
             * @param position the position to append the string at
             * @param format printf format
             * @param vaargs va_list
             * @return int status from printf
             * 
             * @see You need to va_start and va_end the list manually
             */
            int insert_vprintf(size_t position, std::string format, va_list vaargs);

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
            int compare(MSLedit<StringType>& miss);
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
             * @brief Compare with stupid summing each character
             * 
             * @param mystr other string to compare with
             * @return true the checksum of this class' text matches with the checksum of mystr. This means that the text matches
             * @return false not match
             * 
             * @see compareStringSSS
             */
            bool stupidSimpleSummingCompare(std::string mystr);
            /**
             * @brief Compare with stupid summing of each charactee
             * 
             * @param miss other instance
             * @return true both checksum match
             * @return false no
             * 
             * @see compareStringSSS
             */
            bool stupidSimpleSummingCompare(MSLedit<StringType>& miss);
            /**
             * @brief Compare with MD5
             * 
             * @param mystr the sting to compare with
             * @return true the text of this class matches with mystr
             * @return false the text of this class does not match (at all)
             * 
             * @see compareStringMD5
             */
            bool md5Compare(std::string mystr);
            /**
             * @brief Compare with MD5 to another instance
             * 
             * @param miss other instance
             * @return true the text of this instance matches the other instance
             * @return false the text of this instance does not match (at all)
             * 
             * @see compareStringMD5
             */
            bool md5Compare(MSLedit<StringType>& miss);
            /**
             * @brief Compare with CRC32
             * 
             * @param mystr the sting to compare with
             * @return true the text of this class matches with mystr
             * @return false the text of this class does not match (at all)
             * 
             * @see compareStringCRC32
             */
            bool crc32Compare(std::string mystr);
            /**
             * @brief Compare with CRC32 to another instance
             * 
             * @param miss other instance
             * @return true the text of this instance matches the other instance
             * @return false the text of this instance does not match (at all)
             * 
             * @see compareStringCRC32
             */
            bool crc32Compare(MSLedit<StringType>& miss);
            /**
             * @brief Compare with SHA1
             * 
             * @param mystr the sting to compare with
             * @return true the text of this class matches with mystr
             * @return false the text of this class does not match (at all)
             * 
             * @see compareStringSHA1
             */
            bool sha1Compare(std::string mystr);
            /**
             * @brief Compare with SHA1 to another instance
             * 
             * @param miss other instance
             * @return true the text of this instance matches the other instance
             * @return false the text of this instance does not match (at all)
             * 
             * @see compareStringSHA1
             */
            bool sha1Compare(MSLedit<StringType>& miss);

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
            friend bool operator==(MSLedit<StringType>& miss, std::string& str) {return (miss.compare(str) == 0);}
            /**
             * @brief Am not equal
             * 
             * @param miss this class
             * @param str other string
             * @return true ye me no equal
             * @return false no sad me equal
             */
            friend bool operator!=(MSLedit<StringType>& miss, std::string& str) {return !(miss == str);}
            /**
             * @brief Am equal
             * 
             * @param miss this class
             * @param miss2 other stupid bingus of an MSLedit instance
             * @return true equal
             * @return false no sad
             */
            friend bool operator==(MSLedit<StringType>& miss, MSLedit<StringType>& miss2) {
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
            friend bool operator!=(MSLedit<StringType>& miss, MSLedit<StringType>& miss2) {return !(miss == miss2);}
            /**
             * @brief Am equal
             * 
             * @param miss this class
             * @param cstr other C String
             * @return true equal
             * @return false no sad
             */
            friend bool operator==(MSLedit<StringType>& miss, char* cstr) {return (miss.compare(cstr) == 0);}
            /**
             * @brief Am not equal
             * 
             * @param miss this class
             * @param cstr other C String
             * @return true ye me no equal
             * @return false no sad me equal
             */
            friend bool operator!=(MSLedit<StringType>& miss, char* cstr) {return !(miss == cstr);};
            /**
             * @brief Am equal
             * 
             * @param miss this class
             * @param ccstr other frozen chunk/block of icy/ice C String
             * @return true equal
             * @return false no sad
             */
            friend bool operator==(MSLedit<StringType>& miss, const char* ccstr) {return (miss.compare(ccstr) == 0);};
            /**
             * @brief Am not equal
             * 
             * @param miss this class
             * @param ccstr other frozen chunk/block of icy/ice C String
             * @return true ye me no equal
             * @return false no sad me equal
             */
            friend bool operator!=(MSLedit<StringType>& miss, const char* ccstr) {return !(miss == ccstr);}

            /**
             * @brief Output the formatted content of an instance to the stream
             * 
             * @param os output stream
             * @param miss the said instance
             * @return std::ostream& os
             */
            template<typename StringType_T> friend std::ostream& operator<<(std::ostream& os, MSLedit<StringType_T>& miss);
            /**
             * @brief Set the content of an instance from the stream
             * 
             * @param is input stream
             * @param miss the said instance
             * @return std::istream& is
             */
            template<typename StringType_T>friend std::istream& operator>>(std::istream& is, MSLedit<StringType_T>& miss);

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
             * @return MSLedit<StringType>& the other new one
             */
            MSLedit<StringType>& operator=(MSLedit<StringType>& miss){
                return operator=(const_cast<const MSLedit<StringType>&>(miss));
            }
            /**
             * @brief Also no more self assignment errors
             * 
             * @param cmiss that constant one
             * @return MSLedit<StringType>& the other new one
             */
            MSLedit<StringType>& operator=(const MSLedit<StringType>& cmiss){
                if(this != &cmiss) {
                    setInstance(cmiss);
                }

                return *this;
            }
            /**
             * @brief Also no more self assignment errors, pointer edition
             * 
             * @param miss that one, pointer edition
             * @return MSLedit<StringType>& the other boring old same new one
             */
            MSLedit<StringType>& operator=(MSLedit<StringType>* miss){
                return operator=(*miss);
            }

            /**
             * @brief Try operator this, genius implementation with stringstream right?
             * 
             * @param ccstr the string
             * @return MSLedit the instance that was rudely created with a stringstream
             *\/
            friend MSLedit<StringType> operator""_MSLedit(const char* ccstr){
                std::stringstream ss;
                ss << ccstr;
                MSLedit missledit(ss.str());
                return missledit;
            } */
        };

        #ifndef MXPSQL_MSLedit_No_Implementation

        template<typename StringType>
        MSLedit<StringType>::MSLedit(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            setKey(nprompt, "MSLedit> ");
            setKey(nosystem, "false");
            setKey(nobanner, "false");
            setKey(nocolor, "false");
        }

        template<typename StringType>
        MSLedit<StringType>::MSLedit(MSLedit<StringType>& ledit) : MSLedit() {
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            setInstance(ledit);
        }

        template<typename StringType>
        MSLedit<StringType>::MSLedit(MSLedit<StringType>* ledit) : MSLedit(*ledit) {}

        template<typename StringType>
        MSLedit<StringType>::MSLedit(std::string content) : MSLedit(false, content) {}

        template<typename StringType>
        MSLedit<StringType>::MSLedit(char* cstr) : MSLedit(std::string(cstr)) {}

        template<typename StringType>
        MSLedit<StringType>::MSLedit(const char* ccstr) : MSLedit(std::string(ccstr)) {}

        template<typename StringType>
        MSLedit<StringType>::MSLedit(std::vector<std::string> buffer) : MSLedit() {
            setBuffer(buffer);
        }

        template<typename StringType>
        MSLedit<StringType>::MSLedit(std::initializer_list<std::string> ilbuf) : MSLedit(std::vector<std::string>(ilbuf)) {}

        template<typename StringType>
        MSLedit<StringType>::MSLedit(std::vector<char> cbuffer) : MSLedit(){
            std::string s(cbuffer.begin(), cbuffer.end());
            setText(s);
        }

        template<typename StringType>
        MSLedit<StringType>::MSLedit(std::initializer_list<char> cilbuf) : MSLedit(std::vector<char>(cilbuf)) {}

        template<typename StringType>
        MSLedit<StringType>::MSLedit(bool fileinsteadofcontent, std::string obj) : MSLedit() {
            if(fileinsteadofcontent){
                readFile(obj);
            }
            else{
                setText(obj);
            }
        }

        template<typename StringType>
        void MSLedit<StringType>::setInstance(MSLedit<StringType>& other){
            this->setText(other.str());
            this->setConfig(other.getConfig());
        }

        template<typename StringType>
        void MSLedit<StringType>::setInstance(const MSLedit<StringType>& cother){
            this->setInstance(const_cast<MSLedit<StringType>&>(cother));
        }

        template<typename StringType>
        void MSLedit<StringType>::setInstance(MSLedit<StringType>* other){
            this->setInstance(*other);
        }

        template<typename StringType>
        size_t MSLedit<StringType>::lineNums(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return buffer.size();
        }

        template<typename StringType>
        size_t MSLedit<StringType>::length(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().length();
        }

        template<typename StringType>
        size_t MSLedit<StringType>::size(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().size();
        }

        template<typename StringType>
        std::string MSLedit<StringType>::stringAtLine(size_t line){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(line > lineNums() || line < 1){
                throw std::out_of_range("Attempting to access beyond array bounds");
            }

            return buffer.at(line-1);
        }

        template<typename StringType>
        std::pair<size_t, size_t> MSLedit<StringType>::getGridIndexFromStringIndex(size_t strindex){
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

        template<typename StringType>
        char MSLedit<StringType>::charAtPosition(size_t index){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            if(index < 1 || index > length()){
                throw std::out_of_range("Attempting to access beyond array bounds");
            }

            return t.at(index);
        }

        template<typename StringType>
        char MSLedit<StringType>::charAtGrid(size_t line, size_t index){
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


        template<typename StringType>
        std::vector<std::string>::iterator MSLedit<StringType>::getIterator(){
            return buffer.begin();
        }

        template<typename StringType>
        std::vector<std::string>::iterator MSLedit<StringType>::getIteratorEnd(){
            return buffer.end();
        }


        template<typename StringType>
        void MSLedit<StringType>::readFile(std::string path){
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

        template<typename StringType>
        void MSLedit<StringType>::writeFile(std::string path){
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

        template<typename StringType>
        void MSLedit<StringType>::writeFileLocked(std::string path){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string tmpfile = "";
            bool isTaken = true;
            while(isTaken){
                {
                    // make temp file
                    char tmpfile_tmp[L_tmpnam];
                    char* ret = std::tmpnam(tmpfile_tmp); // stfu compilers that warn about this (bad tmpnam), I am not using C-Style FILE* IO (Bad), POSIX mkstemp (Not portable), C++17 Filesystem API (NO C++17 only, C++11 must support).
                    if(((!ret) || (ret == NULL))){
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
                        // compare with char one by one and 4 checksum (md5, crc <16? 32? yep 32>, sha1, simple char to long summing)
                        // probably to overpowered, but we want safe

                        bool simplesum_status = compareStringSSS(
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_1),
                                std::istreambuf_iterator<char>()
                            ),
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_2),
                                std::istreambuf_iterator<char>()
                            )
                        );

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

                        bool md5_status = compareStringMD5(
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_1),
                                std::istreambuf_iterator<char>()
                            ),
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_2),
                                std::istreambuf_iterator<char>()
                            )
                        );
                        

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

                        bool crc32_status = compareStringCRC32(
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_1),
                                std::istreambuf_iterator<char>()
                            ),
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_2),
                                std::istreambuf_iterator<char>()
                            )
                        );             

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

                        bool sha1_status = compareStringSHA1(
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_1),
                                std::istreambuf_iterator<char>()
                            ),
                            std::string(
                                std::istreambuf_iterator<char>(in_cmp_2),
                                std::istreambuf_iterator<char>()
                            )
                        );

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
                        ) && md5_status 
                        && crc32_status 
                        && sha1_status 
                        && simplesum_status;

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


        template<typename StringType>
        void MSLedit<StringType>::setText(std::string text){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::istringstream iss(text);
            std::string token;
            std::vector<std::string> vectr;

            while(std::getline(iss, token, '\n')){
                vectr.push_back(token);
            }

            setBuffer(vectr);
        }

        template<typename StringType>
        std::string MSLedit<StringType>::getText(bool formatted, long int begin, long int end){
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
                    ss << space << ioneAsStr << "|" << nw[i] << "\n";
                }
                else {
                    ss << nw[i] << "\n";
                }
            }

            str = ss.str();
            return str;
        }


        template<typename StringType>
        void MSLedit<StringType>::setTextOfInstance(MSLedit<StringType>& other_miss_ledit_instance){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            setText(other_miss_ledit_instance.str());
        }

        template<typename StringType>
        void MSLedit<StringType>::swap(MSLedit<StringType>& other_miss_ledit_instance){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string tmp = other_miss_ledit_instance.str();
            other_miss_ledit_instance.setText(str());
            setText(tmp);
        }


        template<typename StringType>
        std::string MSLedit<StringType>::getText(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return getText(true, -1, -1);
        }

        template<typename StringType>
        std::string MSLedit<StringType>::getRawText(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return getText(false, -1, -1);
        }

        template<typename StringType>
        std::string MSLedit<StringType>::str(){
            return getText(false, -1, -1);
        }


        template<typename StringType>
        void MSLedit<StringType>::setCString(char* cstr){
            setText(std::string(cstr));
        }

        template<typename StringType>
        void MSLedit<StringType>::setCString(const char* ccstr){
            setCString((char*) ccstr);
        }

        template<typename StringType>
        char* MSLedit<StringType>::getCString(bool formatted, long int begin, long int end){
            std::string cp = getText(formatted, begin, end);
            char* cstr = new char[cp.length() + 1];
            cp.copy(cstr, cp.length() + 1);
            return cstr;
        }

        template<typename StringType>
        const char* MSLedit<StringType>::getConstCString(bool formatted, long int begin, long int end){
            return (const char*) getCString(formatted, begin, end);
        }

        template<typename StringType>
        std::unique_ptr<char[]> MSLedit<StringType>::getUniqueCString(bool formatted, long int begin, long int end){
            return std::unique_ptr<char[]>(getCString(formatted, begin, end));
        }

        template<typename StringType>
        std::shared_ptr<char[]> MSLedit<StringType>::getSharedCString(bool formatted, long int begin, long int end){
            return std::shared_ptr<char[]>(getCString(formatted, begin, end));
        }


        template<typename StringType>
        void MSLedit<StringType>::setBuffer(std::vector<std::string> buffer){
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


        template<typename StringType>
        std::vector<std::string> MSLedit<StringType>::getBuffer(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return buffer;
        }

        template<typename StringType>
        void MSLedit<StringType>::setBuffer(std::vector<char> cbuffer){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::vector<char> tmpcbuffer(cbuffer);
            std::string s(tmpcbuffer.begin(), tmpcbuffer.end());
            setText(s);
        }


        template<typename StringType>
        void MSLedit<StringType>::print(bool formatted, long int begin, long int end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            print(formatted, std::cout, begin, end);
        }

        template<typename StringType>
        void MSLedit<StringType>::print(bool formatted, std::ostream& stream, long int begin, long int end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            stream << getText(formatted, begin, end);
        }

        /* 
        template<typename StringType>
        void MSLedit<StringType>::print(bool formatted, std::stringstream& stream, long int begin, long int end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            print(formatted, stream, begin, end);
        } */


        template<typename StringType>
        void MSLedit<StringType>::dump(bool formatted){
            dump(formatted, std::cout);
        }

        template<typename StringType>
        void MSLedit<StringType>::dump(bool formatted, std::ostream& stream){
            stream << dumpstr(formatted);
        }

        template<typename StringType>
        std::string MSLedit<StringType>::dumpstr(bool formatted){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string state = "";
            std::stringstream ss;
            if(formatted){ // bad json
                ss << "{" << std::endl;
                {
                    // Generate config to json
                    ss << "\t\"ReplAndStrEditorConfig\": {" << std::endl;

                    {
                        std::map<std::string, std::string> maps = getConfig();
                        for(auto it = maps.begin(); it != maps.end(); it++){
                            ss << "\t\t\"" << it->first << "\": \"" << it->second << "\"";
                            if(std::next(it) != maps.end()){
                                ss << ", ";
                            }
                            ss << std::endl;
                        }
                    }

                    ss << "\t}, " << std::endl;
                }

                {
                    // dump last opened file
                    ss << "\t\"LastFileOpened\": \"" << file << "\", " << std::endl;
                }

                {
                    // Generate list of things in an array ( raw document )
                    ss << "\t\"StrArray\": [" << std::endl;

                    {
                        std::vector<std::string> Stringies = getBuffer();
                        for(size_t i = 0; i < Stringies.size(); i++){
                            std::string Stringy = Stringies.at(i);
                            ss << "\t\t\"" << Stringy << "\"";
                            if((i+1) < Stringies.size()){
                                ss << ", ";
                            }
                            ss << std::endl;
                        }
                    }

                    ss << "\t]" << std::endl;
                }
                ss << "}" << std::endl;
            }
            else{
                {
                    // Generate config
                    ss << "config: " << std::endl;
                    {
                        std::map<std::string, std::string> maps = getConfig();
                        for(auto it = maps.begin(); it != maps.end(); it++){
                            ss << "\t\"" << it->first << "\": \"" << it->second << "\"" << std::endl;
                        }
                    }
                }

                {
                    // dump last opened file
                    ss << "LastFileOpened: \"" << file << "\"" << std::endl;
                }

                {
                    // Generate list of things in an array ( raw document )
                    ss << "StrArray: " << std::endl;

                    {
                        std::vector<std::string> Stringies = getBuffer();
                        for(size_t i = 0; i < Stringies.size(); i++){
                            std::string Stringy = Stringies.at(i);
                            ss << "\t\"" << Stringy << "\"" << std::endl;
                        }
                    }

                }
            }
            state = ss.str();
            return state;
        }



        template<typename StringType>
        void MSLedit<StringType>::appendAtEnd(std::string line){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::istringstream iss(line);
            std::string token;
            while(std::getline(iss, token, '\n')) buffer.push_back(token);
        }

        template<typename StringType>
        void MSLedit<StringType>::appendAtLine(int linenum, std::string line){
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

        template<typename StringType>
        void MSLedit<StringType>::insertAtLine(int linenum, std::string line){
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

        template<typename StringType>
        std::pair<size_t, size_t> MSLedit<StringType>::search(std::string text2search){
            return search(text2search, 1);
        }

        template<typename StringType>
        std::pair<size_t, size_t> MSLedit<StringType>::search(std::string text2search, size_t begin_line){
            std::vector<std::pair<size_t, size_t>> s = search(text2search, begin_line, 1);
            if(s.size() < 1) return std::make_pair(std::string::npos, std::string::npos);
            else return s[0];
        }

        template<typename StringType>
        std::vector<std::pair<size_t, size_t>> MSLedit<StringType>::search(std::string text2search, size_t begin_line, size_t count){
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

        template<typename StringType>
        void MSLedit<StringType>::editLine(int linenum, std::string line){
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

        template<typename StringType>
        void MSLedit<StringType>::editChar(int index, char character){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            if(index > (int) length() || index < 1) throw std::out_of_range("Attempting to edit beyond array beyond");
            t[index-1] = character;
            setText(t);
        }

        template<typename StringType>
        void MSLedit<StringType>::deleteAtLine(int linenum){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(linenum > (int) lineNums() || linenum < 1) throw std::out_of_range("Attempting to print beyond array bound");
            buffer.erase(buffer.begin() + linenum - 1);
        }

        template<typename StringType>
        void MSLedit<StringType>::clear(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            buffer.clear();
        }


        template<typename StringType>
        void MSLedit<StringType>::append(char c){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string s = "";
            s += c;
            append(s);
        }

        template<typename StringType>
        void MSLedit<StringType>::append(char* cstr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            if(cstr == NULL && cstr == nullptr){
                append("");
                return;
            }
            append(const_cast<const char*>(cstr));
        }

        template<typename StringType>
        void MSLedit<StringType>::append(const char* ccstr){
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

        template<typename StringType>
        void MSLedit<StringType>::append(double d){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(d));
        }

        template<typename StringType>
        void MSLedit<StringType>::append(float f){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(f));
        }

        template<typename StringType>
        void MSLedit<StringType>::append(int i){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(i));
        }

        template<typename StringType>
        void MSLedit<StringType>::append(long int li){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(li));
        }

        template<typename StringType>
        void MSLedit<StringType>::append(size_t s){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(std::to_string(s));
        }

        template<typename StringType>
        void MSLedit<StringType>::append(bool boolean){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(boolean ? "true" : "false");
        }

        template<typename StringType>
        void MSLedit<StringType>::append(std::string str){
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

        template<typename StringType>
        void MSLedit<StringType>::append(MSLedit miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(miss.str());
        }

        template<typename StringType>
        void MSLedit<StringType>::append(void* ptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append_printf("%p", ptr);
        }

        template<typename StringType>
        void MSLedit<StringType>::append(std::nullptr_t nptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(((void*) nptr));
        }

        template<typename StringType>
        void MSLedit<StringType>::append(long int* liptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append(((void*) liptr));
        }

        template<typename StringType>
        void MSLedit<StringType>::appendNewLine(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            append('\n');
        }

        template<typename StringType>
        int MSLedit<StringType>::append_printf(std::string format, ...){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            va_list vaa;
            int status = 0;
            va_start(vaa, format);
            status = append_vprintf(format, vaa);
            va_end(vaa);
            return status;
        }

        template<typename StringType>
        int MSLedit<StringType>::append_vprintf(std::string format, va_list vaargs){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return insert_vprintf(size(), format, vaargs);
        }


        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, bool boolean){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, boolean ? "true" : "false");
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, char* cstr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::string(cstr));
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, const char* ccstr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::string(ccstr));
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, int i){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::to_string(i));
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, long int li){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::to_string(li));
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, size_t s){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, std::to_string(s));
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, std::string estr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string tstr = str();
            tstr.insert(position, estr);

            setText(tstr);
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, MSLedit miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, miss.getText());
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, void* ptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert_printf(position, "%p", ptr);
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, std::nullptr_t nptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, ((void*) nptr));
        }

        template<typename StringType>
        void MSLedit<StringType>::insert(size_t position, long int* liptr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert(position, ((void*) liptr));
        }

        template<typename StringType>
        void MSLedit<StringType>::insertnewline(size_t position){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            insert('\n', position);
        }

        template<typename StringType>
        int MSLedit<StringType>::insert_printf(size_t position, std::string format, ...){
            va_list vaa;
            int status = 0;
            va_start(vaa, format);
            status = insert_vprintf(position, format, vaa);
            va_end(vaa);
            return status;
        }

        template<typename StringType>
        int MSLedit<StringType>::insert_vprintf(size_t position, std::string format, va_list vaargs){
            std::string ssstr;
            int status = 0;
            int size = std::vsnprintf(nullptr, 0, format.c_str(), vaargs);
            std::vector<char> autocstr(size + 1);
            if((status = std::vsnprintf(&autocstr[0], autocstr.size(), format.c_str(), vaargs)) < 0){
                throw std::length_error("Unable to write pointer to temporary buffer");
                return status;
            }
            for(char c : autocstr){
                ssstr += c;
            }
            insert(position, ssstr);
            return status;
        }


        template<typename StringType>
        void MSLedit<StringType>::deleteAt(size_t begin, size_t end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            t.erase(begin, end);
            setText(t);
        }

        template<typename StringType>
        void MSLedit<StringType>::deleteCharAt(size_t index){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            t.erase(index, 1);
            setText(t);
        }


        template<typename StringType>
        size_t MSLedit<StringType>::indexOf(std::string text){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            return t.find(text);
        }

        template<typename StringType>
        size_t MSLedit<StringType>::indexOf(std::string text, size_t begin){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().find(text, begin);
        }


        template<typename StringType>
        std::vector<std::string> MSLedit<StringType>::split(char delimiter){
            std::vector<std::string> misses;
            std::string t = str();
            std::string token;
            std::istringstream iss(t);
            for(token="";std::getline(iss, token, delimiter);misses.push_back(token));
            return misses;
        }

        template<typename StringType>
        std::vector<std::string> MSLedit<StringType>::split(std::string delimiter){
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


        template<typename StringType>
        std::string MSLedit<StringType>::substring(size_t pos){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().substr(pos);
        }

        template<typename StringType>
        std::string MSLedit<StringType>::substring(size_t pos, size_t end){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return str().substr(pos, end);
        }


        template<typename StringType>
        void MSLedit<StringType>::reverse(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::string t = str();
            std::string rt(t.rbegin(), t.rend());
            setText(rt);
        }


        template<typename StringType>
        void MSLedit<StringType>::setConfig(std::map<std::string, std::string> c2){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            strEditorConfig = c2;
        }

        template<typename StringType>
        void MSLedit<StringType>::parseConfig(std::string configPath){
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

        template<typename StringType>
        std::string MSLedit<StringType>::buildConfig(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            std::map<std::string, std::string>& data = strEditorConfig;
            std::stringstream ss;
            for(auto const& item : data){
                ss << item.first << "=" << item.second << std::endl;
            }
            return ss.str();
        }

        template<typename StringType>
        std::map<std::string, std::string> MSLedit<StringType>::getConfig(){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return strEditorConfig;
        }

        template<typename StringType>
        void MSLedit<StringType>::setKey(std::string key, std::string value){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            strEditorConfig[key] = value;
        }

        template<typename StringType>
        std::string MSLedit<StringType>::getKey(std::string key){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return strEditorConfig[key];
        }

        template<typename StringType>
        bool MSLedit<StringType>::keyExists(std::string key){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            auto it = strEditorConfig.find(key);
            return (it != strEditorConfig.end());
        }


        template<typename StringType>
        int MSLedit<StringType>::repl(){
            std::string prompt = "> ";
            {
                std::unique_lock<std::recursive_mutex> lock(lock_mutex);
                if(keyExists(nprompt)) prompt = getKey(nprompt);
            }

            return repl(prompt);
        }

        template<typename StringType>
        int MSLedit<StringType>::repl(std::string prompt){
            return repl(prompt, std::cout, std::cin, std::cerr);
        }

        template<typename StringType>
        int MSLedit<StringType>::repl(std::string prompt, std::ostream& out, std::istream& in, std::ostream& err){
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
                    
                    {
                        bool is_supported = false;
                        {
                            std::string term = "";
                            {
                                char* cterm = std::getenv("TERM");
                                if(cterm != NULL && cterm != nullptr){
                                    term = cterm;
                                }
                                else{
                                    term = "";
                                }
                            }

                            {
                                bool allowColor = true;
                                {
                                    std::unique_lock<std::recursive_mutex> lock(lock_mutex);
                                    if(keyExists(nocolor)) allowColor = (getKey(nocolor) != "true");
                                }

                                {
                                    bool isTermSupported = true;
                                    isTermSupported = ((term == "gnome-terminal") || (term == "xterm"));

                                    is_supported = allowColor && isTermSupported;
                                }
                            }
                        }

                        if(is_supported){
                            // print green
                            out << "\x1B[32m";
                        }

                        out << prompt;

                        if(is_supported){
                            // normalize
                            out << "\x1B[0m";
                        }
                    }

                    if(!std::getline(in, l, '\n')){
                        err << "Error getting input from user." << std::endl;
                        status = EXIT_FAILURE;
                        run = false;
                    }


                    {
                        bool no_use_comma_it_parenthesis_open_apostrophe_s_parenthesis_close_null_comma_empty_oxford_comma_and_slash_or_whitespace = false; // readable name: no use, it('s) null, empty, and/or whitespace

                        bool empty = l.length() < 1;
                        bool only_whitespace = l.find_first_not_of(" ") == std::string::npos;
                        bool only_tab = l.find_first_not_of('\t') == std::string::npos;

                        no_use_comma_it_parenthesis_open_apostrophe_s_parenthesis_close_null_comma_empty_oxford_comma_and_slash_or_whitespace = empty || only_whitespace || only_tab;

                        if(no_use_comma_it_parenthesis_open_apostrophe_s_parenthesis_close_null_comma_empty_oxford_comma_and_slash_or_whitespace){
                            continue;
                        }
                    }
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
                            out << std::endl;
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
                            out << std::endl;
                        }
                        #else
                        out << "?" << std::endl;
                        /* while(in.get() != 'q' && in.good()){
                            out << "?" << std::endl;
                        } */
                        #endif
                    }
                    else{
                        int hstatus = EXIT_FAILURE;

                        if(((replBeginHandler) && (replBeginHandler != nullptr))){
                            try{
                                hstatus = replBeginHandler(begin, args, arglen, out, in, err);
                            }
                            catch(std::bad_function_call& bfc){
                                err << "Bad REPL handler!" << std::endl << "Message btw: '" << bfc.what() << "'" << std::endl;
                                hstatus = EXIT_FAILURE;
                            }
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


        template<typename StringType>
        int MSLedit<StringType>::compare(std::string mystr){
            return str().compare(mystr);
        }

        template<typename StringType>
        int MSLedit<StringType>::compare(MSLedit<StringType>& miss){
            return compare(miss.str());
        }

        template<typename StringType>
        int MSLedit<StringType>::compare(char* cstr){
            return compare(std::string(cstr));
        }

        template<typename StringType>
        int MSLedit<StringType>::compare(const char* ccstr){
            return compare(std::string(ccstr));
        }


        template<typename StringType>
        bool MSLedit<StringType>::stupidSimpleSummingCompare(std::string mystr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return compareStringSSS(getText(false, -1, -1), mystr);
        }

        template<typename StringType>
        bool MSLedit<StringType>::stupidSimpleSummingCompare(MSLedit<StringType>& miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return stupidSimpleSummingCompare(miss.getText(false, -1, -1));
        }

        template<typename StringType>
        bool MSLedit<StringType>::md5Compare(std::string mystr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return compareStringMD5(getText(false, -1, -1), mystr);
        }

        template<typename StringType>
        bool MSLedit<StringType>::md5Compare(MSLedit<StringType>& miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return md5Compare(miss.getText(false, -1, -1));
        }

        template<typename StringType>
        bool MSLedit<StringType>::crc32Compare(std::string mystr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return compareStringCRC32(getText(false, -1, -1), mystr);
        }

        template<typename StringType>
        bool MSLedit<StringType>::crc32Compare(MSLedit<StringType>& miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return crc32Compare(miss.getText(false, -1, -1));
        }

        template<typename StringType>
        bool MSLedit<StringType>::sha1Compare(std::string mystr){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return compareStringSHA1(getText(false, -1, -1), mystr);
        }

        template<typename StringType>
        bool MSLedit<StringType>::sha1Compare(MSLedit<StringType>& miss){
            std::unique_lock<std::recursive_mutex> lock(lock_mutex);
            return sha1Compare(miss.getText(false, -1, -1));
        }


        /**
         * @brief Stream operatr, put to out stream
         * 
         * @tparam StringType type of the std::string-like string
         * @param os stream
         * @param miss the instance
         * @return std::ostream& os
         */
        template<typename StringType>
        std::ostream& operator<<(std::ostream& os, MSLedit<StringType>& miss){
            os << miss.getText();
            return os;
        }

        /**
         * @brief Stream operator, put to in stream
         * 
         * @tparam StringType type of the std::string-like string
         * @param is stream
         * @param miss the instance
         * @return std::ostream& is
         */
        template<typename StringType>
        std::istream& operator>>(std::istream& is, MSLedit<StringType>& miss){
            std::stringstream ss;
            std::string token;
            while(std::getline(is, token, '\n')) ss << token << '\n';
            miss.setText(ss.str());
            return is;
        }


        template<typename StringType>
        char MSLedit<StringType>::operator[](int index){
            return str()[index];
        }
        #endif

        /**
         * @brief std::string version of MSLedit
         * 
         */
        using MSLedit_Str = MSLedit<std::string>;
        /**
         * @brief std::wstring version of MSLedit
         * 
         */
        using MSLedit_WStr = MSLedit<std::wstring>;
        /**
         * @brief unsigned string version of MSLedit
         * 
         */
        using MSLedit_UStr = MSLedit<std::basic_string<unsigned char>>;
    };
};

#endif
