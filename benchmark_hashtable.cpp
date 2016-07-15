/*
 * milliways - B+ trees and key-value store C++ library
 *
 * Author: Marco Pantaleoni <marco.pantaleoni@gmail.com>
 * Copyright (C) 2016 Marco Pantaleoni. All rights reserved.
 *
 * Distributed under the Apache License, Version 2.0
 * See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 * The author licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include <vector>
#if defined(USE_STD_UNORDERED_MAP)
#include <unordered_map>
#elif defined(USE_TR1_UNORDERED_MAP)
#include <tr1/unordered_map>
#elif defined(USE_BOOST_UNORDERED_MAP)
#include <boost/unordered_map.hpp>
#endif
#include <map>

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include "hashtable.h"

/* ----------------------------------------------------------------- *
 *   PROTOTYPES                                                      *
 * ----------------------------------------------------------------- */

#if 0
static inline int rand_int(int lo, int hi);
static std::string random_string(int length);
static std::vector<std::string>& split(std::vector<std::string>& elems, const std::string& s, char delim);
static std::vector<std::string> split(const std::string& s, char delim);
#endif
static std::string trim(const std::string& str );
static bool startsWith(const std::string& s, const std::string& prefix);
#if 0
static bool endsWith(const std::string& s, const std::string& suffix);
static bool contains(const std::string& s, const std::string& infix);
#endif

#if 0
static inline double epoch_ms();
static inline void chrono_clear();
#endif
static inline void chrono_start();
static inline double chrono_stop();

static void benchmark_1();

int main(int argc, char* argv[]);


/* ----------------------------------------------------------------- *
 *   IMPLEMENTATION                                                  *
 * ----------------------------------------------------------------- */

#if 0
static inline int rand_int(int lo, int hi)
{
	return rand() % (hi - lo + 1) + lo;
}

static std::string random_string(int length)
{
	typedef std::string::size_type s_size_t;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string r(static_cast<s_size_t>(length), '\0');

	for (int i = 0; i < length; ++i)
		r[static_cast<s_size_t>(i)] = alphanum[rand_int(0, sizeof(alphanum) - 1)];

	return r;
}

static std::vector<std::string>& split(std::vector<std::string>& elems, const std::string& s, char delim)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
}

static std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    split(elems, s, delim);
    return elems;
}
#endif

static std::string trim(const std::string& str )
{
	static char const* whitespaceChars = "\n\r\t ";
	std::string::size_type start = str.find_first_not_of( whitespaceChars );
	std::string::size_type end = str.find_last_not_of( whitespaceChars );

	return start != std::string::npos ? str.substr( start, 1+end-start ) : "";
}

static bool startsWith(const std::string& s, const std::string& prefix)
{
	return (s.size() >= prefix.size()) && (s.substr(0, prefix.size()) == prefix);
}

#if 0
static bool endsWith(const std::string& s, const std::string& suffix)
{
	return (s.size() >= suffix.size()) && (s.substr(s.size() - suffix.size(), suffix.size()) == suffix);
}

static bool contains(const std::string& s, const std::string& infix)
{
	return s.find(infix) != std::string::npos;
}

static inline double epoch_ms()
{
	struct timeval tm;
    gettimeofday(&tm, NULL);
    double ms = static_cast<double>(
    	((static_cast<int64_t>(1000LL) * static_cast<int64_t>(tm.tv_sec)) +
    	 (static_cast<int64_t>(tm.tv_usec) / static_cast<int64_t>(1000LL)))
    	);
    return ms;
}
#endif

static struct timeval chrono_tm_start, chrono_tm_end;

#if 0
static inline void chrono_clear()
{
	memset(&chrono_tm_start, 0, sizeof(chrono_tm_start));
	memset(&chrono_tm_end, 0, sizeof(chrono_tm_end));
}
#endif

static inline void chrono_start()
{
    gettimeofday(&chrono_tm_start, NULL);
}

static inline double chrono_stop()
{
    gettimeofday(&chrono_tm_end, NULL);

    double ms = static_cast<double>(
    	((static_cast<int64_t>(1000LL) * static_cast<int64_t>(chrono_tm_end.tv_sec - chrono_tm_start.tv_sec)) +
    	 (static_cast<int64_t>(chrono_tm_end.tv_usec - chrono_tm_start.tv_usec) / static_cast<int64_t>(1000LL)))
    	);
    chrono_tm_start = chrono_tm_end;
    return ms;
}


static const int MAX_WORDS = 1000000;

static void benchmark_1()
{
	typedef milliways::hashtable<std::string, std::string> ht_t;

	const std::string words_pathname("/usr/share/dict/words");

	std::ifstream f(words_pathname.c_str());
	if(!f.is_open())
        throw std::domain_error("Unable to load words input file: " + words_pathname);

	std::vector<std::string> words;
    std::string line;
    int n = 0;
    while (std::getline(f, line)) {
        line = trim(line);
        if ((! line.empty()) && (! startsWith(line, "#")))
        {
        	std::string word(line);
			if (word.length() > 20)
				continue;
        	words.push_back(word);
        	n++;
        	if (n >= MAX_WORDS)
        		break;
        }
    }
    f.close();

	ht_t ht(1000000 + 1);

	int i = 0;
	chrono_start();
	std::vector<std::string>::const_iterator it;
	for (it = words.begin(); it != words.end(); ++it)
	{
		const std::string& word = *it;
		assert(word.length() <= 20);
    	ht.set(word, word);
    	i++;
	}
	double w_elapsed = chrono_stop();

	size_t nwords = words.size();
	double w_wps = 1000.0 * static_cast<double>(nwords) / w_elapsed;
	std::cout << "# " << nwords << " words. SET: " <<  w_wps << " words/s" << std::endl;

	std::map<std::string, std::string> map;
	i = 0;
	chrono_start();
	for (it = words.begin(); it != words.end(); ++it)
	{
		const std::string& word = *it;
		assert(word.length() <= 20);
    	map[word] = word;
    	i++;
	}
	w_elapsed = chrono_stop();

	nwords = words.size();
	w_wps = 1000.0 * static_cast<double>(nwords) / w_elapsed;
	std::cout << "# " << nwords << " words. map SET: " <<  w_wps << " words/s" << std::endl;

	cxx_um::unordered_map<std::string, std::string> umap;
	i = 0;
	chrono_start();
	for (it = words.begin(); it != words.end(); ++it)
	{
		const std::string& word = *it;
		assert(word.length() <= 20);
    	umap[word] = word;
    	i++;
	}
	w_elapsed = chrono_stop();

	nwords = words.size();
	w_wps = 1000.0 * static_cast<double>(nwords) / w_elapsed;
	std::cout << "# " << nwords << " words. unordered_map SET: " <<  w_wps << " words/s" << std::endl;
}

int main(int /* argc */, char* /* argv */[])
{
	benchmark_1();
}
