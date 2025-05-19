# operating-thing-project
? Just 1 lazy and 1 incredible peoples project about things that they don't understand
# 🚀 OS Network Tools Project 5: Route Table Retrieval

&#x20;

---

## 📝 Project Overview

In **Project 5** of our Operating System Network Tools series, we implement a pair of UDP-based applications:

1. **UDP Client**: Sends a `"get route table"` request (up to 1000 characters) to the server and displays the received route table.
2. **UDP Server**: Listens for `"get route table"` requests and responds with the system's formatted route table (up to 1000 characters).

This project demonstrates:

* Low-level socket programming (IPv4/IPv6)
* Message formatting and parsing
* Basic network diagnostics on local machines

---

## 👥 Team Composition

* **Alice Smith** — Frontend & Documentation
* **Bob Jones** — Backend & Networking

*Working in tandem, we designed, coded, and tested both client and server components with robust error handling and clear console output.*

---

## 🚀 Getting Started

### Prerequisites

* Linux-based OS (tested on  CentOS7)
* C/C++ compiler (GCC)
* Wireshark (optional, for packet inspection)

### Installation & Setup

1. Clone this repository:

   ```bash
   git clone https://github.com/your-org/os-network-tools.git
   cd os-network-tools/project5
   ```

2. Build the binaries (for C/C++ version):

   ```bash
   make
   ```

3. Install Python dependencies (for Python version):

   ```bash
   pip install -r requirements.txt
   ```

---

## 📦 Usage

### 1. Start the UDP Server

```bash
# C/C++ version
./udp_route_server <port>
```

### 2. Run the UDP Client

```bash
# C/C++ version
./udp_route_client <server_ip> <port>

# Python version
python3 udp_route_client.py --host 127.0.0.1 --port 9000
```

Upon running, the client will print the route table received from the server in a neatly formatted table.

---

## 📂 Project Structure

```
project5/
├── src/
│   ├── udp_route_client.c
│   ├── udp_route_server.c
├── include/
│   └── utils.h
├── Makefile
├── requirements.txt
└── README.md
```

---

## 🎯 Learning Outcomes

* Mastery of UDP sockets and network byte order conversions.
* Familiarity with parsing and displaying system route tables.
* Experience with cross-language implementations (C/C++ & Python).

---

## 📄 License

This project is licensed under the MIT License. See [LICENSE](../LICENSE) for details.

---

## 💬 Questions?

Feel free to open an issue or contact **Alice Smith** at [alice@university.edu](mailto:alice@university.edu).
