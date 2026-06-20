#pragma once
#include <chrono>
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
  bool isExpired(const std::string& key) {
    auto it = cache.find(key);
    if (it == cache.end()) return true;
    long long time = it->second.expiryTime;
    if (time == -1) return false;
    return getCurrentTime() >= time;
  }

  long long getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
               now.time_since_epoch())
        .count();
  }

 public:
  void set(const std::string& key, const std::string& value,
           long long expiryTime) {
    auto now = std::chrono::system_clock::now();
    long long currentTime =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
            .count();
    cache[key].value = value;
    if (expiryTime == -1) {
      cache[key].expiryTime = -1;
    } else {
      cache[key].expiryTime = currentTime + expiryTime;
    }
  }
  std::string get(const std::string& key) {
    if (cache.find(key) != cache.end()) {
      if (!isExpired(key)) return cache[key].value;
      cache.erase(key);
      return "Nil";
    }
    throw std::runtime_error("No Data Found");
  }
  void remove(const std::string& key) {
    if (cache.find(key) != cache.end()) cache.erase(key);
  }
  bool find(const std::string& key) {
    if (cache.find(key) != cache.end()) {
      if (!isExpired(key)) return true;
      cache.erase(key);
      return false;
    }
    return false;
  }
  int getTTL(const std::string& key) {
    if (cache.find(key) == cache.end()) return -2;
    long long time = cache[key].expiryTime;
    if (time == -1) return (int)time;
    auto now = std::chrono::system_clock::now();
    long long currentTime =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
            .count();

    if (!isExpired(key)) return (int)time - currentTime;
    cache.erase(key);
    return -2;
  }
  void changeTTL(std::string key, long long time) {
    auto now = std::chrono::system_clock::now();
    long long currentTime =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
            .count();
    cache[key].expiryTime = currentTime + time;
  }
};