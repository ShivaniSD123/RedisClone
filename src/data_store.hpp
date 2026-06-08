#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>

class DataStore {
 private:
  std::unordered_map<std::string, std::string> cache;

 public:
  void set(const std::string& key, const std::string& value) {
    cache[key] = value;
  }
  std::string get(const std::string& key) {
    if (cache.find(key) != cache.end()) return cache[key];
    throw std::runtime_error("No Data Found");
  }
  void remove(const std::string& key) {
    if (cache.find(key) != cache.end()) cache.erase(key);
  }
  bool find(const std::string& key) { return cache.find(key) != cache.end(); }
};