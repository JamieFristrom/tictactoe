#include "pch.h"

#include <optional>

#include "../tictactoe/tictactoe.h"
#include "../tictactoe/userio.h"

using namespace TicTacToe;
using namespace std;


TEST(MoveListTests, 4moves_xsAndOs)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(2, 2));
	moveList.addMove(Move(1, 1));
	moveList.addMove(Move(1, 0));
	EXPECT_EQ(0, moveList.getXorO(Move(0, 0)));
	EXPECT_EQ(1, moveList.getXorO(Move(2, 2)));
	EXPECT_EQ(0, moveList.getXorO(Move(1, 1)));
	EXPECT_EQ(1, moveList.getXorO(Move(1, 0)));
	EXPECT_EQ(-1, moveList.getXorO(Move(2, 0)));
}

TEST(MoveListTests, emptyBoard_getXorO_neither)
{
	MoveList board(RuleSet(3, 3, 3));
	EXPECT_EQ(-1, board.getXorO(Move(0, 0)));
	EXPECT_EQ(-1, board.getXorO(Move(2, 2)));
}

TEST(MoveListTests, horizontalWin_player0)
{
	MoveList moveList;
	moveList.addMove(Move(0, 1));
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(1, 1));
	moveList.addMove(Move(1, 0));
	moveList.addMove(Move(2, 1));
	EXPECT_EQ(0, moveList.getOverallWin());
}

TEST(MoveListTests, verticalWin_player0)
{
	MoveList moveList;
	moveList.addMove(Move(2, 0));
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(2, 1));
	moveList.addMove(Move(1, 0));
	moveList.addMove(Move(2, 2));
	EXPECT_EQ(0, moveList.getOverallWin());
}

TEST(MoveListTests, diagonalSEWin_5x5board_player1)
{
	MoveList moveList(RuleSet(5, 5, 3));
	moveList.addMove(Move(2, 0));
	moveList.addMove(Move(2, 1));
	moveList.addMove(Move(3, 0));
	moveList.addMove(Move(3, 2));
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(4, 3));
	EXPECT_EQ(1, moveList.getOverallWin());
}

TEST(MoveListTests, diagonalSWWin_5x5board_player1)
{
	MoveList moveList(RuleSet(5, 5, 3));
	moveList.addMove(Move(2, 0));
	moveList.addMove(Move(2, 1));
	moveList.addMove(Move(3, 0));
	moveList.addMove(Move(1, 2));
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(0, 3));
	EXPECT_EQ(1, moveList.getOverallWin());
}

TEST(MoveListTests, diagonalSEWin_5x5board_lowerleft_player0)
{
	MoveList moveList(RuleSet(5, 5, 3));
	moveList.addMove(Move(0, 1));
	moveList.addMove(Move(4, 4));
	moveList.addMove(Move(1, 2));
	moveList.addMove(Move(4, 2));
	moveList.addMove(Move(2, 3));
	EXPECT_EQ(0, moveList.getOverallWin());
}

TEST(MoveListTests, MoveList_LegitMoveToEmptyBoard_isValid) 
{
	MoveList moveList;

	EXPECT_TRUE(moveList.isValid(Move(0, 0)));
}

TEST(MoveListTests, MoveList_SameMoveTwice_isInvalid)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));
	EXPECT_FALSE(moveList.isValid(Move(0, 0)));
}

TEST(MoveListTests, MoveList_firstAddMove_moveThere)
{
	MoveList origingalList;
	origingalList.addMove(Move(0, 0));
	EXPECT_EQ(0, origingalList.getXorO(Move(0, 0)));
}

// Some would argue these should be separate tests and that we should put tests around isOutOfBounds and isEmptySquare
// as well - but the really important thing is that the code is covered
TEST(MoveListTests, MoveList_OutOfBounds_isInvalid)
{
	MoveList moveList;
	// default MoveList bounds are 3x3
	EXPECT_FALSE(moveList.isValid(Move(4, 0)));
	EXPECT_FALSE(moveList.isValid(Move(0, 4)));
}

TEST(MoveListTests, getValidInput_negativeIntegers_returnsnullopt)
{
	MoveList moveList;
	EXPECT_FALSE(moveList.getValidInput("0,-1"));
}

TEST(MoveListTests, getValidInput_oobIntegers_returnsnullopt)
{
	MoveList moveList;
	EXPECT_FALSE(moveList.getValidInput("4,0"));
}

TEST(MoveListTests, getValidInput_goodIntegers_returnsValidMove)
{
	MoveList moveList;
	EXPECT_EQ(Move(2, 2), moveList.getValidInput("2,2").value());
}
// In real life we'd organize by putting 

TEST(MoveListTests, MoveList_whoseTurn_alternates)
{
	MoveList moveList;
	EXPECT_EQ(0, moveList.whoseTurn());
	moveList.addMove(Move(0, 0));  // auto is another thing that's mostly banned where I work - I generally favor it but another hill I won't die on
	EXPECT_EQ(1, moveList.whoseTurn());
	moveList.addMove(Move(0, 1));
	EXPECT_EQ(0, moveList.whoseTurn());
	moveList.addMove(Move(0, 2));
	EXPECT_EQ(1, moveList.whoseTurn());
	moveList.addMove(Move(1, 0));
	EXPECT_EQ(0, moveList.whoseTurn());            
	// one of the downsides of FP, above - it's easy to mess up the #s at the ends of those moveLists
}

TEST(MoveListTests, undo_undoes)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(0, 1));
	moveList.undo();
	EXPECT_EQ(-1, moveList.getXorO(Move(0, 1)));
	EXPECT_EQ(1, moveList.getTurn());
}

TEST(MoveListTests, undo_dont_undo_too_far)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(0, 1));
	moveList.undo();
	moveList.undo();
	moveList.undo();
	EXPECT_EQ(-1, moveList.getXorO(Move(0, 1)));
	EXPECT_EQ(-1, moveList.getXorO(Move(0, 0)));
	EXPECT_EQ(0, moveList.getTurn());
}

TEST(TicTacToeTests, renderMoveList_empty)
{
	MoveList moveList;
	string boardRep = renderMoveList(moveList);
	EXPECT_EQ("   \n   \n   \n", boardRep);
}

TEST(TicTacToeTests, renderMoves)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));  
	string boardRepresentation = renderMoveList(moveList);
	EXPECT_EQ("X  \n   \n   \n", boardRepresentation);
	moveList.addMove(Move(0, 1));
	boardRepresentation = renderMoveList(moveList);
	EXPECT_EQ("X  \nO  \n   \n", boardRepresentation);
	moveList.addMove(Move(0, 2));
	boardRepresentation = renderMoveList(moveList);
	EXPECT_EQ("X  \nO  \nX  \n", boardRepresentation);
	moveList.addMove(Move(2, 2));
	boardRepresentation = renderMoveList(moveList);
	EXPECT_EQ("X  \nO  \nX O\n", boardRepresentation);
}

TEST(TicTacToeTests, parseCommand_Garbage_returnsnullopt)
{
	optional<Move> result = parseCommand("garbage");
	EXPECT_TRUE(!result);
}

TEST(TicTacToeTests, parseCommand_integers_returnsValidMove)
{
	optional<Move> result = parseCommand("5,5");
	EXPECT_EQ(Move(5, 5), result.value());
}




// and now the integration test. In real life probably a separate file, particularly for the mock
// BTW, I'm pretty good at using fakeit but this particular class is so simple that that would be
// overkill.
class UserIOMock : public IUserIO 
{
public:
	void print(const char* outputString) override 
	{
		outputStrings.push_back(outputString);
	}
	string scan() override 
	{
		return inputStrings[turn++];
	}
	int turn = 0;
	vector<string> outputStrings;
	vector<string> inputStrings;
};

TEST(TicTacToeTests, takeTurn_CatsGame_noWinner)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(0, 1));
	moveList.addMove(Move(1, 0));
	moveList.addMove(Move(1, 1));
	moveList.addMove(Move(2, 1)); // block
	moveList.addMove(Move(2, 0)); // block
	moveList.addMove(Move(0, 2)); 
	moveList.addMove(Move(1, 2));

	auto sharedUserIOMock = make_shared<UserIOMock>();
	sharedUserIOMock->inputStrings.push_back("2,2");
	PlayStatus result = takeTurn(moveList, sharedUserIOMock);
	EXPECT_EQ(PlayStatus::GameOver, result);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[0]);
	EXPECT_EQ("XXO\nOOX\nXOX\n", sharedUserIOMock->outputStrings[1]);
	EXPECT_EQ("Nobody wins.\n", sharedUserIOMock->outputStrings[2]);
}

TEST(TicTacToeTests, takeTurn_winOnLastMove)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(0, 1));
	moveList.addMove(Move(1, 1));
	moveList.addMove(Move(1, 0));
	moveList.addMove(Move(2, 1));
	moveList.addMove(Move(2, 0));
	moveList.addMove(Move(0, 2));
	moveList.addMove(Move(1, 2));

	auto sharedUserIOMock = make_shared<UserIOMock>();
	sharedUserIOMock->inputStrings.push_back("2,2");
	PlayStatus result = takeTurn(moveList, sharedUserIOMock);
	EXPECT_EQ(PlayStatus::GameOver, result);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[0]);
	EXPECT_EQ("XOO\nOXX\nXOX\n", sharedUserIOMock->outputStrings[1]);
	EXPECT_EQ("Player 0 wins!\n", sharedUserIOMock->outputStrings[2]);
}

TEST(TicTacToeTests, takeTurn_undo_works)
{
	MoveList moveList;
	moveList.addMove(Move(0, 0));
	moveList.addMove(Move(0, 1));
	moveList.addMove(Move(1, 1));

	auto sharedUserIOMock = make_shared<UserIOMock>();
	sharedUserIOMock->inputStrings.push_back("u");
	PlayStatus result = takeTurn(moveList, sharedUserIOMock);
	EXPECT_EQ(PlayStatus::InProgress, result);
	EXPECT_EQ("Player 1 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[0]);
	EXPECT_EQ(2, moveList.getTurn());
	EXPECT_EQ(-1, moveList.getXorO(Move(1, 1)));
}

TEST(TicTacToeTests, shallWePlayAGame_fullGameIntegrationTest)
{
	auto sharedUserIOMock = make_shared<UserIOMock>();
	sharedUserIOMock->inputStrings.push_back("0,3");
	sharedUserIOMock->inputStrings.push_back("0,0");
	sharedUserIOMock->inputStrings.push_back("1,0");
	sharedUserIOMock->inputStrings.push_back("1,1");
	sharedUserIOMock->inputStrings.push_back("0,1");
	sharedUserIOMock->inputStrings.push_back("2,2");
	shallWePlayAGame(sharedUserIOMock);
	EXPECT_EQ("Shall we play a game?\n", sharedUserIOMock->outputStrings[0]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[1]);
	EXPECT_EQ("I don't understand that move.\n", sharedUserIOMock->outputStrings[2]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[3]);
	EXPECT_EQ("X  \n   \n   \n", sharedUserIOMock->outputStrings[4]);
	EXPECT_EQ("Player 1 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[5]);
	EXPECT_EQ("XO \n   \n   \n", sharedUserIOMock->outputStrings[6]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[7]);
	EXPECT_EQ("XO \n X \n   \n", sharedUserIOMock->outputStrings[8]);
	EXPECT_EQ("Player 1 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[9]);
	EXPECT_EQ("XO \nOX \n   \n", sharedUserIOMock->outputStrings[10]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[11]);
	EXPECT_EQ("XO \nOX \n  X\n", sharedUserIOMock->outputStrings[12]);
	EXPECT_EQ("Player 0 wins!\n", sharedUserIOMock->outputStrings[13]);
}

TEST(TicTacToeTests, shallWePlayAGame_fullGameIntegrationTestWithUndoes)
{
	auto sharedUserIOMock = make_shared<UserIOMock>();
	sharedUserIOMock->inputStrings.push_back("0,0");
	sharedUserIOMock->inputStrings.push_back("u");
	sharedUserIOMock->inputStrings.push_back("undo");
	sharedUserIOMock->inputStrings.push_back("0,0");
	sharedUserIOMock->inputStrings.push_back("1,0");
	sharedUserIOMock->inputStrings.push_back("1,1");
	sharedUserIOMock->inputStrings.push_back("u");
	sharedUserIOMock->inputStrings.push_back("1,1");
	sharedUserIOMock->inputStrings.push_back("0,1");
	sharedUserIOMock->inputStrings.push_back("2,2");
	shallWePlayAGame(sharedUserIOMock);
	EXPECT_EQ("Shall we play a game?\n", sharedUserIOMock->outputStrings[0]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[1]);
	EXPECT_EQ("X  \n   \n   \n", sharedUserIOMock->outputStrings[2]);
	EXPECT_EQ("Player 1 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[3]);
	EXPECT_EQ("   \n   \n   \n", sharedUserIOMock->outputStrings[4]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[5]);
	EXPECT_EQ("   \n   \n   \n", sharedUserIOMock->outputStrings[6]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[7]);
	EXPECT_EQ("X  \n   \n   \n", sharedUserIOMock->outputStrings[8]);
	EXPECT_EQ("Player 1 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[9]);
	EXPECT_EQ("XO \n   \n   \n", sharedUserIOMock->outputStrings[10]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[11]);
	EXPECT_EQ("XO \n X \n   \n", sharedUserIOMock->outputStrings[12]);
	EXPECT_EQ("Player 1 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[13]);
	EXPECT_EQ("XO \n   \n   \n", sharedUserIOMock->outputStrings[14]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[15]);
	EXPECT_EQ("XO \n X \n   \n", sharedUserIOMock->outputStrings[16]);
	EXPECT_EQ("Player 1 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[17]);
	EXPECT_EQ("XO \nOX \n   \n", sharedUserIOMock->outputStrings[18]);
	EXPECT_EQ("Player 0 enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[19]);
	EXPECT_EQ("XO \nOX \n  X\n", sharedUserIOMock->outputStrings[20]);
	EXPECT_EQ("Player 0 wins!\n", sharedUserIOMock->outputStrings[21]);
}
