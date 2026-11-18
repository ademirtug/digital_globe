#pragma once

#include <list>
#include <map>

template<typename K, typename V, size_t capacity = 8192>
class lru_cache {
	std::list<K> order_;
	std::map<K, std::pair<V, typename std::list<K>::iterator>> data_;
public:
	void clear() { data_.clear(); order_.clear(); }
	bool exists(const K& key) const {
		return data_.find(key) != data_.end();
	}
	V& get(const K& key) {
		auto& item = data_.at(key);
		order_.splice(order_.begin(), order_, item.second);
		return item.first;
	}
	void put(const K& key, const V& value) {
		if (exists(key)) {
			get(key) = value;
			return;
		}
		order_.push_front(key);
		data_.emplace(key, std::make_pair(value, order_.begin()));
		if (data_.size() > capacity) {
			data_.erase(order_.back());
			order_.pop_back();
		}
	}
};
