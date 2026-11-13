#ifndef SHOE_H
#define SHOE_H

#include <array>

class Shoe {
private:
    // Index = rank * 4 + suit
    // Ranks: 0 = Ace, 1 = 2, ..., 12 = King
    // Suits: 0 = Clubs, 1 = Diamonds,
    //        2 = Hearts, 3 = Spades

    std::array<int, 52> cards;
    int remaining;

public:
    Shoe();

    bool removeCard(int rank, int suit);

    int getCount(int rank, int suit) const;

    int getCardCount(int index) const;

    int getRemaining() const;

    std::array<int, 13> getRankCounts() const;
};

#endif