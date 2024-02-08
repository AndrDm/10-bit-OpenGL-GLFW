// Demo-10bit.cpp : 8/10 Bit Display Test
//.AD.04-FEB-2024.
//
#include "Demo-10bit.h"

constexpr auto ImageWidth = 1024;
constexpr auto ImageHeight = 512;

static unsigned short pixelsU16[ImageHeight][ImageWidth][3];
GLFWwindow *wImage8, *wImage10, *wControl;
float I_grays = .0f, DDL_grays = .0f, DDL_percents = .0f;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (GLFW_PRESS == action && GLFW_KEY_ESCAPE == key) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    I_grays = (float)image[ImageHeight - (int)ypos - 1][(int)xpos];
    DDL_grays = (float)pixelsU16[ImageHeight - (int)ypos - 1][(int)xpos][0];
    DDL_percents = (float)ceil((DDL_grays / 65536.0f) * 100.0f);
    if (window == wImage8) DDL_grays = (float)floor(DDL_grays / 256.0f);
    if (window == wImage10) DDL_grays = (float)floor(DDL_grays / 64.0f);
}

int main(int argc, char** argv)
{
    GLuint texture, program, vertex_buffer, vertex_shader, fragment_shader;
    GLint mvp_location, vpos_location, color_location, texture_location;

    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) exit(EXIT_FAILURE);

    wControl = glfwCreateWindow(400, 600, "Control", NULL, NULL);
    wImage8 = glfwCreateWindow(ImageWidth, ImageHeight, "8-bit", NULL, NULL);

    glfwWindowHint(GLFW_RED_BITS, 10); //must be called before windows's creation
    glfwWindowHint(GLFW_GREEN_BITS, 10);
    glfwWindowHint(GLFW_BLUE_BITS, 10);
    glfwWindowHint(GLFW_ALPHA_BITS, 2);
    wImage10 = glfwCreateWindow(ImageWidth, ImageHeight, "10-bit", NULL, NULL);

    int Offset = 0; //-1500 for my Monitor on the left side
    if (!wImage8 || !wImage10 || !wControl) { glfwTerminate(); exit(EXIT_FAILURE); }
    glfwSetWindowPos(wImage10, Offset, 32);
    glfwSetWindowPos(wImage8, Offset, ImageHeight + 62);
    glfwSetWindowPos(wControl, Offset + ImageWidth, 32);

    glfwSetKeyCallback(wImage8, key_callback);
    glfwSetKeyCallback(wImage10, key_callback);
    glfwSetKeyCallback(wControl, key_callback);
    glfwSetCursorPosCallback(wImage8, cursor_position_callback);
    glfwSetCursorPosCallback(wImage10, cursor_position_callback);

    glfwMakeContextCurrent(wControl);
    glfwSwapInterval(1); //Once!
    gladLoadGL(glfwGetProcAddress);

    nk = nk_glfw3_init(wControl, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    float Min = 32512, Max = 48880; //initial values, good for my monitor
    float Center = (Max - Min) / 2 + Min;
    float Width = Max - Min;

    for (int y = 0; y < ImageHeight; y++) {
        for (int x = 0; x < ImageWidth; x++) {
            unsigned short d = (unsigned short)(Min + x * ((Max - Min) / ImageWidth));
            pixelsU16[y][x][0] = d;
            pixelsU16[y][x][1] = d;
            pixelsU16[y][x][2] = d;
        }
    }

    for (int w = 0; w < 2; w++) {
        if (w) glfwMakeContextCurrent(wImage8); else glfwMakeContextCurrent(wImage10);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_SHORT, pixelsU16);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
        color_location = glGetUniformLocation(program, "color");
        texture_location = glGetUniformLocation(program, "texture");
        vpos_location = glGetAttribLocation(program, "vPos");

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glUseProgram(program);
        glUniform1i(texture_location, 0);

        glEnable(GL_TEXTURE_2D);

        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
    }

    int hat_image = GLFW_FALSE;
    while (!glfwWindowShouldClose(wImage8) && !glfwWindowShouldClose(wImage10) && !glfwWindowShouldClose(wControl)) {
        int widthControl, heightControl;
        //---Control---
        glfwMakeContextCurrent(wControl);
        glfwGetWindowSize(wControl, &widthControl, &heightControl);
        float opacity = glfwGetWindowOpacity(wImage10);
        struct nk_rect area = nk_rect(0.f, 0.f, (float)widthControl, (float)heightControl);
        nk_glfw3_new_frame();
        bool need_reset = false;
        if (nk_begin(nk, "", area, 0)) {
 
            nk_layout_row_dynamic(nk, 32, 2);
            nk_label(nk, "Opacity", NK_TEXT_LEFT);
            nk_label(nk, "Control", NK_TEXT_LEFT);
            nk_slider_float(nk, 0.f, &opacity, 1.f, 0.001f);
            nk_labelf(nk, NK_TEXT_LEFT, "%0.3f", opacity);

            if (nk_slider_float(nk, 0, &Min, 65535, 1)) {
                Center = (Max - Min) / 2 + Min; Width = Max - Min;
            }
            nk_labelf(nk, NK_TEXT_LEFT, " Min %0.1f", Min);
            if (nk_slider_float(nk, 0, &Max, 65535, 1)) {
                Center = (Max - Min) / 2 + Min; Width = Max - Min;
            }
            nk_labelf(nk, NK_TEXT_LEFT, " Max %0.1f", Max);
            if (nk_slider_float(nk, 0, &Center, 65535, 1)) {
                Max = Center + Width / 2; Min = Center - Width / 2;
            }
            nk_labelf(nk, NK_TEXT_LEFT, " Center %0.1f", Center);
            if (nk_slider_float(nk, 0, &Width, 65535, 1)) {
                Max = Center + Width / 2; Min = Center - Width / 2;
            }
            nk_labelf(nk, NK_TEXT_LEFT, " Width %0.1f", Width);
            need_reset = nk_checkbox_label(nk, "Image/Ramp", &hat_image);

            if (nk_button_label(nk, "Reset") || need_reset) {
                Min = 0, Max = 65535;
                //if(!hat_image) { Min = 32512, Max = 48880; };
                Center = (Max - Min) / 2 + Min;
                Width = Max - Min;
            }

            if (Min == Max) Max = Max + 1;
            if (Min < 0) Min = 0;
            if (Max > 65535) Max = 65535;
        }

        if (hat_image) {
            unsigned short LUT[65536];

            for (int i = 0; i < Min; i++) LUT[i] = 0;
            for (int i = (int)Min; i < Max; i++) {
                float temp = ((i - Min) / (Max - Min)) * 65535.0f;
                if (temp < 0) temp = 0; if (temp > 65535) temp = 65535;
                LUT[i] = (unsigned short)temp;
            }
            for (int i = (int)Max; i < 65536; i++) LUT[i] = 65535;

            for (int y = 0; y < ImageHeight; y++) {
                for (int x = 0; x < ImageWidth; x++) {
                    unsigned short d = LUT[image[y][x]];
                    pixelsU16[y][x][0] = pixelsU16[y][x][1] = pixelsU16[y][x][2] = d;
                }
            }
        }
        else { //Ramp
            for (int y = 0; y < ImageHeight; y++) {
                for (int x = 0; x < ImageWidth; x++) {
                    unsigned short d = (unsigned short)(Min + x * ((Max - Min) / ImageWidth));
                    pixelsU16[y][x][0] = pixelsU16[y][x][1] = pixelsU16[y][x][2] = d;
                }
            }
        }

        nk_labelf(nk, NK_TEXT_LEFT, " DDL (abs) = %0.1f levels", DDL_grays);
        nk_labelf(nk, NK_TEXT_LEFT, " DDL (rel) = %0.1f %%", DDL_percents);
        if (hat_image) nk_labelf(nk, NK_TEXT_LEFT, " Image = %0.1f grays", I_grays);
        else {
            nk_labelf(nk, NK_TEXT_LEFT, " DDL Range 10bit = %0.1f", floor((Max - Min) / 64.0f)+1);
            nk_labelf(nk, NK_TEXT_LEFT, " 8 bit = %0.1f levels", floor((Max - Min) / 256.0f)+1);
        }
        nk_end(nk);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(wControl);

        //--- Images ---
        for (int w = 0; w < 2; w++) {
            GLFWwindow* wImage;
            mat4x4 mvp;
            int width, height;

            wImage = w ? wImage8 : wImage10;
            glfwMakeContextCurrent(wImage);
            glfwSetWindowOpacity(wImage, opacity);
            glfwGetFramebufferSize(wImage, &width, &height);
            glViewport(0, 0, width, height);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_SHORT, pixelsU16);
            mat4x4_ortho(mvp, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
            glUniform3fv(color_location, 1, color);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glfwSwapBuffers(wImage);
        }
        glfwWaitEvents();
    }

    nk_glfw3_shutdown();

    glfwDestroyWindow(wImage8);
    glfwDestroyWindow(wImage10);
    glfwDestroyWindow(wControl);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
