#include "Data.h"
using namespace std;

// A square can be entered if it is anything except a wall.
bool isWalkable(CellType cellType) {
    return cellType != CellType::Wall;
}
 