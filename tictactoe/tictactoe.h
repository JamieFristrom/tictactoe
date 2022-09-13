#pragma once

#include <optional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class IUserIO;

// Overthinking:
// the capitalization of filenames doesn't match the classnames because in general I like intercaps and camel case but there's a chance we might want to build for
// other OS's / compilers in the future in which case filename capitalization can become a gotcha

namespace TicTacToe {
	// generally passing Move by value assuming it will go on the stack on 64-bit machines; const & might be a microoptimization that doesn't in fact optimize
	struct Move {
		uint32_t x;
		uint32_t y;
		Move(uint32_t _x, uint32_t _y) : x(_x), y(_y) {}
		
		bool operator==(Move m2) const { return x == m2.x && y == m2.y; }
	};

	struct RuleSet {
		uint32_t boardWidth = 3;
		uint32_t boardHeight = 3;
		// how many in a row (not a literal board row but row, col, diagonal) you need to win
		int32_t nInARow = 3;  // I like to default to leaving things signed unless not doing so makes life easier (fewer assert checks for example)

		bool isInBounds(Move move) const;
	};

	class MoveList {
	public:
		MoveList();
		MoveList(const RuleSet& _ruleSet);

		// -1 for nothing, 0 for X, 1 for O 
		int getXorO(Move move) const;

		bool isEmptySquare(Move move) const;
		bool isValid(Move move) const;
		void addMove(Move move);
		void undo();
		Move getNthMove(size_t n) const;
		bool isBoardFull() const;

		std::optional<Move> getValidInput(const std::string& input) const;

		int whoseTurn() const;
		int getTurn() const { return turn; }

		const RuleSet ruleSet;
		std::optional<int> getOverallWin() const;

	private:
		std::optional<int> getRowWin() const;
		std::optional<int> getColumnWin() const;
		std::optional<int> getSEDiagonalWin() const;
		std::optional<int> getSWDiagonalWin() const;
		std::optional<int> searchForWinner(int startX, int startY, int startingDX, int startingDY, int sweepDX, int sweepDY, int count) const;

		void _setCell(Move move, int turn);
		int _getCell(Move move) const;

		// This is duplication of data-two sources of the same truth-since we could find the current turn by taking max()
		// of the board and add 1... but y'all asked me to optimize so doing it this way
		int turn = 0;
		std::vector<int> turnForCell;

	};

	// I'm a fan of document-view paradigms for games rather than what most games do where they keep cosmetic information (meshes, textures)
	// attached to the same classes of the sim (actors, physics) - in my game Sixty Second Shooter they were entirely separate, I could have rendered the
	// same sim to an ascii console if I so desired. :)
	std::string renderMoveList(const MoveList& moveList);

	// returns the coordinates of the move or nothing if it couldn't parse - does not
	// check if it's a valid move
	std::optional<Move> parseCommand(const std::string& command);

	// More overthinking:
	// While I know that this particular app creates the IO on the stack and shared_ptr is safe
	// (even a raw pointer would be safe) if this was the real world we might have to deal with
	// some weird suspend/resume or teardown situations with multiple threads and would want
	// to handle it gracefully, thus the weak_ptr.
	void shallWePlayAGame(std::weak_ptr<IUserIO> userIO);
	// Though I think now it would be better to use a unique_ptr that we return when we're done,
	// like borrowing in Rust.

	void takeTurns(MoveList& previousMoveList, std::weak_ptr<IUserIO> userIO);
	bool takeTurn(MoveList& previousMoveList, std::weak_ptr<IUserIO> userIO);

}

