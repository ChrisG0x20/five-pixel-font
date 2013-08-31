// -----------------------------------------------------------------------
// five-pixel-font library
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
//

//
// Destructions:
//
//  1) Use fpf_create_alpha_texture() to fill a block of memory with an alpha texture of the font
//     atlas. Example:
//          unsigned char alpha_texture[FPF_TEXTURE_WIDTH * FPF_TEXTURE_HEIGHT];
//          fpf_create_alpha_texture(
//              alpha_texture,
//              FPF_TEXTURE_WIDTH * FPF_TEXTURE_HEIGHT,
//              FPF_TEXTURE_WIDTH,
//              FPF_VECTOR_Y_AXIS
//              );
//
//  2) Copy the alpha texture into graphics memory with something like glTexImage2D(). Example:
//          GLuint texture_name;
//          glGenTextures(1, &texture_name);
//          glBindTexture(GL_TEXTURE_2D, texture_name);
//          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//          glTexImage2D(
//              GL_TEXTURE_2D,
//              0,
//              GL_ALPHA,
//              FPF_TEXTURE_WIDTH,
//              FPF_TEXTURE_HEIGHT,
//              0,
//              GL_ALPHA,
//              GL_UNSIGNED_BYTE,
//              alpha_texture
//              );
//
//  3) Get the texture coordinates of a character glyph from fpf_get_glyph_gl_texture_coordinates().
//     Example:
//          float left;
//          float top;
//          float right;
//          float bottom;
//          fpf_get_glyph_gl_texture_coordinates('X', &left, &top, &right, &bottom);
//
//  4) Render a blended, colored, textured quad to the framebuffer. Example:
//          const GLfloat billboard_verts[] =
//          {
//              -1.0f, -1.0f,
//               1.0f, -1.0f,
//              -1.0f,  1.0f,
//               1.0f,  1.0f
//          };
//          const GLfloat texture_coords[] =
//          {
//              left,   bottom,
//              right,  bottom,
//              left,   top,
//              right,  top
//          };
//          glEnable(GL_BLEND);
//          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//          glEnable(GL_TEXTURE_2D);
//          glActiveTexture(GL_TEXTURE0);
//          glUniform4f(text_color_uniform_id, r, g, b, a);
//          glUniform1i(texture_uniform_id, 0);
//          glEnableVertexAttribArray(texture_coord_attr_id);
//          glVertexAttribPointer(texture_coord_attr_id, 2, GL_FLOAT, GL_FALSE, 0, texture_coords);
//          glEnableVertexAttribArray(_vertexPosVar);
//          glVertexAttribPointer(vertex_attr_id, 2, GL_FLOAT, GL_FALSE, 0, billboard_verts);
//          glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
//  Notes: A fragment shader probably contains a line like this:
//          gl_FragColor = u_text_color * texture2D(u_texture_id, v_tex_coord).a;
//

#ifndef FIVEPIXELFONT_H
#define FIVEPIXELFONT_H

// The texture atlas may be created for raster blits or DirectX where the first byte of the texture
// represents the [left, top] corner of the raster image. Or, the texture atlas may be created
// Windows Bitmap (DIB) / OpenGL-style where the first byte of the texture represents the
// [left, bottom] corner of the raster image.
typedef enum
{
    FPF_RASTER_Y_AXIS = 0,  // First byte is at the top of the texture.
    FPF_VECTOR_Y_AXIS       // First byte is at the bottom of the texture.
} fpf_y_axis_direction;

// The font's texture atlas contains three different cursor glyphs. This enumeration is used to
// address a specific cursor glyph.
typedef enum
{
    FPF_UNDERLINE_CURSOR = 0,   // Underline-style cursor (like the default in a cmd.exe window).
    FPF_BLOCK_CURSOR,           // Block-style cursor (like the default iTerm style).
    FPF_VERTICAL_CURSOR         // Vertical-style (like most modern GUIs editors).
} fpf_cursor_style;

// Each glyph fits within a 6x6 pixel character cell. Although, almost every glyph uses only the
// top-left 5x5 pixels so there's space inbetween characters (only cursor glyphs break the rule).
#define FPF_GLYPH_WIDTH     6u
#define FPF_GLYPH_HEIGHT    6u

// The texture atlas surface needs to be at least 64x64 bytes.
#define FPF_TEXTURE_WIDTH   64u
#define FPF_TEXTURE_HEIGHT  64u

// Declarations.
size_t fpf_create_alpha_texture(unsigned char *const p_alpha_texture, const size_t alpha_texture_size_in_bytes,
                                const size_t line_pitch, const fpf_y_axis_direction y_axis_direction);
void fpf_get_glyph_position(const char character, unsigned int *const x, unsigned int *const y);
void fpf_get_glyph_dx_texture_coordinates(const char character, float *const left, float *const top,
                                          float *const right, float *const bottom);
void fpf_get_glyph_gl_texture_coordinates(const char character, float *const left, float *const top,
                                          float *const right, float *const bottom);
void fpf_get_cursor_glyph_position(const fpf_cursor_style cursor_style, unsigned int *const x, unsigned int *const y);
void fpf_get_cursor_glyph_dx_texture_coordinates(const fpf_cursor_style cursor_style, float *const left,
                                                 float *const top, float *const right, float *const bottom);
void fpf_get_cursor_glyph_gl_texture_coordinates(const fpf_cursor_style cursor_style, float *const left,
                                                 float *const top, float *const right, float *const bottom);

#ifdef FPF_IMPLEMENTATION

// #define your own FPF_memset() to override the include of memory.h for the default memset().
#ifndef FPF_memset
#include <memory.h> // For memset() and size_t.
#define FPF_memset(ptr, value, num) memset(ptr, value, num)
#endif

// #define your own FPF_assert() to override the include of assert.h for the default assert().
#ifndef FPF_assert
#include <assert.h> // For parameter checking.
#define FPF_assert(expression) assert(expression)
#endif

// This is the 5x5 pixel font image; lightly compressed.
const unsigned char fpf_compressed_font[] =
{
    0, 1, 133, 20, 123, 34, 8, 16, 128, 0, 1, 133, 62, 163, 69, 16, 32, 64,
    0, 1, 128, 20, 112, 130, 0, 1, 32, 64, 0, 2, 62, 41, 101, 0, 1, 32, 64,
    0, 1, 128, 20, 242, 98, 128, 16, 128, 0, 8, 168, 0, 3, 39, 8, 49, 128,
    112, 128, 0, 2, 73, 152, 72, 64, 249, 192, 28, 0, 1, 138, 136, 16, 128,
    112, 130, 0, 1, 1, 12, 136, 32, 64, 168, 4, 0, 1, 34, 7, 28, 121, 128,
    0, 8, 49, 194, 30, 97, 128, 0, 1, 16, 0, 1, 81, 4, 2, 146, 66, 8, 33,
    192, 121, 142, 4, 97, 192, 0, 1, 64, 0, 1, 16, 73, 8, 144, 66, 8, 33,
    192, 17, 134, 8, 96, 64, 16, 16, 0, 9, 65, 135, 28, 241, 239, 62, 249,
    224, 32, 64, 162, 138, 8, 160, 130, 0, 1, 16, 134, 190, 242, 8, 188,
    242, 96, 32, 10, 162, 138, 8, 160, 130, 32, 64, 135, 34, 241, 239, 62,
    129, 224, 0, 8, 137, 195, 164, 130, 40, 156, 241, 192, 136, 129, 40,
    131, 108, 162, 138, 32, 248, 129, 48, 130, 170, 162, 242, 32, 136, 137,
    40, 130, 41, 162, 130, 64, 137, 198, 36, 242, 40, 156, 129, 160, 0, 8,
    241, 239, 162, 138, 40, 162, 113, 192, 138, 2, 34, 138, 37, 20, 17, 0,
    1, 241, 194, 34, 138, 162, 8, 33, 0, 1, 160, 34, 34, 82, 165, 8, 65, 0,
    1, 147, 194, 28, 33, 72, 136, 113, 192, 0, 8, 129, 194, 0, 1, 32, 8, 0,
    1, 16, 0, 1, 64, 69, 0, 1, 17, 206, 12, 16, 128, 32, 64, 0, 1, 2, 73,
    16, 113, 64, 16, 64, 0, 1, 2, 73, 16, 145, 128, 9, 192, 62, 1, 174, 12,
    112, 192, 0, 8, 16, 196, 8, 17, 2, 0, 3, 33, 68, 0, 1, 1, 2, 52, 96,
    128, 112, 199, 8, 17, 66, 42, 81, 64, 32, 68, 136, 81, 130, 34, 81, 64,
    33, 132, 136, 33, 66, 34, 80, 128, 0, 16, 96, 197, 12, 113, 37, 34, 81,
    64, 81, 70, 8, 33, 37, 42, 33, 64, 96, 196, 4, 33, 37, 42, 32, 128, 64,
    68, 12, 16, 194, 20, 81, 0, 10, 66, 16, 3, 224, 63, 128, 0, 1, 48, 130,
    8, 82, 32, 63, 128, 0, 1, 17, 128, 12, 162, 32, 63, 128, 0, 1, 32, 130,
    8, 2, 32, 63, 128, 0, 1, 48, 66, 16, 3, 239, 255, 128, 0, 5, 15, 255,
    128
};

unsigned char *fpf_next_line(
    unsigned char *const p,
    const size_t line_pitch,
    const unsigned int x,
    const fpf_y_axis_direction y_axis_direction
    )
{
    return (FPF_RASTER_Y_AXIS == y_axis_direction) ?
        p + (line_pitch - x) :
        p - (x + line_pitch);
}

// Creates an alpha texture atlas of the font glyphs from the fpf_compressed_font data.
// Returns: 0 on success, otherwise it the size (in bytes) required to draw the font.
size_t fpf_create_alpha_texture(
    unsigned char *const        p_alpha_texture,                // [in/out] Raw alpha texture memory to create the font atlas in.
    const size_t                alpha_texture_size_in_bytes,    // [in]     Size of the texture memory in bytes.
    const size_t                line_pitch,                     // [in]     Line pitch of the texture memory in bytes.
    const fpf_y_axis_direction  y_axis_direction                // [in]     Direction of the textures y-axis.
    )
{
    const size_t required_bytes = FPF_TEXTURE_WIDTH * FPF_TEXTURE_HEIGHT;

    // Get write pointer.
    unsigned char *p_dst = (FPF_RASTER_Y_AXIS == y_axis_direction) ?
        &p_alpha_texture[0] : &p_alpha_texture[FPF_TEXTURE_HEIGHT * line_pitch - line_pitch];

    unsigned int x = 0; // Keep track of the current texture column index.
    unsigned int y = 0; // Keep track of the current texture row index.

    size_t byte_index = 0; // For the outer loop.
    unsigned int i; // For loops.

    FPF_assert(NULL != p_alpha_texture);

    // Check the line pitch.
    if (line_pitch < FPF_TEXTURE_WIDTH) // if (line pitch is too short)
    {
        return required_bytes;
    }

    // Check the texture size.
    if (alpha_texture_size_in_bytes < line_pitch * FPF_TEXTURE_HEIGHT)
    {
        return required_bytes;
    }

    // Decompress the font.
    for (; byte_index < sizeof(fpf_compressed_font); ++byte_index)
    {
        unsigned char byte = fpf_compressed_font[byte_index]; // Grab the next byte.

        if (0 == byte) // if (this byte indicates a run of zeros)
        {
            // Get the next byte, which indicates the run-length in bytes.
            const unsigned int run_length = fpf_compressed_font[++byte_index] * 8;

            // For each bit in the run.
            for (i = 0; i < run_length; ++i)
            {
                *p_dst++ = 0x00;

                if (++x == FPF_TEXTURE_WIDTH) // if (end-of-line)
                {
                    p_dst = fpf_next_line(p_dst, line_pitch, x, y_axis_direction);
                    x = 0;
                    ++y;
                }
            }

            // Completed a run of zeros, start the next byte of the font.
            continue;
        }

        // Convert the single-byte bitmap to a string of eight bytes.
        for (i = 0; i < 8; ++i)
        {
            *p_dst++ = (byte & 0x80) ? 0xff : 0x00;
            byte <<= 1;
        }

        // Advance the texture column index.
        x += 8;

        if (x >= FPF_TEXTURE_WIDTH)
        {
            p_dst = fpf_next_line(p_dst, line_pitch, x, y_axis_direction);
            x = 0;
            ++y;
        }
    }

    // Fill in any trailing space at the end of the last line.
    FPF_memset(p_dst, 0x00, FPF_TEXTURE_WIDTH - x);
    p_dst = fpf_next_line(p_dst, line_pitch, x, y_axis_direction);

    // Clear any trailing lines.
    for (i = y + 1; i < FPF_TEXTURE_HEIGHT; ++i)
    {
        FPF_memset(p_dst, 0x00, FPF_TEXTURE_WIDTH);
        p_dst = fpf_next_line(p_dst, line_pitch, 0, y_axis_direction);
    }

    return 0;
}

// Gets the [column, row] position of a character's glyph in the texture atlas.
void fpf_get_glyph_position(
    const char          character,  // [in]  Character to locate.
    unsigned int *const x,          // [out] Column of the glyph.
    unsigned int *const y           // [out] Row of the glyph.
    )
{
    const int ascii_value = (character < ' ' || character > '~') ? 127 : character;
    const int glyph_index = ascii_value - ' ';
    const unsigned int glyphs_per_row = FPF_TEXTURE_WIDTH / FPF_GLYPH_WIDTH;
    const int texture_row = glyph_index / glyphs_per_row;
    const int texture_column = glyph_index - (glyphs_per_row * texture_row);
    FPF_assert(NULL != x);
    FPF_assert(NULL != y);
    *x = texture_column * FPF_GLYPH_WIDTH;
    *y = texture_row * FPF_GLYPH_HEIGHT;
}

// Gets the texture coordinates of a character's glyph in the texture atlas.
void fpf_get_glyph_dx_texture_coordinates(
    const char      character,  // [in]  Character to locate.
    float *const    left,       // [out] Left bound of the character's glyph.
    float *const    top,        // [out] Top bound of the character's glyph.
    float *const    right,      // [out] Right bound of the character's glyph.
    float *const    bottom      // [out] Bottom bound of the character's glyph.
    )
{
    unsigned int x;
    unsigned int y;
    FPF_assert(NULL != left);
    FPF_assert(NULL != top);
    FPF_assert(NULL != right);
    FPF_assert(NULL != bottom);
    fpf_get_glyph_position(character, &x, &y);
    *left = (float)x / FPF_TEXTURE_WIDTH;
    *top = (float)y / FPF_TEXTURE_HEIGHT;
    *right = (float)(x + FPF_GLYPH_WIDTH) / FPF_TEXTURE_WIDTH;
    *bottom = (float)(y + FPF_GLYPH_HEIGHT) / FPF_TEXTURE_HEIGHT;
}

// Gets the texture coordinates of a character's glyph in the texture atlas.
void fpf_get_glyph_gl_texture_coordinates(
    const char      character,  // [in]  Character to locate.
    float *const    left,       // [out] Left bound of the character's glyph.
    float *const    top,        // [out] Top bound of the character's glyph.
    float *const    right,      // [out] Right bound of the character's glyph.
    float *const    bottom      // [out] Bottom bound of the character's glyph.
    )
{
    FPF_assert(NULL != left);
    FPF_assert(NULL != top);
    FPF_assert(NULL != right);
    FPF_assert(NULL != bottom);
    fpf_get_glyph_dx_texture_coordinates(character, left, top, right, bottom);
    *top = 1.0f - *top;
    *bottom = 1.0f - *bottom;
}

// Gets the [column, row] position of a cursor glyph in the texture atlas.
void fpf_get_cursor_glyph_position(
    const fpf_cursor_style  cursor_style,   // [in]  Cursor glyph style.
    unsigned int *const     x,              // [out] Column of the glyph.
    unsigned int *const     y               // [out] Row of the glyph.
    )
{
    const int glyph_index = 96 +
        ((cursor_style < FPF_UNDERLINE_CURSOR || cursor_style > FPF_VERTICAL_CURSOR) ?
            FPF_UNDERLINE_CURSOR : cursor_style);
    const unsigned int glyphs_per_row = FPF_TEXTURE_WIDTH / FPF_GLYPH_WIDTH;
    const int texture_row = glyph_index / glyphs_per_row;
    const int texture_column = glyph_index - (glyphs_per_row * texture_row);
    FPF_assert(NULL != x);
    FPF_assert(NULL != y);
    *x = texture_column * FPF_GLYPH_WIDTH;
    *y = texture_row * FPF_GLYPH_HEIGHT;
}

// Gets the texture coordinates of a cursor glyph in the texture atlas.
void fpf_get_cursor_glyph_dx_texture_coordinates(
    const fpf_cursor_style  cursor_style,   // [in]  Cursor glyph style.
    float *const            left,           // [out] Left bound of the character's glyph.
    float *const            top,            // [out] Top bound of the character's glyph.
    float *const            right,          // [out] Right bound of the character's glyph.
    float *const            bottom          // [out] Bottom bound of the character's glyph.
    )
{
    unsigned int x;
    unsigned int y;
    FPF_assert(NULL != left);
    FPF_assert(NULL != top);
    FPF_assert(NULL != right);
    FPF_assert(NULL != bottom);
    fpf_get_cursor_glyph_position(cursor_style, &x, &y);
    *left = (float)x / FPF_TEXTURE_WIDTH;
    *top = (float)y / FPF_TEXTURE_HEIGHT;
    *right = (float)(x + FPF_GLYPH_WIDTH) / FPF_TEXTURE_WIDTH;
    *bottom = (float)(y + FPF_GLYPH_HEIGHT) / FPF_TEXTURE_HEIGHT;
}

// Gets the texture coordinates of a cursor glyph in the texture atlas.
void fpf_get_cursor_glyph_gl_texture_coordinates(
    const fpf_cursor_style  cursor_style,   // [in]  Cursor glyph style.
    float *const            left,           // [out] Left bound of the character's glyph.
    float *const            top,            // [out] Top bound of the character's glyph.
    float *const            right,          // [out] Right bound of the character's glyph.
    float *const            bottom          // [out] Bottom bound of the character's glyph.
    )
{
    FPF_assert(NULL != left);
    FPF_assert(NULL != top);
    FPF_assert(NULL != right);
    FPF_assert(NULL != bottom);
    fpf_get_cursor_glyph_dx_texture_coordinates(cursor_style, left, top, right, bottom);
    *top = 1.0f - *top;
    *bottom = 1.0f - *bottom;
}

#endif // FPF_IMPLEMENT

#endif // FIVEPIXELFONT_H
