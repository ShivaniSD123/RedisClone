#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
 private:
  std::vector<std::thread> workers;
  std::queue<int> clientQueue;
  std::mutex queueMutex;
  std::condition_variable condition;
  bool running = true;
  std::function<void(int)> task;

 private:
  void worker() {
    while (running) {
      std::unique_lock<std::mutex> lock(queueMutex);

      condition.wait(lock,
                     [this]() { return !clientQueue.empty() || !running; });

      if (!running) return;

      int client = clientQueue.front();
      clientQueue.pop();
      lock.unlock();
      task(client);
    }
  }

 public:
  ThreadPool(int n, std::function<void(int)> task) : task(task) {
    workers.reserve(n);
    for (int i = 0; i < n; i++) {
      workers.emplace_back(&ThreadPool::worker, this);
    }
  }
  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      running = false;
    }

    condition.notify_all();
    for (auto& thread : workers) {
      if (thread.joinable()) thread.join();
    }
  }
  void enqueue(int client_fd) {
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      clientQueue.push(client_fd);
    }
    condition.notify_one();
  }
};