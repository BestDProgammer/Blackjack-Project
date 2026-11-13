
#include "SideBetEV.h"
#include "shoe.h"
#include <array>
#include <stdexcept>
#include <algorithm>
#include <cmath>

OutcomeProbabilities calculateProbabilities21Plus3(const Shoe& shoe) {
    OutcomeProbabilities probabilities;

    int N = shoe.getRemaining();

    if (N < 3) {
        throw std::invalid_argument(
            "Shoe must contain at least three cards."
        );
    }
    //N choose 3
    double totalCombinations = static_cast<double>(N) * (N - 1) * (N - 2) / 6.0;

    // Counts of physical combinations by outcome
    std::array<double, 6> counts{};
    //go through every possible selection of three cards
    for (int i = 0; i < 52; ++i) {
        for (int j = i; j < 52; ++j) {
            for (int k = j; k < 52; ++k) {

                int a = shoe.getCardCount(i);
                int b = shoe.getCardCount(j);
                int c = shoe.getCardCount(k);

                double weight = 0.0;
                //calculate the number of combinations for this card selection
                if (i == j && j == k) {
                    weight = a * (a - 1.0) * (a - 2.0) / 6.0;
                }
                else if (i == j) {
                    weight = a * (a - 1.0) / 2.0 * c;
                }
                else if (j == k) {
                    weight = a * b * (b - 1.0) / 2.0;
                }
                else {
                    weight = a * b * c;
                }

                if (weight <= 0.0)
                    continue;
                //classify the hand and add its weight(number of combinations) to the corresponding count
                HandType type = classifyHand(i, j, k);
                counts[static_cast<int>(type)] += weight;
            }
        }
    }
    //convert the combination counts to probabilities
    probabilities.loss = counts[0] / totalCombinations;
    probabilities.flush = counts[1] / totalCombinations;
    probabilities.straight = counts[2] / totalCombinations;
    probabilities.threeOfAKind = counts[3] / totalCombinations;
    probabilities.straightFlush = counts[4] / totalCombinations;
    probabilities.suitedTrips = counts[5] / totalCombinations;

    return probabilities;
}

HandType classifyHand(int a, int b, int c) {
    int r1 = a / 4, r2 = b / 4, r3 = c / 4;
    int s1 = a % 4, s2 = b % 4, s3 = c % 4;

    bool sameSuit = (s1 == s2 && s2 == s3);
    bool sameRank = (r1 == r2 && r2 == r3);

    std::array<int, 3> ranks = {
        r1 == 0 ? 14 : r1 + 1,
        r2 == 0 ? 14 : r2 + 1,
        r3 == 0 ? 14 : r3 + 1
    };

    std::sort(ranks.begin(), ranks.end());

    bool straight =
        (ranks[0] + 1 == ranks[1] &&
         ranks[1] + 1 == ranks[2]) ||
        (ranks[0] == 2 &&
         ranks[1] == 3 &&
         ranks[2] == 14);

    if (sameRank && sameSuit)
        return HandType::SuitedTrips;

    if (sameSuit && straight)
        return HandType::StraightFlush;

    if (sameRank)
        return HandType::ThreeOfAKind;

    if (sameSuit)
        return HandType::Flush;

    if (straight)
        return HandType::Straight;

    return HandType::Loss;
}
PayoutStructure21Plus3::PayoutStructure21Plus3(
    double flush,
    double straight,
    double threeOfAKind,
    double straightFlush,
    double suitedTrips
) {
    setPayouts(
        flush,
        straight,
        threeOfAKind,
        straightFlush,
        suitedTrips
    );
}

void PayoutStructure21Plus3::setPayouts(
    double flush,
    double straight,
    double threeOfAKind,
    double straightFlush,
    double suitedTrips
) {
    if (!std::isfinite(flush) || flush < 0 ||
        !std::isfinite(straight) || straight < 0 ||
        !std::isfinite(threeOfAKind) || threeOfAKind < 0 ||
        !std::isfinite(straightFlush) || straightFlush < 0 ||
        !std::isfinite(suitedTrips) || suitedTrips < 0) {

        throw std::invalid_argument(
            "Payouts must be finite and nonnegative."
        );
    }

    flushPayout = flush;
    straightPayout = straight;
    threeOfAKindPayout = threeOfAKind;
    straightFlushPayout = straightFlush;
    suitedTripsPayout = suitedTrips;
}

double PayoutStructure21Plus3::getFlushPayout() const {
    return flushPayout;
}

double PayoutStructure21Plus3::getStraightPayout() const {
    return straightPayout;
}

double PayoutStructure21Plus3::getThreeOfAKindPayout() const {
    return threeOfAKindPayout;
}

double PayoutStructure21Plus3::getStraightFlushPayout() const {
    return straightFlushPayout;
}

double PayoutStructure21Plus3::getSuitedTripsPayout() const {
    return suitedTripsPayout;
}

double calculateEV21Plus3(
    const Shoe& shoe,
    const PayoutStructure21Plus3& payouts
) {
    OutcomeProbabilities p =
        calculateProbabilities21Plus3(shoe);

    return p.flush * payouts.getFlushPayout() +
           p.straight * payouts.getStraightPayout() +
           p.threeOfAKind * payouts.getThreeOfAKindPayout() +
           p.straightFlush * payouts.getStraightFlushPayout() +
           p.suitedTrips * payouts.getSuitedTripsPayout() -
           p.loss;
}
    