/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** UDP server transport
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

namespace engine::net {

class UdpServer : public IServerTransport {
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
	std::deque<Datagram> _pending;
	std::atomic<std::size_t> _queuedSends;
	std::size_t _maxDatagramSize;
	std::thread _thread;

public:
	UdpServer(std::uint16_t port, std::size_t maxDatagramSize);
	~UdpServer();
	UdpServer(const UdpServer&) = delete;
	UdpServer& operator=(const UdpServer&) = delete;

	std::vector<Datagram> receivePending() override;
	void send(const Endpoint& recipient, std::vector<std::uint8_t> bytes) override;
};

}
