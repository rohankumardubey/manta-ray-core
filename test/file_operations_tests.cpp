#include <pch.h>

#include "utilities.h"

#include "../include/path.h"

using namespace manta;

TEST(FileOperationsTests, PathSanityCheck) {
    Path path1("./");
    Path path2("./");

    EXPECT_TRUE(path1 == path2);
}

TEST(FileOperationsTests, ComplexPathCheck) {
    Path path1(SDL_TEST_FILES "../sdl/basic_bsdf_test.mr");
    Path path2(SDL_TEST_FILES "basic_bsdf_test.mr");

    EXPECT_TRUE(path1 == path2);
}

TEST(FileOperationsTests, AbsolutePathTest) {
    Path path1("C://");
    Path path2(SDL_TEST_FILES "basic_bsdf_test.mr");

    EXPECT_TRUE(path1.isAbsolute());
    EXPECT_FALSE(path2.isAbsolute());
}

TEST(FileOperationsTests, PathConcatenationTest) {
    Path path1(SDL_TEST_FILES);
    Path path2("../sdl/basic_bsdf_test.mr");
    Path path3(SDL_TEST_FILES "../sdl/basic_bsdf_test.mr");
    Path path4 = path1.append(path2);

    EXPECT_TRUE(path3 == path4);
}
