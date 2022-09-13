#pragma once

#include <optional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class IUserIO;

namespace TicTacToe {
	struct Move 
	{
		uint32_t x;
		uint32_t y;
		Move(uint32_t _x, uint32_t _y) : x(_x), y(_y) {}

		bool operator==(Move m2) const { return x == m2.x && y == m2.y; }
	};

	struct RuleSet 
	{
		uint32_t boardWidth = 3;
		uint32_t boardHeight = 3;
		// how many in a row (not a literal board row but row or col or diagonal) you need to win
		int32_t nInARow = 3;  // I like to default to leaving things signed unless not doing so makes life easier (fewer assert checks for example)

		// generally pass Move by value assuming it will go on the stack on 64-bit machines; passing by const & might be a microoptimization that doesn't in fact optimize (but I didn't confirm)
		bool isInBounds(Move move) const;
	};

	class MoveList 
	{
	public:
		MoveList();
		MoveList(const RuleSet& _ruleSet);

		// -1 for nothing, 0 for X, 1 for O 
		int getXorO(Move move) const;

		bool isEmptySquare(Move move) const;
		bool isValid(Move move) const;

		void addMove(Move move);
		void undo();

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

		// This insight didn't come to me right away but implementing it almost as if it was a newspaper article on
		// a Go game, where each square contains the turn its piece was played (or -1 for empty), and X and O
		// can be determined by the modulo 2 of the turn - keeps the history of the moves compact for undo/replay
		// purposes, doesn't lose data (until you undo), and is amenable to searching for wins in O(n) time.
		std::vector<int> turnForCell;

	};

	// I'm a fan of document-view paradigms for games rather than what most games do where they keep cosmetic information (meshes, textures)
	// attached to the same classes of the sim (actors, physics) - in my game Sixty Second Shooter they were entirely separate, I could have rendered the
	// same sim to an ascii console if I so desired. :)
	std::string renderMoveList(const MoveList& moveList);

	// returns the coordinates of the move or nothing if it couldn't parse - does not
	// check if it's a valid move
	std::optional<Move> parseCommand(const std::string& command);

	// Overthinking:
	// While I know that this particular app creates the IO on the stack and a shared_ptr would be safe
	// (even a raw pointer would be safe) if this was the real world we might have to deal with
	// some weird suspend/resume or teardown situations with multiple threads and would want
	// to handle it gracefully, thus the weak_ptr.
	void shallWePlayAGame(std::weak_ptr<IUserIO> userIO);
	// Though I think now it would be better to use a unique_ptr that we return when we're done,
	// like borrowing in Rust - though then there'd be the ergonomic hassle of takeTurn (below) having two things to return.

	void takeTurns(MoveList& previousMoveList, std::weak_ptr<IUserIO> userIO);

	enum class PlayStatus
	{
		InProgress,
		GameOver
	};
	PlayStatus takeTurn(MoveList& previousMoveList, std::weak_ptr<IUserIO> userIO);

}

