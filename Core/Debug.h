#pragma once
#include <iostream>
#include <sstream>

struct Debug
{
private:
	static void SendMessage(std::string message)
	{
		std::cout << message << std::endl;
	}
public:
	Debug()
	{

	}

	static void Log(std::string message)
	{
		std::stringstream ss;
		ss << message;
		SendMessage(ss.str());
	}

	static void Warning(std::string message)
	{
		std::stringstream ss;
		ss << "Warning: " << message;
		SendMessage(ss.str());
	}

	static void Error(std::string message)
	{
		std::stringstream ss;
		ss << "Error: " << message;
		SendMessage(ss.str());
	}
};