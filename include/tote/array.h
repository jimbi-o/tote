#pragma once
#include <stdint.h>
#include <string.h>
#include <utility>
#include "allocator_callbacks.h"
namespace tote {
template <typename T, typename U>
class ResizableArray final {
 public:
  ResizableArray(AllocatorCallbacks<U> allocator_callbacks, const uint32_t initial_size, const uint32_t initial_capacity);
  ~ResizableArray();
  constexpr uint32_t size() const { return size_; }
  constexpr uint32_t capacity() const { return capacity_; }
  constexpr bool empty() const { return size() == 0; }
  /**
   * reset size to zero.
   * destructor for T is not called.
   **/
  void clear() { size_ = 0; }
  /**
   * release allocated buffer which reduces size and capacity to zero.
   * destructor for T is not called.
   **/
  void release_allocated_buffer();
  void push_back(T);
  T* begin() { return head_; }
  const T* begin() const { return head_; }
  T* end() { return head_ + size_; }
  const T* end() const { return head_ + size_; }
  T& front() { return *head_; }
  const T& front() const { return *head_; }
  T& back() { return *(head_ + size_ - 1); }
  const T& back() const { return *(head_ + size_ - 1); }
  T& operator[](const uint32_t index) { return *(head_ + index); }
  const T& operator[](const uint32_t index) const { return *(head_ + index); }
 private:
  void change_capacity(const uint32_t new_capacity);
  AllocatorCallbacks<U> allocator_callbacks_;
  uint32_t size_;
  uint32_t capacity_;
  T* head_;
};
template <typename T, typename U>
ResizableArray<T, U>::ResizableArray(AllocatorCallbacks<U> allocator_callbacks, const uint32_t initial_size, const uint32_t initial_capacity)
    : allocator_callbacks_(allocator_callbacks)
    , size_(initial_size)
    , capacity_(0)
    , head_(nullptr)
{
  change_capacity(initial_size > initial_capacity ? initial_size: initial_capacity);
}
template <typename T, typename U>
ResizableArray<T, U>::~ResizableArray() {
  release_allocated_buffer();
}
template <typename T, typename U>
void ResizableArray<T, U>::release_allocated_buffer() {
  if (head_ != nullptr) {
    allocator_callbacks_.deallocate(head_, allocator_callbacks_.user_context);
    head_ = nullptr;
  }
  size_ = 0;
  capacity_ = 0;
  head_ = nullptr;
}
template <typename T, typename U>
void ResizableArray<T, U>::push_back(T val) {
  auto index = size_;
  size_++;
  if (index >= capacity_) {
    change_capacity(size_ * 2);
  }
  head_[index] = val;
}
template <typename T, typename U>
void ResizableArray<T, U>::change_capacity(const uint32_t new_capacity) {
  if (new_capacity < capacity_) { return; }
  const auto prev_head = head_;
  if (size_ > new_capacity) {
    size_ = new_capacity;
  }
  capacity_ = new_capacity;
  head_ = static_cast<T*>(allocator_callbacks_.allocate(sizeof(T) * new_capacity, allocator_callbacks_.user_context));
  if (prev_head != nullptr) {
    memcpy(head_, prev_head, sizeof(T) * (size_));
    allocator_callbacks_.deallocate(prev_head, allocator_callbacks_.user_context);
  }
}
} // namespace tote
