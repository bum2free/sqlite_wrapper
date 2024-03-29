// Main.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

std::string config_root;

int main(int argc, char** argv) {
	testing::InitGoogleMock(&argc, argv);
	// Runs all tests using Google Test.
	return RUN_ALL_TESTS();
}
