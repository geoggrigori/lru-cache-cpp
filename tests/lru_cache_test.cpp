#include "lru_cache.hpp"
#include "simple_test.hpp"

#include <optional>
#include <string>

using lru::LRUCache;

TEST(put_and_get_basics) {
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");
    cache.put(2, "two");

    CHECK_EQ(cache.size(), static_cast<std::size_t>(2));
    CHECK_EQ(cache.capacity(), static_cast<std::size_t>(2));

    auto a = cache.get(1);
    auto b = cache.get(2);
    CHECK(a.has_value());
    CHECK(b.has_value());
    CHECK_EQ(*a, std::string("one"));
    CHECK_EQ(*b, std::string("two"));
}

TEST(miss_returns_nullopt) {
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");

    auto missing = cache.get(42);
    CHECK(!missing.has_value());
    CHECK(missing == std::nullopt);
    CHECK(!cache.contains(42));
    CHECK(cache.contains(1));
}

TEST(eviction_of_lru_on_overflow) {
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");  // exceeds capacity -> evicts key 1 (LRU)

    CHECK_EQ(cache.size(), static_cast<std::size_t>(2));
    CHECK(!cache.contains(1));
    CHECK(cache.contains(2));
    CHECK(cache.contains(3));
    CHECK(!cache.get(1).has_value());
}

TEST(get_refreshes_recency) {
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");
    cache.put(2, "two");

    // Access key 1 so it becomes most-recently-used; key 2 is now LRU.
    auto a = cache.get(1);
    CHECK(a.has_value());

    cache.put(3, "three");  // should evict key 2, not key 1

    CHECK(cache.contains(1));
    CHECK(!cache.contains(2));
    CHECK(cache.contains(3));
}

TEST(update_existing_key_no_growth_and_refreshes_recency) {
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");
    cache.put(2, "two");

    // Re-insert key 1: value updated, size unchanged, key 1 becomes MRU.
    cache.put(1, "ONE");
    CHECK_EQ(cache.size(), static_cast<std::size_t>(2));
    CHECK_EQ(*cache.get(1), std::string("ONE"));

    // After the update, key 2 is the LRU and should be evicted next.
    cache.put(3, "three");
    CHECK(cache.contains(1));
    CHECK(!cache.contains(2));
    CHECK(cache.contains(3));
}

TEST(erase_removes_entry) {
    LRUCache<int, std::string> cache(3);
    cache.put(1, "one");
    cache.put(2, "two");

    CHECK(cache.erase(1));
    CHECK(!cache.contains(1));
    CHECK_EQ(cache.size(), static_cast<std::size_t>(1));

    // Erasing a missing key is a no-op returning false.
    CHECK(!cache.erase(99));
    CHECK_EQ(cache.size(), static_cast<std::size_t>(1));
}

TEST(clear_empties_cache) {
    LRUCache<int, std::string> cache(3);
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    cache.clear();
    CHECK_EQ(cache.size(), static_cast<std::size_t>(0));
    CHECK(cache.empty());
    CHECK(!cache.contains(1));
    CHECK(!cache.contains(2));
    CHECK(!cache.contains(3));
}

TEST(capacity_boundary) {
    LRUCache<int, int> cache(1);
    cache.put(1, 100);
    CHECK_EQ(cache.size(), static_cast<std::size_t>(1));
    CHECK_EQ(*cache.get(1), 100);

    cache.put(2, 200);  // capacity 1 -> evicts key 1
    CHECK_EQ(cache.size(), static_cast<std::size_t>(1));
    CHECK(!cache.contains(1));
    CHECK_EQ(*cache.get(2), 200);
}

SIMPLE_TEST_MAIN()
