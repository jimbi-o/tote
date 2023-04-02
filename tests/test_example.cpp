#include <stdlib.h>
#include <doctest/doctest.h>
#include "tote/tote_array.h"
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
  CHECK_UNARY(!resizable_array.empty());
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
  CHECK_UNARY(!resizable_array.empty());
  CHECK_EQ(resizable_array.size(), 2);
  CHECK_GE(resizable_array.capacity(), 2);
}
