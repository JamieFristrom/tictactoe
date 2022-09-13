//
// Hello Psyonix!
//
// I know you told me not to overthink this but couldn't help myself. I wanted to show off my automated testing skills,
// give googletest a whirl (I'm much more familiar with Microsoft's framework but it's not as Switch/PS4 friendly),
// and experiment with a mostly pure FP approach - which I later abandoned.
// It's actually been really fun to work on: it's been months since I coded for pure pleasure with short build times,
// and it's been a reminder why I enjoy test-first development so much - refactoring without fear.
// 
// This supports the full m x n x k case but you have to tweak the code to do it: see shallWePlayAGame, line 242.
// 
// I think what I ended up with here has most of the good traits of an FP program (care with sources of truth,
// not losing history of data) without the downsides (perf, space, risk of blowing out the stack.)
// If you want to see my process or the early FP implementation I threw it up on my github - https://github.com/JamieFristrom/tictactoe
// Things I'm particularly proud of here include the test framework (check Test->Test Explorer) and the
// internal representation of the game state (see comment by turnForCell in tictactoe.h.)

#include <assert.h>

#include <algorithm>

#include "tictactoe.h"
#include "userio.h"

// I'm not a fan of "std::" scattershot through my code - it hurts my eyes a bit - but this is against the coding guidelines at my company
// and if it's against yours too happy to comply, not a hill I'll die on.
using namespace std;


namespace TicTacToe {

	const Move UndoMove(0xffffffff, 0xffffffff);

	//
	// RuleSet
	//
	bool RuleSet::isInBounds(Move move) const {
		return(move.x < boardWidth&& move.y < boardHeight);
	}

	//
	// MoveList
	//
	MoveList::MoveList() :
		turnForCell(ruleSet.boardWidth*ruleSet.boardHeight, -1) {}

	MoveList::MoveList(const RuleSet& _ruleSet) :
		ruleSet(_ruleSet),
		turnForCell(_ruleSet.boardWidth*_ruleSet.boardHeight, -1) {}

	// considered having addMove, getNthMove, etc be able to return errors but this is ergonomically less of a hassle
	bool MoveList::isValid(Move move) const 
	{
		return(ruleSet.isInBounds(move) && isEmptySquare(move));
	}

	bool MoveList::isEmptySquare(Move move) const 
	{
		return _getCell(move) == -1;
	}

	void MoveList::addMove(Move move) {
		assert(isValid(move));
		_setCell(move, turn++);
	}

	void MoveList::undo() {
		if (turn > 0)
		{
			turn--;

			// O(n), it could be O(k) if I store the last move instead of the last turn #
			// this feels less likely to have bugs later though
			replace(turnForCell.begin(), turnForCell.end(), turn, -1);
		}
	}

	void MoveList::_setCell(Move move, int turn)
	{
		// on which turn was an x or o placed in that cell
		turnForCell[move.y * ruleSet.boardWidth + move.x] = turn;
	}

	int MoveList::_getCell(Move move) const
	{
		return turnForCell[move.y * ruleSet.boardWidth + move.x];
	}

	int MoveList::whoseTurn() const {
		return turn % 2;        // wishlist: n-player game
	}

	optional<Move> MoveList::getValidInput(const string& input) const
	{
		const optional<Move> interimResult = parseCommand(input);
		return interimResult 
				&& ((interimResult.value()==UndoMove) || (isValid(interimResult.value()) && ruleSet.isInBounds(interimResult.value())))
			? interimResult
			: nullopt;
	}

	// CPU perf wise this is currently an O(n) algorithm where n is the number of squares
	// on the board.
	// If I only checked rays coming out of the latest move it would have to do a much smaller
	// number of checks.
	// But this theoretically is safer since it doesn't rely on that assumption...
	optional<int> MoveList::getOverallWin() const
	{
		const auto rowWinner = getRowWin();
		if (rowWinner)
			return rowWinner;

		const auto columnWinner = getColumnWin();
		if (columnWinner)
			return columnWinner;

		const auto swDiagWinner = getSWDiagonalWin();
		if (swDiagWinner)
			return swDiagWinner;

		return getSEDiagonalWin();
	}

	optional<int> MoveList::getRowWin() const
	{
		// go from top to bottom searching rows
		return searchForWinner(0, 0, 0, +1, +1, 0, ruleSet.boardHeight);
	}

	optional<int> MoveList::getColumnWin() const
	{
		// go from left to right searching columns
		return searchForWinner(0, 0, +1, 0, 0, +1, ruleSet.boardWidth);
	}

	optional<int> MoveList::getSWDiagonalWin() const
	{
		// go from left to right searching left-down diagonals
		return searchForWinner(0, 0, +1, 0, -1, +1, ruleSet.boardHeight+ruleSet.boardWidth);
	}

	optional<int> MoveList::getSEDiagonalWin() const
	{
		// go from right to left searching right-down diagonals
		return searchForWinner(ruleSet.boardWidth-1, 0, -1, 0, +1, +1, ruleSet.boardHeight + ruleSet.boardWidth);
	}

	// startX, startY is where we begin our search
	// startingDX/DY is how the starting point for the search for rows moves
	// sweepDX/DY is how it then sweeps
	// for example 0,0 - 0,1 and 1,0 will go from left to right, sweeping downward, checking for adjacent Xs/Os in that column
	optional<int> MoveList::searchForWinner(int startX, int startY, int startingDX, int startingDY, int sweepDX, int sweepDY, int count) const
	{
		// mostly these asserts are for documentation purposes
		assert(abs(startingDX) <= 1);
		assert(abs(startingDY) <= 1);
		assert(abs(sweepDX) <= 1);
		assert(abs(sweepDY) <= 1);
		assert(startY >= 0);
		assert(startY < (int)ruleSet.boardHeight);
		// we can start "off the board" for diagonal sweeps:
		assert(startX > -((int)ruleSet.boardHeight));
		assert(startX < (int)(ruleSet.boardWidth + ruleSet.boardHeight));

		int lineBeginX = startX;
		int lineBeginY = startY;
		// iterate through lines
		for (int curLine=0; curLine<count; curLine++)
		{
			// travel along line
			int counter = 0;
			int lastXorO = -1;
			int lineCheckerX = lineBeginX;
			int lineCheckerY = lineBeginY;

			// if we're not travelling in y we're travelling in x so check against width, otherwise check against height:
			for (;(sweepDY==0)?(lineCheckerX < (int)ruleSet.boardWidth):(lineCheckerY < (int)ruleSet.boardHeight);)
			{
				int xOrO = (lineCheckerX >= 0 && lineCheckerX < (int)ruleSet.boardWidth) ?
					turnForCell[lineCheckerY * ruleSet.boardWidth + lineCheckerX] % 2 : // conveniently, -1 % 2 is -1 so it does what we want for the 'empty' case
					-1;
				if (lastXorO == xOrO)
				{
					counter++;
					if (counter >= ruleSet.nInARow) 
					{
						if (xOrO != -1)
						{
							assert(counter <= ruleSet.nInARow);
							return optional<int>(xOrO);
						}
					}
				}
				else
				{
					counter = 1;
					lastXorO = xOrO;
				}
				lineCheckerX += sweepDX;
				lineCheckerY += sweepDY;
			}
			lineBeginX += startingDX;
			lineBeginY += startingDY;
		}
		return nullopt;
	}

	// -1 for nothing, 0 for X (because player 0), 1 for O 
	int MoveList::getXorO(Move move) const
	{
		const int turnForXY = _getCell(move);
		return (turnForXY > 0) ? turnForXY % 2 : turnForXY;
	}

	bool MoveList::isBoardFull() const
	{
		// makes the assumption that no invalid moves have been made
		assert(turn <= (int)(ruleSet.boardWidth * ruleSet.boardHeight));
		return turn >= (int)(ruleSet.boardWidth * ruleSet.boardHeight);
	}

	// general functions in the Tic-Tac-Toe namespace
	optional<Move> parseCommand(const string& input) {
		if (input.c_str()[0] == 'u')
		{
			return optional(UndoMove);
		}
		uint32_t input1 = numeric_limits<uint32_t>::max();
		uint32_t input2 = numeric_limits<uint32_t>::max();
		int count = sscanf_s(input.c_str(), "%u,%u", &input1, &input2);
		return (count == 2) ? optional(Move(input1, input2)) : nullopt;
	}

	void shallWePlayAGame(weak_ptr<IUserIO> userIO)
	{
		auto lockedUserIO = userIO.lock();
		assert(lockedUserIO);  // if it's already invalid that's wack
		lockedUserIO->print("Shall we play a game?\n");
		MoveList initialMoveList(RuleSet(3, 3, 3));
		takeTurns(initialMoveList, userIO);
	}

	void takeTurns(MoveList& moveList, weak_ptr<IUserIO> userIO)
	{
		for (PlayStatus playStatus = PlayStatus::InProgress; playStatus != PlayStatus::GameOver;)
		{
			playStatus = takeTurn(moveList, userIO);
		}
	}

	PlayStatus takeTurn(MoveList& moveList, weak_ptr<IUserIO> userIO)
	{
		auto lockedUserIO = userIO.lock();  // I'm not really a fan of the if( auto lockedUserIO = userIO.lock()) idiom just because it doesn't strike me as 'natural' but if that's popular at Psyonix I'll conform
		if (lockedUserIO)
		{
			std::string outputPrompt = "Player " + to_string(moveList.whoseTurn()) + " enter your move or 'undo'. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n";
			lockedUserIO->print(outputPrompt.c_str());
			string command = lockedUserIO->scan();
			auto input = moveList.getValidInput(command);
			if (!input)
			{
				lockedUserIO->print("I don't understand that move.\n");
				return PlayStatus::InProgress;
			}
			else if (input == UndoMove)
			{
				moveList.undo();
				lockedUserIO->print(renderMoveList(moveList).c_str());
				return PlayStatus::InProgress;
			}
			else
			{
				moveList.addMove(input.value());
				lockedUserIO->print(renderMoveList(moveList).c_str());

				const optional<int> winner = moveList.getOverallWin();
				if (winner)
				{
					std::string winMessage = "Player " + to_string(winner.value()) + " wins!\n";
					lockedUserIO->print(winMessage.c_str());
					return PlayStatus::GameOver;
				}
				else
				{
					if (moveList.isBoardFull())
					{
						lockedUserIO->print("Nobody wins.\n");
						return PlayStatus::GameOver;
					}
					else
					{
						return PlayStatus::InProgress;
					}
				}
			}
		}
		return PlayStatus::GameOver;
	}

	string renderMoveList(const MoveList& moveList)
	{
		string boardRepresentation((moveList.ruleSet.boardWidth + 1) * moveList.ruleSet.boardHeight, ' ');
		// x's & o's
		for (uint32_t y = 0; y < moveList.ruleSet.boardHeight; y++)
		{
			for (uint32_t x = 0; x < moveList.ruleSet.boardWidth; x++)
			{
				const int xOrO = moveList.getXorO(Move(x, y));
				boardRepresentation[y * (moveList.ruleSet.boardWidth + 1) + x] =
					(xOrO == -1) ? ' ' : (xOrO == 0) ? 'X' : 'O';
			}
		}
		// linefeeds
		for (uint32_t line = 1; line <= moveList.ruleSet.boardHeight; line++)
		{
			boardRepresentation[line * (moveList.ruleSet.boardWidth + 1) - 1] = (char)'\n';
		}
		return boardRepresentation;
	}

}