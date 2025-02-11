//
// Created by Ty Qualters on 2/10/2025.
//

#include "win_exports.hpp"
#pragma comment(lib, "ws2_32.lib")

static WSADATA wsa;

void PrintLastError() {
    int error = WSAGetLastError();

    // Allocate a buffer to hold the error message
    char errorMessage[512];

    // Use FormatMessage to get a human-readable error message
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        errorMessage,
        sizeof(errorMessage),
        NULL
    );


    fmt::println(stderr, "WSA Error Code: {}", error);
    fmt::println(stderr, "Error Message: {}", errorMessage);
}

extern void InitNetworking() {
    int res = WSAStartup(MAKEWORD(2,2), &wsa);
    if(res != 0) {
        throw std::runtime_error("Failed to initialize Winsock2");
    }
}

extern sol::optional<SOCKET> CreateTCPSocket(bool ipv6) {
    SOCKET sock = socket(ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET) {
        fmt::println(stderr, "Failed to create a socket");
        PrintLastError();
        return sol::nullopt;
    }
    return sock;
}

extern sol::optional<SOCKET> CreateUDPSocket(bool ipv6) {
    SOCKET sock = socket(ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_SOCKET) {
        fmt::println(stderr, "Failed to create a socket");
        PrintLastError();
        return sol::nullopt;
    }
    return sock;
}

extern sockaddr_in CreateSocketAddress(std::string addr, bool ipv6, int port) {
    sockaddr_in server{};
    server.sin_addr.s_addr = inet_addr(addr.c_str());
    server.sin_family = ipv6 ? AF_INET6 : AF_INET;
    server.sin_port = htons(port);
    return server;
}

extern bool ConnectToSocket(SOCKET sock, sockaddr_in& server) {
    int res = connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(sockaddr));
    if(res < 0) {
        fmt::println(stderr, "Failed to connect to socket");
        PrintLastError();
        return false;
    }
    return true;
}

extern bool WriteToSocket(SOCKET sock, std::string data) {
    int res = send(sock, data.c_str(), data.size(), 0);
    if(res < 0) {
        fmt::println(stderr, "Failed to connect to socket");
        PrintLastError();
        return false;
    }
    return true;
}

extern void CloseSocket(SOCKET socket) {
    closesocket(socket);
}

extern void DestroyNetworking() {
    WSACleanup();
}