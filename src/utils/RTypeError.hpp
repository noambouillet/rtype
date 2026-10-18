/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Project error types
*/

#pragma once

#include <exception>
#include <string>
#include <utility>

namespace rtype {

class RTypeError : public std::exception {
private:
	std::string _message;

public:
	explicit RTypeError(std::string message) : _message(std::move(message))
	{
	}

	const char* what() const noexcept override
	{
		return _message.c_str();
	}
};

class LoggerError : public RTypeError {
public:
	explicit LoggerError(const std::string& message) : RTypeError(message)
	{
	}
};

class NetworkError : public RTypeError {
public:
	explicit NetworkError(const std::string& message) : RTypeError(message)
	{
	}
};

class GraphicsError : public RTypeError {
public:
	explicit GraphicsError(const std::string& message) : RTypeError(message)
	{
	}
};

class AudioError : public RTypeError {
public:
	explicit AudioError(const std::string& message) : RTypeError(message)
	{
	}
};

class ConfigError : public RTypeError {
public:
	explicit ConfigError(const std::string& message) : RTypeError(message)
	{
	}
};

}
