#pragma once
namespace tote{
template <typename T>
struct AllocatorCallbacks {
  using AllocateFunction = void*(const uint32_t size, const uint32_t alignment, T* user_context);
  using DeallocateFunction = void(void*, T* user_context);
  AllocateFunction*   allocate;
  DeallocateFunction* deallocate;
  T* user_context;
};
}
