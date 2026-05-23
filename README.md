# Remote POSIX Command Server 🐚🌐

A concurrent remote command execution server written in **pure C** for Linux environments.
The project focuses on low-level systems programming concepts including POSIX process management, TCP socket communication, inter-process communication (IPC), and asynchronous client handling.

The server supports multiple remote clients simultaneously, command execution through the Linux shell environment, and Unix pipe orchestration using native Linux system calls.

---

# 🚀 Core System Features

## 1. Concurrent Client Handling with `poll()`

Instead of creating a dedicated blocking execution flow for every incoming client connection, the server utilizes a dynamic `poll()`-based event loop to monitor multiple active file descriptors concurrently.

This architecture enables:

* Efficient multi-client support
* Non-blocking connection management
* Lightweight event-driven network handling
* Scalable socket monitoring

---

## 2. POSIX Process Management & Command Execution

The execution engine dynamically parses incoming commands and launches isolated child processes for execution.

Implemented system-level mechanisms include:

* `fork()` for process creation
* `execv()` for executable mapping
* `waitpid()` for child synchronization
* `$PATH` resolution for command discovery
* `dup2()` for file descriptor redirection

The server supports:

* Standard Linux command execution
* Remote terminal interaction
* Pipe-based command chaining (`|`)

---

## 3. Inter-Process Communication (IPC) with Pipes

Pipe handling is implemented using native Unix pipes.

When a pipe symbol (`|`) is detected:

* The command sequence splits into producer/consumer execution stages
* Two synchronized child processes are spawned
* Output redirection is handled through `dup2()`
* Pipe file descriptors are carefully managed to prevent descriptor leaks

This demonstrates practical IPC orchestration in Linux environments.

---

## 4. Asynchronous Signal Handling & Zombie Prevention

To prevent zombie process accumulation during concurrent execution, the server overrides `SIGCHLD` using `sigaction()`.

A non-blocking cleanup routine continuously reaps completed child processes through:

```c
waitpid(-1, NULL, WNOHANG)
```

This ensures:

* Proper child lifecycle management
* Stable long-running server execution
* Non-blocking parent process behavior

---

# 🛠️ System Architecture

```text
Remote Clients
       │
       ▼
TCP Socket Server
       │
       ▼
poll() Event Loop
       │
       ▼
Command Parsing Engine
       │
 ┌─────┴─────┐
 ▼           ▼
fork()    pipe()
 ▼           ▼
execv()  dup2()
       │
       ▼
Remote Output Stream
```

---

# ⚙️ Key Linux APIs & Concepts

| Category           | Technologies                                           |
| ------------------ | ------------------------------------------------------ |
| Networking         | `socket()`, `bind()`, `listen()`, `accept()`, `recv()` |
| Multiplexing       | `poll()`                                               |
| Process Management | `fork()`, `waitpid()`                                  |
| Command Execution  | `execv()`                                              |
| IPC                | `pipe()`, `dup2()`                                     |
| Signal Handling    | `sigaction()`                                          |
| Memory Management  | `malloc()`, `free()`                                   |
| Linux Concepts     | File descriptors, concurrent systems, POSIX APIs       |

---

# 💻 Supported Functionality

## Basic Command Execution

```bash
ls
pwd
whoami
date
```

---

## Pipe Execution

```bash
ls | wc
ps aux | grep bash
```

---

## Multiple Remote Clients

The server supports simultaneous remote client sessions through socket multiplexing.

---

# 🔧 Build & Run

## Compilation

```bash
gcc -Wall -Wextra -O2 main.c -o remote_shell
```

---

## Start Server

```bash
./remote_shell
```

Expected output:

```text
Server listening on port 8080 with poll()...
```

---

# 🌐 Connect to the Server

## Using Netcat

```bash
nc 127.0.0.1 8080
```

## Using Telnet

```bash
telnet 127.0.0.1 8080
```

---

# 📚 Learning Objectives

This project was developed to strengthen understanding of:

* Linux systems programming
* POSIX APIs
* Concurrent server architecture
* TCP/IP socket communication
* Inter-process communication (IPC)
* Process synchronization
* Signal handling
* Low-level software design

---

# 🚀 Future Improvements

* Multi-pipe command support
* Thread-based concurrency using `pthreads`
* Authentication & permissions
* Logging subsystem
* Thread pool architecture
* Advanced shell parsing
* Configuration file support
* Improved error handling & hardening

---

# 👨‍💻 Author

**Neriya Zikri**
Computer Science Student focused on:

* Systems Programming
* Linux Internals
* Networking
* Low-Level Software Development
* Concurrent Systems
