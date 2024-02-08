//========================================================================
// Simple GLFW example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
// Changed by AD 04-FEB-2024
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "..\include\glad_gl.h"
#include "..\include\glfw3.h"
#include "..\include\linmath.h"
#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "..\include\\nuklear.h"

#define NK_GLFW_GL2_IMPLEMENTATION
#include "..\include\nuklear_glfw_gl2.h"

#pragma comment(lib, "..\\lib\\glfw3.lib")

constexpr auto wWidth = 768;
constexpr auto wHeight = 768;

static struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -.88f, -.50f, .25f, .25f, .25f },
    {  .88f, -.50f, .2f, .2f, .2f },
    {  0.0f,  1.0f, .2f, .2f, .2f }
};

static const char* vertex_shader_text =
"#version 330 core\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330 core\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    GLFWwindow* window8, * window10, * wControl;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) exit(EXIT_FAILURE);

    wControl = glfwCreateWindow(wWidth*2, 80, "Control", NULL, NULL);
    window8 = glfwCreateWindow(wWidth, wHeight, "8-bit", NULL, NULL);
    glfwWindowHint(GLFW_RED_BITS, 10); //must be called before windows's creation
    glfwWindowHint(GLFW_GREEN_BITS, 10);
    glfwWindowHint(GLFW_BLUE_BITS, 10);
    glfwWindowHint(GLFW_ALPHA_BITS, 2);
    window10 = glfwCreateWindow(wWidth, wHeight, "10-bit", NULL, NULL);

    if (!window8 || !window10) { glfwTerminate(); exit(EXIT_FAILURE); }

    glfwSetKeyCallback(window8, key_callback);
    glfwSetKeyCallback(window10, key_callback);
    glfwSetKeyCallback(wControl, key_callback);

    glfwSetWindowPos(window8, 0, 32);
    glfwSetWindowPos(window10, wWidth, 32);
    glfwSetWindowPos(wControl, 0, wHeight+64);

    glfwMakeContextCurrent(wControl);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    
    nk = nk_glfw3_init(wControl, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    // NOTE: OpenGL error checks have been omitted for brevity
    for (int w = 0; w < 2; w++) {
        if (w)glfwMakeContextCurrent(window8); else glfwMakeContextCurrent(window10);
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        mvp_location = glGetUniformLocation(program, "MVP");
        vpos_location = glGetAttribLocation(program, "vPos");
        vcol_location = glGetAttribLocation(program, "vCol");

        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(sizeof(float) * 2));
    }
    float g0=.25f, g1=.22f, g2=.20f;

    while (!glfwWindowShouldClose(window8) && !glfwWindowShouldClose(window10) && !glfwWindowShouldClose(wControl))
    {
        //---Control---
        glfwMakeContextCurrent(wControl);
        int widthControl, heightControl;
        glfwGetWindowSize(wControl, &widthControl, &heightControl);
        struct nk_rect area = nk_rect(0.f, 0.f, (float)widthControl, (float)heightControl);
        nk_glfw3_new_frame();
        bool need_reset = false;
        if (nk_begin(nk, "", area, 0)) {
            nk_layout_row_dynamic(nk, 60, 6);
            nk_slider_float(nk, 0.f, &g0, 1.f, 0.001f);
            nk_labelf(nk, NK_TEXT_LEFT, "%0.3f", g0);
            nk_slider_float(nk, 0.f, &g1, 1.f, 0.001f);
            nk_labelf(nk, NK_TEXT_LEFT, "%0.3f", g1);
            nk_slider_float(nk, 0.f, &g2, 1.f, 0.001f);
            nk_labelf(nk, NK_TEXT_LEFT, "%0.3f", g2);
        }
        nk_end(nk);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(wControl);

        for (int i = 0; i < 2; i++) {
            GLFWwindow* window;
            float ratio;
            int width, height;
            mat4x4 m, p, mvp;

            window = i ? window10 : window8;
            glfwMakeContextCurrent(window);

            vertices[0].r = vertices[0].g = vertices[0].b = g0;
            vertices[1].r = vertices[1].g = vertices[1].b = g1;
            vertices[2].r = vertices[2].g = vertices[2].b = g2;

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glfwGetFramebufferSize(window, &width, &height);
            ratio = width / (float)height;

            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(.1f, .1f, .1f, 1.f);

            mat4x4_identity(m);
            mat4x4_rotate_Z(m, m, (float)glfwGetTime());
            mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
            mat4x4_mul(mvp, p, m);

            glUseProgram(program);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            GLuint texture_map;
            glGenTextures(1, &texture_map);
            glBindTexture(GL_TEXTURE_2D, texture_map);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glBindTexture(GL_TEXTURE_2D, 0);

            glfwSwapBuffers(window);
        }
        glfwPollEvents();
    }

    glfwDestroyWindow(window8);
    glfwDestroyWindow(window10);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

