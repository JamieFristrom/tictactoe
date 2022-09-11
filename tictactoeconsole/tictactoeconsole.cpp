// tictactoeconsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>  // not sure if y'all meant by "use standard input output" "use stdin/stdout, iostream is ok" or "use stdio"

#include "../tictactoe/tictactoe.h"
#include "../tictactoe/userio.h"

int main()
{
    printf("Hello Psyonix.\n");
    auto userIO = std::make_shared<UserIOStd>();
    TicTacToe::shallWePlayAGame(userIO);
}

