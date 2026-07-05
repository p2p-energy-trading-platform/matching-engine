#include <iostream>

#include <gridx/matching/common/Version.hpp>

int main()
{
    std::cout << "GridX Matching Engine "
              << gridx::matching::common::Version::getVersion()
              << std::endl;

    return 0;
}