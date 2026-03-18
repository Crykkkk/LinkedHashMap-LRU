#ifndef SJTU_LRU_HPP
#define SJTU_LRU_HPP

#include "class-integer.hpp"
#include "class-matrix.hpp"
#include "exceptions.hpp"
#include "utility.hpp"
#include <iterator>
#include <vector>
#include <list>

class Hash {
public:
    unsigned int operator()(Integer lhs) const {
        int val = lhs.val;
        return std::hash<int>()(val);
    }
};
// Hash 实现

class Equal {
public:
    bool operator()(const Integer &lhs, const Integer &rhs) const { return lhs.val == rhs.val; }
};
// Integer 类的 =

namespace sjtu {
template<class T>
class double_list {
public:
	struct node{
		T data;
		node* prev = nullptr;
		node* next = nullptr;
		node (T d, node* p, node* n) : data(d), prev(p), next(n) {}
		node (){}
	};

	int size = 0;
	node* head_ = new node();
	node* tail_ = new node();
	// --------------------------
	/**
	 * the follows are constructors and destructors
	 * you can also add some if needed.
	 */
	double_list() {
		head_->next = tail_;
		tail_->prev = head_;
	}
	double_list(const double_list<T> &other) {
		head_->next = tail_;
		tail_->prev = head_;
		size = 0;
		node* track = other.head_->next;
		while (track != other.tail_) {
			insert_tail(track->data);
			track = track->next;
		}
	}
	~double_list() {
		node* curr = head_->next;
		while (curr != tail_) {
			node* next_node = curr->next;
			delete curr;
			curr = next_node;
		}
		head_->next = tail_;
		tail_->prev = head_;
		size = 0;
		delete head_;
		delete tail_;
	}

	class iterator {
	public:
		/**
		 * elements
		 * add whatever you want
		 */
		 node* at = nullptr;
		// --------------------------
		/**
		 * the follows are constructors and destructors
		 * you can also add some if needed.
		 */
		iterator() {}
		iterator(const iterator &t) {
			at = t.at;
		}
		iterator(node* a):at(a){}
		~iterator() {} // 应该不能 delete 吧？
		
		/**
		 * iter++
		 */
		iterator operator++(int) {
			if (at == nullptr || at->next == nullptr) throw sjtu::invalid_iterator();
			iterator tmp = *this;
			at = at->next;
			return tmp;	
		}
		/**
		 * ++iter
		 */
		iterator &operator++() {
			if (at == nullptr|| at->next == nullptr) throw sjtu::invalid_iterator();
			at = at->next;
			return *this; // 引用的返回引用
		}
		/**
		 * iter--
		 */
		iterator operator--(int) {
			if (at == nullptr|| at->prev == nullptr|| at->prev->prev == nullptr) throw sjtu::invalid_iterator();
			iterator tmp = *this;
			at = at->prev;
			return tmp;	
		}
		/**
		 * --iter
		 */
		iterator &operator--() {
			if (at == nullptr|| at->prev == nullptr|| at->prev->prev == nullptr) throw sjtu::invalid_iterator();
			at = at->prev;
			return *this; 
		}
		
		/**
		 * if the iter didn't point to a value
		 * throw " invalid"
		 */
		T &operator*() const { // 现在还在搞混..&返回的也是数据本身，但是作为引用返回
			if (at == nullptr || at->next == nullptr || at->prev == nullptr) {
				throw sjtu::invalid_iterator();
			}
			return at->data;
		}
		
		/**
		 * other operation
		 */
		T *operator->() const noexcept {
			return &(operator*()); // 复用
		}
		bool operator==(const iterator &rhs) const {
			return (at == rhs.at);
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
	};
	/**
	 * return an iterator to the beginning
	 */
	iterator begin() {
		return iterator(head_->next);
	} 
	/**
	 * return an iterator to the ending
	 * in fact, it returns the iterator point to nothing,
	 * just after the last element.
	 */
	iterator end() {
		return iterator(tail_);
	}
	/**
	 * if the iter didn't point to anything, do nothing,
	 * otherwise, delete the element pointed by the iter
	 * and return the iterator point at the same "index"
	 * e.g.
	 * 	if the origin iterator point at the 2nd element
	 * 	the returned iterator also point at the
	 *  2nd element of the list after the operation
	 *  or nothing if the list after the operation
	 *  don't contain 2nd elememt.
	 */
	iterator erase(iterator pos) {
		if (!pos.at || pos == end()) throw sjtu::invalid_iterator();
		node* ptr = pos.at;
		node* ret = ptr->next;
		ptr->prev->next = ptr->next;
		ptr->next->prev = ptr->prev;
		delete(ptr);
		size--;
		return iterator(ret);
	}

	/**
	 * the following are operations of double list
	 */
	void insert_head(const T &val) {
		node* nd = new node(val, head_, head_->next);
		head_->next = nd;
		nd->next->prev = nd;
		size++;
	}
	void insert_tail(const T &val) {
		node* nd = new node(val, tail_->prev, tail_);
		tail_->prev = nd;
		nd->prev->next = nd;
		size++;
	}
	void delete_head() {
		if (!size) throw sjtu::container_is_empty();
		node* target = head_->next;
		head_->next = target->next;
		target->next->prev = head_;
		delete(target);
		size--;
	}
	void delete_tail() {
		if (!size) throw sjtu::container_is_empty();
		node* target = tail_->prev;
		tail_->prev = target->prev;
		target->prev->next = tail_;
		delete(target);	
		size--;	
	}
	/**
	 * if didn't contain anything, return true,
	 * otherwise false.
	 */
	bool empty() {
		return !size;
	}
};

template<class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>> // 注意 equal 仅 key
class hashmap {
public:
	using value_type = pair<const Key, T>;
	struct Entry{
		value_type* kv;
		typename double_list<value_type*>::iterator list_pos; // 这里是为了 Linked hash map 做准备
		Entry(value_type* k) : kv(k) {}
	};

	using Bucket = std::list<Entry*>;
   using BucketIterator = typename Bucket::iterator; 

	int Size;
	int cnt = 0;
	const float load_factor = 0.8;
	std::vector<std::list<Entry*>> data;
	// --------------------------

	/**
	 * the follows are constructors and destructors
	 * you can also add some if needed.
	 */
	hashmap(): Size(100) {
		data.resize(Size);
	}
	hashmap(const hashmap &other) { // Done
		Size = other.Size;
		cnt = other.cnt;
		data.resize(Size);
		for (int i = 0; i < Size; i++) {
         for (Entry* ptr : other.data[i]) {
            value_type* new_pair = new value_type(*(ptr->kv));
            data[i].push_back(new Entry(new_pair));
        }
   	}
	}
	~hashmap() { // Done
		clear();
	}
	hashmap &operator=(const hashmap &other) { // Done
		if (this == &other) {
       	return *this;
    	}

		clear();

		Size = other.Size;
		cnt = other.cnt;
		data.resize(Size);
		for (int i = 0; i < Size; i++) {
         for (Entry* ptr : other.data[i]) {
            value_type* new_pair = new value_type(*(ptr->kv));
            data[i].push_back(new Entry(new_pair));
        }
   	}
		return *this;
	}

	class iterator {
	public: // 用 idx 来指示
		int bucket = 0; 
		BucketIterator it = BucketIterator(); 
		hashmap* host = nullptr;  

		// --------------------------
		/**
		 * the follows are constructors and destructors
		 * you can also add some if needed.
		 */
		iterator() {}
		iterator (int b, BucketIterator i, hashmap* h): bucket(b), it(i), host(h) {}
		iterator(const iterator &t) {
			bucket = t.bucket;
			it = t.it;
			host = t.host;
		}
		~iterator() {
		}

		/**
		 * if point to nothing
		 * throw
		 */
		value_type &operator*() const {
			// if (host == nullptr || it == nullptr) {
			if (host == nullptr) {
				throw sjtu::container_is_empty();
			}
			return *((*it)->kv);
		}

		/**
		 * other operation
		 */
		value_type *operator->() const noexcept {
			// if (host == nullptr || it == nullptr) {
			if (host == nullptr) {
				throw sjtu::container_is_empty();
			}
			return (*it)->kv;
		}
		bool operator==(const iterator &rhs) const { // Done
			return (host == rhs.host) && (bucket == rhs.bucket) && (it == rhs.it);
		}
		bool operator!=(const iterator &rhs) const { // Done
			return !(*this == rhs);
		}
	};

	void clear() { // Done
		for (int i = 0; i < Size; i++) {
			for (Entry* ptr : data[i]) {
				delete (ptr->kv);
				delete ptr; 
			}
			data[i].clear();
		}
		cnt = 0;
	}

	/**
	 * you need to expand the hashmap dynamically
	 */
	void expand() { // 这里需要 rehash，才能支持后面的 find
		Size *= 2;
		std::vector<std::list<Entry*>> newdata(Size);
		for (int i = 0; i < data.size(); i++) {
			for (Entry* j : data[i]) {
				int nh = Hash()(j->kv->first) % Size;
				// newdata[nh].push_back(new Entry(j->kv));
				newdata[nh].push_back(j); // 直接搬运指针
				// j->kv = nullptr; 
			}
		}
		data.swap(newdata); 
	} 

	/**
	 * the iterator point at nothing
	 */
	iterator end() const {
		return iterator(Size, BucketIterator(), const_cast<hashmap*>(this));
	}
	/**
	 * find, return a pointer point to the value
	 * not find, return the end (point to nothing)
	 */
	iterator find(const Key &key) const {
		int hash_out = Hash()(key) % Size;

		std::list<Entry*> &check = const_cast<hashmap*>(this)->data[hash_out];

		for (auto it = check.begin(); it != check.end(); it++) {
			if (Equal()((*it)->kv->first, key)) {
				return iterator(hash_out, it, const_cast<hashmap*>(this));
			}
		}
		return end();
	}
	/**
	 * already have a value_pair with the same key
	 * -> just update the value, return false
	 * not find a value_pair with the same key
	 * -> insert the value_pair, return true
	 */
	sjtu::pair<iterator, bool> insert(const value_type &value_pair) {
		// 需要注意什么时候调用 expand
		int hash_out = Hash()(value_pair.first) % Size;
		std::list<Entry*> &check = data[hash_out];
		for (auto it = check.begin(); it != check.end(); it++) {
			if (Equal()((*it)->kv->first, value_pair.first)) {
				(*it)->kv->second = value_pair.second;
				return sjtu::pair<iterator, bool>(iterator(hash_out, it, const_cast<hashmap*>(this)), false);
			}
		}
		
		check.push_back(new Entry(new value_type(value_pair))); // valuepair是一个引用，可能是临时的

		cnt++;
		if (cnt >= Size * load_factor) {
			expand();
			return sjtu::pair<iterator, bool>(find(value_pair.first), true);
		}
		auto new_it = check.end();
		--new_it; 
		return sjtu::pair<iterator, bool>(iterator(hash_out, new_it, this), true);
	}
	/**
	 * the value_pair exists, remove and return true
	 * otherwise, return false
	 */
	bool remove(const Key &key) {
		int hash_out = Hash()(key) % Size;
		std::list<Entry*> &check = data[hash_out];
		for (auto it = check.begin(); it != check.end(); ++it) {
			if (Equal()((*it)->kv->first, key)) {
					delete (*it)->kv;
					delete (*it);        
					check.erase(it); 
					cnt--;
					return true;
			}
		}
		return false;
		}
};

template<class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key>>
class linked_hashmap : public hashmap<Key, T, Hash, Equal> {
public:
	typedef pair<const Key, T> value_type;
	/**
	 * elements
	 * add whatever you want
	 */
	double_list<value_type*> ins_ord; // data type 为指针保证了有默认构造函数（也好找）

	// --------------------------
	class const_iterator;
	class iterator {
	public:
		/**
		 * elements
		 * add whatever you want
		 */
		// --------------------------
		iterator() {}
		iterator(const iterator &other) {}
		~iterator() {}

		/**
		 * iter++
		 */
		iterator operator++(int) {}
		/**
		 * ++iter
		 */
		iterator &operator++() {}
		/**
		 * iter--
		 */
		iterator operator--(int) {}
		/**
		 * --iter
		 */
		iterator &operator--() {}

		/**
		 * if the iter didn't point to a value
		 * throw "star invalid"
		 */
		value_type &operator*() const {}
		value_type *operator->() const noexcept {}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {}
		bool operator!=(const iterator &rhs) const {}
		bool operator==(const const_iterator &rhs) const {}
		bool operator!=(const const_iterator &rhs) const {}
	};

	class const_iterator {
	public:
		/**
		 * elements
		 * add whatever you want
		 */
		// --------------------------
		const_iterator() {}
		const_iterator(const iterator &other) {}

		/**
		 * iter++
		 */
		const_iterator operator++(int) {}
		/**
		 * ++iter
		 */
		const_iterator &operator++() {}
		/**
		 * iter--
		 */
		const_iterator operator--(int) {}
		/**
		 * --iter
		 */
		const_iterator &operator--() {}

		/**
		 * if the iter didn't point to a value
		 * throw
		 */
		const value_type &operator*() const {}
		const value_type *operator->() const noexcept {}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {}
		bool operator!=(const iterator &rhs) const {}
		bool operator==(const const_iterator &rhs) const {}
		bool operator!=(const const_iterator &rhs) const {}
	};

	linked_hashmap() {}
	linked_hashmap(const linked_hashmap &other) {}
	~linked_hashmap() {}
	linked_hashmap &operator=(const linked_hashmap &other) {}

	/**
	 * return the value connected with the Key(O(1))
	 * if the key not found, throw
	 */
	T &at(const Key &key) {}
	const T &at(const Key &key) const {}
	T &operator[](const Key &key) {}
	const T &operator[](const Key &key) const {}

	/**
	 * return an iterator point to the first
	 * inserted and existed element
	 */
	iterator begin() {}
	const_iterator cbegin() const {}
	/**
	 * return an iterator after the last inserted element
	 */
	iterator end() {}
	const_iterator cend() const {}
	/**
	 * if didn't contain anything, return true,
	 * otherwise false.
	 */
	bool empty() const {}

	void clear() {}

	size_t size() const {}
	/**
	 * insert the value_piar
	 * if the key of the value_pair exists in the map
	 * update the value instead of adding a new element，
	 * then the order of the element moved from inner of the
	 * list to the head of the list
	 * and return false
	 * if the key of the value_pair doesn't exist in the map
	 * add a new element and return true
	 */
	pair<iterator, bool> insert(const value_type &value) {}
	/**
	 * erase the value_pair pointed by the iterator
	 * if the iterator points to nothing
	 * throw
	 */
	void remove(iterator pos) {}
	/**
	 * return how many value_pairs consist of key
	 * this should only return 0 or 1
	 */
	size_t count(const Key &key) const {}
	/**
	 * find the iterator points at the value_pair
	 * which consist of key
	 * if not find, return the iterator
	 * point at nothing
	 */
	iterator find(const Key &key) {}
};

class lru {
	using lmap = sjtu::linked_hashmap<Integer, Matrix<int>, Hash, Equal>;
	using value_type = sjtu::pair<const Integer, Matrix<int>>;

public:
	lru(int size) {}
	~lru() {}
	/**
	 * save the value_pair in the memory
	 * delete something in the memory if necessary
	 */
	void save(const value_type &v) {}
	/**
	 * return a pointer contain the value
	 */
	Matrix<int> *get(const Integer &v) {}
	/**
	 * just print everything in the memory
	 * to debug or test.
	 * this operation follows the order, but don't
	 * change the order.
	 */
	void print() {
   // sjtu::linked_hashmap<Integer, Matrix<int>, Hash, Equal>::iterator it;
   // for (it = mem->begin(); it != mem->end(); it++) {
   //      std::cout << (*it).first.val << " "
   //                << (*it).second << std::endl;
   // 	}
	}
};
} // namespace sjtu

#endif
