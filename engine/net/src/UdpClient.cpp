/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** UDP client transport implementation
*/

#include "engine/net/UdpClient.hpp"

#include "engine/core/Logger.hpp"
#include "engine/net/NetworkError.hpp"

namespace engine::net {

/// Resolves the server and connects a UDP socket so only its packets are read.
UdpClient::UdpClient(const UdpClientConfig& config)
	: _socket(_context), _maxDatagramSize(config.maxDatagramSize), _receiveBuffer(config.maxDatagramSize + 1)
{
	core::Logger::defaultLogger().debug("UdpClient: Resolving server " + config.host + ':' + std::to_string(config.port));
	connect(resolveServer(config.host, config.port));
}

/// Resolves the server name, preferring IPv4 for local servers.
asio::ip::udp::endpoint UdpClient::resolveServer(const std::string& host, std::uint16_t port)
{
	asio::ip::udp::resolver resolver(_context);
	asio::error_code error;
	const auto endpoints = resolver.resolve(host, std::to_string(port), error);
	if (error) {
		core::Logger::defaultLogger().error("UdpClient: Cannot resolve server " + host + ": " + error.message());
		throw NetworkError("Cannot resolve server: " + error.message());
	}
	if (endpoints.empty()) {
		core::Logger::defaultLogger().error("UdpClient: No UDP address found for server " + host);
		throw NetworkError("No UDP address found for server");
	}
	asio::ip::udp::endpoint endpoint = endpoints.begin()->endpoint();
	for (const auto& entry : endpoints) {
		if (entry.endpoint().address().is_v4()) {
			endpoint = entry.endpoint();
			break;
		}
	}
	core::Logger::defaultLogger().debug("UdpClient: Resolved server to " + endpoint.address().to_string() + ':' + std::to_string(endpoint.port()));
	return endpoint;
}

/// Opens a nonblocking UDP socket connected to the selected server address.
void UdpClient::connect(const asio::ip::udp::endpoint& endpoint)
{
	try {
		core::Logger::defaultLogger().debug("UdpClient: Connecting socket to " + endpoint.address().to_string() + ':' + std::to_string(endpoint.port()));
		_socket.open(endpoint.protocol());
		_socket.connect(endpoint);
		_socket.non_blocking(true);
		core::Logger::defaultLogger().debug("UdpClient: Connected socket opened in non-blocking mode");
	} catch (const asio::system_error& error) {
		core::Logger::defaultLogger().error("UdpClient: Cannot open UDP client socket: " + std::string(error.what()));
		throw NetworkError("Cannot open UDP client socket: " + std::string(error.what()));
	}
}

/// Sends a validated protocol packet to the connected server.
void UdpClient::send(const std::vector<std::uint8_t>& bytes)
{
	if (bytes.empty() || bytes.size() > _maxDatagramSize) {
		if (bytes.size() > _maxDatagramSize) {
			core::Logger::defaultLogger().warn("UdpClient: Packet dropped, size " + std::to_string(bytes.size()) +
											   " exceeds max datagram size " + std::to_string(_maxDatagramSize));
		}
		return;
	}
	asio::error_code error;
	_socket.send(asio::buffer(bytes), 0, error);
	if (error && error != asio::error::connection_refused &&
		error != asio::error::would_block && error != asio::error::try_again) {
		core::Logger::defaultLogger().error("UdpClient: UDP send failed: " + error.message());
		throw NetworkError("UDP send failed: " + error.message());
	}
}

/// Reads at most one frame's bounded batch without blocking rendering.
std::vector<std::vector<std::uint8_t>> UdpClient::receivePending()
{
	std::vector<std::vector<std::uint8_t>> result;
	result.reserve(MAX_PENDING);
	for (std::size_t index = 0; index < MAX_PENDING; index++) {
		asio::error_code error;
		const std::size_t size = _socket.receive(asio::buffer(_receiveBuffer), 0, error);
		if (error == asio::error::would_block || error == asio::error::try_again ||
			error == asio::error::connection_refused) {
			break;
		}
		if (error) {
			core::Logger::defaultLogger().error("UdpClient: UDP receive failed: " + error.message());
			throw NetworkError("UDP receive failed: " + error.message());
		}
		result.emplace_back(_receiveBuffer.begin(), _receiveBuffer.begin() + static_cast<std::ptrdiff_t>(size));
	}
	return result;
}

}
