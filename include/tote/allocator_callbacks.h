#pragma once
struct AllocatorCallbacks {
  using AllocateFunction = void*(const uint32_t size, void* user_context);
  using DeallocateFunction = void(void*, void* user_context);
  AllocateFunction*   allocate;
  DeallocateFunction* deallocate;
  void* user_context;
};
