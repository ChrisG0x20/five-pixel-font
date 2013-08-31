// -----------------------------------------------------------------------
// Example program showing use of the five-pixel-font library.
// by Chris Gassib
// -----------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>

#include <stdio.h>
#define FPF_IMPLEMENTATION
#include "five_pixel_font.h"

const size_t line_pitch = 75;
const size_t alpha_texture_size = FPF_TEXTURE_HEIGHT * line_pitch;
unsigned char alpha_texture[alpha_texture_size];

int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);

    memset(alpha_texture, 0xcc, alpha_texture_size);

    size_t result = fpf_create_alpha_texture(
        alpha_texture,
        alpha_texture_size,
        line_pitch,
        FPF_RASTER_Y_AXIS // FPF_VECTOR_Y_AXIS
        );

#ifdef WIN32
    printf("create texture result = %ld\n", result);
#else
    printf("create texture result = %zd\n", result);
#endif

    if (0 != result)
    {
        printf("failed to create texture atlas");
        return 1;
    }

    unsigned int x;
    unsigned int y;
    for (y = 0; y < FPF_TEXTURE_HEIGHT; ++y)
    {
        putchar('[');
        for (x = 0; x < line_pitch; ++x)
        {
            const unsigned char texel = alpha_texture[y * line_pitch + x];
            if (0xff == texel)
            {
                putchar('#');
            }
            else if (0x00 == texel)
            {
                putchar(' ');
            }
            else
            {
                putchar('~');
            }
        }
        putchar(']');
        putchar('\n');
    }

    fpf_get_glyph_position('A', &x, &y);

    for (unsigned int k = y; k < y + 6; ++k)
    {
        for (unsigned int j = x; j < x + 6; ++j)
        {
            if (alpha_texture[k * line_pitch + j])
            {
                putchar('#');
            }
            else
            {
                putchar(' ');
            }
        }
        putchar('\n');
    }

    float left, top, right, bottom;
    fpf_get_glyph_gl_texture_coordinates(' ', &left, &top, &right, &bottom);
    printf("[%f, %f] [%f, %f]\n", left, top, right, bottom);

    fpf_get_glyph_gl_texture_coordinates('~' + 1, &left, &top, &right, &bottom);
    printf("[%f, %f] [%f, %f]\n", left, top, right, bottom);

    return 0;
}
