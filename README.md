# Redis-Inspired Key-Value Server

A lightweight Redis-inspired in-memory key-value database server built in **C++**. The server communicates over TCP using a custom implementation of the **RESP (Redis Serialization Protocol)** and supports concurrent client connections, key expiration, and append-only file (AOF) persistence.

---

## Features

- TCP client-server communication
- Concurrent client handling using multithreading
- RESP protocol parsing and serialization
- In-memory key-value storage
- Key expiration (TTL)
- Persistent storage using Append-Only File (AOF)
- Automatic recovery from AOF on server startup

---

## Supported Commands

| Command | Description |
|---------|-------------|
| `SET key value` | Store a key-value pair |
| `GET key` | Retrieve the value of a key |
| `DEL key` | Delete a key |
| `EXISTS key` | Check whether a key exists |
| `TTL key` | Get the remaining time-to-live of a key |
| `EXPIRE key seconds` | Set an expiration time on a key |
| `PING` | Test server connectivity |

---

## Technologies Used

- C++17
- POSIX Socket Programming
- TCP/IP Networking
- Multithreading (`std::thread`, `std::mutex`)
- RESP (Redis Serialization Protocol)
- Append-Only File (AOF) Persistence
- STL
- Git & GitHub

---

## Project Structure

```
.
├── tcp_server.hpp
├── data_store.hpp
├── dispatcher.hpp
├── resp_parser.hpp
├── serializer.hpp
├── aof_manager.hpp
├── main.cpp
└── README.md
```

---

## Build

Compile the project using a C++17 compatible compiler.

Example:

```bash
g++ -std=c++17 -pthread main.cpp -o redis_server
```

---

## Run

```bash
./redis_server
```

The server listens on **port 6379** by default.

---

## Testing

Connect using `redis-cli`:

```bash
redis-cli -p 6379
```

Example:

```text
SET name Shivani
OK

GET name
Shivani

EXPIRE name 10
(integer) 1

TTL name
(integer) 9
```

---

## Future Improvements

- Thread pool for client handling
- Event-driven I/O (`epoll` / `kqueue`)
- Additional Redis commands
- Performance benchmarking

---

## Author

**Shivani Dwivedi**