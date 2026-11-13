#ifndef SIDEBETEV_H
#define SIDEBETEV_H
#include "shoe.h"


enum class HandType {
    Loss,
    Flush,
    Straight,
    ThreeOfAKind,
    StraightFlush,
    SuitedTrips
};

struct OutcomeProbabilities {
    double loss = 0.0;
    double flush = 0.0;
    double straight = 0.0;
    double threeOfAKind = 0.0;
    double straightFlush = 0.0;
    double suitedTrips = 0.0;
};

HandType classifyHand(int card1, int card2, int card3);

OutcomeProbabilities calculateProbabilities21Plus3(const Shoe& shoe);

double calculateEV21Plus3(const Shoe& shoe);
#endif