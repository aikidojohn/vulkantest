// BlockEngineTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../VulkanTest/World.h"
#include "../VulkanTest/Font.h"
using namespace blok;

TEST(testEngine, myTest) {
	ChunkManager cm;
	std::optional<Block> block = cm.getBlockAt({ 0.0f, 0.0f, 0.0f });
	EXPECT_TRUE(block.has_value());

	/*int count = 0;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 2; x++) {
			for (int z = 0; z < 2; z++) {
				//int index = x + 2 * (y + 3 * z);
				int index = y * 4 + x * 2 + z;
				EXPECT_EQ(index, count);
				count++;
			}
		}
	}
	EXPECT_EQ(1, 1);*/
}

TEST(fonts, loadFontTest) {
	Font* font = Font::load("c:/Users/John/source/repos/VulkanTest/BlockEngineTest/assets/fonts/consolas.fnt");
	delete font;
}

