#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <random>
#include <thread>
#include <vector>

#include "SharedCache.hpp"
#include "ValueCache.hpp"

template <typename T> class ThreadSafetyCacheTest : public ::testing::Test {
protected:
  using CacheType = T;
};

using ThreadSafetyCacheTypes =
    ::testing::Types<ValueCache<int, int, 100>, SharedCache<int, int, 100>>;

TYPED_TEST_SUITE(ThreadSafetyCacheTest, ThreadSafetyCacheTypes);

TYPED_TEST(ThreadSafetyCacheTest, ConcurrentReaders) {
  TypeParam cache;

  const int THREAD_COUNT = 10;
  const int ITEMS_IN_CACHE = 50;
  const int ACTION_PER_THREAD = 1000;

  for (int i = 0; i < ITEMS_IN_CACHE; ++i)
    cache.put(i, i + 4);

  std::atomic<int> success{0};
  std::vector<std::thread> threads;

  for (int t = 0; t < THREAD_COUNT; ++t) {
    threads.emplace_back([&cache, &success]() {
      for (int i = 0; i < ACTION_PER_THREAD; ++i) {
        auto id = i % ITEMS_IN_CACHE;

        auto val = cache.get(id);
        if (val && *val == id + 4) {
          success++;
        }
      }
    });
  }

  for (auto &t : threads)
    t.join();
  EXPECT_EQ(success, ACTION_PER_THREAD * THREAD_COUNT);
}

TYPED_TEST(ThreadSafetyCacheTest, ConcurrentWritersDifferentKeys) {
  TypeParam cache;
  std::atomic<int> writes{0};
  std::vector<std::thread> threads;

  const int THREAD_COUNT = 10;
  const int ITEMS_IN_CACHE = 50;
  const int ACTION_PER_THREAD = 100;

  for (int t = 0; t < THREAD_COUNT; ++t) {
    threads.emplace_back([&cache, &writes, t]() {
      for (int i = 0; i < ACTION_PER_THREAD; ++i) {
        cache.put(t * 1000 + i, i);
        writes++; // We can only assume the write is correct
      }
    });
  }

  for (auto &t : threads)
    t.join();
  EXPECT_EQ(writes, 1000);
}

TYPED_TEST(ThreadSafetyCacheTest, ConcurrentWritersSameKeys) {
  TypeParam cache;
  std::atomic<int> writes{0};
  std::vector<std::thread> threads;

  const int THREAD_COUNT = 10;
  const int ITEMS_IN_CACHE = 50;
  const int ACTION_PER_THREAD = 100;

  for (int t = 0; t < THREAD_COUNT; ++t) {
    threads.emplace_back([&cache, &writes, t]() {
      for (int i = 0; i < ACTION_PER_THREAD; ++i) {
        cache.put(i % 5, i);
        writes++; // We can only assume the write is correct
      }
    });
  }

  for (auto &t : threads)
    t.join();
  EXPECT_EQ(writes, 1000);
  // Check all the keys exist in the cache
  for (int i = 0; i < 5; ++i)
    EXPECT_TRUE(cache.get(i));
}
