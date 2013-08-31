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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define MySleep Sleep
#else
#include <unistd.h>
#define MySleep(ms) usleep(ms * 1000u)
#endif

#include <GLFW/glfw3.h>

#define FPF_IMPLEMENTATION
#include "five_pixel_font.h"

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    GLuint texture_name;

    float left;
    float top;
    float right;
    float bottom;

    char ch = ' ';

    const GLfloat billboard_verts[] =
    {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f
    };

    GLfloat texture_coords[] =
    {
        0.0f, 0.0f, // DEBUG: Useful for seeing the whole texture.
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    unsigned char alpha_texture[FPF_TEXTURE_WIDTH * FPF_TEXTURE_HEIGHT];

    (void)(argc);
    (void)(argv);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glViewport(0, 0, 640, 640);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 100.0f);
    glFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -1.0f);

    fpf_create_alpha_texture(
        alpha_texture,
        FPF_TEXTURE_WIDTH * FPF_TEXTURE_HEIGHT,
        FPF_TEXTURE_WIDTH,
        FPF_VECTOR_Y_AXIS
        );

    glGenTextures(1, &texture_name);
    glBindTexture(GL_TEXTURE_2D, texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_ALPHA,
        FPF_TEXTURE_WIDTH,
        FPF_TEXTURE_HEIGHT,
        0,
        GL_ALPHA,
        GL_UNSIGNED_BYTE,
        alpha_texture
        );

    glShadeModel(GL_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    //glActiveTexture(GL_TEXTURE0);

    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    //fpf_get_cursor_glyph_gl_texture_coordinates(FPF_UNDERLINE_CURSOR, &left, &top, &right, &bottom);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        //fpf_get_glyph_gl_texture_coordinates('~' + 1, &left, &top, &right, &bottom);
        fpf_get_glyph_gl_texture_coordinates(ch++, &left, &top, &right, &bottom);
        if (ch > '~')
        {
            ch = ' ';
        }

        texture_coords[0] = left; texture_coords[1] = bottom;
        texture_coords[2] = right; texture_coords[3] = bottom;
        texture_coords[4] = left; texture_coords[5] = top;
        texture_coords[6] = right; texture_coords[7] = top;

        glTexCoordPointer(2, GL_FLOAT, 0, texture_coords);
        glVertexPointer(2, GL_FLOAT, 0, billboard_verts);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Relax guy... take it easy.
        MySleep(250);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
