#include <stdlib.h>
#include "tote/hash_map.h"
#include "test_alloc.inl"
#include <doctest/doctest.h>
namespace {
void* Allocate(const uint32_t size, void*) {
  return malloc(size);
}
void Deallocate(void* ptr, void*) {
  free(ptr);
}
} // namespace
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
TEST_CASE("power of 2 align") {
  using namespace tote;
  CHECK_EQ(Align(0, 2), 0);
  CHECK_EQ(Align(1, 2), 2);
  CHECK_EQ(Align(2, 2), 2);
  CHECK_EQ(Align(3, 2), 4);
  CHECK_EQ(Align(4, 2), 4);
  CHECK_EQ(Align(4, 8), 8);
  CHECK_EQ(Align(5, 8), 8);
  CHECK_EQ(Align(6, 8), 8);
  CHECK_EQ(Align(7, 8), 8);
  CHECK_EQ(Align(8, 8), 8);
  CHECK_EQ(Align(8, 16), 16);
  CHECK_EQ(Align(17, 16), 32);
}
TEST_CASE("hash map") {
  using namespace tote;
  UserContext user_context{};
  AllocatorCallbacks<UserContext> allocator_callbacks {
    .allocate = Allocate,
    .deallocate = Deallocate,
    .user_context = &user_context,
  };
  HashMap<uint32_t, UserContext> hash_map(allocator_callbacks, 5);
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
  hash_map.~HashMap();
  CHECK_EQ(user_context.alloc_count, user_context.dealloc_count);
}