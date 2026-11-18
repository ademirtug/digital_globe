#pragma once

#include <array>
#include <memory>
#include <unordered_map> 
#include <vector>
#include <stdexcept>
#include <tuple>

namespace ecs_s {
#define _SPARSE_CAPACITY_ 8192

	using entity = uint64_t;
	using component_id = entity;
	class registry;

	//term: type erasure
	class base {
	public:
		virtual void erase(const size_t& index) {};
	};

	//term: sparse sparse_set
	template<typename T, size_t capacity = _SPARSE_CAPACITY_> class sparse_set : public base {
		struct storage {
			size_t index{ UINT64_MAX };
			T payload;
		};
		std::vector<size_t> sparse_;
		std::array<storage, capacity> dense_;
		size_t n{ 0 };
		using sparse_type = T;
	public:

		sparse_set() : sparse_(capacity, UINT64_MAX) {
		}
		T& operator[](const size_t& index) {
			return dense_[sparse_[index]].payload;
		}
		void insert(const size_t& index, T& t) {
			if (has(index)) { (*this)[index] = t; return; }
			if (n == capacity) throw std::runtime_error("sparse_set capacity exceeded");
			if (index >= sparse_.size()) sparse_.resize(index + 1, UINT64_MAX);
			dense_[n] = { index, t };
			sparse_[index] = n++;
		}
		void erase(const size_t& index) {
			if (!has(index))
				return;
			size_t slot = sparse_[index];
			if (slot != --n) {
				dense_[slot] = std::move(dense_[n]);
				sparse_[dense_[slot].index] = slot;
			}
			dense_[n] = {};
			sparse_[index] = UINT64_MAX;
		}
		bool has(const size_t& index) {
			return index < sparse_.size() && sparse_[index] < n &&
				sparse_[index] != UINT64_MAX &&
				dense_[sparse_[index]].index == index;
		}
		auto begin() {
			return dense_.begin();
		}
		auto end() {
			return dense_.begin() + n;
		}
	};


	class registry {
		//term: recursive variadic helper template structure
		template <size_t N, typename T, typename ...Ts>
		struct component_has_helper {
			static bool component_has_impl(registry& world, const entity& e) {
				return world.get_component_data<T>()->has(e) &&
				component_has_helper<N - 1, Ts...>::component_has_impl(world, e);
			}
		};
		template <typename T>
		struct component_has_helper<0, T> {
			static bool component_has_impl(registry& world, const entity& e) {
				return world.get_component_data<T>()->has(e);
			}
		};
		template<typename T>
		auto get_component_data() {
			if (component_data_.find(get_component_id<T>()) == component_data_.end()) {
				component_data_[get_component_id<T>()] = std::make_shared<sparse_set<T>>();
			}
			return static_cast<sparse_set<T>*>(component_data_[get_component_id<T>()].get());
		}

		std::unordered_map<component_id, std::shared_ptr<base>> component_data_;
	public:
		[[nodiscard]] entity new_entity() {
			static uint64_t entity_counter;
			return ++entity_counter;
		}

		void remove_entity(const entity& e) {
			for (auto it : component_data_) {
				it.second->erase(e);
			}
		}
		//registry impl
		template<typename T>
		inline T& get_component_value_for(const entity& e) {
			return (*(get_component_data<T>()))[e];
		}
		template<typename ...Ts>
		inline bool component_has(const entity& e) {
			return component_has_helper<sizeof...(Ts) - 1, Ts...>::component_has_impl(*this, e);
		}
		//TODO: passing lvalue causes C2280! hence T&&	
		template<typename T>
		void add_component(const entity& e, T&& p) {
			get_component_data<T>()->insert(e, p);
		}
		template<typename T>
		void add_component(const entity& e, T& p) {
			get_component_data<T>()->insert(e, p);
		}
		template<typename T>
		void remove_component(const entity& e) {
			get_component_data<T>()->erase(e);
		}
		template<typename T>
		void truncate_component() {
			component_data_[get_component_id<T>()] = std::make_shared<sparse_set<T>>();
		}
		//trick: each T will get its own static variable
		template<typename T>
		[[nodiscard]] component_id get_component_id() {
			static component_id id = new_entity();
			return id;
		}
		template<typename T, typename F>
		void each(F f) {
			sparse_set<T>* ssp = get_component_data<T>();
			auto it = ssp->begin();
			while (it++ != ssp->end()) {
				f(it->index, it->payload);
			}
		}
		template<typename... Ts, typename F>
		void view(F f) {
			using T = std::tuple_element_t<0, std::tuple<Ts...>>;
			sparse_set<T>* ssp = get_component_data<T>();
			for (auto it = ssp->begin(); it != ssp->end(); it++) {
				entity e = it->index;
				if (!component_has<Ts...>(e))
					continue;
				auto tx = std::forward_as_tuple(e, get_component_value_for<Ts>(e)...);
				std::apply(f, tx);
			}
		}
	};



}
