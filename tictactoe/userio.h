#pragma once

#include <string>

// wrapping stdio so we can mock in tests
class IUserIO
{
public:
	virtual void print(const char* outputString) = 0;
	virtual std::string scan() = 0;
};

class UserIOStd : public IUserIO
{
public:
	void print(const char* outputString) override;
	std::string scan() override;
};
