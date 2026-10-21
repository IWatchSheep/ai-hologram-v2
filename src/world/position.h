// Purpose: Defines grid positions and compares them.
// Introduced in: Stage 1a (rebuild)
// Architecture: Positions are shared by the world model and its
// helpers. A position is a plain value with no behaviour of its
// own, so later code can copy and compare positions freely.

#ifndef AI_HOLOGRAM_SRC_WORLD_POSITION_H
#define AI_HOLOGRAM_SRC_WORLD_POSITION_H

namespace aih {

// A struct groups related values together. Unlike a class, its
// members are public by default, which suits a simple value like
// this: x is the column (0 to 9, left to right) and y is the row
// (0 to 9, top to bottom).
struct Position {
    int x;
    int y;
};

// Operator overloading gives == a meaning for Position: two
// positions are equal when both coordinates are equal. There is
// no position.cpp (the plan names none), so these small operators
// are defined here; constexpr also permits compile-time use.
constexpr bool operator==(const Position& left, const Position& right) {
    return left.x == right.x && left.y == right.y;
}

// The != operator is defined in terms of ==, so the two can
// never disagree with each other.
constexpr bool operator!=(const Position& left, const Position& right) {
    return !(left == right);
}

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_WORLD_POSITION_H
