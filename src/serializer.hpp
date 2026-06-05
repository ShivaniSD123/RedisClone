#pragma once

#include <string>

#include "response_type.hpp"

struct Response {
  response_type type;
  std::string value;
};
class Serializer {
 private:
  std::string respFormatConvert(const Response& response) {
    std::string responseValue = response.value;
    std::string sz = std::to_string(responseValue.size());
    switch (response.type) {
      case response_type::INTEGER:
        return ":" + responseValue + "\r\n";
      case response_type::BULK_STRING:

        return "$" + sz + "\r\n" + responseValue + "\r\n";
      case response_type::ERROR:
        return "-ERR " + responseValue + "\r\n";
      case response_type::SIMPLE_STRING:
        return "+" + responseValue + "\r\n";
      default:
        return "-ERR unknown response type\r\n";
    }
  }

 public:
  std::string execute(const Response& response) {
    return respFormatConvert(response);
  }
};
