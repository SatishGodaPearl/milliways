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

#ifndef MILLIWAYS_LRUCACHE_H
#define MILLIWAYS_LRUCACHE_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <deque>
#include <functional>

#include <stdint.h>
#include <assert.h>

#include "config.h"
#include "ordered_map.h"

namespace milliways {

static const int LRUCACHE_L1_CACHE_SIZE = 16;

template <size_t SIZE, typename Key, typename T>
class LRUCache
{
public:
	typedef Key key_type;
	typedef T mapped_type;
	typedef std::pair<Key, T> value_type;
	typedef ordered_map<key_type, mapped_type> ordered_map_type;
	typedef typename ordered_map<key_type, mapped_type>::size_type size_type;

	static const size_type Size = SIZE;
	static const int L1_SIZE = LRUCACHE_L1_CACHE_SIZE;

	typedef enum { op_get, op_set, op_sub } op_type;

	// LRUCache();
	LRUCache(const key_type& invalid);
	virtual ~LRUCache() { /* call evict_all() in final destructor */ evict_all(); }

	virtual bool on_miss(op_type op, const key_type& key, mapped_type& value);
	virtual bool on_set(const key_type& key, const mapped_type& value);
	virtual bool on_delete(const key_type& key);
	virtual bool on_eviction(const key_type& key, mapped_type& value);

	bool empty() const { return m_omap.empty(); }
	size_type size() const { return m_omap.size(); }
	size_type max_size() const { return Size; }

	void clear() { clear_l1(); m_omap.clear(); }
	void clear_l1();

	bool has(key_type& key) const { return m_omap.has(key); }
	bool get(mapped_type& dst, key_type& key);
	bool set(key_type& key, mapped_type& value);
	bool del(key_type& key);

	size_type count(const key_type& key) const { return m_omap.count(key); }
	mapped_type& operator[](const key_type& key);

	void evict(bool force = false);     // evict the LRU item
	void evict_all();

	value_type pop();                   // pop LRU item and return it

	key_type invalid_key() const { return m_invalid_key; }
	void invalid_key(const key_type& value) { m_invalid_key = value; }
	void invalidate_key(key_type& key) const { key = m_invalid_key; }

	void keys(std::vector<key_type>& dst) {
		dst.clear();
		typename ordered_map<key_type, mapped_type>::const_iterator it;
		for (it = m_omap.begin(); it != m_omap.end(); ++it)
			dst.push_back(it->first);
	}

	void values(std::vector<value_type>& dst) {
		dst.clear();
		typename ordered_map<key_type, mapped_type>::const_iterator it;
		for (it = m_omap.begin(); it != m_omap.end(); ++it)
			dst.push_back(*it);
	}

private:
	LRUCache();
	LRUCache(const LRUCache<SIZE, Key, T>& other);
	LRUCache& operator= (const LRUCache<SIZE, Key, T>& rhs);

	mutable key_type m_l1_key[L1_SIZE];
	mutable mapped_type* m_l1_mapped[L1_SIZE];
	mutable int m_l1_last;

	ordered_map<key_type, mapped_type> m_omap;
	key_type m_invalid_key;
};

} /* end of namespace milliways */

#include "LRUCache.impl.hpp"

#endif /* MILLIWAYS_LRUCACHE_H */
