#include "Labyrinth.h"
#include "set.h"
using namespace std;

bool isPathToFreedom(MazeCell* start, const string& moves) {
    //start with a set that keeps track of picked up items
    //add SPELLBOOK, POTION, WAND to it as we pick them up
    Set<Item> items;

    //create a MazeCell pointer for the current move
    MazeCell* current = start;

    //iterate over the moves and pick up any items along the way
    for (char direction: moves) {
        //if the current cells item is not nothing, add the current cells item to set
        if (current->whatsHere != Item::NOTHING) {
            items.add(current->whatsHere);
        }

        //check the next cell if its nullptr return false
        //else update current to be the next cell
        if (direction == 'N') {
            if (current->north == nullptr) {
                return false;
            }
            current = current->north;
        } else if (direction == 'E') {
            if (current->east == nullptr) {
                return false;
            }
            current = current->east;
        } else if (direction == 'S') {
            if (current->south == nullptr) {
                return false;
            }
            current = current->south;
        } else {
            if (current->west == nullptr) {
                return false;
            }
            current = current->west;
        }
    }

    //account for the last move
    //if the current cells item is not nothing, add the current cells item to set
    if (current->whatsHere != Item::NOTHING) {
        items.add(current->whatsHere);
    }

    //if we found all the items and there were no illegal moves return true
    //else return false
    if (items.size() == 3) {
        return true;
    } else {
        return false;
    }
}


/* * * * * * Test Cases Below This Point * * * * * */
#include "GUI/SimpleTest.h"
#include "Demos/MazeGenerator.h"

/* * * * * Provided Tests Below This Point * * * * */

/* Utility function to free all memory allocated for a maze. */
void deleteMaze(const Grid<MazeCell*>& maze) {
    for (auto* elem: maze) {
        delete elem;
    }
    /* Good question to ponder: why don't we write 'delete maze;'
     * rather than what's shown above?
     */
}

PROVIDED_TEST("Checks paths in the sample maze.") {
    auto maze = toMaze({"* *-W *",
                        "| |   |",
                        "*-* * *",
                        "  | | |",
                        "S *-*-*",
                        "|   | |",
                        "*-*-* P"});

    /* These paths are the ones in the handout. They all work. */
    EXPECT(isPathToFreedom(maze[2][2], "ESNWWNNEWSSESWWN"));
    EXPECT(isPathToFreedom(maze[2][2], "SWWNSEENWNNEWSSEES"));
    EXPECT(isPathToFreedom(maze[2][2], "WNNEWSSESWWNSEENES"));

    /* These paths don't work, since they don't pick up all items. */
    EXPECT(!isPathToFreedom(maze[2][2], "ESNW"));
    EXPECT(!isPathToFreedom(maze[2][2], "SWWN"));
    EXPECT(!isPathToFreedom(maze[2][2], "WNNE"));

    /* These paths don't work, since they aren't legal paths. */
    EXPECT(!isPathToFreedom(maze[2][2], "WW"));
    EXPECT(!isPathToFreedom(maze[2][2], "NN"));
    EXPECT(!isPathToFreedom(maze[2][2], "EE"));
    EXPECT(!isPathToFreedom(maze[2][2], "SS"));

    deleteMaze(maze);
}

PROVIDED_TEST("Can't walk through walls.") {
    auto maze = toMaze({"* S *",
                        "     ",
                        "W * P",
                        "     ",
                        "* * *"});

    EXPECT(!isPathToFreedom(maze[1][1], "WNEES"));
    EXPECT(!isPathToFreedom(maze[1][1], "NWSEE"));
    EXPECT(!isPathToFreedom(maze[1][1], "ENWWS"));
    EXPECT(!isPathToFreedom(maze[1][1], "SWNNEES"));

    deleteMaze(maze);
}

PROVIDED_TEST("Works when starting on an item.") {
    auto maze = toMaze({"P-S-W"});

    EXPECT(isPathToFreedom(maze[0][0], "EE"));
    EXPECT(isPathToFreedom(maze[0][1], "WEE"));
    EXPECT(isPathToFreedom(maze[0][2], "WW"));

    deleteMaze(maze);
}

/* Printer for items. */
ostream& operator<< (ostream& out, Item item) {
    if (item == Item::NOTHING) {
        return out << "Item::NOTHING";
    } else if (item == Item::WAND) {
        return out << "Item::WAND";
    } else if (item == Item::POTION) {
        return out << "Item::POTION";
    } else if (item == Item::SPELLBOOK) {
        return out << "Item::SPELLBOOK";
    } else {
        return out << "<unknown item type>";
    }
}
