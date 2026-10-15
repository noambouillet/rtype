/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Network transport interfaces
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace rtype::network {

struct Endpoint {
	std::string address;
	std::uint16_t port;

	bool operator==(const Endpoint& other) const = default;
};

struct Datagram {
	Endpoint sender;
	std::vector<std::uint8_t> bytes;
};

class IClientTransport {
public:
	virtual ~IClientTransport() = default;
	virtual void send(const std::vector<std::uint8_t>& bytes) = 0;
	virtual std::vector<std::vector<std::uint8_t>> receivePending() = 0;
};

class IServerTransport {
public:
	virtual ~IServerTransport() = default;
	virtual void send(const Endpoint& recipient, std::vector<std::uint8_t> bytes) = 0;
	virtual std::vector<Datagram> receivePending() = 0;
};

}
