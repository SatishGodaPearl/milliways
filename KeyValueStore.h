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

#ifndef MILLIWAYS_KEYVALUESTORE_H
#define MILLIWAYS_KEYVALUESTORE_H

#include <iostream>
#include <fstream>
#include <string>
#include <functional>

#include <stdint.h>
#include <assert.h>

#include "Seriously.h"
#include "BlockStorage.h"
#include "BTreeCommon.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "BTreeFileStorage.h"

namespace milliways {

static const size_t KV_BLOCKSIZE = 4096;
static const int KV_CACHESIZE = 1024;
static const int KV_B = 5;

typedef uint32_t serialized_value_size_type;

/* ----------------------------------------------------------------- *
 *   DataLocator                                                     *
 * ----------------------------------------------------------------- */

struct DataLocator
{
public:
	typedef int16_t offset_t;
	typedef uint16_t uoffset_t;

	DataLocator() :
		m_block_id(BLOCK_ID_INVALID), m_offset(0) {}
	DataLocator(block_id_t block_id, offset_t offset) :
		m_block_id(block_id), m_offset(offset) {}
	DataLocator(const DataLocator& other) :
		m_block_id(other.m_block_id), m_offset(other.m_offset) {}
	DataLocator(const DataLocator& other, offset_t delta_) :
			m_block_id(other.m_block_id), m_offset(other.m_offset) { delta(delta_); }
	DataLocator& operator= (const DataLocator& other) { m_block_id = other.m_block_id; m_offset = other.m_offset; return *this; }

	bool operator== (const DataLocator& rhs) const { return (((! block_id_valid(m_block_id)) && (! block_id_valid(rhs.m_block_id))) || ((m_block_id == rhs.m_block_id) && (m_offset == rhs.m_offset))); }
	bool operator!= (const DataLocator& rhs) const { return (! (*this == rhs)); }
	bool operator< (const DataLocator& rhs) const {
		if ((! block_id_valid(m_block_id)) && (! block_id_valid(rhs.m_block_id)))
			return false;
		if (m_block_id < rhs.m_block_id)
			return true;
		else if (m_block_id == rhs.m_block_id)
			return (m_offset < rhs.m_offset);
		return false;
	}
	operator bool() const { return valid(); }

	bool valid() const { return block_id_valid(m_block_id) && (m_offset < static_cast<offset_t>(KV_BLOCKSIZE)); }

	DataLocator& invalidate() { m_block_id = BLOCK_ID_INVALID; return *this; }

	block_id_t block_id() const { return m_block_id; }
	block_id_t block_id(block_id_t value) { block_id_t old = m_block_id; m_block_id = value; return old; }

	offset_t offset() const { return m_offset; }
	offset_t offset(offset_t value) { offset_t old = m_offset; m_offset = value; return old; }

	uoffset_t uoffset() { normalize(); return static_cast<uoffset_t>(m_offset); }

	DataLocator& delta(offset_t delta_) { m_offset += delta_; return normalize(); }

	DataLocator& normalize()
	{
		if (! block_id_valid(m_block_id))
			return *this;
		assert(block_id_valid(m_block_id));
		while (m_offset >= static_cast<offset_t>(KV_BLOCKSIZE)) {
			m_block_id++;
			m_offset -= static_cast<offset_t>(KV_BLOCKSIZE);
		}
		while (m_offset < 0) {
			assert(m_block_id > 0);
			m_block_id--;
			m_offset += static_cast<offset_t>(KV_BLOCKSIZE);
		}
		assert(block_id_valid(m_block_id));
		assert((m_offset >= 0) && (m_offset < static_cast<offset_t>(KV_BLOCKSIZE)));
		return *this;
	}

protected:
	block_id_t m_block_id;
	offset_t m_offset;
};

inline std::ostream& operator<< (std::ostream& out, const DataLocator& value)
{
	if (value.valid())
		out << "<KVDataLocator block:" << value.block_id() << " offset:" << (int)value.offset() << ">";
	else
		out << "<KVDataLocator invalid>";
	return out;
}

/* ----------------------------------------------------------------- *
 *   SizedLocator                                                    *
 * ----------------------------------------------------------------- */

struct SizedLocator : public DataLocator
{
public:
	typedef size_t size_type;

	SizedLocator() : DataLocator(), m_size(0) {}
	SizedLocator(block_id_t block_id_, offset_t offset_, size_t size_) :
		DataLocator(block_id_, offset_), m_size(size_) {}
	SizedLocator(const SizedLocator& other) :
		DataLocator(other.block_id(), other.offset()), m_size(other.m_size) {}
	SizedLocator(const DataLocator& dataLocator, size_t size_) :
		DataLocator(dataLocator), m_size(size_) {}
	SizedLocator(const SizedLocator& other, offset_t delta_) :
		DataLocator(other.block_id(), other.offset()), m_size(other.m_size) { delta(delta_); }
	SizedLocator& operator=(const SizedLocator& other) { m_block_id = other.m_block_id; m_offset = other.m_offset; m_size = other.m_size; return *this; }
	SizedLocator& operator=(const DataLocator& dl) { m_block_id = dl.block_id(); m_offset = dl.offset(); return *this; }

	bool operator==(const SizedLocator& rhs) const
	{
		return (((!block_id_valid(m_block_id)) && (!block_id_valid(rhs.m_block_id))) ||
		        ((m_block_id == rhs.m_block_id) && (m_offset == rhs.m_offset) && (m_size == rhs.m_size)));
	}
	bool operator!=(const SizedLocator& rhs) const { return (!(*this == rhs)); }
	bool operator<(const SizedLocator& rhs) const {
		if ((!block_id_valid(m_block_id)) && (!block_id_valid(rhs.m_block_id)))
			return false;
		if (m_block_id < rhs.m_block_id)
			return true;
		else if ((m_block_id == rhs.m_block_id) && (m_offset == rhs.m_offset))
			return (m_size < rhs.m_size);
		else if (m_block_id == rhs.m_block_id)
			return (m_offset < rhs.m_offset);
		return false;
	}

	DataLocator dataLocator() const { return DataLocator(m_block_id, m_offset); }
	SizedLocator& dataLocator(const DataLocator& value) { block_id(value.block_id()); offset(value.offset()); return *this; }

	size_type size() const { return m_size; }
	size_type size(size_type value) { size_type old = m_size; m_size = value; return old; }

	SizedLocator& shrink(size_type value) { m_size -= value; return *this; }
	SizedLocator& grow(size_type value) { m_size += value; return *this; }

	size_type envelope_size() const { return size(); }
	size_type envelope_size(size_type value) { return size(value); }

	size_type contents_size() const { return (envelope_size() - sizeof(serialized_value_size_type)); }
	size_type contents_size(size_type value) { return envelope_size(value + sizeof(serialized_value_size_type)); }

protected:
	size_type m_size;
};

inline std::ostream& operator<< (std::ostream& out, const SizedLocator& value)
{
	if (value.valid())
		out << "<KVSizedLocator block:" << value.block_id() << " offset:" << (int)value.offset() << " envelope-size:" << value.envelope_size() << " contents-size:" << value.contents_size() << ">";
	else
		out << "<KVSizedLocator invalid>";
	return out;
}

} /* end of namespace milliways */

namespace seriously {

/* ----------------------------------------------------------------- *
 *   ::seriously::Traits<milliways::DataLocator>                     *
 * ----------------------------------------------------------------- */

//template <typename T>
//struct KVTraits;

template <>
struct Traits<milliways::DataLocator>
{
	typedef milliways::DataLocator type;
	typedef type serialized_type;
	enum { Size = sizeof(type) };
	enum { SerializedSize = (sizeof(int16_t) + sizeof(uint16_t)) };

	static ssize_t serialize(char*& dst, size_t& avail, const type& v);
	static ssize_t deserialize(const char*& src, size_t& avail, type& v);

	static size_t size(const type& value)    { UNUSED(value); return Size; }
	static size_t maxsize(const type& value) { UNUSED(value); return Size; }
	static size_t serializedsize(const type& value) { UNUSED(value); return Size; }

	static bool valid(const type& value)     { return value.valid(); }

	static int compare(const type& a, const type& b) { if (a == b) return 0; else if (a < b) return -1; else return +1; }
};

template <>
struct Traits<milliways::SizedLocator>
{
	typedef milliways::SizedLocator type;
	typedef type serialized_type;
	typedef milliways::serialized_value_size_type serialized_size_type;	/* uint32_t */
	enum { Size = sizeof(type) };
	enum { SerializedSize = (sizeof(int16_t) + sizeof(uint16_t) + sizeof(serialized_size_type)) };

	static ssize_t serialize(char*& dst, size_t& avail, const type& v);
	static ssize_t deserialize(const char*& src, size_t& avail, type& v);

	static size_t size(const type& value)    { UNUSED(value); return Size; }
	static size_t maxsize(const type& value) { UNUSED(value); return Size; }
	static size_t serializedsize(const type& value) { UNUSED(value); return Size; }

	static bool valid(const type& value)     { return value.valid(); }

	static int compare(const type& a, const type& b) { if (a == b) return 0; else if (a < b) return -1; else return +1; }
};

} /* end of namespace seriously */

namespace milliways {

/* ----------------------------------------------------------------- *
 *   KeyValueStore                                                   *
 * ----------------------------------------------------------------- */

class KeyValueStore
{
public:
	static const int MAJOR_VERSION = 0;
	static const int MINOR_VERSION = 1;

	static const size_t BLOCKSIZE = KV_BLOCKSIZE;
	static const int CACHESIZE = KV_CACHESIZE;
	static const int B = KV_B;
	static const int KEY_HASH_SIZE = 20;

	static const int KEY_MAX_SIZE = 20;

	/*
	 * we use our B+Tree to map a hash of the original key to a value-locator
	 * (DataLocator struct above). So from the BTree point of view, its key is
	 * the hash of the user key.
	 * The hash is assembled this way:
	 *   4 bytes uid + 128-bit MurmurHash3 (16 bytes) == 20 bytes
	 *
	 * Alternative:
	 *   1st 2 bytes + 4-bytes length + 128-bit MurmurHash3 (16 bytes) + last 2 bytes == 24 bytes
	 */
	typedef seriously::Traits<std::string> key_traits;
	typedef seriously::Traits<DataLocator> mapped_traits;
	typedef BTree< B, key_traits, mapped_traits > kv_tree_type;
	typedef BTreeFileStorage< BLOCKSIZE, B, key_traits, mapped_traits > kv_tree_storage_type;
	typedef typename kv_tree_type::node_type kv_tree_node_type;
	typedef typename kv_tree_type::lookup_type kv_tree_lookup_type;
	typedef typename kv_tree_type::iterator kv_tree_iterator_type;

//	typedef FileBlockStorage<BLOCKSIZE, BLOCK_CACHESIZE> block_storage_type;
	typedef typename kv_tree_storage_type::block_storage_t block_storage_type;
	typedef typename block_storage_type::block_t block_type;

	typedef int32_t key_index_type;
	typedef seriously::Traits<key_index_type> index_key_traits;
	typedef seriously::Traits<DataLocator> index_mapped_traits;
	typedef BTree< B, index_key_traits, index_mapped_traits > kv_index_tree_type;

	class iterator;
	class const_iterator;

	struct Search
	{
	public:
		typedef typename SizedLocator::offset_t offset_t;
		typedef typename SizedLocator::uoffset_t uoffset_t;
		typedef typename SizedLocator::size_type size_type;

		Search() {}
		Search(const Search& other) : m_lookup(other.m_lookup), m_value_loc(SizedLocator(other.m_value_loc)) {}
		Search& operator= (const Search& other) { m_lookup = other.m_lookup; m_value_loc = other.m_value_loc; return *this; }

		bool operator== (const Search& rhs) const { return (m_lookup == rhs.m_lookup) && (m_value_loc == rhs.m_value_loc); }
		bool operator!= (const Search& rhs) const { return (! (*this == rhs)); }

		operator bool() const { return valid() && found(); }

		const kv_tree_lookup_type& lookup() const { return m_lookup; }
		kv_tree_lookup_type& lookup() { return m_lookup; }

		const SizedLocator& locator() const { return m_value_loc; }
		SizedLocator& locator() { return m_value_loc; }
		Search& locator(const SizedLocator& locator) { m_value_loc = locator; return *this; }

		bool found() const { return m_lookup.found(); }
		const std::string& key() const { return m_lookup.key(); }
		kv_tree_node_type* node() const { return m_lookup.node(); }
		int pos() const { return m_lookup.pos(); }
		node_id_t nodeId() const { return m_lookup.nodeId(); }

		bool valid() const { return m_value_loc.valid(); }
		Search& invalidate() { m_value_loc.invalidate(); return *this; }
		block_id_t block_id() const { return m_value_loc.block_id(); }
		block_id_t block_id(block_id_t value) { return m_value_loc.block_id(value); }
		offset_t offset() const { return m_value_loc.offset(); }
		offset_t offset(offset_t value) { return m_value_loc.offset(value); }
		uoffset_t uoffset() { return m_value_loc.uoffset(); }

		DataLocator dataLocator() const { return m_value_loc.dataLocator(); }
		Search& dataLocator(const DataLocator& value) { m_value_loc.dataLocator(value); return *this; }

		const SizedLocator& headLocator() const { return locator(); }
		SizedLocator& headLocator() { return locator(); }
		SizedLocator contentsLocator() const { size_t off = sizeof(serialized_value_size_type); SizedLocator cl = SizedLocator(locator()); cl.delta(off); cl.shrink(off); return cl; }

		DataLocator headDataLocator() const { return headLocator().dataLocator(); }
		DataLocator contentsDataLocator() const { return contentsLocator().dataLocator(); }

		size_type size() const { return m_value_loc.size(); }
		size_type size(size_type value) { return m_value_loc.size(value); }

		size_type envelope_size() const { return m_value_loc.envelope_size(); }
		size_type envelope_size(size_type value) { return m_value_loc.envelope_size(value); }

		size_type contents_size() const { return m_value_loc.contents_size(); }
		size_type contents_size(size_type value) { return m_value_loc.contents_size(value); }

	private:
		Search(const kv_tree_lookup_type& lookup_, const SizedLocator& vl) :
			m_lookup(lookup_), m_value_loc(SizedLocator(vl)) {}

		kv_tree_lookup_type m_lookup;
		SizedLocator m_value_loc;
	};

	KeyValueStore(block_storage_type* blockstorage);
	~KeyValueStore();

	bool isOpen() const;
	bool open();
	bool close();

	bool has(const std::string& key);
	bool find(const std::string& key, Search& result);
	Search find(const std::string& key) { Search result; find(key, result); return result; }
	bool get(const std::string& key, std::string& value);
	bool get(Search& result, std::string& value, ssize_t partial = -1);			/* streaming/partial reads */ 
	std::string get(const std::string& key);
	bool put(const std::string& key, const std::string& value, bool overwrite = true);
	bool rename(const std::string& old_key, const std::string& new_key);

	/* -- Iteration ------------------------------------------------ */

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(this, /* forward */ true, /* end */ true); }

	iterator rbegin() { return iterator(this, /* forward */ false); }
	iterator rend() { return iterator(this, /* forward */ false, /* end */ true); }

	class base_iterator
	{
	public:
		typedef KeyValueStore kv_type;
		typedef base_iterator self_type;
		typedef typename kv_type::kv_tree_type kv_tree_type;
		typedef typename kv_type::kv_tree_node_type kv_tree_node_type;
		typedef typename kv_type::kv_tree_lookup_type kv_tree_lookup_type;
		typedef typename kv_type::kv_tree_iterator_type kv_tree_iterator_type;
		typedef std::string value_type;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;

		base_iterator() : m_kv(NULL), m_tree(NULL), m_forward(true), m_end(true) {}
		base_iterator(kv_type* kv, bool forward_ = true, bool end_ = false) : m_kv(kv), m_tree(NULL), m_forward(forward_), m_end(end_) { m_tree = m_kv->kv_tree(); kv_tree_iterator_type it(m_tree, m_tree->root(), forward_, end_); m_tree_it = it; rewind(end_); }
		base_iterator(const base_iterator& other) : m_kv(other.m_kv), m_tree(other.m_tree), m_tree_it(other.m_tree_it), m_forward(other.m_forward), m_end(other.m_end), m_current_key(other.m_current_key) { }
		base_iterator& operator= (const base_iterator& other) { m_kv = other.m_kv; m_tree = other.m_tree; m_tree_it = other.m_tree_it; m_forward = other.m_forward; m_end = other.m_end; m_current_key = other.m_current_key; return *this; }

		self_type& operator++() { next(); return *this; }
		self_type& operator++(int junk) { next(); return *this; }
		self_type& operator--() { prev(); return *this; }
		self_type& operator--(int junk) { prev(); return *this; }
		// reference operator*() { m_current_key = (*m_tree_it).key(); return m_current_key; }
		const_reference operator*() const { m_current_key = (*m_tree_it).key(); return m_current_key; }
		// pointer operator->() { m_current_key = (*m_tree_it).key(); return &m_current_key; }
		const_pointer operator->() const { m_current_key = (*m_tree_it).key(); return &m_current_key; }
		bool operator== (const self_type& rhs) {
			return (m_kv == rhs.m_kv) && (m_tree == rhs.m_tree) &&
					((end() && rhs.end()) ||
					 ((m_forward == rhs.m_forward) && (m_end == rhs.m_end) && (m_tree_it == rhs.m_tree_it))); }
		bool operator!= (const self_type& rhs) { return (! (*this == rhs)); }

		operator bool() const { return (! end()) && m_tree_it; }

		self_type& rewind(bool end_) { m_tree_it.rewind(end_); return *this; }
		bool next() { return m_tree_it.next(); }
		bool prev() { return m_tree_it.prev(); }

		kv_type* kv() const { return m_kv; }
		const_reference key() const { m_current_key = (*m_tree_it).key(); return m_current_key; }
		bool forward() const { return m_forward; }
		bool backward() const { return (! m_forward); }
		bool end() const { return m_end || (m_tree_it.end()); }

	protected:
		kv_type* m_kv;
		kv_tree_type* m_tree;
		mutable kv_tree_iterator_type m_tree_it;
		bool m_forward;
		bool m_end;
		mutable std::string m_current_key;
	};

	class iterator : public base_iterator
	{
	public:
		iterator(kv_type* kv, bool forward_ = true, bool end_ = false) : base_iterator(kv, forward_, end_) {}
		iterator(const iterator& other) : base_iterator(other) {}

		reference operator*() { m_current_key = (*m_tree_it).key(); return m_current_key; }
		pointer operator->() { m_current_key = (*m_tree_it).key(); return &m_current_key; }
	};

	class const_iterator : public base_iterator
	{
	public:
		const_iterator(kv_type* kv, bool forward_ = true, bool end_ = false) : base_iterator(kv, forward_, end_) {}
		const_iterator(const const_iterator& other) : base_iterator(other) {}
	};

	friend class base_iterator;
	friend class iterator;
	friend class const_iterator;

protected:
	bool find(const std::string& key, DataLocator& data_pos);
	bool find(const std::string& key, SizedLocator& sized_pos);

	bool read(std::string& dst, SizedLocator& location);
	bool write(const std::string& src, SizedLocator& location);

	bool alloc_value_envelope(SizedLocator& dst);
	size_t size_in_blocks(size_t size);

	/* -- Header I/O ----------------------------------------------- */

	bool header_write();
	bool header_read();

	/* -- Block I/O ------------------------------------------------ */

	block_id_t block_alloc_id(int n_blocks = 1) { assert(m_blockstorage); return m_blockstorage->allocId(n_blocks); }
	bool block_dispose(block_id_t block_id, int count = 1) { assert(m_blockstorage); return m_blockstorage->dispose(block_id, count); }
	block_type* block_get(block_id_t block_id) { assert(m_blockstorage); return m_blockstorage->get(block_id); }
	bool block_put(const block_type& src) { assert(m_blockstorage); return m_blockstorage->put(src); }

	/* -- Tree access ---------------------------------------------- */

	kv_tree_type* kv_tree() { return m_kv_tree; }

private:
	KeyValueStore();
	KeyValueStore(const KeyValueStore& other);
	KeyValueStore& operator= (const KeyValueStore& other);

	block_storage_type* m_blockstorage;
	kv_tree_storage_type* m_storage;
	kv_tree_type* m_kv_tree;

	/*
	 * We either allocate a single block for a short string
	 * or multiple consecutive blocks for a long string.
	 * When allocating multiple blocks, these are initially all free
	 * and at the end of their use, only the last block could remain
	 * partially available.
	 * So we store only:
	 *   - the current free block index (first in the span)
	 *   - the current offset in the block (for available space)
	 *   - currently available space
	 * If the currently available space is > BLOCKSIZE then
	 * we have multiple blocks in the span and they are completely
	 * available (the offset has no meaning). Otherwise only one
	 * block is in the span and the offset indicates where the
	 * free space starts.
	 */
	block_id_t m_first_block_id;
	block_id_t m_current_block_id;
	size_t m_current_block_offset;
	size_t m_current_block_avail;
	block_type* m_current_block;

	int m_kv_header_uid;
};

inline std::ostream& operator<< ( std::ostream& out, const KeyValueStore::iterator& value )
{
	out << "<KeyValueStore::iterator " << (value.forward() ? "forward" : "backward") << " " << (value.end() ? "END " : "") << "key:'" << value.key() << "'>";
	return out;
}

inline std::ostream& operator<< ( std::ostream& out, const KeyValueStore::const_iterator& value )
{
	out << "<KeyValueStore::const_iterator " << (value.forward() ? "forward" : "backward") << " " << (value.end() ? "END " : "") << "key:'" << value.key() << "'>";
	return out;
}

inline std::ostream& operator<< (std::ostream& out, const KeyValueStore::Search& value)
{
	if (value.valid())
		out << "<KV::Search lookup:" << value.lookup() << " locator:" << value.locator() << ">";
	else
		out << "<KV::Search invalid>";
	return out;
}

} /* end of namespace milliways */

#include "KeyValueStore.impl.hpp"

#endif /* MILLIWAYS_KEYVALUESTORE_H */
