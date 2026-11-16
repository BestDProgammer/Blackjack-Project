#include "ShoeController.h"
#include "SideBetEV.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    ShoeController c(6);
    require(c.getShoe().getRemaining() == 312, "Fresh shoe size");
    require(!c.removeCard(-1, 0) && !c.removeCard(0, 4), "Reject invalid cards");
    for (int i = 0; i < 6; ++i) require(c.removeCard(0, 0), "Remove available copy");
    require(!c.removeCard(0, 0), "Prevent negative inventory");
    require(c.removeCard(12, 3), "Remove last card");
    require(c.undoLastRemoval() && c.getShoe().getCount(12, 3) == 6, "Undo is chronological");
    require(c.getShoe().getCount(0, 0) == 0, "Undo preserves earlier removals");
    c.shuffle();
    require(c.getShoe().getRemaining() == 312 && c.history().empty(), "Shuffle resets history");
    require(!c.undoLastRemoval(), "Undo empty history");
    try { c.configure(0); require(false, "Invalid deck count accepted"); } catch (const std::invalid_argument&) {}
    require(c.getDecks() == 6, "Invalid configuration preserves shoe");
    double fresh = calculateEV21Plus3(c.getShoe());
    require(std::abs(fresh + 0.032386) < 0.000001, "Six-deck flat paytable benchmark");
    c.removeCard(0, 0);
    require(std::abs(calculateEV21Plus3(c.getShoe()) - fresh) > 1e-9, "Composition changes EV");
    c.undoLastRemoval();
    require(std::abs(calculateEV21Plus3(c.getShoe()) - fresh) < 1e-12, "Undo restores EV");
    for (int decks : {1, 2, 6, 8, 16}) {
        c.configure(decks);
        auto p = calculateProbabilities21Plus3(c.getShoe());
        require(std::abs(p.loss + p.flush + p.straight + p.threeOfAKind + p.straightFlush + p.suitedTrips - 1) < 1e-12, "Probabilities sum to one");
    }
    c.configure(1);
    for (int rank = 0; rank < 13; ++rank) for (int suit = 0; suit < 4; ++suit)
        if (!(suit == 0 && rank < 3)) c.removeCard(rank, suit);
    require(calculateProbabilities21Plus3(c.getShoe()).straightFlush == 1, "A-2-3 suited terminal shoe");
    require(calculateEV21Plus3(c.getShoe()) == 9, "Certain win uses net payout");
    c.removeCard(0, 0);
    try { calculateEV21Plus3(c.getShoe()); require(false, "Accepted fewer than three cards"); } catch (const std::invalid_argument&) {}
    std::cout << "All core tests passed\n";
}
