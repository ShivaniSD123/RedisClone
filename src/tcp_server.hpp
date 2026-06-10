
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "data_store.hpp"
#include "dispacher.hpp"
#include "resp_parser.hpp"
#include "serializer.hpp"
#include <netinet/in.h>

class TCPServer {
 private:
  int port;

  DataStore store;
  RESPParser parser;
  Dispatcher dispatcher;
  Serializer serializer;

 public:
  TCPServer(int port)
      : port(port), store(), parser(), dispatcher(store), serializer() {}

  void start() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
      std::perror("socket");
      return;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      std::perror("setsockopt");
      close(server_fd);
      return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
      std::perror("bind");
      close(server_fd);
      return;
    }

    if (listen(server_fd, 5) < 0) {
      std::perror("listen");
      close(server_fd);
      return;
    }

    std::cout << "Server listening on port " << port << "\n";

    while (true) {
      int client_fd = accept(server_fd, nullptr, nullptr);

      char buffer[4096];
      std::string recvBuf;

      while (true) {
        int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes < 0) {
          std::perror("recv");
          break;
        }
        if (bytes == 0) break; // client closed

        recvBuf.append(buffer, buffer + bytes);

        // simple heuristic: only attempt parse when we see CRLF and leading '*'
        if (recvBuf.size() > 0 && recvBuf[0] == '*' && recvBuf.find("\r\n") != std::string::npos) {
          try {
            auto command = parser.parser(recvBuf);
            Response response = dispatcher.execute(command);
            std::string resp = serializer.execute(response);
            if (send(client_fd, resp.c_str(), resp.size(), 0) < 0) {
              std::perror("send");
              break;
            }
            // assume one command per buffer for now; clear buffer
            recvBuf.clear();
          } catch (const std::exception& e) {
            // parser failed; send error response and clear buffer
            Response response{response_type::ERROR, e.what()};
            std::string resp = serializer.execute(response);
            if (send(client_fd, resp.c_str(), resp.size(), 0) < 0) {
              std::perror("send");
              break;
            }
            recvBuf.clear();
          }
        }
      }

      close(client_fd);
    }

    close(server_fd);
  }
};