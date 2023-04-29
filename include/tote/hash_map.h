#pragma once
#include <cstdint>
#include <string.h>
#include <utility>
#include "allocator_callbacks.h"
namespace tote {
/**
 * HashMap using open addressing.
 **/
template <typename K, typename V, typename U>
class HashMap final {
 public:
  HashMap(AllocatorCallbacks<U> allocator_callbacks, const uint32_t initial_capacity = 0);
  HashMap(HashMap&&);
  HashMap& operator=(HashMap&&);
  ~HashMap();
  constexpr uint32_t size() const { return size_; }
  constexpr uint32_t capacity() const { return capacity_; }
  constexpr bool empty() const { return size() == 0; }
  /**
   * clear entries and reset size to zero.
   * destructor for T is not called.
   **/
  void clear();
  /**
   * release allocated buffer which reduces size and capacity to zero.
   * destructor for T is not called.
   **/
  void release_allocated_buffer();
  void insert(const K, V);
  void erase(const K);
  bool contains(const K) const;
  V& operator[](const K);
  const V& operator[](const K) const;
  using SimpleIteratorFunction = void (*)(const K, V*);
  void iterate(SimpleIteratorFunction&&);
  template <typename T>
  using IteratorFunction = void (*)(T*, const K, V*);
  template <typename T>
  void iterate(IteratorFunction<T>&&, T*);
 private:
  uint32_t find_slot_index(const K) const;
  bool check_load_factor_and_resize();
  void change_capacity(const uint32_t new_capacity);
  void insert_impl(const uint32_t, const K, V value);
  AllocatorCallbacks<U> allocator_callbacks_;
  bool* occupied_flags_{};
  K* keys_{};
  V* values_{};
  uint32_t size_{};
  uint32_t capacity_{}; // always >0 for simple implementation.
  HashMap() = delete;
  HashMap(const HashMap&) = delete;
  void operator=(const HashMap&) = delete;
};
bool IsPrimeNumber(const uint32_t);
uint32_t GetLargerOrEqualPrimeNumber(const uint32_t);
bool IsCloseToFull(const uint32_t load, const uint32_t capacity);
uint32_t Align(const uint32_t val, const uint32_t alignment);
template <typename K, typename V, typename U>
HashMap<K, V, U>::HashMap(AllocatorCallbacks<U> allocator_callbacks, const uint32_t initial_capacity)
    : allocator_callbacks_(allocator_callbacks)
    , size_(0)
    , capacity_(0)
{
  change_capacity(GetLargerOrEqualPrimeNumber(initial_capacity));
}
template <typename K, typename V, typename U>
HashMap<K, V, U>::HashMap(HashMap&& other)
    : allocator_callbacks_(std::move(other.allocator_callbacks_))
    , occupied_flags_(other.occupied_flags_)
    , keys_(other.keys_)
    , values_(other.values_)
    , size_(other.size_)
    , capacity_(other.capacity_)
{
  other.allocator_callbacks_ = {};
  other.occupied_flags_ = nullptr;
  other.keys_ = nullptr;
  other.values_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}
template <typename K, typename V, typename U>
HashMap<K, V, U>& HashMap<K, V, U>::operator=(HashMap&& other)
{
  if (this != &other) {
    if (capacity_ > 0) {
      allocator_callbacks_.deallocate(occupied_flags_, allocator_callbacks_.user_context);
      allocator_callbacks_.deallocate(keys_, allocator_callbacks_.user_context);
      allocator_callbacks_.deallocate(values_, allocator_callbacks_.user_context);
    }
    allocator_callbacks_ = std::move(other.allocator_callbacks_);
    occupied_flags_ = other.occupied_flags_;
    keys_ = other.keys_;
    values_ = other.values_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.allocator_callbacks_ = {};
    other.occupied_flags_ = nullptr;
    other.keys_ = nullptr;
    other.values_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }
  return *this;
}
template <typename K, typename V, typename U>
HashMap<K, V, U>::~HashMap() {
  release_allocated_buffer();
}
template <typename K, typename V, typename U>
void HashMap<K, V, U>::clear() {
  if (capacity_ > 0) {
    memset(occupied_flags_, 0, sizeof(occupied_flags_[0]) * capacity_);
  }
  size_ = 0;
}
template <typename K, typename V, typename U>
void HashMap<K, V, U>::release_allocated_buffer() {
  if (capacity_ > 0) {
    allocator_callbacks_.deallocate(occupied_flags_, allocator_callbacks_.user_context);
    allocator_callbacks_.deallocate(keys_, allocator_callbacks_.user_context);
    allocator_callbacks_.deallocate(values_, allocator_callbacks_.user_context);
    capacity_ = 0;
  }
  size_ = 0;
}
template <typename K, typename V, typename U>
void HashMap<K, V, U>::insert(const K key, V value) {
  auto index = capacity_ > 0 ? find_slot_index(key) : ~0U;
  if (index != ~0U && occupied_flags_[index]) {
    values_[index] = value;
    return;
  }
  size_++;
  if (check_load_factor_and_resize()) {
    index = find_slot_index(key);
  }
  insert_impl(index, key, value);
}
template <typename K, typename V, typename U>
void HashMap<K, V, U>::insert_impl(const uint32_t index, const K key, V value) {
  occupied_flags_[index] = true;
  keys_[index] = key;
  values_[index] = value;
}
template <typename K, typename V, typename U>
void HashMap<K, V, U>::erase(const K key) {
  auto i = find_slot_index(key);
  if (!occupied_flags_[i]) { return; }
  occupied_flags_[i] = false;
  auto j = i;
  while (true) {
    j = (j + 1) % capacity_;
    if (!occupied_flags_[j]) { break; }
    auto k = keys_[j] % capacity_;
    if (i <= j) {
      if (i < k && k <= j) {
        continue;
      }
    } else {
      if (i < k || k <= j) {
        continue;
      }
    }
    occupied_flags_[i] = occupied_flags_[j];
    keys_[i] = keys_[j];
    values_[i] = values_[j];
    occupied_flags_[j] = false;
    i = j;
  }
  size_--;
}
template <typename K, typename V, typename U>
bool HashMap<K, V, U>::contains(const K key) const {
  if (size_ == 0) { return false; }
  const auto index = find_slot_index(key);
  return occupied_flags_[index];
}
template <typename K, typename V, typename U>
V& HashMap<K, V, U>::operator[](const K key) {
  if (!contains(key)) {
    insert(key, {});
  }
  const auto index = find_slot_index(key);
  return values_[index];
}
template <typename K, typename V, typename U>
const V& HashMap<K, V, U>::operator[](const K key) const {
  const auto index = find_slot_index(key);
  return values_[index];
}
template <typename K, typename V, typename U>
void HashMap<K, V, U>::iterate(SimpleIteratorFunction&& f) {
  for (uint32_t i = 0; i < capacity_; i++) {
    if (!occupied_flags_[i]) { continue; }
    f(keys_[i], &values_[i]);
  }
}
template <typename K, typename V, typename U>
template <typename T>
void HashMap<K, V, U>::iterate(IteratorFunction<T>&& f, T* entity) {
  for (uint32_t i = 0; i < capacity_; i++) {
    if (!occupied_flags_[i]) { continue; }
    f(entity, keys_[i], &values_[i]);
  }
}
template <typename K, typename V, typename U>
uint32_t HashMap<K, V, U>::find_slot_index(const K key) const {
  auto index = key % capacity_;
  while (occupied_flags_[index] && keys_[index] != key) {
    index = (index + 1) % capacity_;
  }
  if constexpr (sizeof(K) == 4) { return index; }
  return static_cast<uint32_t>(index);
}
template <typename K, typename V, typename U>
bool HashMap<K, V, U>::check_load_factor_and_resize() {
  if (!IsCloseToFull(size_, capacity_)) { return false; }
  change_capacity(GetLargerOrEqualPrimeNumber(capacity_ + 2));
  return true;
}
template <typename K, typename V, typename U>
void HashMap<K, V, U>::change_capacity(const uint32_t new_capacity) {
  if (capacity_ >= new_capacity) { return; }
  const auto prev_capacity = capacity_;
  const auto prev_size = size_;
  const auto prev_occupied_flags = occupied_flags_;
  const auto prev_keys = keys_;
  const auto prev_values = values_;
  capacity_ = new_capacity;
  {
    occupied_flags_ = static_cast<bool*>(allocator_callbacks_.allocate(sizeof(occupied_flags_[0]) * capacity_, alignof(bool), allocator_callbacks_.user_context));
    keys_ = static_cast<K*>(allocator_callbacks_.allocate(sizeof(K) * capacity_, alignof(K), allocator_callbacks_.user_context));
    values_ = static_cast<V*>(allocator_callbacks_.allocate(sizeof(V) * capacity_, alignof(V), allocator_callbacks_.user_context));
  }
  clear();
  for (uint32_t i = 0; i < prev_capacity; i++) {
    if (prev_occupied_flags[i]) {
      const auto index = find_slot_index(prev_keys[i]);
      insert_impl(index, prev_keys[i], prev_values[i]);
    }
  }
  size_ = prev_size;
  if (prev_capacity > 0) {
    allocator_callbacks_.deallocate(prev_occupied_flags, allocator_callbacks_.user_context);
    allocator_callbacks_.deallocate(prev_keys, allocator_callbacks_.user_context);
    allocator_callbacks_.deallocate(prev_values, allocator_callbacks_.user_context);
  }
}
} // namespace tote
#undef TOTE_HASH_KEY_TYPE
#undef TOTE_ALIGNMENT_BYTE
