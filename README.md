# Redis-Inspired Key-Value Server

A lightweight Redis-inspired in-memory key-value database server built from scratch in **C++17**. It communicates over TCP using a custom implementation of the **Redis Serialization Protocol (RESP)** and implements concurrent client handling, key expiration, and append-only persistence.

The project focuses on understanding the internals of networked database systems, including protocol parsing, concurrent request handling, synchronization, expiration semantics, and persistence.

## Features

* TCP client-server communication using POSIX sockets
* Custom RESP parser and serializer
* Redis CLI compatibility
* In-memory key-value storage
* Fixed-size worker thread pool for concurrent client handling
* Thread-safe access to shared data
* Key expiration and TTL support
* Append-Only File (AOF) persistence
* Automatic state recovery from AOF on server startup

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

Each accepted client connection is placed into a connection queue and assigned to a worker from a fixed-size thread pool. Commands received over TCP are parsed from RESP, dispatched to the appropriate datastore operation, and serialized back into RESP responses.

Shared datastore access is synchronized to support concurrent clients safely.

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

## RESP Protocol

The server implements the Redis Serialization Protocol rather than relying on a Redis networking library.

For example:

```text
SET name Shivani
```

is transmitted as:

```text
*3\r\n
$3\r\n
SET\r\n
$4\r\n
name\r\n
$7\r\n
Shivani\r\n
```

The server parses the command, executes it through the command dispatcher, and serializes the result back into RESP format.

This allows the server to communicate directly with standard tools such as `redis-cli`.

## Key Expiration

Keys can be assigned expiration times using `EXPIRE` or during supported `SET` operations.

Expiration timestamps are maintained using C++ time utilities. Expired keys are treated as nonexistent when accessed.

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

## Persistence

The server implements **Append-Only File (AOF)** persistence.

Write operations are recorded in an append-only log. When the server starts, the AOF is replayed to reconstruct the in-memory datastore.

```text
Command
   |
   +----> DataStore
   |
   +----> AOF
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

This allows stored data to survive server restarts.

## Concurrency

Instead of creating an unbounded number of threads, the server uses a **fixed-size worker thread pool** based on `std::thread::hardware_concurrency()`.

Accepted client connections are placed in a synchronized queue. Available workers retrieve connections from the queue and process commands while shared datastore operations are protected against concurrent access.

This bounds thread creation and avoids creating a new operating-system thread for every incoming connection.

### Scalability Consideration

The current implementation uses blocking socket I/O, and a worker handles a persistent connection for its lifetime. Therefore, a worker can remain occupied while waiting for additional commands from its assigned client.

For workloads involving significantly more persistent connections than available workers, an event-driven networking layer using mechanisms such as `epoll`, `kqueue`, or IOCP would provide better connection scalability.

The current design intentionally focuses on a thread-pool-based concurrent server architecture.

## Performance Testing

The server was tested using `redis-benchmark` to evaluate command processing and concurrent connection behavior.

Example:

```bash
redis-benchmark -p 6379 -t set,get -n 1000 -c 1
```

A single-client baseline successfully processed GET and SET workloads with sub-millisecond latency.

Higher-concurrency testing was also used to identify the scalability characteristics of the blocking-I/O worker-pool architecture described above.

> Benchmark results are environment-dependent and are intended for evaluating this implementation rather than comparison with production Redis.

## Build

Requires:

* C++17-compatible compiler
* POSIX-compatible operating system
* pthread support

Compile:

```bash
g++ -std=c++17 -pthread src/main.cpp -o redis_server
```

> Adjust the source path if building from a different directory.

## Run

Start the server:

```bash
./redis_server
```

By default, the server listens on:

```text
127.0.0.1:6379
```

Connect using Redis CLI:

```bash
redis-cli -p 6379
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

## Testing

The server can currently be tested directly using `redis-cli` and `redis-benchmark`.

Integration tests covering command execution, expiration, concurrent access, and persistence are planned.

## Technologies

* C++17
* POSIX Sockets
* TCP/IP
* Multithreading
* `std::thread`
* `std::mutex`
* `std::condition_variable`
* Redis Serialization Protocol (RESP)
* Append-Only File persistence
* STL

## Future Improvements

The current implementation focuses on the core architecture of an in-memory database server. Possible extensions include:

* Automated integration test suite
* Non-blocking/event-driven networking
* Additional Redis data types and commands
* AOF compaction/rewrite

## What This Project Explores

This project was built to explore several systems and backend engineering concepts:

* TCP socket programming
* Application-layer protocol implementation
* Concurrent server architecture
* Thread pools and task queues
* Mutexes and condition variables
* Shared-state synchronization
* In-memory database design
* Key expiration
* Persistence and recovery
* Performance benchmarking
* Scalability trade-offs

## Author

**Shivani Dwivedi**
