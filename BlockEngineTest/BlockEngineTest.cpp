// BlockEngineTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "../VulkanTest/World.h"
#include "../VulkanTest/Font.h"
#include "../VulkanTest/Floats.h"
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
	/*char** dir = new char*();
	_get_pgmptr(dir);

	std::cout << *dir << std::endl;*/
	Font* font = Font::load("../../BlockEngineTest/assets/fonts/consolas");
	CharData cd = (*font)['c'];
	EXPECT_EQ(cd.id, 'c');
	EXPECT_TRUE(floats::almostEqual(cd.x, 0.0f));
	EXPECT_TRUE(floats::almostEqual(cd.y, 142.0f));
	EXPECT_TRUE(floats::almostEqual(cd.width, 36.0f));
	EXPECT_TRUE(floats::almostEqual(cd.height, 71.0f));
	EXPECT_TRUE(floats::almostEqual(cd.xOffset, 1.0f));
	EXPECT_TRUE(floats::almostEqual(cd.yOffset, -3.0f));
	EXPECT_TRUE(floats::almostEqual(cd.xAdvance, 34.0f));

	//last
	cd = (*font)[127];
	EXPECT_EQ(cd.id, 127);
	EXPECT_TRUE(floats::almostEqual(cd.x, 360.0f));
	EXPECT_TRUE(floats::almostEqual(cd.y, 426.0f));
	EXPECT_TRUE(floats::almostEqual(cd.width, 36.0f));
	EXPECT_TRUE(floats::almostEqual(cd.height, 71.0f));
	EXPECT_TRUE(floats::almostEqual(cd.xOffset, -1.0f));
	EXPECT_TRUE(floats::almostEqual(cd.yOffset, -3.0f));
	EXPECT_TRUE(floats::almostEqual(cd.xAdvance, 34.0f));

	//first
	cd = (*font)[0];
	EXPECT_EQ(cd.id, 0);
	EXPECT_TRUE(floats::almostEqual(cd.x, 0.0f));
	EXPECT_TRUE(floats::almostEqual(cd.y, 0.0f));
	EXPECT_TRUE(floats::almostEqual(cd.width, 0.0f));
	EXPECT_TRUE(floats::almostEqual(cd.height, 0.0f));
	EXPECT_TRUE(floats::almostEqual(cd.xOffset, -3.0f));
	EXPECT_TRUE(floats::almostEqual(cd.yOffset, 0.0f));
	EXPECT_TRUE(floats::almostEqual(cd.xAdvance, 34.0f));


	//font data
	FontData fd = font->getFontData();
	EXPECT_EQ(fd.face, "Consolas");
	EXPECT_TRUE(floats::almostEqual(fd.base, 46.0f));
	EXPECT_TRUE(floats::almostEqual(fd.lineHeight, 69.0f));
	EXPECT_TRUE(floats::almostEqual(fd.padding, 3.0f));
	EXPECT_TRUE(floats::almostEqual(fd.scaleH, 512.0f));
	EXPECT_TRUE(floats::almostEqual(fd.scaleW, 512.0f));
	EXPECT_TRUE(floats::almostEqual(fd.size, 55.0f));
	EXPECT_TRUE(floats::almostEqual(fd.spacing, -2.0f));

	delete font;
}
