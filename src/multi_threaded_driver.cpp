#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

#include "CachedDatabaseExample.hpp"
#include "SharedCache.hpp"
#include "dummy_product_db.hpp"

void run_thread(CachedDatabaseExample &cached_db,
                const std::vector<uint64_t> &potential_ids) {
  std::cout << "Thread ID: " << std::this_thread::get_id() << " started" << nl;

  for (size_t i = 0; i < 20; ++i) {
    const auto &id = potential_ids[rand() % potential_ids.size()];
    check_database_lookup(cached_db, id, std::to_string(id));
  }
}

void run_multi_threaded_example(size_t thread_count) {
  Database db;
  CachedDatabaseExample cached_db(db);

  const auto potential_ids = get_potential_ids();

  std::vector<std::jthread> threads;
  for (size_t i = 0; i < thread_count; ++i) {
    threads.emplace_back(
        std::jthread([&]() { run_thread(cached_db, potential_ids); }));
  }
}

int main() { run_multi_threaded_example(10); }
