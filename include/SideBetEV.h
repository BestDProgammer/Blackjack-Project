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

class PayoutStructure21Plus3 {
private:
    double flushPayout;
    double straightPayout;
    double threeOfAKindPayout;
    double straightFlushPayout;
    double suitedTripsPayout;

public:
    // Defaults to 9:1 for every winning outcome
    PayoutStructure21Plus3(
        double flush = 9.0,
        double straight = 9.0,
        double threeOfAKind = 9.0,
        double straightFlush = 9.0,
        double suitedTrips = 9.0
    );

    void setPayouts(
        double flush,
        double straight,
        double threeOfAKind,
        double straightFlush,
        double suitedTrips
    );

    double getFlushPayout() const;
    double getStraightPayout() const;
    double getThreeOfAKindPayout() const;
    double getStraightFlushPayout() const;
    double getSuitedTripsPayout() const;
};


double calculateEV21Plus3(const Shoe& shoe, const PayoutStructure21Plus3& payouts = PayoutStructure21Plus3());
#endif