#pragma once
#include <stdint.h>
#include <string.h>
#include <utility>
namespace tote {
struct AllocatorCallbacks {
  using AllocateFunction = void*(const uint32_t size);
  using DeallocateFunction = void(void*);
  AllocateFunction*   allocate;
  DeallocateFunction* deallocate;
};
template <typename T>
class ResizableArray final {
 public:
  ResizableArray(AllocatorCallbacks allocator_callbacks, const uint32_t initial_size, const uint32_t initial_capacity);
  ~ResizableArray();
  constexpr uint32_t size() const { return size_; }
  constexpr uint32_t capacity() const { return capacity_; }
  constexpr bool empty() const { return size() == 0; }
  /**
   * reset size to zero.
   * destructor for T is no called.
   **/
  void clear() { size_ = 0; }
  /**
   * release allocated buffer.
   * destructor for T is no called.
   **/
  void release_allocated_buffer();
  void push_back(T);
  void emplace_back(T&&);
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
  AllocatorCallbacks allocator_callbacks_;
  uint32_t size_;
  uint32_t capacity_;
  T* head_;
};
template <typename T>
ResizableArray<T>::ResizableArray(AllocatorCallbacks allocator_callbacks, const uint32_t initial_size, const uint32_t initial_capacity)
    : allocator_callbacks_(allocator_callbacks)
    , size_(initial_size)
    , capacity_(initial_capacity)
    , head_(nullptr)
{
  if (size_ > capacity_) {
    capacity_ = size_;
  }
  change_capacity(capacity_);
}
template <typename T>
ResizableArray<T>::~ResizableArray() {
  release_allocated_buffer();
}
template <typename T>
void ResizableArray<T>::release_allocated_buffer() {
  if (head_ != nullptr) {
    allocator_callbacks_.deallocate(head_);
  }
  size_ = 0;
  capacity_ = 0;
  head_ = nullptr;
}
template <typename T>
void ResizableArray<T>::push_back(T val) {
  auto index = size_;
  size_++;
  if (index >= capacity_) {
    change_capacity(size_);
  }
  head_[index] = val;
}
template <typename T>
void ResizableArray<T>::emplace_back(T&& val) {
  auto index = size_;
  size_++;
  if (index >= capacity_) {
    change_capacity(size_);
  }
  head_[index] = std::move(val);
}
template <typename T>
void ResizableArray<T>::change_capacity(const uint32_t new_capacity) {
  auto prev_head = head_;
  if (size_ > new_capacity) {
    size_ = new_capacity;
  }
  capacity_ = new_capacity;
  head_ = static_cast<T*>(allocator_callbacks_.allocate(sizeof(T) * new_capacity));
  if (prev_head != nullptr) {
    memcpy(head_, prev_head, sizeof(T) * (size_));
    allocator_callbacks_.deallocate(prev_head);
  }
}
} // namespace tote
