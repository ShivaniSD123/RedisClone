
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "aof_manager.hpp"
#include "data_store.hpp"
#include "dispacher.hpp"
#include "resp_parser.hpp"
#include "serializer.hpp"

class TCPServer {
 private:
  int port;

  DataStore store;
  AOFManager aof;
  RESPParser parser;
  Dispatcher dispatcher;
  Serializer serializer;

 public:
  TCPServer(int port)
      : port(port),
        store(),
        aof(),
        parser(),
        dispatcher(store, aof),
        serializer() {}

  void start() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));

    listen(server_fd, 5);

    std::cout << "Server listening on port " << port << "\n";

    while (true) {
      int client_fd = accept(server_fd, nullptr, nullptr);

      char buffer[4096];

      while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes <= 0) break;

        std::string input(buffer, bytes);

        try {
          auto command = parser.parser(input);

          Response response = dispatcher.execute(command);

          std::string resp = serializer.execute(response);

          send(client_fd, resp.c_str(), resp.size(), 0);
        } catch (const std::exception& e) {
          Response response{response_type::ERROR, e.what()};

          std::string resp = serializer.execute(response);

          send(client_fd, resp.c_str(), resp.size(), 0);
        }
      }

      close(client_fd);
    }

    close(server_fd);
  }
};