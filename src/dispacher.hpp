#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "data_store.hpp"
#include "response_type.hpp"

struct Response {
  response_type type;
  std::string value;
};
class Dispatcher {
 private:
  DataStore& d;

 public:
  Dispatcher(DataStore& store) : d(store) {}

  std::string get_value(const std::string& key) { return d.get(key); }
  void set_value(const std::string& key, const std::string& value,
                 long long expiryTime) {
    d.set(key, value, expiryTime);
  }
  void del_value(const std::string& key) { d.remove(key); }
  bool check(const std::string& key) { return d.find(key); }
  int getTTL(const std::string& key) { return d.getTTL(key); }
  Response execute(const std::vector<std::string>& cmd) {
    if (cmd.empty()) return {response_type::ERROR, "INVALID COMMAND"};
    const std::string& token = cmd[0];
    if (token == "GET") {
      if (cmd.size() != 2) return {response_type::ERROR, "INVALID COMMAND"};
      const std::string& key = cmd[1];
      return {response_type::BULK_STRING, get_value(key)};
    } else if (token == "SET") {
      if (cmd.size() < 3) return {response_type::ERROR, "INVALID COMMAND"};
      const std::string& key = cmd[1];
      const std::string& value = cmd[2];
      long long expiryTime = -1;

      if (cmd.size() == 5) {
        if (cmd[3] != "EX") return {response_type::ERROR, "INVALID COMMAND"};
        expiryTime = stoll(cmd[4]);
      }
      set_value(key, value, expiryTime);
      return {response_type::SIMPLE_STRING, "OK"};
    } else if (token == "PING") {
      if (cmd.size() != 1) return {response_type::ERROR, "INVALID COMMAND"};
      return {response_type::SIMPLE_STRING, "PONG"};
    } else if (token == "DEL") {
      if (cmd.size() != 2) return {response_type::ERROR, "INVALID COMMAND"};
      del_value(cmd[1]);
      return {response_type::SIMPLE_STRING, "OK"};
    } else if (token == "EXISTS") {
      if (cmd.size() != 2) return {response_type::ERROR, "INVALID COMMAND"};
      const std::string& key = cmd[1];
      return {response_type::INTEGER, check(key) ? "1" : "0"};
    } else if (token == "TTL") {
      if (cmd.size() != 2) return {response_type::ERROR, "INVALID COMMAND"};
      const std::string key = cmd[1];
      return {response_type::INTEGER, std::to_string(getTTL(key))};
    } else
      return {response_type::ERROR, "INVALID COMMAND"};
  }
};