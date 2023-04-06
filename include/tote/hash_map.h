#pragma once
#include <stdint.h>
#include <string.h>
#include "allocator_callbacks.h"
#ifndef TOTE_HASH_KEY_TYPE
#define TOTE_HASH_KEY_TYPE uint32_t
#endif
namespace tote {
/**
 * HashMap using open addressing.
 **/
template <typename V>
class HashMap final {
 public:
  using KeyType = TOTE_HASH_KEY_TYPE;
  HashMap(AllocatorCallbacks allocator_callbacks, const uint32_t initial_capacity);
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
  uint32_t find_slot_index(const KeyType) const;
  bool check_load_factor_and_resize();
  void change_capacity(const uint32_t new_capacity);
  void insert_impl(const uint32_t, const KeyType, V value);
  void release_allocated_buffer();
  AllocatorCallbacks allocator_callbacks_;
  bool* occupied_flags_;
  KeyType* keys_;
  V* values_;
  uint32_t size_;
  uint32_t capacity_; // always >0 for simple implementation.
};
bool IsPrimeNumber(const uint32_t);
uint32_t GetLargerOrEqualPrimeNumber(const uint32_t);
bool IsCloseToFull(const uint32_t load, const uint32_t capacity);
template <typename V>
HashMap<V>::HashMap(AllocatorCallbacks allocator_callbacks, const uint32_t initial_capacity)
    : allocator_callbacks_(allocator_callbacks)
    , size_(0)
    , capacity_(0)
{
  change_capacity(GetLargerOrEqualPrimeNumber(initial_capacity));
}
template <typename V>
HashMap<V>::~HashMap() {
  release_allocated_buffer();
}
template <typename V>
void HashMap<V>::clear() {
  if (capacity_ > 0) {
    memset(occupied_flags_, 0, sizeof(occupied_flags_[0]) * capacity_);
  }
  size_ = 0;
}
template <typename V>
void HashMap<V>::clear_and_shrink_capacity() {
  release_allocated_buffer();
  change_capacity(2);
}
template <typename V>
void HashMap<V>::release_allocated_buffer() {
  if (capacity_ > 0) {
    // TODO deallocate at once
    allocator_callbacks_.deallocate(occupied_flags_, allocator_callbacks_.user_context);
    allocator_callbacks_.deallocate(keys_, allocator_callbacks_.user_context);
    allocator_callbacks_.deallocate(values_, allocator_callbacks_.user_context);
    capacity_ = 0;
  }
  size_ = 0;
}
template <typename V>
void HashMap<V>::insert(const KeyType key, V value) {
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
template <typename V>
void HashMap<V>::insert_impl(const uint32_t index, const KeyType key, V value) {
  occupied_flags_[index] = true;
  keys_[index] = key;
  values_[index] = value;
}
template <typename V>
void HashMap<V>::erase(const KeyType key) {
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
template <typename V>
bool HashMap<V>::contains(const KeyType key) const {
  const auto index = find_slot_index(key);
  return occupied_flags_[index];
}
template <typename V>
V& HashMap<V>::operator[](const KeyType key) {
  const auto index = find_slot_index(key);
  return values_[index];
}
template <typename V>
const V& HashMap<V>::operator[](const KeyType key) const {
  const auto index = find_slot_index(key);
  return values_[index];
}
template <typename V>
template <typename T>
void HashMap<V>::iterate(IteratorFunction<T> f, T* entity) {
  for (uint32_t i = 0; i < capacity_; i++) {
    if (!occupied_flags_[i]) { continue; }
    f(entity, keys_[i], &values_[i]);
  }
}
template <typename V>
uint32_t HashMap<V>::find_slot_index(const KeyType key) const {
  auto index = key % capacity_;
  while (occupied_flags_[index] && keys_[index] != key) {
    index = (index + 1) % capacity_;
  }
  return index;
}
template <typename V>
bool HashMap<V>::check_load_factor_and_resize() {
  if (!IsCloseToFull(size_, capacity_)) { return false; }
  change_capacity(GetLargerOrEqualPrimeNumber(capacity_ + 2));
  return true;
}
template <typename V>
void HashMap<V>::change_capacity(const uint32_t new_capacity) {
  if (capacity_ >= new_capacity) { return; }
  const auto prev_capacity = capacity_;
  const auto prev_size = size_;
  const auto prev_occupied_flags = occupied_flags_;
  const auto prev_keys = keys_;
  const auto prev_values = values_;
  capacity_ = new_capacity;
  // TODO allocate at once
  occupied_flags_ = static_cast<bool*>(allocator_callbacks_.allocate(sizeof(occupied_flags_[0]) * capacity_, allocator_callbacks_.user_context));
  keys_ = static_cast<KeyType*>(allocator_callbacks_.allocate(sizeof(keys_[0]) * capacity_, allocator_callbacks_.user_context));
  values_ = static_cast<V*>(allocator_callbacks_.allocate(sizeof(values_[0]) * capacity_, allocator_callbacks_.user_context));
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
