#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "data_store.hpp"

class AOFManager {
 public:
  void append(const std::vector<std::string>& command) {
    std::ofstream file("appendonly.aof", std::ios::app);
    if (!file.is_open()) {
      throw std::runtime_error("Unable to open AOF");
    }
    for (const std::string& s : command) file << s << " ";
    file << "\n";
  }
  void loadCommands(DataStore& store) {
    std::ifstream file("appendonly.aof");
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
      std::stringstream ss(line);
      std::string token;
      std::vector<std::string> tokens;
      while (ss >> token) {
        tokens.push_back(token);
      }
      if (tokens.empty()) continue;
      if (tokens[0] == "SET" && tokens.size() >= 3) {
        store.set(tokens[1], tokens[2], -1);
      } else if (tokens[0] == "DEL" && tokens.size() >= 2) {
        store.remove(tokens[1]);
      }
    }
  }
};