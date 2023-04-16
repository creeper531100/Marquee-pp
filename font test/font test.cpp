#define _CRT_SECURE_NO_WARNINGS
#include <bitset>
#include <cstdint>
#include <ft2build.h>
#include <iostream>
#include <ostream>
#include <Windows.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_SIZE 16

#define printf(fmt,...) (void)0

static int g_index = 0;

void set_bit_value(int value, char font_buf[32], int font_size) {
    g_index %= (font_size * font_size);
    int byte_index = g_index / 8;
    int bit_index = g_index % 8;
    bit_index = 7 - bit_index;
    font_buf[byte_index] &= ~(1 << bit_index);
    font_buf[byte_index] |= (value << bit_index);
    g_index++;
}


static FT_Library library;
static FT_Face face;

int freeType_init(const char* ttf_path, int font_size) {
    int error;
    error = FT_Init_FreeType(&library);
    if (error) {
        printf("can not init free type library!\n");
        return 0;
    }

    error = FT_New_Face(library, ttf_path, 0, &face);
    if (error) {
        printf("create new face falied!\n");
        return 0;
    }


    printf("faces num : %ld , glyphs num : %ld\n", face->num_faces, face->num_glyphs);

    error = FT_Set_Pixel_Sizes(face, 0, font_size);
    if (error) {
        printf("set font size error!\n");
        return 0;
    }

    error = FT_Select_Charmap(face, ft_encoding_unicode);
    if (error) {
        printf("select charmap error!\n");
        return 0;
    }

    return 0;
}

int freeType_Uninit() {
    int error;
    error = FT_Done_FreeType(library);
    if (error) {
        printf("can not Uninit free type library!\n");
        return 0;
    }
    return 0;
}

int get_font_by_index(const char* ttf_path, int font_size, int char_index, char font_buf[32]) {
    int error;
    int i, j, k, counter;
    unsigned char temp;
    FT_UInt glyph_index;

    printf("char_index: 0x%x\n", char_index);

    glyph_index = FT_Get_Char_Index(face, char_index);

    printf("glyph_index : %d\n", glyph_index);

    if (!glyph_index) return 0;
    /**********************************************************/


    error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    if (error) {
        printf("Load char error!\n");
        return 0;
    }

    if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
        if (error) {
            printf("render char failed!\n");
            return 0;
        }
    }


    FT_Bitmap* bitmap = &face->glyph->bitmap;
    printf("r:%d, w:%d, p: %d, s: %d\n",
           bitmap->rows,
           bitmap->width,
           bitmap->pitch,
           bitmap->num_grays);

    for (j = 0; j < (font_size * 26) / 32 - face->glyph->bitmap_top; j++) {
        for (i = 0; i < font_size; i++) {
            printf("1");
            set_bit_value(0, font_buf, font_size);
        }
        printf("\n");
    }

    for (; j < face->glyph->bitmap.rows + (font_size * 26) / 32 - face->glyph->bitmap_top; j++) {
        for (i = 1; i <= face->glyph->bitmap_left; i++) {
            printf("2");
            set_bit_value(0, font_buf, font_size);
        }

        for (k = 0; k < face->glyph->bitmap.pitch; k++) {
            temp = face->glyph->bitmap.buffer[face->glyph->bitmap.pitch * (j + face->glyph->bitmap_top - (font_size *
                26) / 32) + k];
            for (counter = 0; counter < 8; counter++) {
                if (temp & 0x80) {
                    printf("*");
                    set_bit_value(1, font_buf, font_size);
                }
                else {
                    printf("_");
                    set_bit_value(0, font_buf, font_size);
                }
                temp <<= 1;
                i++;
                if (i > font_size) {
                    break;
                }
            }
        }

        for (; i <= font_size; i++) {
            // printf("|");

            set_bit_value(0, font_buf, font_size);
        }
        printf("\n");
    }

    for (; j < font_size; j++) {
        for (i = 0; i < font_size; i++) {
            printf("3");
            set_bit_value(0, font_buf, font_size);
        }
        printf("\n");
    }

    return 1;
}

struct Pack {
    uint16_t font[16];
};

std::string print_font(char* buf, int point_size) {
    std::string ans = "";
    for (int i = 0; i < point_size; i++) {
        unsigned char mask = 128;
        for (int j = 0; j < 8; j++) {
            if (mask & buf[i]) {
                ans += "█.";
            }
            else {
                ans += "=.";
            }
            mask >>= 1;
        }
        if ((i + 1) % (FONT_SIZE / 8) == 0) {
            ans += "\n";
        }
    }

    return ans;
}

void to_pack(char* buf, Pack* pack) {
    std::bitset<16> bits(0);
    int ans = 15;
    int count = 0;

    for (int i = 0; i < 32; i++) {
        unsigned char mask = 128;

        for (int j = 0; j < 8; j++) {
            bool bit = (bool)(mask & buf[i]);
            bits[ans--] = bit;
            mask >>= 1;
        }

        if ((i + 1) % (FONT_SIZE / 8) == 0) {
            pack->font[count++] = bits.to_ullong();
            bits = 0;
            ans = 15;
        }
    }
}

struct Pack2 {
    uint16_t font[16];
    Pack2* next;
};

int main() {
    char font_buf[FONT_SIZE * FONT_SIZE / 8] = {'\0'};
    const char* unicode_font_name = "mingliu_16x16.bin";
    FILE* file = fopen(unicode_font_name, "wb+");

    freeType_init("mingliu.ttc", FONT_SIZE);

    Pack* pack = new Pack[0xFFFF];

    for (int i = 0; i < 0xFFFF; ++i) {
        if (get_font_by_index("mingliu.ttc", FONT_SIZE, i, font_buf)) {
            //std::cout << print_font(font_buf, sizeof(font_buf)) << std::endl;
            to_pack(font_buf, &pack[i]);
            fwrite(&pack[i], 1, sizeof(Pack), file);
        }
        else {
            memset(pack[i].font, 0, sizeof(pack[i].font));
            fwrite(&pack[i], 1, sizeof(Pack), file);
        }

        g_index = 0;
    }

    freeType_Uninit();
    return 0;
}
