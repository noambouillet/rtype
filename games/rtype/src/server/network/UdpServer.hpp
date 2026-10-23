/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UDP Server class definition
*/

#pragma once

#include <array>
#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "engine/net/Transport.hpp"

namespace rtype::network {

class UdpServer : public engine::net::IServerTransport {
private:
	static constexpr std::size_t MAX_PENDING = 256;
	static constexpr std::size_t MAX_QUEUED_SENDS = 1024;
	static constexpr std::size_t MAX_UDP_DATAGRAM_SIZE = 65535;

	void receiveNext();

	asio::io_context _context;
	asio::ip::udp::socket _socket;
	asio::ip::udp::endpoint _sender;
	std::array<std::uint8_t, MAX_UDP_DATAGRAM_SIZE> _receiveBuffer;
	std::mutex _mutex;
	std::deque<engine::net::Datagram> _pending;
	std::atomic<std::size_t> _queuedSends;
	std::thread _thread;

public:
	explicit UdpServer(std::uint16_t port);
	~UdpServer();
	UdpServer(const UdpServer&) = delete;
	UdpServer& operator=(const UdpServer&) = delete;

	std::vector<engine::net::Datagram> receivePending() override;
	void send(const engine::net::Endpoint& recipient, std::vector<std::uint8_t> bytes) override;
};

}
