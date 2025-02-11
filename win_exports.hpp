//
// Created by Ty Qualters on 2/10/2025.
//

#ifndef WIN_EXPORTS_HPP
#define WIN_EXPORTS_HPP

#include <sol/sol.hpp>
#include <fmt/color.h>

#define WIN32_LEAN_AND_MEAN
#include <stdexcept>
#include <Windows.h>
#include <winsock2.h>

extern void InitNetworking();
extern sol::optional<SOCKET> CreateTCPSocket(bool ipv6);
extern sol::optional<SOCKET> CreateUDPSocket(bool ipv6);
extern sockaddr_in CreateSocketAddress(std::string addr, bool ipv6, int port);
extern bool ConnectToSocket(SOCKET sock, sockaddr_in& server);
extern bool WriteToSocket(SOCKET socket, std::string data);
extern void CloseSocket(SOCKET socket);
extern void DestroyNetworking();

#endif //WIN_EXPORTS_HPP
