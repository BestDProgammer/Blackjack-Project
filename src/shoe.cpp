#include <array>
#include <cstdint>
#include "shoe.h"


Shoe::Shoe() {
    cards.fill(6);
    remaining = 312;
}

bool Shoe::removeCard(int rank, int suit) {
    if (rank < 0 || rank >= 13 ||
        suit < 0 || suit >= 4)
        return false;

    int index = rank * 4 + suit;

    if (cards[index] == 0)
        return false;

    --cards[index];
    --remaining;

    return true;
}

int Shoe::getCount(int rank, int suit) const {
    return cards.at(rank * 4 + suit);
}

int Shoe::getCardCount(int index) const {
    return cards.at(index);
}

int Shoe::getRemaining() const {
    return remaining;
}

std::array<int, 13> Shoe::getRankCounts() const {
    std::array<int, 13> ranks{};

    for (int rank = 0; rank < 13; ++rank) {
        for (int suit = 0; suit < 4; ++suit) {
            ranks[rank] += cards[rank * 4 + suit];
        }
    }

    return ranks;
}