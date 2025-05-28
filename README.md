# UDP Route Table Retrieval (OS Network Tools - Project 5)

## 📝 Project Overview

This project implements a UDP-based client-server application in C for retrieving and displaying a system's IP routing table. It corresponds to **Project V** from the course assignment. The two components are:

1. **UDP Client**: Sends a fixed-size request string (`"get route table"`) to the server. Upon receiving the routing table response, it displays it on the console.
2. **UDP Server**: Listens for the request. When it receives the valid 1000-character request string, it executes the system command (`ip route show` or `netstat -nr`), captures up to 1000 bytes of output, and sends it back to the client.

This demonstrates key concepts in network programming with IPv4, including message handling, socket I/O, system command integration, and command-line parsing.

## 👥 Team Composition

* **Nurefşan Altın** — Backend & Networking
* **Arda Engin** — Client Interface & Documentation

---

## 🚀 Getting Started

### Prerequisites

* A Linux-based OS (e.g., CentOS 7, Ubuntu) or macOS with `ip route show` / `netstat -nr` available.
* C compiler (GCC or Clang).
* `make` (optional, if a Makefile is provided).

### Compilation

1. Ensure you have the source files: `udp_client.c` and `udp_server.c`.

2. Compile the server:

   ```bash
   gcc udp_server.c -o udp_server
   ```

3. Compile the client:

   ```bash
   gcc udp_client.c -o udp_client
   ```

*(If you have a `Makefile`, you can typically run `make` to build both.)*

---

## 💡 Usage

The client and server accept optional command-line arguments to override default ports and IP addresses.

### 1. Start the UDP Server

**Syntax**:

```bash
./udp_server [SERVER_PORT [SERVER_BIND_IP]]
```

**Examples**:

* Default (port `5005`, bind to `0.0.0.0`):

  ```bash
  ./udp_server
  ```
* Specific port (`5555`):

  ```bash
  ./udp_server 5555
  ```
* Specific port and IP (`192.168.1.100`):

  ```bash
  ./udp_server 5555 192.168.1.100
  ```

The server will print status messages like "Socket created", "Bound to ...", and "Listening...".

### 2. Run the UDP Client

**Syntax**:

```bash
./udp_client <LOCAL_PORT> <SERVER_IP> [SERVER_PORT]
```

* `<LOCAL_PORT>`: The UDP port for the client to bind (default: `6006`).
* `<SERVER_IP>`: The server's IP address (`127.0.0.1`).
* `[SERVER_PORT]`: Optional (default: `5005`).

**Examples**:

* Connect to default server:

  ```bash
  ./udp_client 6006 127.0.0.1
  ```
* Connect to server on port `5555`:

  ```bash
  ./udp_client 6006 127.0.0.1 5555
  ```
* Connect to remote server:

  ```bash
  ./udp_client 6006 192.168.1.100 5555
  ```

**Client Commands**:

* `get route table` — Request and display the routing table.
* `quit` — Exit the client.

---

## 📂 Project Structure

This project consists of the following core files:

* `udp_client.c` — Source code for the client application.
* `udp_server.c` — Source code for the server application and `get_route_table()` implementation.
* `README.md` — This documentation file.
* `Makefile` (optional) — For easy compilation.

---

## 🎯 Learning Outcomes & Key Features

* **UDP Socket Programming**: Use of `socket()`, `bind()`, `sendto()`, and `recvfrom()` in C for IPv4.
* **Client-Server Architecture**: Separation of request initiation (client) and response provision (server).
* **Message Handling**: Fixed-size buffers (`BUF_SIZE = 1000`), precise `sendto`/`recvfrom` lengths.
* **System Command Interaction**: Server uses `popen()` to run `ip route show` or `netstat -nr` and captures its output.
* **Command-Line Parsing**: Use of `argc`, `argv[]`, and `atoi()` to parse ports and IPs.
* **Error Handling**: Comprehensive checks with `perror()` and early exits.
* **Style & Naming**: Descriptive variable names (e.g., `sock_fd`, `server_addr`, `req_buf`) and Allman brace style.

---

## 📜 License

This project is released under the **MIT License**. See `LICENSE` for details.
