#include <exception>
#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include "SharedCache.hpp"
#include "ValueCache.hpp"

template <typename cache_t>
void expectCacheNoKey(cache_t &cache, const typename cache_t::key_t &key) {
  auto cached = cache.get(key);
  ASSERT_FALSE(cached);
}

template <typename cache_t>
void expectCacheKeyValue(cache_t &cache, const typename cache_t::key_t &key,
                         const typename cache_t::value_t &value) {
  auto cached = cache.get(key);
  ASSERT_TRUE(cached);
  EXPECT_EQ(*cached, value);
}

template <typename cache_t, typename Value>
void expectCacheKeyValue_DoubleDeref(cache_t &cache,
                                     const typename cache_t::key_t &key,
                                     const Value &value) {
  auto cached = cache.get(key);
  ASSERT_TRUE(cached);
  EXPECT_EQ(**cached, value);
}

template <typename T> class TestCache : public ::testing::Test {
protected:
  using CacheType = T;
};

using IntCacheTypes = ::testing::Types<ValueCache<uint64_t, int, 3>,
                                       SharedCache<uint64_t, int, 3>>;

TYPED_TEST_SUITE(TestCache, IntCacheTypes);

TYPED_TEST(TestCache, EmptyCache) {
  TypeParam cache;

  expectCacheNoKey(cache, 0);
  expectCacheNoKey(cache, 1);
}

TEST(TestCache, CacheTypes) {
  ValueCache<int, std::string, 3> cache;
  cache.put(123, "hello");
  cache.put(456, "world");

  expectCacheKeyValue(cache, 123, "hello");
  expectCacheKeyValue(cache, 456, "world");
}

TEST(TestCache, SingleItemCapacity) {
  ValueCache<uint64_t, int, 1> cache;
  cache.put(0, 100);
  expectCacheKeyValue(cache, 0, 100);

  // Every insert now evicts the existing cached item
  cache.put(1, 200);
  expectCacheNoKey(cache, 0);
  expectCacheKeyValue(cache, 1, 200);

  cache.put(2, 300);
  expectCacheNoKey(cache, 0);
  expectCacheNoKey(cache, 1);
  expectCacheKeyValue(cache, 2, 300);
}

TYPED_TEST(TestCache, BasicFeatures) {
  TypeParam cache;
  cache.put(0, 0);

  expectCacheKeyValue(cache, 0, 0);
  expectCacheNoKey(cache, 1);

  cache.put(1, 1);
  cache.put(2, 2);

  expectCacheKeyValue(cache, 0, 0);
  expectCacheKeyValue(cache, 1, 1);
  expectCacheKeyValue(cache, 2, 2);
  expectCacheNoKey(cache, 3);

  // Capacity of the cache is 3; Inserting a 4th should evict the LRU
  cache.put(3, 3);

  expectCacheNoKey(cache, 0);
  expectCacheKeyValue(cache, 1, 1);
  expectCacheKeyValue(cache, 2, 2);
  expectCacheKeyValue(cache, 3, 3);

  // Looking at key{1} should make this the MRU; thus making key{2} the LRU.
  // Capacity of the cache is 3; Inserting a 4th should evict the LRU
  cache.get(1);
  cache.put(4, 4);

  expectCacheNoKey(cache, 0);
  expectCacheKeyValue(cache, 1, 1);
  expectCacheNoKey(cache, 2);
  expectCacheKeyValue(cache, 3, 3);
  expectCacheKeyValue(cache, 4, 4);
}

TYPED_TEST(TestCache, FullCacheEvictionPattern) {
  TypeParam cache;

  // Fill cache
  cache.put(0, 0);
  cache.put(1, 1);
  cache.put(2, 2);

  // Multiple evictions in sequence
  cache.put(3, 3); // evicts 0
  cache.put(4, 4); // evicts 1
  cache.put(5, 5); // evicts 2

  expectCacheNoKey(cache, 0);
  expectCacheNoKey(cache, 1);
  expectCacheNoKey(cache, 2);
  expectCacheKeyValue(cache, 3, 3);
  expectCacheKeyValue(cache, 4, 4);
  expectCacheKeyValue(cache, 5, 5);
}

TYPED_TEST(TestCache, OverridePreviousKeyWithNewValue_OverideLRU) {
  TypeParam cache;
  cache.put(0, 0);
  cache.put(1, 1);

  expectCacheKeyValue(cache, 0, 0);
  expectCacheKeyValue(cache, 1, 1);

  cache.put(0, 9001);

  expectCacheKeyValue(cache, 0, 9001);
  expectCacheKeyValue(cache, 1, 1);

  cache.put(1, 9002);

  expectCacheKeyValue(cache, 0, 9001);
  expectCacheKeyValue(cache, 1, 9002);
}

TYPED_TEST(TestCache, OverridePreviousKeyWithNewValue_OverideMRU) {
  TypeParam cache;
  cache.put(0, 0);
  cache.put(1, 1);

  expectCacheKeyValue(cache, 0, 0);
  expectCacheKeyValue(cache, 1, 1);

  cache.put(1, 9001);

  expectCacheKeyValue(cache, 0, 0);
  expectCacheKeyValue(cache, 1, 9001);

  cache.put(0, 9002);

  expectCacheKeyValue(cache, 0, 9002);
  expectCacheKeyValue(cache, 1, 9001);
}

TYPED_TEST(TestCache, GettingACacheItemIncreasesItsMRU) {
  TypeParam cache;
  cache.put(0, 0);
  cache.put(1, 1);
  cache.put(2, 2); // LRU{0, 1, 2}

  // Looking at key{0} should make this the MRU; thus making key{1} the LRU.
  // Capacity of the cache is 3; Inserting a 4th should evict the LRU
  cache.get(0);    // LRU{1, 2, 0}
  cache.put(3, 3); // LRU{2, 0, 3}

  expectCacheKeyValue(cache, 0, 0);
  expectCacheNoKey(cache, 1);
  expectCacheKeyValue(cache, 2, 2);
  expectCacheKeyValue(cache, 3, 3);
}

TYPED_TEST(TestCache, GettingACacheItemIncreasesItsMRU_v2) {
  TypeParam cache;
  cache.put(0, 0);
  cache.put(1, 1);
  cache.put(2, 2); // LRU{0, 1, 2}

  // Looking at key{1} should make this the MRU
  // Then, Looking at key{0} should make this the MRU; thus making key{2} the
  // LRU. Capacity of the cache is 3; Inserting a 4th should evict the LRU
  cache.get(1);    // LRU{0, 2, 1}
  cache.get(0);    // LRU{2, 1, 0}
  cache.put(3, 3); // LRU{1, 0, 3}

  expectCacheKeyValue(cache, 0, 0);
  expectCacheKeyValue(cache, 1, 1);
  expectCacheNoKey(cache, 2);
  expectCacheKeyValue(cache, 3, 3);
}

TEST(TestCache, LargeValueType) {
  struct LargeObject {
    int id;
    std::array<char, 64 * 1024> data;
    bool operator==(const LargeObject &other) const { return id == other.id; }
  };

  ValueCache<int, LargeObject, 3> cache;
  LargeObject obj{42, {}};
  cache.put(1, std::move(obj));

  expectCacheKeyValue(cache, 1, LargeObject{42, {}});
}

TEST(TestCache, MoveOnlyTypeValues) {
  /* Move-only types cannot work. Passing a move only type into the cache will
   * not allow access to this cached object as the access will need to remove
   * the item from the cache. Constrained Cache from being instantiated with
   * non-copy-constructable types as values.
   */

  // Compilation error
  // ValueCache<int, std::unique_ptr<int>, 2> cache;

  /* Shared pointer of std::unique_ptrs are perfectly valid as the unique_ptr is
   * not being copied.
   */
  SharedCache<int, std::unique_ptr<int>, 3> cache;
  cache.put(1, std::make_unique<int>(100));

  expectCacheKeyValue_DoubleDeref(cache, 1, 100);
  expectCacheNoKey(cache, 2);

  cache.put(2, std::make_unique<int>(200));
  cache.put(3, std::make_unique<int>(300));
  cache.put(4, std::make_unique<int>(400));

  expectCacheNoKey(cache, 1);
  expectCacheKeyValue_DoubleDeref(cache, 2, 200);
  expectCacheKeyValue_DoubleDeref(cache, 3, 300);
  expectCacheKeyValue_DoubleDeref(cache, 4, 400);
}

TEST(TestCache, ReferenceTypeValues) {
  /* References just do not make sense as values. Explicitly constrained.
   */

  // ValueCache<int, int&, 2> cache;

  /* Shared pointer references just do not make sense as values.
   */

  // SharedCache<int, int&, 3> cache;
}

TEST(TestCache, SharedPtrSurvivesEviction) {
  struct Data {
    int value;
  };

  ValueCache<int, std::shared_ptr<const Data>, 2> cache;

  auto data1 = std::make_shared<Data>(Data{100});
  auto data2 = std::make_shared<Data>(Data{200});
  auto data3 = std::make_shared<Data>(Data{300});

  cache.put(1, data1);
  cache.put(2, data2);

  auto cached_value = *cache.get(1);
  ASSERT_TRUE(cached_value);

  // Move key{2} to the MRU; Moving key{1} to LRU
  cache.get(2);

  // Fill cache, evicting key 1
  cache.put(3, data3);
  expectCacheNoKey(cache, 1);

  // Original shared_ptr and held reference should still be valid!
  EXPECT_EQ(data1->value, 100);
  EXPECT_EQ(cached_value->value, 100);
  EXPECT_EQ(data1.use_count(), 2); // data1 + cached_value

  /* TODO: This could indicate a bad interface. The data in the cache could be
   * modified after it has been inserted into the cache?
   */
  data1->value = 999;

  EXPECT_EQ(data1->value, 999);
  EXPECT_EQ(cached_value->value, 999);

  /*
cached_value->value = 999;

EXPECT_EQ(data1->value, 999);
EXPECT_EQ(cached_value->value, 999);
  */
}

/*
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/
