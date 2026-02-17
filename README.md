# Thread-Safe LRU Cache

A generic, thread-safe LRU cache implementation in C++20, providing two complementary interfaces for different use cases.

Coincidentally, I have done a bit of work on caches in the open source project Kodi. Please feel free to checkout that work:

[My Kodi _cache_ contributions](https://github.com/damienruscoe/xbmc/branches/all?query=cache&lastTab=overview)

[All My Kodi contributions](https://github.com/damienruscoe/xbmc/branches/yours)

## Quick Start

- **include/** contains the implementations of the two cache classes.
- **src/** contains the driver programs and dummy database classes. This contains mostly noddy code to serve as example usage.
- **tests/** contains the test code for the implementations.

### Running Tests

The test suite is written in Google Test.

```bash
make test_all
```

### Running the Example Driver

Emulates a fake database with an associated cache with multiple threads.

```bash
make driver
```

## Design Overview

Both cache implementations use the same internal structure:

- Doubly-linked list
    - Stores the cache values
    - `std::list` implementation
    - O(1) LRU ordering and eviction
- Hash map
    - Stores key -> list index
    - `std::unordered_map` implementation
    - O(1) key lookups

### Threading Model

Thread safety is implemented using `std::shared_mutex` with exclusive locks (`std::unique_lock`). Both `get()` and `put()` require write access because:
- Obviously `put()` modifies the cache contents so an exclusive lock is required.
- `get()` modifies the LRU ordering by promoting accessed items to the front

This means operations are serialized, which I consider acceptable as cache operations are orders of magnitude faster than the expensive operations they're
caching (like database queries). I would be open to investigating more nuanced thread synchronisation but only if profiling shows lock contention is an issue.

## Implementations

The implementations are `ValueCache` and `SharedCache`

Both cache implementations provide thread safety by returning copies of cached values, ensuring each thread receives independent data.
This works because value types like integers or strings, copying creates truly separate objects. Storing pointers breaks this model
because copying a pointer only copies the address, not the pointed-to data. Multiple threads can end up sharing access to the same
underlying object with no synchronization, creating data races.

The `SharedCache` returns copies of `std::shared_ptr` to const data. Allowing for effecient retreival from the cache by avoiding copies
of large value objects. Allowing only `const` access to the data prevents modification and avoids data races.

The cache's locks protect its internal structure but cannot protect memory accessed through pointers you've stored. ValueCache should
only hold concrete value types that copy deeply. For large objects where copying is expensive, use SharedCache - it wraps values in
shared pointers and enforces immutability through const-correctness, providing both efficient sharing and thread safety.

### ValueCache

Returning a pointer or reference would be unsafe in a multithreaded context when another thread could evict the item, invalidating the
reference. Returning copies ensures each thread has its own copy of the value.

```cpp
ValueCache<uint64_t, int, 100> cache;
cache.put(42, 1337);
auto value = cache.get(42);  // Returns std::optional<int>
if (value) {
    // Use *value
}
```

- Returns `std::optional<T>` from `get()`
- Stores and returns copies of values
- Thread-safe: each thread gets its own copy
- Compile-time rejection of references
- **Don't use with pointers** (raw or smart) - defeats thread safety. Use `SharedCache` instead.

**Required**
- Types that are copy-constructible

**When to use:**
- Small values that are cheap to copy

### SharedCache

`SharedCache` is implemented in terms of `ValueCache`, wrapping values in `shared_ptr` and unwrapping nulls. This means `ValueCache` stores
`shared_ptr` and returns `std::optional<shared_ptr>`, which `SharedCache` converts to just `shared_ptr` (using `nullptr` for misses).

`SharedCache` enforces modification cannot happen to values in the cache. The `shared_ptr<const T>` return type allows sharing of data and
stops callers from modifying cached data, which would break cache coherency and create race conditions.

```cpp
SharedCache<uint64_t, Product, 100> cache;
cache.put(42, Product{...});
auto ptr = cache.get(42);  // Returns std::shared_ptr<const Product>
if (ptr) {
    // Use ptr->field
}
```

- Wraps values in `std::shared_ptr<const T>`
- Returns `std::shared_ptr<const T>` from `get()`
- Thread-safe: shared pointers manage lifetime
    - The `const` pointee prevents mutation of values in the cache.
- Threads can maintain a valid value even when the value has been evicted from the cache.
- Compile-time rejection of references
- __Don't use with pointers__, just let `SharedCache` manage the `shared_ptr` for you

**Required**
- Copy-constructable or Move-constructable types

**When to use:**
- Large objects expensive to copy (images, large strings, complex structures)
- When you want shared ownership semantics

## Usage Examples

### Basic ValueCache
```cpp
ValueCache<std::string, int, 10> scores;
scores.put("alice", 100);
scores.put("bob", 200);

if (auto score = scores.get("alice")) {
    std::cout << "Alice's score: " << *score << '\n';
}
```

### Product Cache with SharedCache
```cpp
struct Product {
    uint64_t id;
    std::string name;
    std::vector<uint8_t> thumbnail;
};

SharedCache<uint64_t, Product, 1000> product_cache;
product_cache.put(12, Product{.id=12, .name="AAPL", .thumbnail={0,0,0,0,0});
product_cache.put(94, Product{.id=94, .name="NVDA", .thumbnail={0,0,0,0,0});

if (auto product = product_cache.get(12)) {
    std::cout << product->name << '\n';
    // Product stays valid even if evicted from cache
}
```

## Potential Future Direction

The implementation of `ValueCache::insert_item` may evict an item from the cache, which will deallocate a list node. The following insertion
will then allocate a new list node. In this case we can reuse the already existing node preventing a pair of allocation/deallocation.

Current threaded tests are only providing minimal test coverage. These could be expanded much further to provide a more comprehensive test suite.

Potentially have the mutex and locks as compile time parameters which could improve performance as a general purpose LRU cache in non-
threaded environments.

In `SharedCache`, we could eliminate the `std::optional` wrapper for a small performance gain, but it would require invasive implementation
between the two current implementations. The current design favors clear sepereration over small performance improvements.

The current design is simple and easy to maintain. I would prefer to optimize based on measurements, not speculation. That being said, I
have noted some areas of improvement and/or tweaks which could be made for specific use-cases:

The spec gave the limitation that the cache should be a fixed size. I embedded the cache size into a template parameter thining this could
offer the compiler better optimization possibilities. As the design fleshed out I discovered that big wins for compile time sizes were not
utilized (thinking `std::array`). The size parameter could be possibly be made a runtime value; this could help reduce binary size if various
sized caches are used.

Maintaining a strict LRU mechanism is probably not stricly necessary in a multithreaded system. If this consdition was loosened there is
possibly an implementation which could allow concurrent reads to the cache. This would need to be benchmarked in real-world applications
compared for benefits.

Either a custom allocator or a custom intrusive linked lists could reduce allocations by pre-allocating a pool of nodes.
  - This will reduce latency and jitters in the cache perfromance profile.
  - This may empose stronger conditions such as a default-constructable value type as well as increase memory consumption when the cache is empty.

Indirect types are not permitted to be inserted into the cache as this will lead to shared, unsynchronised data in a multithreaded environment.
Usage of raw pointers and `std::shared_ptr` can be detected and prevented at compile time using constraints but this is not the case for user 
defined types containing indirect types. In order to not give a false sense of security the implementation does not try to constrain pointer types
from being inserted. C++26 reflection could introspect all value types checking for aggregate indirect value types to provide a robust constraint
guarding against this at compile time.

