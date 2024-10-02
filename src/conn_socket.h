#ifndef CONN_SOCKET_H
#define CONN_SOCKET_H

#if defined(__cplusplus)
extern "C" {
#endif
int connectWithTimeout(const char* host, int port, int timeout);
void closesocket(int fd);

#if defined(__cplusplus)
}
#endif

#endif // CONN_SOCKET_H