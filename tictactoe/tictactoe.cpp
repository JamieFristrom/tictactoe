// tictactoe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// todo:
// - handle tie game CHECK
// - fix diagonal detection bug CHECK
// - refactor MoveList + BoardState into one class CHECK
// - remove BoardState (CHECK), rename MoveList->BoardState
// - implement undo
// - back out of FP: take recursion out of takeTurn, stop copying MoveList in tests
// - optimize

#include <assert.h>

// This is actually my first time trying ranges - thought I'd use it for more, basically just used
// its find() shorthand
#include <ranges>

#include "tictactoe.h"
#include "userio.h"

// I'm not a fan of "std::" scattershot through my code - it hurts my eyes a bit - but this is against the coding guidelines at my company
// and if it's against yours too happy to comply, not a hill I'll die on.
using namespace std;


namespace TicTacToe {
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

	MoveList MoveList::addMove(Move move) {
		assert(isValid(move));
		_setCell(move, turn++);
		return *this;
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

	Move MoveList::getNthMove(size_t n) const {
		assert(n < turn);// moves.size());
		auto iterator = ranges::find(turnForCell, n);
		size_t index = iterator - turnForCell.begin();
		int x = index % ruleSet.boardWidth;
		int y = index / ruleSet.boardWidth;
		return Move(x, y);
	}

	int MoveList::whoseTurn() const {
		return turn % 2;        // wishlist: n-player game
	}

	optional<Move> MoveList::getValidInput(const string& input) const
	{
		const optional<Move> interimResult = parseCommand(input);
		return interimResult && isValid(interimResult.value()) && ruleSet.isInBounds(interimResult.value())
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

	// OH. There's a bug here, it can miss some diagonal wins in the lower left when nInARow is
	// less than height.
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

	// -1 for nothing, 0 for X, 1 for O 
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

	//
	// BoardState
	//

	// Initialize two-d array : -1 means nobody has taken a turn there yet.
	BoardState::BoardState(const RuleSet& _ruleSet) :
		ruleSet(_ruleSet),
		moveForCell(_ruleSet.boardWidth * _ruleSet.boardHeight, -1) {}

	// -1 for nothing, 0 for X, 1 for O 
	int BoardState::getXorO(uint32_t x, uint32_t y) const
	{
		const int moveForXY = moveForCell[y * ruleSet.boardWidth + x];
		return (moveForXY > 0) ? moveForXY % 2 : moveForXY;
	}


	// CPU perf wise this is currently an O(n) algorithm where n is the number of squares
	// on the board.
	// If I only checked rays coming out of the latest move it would have to do a much smaller
	// number of checks.
	// This theoretically is safer since it doesn't rely on that assumption...
	optional<int> BoardState::getOverallWin() const
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

	optional<int> BoardState::getRowWin() const
	{
		// go from top to bottom searching rows
		return searchForWinner(0, 0, 0, +1, +1, 0, ruleSet.boardHeight);
	}

	optional<int> BoardState::getColumnWin() const
	{
		// go from left to right searching columns
		return searchForWinner(0, 0, +1, 0, 0, +1, ruleSet.boardWidth);
	}

	optional<int> BoardState::getSWDiagonalWin() const
	{
		// go from left to right searching left-down diagonals
		return searchForWinner(0, 0, +1, 0, -1, +1, ruleSet.boardHeight+ruleSet.boardWidth);
	}

	// OH. There's a bug here, it can miss some diagonal wins in the lower left when nInARow is
	// less than height.
	optional<int> BoardState::getSEDiagonalWin() const
	{
		// go from right to left searching right-down diagonals
		return searchForWinner(ruleSet.boardWidth-1, 0, -1, 0, +1, +1, ruleSet.boardHeight + ruleSet.boardWidth);
	}

	// startX, startY is where we begin our search
	// startingDX/DY is how the starting point for the search for rows moves
	// sweepDX/DY is how it then sweeps
	// for example 0,0 - 0,1 and 1,0 will go from left to right, sweeping downward, checking for adjacent Xs/Os in that column
	optional<int> BoardState::searchForWinner(int startX, int startY, int startingDX, int startingDY, int sweepDX, int sweepDY, int count) const
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
					moveForCell[lineCheckerY * ruleSet.boardWidth + lineCheckerX] % 2 : // conveniently, -1 % 2 is -1 so it does what we want for the 'empty' case
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

	// general functions in the Tic-Tac-Toe namespace
	optional<Move> parseCommand(const string& input) {
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
		takeTurn(initialMoveList, userIO);
	}

	void takeTurn(MoveList& previousMoveList, weak_ptr<IUserIO> userIO)
	{
		auto lockedUserIO = userIO.lock();  // I'm not really a fan of the if( auto lockedUserIO = userIO.lock()) idiom just because it doesn't strike me as 'natural' but if that's popular at Psyonix I'll conform
		if (lockedUserIO)
		{
			std::string outputPrompt = "Player " + to_string(previousMoveList.whoseTurn()) + " enter your move. For example: 0,0 for the top-left corner; 1,2 for the bottom-middle square.\n";
			lockedUserIO->print(outputPrompt.c_str());
			string command = lockedUserIO->scan();
			auto input = previousMoveList.getValidInput(command);
			if (!input)
			{
				lockedUserIO->print("I don't understand that move.\n");
				takeTurn(previousMoveList, userIO);
				// Got cute here and made it recursive, still acting like an FP wonk.
				// This could well be unsafe, not sure if tail-call optimization
				// is going to kick in here, in which case a string
				// of garbage inputs will eventually blow out the stack.
			}
			else
			{
				MoveList newMoveList = previousMoveList.addMove(input.value());
				lockedUserIO->print(renderMoveList(newMoveList).c_str());

				const optional<int> winner = newMoveList.getOverallWin();
				if (winner)
				{
					std::string winMessage = "Player " + to_string(winner.value()) + " wins!\n";
					lockedUserIO->print(winMessage.c_str());
					// recursion ends
				}
				else
				{
					if (newMoveList.isBoardFull())
					{
						lockedUserIO->print("Nobody wins.\n");
						// recursion ends
					}
					else
					{
						takeTurn(newMoveList, userIO);
					}
				}
			}
		}
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