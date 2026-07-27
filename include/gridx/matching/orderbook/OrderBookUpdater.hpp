#pragma once

#include "gridx/matching/domain/MatchingResult.hpp"
#include "gridx/matching/orderbook/ZoneOrderBook.hpp"

namespace gridx::matching::orderbook {


class OrderBookUpdater {
public:
    /*  
    * Applies the matching result to the order book.
      * This includes updating the resting orders and inserting the incoming order if it was not fully matched.
      * 
      * @param matchingResult The result of the matching process, containing trades, updated orders, and the incoming order to insert.
      * @param zoneOrderBook The order book to which the matching result will be applied.
    */
    void apply(
        const MatchingResult& matchingResult,
        ZoneOrderBook& zoneOrderBook) const;
};

}  // namespace gridx::matching::orderbook