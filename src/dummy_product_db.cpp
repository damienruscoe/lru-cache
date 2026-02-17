#include "dummy_product_db.hpp"

#include <random>

Product make_product(uint64_t id) {
  return Product{.id = id,
                 .category = (unsigned int)rand() %
                             4, // Only 4 categories so we get duplicates
                 .name = "",
                 .description = "",
                 .thumbnail = std::vector<uint8_t>(100000000, uint8_t{})};
}

std::ostream &operator<<(std::ostream &os, const Product &product) {
  return os << "Product{id=" << product.id << ", category=" << product.category
            << "}";
}

Product make_product() { return make_product(rand() % UINT64_MAX); }

std::optional<Product> Database::fetchProduct(uint64_t product_id) const {
  if (product_id % 2 == 0)
    return make_product(product_id);
  return std::nullopt;
}

size_t Database::fetchProductCount(uint32_t category_id) const { return 0; }

std::vector<uint64_t> get_potential_ids() {
  std::vector<uint64_t> potential_ids = {1,  3,  5,  7,  9,
                                         11, 13, 15, 17, 19}; // Invalid IDs
  for (size_t i = 0; i < 500; ++i)
    potential_ids.push_back(2 * i); // Valid IDs

  std::vector<uint64_t> chosen;
  std::sample(potential_ids.begin(), potential_ids.end(),
              std::back_inserter(chosen), 30,
              std::mt19937{std::random_device{}()});

  return chosen;
}
