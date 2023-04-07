#include <stdint.h>
#include <string.h>
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
uint32_t Align(const uint32_t val, const uint32_t alignment) {
  const auto mask = alignment - 1;
  return (val + mask) & ~mask;
}
} // namespace tote
