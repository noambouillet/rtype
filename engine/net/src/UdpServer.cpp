/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** UDP asynchronous handling
*/

#include "engine/net/UdpServer.hpp"

#include <algorithm>
#include <utility>

namespace engine::net {

UdpServer::UdpServer(const UdpServerConfig& config)
	: _socket(_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), config.port)), _receiveBuffer{}, _queuedSends(0), _maxDatagramSize(config.maxDatagramSize)
{
	receiveNext();
	_thread = std::thread([this] { _context.run(); });
}

UdpServer::~UdpServer()
{
	_context.stop();
	if (_thread.joinable()) {
		_thread.join();
	}
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
		return;
	}
	asio::post(_context, [this, recipient, bytes = std::move(bytes)] {
		asio::error_code error;
		const asio::ip::address address = asio::ip::make_address(recipient.address, error);
		if (!error) {
			_socket.send_to(asio::buffer(bytes), asio::ip::udp::endpoint(address, recipient.port), 0, error);
		}
		_queuedSends.fetch_sub(1);
	});
}

}
