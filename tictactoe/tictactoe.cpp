// tictactoe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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

	// general functions in the Tic-Tac-Toe namespace
	optional<Move> parseCommand(const string& input) {
		uint32_t input1 = numeric_limits<uint32_t>::max();
		uint32_t input2 = numeric_limits<uint32_t>::max();
		int count = sscanf_s(input.c_str(), "%u %u", &input1, &input2);
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
			std::string outputPrompt = "Player " + std::to_string(previousMoveList.whoseTurn()) + " enter your move. For example: 0 0 for the top-left corner; 1 2 for the bottom-middle square.\n";
			lockedUserIO->print(outputPrompt.c_str());
			string command = lockedUserIO->scan();
			MoveList moveList;
			auto input = moveList.getValidInput(command);
			if (!input)
			{
				lockedUserIO->print("I don't understand that move.\n");
				takeTurn(previousMoveList, userIO);
				// Got cute here and made it recursive, still being an FP wonk.
				// This could well be incorrect, not sure if tail-call optimization
				// is going to kick in for this one, in which case a string
				// of garbage inputs will eventually blow out the stack.
			}
			else
			{
				MoveList newMoveList = moveList.addMove(input.value());
				lockedUserIO->print(renderMoveList(newMoveList).c_str());
				//takeTurn(newMoveList, userIO);
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