#pragma once

#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

/**
 * @brief Thread-safe LRU cache storing values by copy.
 *
 * ValueCache stores copies of values and returns copies on retrieval. This
 * ensures thread safety through value semantics. Each thread receives
 * independent data.
 *
 * @tparam K Key type
 * @tparam T Value type
 * @tparam size Number of cached items
 *
 * Thread Safety:
 *   - All operations are thread-safe
 *   - Both get() and put() use exclusive locking with no concurrent access.
 *   - Cache values are returned by-value, giving each caller independent data
 *
 * @warning *DO NOT* to use pointer types for the value_type T. This includes
 *          value types which are shallow copied. Doing so will leave you
 *          exposed to data races if a thread decides to write to any of the
 *          memory which is shared by this indirection. Prefer `SharedCache`
 *          if value types are expensive to copy.
 */
template <typename K, typename T, uint32_t size = 64>
requires(std::is_copy_constructible_v<T> &&
         !std::is_reference_v<T>) class ValueCache {
public:
  using key_t = K;
  using value_t = T;

  /**
   * @brief Retrieve a value from the cache if it exists. `std::nullopt`
   * otherwise.
   *
   * @param key The key to look up
   * @return `std::optional<T>` Copy of the cached value if found,
   * `std::nullopt` otherwise
   *
   * This returns a COPY of the cached value. If T is a pointer type, the
   * pointer is copied (not the pointed-to data), defeating thread safety.
   */
  std::optional<value_t> get(const key_t &key) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    const auto &found = m_key_map.find(key);
    return found == m_key_map.end() ? std::nullopt
                                    : retrieve_item(found->second);
  }

  /**
   * @brief Insert or update a value in the cache. Stores a copy of the value
   * and evicts the least-recently-used item if full.
   *
   * @param key The key for the value. For later retrieval.
   * @param item The value to cache
   */
  void put(const key_t &key, value_t item) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    const auto &found = m_key_map.find(key);
    found == m_key_map.end() ? insert_item(key, std::move(item))
                             : set_item(found->second, std::move(item));
  }

private:
  using cached_items_t = std::list<std::pair<key_t, value_t>>;
  using cached_item_p = cached_items_t::iterator;
  using key_lookup_t = std::unordered_map<key_t, cached_item_p>;

  inline void evict() {
    m_key_map.erase(m_cached_items.back().first);
    m_cached_items.pop_back();
  }

  inline void promote_to_MRU(cached_item_p it) {
    m_cached_items.splice(m_cached_items.begin(), m_cached_items, it);
  }

  /**
   * @brief Set the item value and reorder list item to the MRU position
   */
  void set_item(cached_item_p it, value_t &&item) {
    it->second = std::move(item);
    promote_to_MRU(it);
  }

  /**
   * @brief Evict if necessary to ensure space is available in the cache
   * and then insert item into the MRU position
   */
  void insert_item(const key_t &key, value_t &&item) {
    if (m_cached_items.size() >= size)
      evict();

    m_cached_items.emplace_front(key, std::move(item));
    m_key_map.emplace(key, m_cached_items.begin());
  }

  /**
   * @brief Move item into the MRU position and return its value
   */
  std::optional<value_t> retrieve_item(cached_item_p it) {
    promote_to_MRU(it);
    return it->second;
  }

  key_lookup_t m_key_map;
  cached_items_t m_cached_items;
  std::shared_mutex m_mutex;
};
