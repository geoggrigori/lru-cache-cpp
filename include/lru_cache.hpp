#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace lru {

/// A fixed-capacity Least-Recently-Used (LRU) cache.
///
/// All core operations (`put`, `get`, `contains`, `erase`) run in amortized
/// O(1) time. The implementation pairs a doubly linked list (which tracks
/// recency order, most-recently-used at the front) with an unordered map
/// (which provides O(1) lookup from a key to its node in the list).
///
/// When the cache is full and a new key is inserted, the least-recently-used
/// entry (at the back of the list) is evicted.
///
/// @tparam Key   Type used to identify entries. Must be hashable by
///               `std::hash<Key>` and equality-comparable.
/// @tparam Value Type of the stored values.
template <typename Key, typename Value>
class LRUCache {
public:
    using key_type = Key;
    using value_type = Value;
    using size_type = std::size_t;

    /// Snapshot of cumulative lookup statistics. Only `get` updates these
    /// counters; `peek` and `contains` leave them untouched.
    struct Stats {
        size_type hits = 0;    ///< Number of `get` calls that found the key.
        size_type misses = 0;  ///< Number of `get` calls that did not.
    };

    /// Constructs a cache that holds at most `capacity` entries.
    /// @throws std::invalid_argument if `capacity` is zero.
    explicit LRUCache(size_type capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("LRUCache capacity must be greater than zero");
        }
    }

    /// Inserts or updates the value for `key` and marks it most recently used.
    ///
    /// If the key already exists, its value is replaced and the size does not
    /// grow. If inserting a new key exceeds the capacity, the least recently
    /// used entry is evicted.
    void put(const Key& key, Value value) {
        auto it = index_.find(key);
        if (it != index_.end()) {
            // Existing key: update value and refresh recency.
            it->second->second = std::move(value);
            touch(it->second);
            return;
        }

        if (size() >= capacity_) {
            evict();
        }

        items_.emplace_front(key, std::move(value));
        index_.emplace(key, items_.begin());
    }

    /// Looks up `key`. On a hit, marks the entry most recently used and returns
    /// its value; on a miss, returns `std::nullopt`.
    std::optional<Value> get(const Key& key) {
        auto it = index_.find(key);
        if (it == index_.end()) {
            ++stats_.misses;
            return std::nullopt;
        }
        ++stats_.hits;
        touch(it->second);
        return it->second->second;
    }

    /// Looks up `key` without affecting recency order. On a hit, returns the
    /// stored value; on a miss, returns `std::nullopt`. Unlike `get`, `peek`
    /// does not refresh recency, so it can inspect the cache without disturbing
    /// eviction behaviour.
    std::optional<Value> peek(const Key& key) const {
        auto it = index_.find(key);
        if (it == index_.end()) {
            return std::nullopt;
        }
        return it->second->second;
    }

    /// Returns true if `key` is present, without affecting recency order.
    bool contains(const Key& key) const {
        return index_.find(key) != index_.end();
    }

    /// Removes `key` if present. Returns true if an entry was erased.
    bool erase(const Key& key) {
        auto it = index_.find(key);
        if (it == index_.end()) {
            return false;
        }
        items_.erase(it->second);
        index_.erase(it);
        return true;
    }

    /// Removes all entries.
    void clear() noexcept {
        items_.clear();
        index_.clear();
    }

    /// Number of entries currently stored.
    size_type size() const noexcept { return index_.size(); }

    /// Maximum number of entries the cache can hold.
    size_type capacity() const noexcept { return capacity_; }

    /// True if the cache holds no entries.
    bool empty() const noexcept { return index_.empty(); }

    /// Returns the cumulative hit/miss counters accumulated by `get`.
    Stats stats() const noexcept { return stats_; }

    /// Resets the hit/miss counters to zero. Does not affect cached entries.
    void reset_stats() noexcept { stats_ = Stats{}; }

private:
    using list_type = std::list<std::pair<Key, Value>>;
    using iterator = typename list_type::iterator;

    /// Moves the node at `it` to the front (most recently used) of the list.
    void touch(iterator it) {
        items_.splice(items_.begin(), items_, it);
    }

    /// Evicts the least-recently-used entry (back of the list).
    void evict() {
        const Key& victim = items_.back().first;
        index_.erase(victim);
        items_.pop_back();
    }

    size_type capacity_;
    list_type items_;                              // front = MRU, back = LRU
    std::unordered_map<Key, iterator> index_;      // key -> node in `items_`
    Stats stats_;                                  // cumulative get hit/miss counters
};

}  // namespace lru

#endif  // LRU_CACHE_HPP
