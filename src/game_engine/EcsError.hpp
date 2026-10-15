/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EcsError
*/

#pragma once
#include <exception>
#include <string>

class EcsError : public std::exception {
public:
	explicit EcsError(const std::string& message) : _message(message)
	{
	}
	~EcsError() override = default;
	const char* what() const noexcept override
	{
		return _message.c_str();
	}

private:
	std::string _message;
};

class CoordinatorError : public EcsError {
public:
	explicit CoordinatorError(const std::string& message) : EcsError(message)
	{
	}
};
