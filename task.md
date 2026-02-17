# C++ assignment

Clients visit our e-commerce website to browse products we have for sale. For each product we
store the following details:

- uint64 id
- uint32 category
- string name
- string description
- bytes thumbnail image

Our products are stored in a database through a class which has the following interface. Both methods may return the requested data if the id exists, or indication of error when the id does not exist or database is unavailable.

1. A method to fetch product details given a product id
2. A method to fetch total number of products given a category id

Our profiling shows that fetching the product details (1) is an expensive operation. Furthermore, we observed that clients tend to focus on the same products on a particular day or keep browsing back and forth between the same products. For this reason, it was proposed by a colleague to create a cache that will hold the frequently browsed product details.

In this exercise you need to:
- Create an in-memory cache with LRU (least-recently-used) replacement policy and a predetermined capacity in terms of item count.
- The cache can hold only up to a maximum number of items, so when the cache is full and a new item needs to be cached, the least-recently-used item must be evicted, i.e.  the one that has been unused for a longer period of time than the others.
- The cache component needs to expose:
    - a retrieval method that requests a cached item, which may or not exist
    - an insertion method that caches an item
- The cache implementation should be generic so that it can be reused for other use cases in the future with possible different data type for key or value. e.g. to cache list of product ids per category
- Using the LRU cache implementation, create a class that routes the fetch product details queries either to the cache or to the database
- Assume that the fetch product details queries will be done from multiple threads
- The actual database can be faked with hardcoded data as we are not focused on this part and the database component is thread-safe by itself.
- Provide a simple driver application that demonstrates their use
- Do not use an AI generated solution.  The purpose of the exercise is to give you an opportunity to show us your engineering skills.
