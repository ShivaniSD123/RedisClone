#pragma once

#include <chrono>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../include/data_store.hpp"

class AOFManager {
 private:
  std::ofstream file;
  std::mutex fileMutex;

 public:
  AOFManager() {
    file.open("src/persistence/appendonly.aof", std::ios::app);

    if (!file.is_open()) {
      throw std::runtime_error("Unable to open AOF");
    }
  }

  ~AOFManager() {
    if (file.is_open()) {
      file.close();
    }
  }

  void append(const std::vector<std::string>& command) {
    std::lock_guard<std::mutex> lock(fileMutex);

    for (const auto& s : command) {
      file << s << " ";
    }

    file << '\n';
    file.flush();
  }

  long long getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
               now.time_since_epoch())
        .count();
  }

  void loadCommands(DataStore& store) {
    std::ifstream input("src/persistence/appendonly.aof");

    if (!input.is_open()) std::cout << "Cant open!!" << std::endl;

    std::string line;

    while (std::getline(input, line)) {
      std::stringstream ss(line);

      std::string token;
      std::vector<std::string> tokens;

      while (ss >> token) {
        tokens.push_back(token);
      }

      if (tokens.empty()) continue;

      if (tokens[0] == "SET" && tokens.size() == 3) {
        store.set(tokens[1], tokens[2], -1);

      } else if (tokens[0] == "SET" && tokens.size() == 5) {
        long long storedExpiry = std::stoll(tokens[4]);

        if (storedExpiry == -1) {
          store.set(tokens[1], tokens[2], -1);

        } else {
          long long remaining = storedExpiry - getCurrentTime();

          if (remaining > 0) {
            store.set(tokens[1], tokens[2], remaining);
          }
        }

      } else if (tokens[0] == "DEL" && tokens.size() == 2) {
        store.remove(tokens[1]);

      } else if (tokens[0] == "EXPIRE" && tokens.size() == 3) {
        if (!store.find(tokens[1])) continue;

        long long absoluteExpiry = std::stoll(tokens[2]);

        long long remaining = absoluteExpiry - getCurrentTime();

        if (remaining > 0) {
          store.changeTTL(tokens[1], remaining);
        }
      }
    }
  }
};