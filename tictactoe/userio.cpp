#include <stdio.h>  // not sure if y'all meant by "use standard input output" "use stdin/stdout, iostream is ok" or "use stdio"

#include "userio.h"

void UserIOStd::print(const char* outputString) 
{
	printf("%s", outputString);
}

std::string UserIOStd::scan() 
{
	// it's been a while. I don't think this would compile with clang, but trying not
	// to overthink it.
	char inputChars[16];
	scanf_s("%15s", inputChars, (unsigned)_countof(inputChars));
	return std::string(inputChars);
}
