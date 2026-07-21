#pragma once

#include <chrono>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

struct Entry {
  std::string value;
  long long expiryTime;
};

class DataStore {
 private:
  std::unordered_map<std::string, Entry> cache;
  mutable std::shared_mutex mutex;

  long long getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
               now.time_since_epoch())
        .count();
  }

  bool isExpired(const std::string& key) {
    auto it = cache.find(key);

    if (it == cache.end()) return true;

    long long time = it->second.expiryTime;

    if (time == -1) return false;

    return getCurrentTime() >= time;
  }

 public:
  void set(const std::string& key, const std::string& value,
           long long expiryTime) {
    std::unique_lock lock(mutex);

    long long currentTime = getCurrentTime();

    cache[key].value = value;

    if (expiryTime == -1)
      cache[key].expiryTime = -1;
    else
      cache[key].expiryTime = currentTime + expiryTime;
  }

  std::string get(const std::string& key) {
    std::unique_lock lock(mutex);

    auto it = cache.find(key);

    if (it == cache.end()) throw std::runtime_error("No Data Found");

    if (!isExpired(key)) return it->second.value;

    cache.erase(it);

    return "Nil";
  }

  void remove(const std::string& key) {
    std::unique_lock lock(mutex);

    cache.erase(key);
  }

  bool find(const std::string& key) {
    std::unique_lock lock(mutex);

    auto it = cache.find(key);

    if (it == cache.end()) return false;

    if (!isExpired(key)) return true;

    cache.erase(it);

    return false;
  }

  int getTTL(const std::string& key) {
    std::unique_lock lock(mutex);

    auto it = cache.find(key);

    if (it == cache.end()) return -2;

    long long expiry = it->second.expiryTime;

    if (expiry == -1) return -1;

    long long currentTime = getCurrentTime();

    if (!isExpired(key)) return static_cast<int>(expiry - currentTime);

    cache.erase(it);

    return -2;
  }

  int changeTTL(const std::string& key, long long time) {
    std::unique_lock lock(mutex);

    auto it = cache.find(key);

    if (it == cache.end()) return 0;

    if (isExpired(key)) {
      cache.erase(it);
      return 0;
    }

    it->second.expiryTime = getCurrentTime() + time;

    return 1;
  }
};