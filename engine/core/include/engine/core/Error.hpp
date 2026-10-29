/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Engine error types
*/

#pragma once

#include <exception>
#include <string>
#include <utility>

namespace engine::core {

class Error : public std::exception {
private:
	std::string _message;

public:
	explicit Error(std::string message) : _message(std::move(message))
	{
	}

	const char* what() const noexcept override
	{
		return _message.c_str();
	}
};

class EcsError : public Error {
public:
	explicit EcsError(const std::string& message) : Error(message)
	{
	}
};

class CoordinatorError : public EcsError {
public:
	explicit CoordinatorError(const std::string& message) : EcsError(message)
	{
	}
};

class ConfigError : public Error {
public:
	explicit ConfigError(const std::string& message) : Error(message)
	{
	}
};

class LoggerError : public Error {
public:
	explicit LoggerError(const std::string& message) : Error(message)
	{
	}
};

}
