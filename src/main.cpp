

#include "tcp_server.hpp"
int main() {
  TCPServer server(6379);
  server.start();
}