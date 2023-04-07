#include <assert.h>
#include <stdlib.h>
namespace {
struct UserContext {
  void* ptr_prev = nullptr;
  void* ptr_new = nullptr;
  uint32_t alloc_count = 0;
  uint32_t dealloc_count = 0;
};
void* Allocate(const uint32_t size, UserContext* user_context) {
  user_context->alloc_count++;
  auto ptr = malloc(size);
  user_context->ptr_prev = user_context->ptr_new;
  user_context->ptr_new = ptr;
  return ptr;
}
void Deallocate(void* ptr, UserContext* user_context) {
  assert(ptr == user_context->ptr_prev);
  user_context->ptr_prev = user_context->ptr_new;
  user_context->dealloc_count++;
  free(ptr);
}
} // namespace
