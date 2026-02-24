#include "dummy_product_db.hpp"

#include <algorithm>
#include <array>
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
  if (product_id < 20)
    return make_product(product_id);
  return std::nullopt;
}

size_t Database::fetchProductCount(uint32_t category_id) const { return 0; }
