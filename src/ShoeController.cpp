#include "ShoeController.h"

ShoeController::ShoeController(int decks) : decks(decks), shoe(decks) {}

bool ShoeController::removeCard(int rank, int suit) {
    if (!shoe.removeCard(rank, suit)) return false;
    removals.emplace_back(rank, suit);
    return true;
}

bool ShoeController::undoLastRemoval() {
    if (removals.empty()) return false;
    removals.pop_back();
    shoe = Shoe(decks);
    for (auto [rank, suit] : removals) shoe.removeCard(rank, suit);
    return true;
}

void ShoeController::shuffle() {
    shoe = Shoe(decks);
    removals.clear();
}

void ShoeController::configure(int count) {
    Shoe fresh(count);
    shoe = fresh;
    decks = count;
    removals.clear();
}
