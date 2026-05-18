# Remote POSIX Mini-Shell Server 🐚🌐

A lightweight, concurrent, and high-performance remote terminal server implemented in **pure C**. The project demonstrates advanced Linux systems programming techniques, network socket handling, and deterministic process synchronization.

By using asynchronous edge-case handling, it completely avoids deadlocks and eliminates zombie processes while serving multiple clients simultaneously without blocking.

---

## 🚀 Key Architectural Features

### 1. I/O Multiplexing via `poll()`
Instead of spawning a unique heavy kernel thread for every incoming network request, the server uses a dynamic `poll()` event loop. This allows a single-threaded server architecture to scale and efficiently monitor multiple network file descriptors (FDs) concurrently.

### 2. Advanced Process Orchestration & Pipe Handling
The engine dynamically parses inputs to look for IPC instructions like pipes (`|`). 
* **Process Isolation:** Spawns targeted child and grandchild processes to encapsulate command executions.
* **Deterministic I/O Streams:** Safely orchestrates file-descriptor redirections via `dup2()`. It guarantees deterministic terminal prompt deliveries by flushing inputs/outputs to the network socket only after the target process sequence completes.

### 3. Asynchronous Signal Handling (Anti-Zombie Guard)
To prevent accumulation of resource-leaking "Zombie Processes", the engine implements an asynchronous execution monitoring handler utilizing POSIX signals:
* Overrides `SIGCHLD` via `sigaction`.
* Periodically invokes a non-blocking `waitpid(-1, NULL, WNOHANG)` sweep to thoroughly reap completed child processes without context-blocking the core parent routine.

---

## 🛠️ Design & Data Flow

1. **Connection Stage:** Client establishes a TCP connection -> `poll()` intercepts `POLLIN` on the main server socket -> connection accepted and registered into the active file-descriptor array.
2. **Command Analysis:** Input is stripped of trailing whitespaces. If a pipe symbol (`|`) is located, the execution pipeline splits into a consumer-producer fork chain.
3. **Execution Stage:** Resolves path directories inside the environment `$PATH`, triggers `execv` mappings, writes back execution outputs to the socket stream, and returns a secure, refreshed network prompt.

---

## 💻 How to Build and Run

### Prerequisites
* Linux / Unix-based environment.
* `gcc` compiler.

### Installation & Compilation
Clone the repository and compile using the standard C build toolchain:

```bash
gcc -Wall -Wextra -O2 main.c -o remote_shell