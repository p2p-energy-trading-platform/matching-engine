#pragma once

#include "gridx/matching/domain/Order.hpp"
#include "gridx/matching/validation/ValidationResult.hpp"

namespace gridx::matching::validation
{

class OrderValidator
{
public:
    ValidationResult validate(const Order& order) const;
};

} // namespace gridx::matching::validation