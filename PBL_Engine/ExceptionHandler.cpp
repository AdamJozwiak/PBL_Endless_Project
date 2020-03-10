#include "ExceptionHandler.h"
#include <sstream>

ExceptionHandler::ExceptionHandler(int line, const char* file) noexcept : 
	line(line), file(file)
{}

const char* ExceptionHandler::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* ExceptionHandler::GetType() const noexcept
{
	return "Exception";
}

int ExceptionHandler::GetLine() const noexcept
{
	return line;
}

const std::string& ExceptionHandler::GetFile() const noexcept
{
	return file;
}

std::string ExceptionHandler::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "File: " << file << std::endl << "Line: " << line;
	return oss.str();
}