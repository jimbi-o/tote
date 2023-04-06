#include <stdlib.h>
#include <doctest/doctest.h>
#include "tote/array.h"

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
namespace tote {
bool IsPrimeNumber(const uint32_t n) {
  if (n <= 1) { return false; }
  for (uint32_t i = 2; i * i <= n; i++) {
    if (n % i == 0) { return false; }
  }
  return true;
}
uint32_t GetLargerOrEqualPrimeNumber(const uint32_t n) {
  if (IsPrimeNumber(n)) { return n; }
  if (n <= 2) { return 2; }
  auto p = n + 1 + n % 2; // odd number larger than n.
  while (!IsPrimeNumber(p)) {
    p += 2;
  }
  return p;
}
bool IsCloseToFull(const uint32_t load, const uint32_t capacity) {
  const float loadFactor = 0.65f;
  return static_cast<float>(load) / static_cast<float>(capacity) >= loadFactor;
}
} // namespace tote
namespace {
void* Allocate(const uint32_t size, void*) {
  return malloc(size);
}
void Deallocate(void* ptr, void*) {
  free(ptr);
}
} // namespace
TEST_CASE("resizable array") {
  using namespace tote;
  AllocatorCallbacks allocator_callbacks {
    .allocate = Allocate,
    .deallocate = Deallocate,
    .user_context = nullptr,
  };
  ResizableArray<uint32_t> resizable_array(allocator_callbacks, 0, 4);
  CHECK_UNARY(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 0);
  CHECK_EQ(resizable_array.capacity(), 4);
  resizable_array.push_back(0);
  CHECK_EQ(resizable_array.front(), 0);
  CHECK_EQ(resizable_array.back(), 0);
  resizable_array.push_back(1);
  resizable_array.push_back(2);
  CHECK_UNARY_FALSE(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 3);
  CHECK_EQ(resizable_array.capacity(), 4);
  CHECK_EQ(resizable_array[0], 0);
  CHECK_EQ(resizable_array[1], 1);
  CHECK_EQ(resizable_array[2], 2);
  CHECK_EQ(resizable_array.front(), 0);
  CHECK_EQ(resizable_array.back(), 2);
  auto it = resizable_array.begin();
  CHECK_EQ(*it, 0);
  it++;
  CHECK_EQ(*it, 1);
  it++;
  CHECK_EQ(*it, 2);
  it++;
  CHECK_EQ(it, resizable_array.end());
  resizable_array[0] = 99;
  CHECK_EQ(resizable_array[0], 99);
  CHECK_EQ(resizable_array[1], 1);
  CHECK_EQ(resizable_array[2], 2);
  resizable_array[1] = 18;
  CHECK_EQ(resizable_array[0], 99);
  CHECK_EQ(resizable_array[1], 18);
  CHECK_EQ(resizable_array[2], 2);
  resizable_array[2] = 21;
  CHECK_EQ(resizable_array[0], 99);
  CHECK_EQ(resizable_array[1], 18);
  CHECK_EQ(resizable_array[2], 21);
  resizable_array.push_back(3);
  CHECK_EQ(resizable_array.size(), 4);
  CHECK_EQ(resizable_array.capacity(), 4);
  CHECK_EQ(resizable_array[0], 99);
  CHECK_EQ(resizable_array[1], 18);
  CHECK_EQ(resizable_array[2], 21);
  CHECK_EQ(resizable_array[3], 3);
  resizable_array.push_back(4);
  CHECK_EQ(resizable_array.size(), 5);
  CHECK_GE(resizable_array.capacity(), 5);
  CHECK_EQ(resizable_array[0], 99);
  CHECK_EQ(resizable_array[1], 18);
  CHECK_EQ(resizable_array[2], 21);
  CHECK_EQ(resizable_array[3], 3);
  CHECK_EQ(resizable_array[4], 4);
  CHECK_EQ(resizable_array.front(), 99);
  CHECK_EQ(resizable_array.back(), 4);
  CHECK_NE(it, resizable_array.end());
  const auto capacity = resizable_array.capacity();
  resizable_array.clear();
  CHECK_UNARY(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 0);
  CHECK_EQ(resizable_array.capacity(), capacity);
  resizable_array.push_back(0);
  resizable_array.push_back(1);
  CHECK_EQ(resizable_array.size(), 2);
  CHECK_EQ(resizable_array[0], 0);
  CHECK_EQ(resizable_array[1], 1);
  resizable_array.release_allocated_buffer();
  CHECK_UNARY(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 0);
  CHECK_EQ(resizable_array.capacity(), 0);
  resizable_array.push_back(0);
  resizable_array.push_back(1);
  CHECK_EQ(resizable_array.size(), 2);
  CHECK_EQ(resizable_array[0], 0);
  CHECK_EQ(resizable_array[1], 1);
  CHECK_UNARY_FALSE(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 2);
  CHECK_GE(resizable_array.capacity(), 2);
}
TEST_CASE("empty resizable array") {
  using namespace tote;
  ResizableArray<uint32_t> resizable_array({.allocate = Allocate, .deallocate = Deallocate, .user_context = nullptr,}, 0, 0);
  CHECK_UNARY(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 0);
  CHECK_EQ(resizable_array.capacity(), 0);
  resizable_array.push_back(0);
  CHECK_EQ(resizable_array.front(), 0);
  CHECK_EQ(resizable_array.back(), 0);
  CHECK_UNARY_FALSE(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 1);
  CHECK_GT(resizable_array.capacity(), 0);
}
TEST_CASE("prime number") {
  using namespace tote;
  CHECK_UNARY_FALSE(IsPrimeNumber(0));
  CHECK_UNARY_FALSE(IsPrimeNumber(1));
  CHECK_UNARY(IsPrimeNumber(2));
  CHECK_UNARY(IsPrimeNumber(3));
  CHECK_UNARY_FALSE(IsPrimeNumber(4));
  CHECK_UNARY(IsPrimeNumber(5));
  CHECK_UNARY_FALSE(IsPrimeNumber(6));
  CHECK_UNARY(IsPrimeNumber(7));
  CHECK_UNARY_FALSE(IsPrimeNumber(1000));
  CHECK_UNARY(IsPrimeNumber(1013));
  CHECK_UNARY_FALSE(IsPrimeNumber(1014));
  CHECK_UNARY_FALSE(IsPrimeNumber(1015));
  CHECK_EQ(GetLargerOrEqualPrimeNumber(0), 2);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(1), 2);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(2), 2);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(3), 3);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(4), 5);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(5), 5);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(6), 7);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(7), 7);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(8), 11);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(1011), 1013);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(1013), 1013);
  CHECK_EQ(GetLargerOrEqualPrimeNumber(1013), 1013);
}
TEST_CASE("hash map") {
  using namespace tote;
  AllocatorCallbacks allocator_callbacks {
    .allocate = Allocate,
    .deallocate = Deallocate,
    .user_context = nullptr,
  };
  HashMap<uint32_t> hash_map(allocator_callbacks, 5);
  CHECK_UNARY(hash_map.empty());
  CHECK_EQ(hash_map.size(), 0);
  CHECK_EQ(hash_map.capacity(), 5);
  hash_map.insert(0, 1);
  CHECK_UNARY_FALSE(hash_map.empty());
  CHECK_UNARY(hash_map.contains(0));
  CHECK_EQ(hash_map.size(), 1);
  CHECK_EQ(hash_map.capacity(), 5);
  CHECK_EQ(hash_map[0], 1);
  hash_map.insert(0, 1);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_EQ(hash_map.size(), 1);
  CHECK_EQ(hash_map.capacity(), 5);
  CHECK_EQ(hash_map[0], 1);
  hash_map.insert(1, 2);
  CHECK_EQ(hash_map.size(), 2);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  hash_map.insert(1, 2);
  CHECK_EQ(hash_map.size(), 2);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  hash_map.insert(2, 3);
  CHECK_EQ(hash_map.size(), 3);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_UNARY(hash_map.contains(2));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  CHECK_EQ(hash_map[2], 3);
  hash_map.insert(3, 4);
  CHECK_EQ(hash_map.size(), 4);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_UNARY(hash_map.contains(2));
  CHECK_UNARY(hash_map.contains(3));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  CHECK_EQ(hash_map[2], 3);
  CHECK_EQ(hash_map[3], 4);
  hash_map.insert(4, 5);
  CHECK_EQ(hash_map.size(), 5);
  CHECK_GT(hash_map.capacity(), 5);
  CHECK_LE(hash_map.capacity(), 11);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_UNARY(hash_map.contains(2));
  CHECK_UNARY(hash_map.contains(3));
  CHECK_UNARY(hash_map.contains(4));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  CHECK_EQ(hash_map[2], 3);
  CHECK_EQ(hash_map[3], 4);
  CHECK_EQ(hash_map[4], 5);
  auto capacity = hash_map.capacity();
  hash_map.erase(0);
  CHECK_EQ(hash_map.size(), 4);
  CHECK_EQ(hash_map.capacity(), capacity);
  CHECK_UNARY_FALSE(hash_map.contains(0));
  hash_map.insert(5, 6);
  CHECK_EQ(hash_map.size(), 5);
  CHECK_GE(hash_map.capacity(), capacity);
  CHECK_LE(hash_map.capacity(), capacity * 2);
  CHECK_UNARY_FALSE(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_UNARY(hash_map.contains(2));
  CHECK_UNARY(hash_map.contains(3));
  CHECK_UNARY(hash_map.contains(4));
  CHECK_UNARY(hash_map.contains(5));
  CHECK_EQ(hash_map[1], 2);
  CHECK_EQ(hash_map[2], 3);
  CHECK_EQ(hash_map[3], 4);
  CHECK_EQ(hash_map[4], 5);
  CHECK_EQ(hash_map[5], 6);
  hash_map.insert(5, 6);
  CHECK_EQ(hash_map.size(), 5);
  CHECK_GE(hash_map.capacity(), capacity);
  CHECK_LE(hash_map.capacity(), capacity * 2);
  CHECK_UNARY(hash_map.contains(1));
  CHECK_UNARY(hash_map.contains(2));
  CHECK_UNARY(hash_map.contains(3));
  CHECK_UNARY(hash_map.contains(4));
  CHECK_UNARY(hash_map.contains(5));
  CHECK_EQ(hash_map[1], 2);
  CHECK_EQ(hash_map[2], 3);
  CHECK_EQ(hash_map[3], 4);
  CHECK_EQ(hash_map[4], 5);
  CHECK_EQ(hash_map[5], 6);
  hash_map.insert(6, 7);
  CHECK_EQ(hash_map.size(), 6);
  CHECK_GE(hash_map.capacity(), capacity);
  CHECK_LT(hash_map.capacity(), capacity * 3);
  CHECK_UNARY(IsPrimeNumber(hash_map.capacity()));
  CHECK_UNARY_FALSE(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_UNARY(hash_map.contains(2));
  CHECK_UNARY(hash_map.contains(3));
  CHECK_UNARY(hash_map.contains(4));
  CHECK_UNARY(hash_map.contains(5));
  CHECK_UNARY(hash_map.contains(6));
  CHECK_EQ(hash_map[1], 2);
  CHECK_EQ(hash_map[2], 3);
  CHECK_EQ(hash_map[3], 4);
  CHECK_EQ(hash_map[4], 5);
  CHECK_EQ(hash_map[5], 6);
  CHECK_EQ(hash_map[6], 7);
  CHECK_UNARY_FALSE(hash_map.empty());
  capacity = hash_map.capacity();
  hash_map.clear();
  CHECK_UNARY(hash_map.empty());
  CHECK_EQ(hash_map.size(), 0);
  CHECK_EQ(hash_map.capacity(), capacity);
  CHECK_UNARY_FALSE(hash_map.contains(0));
  CHECK_UNARY_FALSE(hash_map.contains(1));
  CHECK_UNARY_FALSE(hash_map.contains(2));
  CHECK_UNARY_FALSE(hash_map.contains(3));
  CHECK_UNARY_FALSE(hash_map.contains(4));
  CHECK_UNARY_FALSE(hash_map.contains(5));
  CHECK_UNARY_FALSE(hash_map.contains(6));
  hash_map.insert(100, 101);
  CHECK_UNARY_FALSE(hash_map.empty());
  CHECK_EQ(hash_map.size(), 1);
  CHECK_EQ(hash_map.capacity(), capacity);
  CHECK_UNARY_FALSE(hash_map.contains(0));
  CHECK_UNARY_FALSE(hash_map.contains(1));
  CHECK_UNARY_FALSE(hash_map.contains(2));
  CHECK_UNARY_FALSE(hash_map.contains(3));
  CHECK_UNARY_FALSE(hash_map.contains(4));
  CHECK_UNARY_FALSE(hash_map.contains(5));
  CHECK_UNARY_FALSE(hash_map.contains(6));
  CHECK_UNARY(hash_map.contains(100));
  hash_map.clear_and_shrink_capacity();
  CHECK_UNARY(hash_map.empty());
  CHECK_EQ(hash_map.size(), 0);
  CHECK_EQ(hash_map.capacity(), 2);
  CHECK_UNARY_FALSE(hash_map.contains(0));
  CHECK_UNARY_FALSE(hash_map.contains(1));
  CHECK_UNARY_FALSE(hash_map.contains(2));
  CHECK_UNARY_FALSE(hash_map.contains(3));
  CHECK_UNARY_FALSE(hash_map.contains(4));
  CHECK_UNARY_FALSE(hash_map.contains(5));
  CHECK_UNARY_FALSE(hash_map.contains(6));
  CHECK_UNARY_FALSE(hash_map.contains(100));
  uint32_t key_sum_calculated = 0;
  uint32_t sum_calculated = 0;
  for (uint32_t i = 0; i < 101; i++) {
    hash_map.insert(i, i + 100);
    key_sum_calculated += i;
    sum_calculated += i + 100;
  }
  struct Entity {
    uint32_t count = 0;
    uint32_t key_sum = 0;
    uint32_t sum = 0;
  } entity {};
  auto f = [](Entity* data, const uint32_t key, uint32_t* value) {
    data->count++;
    data->key_sum += key;
    data->sum += *value;
  };
  hash_map.iterate<Entity>(f, &entity);
  CHECK_EQ(hash_map.size(), 101);
  CHECK_LT(hash_map.size(), hash_map.capacity());
  CHECK_GT(hash_map.size() * 2, hash_map.capacity());
  CHECK_EQ(entity.count, hash_map.size());
  CHECK_EQ(entity.key_sum, key_sum_calculated);
  CHECK_EQ(entity.sum, sum_calculated);
}
