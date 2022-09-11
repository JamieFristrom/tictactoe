#include "pch.h"

#include <optional>

#include "../tictactoe/tictactoe.h"
#include "../tictactoe/userio.h"

using namespace TicTacToe;
using namespace std;

TEST(MoveListTests, MoveList_LegitMoveToEmptyBoard_isValid) 
{
	MoveList moveList;

	EXPECT_TRUE(moveList.isValid(Move(0, 0)));
}

TEST(MoveListTests, MoveList_SameMoveTwice_isInvalid)
{
	MoveList moveList;
	MoveList newList = moveList.addMove(Move(0, 0));
	EXPECT_FALSE(newList.isValid(Move(0, 0)));
}

TEST(MoveListTests, MoveList_firstAddMove_moveThere)
{
	MoveList origingalList;
	MoveList newList = origingalList.addMove(Move(0, 0));
	EXPECT_TRUE(newList.getNthMove(0) == Move(0, 0));
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
	EXPECT_FALSE(moveList.getValidInput("0 -1"));
}

TEST(MoveListTests, getValidInput_oobIntegers_returnsnullopt)
{
	MoveList moveList;
	EXPECT_FALSE(moveList.getValidInput("4 0"));
}

TEST(MoveListTests, getValidInput_goodIntegers_returnsValidMove)
{
	MoveList moveList;
	EXPECT_EQ(Move(2, 2), moveList.getValidInput("2 2").value());
}
// In real life we'd organize by putting 

TEST(MoveListTests, MoveList_whoseTurn_alternates)
{
	MoveList moveList;
	EXPECT_EQ(0, moveList.whoseTurn());
	auto moveList2 = moveList.addMove(Move(0, 0));  // auto is another thing that's mostly banned where I work - I generally favor it but another hill I won't die on
	EXPECT_EQ(1, moveList2.whoseTurn());
	auto moveList3 = moveList2.addMove(Move(0, 1));
	EXPECT_EQ(0, moveList3.whoseTurn());
	auto moveList4 = moveList3.addMove(Move(0, 2));
	EXPECT_EQ(1, moveList4.whoseTurn());
	auto moveList5 = moveList4.addMove(Move(1, 0));
	EXPECT_EQ(0, moveList5.whoseTurn());            
	// one of the downsides of FP, above - it's easy to mess up the #s at the ends of those moveLists
}

TEST(renderMoveListTests, renderMoveList_empty)
{
	MoveList moveList;
	string boardRep = renderMoveList(moveList);
	EXPECT_EQ("   \n   \n   \n", boardRep);
}

TEST(renderMoveListTests, renderMoves)
{
	MoveList moveList;
	auto moveList2 = moveList.addMove(Move(0, 0));  
	string boardRepresentation = renderMoveList(moveList2);
	EXPECT_EQ("X  \n   \n   \n", boardRepresentation);
	auto moveList3 = moveList2.addMove(Move(0, 1));
	boardRepresentation = renderMoveList(moveList3);
	EXPECT_EQ("X  \nO  \n   \n", boardRepresentation);
	auto moveList4 = moveList3.addMove(Move(0, 2));
	boardRepresentation = renderMoveList(moveList4);
	EXPECT_EQ("X  \nO  \nX  \n", boardRepresentation);
	auto moveList5 = moveList4.addMove(Move(2, 2));
	boardRepresentation = renderMoveList(moveList5);
	EXPECT_EQ("X  \nO  \nX O\n", boardRepresentation);
}

TEST(parseCommandTests, parseCommand_Garbage_returnsnullopt)
{
	optional<Move> result = parseCommand("garbage");
	EXPECT_TRUE(!result);
}

TEST(parseCommandTests, parseCommand_integers_returnsValidMove)
{
	optional<Move> result = parseCommand("5 5");
	EXPECT_EQ(Move(5, 5), result.value());
}




// and now the integration test. In real life probably a separate file, particularly for the mock
// BTW, I'm pretty good at using fakeit but this particular class is so simple that that would be
// overkill.
class UserIOMock : public IUserIO {
public:
	void print(const char* outputString) override {
		outputStrings.push_back(outputString);
	}
	string scan() override {
		return inputStrings[turn++];
	}
	int turn = 0;
	vector<string> outputStrings;
	vector<string> inputStrings;
};

TEST(shallWePlayAGameTest, shallWePlayAGame)
{
	auto sharedUserIOMock = make_shared<UserIOMock>();
	sharedUserIOMock->inputStrings.push_back("0 3");
	sharedUserIOMock->inputStrings.push_back("0 0");
	shallWePlayAGame(sharedUserIOMock);
	EXPECT_EQ("Shall we play a game?\n", sharedUserIOMock->outputStrings[0]);
	EXPECT_EQ("Player 0 enter your move. For example: 0 0 for the top-left corner; 1 2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[1]);
	EXPECT_EQ("I don't understand that move.\n", sharedUserIOMock->outputStrings[2]);
	EXPECT_EQ("Player 0 enter your move. For example: 0 0 for the top-left corner; 1 2 for the bottom-middle square.\n", sharedUserIOMock->outputStrings[3]);
	EXPECT_EQ("X  \n   \n   \n", sharedUserIOMock->outputStrings[4]);
}