#include <iostream>
#include <stdexcept>
#include <vector>

class RESPParser {
 private:
  std::vector<std::string> parse(std::string& input) {
    std::vector<std::string> command;
    int n = input.size();
    if (input.empty()) throw std::runtime_error("Invalid Input...");
    if (input[0] != '*') throw std::runtime_error("Invalid Input...");
    int i = 0;
    while (i < n && input[i] != '\n') i++;
    i++;

    while (i < n) {
      while (i < n && input[i] != '\n') i++;
      i++;

      std::string token;
      while (i < n && input[i] != '\r') {
        token += input[i];
        i++;
      }

      command.push_back(token);
      i += 2;
    }
    return command;
  }

 public:
  std::vector<std::string> parser(std::string& input) { return parse(input); }
};
