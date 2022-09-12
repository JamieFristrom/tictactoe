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
		Move(int _x, int _y) : x(_x), y(_y) {}
		
		bool operator==(Move m2) const { return x == m2.x && y == m2.y; }
	};

	struct RuleSet {
		uint32_t boardWidth = 3;
		uint32_t boardHeight = 3;
		// how many in a row (not a literal board row but row, col, diagonal) you need to win
		int32_t nInARow = 3;  // I like to default to leaving things signed unless not doing so makes life easier (fewer assert checks for example)

		bool isInBounds(Move move) const;
	};

	// alternate representation for gamestate that works like Go game notation to make
	// some algorithms more convenient
	// So even numbers are player 0, odd numbers are player 1
	class BoardState
	{
	public:
		BoardState(const RuleSet& _ruleSet);
		int getXorO(uint32_t x, uint32_t y) const;

		std::optional<int> getOverallWin() const;
	private:
		std::optional<int> getRowWin() const;
		std::optional<int> getColumnWin() const;
		std::optional<int> getSEDiagonalWin() const;
		std::optional<int> getSWDiagonalWin() const;

		const RuleSet ruleSet;
		std::vector<int> moveForCell;

		friend class MoveList;
	};

	class MoveList {
	public:
		MoveList() {}
		MoveList(const RuleSet& _ruleSet);

		bool isEmptySquare(Move move) const;
		bool isValid(Move move) const;
		MoveList addMove(Move move);
		void undo();
		Move getNthMove(size_t n) const;
		bool isBoardFull() const;

		const std::vector<Move>& getMovesView() const { return moves; }
		std::optional<Move> getValidInput(const std::string& input) const;

		int whoseTurn() const;
		BoardState toBoardState() const;

		const RuleSet ruleSet;
	private:
		std::vector<Move> moves;
		
	};

	// pass in a series of values extracted from a row, column, or diagonal;
	// returns 0 if player 0 wins, 1 if player 1 wins, or -1 if nobody has n-in-a-row
	int getWin(int nInARow, const std::vector<int>& values);

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

	// Got cute here and made it recursive, still being an FP wonk.
	void takeTurn(MoveList& previousMoveList, std::weak_ptr<IUserIO> userIO);

}

