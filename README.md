# Quick Start Guide

**libconn** is a lightweight and user-friendly library designed to simplify the development of TCP, HTTP, and WebSocket clients and servers. With built-in support for OpenSSL, it ensures secure and efficient communication for your applications.

## Getting Started

Follow these simple steps to get started with **libconn**:

1. **Download the Code**

   Clone the repository to your local machine using the following command:

   ```shell
   git clone git@github.com:brightzi/libconn.git
   ```

2. **Build the Library**

   Navigate to the project directory and build the library:

   ```shell
   cd libconn
   mkdir build
   cd build
   cmake ..
   make -j
   ```

3. **Explore the Examples**

   After building, you will find a variety of example executables in the `bin` directory. These examples demonstrate the functionality of **libconn**

   ```
   test_http_server
   test_http_client
   test_tcp_client
   test_tcp_server
   test_tcp_ssl_client
   test_tcp_ssl_server
   test_ws_client
   test_ws_server
   ```

These examples cover a wide range of use cases, from basic TCP communication to secure WebSocket interactions. Feel free to explore and modify them to suit your needs.

