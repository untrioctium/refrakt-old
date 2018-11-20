#include <stdexcept>
#include <string>
#include <cstring>
#include <iostream>

namespace refrakt {
	template<typename T>
	class fixed_vector {
	public:

		class iterator {
		public:
			using self_type = iterator;

			using value_type = T;
			using reference = T & ;
			using pointer = T * ;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			iterator(pointer p) : ptr_(p) {}
			iterator() : ptr_(0) {}

			// Iterator requirements
			reference operator*() { return *ptr_; }
			self_type& operator++() { ++ptr_; return *this; }

			// InputIterator requirements
			bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
			bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
			pointer operator->() { return ptr_; }
			self_type operator++(int) { self_type ret(ptr_); ++ptr_; return ret; }

			// BidirectionalIterator requirements
			self_type& operator--() { --ptr_; return *this; }
			self_type operator--(int) { self_type ret(ptr_); --ptr_; return ret; }

			self_type operator+(difference_type d) { return self_type(ptr_ + d); }

			friend class fixed_vector<T>;
		private:
			pointer ptr_;

		};
		class const_iterator {
		public:
			using self_type = const_iterator;

			using value_type = T;
			using reference = T & ;
			using pointer = T * ;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			const_iterator(pointer p) : ptr_(p) {}
			const_iterator() : ptr_(0) {}

			// Iterator requirements
			const reference operator*() { return *ptr_; }
			self_type& operator++() { ++ptr_; return *this; }

			// InputIterator requirements
			bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
			bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
			const pointer operator->() { return ptr_; }
			self_type operator++(int) { self_type ret(ptr_); ++ptr_; return ret; }

			// BidirectionalIterator requirements
			self_type& operator--() { --ptr_; return *this; }
			self_type operator--(int) { self_type ret(ptr_); --ptr_; return ret; }

			friend class fixed_vector<T>;
		private:
			pointer ptr_;
		};

		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		fixed_vector(std::size_t size) : max_size_(size), size_(0), should_delete_(true) {
			data_ = new T[size];
		}

		fixed_vector(const fixed_vector& o) {
			data_ = new T[o.max_size_];
			std::memcpy(data_, o.data_, sizeof(T) * o.max_size_);

			max_size_ = o.max_size_;
			size_ = o.size_;
			should_delete_ = true;
		}

		fixed_vector(fixed_vector&& o) {
			data_ = o.data_;
			max_size_ = o.max_size_;
			size_ = o.size_;
			should_delete_ = true;
			o.should_delete_ = false;
		}

		~fixed_vector() {
			if( should_delete_ )
				delete[] data_;
		}

		fixed_vector() : fixed_vector(1) {}

		iterator begin() noexcept { return iterator(data_); }
		const_iterator begin() const noexcept { return const_iterator(data_); }
		const_iterator cbegin() noexcept { return const_iterator(data_); }

		iterator end() noexcept { return iterator(data_ + size_); }
		const_iterator end() const noexcept { return const_iterator(data_ + size_); }
		const_iterator cend() noexcept { return const_iterator(data_ + size_); }

		T* data() noexcept { return data_; }

		bool empty() { return size_ == 0; }
		std::size_t size() { return size_; }
		std::size_t max_size() { return max_size_; }

		void clear() noexcept { size_ = 0; }
		iterator insert( iterator pos, const T& value ) {
			if (size_ + 1 > max_size_) throw std::length_error(std::string("attempt to increase fixed_vector size beyond ") + std::to_string(max_size_));
			memmove( pos.ptr_ + 1, pos.ptr_, sizeof(T) * (data_ + size_ - pos.ptr_) );

			*pos = T(value);
			size_++;
			return iterator(pos.ptr_);
		}

		void assign( std::initializer_list<T> ilist ) {
			/*if (ilist.size() > max_size_)
				throw std::length_error(
					"attempt to initialize " + std::to_string(max_size_) + 
					" max length fixed_vector from list of size" +
					std::to_string(ilist.size()));*/

			memcpy( data_, ilist.begin(), sizeof(T) * std::min(ilist.size(), max_size_));
			size_ = ilist.size();
		}

		auto operator=( std::initializer_list<T> ilist ) {
			assign(ilist);
			return *this;
		}

		void push_back(const value_type& val) {
			if (size_ + 1 > max_size_) throw std::length_error(std::string("attempt to increase fixed_vector size beyond ") + std::to_string(max_size_));
			data_[size_++] = T(val);
		}

		void pop_back() { if (size_ > 0) size_--; }


	private:
		std::size_t size_;
		std::size_t max_size_;

		bool should_delete_;

		T* data_;
	};
}
