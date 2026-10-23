/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UDP client transport
*/

#pragma once

#include <array>
#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "game_engine/Network/Transport.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype::network {

class UdpClient : public IClientTransport {
private:
	static constexpr std::size_t MAX_PENDING = 256;
	asio::ip::udp::endpoint resolveServer(const std::string& host, std::uint16_t port);
	void connect(const asio::ip::udp::endpoint& endpoint);

	asio::io_context _context;
	asio::ip::udp::socket _socket;
	std::array<std::uint8_t, MAX_DATAGRAM_SIZE + 1> _receiveBuffer;

public:
	UdpClient(const std::string& host, std::uint16_t port);
	void send(const std::vector<std::uint8_t>& bytes) override;
	std::vector<std::vector<std::uint8_t>> receivePending() override;
};

}
