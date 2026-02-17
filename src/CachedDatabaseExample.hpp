#pragma once

#include <cassert>
#include <iostream>
#include <thread>

#include "SharedCache.hpp"
#include "dummy_product_db.hpp"

std::ostream &nl(std::ostream &os) { return os << '\n'; }

class CachedDatabaseExample {
public:
  CachedDatabaseExample(Database db) : m_database(std::move(db)) {}

  std::shared_ptr<const Product> fetchProduct(uint64_t id) {
    std::cout << "[" << std::this_thread::get_id() << "] Fetching: " << id
              << nl;

    if (auto cached = m_cache.get(id)) {
      std::cout << "[" << std::this_thread::get_id()
                << "] Cache Hit: " << *cached << nl;
      return cached;
    }

    std::cout << "[" << std::this_thread::get_id() << "] Cache Miss: " << id
              << nl;

    auto product = m_database.fetchProduct(id);
    if (product) {
      std::cout << "[" << std::this_thread::get_id()
                << "] Loaded from Database (Adding to cache): " << *product
                << nl;
      return m_cache.put(id, *product);
    } else {
      std::cout << "[" << std::this_thread::get_id()
                << "] Database lookup failure: " << id << nl;
      return {};
    }
  }

  size_t fetchProductCount(uint32_t category_id) const {
    return m_database.fetchProductCount(category_id);
  }

private:
  Database m_database;
  SharedCache<uint64_t, Product, 5> m_cache;
};

void check_database_lookup(CachedDatabaseExample &cached_db, uint64_t id,
                           std::string message) {
  std::cout << '\n' << message << '\n';
  auto product = cached_db.fetchProduct(id);
  if (!product) {
    std::cout << "Result: <No Product>\n";
  } else {
    std::cout << "Result: " << *product << '\n';
    assert(product->id == id && "Incorrect ID retreived\n");
  }
}
