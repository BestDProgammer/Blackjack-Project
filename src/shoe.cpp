#include <array>
#include <cstdint>
#include <stdexcept>
#include "shoe.h"


Shoe::Shoe(int decks) {
    if (decks < 1) {
        throw std::invalid_argument(
            "Number of decks must be at least 1."
        );
    }
    cards.fill(decks);
    remaining = decks * 52;
}
Shoe::Shoe() : Shoe(6) {}

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

//get the number of cards of each rank in the shoe
std::array<int, 13> Shoe::getRankCounts() const {
    std::array<int, 13> ranks{};

    for (int rank = 0; rank < 13; ++rank) {
        for (int suit = 0; suit < 4; ++suit) {
            ranks[rank] += cards[rank * 4 + suit];
        }
    }

    return ranks;
}