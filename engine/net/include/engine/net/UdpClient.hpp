/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** UDP client transport
*/

#pragma once

#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "engine/net/Transport.hpp"

namespace engine::net {

class UdpClient : public IClientTransport {
private:
	static constexpr std::size_t MAX_PENDING = 256;
	asio::ip::udp::endpoint resolveServer(const std::string& host, std::uint16_t port);
	void connect(const asio::ip::udp::endpoint& endpoint);

	asio::io_context _context;
	asio::ip::udp::socket _socket;
	std::size_t _maxDatagramSize;
	std::vector<std::uint8_t> _receiveBuffer;

public:
	UdpClient(const std::string& host, std::uint16_t port, std::size_t maxDatagramSize);
	void send(const std::vector<std::uint8_t>& bytes) override;
	std::vector<std::vector<std::uint8_t>> receivePending() override;
};

}
