#pragma once

#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

struct Product {
  uint64_t id;
  uint32_t category;
  std::string name;
  std::string description;
  std::vector<uint8_t> thumbnail;
};

std::ostream &operator<<(std::ostream &os, const Product &product);

Product make_product(uint64_t id);
Product make_product();

class Database {
public:
  std::optional<Product> fetchProduct(uint64_t product_id) const;
  size_t fetchProductCount(uint32_t category_id) const;
};

// For testing purposes

template <std::size_t... Is>
consteval std::array<int, sizeof...(Is)>
make_array_impl(std::index_sequence<Is...>) {
  return {{static_cast<int>(Is)...}};
}

template <std::size_t N> consteval std::array<int, N> make_sequential_array() {
  return make_array_impl(std::make_index_sequence<N>{});
}

struct constexpr_minstd_rand {
  uint32_t state;
  using result_type = uint32_t;

  static constexpr uint32_t min() { return 1; }
  static constexpr uint32_t max() { return 2147483646; }

  constexpr uint32_t operator()() {
    // Standard minstd_rand parameters: a=48271, m=2147483647
    uint64_t next = static_cast<uint64_t>(state) * 48271;
    state = static_cast<uint32_t>(next % 2147483647);
    return state;
  }
};

consteval std::array<uint64_t, 30> get_potential_ids() {
  auto potential_ids = make_sequential_array<500>();

  /*
std::array invalid_ids = {1,  3,  5,  7,  9, 11, 13, 15, 17, 19};
std::vector<uint64_t> potential_ids = {1,  3,  5,  7,  9,
                                   11, 13, 15, 17, 19}; // Invalid IDs
for (size_t i = 0; i < 500; ++i)
potential_ids.push_back(2 * i); // Valid IDs
  */

  constexpr_minstd_rand gen{42};

  std::array<uint64_t, 30> chosen;
  /*
std::sample(potential_ids.begin(), potential_ids.end(),
        chosen.begin(), chosen.size(),
        gen);
                                                  */
  for (size_t i = 0; i < 30; ++i) {
    // Generate index in range [i, 99]
    size_t j = i + (gen() % (100 - i));

    // Swap to "pick" the element
    size_t temp = potential_ids[i];
    potential_ids[i] = potential_ids[j];
    potential_ids[j] = temp;

    chosen[i] = potential_ids[i];
  }

  return chosen;
}
