/*****************************************************************************/
/*  Milliways - B+ trees and key-value store C++ library                     */
/*                                                                           */
/*  Copyright 2016 Marco Pantaleoni and J CUBE Inc. Tokyo, Japan.            */
/*                                                                           */
/*  Author: Marco Pantaleoni <marco.pantaleoni@gmail.com>                    */
/*                                                                           */
/*  Licensed under the Apache License, Version 2.0 (the "License");          */
/*  you may not use this file except in compliance with the License.         */
/*  You may obtain a copy of the License at                                  */
/*                                                                           */
/*      http://www.apache.org/licenses/LICENSE-2.0                           */
/*                                                                           */
/*  Unless required by applicable law or agreed to in writing, software      */
/*  distributed under the License is distributed on an "AS IS" BASIS,        */
/*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*  See the License for the specific language governing permissions and      */
/*  limitations under the License.                                           */
/*****************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

#include <stdint.h>
#include <string.h>
#include <assert.h>

#ifndef _MSC_VER
#include <sys/time.h>
#else
#include <Windows.h>
#endif

/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);

/*
 * timezone information is stored outside the kernel so tzp isn't used anymore.
 *
 * Note: this function is not for Win32 high precision timing purpose. See
 * elapsed_time().
 */
int
gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

    return 0;
}

#include "KeyValueStore.h"

/* ----------------------------------------------------------------- *
 *   PROTOTYPES                                                      *
 * ----------------------------------------------------------------- */

static inline int rand_int(int lo, int hi);
static std::string random_string(int length);
static std::vector<std::string>& split(std::vector<std::string>& elems, const std::string& s, char delim);
static std::vector<std::string> split(const std::string& s, char delim);
static std::string trim(const std::string& str );
static bool startsWith(const std::string& s, const std::string& prefix);
static bool endsWith(const std::string& s, const std::string& suffix);
static bool contains(const std::string& s, const std::string& infix);

static inline double epoch_ms();
static inline void chrono_clear();
static inline void chrono_start();
static inline double chrono_stop();

static void benchmark_1();

int main(int argc, char* argv[]);


/* ----------------------------------------------------------------- *
 *   IMPLEMENTATION                                                  *
 * ----------------------------------------------------------------- */

static inline int rand_int(int lo, int hi)
{
	return rand() % (hi - lo + 1) + lo;
}

static std::string random_string(int length)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string r(length, '\0');

	for (int i = 0; i < length; ++i)
		r[i] = alphanum[rand_int(0, sizeof(alphanum) - 1)];

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

static struct timeval chrono_tm_start, chrono_tm_end;

static inline void chrono_clear()
{
	memset(&chrono_tm_start, 0, sizeof(chrono_tm_start));
	memset(&chrono_tm_end, 0, sizeof(chrono_tm_end));
}

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
	typedef milliways::KeyValueStore kv_t;
	typedef XTYPENAME kv_t::block_storage_type kv_blockstorage_t;

	const std::string words_pathname("./benchmark_1_words");

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

	const std::string kv_pathname("/tmp/benchmark_kv_1");

	std::remove(kv_pathname.c_str());

	kv_blockstorage_t* bs = new kv_blockstorage_t(kv_pathname);

	kv_t kv(bs);

	kv.open();
	assert(kv.isOpen());

	int i = 0;
	chrono_start();
	std::vector<std::string>::const_iterator it;
	for (it = words.begin(); it != words.end(); ++it)
	{
		const std::string& word = *it;
		assert(word.length() <= 20);
    	bool ok = kv.put(word, word);
    	if (! ok)
    	{
    		std::cerr << "failed word #:" << i << " word:" << word << std::endl;
    	}
    	assert(ok);
    	i++;
	}
	double w_elapsed = chrono_stop();

	size_t nwords = words.size();
	double w_wps = 1000.0 * static_cast<double>(nwords) / w_elapsed;
	std::cout << "# " << nwords << " words. PUT: " <<  w_wps << " words/s" << std::endl;

	kv.close();
	std::remove(kv_pathname.c_str());
}

int main(int argc, char* argv[])
{
	benchmark_1();
}
