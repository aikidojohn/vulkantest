// BlockEngineTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace blok;

TEST(testEngine, myTest) {
	World world;
	std::optional<Block> block = world.getBlockAt(0.0f, 0.0f, 0.0f);
	EXPECT_TRUE(block.has_value());
	EXPECT_EQ(1, 1);
}

