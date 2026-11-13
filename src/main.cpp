#include <iostream>
#include "SideBetEV.h"
#include "shoe.h"

int main()
{
    Shoe shoe = Shoe();
    OutcomeProbabilities probabilities = calculateProbabilities21Plus3(shoe);
    std::cout << "Loss probability: " << probabilities.loss << std::endl;
    std::cout << "Flush probability: " << probabilities.flush << std::endl;
    std::cout << "Straight probability: " << probabilities.straight << std::endl;
    std::cout << "Three of a Kind probability: " << probabilities.threeOfAKind << std::endl;
    std::cout << "Straight Flush probability: " << probabilities.straightFlush << std::endl;
    std::cout << "Suited Trips probability: " << probabilities.suitedTrips << std::endl;

    double ev = calculateEV21Plus3(shoe);
    std::cout << "Expected value: " << ev << std::endl;
    
    return 0;
}
