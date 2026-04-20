#pragma once

#include "ValueCache.hpp"

/**
 * @brief Thread-safe LRU cache with shared pointer semantics. To avoid
 * expensive copies of large objects into and out of the cache.
 *
 * SharedCache wraps values in std::shared_ptr<const T>, providing:
 *   - Efficient caching of large objects
 *   - Thread-safe shared ownership as shared values dont get deleted on
 * eviction.
 *
 * @tparam K Key type
 * @tparam T Value type
 * @tparam size Number of cached items
 *
 * Thread Safety:
 *   - All operations are thread-safe
 *   - Both get() and put() use exclusive locking with no concurrent access.
 *   - Cache values are wrapped in a `std::shared_ptr` to `const` data for you.
 *
 * @warning *DO NOT* to use pointer types for the value_type T. This includes
 *          value types which are shallow copied. Doing so will leave you
 *          exposed to data races if a thread decides to write to any of the
 *          memory which is shared by this indirection. Use concrete values only
 *          and `SharedCache` will safely add the pointer semantics on your
 *          behalf.
 */
template <typename K, typename T>
requires((std::is_copy_constructible_v<T> ||
          std::is_move_constructible_v<
              T>)&&!std::is_reference_v<T>) class SharedCache {
public:
  using key_t = K;
  using value_t = T;
  using ptr_t = std::shared_ptr<const value_t>;

  SharedCache(uint32_t cache_size = 64) : m_impl(cache_size) {}

  /**
   * @brief Retrieve a value from the cache if it exists. A null
   * `std::shared_ptr` otherwise.
   *
   * @param key The key to look up
   * @return `std::shared_ptr<T>` A shared pointer of the cached value
   * if found. The pointer will be `nullptr` otherwise.
   */
  ptr_t put(const key_t &key, value_t item) {
    auto shared = std::make_shared<const value_t>(std::move(item));
    m_impl.put(key, shared);
    return shared;
  }

  /**
   * @brief Insert or update a value in the cache. Stores a shared copy
   * of the value and evicts the least-recently-used item if full.
   *
   * @param key The key for the value. For later retrieval.
   * @param item The value to cache
   */
  ptr_t get(const key_t &key) { return m_impl.get(key).value_or(nullptr); }

private:
  ValueCache<key_t, ptr_t> m_impl;
};
