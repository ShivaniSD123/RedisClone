#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "../include/data_store.hpp"
#include "../include/dispacher.hpp"
#include "../include/resp_parser.hpp"
#include "../include/serializer.hpp"
#include "../persistence/aof_manager.hpp"

class TCPServer {
 private:
  int port;

  DataStore store;
  AOFManager aof;
  RESPParser parser;
  Dispatcher dispatcher;
  Serializer serializer;

 private:
  void handleClient(int client_fd) {
    char buffer[4096];

    while (true) {
      memset(buffer, 0, sizeof(buffer));

      int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

      if (bytes == 0) {
        std::cout << "Client disconnected\n";
        break;
      }

      if (bytes < 0) {
        perror("recv");
        break;
      }

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

    if (server_fd < 0) {
      perror("socket");
      return;
    }

    // Allows immediate restart after server exits
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
      perror("bind");
      close(server_fd);
      return;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
      perror("listen");
      close(server_fd);
      return;
    }

    std::cout << "Loading AOF..." << std::endl;
    aof.loadCommands(store);

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
      int client_fd = accept(server_fd, nullptr, nullptr);

      if (client_fd < 0) {
        perror("accept");
        continue;
      }

      std::cout << "Client connected." << std::endl;

      std::thread(&TCPServer::handleClient, this, client_fd).detach();
    }

    close(server_fd);
  }
};