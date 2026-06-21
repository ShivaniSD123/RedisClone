#pragma once

#include <fstream>
#include <string>
#include <vector>

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
};