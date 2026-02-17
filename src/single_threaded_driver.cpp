#include <cassert>
#include <iostream>

#include "CachedDatabaseExample.hpp"
#include "SharedCache.hpp"
#include "dummy_product_db.hpp"

void run_manual_single_threaded_example() {
  Database db;
  CachedDatabaseExample cached_db(db);

  check_database_lookup(
      cached_db, 1230,
      "Product 1230 should not be in the cache yet. Search from database");
  check_database_lookup(cached_db, 1230,
                        "Retry product 1230; Should be in the cache");

  check_database_lookup(
      cached_db, 2340,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");
  check_database_lookup(
      cached_db, 3450,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");
  check_database_lookup(
      cached_db, 4560,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");
  check_database_lookup(
      cached_db, 123,
      "Not found in the database hence nor the cache. Dont add to the cache. "
      "No-op.");
  check_database_lookup(
      cached_db, 5670,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");
  check_database_lookup(
      cached_db, 7890,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");
  check_database_lookup(
      cached_db, 123,
      "Not found in the database hence nor the cache. Dont add to the cache. "
      "No-op.");
  check_database_lookup(
      cached_db, 8900,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");
  check_database_lookup(
      cached_db, 120,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");
  check_database_lookup(
      cached_db, 990,
      "This product should be missing from the cache, found in the DB and "
      "added to the cache");

  check_database_lookup(
      cached_db, 1230,
      "Retry product 1230; Should have been evicted from the cache.");
}

int main() { run_manual_single_threaded_example(); }
