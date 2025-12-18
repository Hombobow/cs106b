/* This file includes the function solve that takes in a puzzle of Puzzle class and
 * a vector of tiles of Tile class and returns true if the puzzle has a valid solution and false otherwise
 * it also fills the reference puzzle with the correct solution of tiles.
 */

#include "puzzle-solve.h"
#include "Puzzle.h"
#include "PuzzleGUI.h"
#include "SimpleTest.h"

using namespace std;

void tileMatch() {
    Puzzle puzzle;
    Vector<Tile> tiles;
    Action action;

    loadPuzzleConfig("puzzles/turtles/turtles.txt", puzzle, tiles);
    updateDisplay(puzzle, tiles);

    do {
        action = playInteractive(puzzle, tiles);
        if (action == LOAD_NEW) {
            string configFile = chooseFileDialog();
            loadPuzzleConfig(configFile, puzzle, tiles);
            updateDisplay(puzzle, tiles);
        } else if (action == RUN_SOLVE) {
            bool success = solve(puzzle, tiles);
            cout << "Found solution to puzzle? " << boolalpha << success << endl;
            updateDisplay(puzzle, tiles);
        }
    } while (action != QUIT);
}

/* The solve function takes a reference to a puzzle of Puzzle class and a
 * vector of tiles that fit into the puzzle and returns true if a valid solution can be found
 * and also fills the reference to the puzzle with the correct solution.
 */
bool solve(Puzzle& puzzle, Vector<Tile>& tileVec) {

    for (int startTile = 0; startTile < tileVec.size(); startTile++) {
        for (int rotationNum = 0; rotationNum < 4; rotationNum++) {
            Tile currentTile = tileVec[startTile];
            if (puzzle.canAdd(tileVec[startTile])) {
                //choose
                puzzle.add(currentTile);
                updateDisplay(puzzle, tileVec, 1);
                tileVec.remove(startTile);

                //explore
                solve(puzzle, tileVec);

                //if we solve the puzzle return true
                if (puzzle.isFull()) {
                    return true;
                }

                //unchoose
                puzzle.remove();
                tileVec.insert(startTile, currentTile);
            }
            tileVec[startTile].rotate();
            // updateDisplay(puzzle, tileVec, 1);
        }
    }
    return false;
}
