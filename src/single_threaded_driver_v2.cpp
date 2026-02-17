#include <cassert>
#include <iostream>

#include "CachedDatabaseExample.hpp"
#include "SharedCache.hpp"
#include "dummy_product_db.hpp"

int main() {
  Database db;
  CachedDatabaseExample cached_db(db);

  const auto potential_ids = get_potential_ids();

  for (size_t i = 0; i < 20; ++i) {
    const auto &id = potential_ids[rand() % potential_ids.size()];
    check_database_lookup(cached_db, id, std::to_string(id));
  }
}
