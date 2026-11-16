#ifndef SHOE_CONTROLLER_H
#define SHOE_CONTROLLER_H
#include "shoe.h"
#include <utility>
#include <vector>

// Shared input boundary for the manual UI and future card recognition.
class ShoeController {
public:
    explicit ShoeController(int decks = 6);
    bool removeCard(int rank, int suit);
    bool undoLastRemoval();
    void shuffle();
    void configure(int decks);
    const Shoe& getShoe() const { return shoe; }
    int getDecks() const { return decks; }
    const std::vector<std::pair<int, int>>& history() const { return removals; }
private:
    int decks;
    Shoe shoe;
    std::vector<std::pair<int, int>> removals;
};
#endif
