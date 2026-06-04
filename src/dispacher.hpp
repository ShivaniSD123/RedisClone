#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "data_store.hpp"

class Dispacher {
 private:
  datastore& d;

 public:
  Dispacher(datastore& store) : d(store) {}

  std::string get_value(const std::string& key) { return d.get(key); }
  void set_value(const std::string& key, const std::string& value) {
    d.set(key, value);
  }
  void del_value(const std::string& key) { d.remove(key); }
  bool check(const std::string& key) { return d.find(key); }
  std::string execute(const std::vector<std::string>& cmd) {
    if (cmd.empty()) return "INVALID COMMAND";
    const std::string& token = cmd[0];
    if (token == "GET") {
      if (cmd.size() < 2) return "INVALID COMMAND";
      const std::string& key = cmd[1];
      return get_value(key);
    } else if (token == "SET") {
      if (cmd.size() < 3) return "INVALID COMMAND";
      const std::string& key = cmd[1];
      const std::string& value = cmd[2];
      set_value(key, value);
      return "OK";
    } else if (token == "PING") {
      if (cmd.size() != 1) return "INVALID COMMAND";
      return "PONG";
    } else if (token == "DEL") {
      if (cmd.size() != 2) return "INVALID COMMAND";
      del_value(cmd[1]);
      return "OK";
    } else if (token == "EXISTS") {
      if (cmd.size() != 2) return "INVALID COMMAND";
      const std::string& key = cmd[1];
      return check(key) ? "1" : "0";
    } else
      return "UNKNOWN COMMAND";
  }
};