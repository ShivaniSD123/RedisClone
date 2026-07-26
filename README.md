# Redis-Inspired Key-Value Server

A lightweight Redis-inspired **in-memory key-value database server built from scratch in C++17**.

The server communicates over TCP using a custom implementation of the **Redis Serialization Protocol (RESP)** and supports concurrent clients, key expiration, TTL semantics, and Append-Only File (AOF) persistence.

The project focuses on understanding the internals of networked database systems, including socket programming, protocol parsing, concurrent request processing, synchronization, persistence, and containerization.

---

## Features

* TCP client-server communication using POSIX sockets
* Custom RESP parser and serializer
* Compatible with `redis-cli`
* In-memory key-value datastore
* Fixed-size worker thread pool for concurrent client handling
* Thread-safe access to shared datastore state
* Key expiration and TTL support
* Append-Only File (AOF) persistence
* Automatic state recovery from AOF on startup
* Redis benchmark compatibility for performance testing
* Dockerized build and runtime environment

---

## Architecture

```text
                     TCP Clients
                          |
                          v
                    +------------+
                    | TCP Server |
                    +------------+
                          |
                          v
                  +----------------+
                  | Connection     |
                  | Queue          |
                  +----------------+
                          |
                          v
                  +----------------+
                  |  Thread Pool   |
                  +----------------+
                          |
                          v
                    +-----------+
                    | RESP      |
                    | Parser    |
                    +-----------+
                          |
                          v
                    +------------+
                    | Dispatcher |
                    +------------+
                          |
                 +--------+--------+
                 |                 |
                 v                 v
          +-------------+    +-------------+
          |  DataStore  |    | AOF Manager |
          +-------------+    +-------------+
                 |
                 v
          +-------------+
          | Serializer  |
          +-------------+
                 |
                 v
            TCP Response
```

Each accepted TCP connection is placed into a synchronized connection queue. A worker from the fixed-size thread pool retrieves the connection and processes incoming commands.

Commands are:

1. Received over TCP
2. Parsed from RESP
3. Dispatched to the appropriate datastore operation
4. Applied to the in-memory datastore
5. Persisted when required
6. Serialized back into RESP
7. Returned to the client

Shared datastore access is synchronized to allow multiple clients to operate safely on the same in-memory state.

---

## Supported Commands

| Command              | Description                    |
| -------------------- | ------------------------------ |
| `SET key value`      | Store a key-value pair         |
| `GET key`            | Retrieve a value               |
| `DEL key`            | Delete a key                   |
| `EXISTS key`         | Check whether a key exists     |
| `EXPIRE key seconds` | Assign an expiration time      |
| `TTL key`            | Get the remaining time-to-live |
| `PING`               | Test server connectivity       |

Example:

```text
SET name Shivani
OK

GET name
"Shivani"

EXPIRE name 10
(integer) 1

TTL name
(integer) 9
```

---

## RESP Protocol

The server implements the **Redis Serialization Protocol (RESP)** directly instead of relying on a Redis networking or protocol library.

For example, the command:

```text
SET name Shivani
```

is represented over the network as:

```text
*3\r\n
$3\r\n
SET\r\n
$4\r\n
name\r\n
$7\r\n
Shivani\r\n
```

The server parses this representation into a command, executes it through the dispatcher, and serializes the result back into RESP format.

This makes the server compatible with standard Redis tooling such as:

```bash
redis-cli
```

---

## Key Expiration and TTL

Keys can be assigned expiration times using `EXPIRE` or during supported `SET` operations.

Expiration timestamps are maintained using C++ time utilities. When an expired key is accessed, it is treated as nonexistent.

Example:

```text
SET session abc
EXPIRE session 5
TTL session
```

After the expiration period:

```text
GET session
(nil)
```

The server also supports querying the remaining lifetime of a key through `TTL`.

---

## Persistence

The server implements **Append-Only File (AOF) persistence**.

Write operations are recorded in an append-only log. When the server starts, the AOF is read and previously persisted commands are replayed to reconstruct the in-memory datastore.

```text
Write Command
     |
     +-----------> DataStore
     |
     +-----------> AOF
                     |
                     v
                  Disk File


Server Restart
      |
      v
   Read AOF
      |
      v
Replay Commands
      |
      v
Restore DataStore
```

This provides persistence across normal server restarts.

---

## Concurrency

The server uses a **fixed-size worker thread pool** instead of creating a new operating-system thread for every incoming connection.

The number of workers is based on:

```cpp
std::thread::hardware_concurrency()
```

The architecture is:

```text
Incoming Connections
         |
         v
+-------------------+
| Connection Queue  |
+-------------------+
         |
    +----+----+
    |    |    |
    v    v    v
   W1   W2   W3  ... Wn
```

Accepted connections are placed into a synchronized queue.

Worker threads wait for available connections using synchronization primitives such as mutexes and condition variables.

Shared datastore operations are also synchronized to prevent data races when multiple clients access or modify the datastore concurrently.

### Scalability Consideration

The current implementation uses **blocking socket I/O**.

A worker processes a persistent client connection for its lifetime. Therefore, a worker may remain occupied while waiting for additional commands from that client.

For workloads involving substantially more persistent connections than available workers, an event-driven networking architecture using mechanisms such as:

* `epoll` on Linux
* `kqueue` on macOS/BSD
* IOCP on Windows

would provide better connection scalability.

The current implementation intentionally focuses on understanding and implementing a **thread-pool-based concurrent server architecture**.

---

## Performance Testing

The server can be tested using `redis-benchmark` to evaluate command throughput, latency, and concurrent connection behaviour.

Example:

```bash
redis-benchmark -p 6379 -t set,get -n 1000 -c 1
```

A single-client baseline can be used to verify command processing and latency before increasing concurrency.

For example:

```bash
redis-benchmark -p 6379 -t set,get -n 10000 -c 10
```

Higher-concurrency tests can also be used to observe the scalability characteristics of the blocking-I/O worker-pool architecture.

> Benchmark results depend on hardware, operating system, container configuration, and workload. They are intended to evaluate this implementation rather than directly compare it with production Redis.

---

# Running Locally

## Requirements

To compile the project locally, you need:

* C++17-compatible compiler
* POSIX-compatible operating system
* pthread support
* `redis-cli` for interactive testing

### Compile

From the project root:

```bash
g++ -std=c++17 -pthread src/main.cpp -o main
```

### Start the Server

```bash
./main
```

The server listens on TCP port:

```text
6379
```

The server binds to available network interfaces using `INADDR_ANY`.

### Connect with Redis CLI

Open another terminal and run:

```bash
redis-cli -h localhost -p 6379
```

Then execute commands normally:

```text
127.0.0.1:6379> PING
PONG

127.0.0.1:6379> SET language cpp
OK

127.0.0.1:6379> GET language
"cpp"
```

---

# Running with Docker

The project includes a `Dockerfile`, allowing the server to be built and executed inside an isolated Linux container.

Docker provides a reproducible environment containing the required compiler, dependencies, application source, and server executable.

## Build the Docker Image

From the project root:

```bash
docker build -t kvs-server .
```

Docker will:

```text
Read Dockerfile
      |
      v
Create Linux build environment
      |
      v
Install C++ compiler
      |
      v
Copy project files
      |
      v
Compile KVS server
      |
      v
Create kvs-server image
```

Verify that the image was created:

```bash
docker images
```

You should see an image named:

```text
kvs-server
```

---

## Run the Container

Create and start a container:

```bash
docker run --name kvs-container -p 6379:6379 kvs-server
```

The port mapping:

```text
-p 6379:6379
```

maps:

```text
Host Port                 Container Port

   6379    ------------->     6379
```

The resulting architecture is:

```text
                Host Machine

             redis-cli
                 |
                 | localhost:6379
                 v
          Host TCP Port 6379
                 |
                 | Docker port mapping
                 v
+---------------------------------------+
|          Docker Container             |
|                                       |
|          Port 6379                    |
|              |                        |
|              v                        |
|        C++ KVS Server                 |
|              |                        |
|      +-------+-------+                |
|      |               |                |
|      v               v                |
|   DataStore       AOF Manager         |
|                                       |
+---------------------------------------+
```

Because the server binds using `INADDR_ANY`, it can accept connections forwarded to the container's network interface.

---

## Connect to the Dockerized Server

Keep the container running and open another terminal:

```bash
redis-cli -h localhost -p 6379
```

Example:

```text
127.0.0.1:6379> PING
PONG

127.0.0.1:6379> SET name Shivani
OK

---

## Testing

The server can be tested interactively using:

```bash
redis-cli -h localhost -p 6379
```

and benchmarked using:

```bash
redis-benchmark -p 6379
```

Testing currently covers the server through Redis-compatible clients and benchmarking tools.

An automated integration test suite covering command execution, expiration, concurrent access, and persistence is a planned extension.

---

## Project Structure

A simplified view of the repository:

```text
redis_clone/
|
├── Dockerfile
├── README.md
|
└── src/
    ├── main.cpp
    |
    ├── connection/
    |   └── tcp_server.hpp
    |
    ├── include/
    |   ├── data_store.hpp
    |   ├── dispatcher.hpp
    |   ├── multi_access.hpp
    |   ├── resp_parser.hpp
    |   └── serializer.hpp
    |
    └── persistence/
        ├── aof_manager.hpp
        └── appendonly.aof
```

> The exact structure may evolve as additional features and tests are introduced.

---

## Technologies

* **C++17**
* **POSIX Sockets**
* **TCP/IP**
* **Multithreading**
* **Thread Pools**
* `std::thread`
* `std::mutex`
* `std::shared_mutex`
* `std::condition_variable`
* **Redis Serialization Protocol (RESP)**
* **Append-Only File (AOF) persistence**
* **STL**
* **Docker**
* **Redis CLI**
* **redis-benchmark**

---

## Engineering Concepts Explored

This project was built to explore systems and backend engineering concepts including:

* TCP socket programming
* Client-server architecture
* Application-layer protocol implementation
* RESP parsing and serialization
* Concurrent server design
* Worker thread pools
* Producer-consumer queues
* Mutexes and condition variables
* Shared-state synchronization
* In-memory database design
* Key expiration and TTL semantics
* Persistence and state recovery
* Blocking I/O
* Performance benchmarking
* Containerization
* Port mapping
* Reproducible build environments
* Connection scalability trade-offs

---

## Future Improvements

Potential extensions include:

* Automated unit and integration tests
* Non-blocking/event-driven networking
* `epoll`-based connection handling on Linux
* Additional Redis commands
* Additional Redis data types
* Improved error handling
* Graceful shutdown using `SIGINT`/`SIGTERM`
* AOF compaction and rewrite
* Docker volume support for persistent AOF storage
* Multi-stage Docker builds for smaller runtime images
* CI/CD pipeline for automated builds and testing

---

## Learning Goals

Rather than using existing database or networking abstractions, the project implements the core server components directly to better understand how networked database systems operate internally.

The implementation explores the complete request lifecycle:

```text
Client
   |
   v
TCP Connection
   |
   v
Connection Queue
   |
   v
Worker Thread
   |
   v
RESP Parser
   |
   v
Command Dispatcher
   |
   +---------> DataStore
   |
   +---------> AOF Persistence
   |
   v
RESP Serializer
   |
   v
TCP Response
   |
   v
Client
```

This provides hands-on experience with the interaction between **networking, concurrency, synchronization, protocol design, persistence, and containerization** in a backend system.

---

## Author

**Shivani Dwivedi**

