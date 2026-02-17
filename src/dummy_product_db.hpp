#pragma once

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
std::vector<uint64_t> get_potential_ids();
