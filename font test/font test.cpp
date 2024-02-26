#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <stdio.h>
#include <ft2build.h>
#include <bitset>
#include FT_FREETYPE_H

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

void draw(wchar_t character, unsigned char* buffer) {
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        fprintf(stderr, "Failed to initialize FreeType library\n");
        return;
    }

    FT_Face face;
    if (FT_New_Face(library, "STSONG.TTF", 0, &face)) {
        fprintf(stderr, "Failed to load font\n");
        FT_Done_FreeType(library);
        return;
    }

    if (FT_Set_Pixel_Sizes(face, 0, 16)) {
        fprintf(stderr, "Failed to set pixel sizes\n");
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return;
    }

    FT_UInt charIndex = FT_Get_Char_Index(face, character);
    if (FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT)) {
        fprintf(stderr, "Failed to load glyph\n");
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return;
    }

    if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
        fprintf(stderr, "Failed to render glyph\n");
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return;
    }

    FT_Bitmap bitmap = face->glyph->bitmap;

    // 計算位圖的偏移量
    int xOffset = (BITMAP_WIDTH - bitmap.width) / 2;
    int yOffset = (BITMAP_HEIGHT - bitmap.rows) / 2;

    // 調整位圖大小為16x16，並將字形置中
    for (int y = 0; y < BITMAP_HEIGHT; y++) {
        for (int x = 0; x < BITMAP_WIDTH; x++) {
            if (x >= xOffset && x < xOffset + bitmap.width &&
                y >= yOffset && y < yOffset + bitmap.rows) {
                int pixel = bitmap.buffer[(y - yOffset) * bitmap.width + (x - xOffset)];
                buffer[y * BITMAP_WIDTH + x] = pixel;
            }
            else {
                buffer[y * BITMAP_WIDTH + x] = 0;  // 將超出字形範圍的像素設置為0
            }
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

struct Pack {
    uint16_t font[16];
};

void to_pack(unsigned char* buf, Pack* pack) {
    // 打印位圖數據
    int count = 0;
    for (int y = 0; y < BITMAP_HEIGHT; y++) {
        std::string str = "";
        for (int x = 0; x < BITMAP_WIDTH; x++) {
            int pixel = buf[y * BITMAP_WIDTH + x];
            //printf("%c", pixel > 0 ? '#' : ' ');
            str += pixel > 0 ? "1" : "0";
        }
        std::bitset<16> bit(str);
        pack->font[count++] = bit.to_ullong();
    }
}

int main() {
    unsigned char buffer[BITMAP_WIDTH * BITMAP_HEIGHT] = { '\0' };

    const char* unicode_font_name = "69.bin";
    FILE* file = fopen(unicode_font_name, "wb+");
    Pack* pack = new Pack[0xFFFF];

    for (int i = 0; i < 0xFFFF; ++i) {
        draw(i, buffer);
        to_pack(buffer, &pack[i]);
        fwrite(&pack[i], 1, sizeof(Pack), file);
        printf("%lf\r", (((float)i) / (float)0xFFFF) * 100.0f);
    }

    return 0;
}