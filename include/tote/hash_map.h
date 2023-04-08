#pragma once
#include <cstdint>
#include <string.h>
#include <utility>
#include "allocator_callbacks.h"
#ifndef TOTE_HASH_KEY_TYPE
#define TOTE_HASH_KEY_TYPE uint32_t
#endif
#ifndef TOTE_ALIGNMENT_BYTE
#define TOTE_ALIGNMENT_BYTE 8
#endif
namespace tote {
/**
 * HashMap using open addressing.
 **/
template <typename V, typename U>
class HashMap final {
 public:
  using KeyType = TOTE_HASH_KEY_TYPE;
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
   * clears the map and reduce capacity to minimum.
   * destructor for T is not called.
   **/
  void clear_and_shrink_capacity();
  void insert(const KeyType, V);
  void erase(const KeyType);
  bool contains(const KeyType) const;
  V& operator[](const KeyType);
  const V& operator[](const KeyType) const;
  template <typename T>
  using IteratorFunction = void (*)(T*, const KeyType, V*);
  template <typename T>
  void iterate(IteratorFunction<T>, T*);
 private:
  static const uint32_t kAlignmentByte = TOTE_ALIGNMENT_BYTE;
  uint32_t find_slot_index(const KeyType) const;
  bool check_load_factor_and_resize();
  void change_capacity(const uint32_t new_capacity);
  void insert_impl(const uint32_t, const KeyType, V value);
  void release_allocated_buffer();
  AllocatorCallbacks<U> allocator_callbacks_;
  bool* occupied_flags_{};
  KeyType* keys_{};
  V* values_{};
  uint32_t size_{};
  uint32_t capacity_{}; // always >0 for simple implementation.
  void* ptr_{};
  HashMap() = delete;
  HashMap(const HashMap&) = delete;
  void operator=(const HashMap&) = delete;
};
bool IsPrimeNumber(const uint32_t);
uint32_t GetLargerOrEqualPrimeNumber(const uint32_t);
bool IsCloseToFull(const uint32_t load, const uint32_t capacity);
uint32_t Align(const uint32_t val, const uint32_t alignment);
template <typename V, typename U>
HashMap<V, U>::HashMap(AllocatorCallbacks<U> allocator_callbacks, const uint32_t initial_capacity)
    : allocator_callbacks_(allocator_callbacks)
    , size_(0)
    , capacity_(0)
{
  change_capacity(GetLargerOrEqualPrimeNumber(initial_capacity));
}
template <typename V, typename U>
HashMap<V, U>::HashMap(HashMap&& other)
    : allocator_callbacks_(std::move(other.allocator_callbacks_))
    , occupied_flags_(other.occupied_flags_)
    , keys_(other.keys_)
    , values_(other.values_)
    , size_(other.size_)
    , capacity_(other.capacity_)
    , ptr_(other.ptr_)
{
  other.allocator_callbacks_ = {};
  other.occupied_flags_ = nullptr;
  other.keys_ = nullptr;
  other.values_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
  other.ptr_ = nullptr;
}
template <typename V, typename U>
HashMap<V, U>& HashMap<V, U>::operator=(HashMap&& other)
{
  if (this != &other) {
    if (ptr_) {
      allocator_callbacks_.deallocate(ptr_, allocator_callbacks_.user_context);
    }
    allocator_callbacks_ = std::move(other.allocator_callbacks_);
    occupied_flags_ = other.occupied_flags_;
    keys_ = other.keys_;
    values_ = other.values_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    ptr_ = other.ptr_;
    other.allocator_callbacks_ = {};
    other.occupied_flags_ = nullptr;
    other.keys_ = nullptr;
    other.values_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    other.ptr_ = nullptr;
  }
  return *this;
}
template <typename V, typename U>
HashMap<V, U>::~HashMap() {
  release_allocated_buffer();
}
template <typename V, typename U>
void HashMap<V, U>::clear() {
  if (capacity_ > 0) {
    memset(occupied_flags_, 0, sizeof(occupied_flags_[0]) * capacity_);
  }
  size_ = 0;
}
template <typename V, typename U>
void HashMap<V, U>::clear_and_shrink_capacity() {
  release_allocated_buffer();
  change_capacity(2);
}
template <typename V, typename U>
void HashMap<V, U>::release_allocated_buffer() {
  if (capacity_ > 0) {
    allocator_callbacks_.deallocate(ptr_, allocator_callbacks_.user_context);
    capacity_ = 0;
  }
  size_ = 0;
}
template <typename V, typename U>
void HashMap<V, U>::insert(const KeyType key, V value) {
  auto index = find_slot_index(key);
  if (occupied_flags_[index]) {
    values_[index] = value;
    return;
  }
  size_++;
  if (check_load_factor_and_resize()) {
    index = find_slot_index(key);
  }
  insert_impl(index, key, value);
}
template <typename V, typename U>
void HashMap<V, U>::insert_impl(const uint32_t index, const KeyType key, V value) {
  occupied_flags_[index] = true;
  keys_[index] = key;
  values_[index] = value;
}
template <typename V, typename U>
void HashMap<V, U>::erase(const KeyType key) {
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
template <typename V, typename U>
bool HashMap<V, U>::contains(const KeyType key) const {
  const auto index = find_slot_index(key);
  return occupied_flags_[index];
}
template <typename V, typename U>
V& HashMap<V, U>::operator[](const KeyType key) {
  const auto index = find_slot_index(key);
  return values_[index];
}
template <typename V, typename U>
const V& HashMap<V, U>::operator[](const KeyType key) const {
  const auto index = find_slot_index(key);
  return values_[index];
}
template <typename V, typename U>
template <typename T>
void HashMap<V, U>::iterate(IteratorFunction<T> f, T* entity) {
  for (uint32_t i = 0; i < capacity_; i++) {
    if (!occupied_flags_[i]) { continue; }
    f(entity, keys_[i], &values_[i]);
  }
}
template <typename V, typename U>
uint32_t HashMap<V, U>::find_slot_index(const KeyType key) const {
  auto index = key % capacity_;
  while (occupied_flags_[index] && keys_[index] != key) {
    index = (index + 1) % capacity_;
  }
  return index;
}
template <typename V, typename U>
bool HashMap<V, U>::check_load_factor_and_resize() {
  if (!IsCloseToFull(size_, capacity_)) { return false; }
  change_capacity(GetLargerOrEqualPrimeNumber(capacity_ + 2));
  return true;
}
template <typename V, typename U>
void HashMap<V, U>::change_capacity(const uint32_t new_capacity) {
  if (capacity_ >= new_capacity) { return; }
  const auto prev_capacity = capacity_;
  const auto prev_size = size_;
  const auto prev_occupied_flags = occupied_flags_;
  const auto prev_keys = keys_;
  const auto prev_values = values_;
  const auto prev_ptr = ptr_;
  capacity_ = new_capacity;
  {
    const auto occupied_flags_size_in_bytes = static_cast<uint32_t>(sizeof(occupied_flags_[0])) * capacity_;
    const auto keys_size_in_bytes = static_cast<uint32_t>(sizeof(keys_[0])) * capacity_;
    const auto values_size_in_bytes = static_cast<uint32_t>(sizeof(values_[0])) * capacity_;
    const auto offset1 = Align(occupied_flags_size_in_bytes, kAlignmentByte);
    const auto offset2 = Align(offset1 + keys_size_in_bytes, kAlignmentByte);
    ptr_ = allocator_callbacks_.allocate(offset2 + values_size_in_bytes, allocator_callbacks_.user_context);
    occupied_flags_ = static_cast<bool*>(ptr_);
    keys_ = static_cast<KeyType*>(reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(ptr_) + offset1));
    values_ = static_cast<V*>(reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(ptr_) + offset2));
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
    allocator_callbacks_.deallocate(prev_ptr, allocator_callbacks_.user_context);
  }
}
} // namespace tote
