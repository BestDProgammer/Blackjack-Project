#include <iomanip>
#include <iostream>
#include "SideBetEV.h"
#include "shoe.h"

int main() {
    Shoe shoe(6);

    // Default: 9:1 on everything
    PayoutStructure21Plus3 configuration1;

    // Alternative paytable
    PayoutStructure21Plus3 configuration2(
        5, 10, 25, 40, 100
    );

    std::cout << std::fixed
              << std::setprecision(6);

    std::cout << "Configuration 1 EV: "
              << calculateEV21Plus3(shoe, configuration1)
              << '\n';

    std::cout << "Configuration 2 EV: "
              << calculateEV21Plus3(shoe, configuration2)
              << '\n';

    // Modify configuration 2
    configuration2.setPayouts(5, 10, 20, 30, 20);

    std::cout << "Updated Configuration 2 EV: "
              << calculateEV21Plus3(shoe, configuration2)
              << '\n';

    return 0;
}

