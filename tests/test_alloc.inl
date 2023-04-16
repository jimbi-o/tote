#include <assert.h>
#include <stdlib.h>
#include <unordered_set>
namespace {
struct UserContext {
  uint32_t alloc_count = 0;
  uint32_t dealloc_count = 0;
  std::unordered_set<void*> ptr{};
};
void* Allocate(const uint32_t size, const uint32_t alignment, UserContext* user_context) {
  user_context->alloc_count++;
#ifdef _MSC_VER
  auto ptr = _aligned_malloc(size, alignment);
#else
  auto ptr = aligned_alloc(alignment, size);
#endif
  assert(!user_context->ptr.contains(ptr));
  user_context->ptr.insert(ptr);
  return ptr;
}
void Deallocate(void* ptr, UserContext* user_context) {
  assert(user_context->ptr.contains(ptr));
  user_context->ptr.erase(ptr);
  user_context->dealloc_count++;
#ifdef _MSC_VER
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}
} // namespace
