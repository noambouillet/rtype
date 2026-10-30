/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** UDP asynchronous handling
*/

#include "engine/net/UdpServer.hpp"

#include <algorithm>
#include <utility>

#include "engine/core/Logger.hpp"

namespace engine::net {

UdpServer::UdpServer(const UdpServerConfig& config)
	: _socket(_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), config.port)), _receiveBuffer{}, _queuedSends(0), _maxDatagramSize(config.maxDatagramSize)
{
	core::Logger::defaultLogger().info("UdpServer: Bound to IPv4 port " + std::to_string(config.port) + " (maxDatagram: " + std::to_string(config.maxDatagramSize) + ")");
	receiveNext();
	_thread = std::thread([this] { _context.run(); });
	core::Logger::defaultLogger().debug("UdpServer: Asynchronous worker thread started");
}

UdpServer::~UdpServer()
{
	core::Logger::defaultLogger().debug("UdpServer: Stopping server socket and worker thread...");
	_context.stop();
	if (_thread.joinable()) {
		_thread.join();
	}
	core::Logger::defaultLogger().debug("UdpServer: Worker thread joined successfully");
}

/// Starts another asynchronous receive operation.
void UdpServer::receiveNext()
{
	_socket.async_receive_from(asio::buffer(_receiveBuffer), _sender, [this](const asio::error_code& error, std::size_t size) {
		if (!error) {
			std::scoped_lock lock(_mutex);
			if (_pending.size() < MAX_PENDING) {
				// Keep one extra byte so the parser can reject oversized datagrams.
				const auto storedSize = std::min(size, _maxDatagramSize + 1);
				_pending.push_back({{.address = _sender.address().to_string(), .port = _sender.port()},
					{_receiveBuffer.begin(), _receiveBuffer.begin() + storedSize}});
			} else {
				core::Logger::defaultLogger().warn("UdpServer: Packet dropped, pending buffer full (" + std::to_string(MAX_PENDING) + ")");
			}
		}
		if (_socket.is_open()) {
			receiveNext();
		}
	});
}

/// Transfers all pending datagrams to the calling game thread.
std::vector<Datagram> UdpServer::receivePending()
{
	std::scoped_lock lock(_mutex);
	std::vector<Datagram> result;

	result.reserve(_pending.size());
	while (!_pending.empty()) {
		result.push_back(std::move(_pending.front()));
		_pending.pop_front();
	}
	return result;
}

/// Schedules one datagram for transmission from the network thread.
void UdpServer::send(const Endpoint& recipient, std::vector<std::uint8_t> bytes)
{
	if (bytes.empty()) {
		return;
	}
	if (_queuedSends.fetch_add(1) >= MAX_QUEUED_SENDS) {
		_queuedSends.fetch_sub(1);
		core::Logger::defaultLogger().warn("UdpServer: Packet dropped to " + recipient.address + ":" + std::to_string(recipient.port) + ", send queue full");
		return;
	}
	asio::post(_context, [this, recipient, bytes = std::move(bytes)] {
		asio::error_code error;
		const asio::ip::address address = asio::ip::make_address(recipient.address, error);
		if (!error) {
			_socket.send_to(asio::buffer(bytes), asio::ip::udp::endpoint(address, recipient.port), 0, error);
			if (error) {
				core::Logger::defaultLogger().warn("UdpServer: Failed to send datagram to " + recipient.address + ":" + std::to_string(recipient.port) + ": " + error.message());
			}
		} else {
			core::Logger::defaultLogger().warn("UdpServer: Invalid address format for recipient: " + recipient.address);
		}
		_queuedSends.fetch_sub(1);
	});
}

}
