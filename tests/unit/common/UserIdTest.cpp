#include <gtest/gtest.h>

#include <type_traits>

#include "gridx/matching/common/UserId.hpp"

using gridx::matching::UserId;

namespace {
constexpr auto kUuid = "550e8400-e29b-41d4-a716-446655440000";
}  // namespace

TEST(UserIdTest, IsTriviallyCopyableForTheHotPath) {
    EXPECT_TRUE(std::is_trivially_copyable_v<UserId>);
    EXPECT_EQ(sizeof(UserId), UserId::kByteCount);
}

TEST(UserIdTest, DefaultIsNil) {
    EXPECT_TRUE(UserId{}.isNil());
    EXPECT_EQ(UserId{}.toString(), "00000000-0000-0000-0000-000000000000");
}

TEST(UserIdTest, RoundTripsCanonicalUuid) {
    const auto userId = UserId::fromString(kUuid);

    ASSERT_TRUE(userId.has_value());
    EXPECT_FALSE(userId->isNil());
    EXPECT_EQ(userId->toString(), kUuid);
}

TEST(UserIdTest, ParsesUpperCaseAndFormatsLowerCase) {
    const auto userId = UserId::fromString("550E8400-E29B-41D4-A716-446655440000");

    ASSERT_TRUE(userId.has_value());
    EXPECT_EQ(userId->toString(), kUuid);
    EXPECT_EQ(*userId, *UserId::fromString(kUuid));
}

TEST(UserIdTest, DistinctUuidsAreNotEqual) {
    EXPECT_NE(*UserId::fromString(kUuid),
              *UserId::fromString("550e8400-e29b-41d4-a716-446655440001"));
}

TEST(UserIdTest, RejectsMalformedInput) {
    EXPECT_FALSE(UserId::fromString("").has_value());
    EXPECT_FALSE(UserId::fromString("202").has_value());
    // Missing dashes.
    EXPECT_FALSE(UserId::fromString("550e8400e29b41d4a716446655440000").has_value());
    // Dash in the wrong place.
    EXPECT_FALSE(UserId::fromString("550e840-0e29b-41d4-a716-446655440000").has_value());
    // Non-hex character.
    EXPECT_FALSE(UserId::fromString("550e8400-e29b-41d4-a716-44665544000g").has_value());
    // Braced form is not accepted.
    EXPECT_FALSE(UserId::fromString("{550e8400-e29b-41d4-a716-446655440000}").has_value());
}

TEST(UserIdTest, ParsesAtCompileTime) {
    constexpr auto userId = UserId::fromString(kUuid);
    static_assert(userId.has_value());
    static_assert(!userId->isNil());
    SUCCEED();
}
