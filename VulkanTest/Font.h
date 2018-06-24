#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace blok {
	struct CharData {
		char id;
		float x;
		float y;
		float width;
		float height;
		float xOffset;
		float yOffset;
		float xAdvance;
	};

	struct FontData {
		std::string face;
		float size;
		float padding;
		float spacing;
		float lineHeight;
		float base;
		float scaleW;
		float scaleH;
	};

	struct FontToken {
		std::string token;
		int end;
	};

	class Font {
	public:
		FontData getFontData() {
			return fdata;
		}

		CharData& operator[] (char key) {
			return cdata[key];
		}

		static Font* load(std::string file) {

			std::ifstream ff;
			ff.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			std::string line;
			Font* font = new Font();
			try {
				ff.open(file.append(".fnt"));
				font->atlasFile = file.append(".png");
				while (std::getline(ff, line)) {
					FontToken token = nextToken(line, 0);
					while (token.end > 0) {
						if (token.token == "face") {
							token = nextToken(line, token.end);
							font->fdata.face = token.token.substr(1, token.token.length() - 2);
						}
						else if (token.token == "size") {
							token = nextToken(line, token.end);
							font->fdata.size = std::stof(token.token);
						}
						else if (token.token == "padding") {
							token = nextToken(line, token.end);
							size_t commaInd = token.token.find_first_of(',');
							font->fdata.padding = std::stof(token.token.substr(0, commaInd));
						}
						else if (token.token == "spacing") {
							token = nextToken(line, token.end);
							size_t commaInd = token.token.find_first_of(',');
							font->fdata.spacing = std::stof(token.token.substr(0, commaInd));
						}
						else if (token.token == "lineHeight") {
							token = nextToken(line, token.end);
							font->fdata.lineHeight = std::stof(token.token);
						}
						else if (token.token == "base") {
							token = nextToken(line, token.end);
							font->fdata.base = std::stof(token.token);
						}
						else if (token.token == "scaleW") {
							token = nextToken(line, token.end);
							font->fdata.scaleW = std::stof(token.token);
						}
						else if (token.token == "scaleH") {
							token = nextToken(line, token.end);
							font->fdata.scaleH = std::stof(token.token);
						}
						else if (token.token == "char") {
							CharData cd = parseChar(line, token.end);
							font->cdata[cd.id] = cd;
						}
						token = nextToken(line, token.end);
					}
				}
			}
			catch (std::ifstream::failure e) {
				std::cerr << "failed to open file. " << e.what() << " : " << e.code().message() << std::endl;
			}
			return font;
		}

		void copyTextureAtlaas(void* dst) {
			int channels, width, height;
			stbi_uc* pixels = stbi_load(atlasFile.c_str(), &width, &height, &channels, STBI_grey);
			memcpy(dst, pixels, width * height * channels);
			stbi_image_free(pixels);
		}

	private:
		std::unordered_map<char, CharData> cdata;
		FontData fdata;
		std::string atlasFile;

		static FontToken nextToken(std::string line, int offset) {
			if (offset >= line.length()) {
				return { "", -1 };
			}
			int end = offset;
			int start = offset;
			char current = line[end];
			//scan through any leading spaces
			while (current == ' ' && end < line.length()) {
				start++;
				end++;
				current = line[end];
			}
			//find the next token delimiter (space or equal sign)
			while (current != ' ' && current != '=' && end < line.length()) {
				end++;
				current = line[end];
			}
			if (end - start <= 0) {
				return { "", -1 };
			}
			return { line.substr(start, end - start), end + 1 };
		}

		static CharData parseChar(std::string line, int begin) {
			FontToken token = nextToken(line, begin);
			CharData cd;
			while (token.end > 0) {
				if (token.token == "id") {
					token = nextToken(line, token.end);
					cd.id = std::stoi(token.token);
				}
				else if (token.token == "x") {
					token = nextToken(line, token.end);
					cd.x = std::stof(token.token);
				}
				else if (token.token == "y") {
					token = nextToken(line, token.end);
					cd.y = std::stof(token.token);
				}
				else if (token.token == "width") {
					token = nextToken(line, token.end);
					cd.width = std::stof(token.token);
				}
				else if (token.token == "height") {
					token = nextToken(line, token.end);
					cd.height = std::stof(token.token);
				}
				else if (token.token == "xoffset") {
					token = nextToken(line, token.end);
					cd.xOffset = std::stof(token.token);
				}
				else if (token.token == "yoffset") {
					token = nextToken(line, token.end);
					cd.yOffset = std::stof(token.token);
				}
				else if (token.token == "xadvance") {
					token = nextToken(line, token.end);
					cd.xAdvance = std::stof(token.token);
				}
				token = nextToken(line, token.end);
			}
			return cd;
		}
	};
}
