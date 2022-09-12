// tictactoe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// todo:
// - handle tie game
// - implement undo
// - optimize

#include <assert.h>

// I've never used ranges before, and in the real world would advise against it - because not many developers know it, so it would
// increase their congitive load when trying to read this code. And it may be a bad idea for this coding test because it might increase your
// cognitive load, too. (Also it's a newfangled thing and it's wise to mistrust newfangled things until other people have discovered and worked
// out the gotchas.)
// But this made it more fun for me: I got to learn something new that I've been wanting to try for some time.
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
	MoveList::MoveList(const RuleSet& _ruleSet) :
		ruleSet(_ruleSet) {}

	// considered having addMove, getNthMove, etc be able to return errors but this is ergonomically less of a hassle
	bool MoveList::isValid(Move move) const {
		return(ruleSet.isInBounds(move) && isEmptySquare(move));
	}

	bool MoveList::isEmptySquare(Move move) const {
		return(ranges::find(moves, move) == moves.end());
	}

	MoveList MoveList::addMove(Move move) const {
		assert(isValid(move));
		MoveList newList = *this;
		newList.moves.push_back(move);
		return newList;
	}

	Move MoveList::getNthMove(size_t n) const {
		assert(n < moves.size());
		return moves[n];
	}

	int MoveList::whoseTurn() const {
		return moves.size() % 2;        // wishlist: n-player game
	}

	optional<Move> MoveList::getValidInput(const string& input) const
	{
		const optional<Move> interimResult = parseCommand(input);
		return interimResult && isValid(interimResult.value()) && ruleSet.isInBounds(interimResult.value())
			? interimResult
			: nullopt;
	}

	BoardState MoveList::toBoardState() const
	{
		BoardState boardState(ruleSet);
		for (int turn = 0; turn < moves.size(); turn++)
		{
			Move move = getNthMove(turn);
			boardState.moveForCell[move.y * ruleSet.boardWidth + move.x] = turn;
		}
		return boardState;
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

	// I thought my general-purpose getWin() function would mean less duplication in the following code
	// but there's still so much, so kind of regretting the getWin()
	optional<int> BoardState::getRowWin() const
	{
		for (uint32_t row = 0; row < ruleSet.boardHeight; row++)
		{
			vector<int> series;
			for (uint32_t col = 0; col < ruleSet.boardWidth; col++)
			{
				series.push_back(getXorO(col, row));
			}
			const int winner = getWin(ruleSet.nInARow, series);
			if (winner >= 0) {
				return optional<int>(winner);
			}
		}
		return nullopt;
	}

	optional<int> BoardState::getColumnWin() const
	{
		for (uint32_t col = 0; col < ruleSet.boardWidth; col++)
		{
			vector<int> series;
			for (uint32_t row = 0; row < ruleSet.boardHeight; row++)
			{
				series.push_back(getXorO(col, row));
			}
			const int winner = getWin(ruleSet.nInARow, series);
			if (winner >= 0) {
				return optional<int>(winner);
			}
		}
		return nullopt;
	}

	optional<int> BoardState::getSWDiagonalWin() const
	{
		for (uint32_t startCol = 0; startCol < ruleSet.boardWidth + ruleSet.boardHeight; startCol++)
		{
			vector<int> series;
			for (uint32_t startRow = 0; startRow < ruleSet.boardHeight; startRow++)
			{
				const uint32_t x = startCol - startRow;
				if (x < ruleSet.boardWidth)
				{
					series.push_back(getXorO(x, startRow));
				}
				else
				{
					break;
				}
			}
			if (series.size() >= ruleSet.nInARow)
			{
				const int winner = getWin(ruleSet.nInARow, series);
				if (winner >= 0) {
					return optional<int>(winner);
				}
			}
		}
		return nullopt;
	}

	optional<int> BoardState::getSEDiagonalWin() const
	{
		for (uint32_t startCol = 0; startCol < ruleSet.boardWidth + ruleSet.boardHeight; startCol++)
		{
			vector<int> series;
			for (uint32_t startRow = 0; startRow < ruleSet.boardHeight; startRow++)
			{
				const uint32_t x = startCol + startRow;
				if (x < ruleSet.boardWidth)
				{
					series.push_back(getXorO(x, startRow));
				}
				else
				{
					break;
				}
			}
			if (series.size() >= ruleSet.nInARow)
			{
				const int winner = getWin(ruleSet.nInARow, series);
				if (winner >= 0) {
					return optional<int>(winner);
				}
			}
		}
		return nullopt;
	}


	// my FP enthusiasm doesn't extend so far that I want to take the trouble to
	// write some kind of reduce/fold for this or do it recursively
	int getWin(int nInARow, const vector<int>& values)
	{
		int lastXorO = -1;
		int counter = 0;
		for (auto xOrO: values)
		{
			if (xOrO == lastXorO)
			{
				counter++;
			}
			else
			{
				lastXorO = xOrO;
				counter = 1;
			}
			if (counter >= nInARow)
			{
				if (lastXorO >= 0)
				{
					assert(counter == nInARow); // otherwise we played too long without detecting victory
					return lastXorO;
				}
			}
		}
		return -1;
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
		MoveList initialMoveList;
		takeTurn(initialMoveList, userIO);
	}

	void takeTurn(const MoveList& previousMoveList, weak_ptr<IUserIO> userIO)
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
				const optional<int> winner = newMoveList.toBoardState().getOverallWin();
				if (winner)
				{
					std::string winMessage = "Player " + to_string(winner.value()) + " wins!\n";
					lockedUserIO->print(winMessage.c_str());
				}
				else
				{
					takeTurn(newMoveList, userIO);
				}
			}
		}
	}

	string renderMoveList(const MoveList& moveList)
	{
		string boardRepresentation((moveList.ruleSet.boardWidth + 1) * moveList.ruleSet.boardHeight, ' ');
		// x's & o's
		for (int turn = 0; turn < moveList.getMovesView().size(); turn++)
		{
			Move move = moveList.getNthMove(turn);
			boardRepresentation[move.y * (moveList.ruleSet.boardWidth + 1) + move.x] = (turn % 2) ? 'O' : 'X';
		}
		// linefeeds
		for (uint32_t line = 1; line <= moveList.ruleSet.boardHeight; line++)
		{
			boardRepresentation[line * (moveList.ruleSet.boardWidth + 1) - 1] = (char)'\n';
		}
		return boardRepresentation;
	}

}