#include <gridx/matching/common/Version.hpp>

TEST(VersionTest, ReturnsCurrentVersion) {
    EXPECT_EQ(gridx::matching::common::Version::getVersion(), "0.1.0");
}