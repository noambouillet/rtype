/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UDP client transport implementation
*/

#include "UdpClient.hpp"

#include "utils/RTypeError.hpp"

namespace rtype::network {

/// Resolves the server and connects a UDP socket so only its packets are read.
UdpClient::UdpClient(const std::string& host, std::uint16_t port) : _socket(_context), _receiveBuffer{}
{
	connect(resolveServer(host, port));
}

/// Resolves the server name, preferring IPv4 for local servers.
asio::ip::udp::endpoint UdpClient::resolveServer(const std::string& host, std::uint16_t port)
{
	asio::ip::udp::resolver resolver(_context);
	asio::error_code error;
	const auto endpoints = resolver.resolve(host, std::to_string(port), error);
	if (error) {
		throw NetworkError("Cannot resolve server: " + error.message());
	}
	if (endpoints.empty()) {
		throw NetworkError("No UDP address found for server");
	}
	asio::ip::udp::endpoint endpoint = endpoints.begin()->endpoint();
	for (const auto& entry : endpoints) {
		if (entry.endpoint().address().is_v4()) {
			endpoint = entry.endpoint();
			break;
		}
	}
	return endpoint;
}

/// Opens a nonblocking UDP socket connected to the selected server address.
void UdpClient::connect(const asio::ip::udp::endpoint& endpoint)
{
	try {
		_socket.open(endpoint.protocol());
		_socket.connect(endpoint);
		_socket.non_blocking(true);
	} catch (const asio::system_error& error) {
		throw NetworkError("Cannot open UDP client socket: " + std::string(error.what()));
	}
}

/// Sends a validated protocol packet to the connected server.
void UdpClient::send(const std::vector<std::uint8_t>& bytes)
{
	if (bytes.empty() || bytes.size() > MAX_DATAGRAM_SIZE) {
		return;
	}
	asio::error_code error;
	_socket.send(asio::buffer(bytes), 0, error);
	if (error && error != asio::error::connection_refused &&
		error != asio::error::would_block && error != asio::error::try_again) {
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
			throw NetworkError("UDP receive failed: " + error.message());
		}
		result.emplace_back(_receiveBuffer.begin(), _receiveBuffer.begin() + size);
	}
	return result;
}

}
