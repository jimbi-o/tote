#include <stdlib.h>
#include <doctest/doctest.h>
#include "tote/tote_array.h"
namespace tote {
template <typename K, typename V>
class HashMap final {
 public:
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
   * release allocated buffer reducing size to zero.
   * destructor for T is not called.
   **/
  void release_allocated_buffer();
  void insert(const K, V);
  void emplace(const K, V&&);
  void erase(const K);
  bool contains(const K) const;
  V& operator[](const K);
  const V& operator[](const K) const;
  template <typename T>
  using IteratorFunction = void (*)(T*, const K, V*);
  template <typename T>
  void iterate(IteratorFunction<T>, T*);
 private:
  void change_capacity(const uint32_t new_capacity);
  AllocatorCallbacks allocator_callbacks_;
  uint32_t size_;
  uint32_t capacity_;
};
bool IsPrimeNumber(const uint32_t);
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
  resizable_array.emplace_back(0);
  resizable_array.emplace_back(1);
  CHECK_EQ(resizable_array.size(), 2);
  CHECK_EQ(resizable_array[0], 0);
  CHECK_EQ(resizable_array[1], 1);
  resizable_array.release_allocated_buffer();
  CHECK_UNARY(resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 0);
  CHECK_EQ(resizable_array.capacity(), 0);
  resizable_array.emplace_back(0);
  resizable_array.emplace_back(1);
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
TEST_CASE("hash map") {
  using namespace tote;
  AllocatorCallbacks allocator_callbacks {
    .allocate = Allocate,
    .deallocate = Deallocate,
    .user_context = nullptr,
  };
  HashMap<uint32_t, uint32_t> hash_map(allocator_callbacks, 5);
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
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  hash_map.insert(1, 2);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  hash_map.insert(2, 3);
  CHECK_UNARY(hash_map.contains(0));
  CHECK_UNARY(hash_map.contains(1));
  CHECK_UNARY(hash_map.contains(2));
  CHECK_EQ(hash_map[0], 1);
  CHECK_EQ(hash_map[1], 2);
  CHECK_EQ(hash_map[2], 3);
  hash_map.insert(3, 4);
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
  CHECK_EQ(hash_map.capacity(), 5);
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
  hash_map.erase(0);
  CHECK_EQ(hash_map.size(), 4);
  CHECK_EQ(hash_map.capacity(), 5);
  CHECK_UNARY_FALSE(hash_map.contains(0));
  hash_map.emplace(5, 6);
  CHECK_EQ(hash_map.size(), 5);
  CHECK_EQ(hash_map.capacity(), 5);
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
  hash_map.emplace(5, 6);
  CHECK_EQ(hash_map.size(), 5);
  CHECK_EQ(hash_map.capacity(), 5);
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
  hash_map.emplace(6, 7);
  CHECK_EQ(hash_map.size(), 6);
  CHECK_GT(hash_map.capacity(), 6);
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
  const auto capacity = hash_map.capacity();
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
  hash_map.emplace(100, 101);
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
  hash_map.release_allocated_buffer();
  CHECK_UNARY_FALSE(hash_map.empty());
  CHECK_EQ(hash_map.size(), 0);
  CHECK_EQ(hash_map.capacity(), 0);
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
  CHECK_EQ(entity.count, hash_map.size());
  CHECK_EQ(entity.key_sum, key_sum_calculated);
  CHECK_EQ(entity.sum, sum_calculated);
}
