//========================================================================
// Simple GLFW example
// Copyright (c) https://github.com/derhass/HelloCube
// Changed by AD 04-FEB-2024
//========================================================================

#include "..\include\glad_gl.h"
#include "..\include\glfw3.h"

#include "..\include\glm\mat4x4.hpp"
#include "..\include\glm\gtc\type_ptr.hpp"
#include "..\include\glm\gtx\transform.hpp"

#pragma comment(lib, "..\\lib\\glfw3.lib")

constexpr auto wWidth = 768;
constexpr auto wHeight = 768;

// We use the following layout for vertex data
typedef struct {
    GLfloat pos[3]; // 3D cartesian coordinates
    GLfloat clr[3]; // RGB (floats, because we need 10 bit rendering)
} Vertex;

static const Vertex cubeGeometry[] = {
    //  X     Y     Z   |   R     G     B
    // front face
    {{-1.0, -1.0,  1.0},  {.25f, .25f, .25f}},
    {{ 1.0, -1.0,  1.0},  {.23f, .23f, .23f}},
    {{-1.0,  1.0,  1.0},  {.20f, .20f, .20f}},
    {{ 1.0,  1.0,  1.0},  {.20f, .20f, .20f}},
    // back face
    {{ 1.0, -1.0, -1.0},  {.35f, .35f, .35f}},
    {{-1.0, -1.0, -1.0},  {.35f, .35f, .35f}},
    {{ 1.0,  1.0, -1.0},  {.30f, .30f, .30f}},
    {{-1.0,  1.0, -1.0},  {.30f, .30f, .30f}},
    // left  face
    {{-1.0, -1.0, -1.0},  {.45f, .45f, .45f}},
    {{-1.0, -1.0,  1.0},  {.45f, .45f, .45f}},
    {{-1.0,  1.0, -1.0},  {.40f, .40f, .40f}},
    {{-1.0,  1.0,  1.0},  {.40f, .40f, .40f}},
    // right face
    {{ 1.0, -1.0,  1.0},  {.55f, .55f, .55f}},
    {{ 1.0, -1.0, -1.0},  {.55f, .55f, .55f}},
    {{ 1.0,  1.0,  1.0},  {.50f, .50f, .50f}},
    {{ 1.0,  1.0, -1.0},  {.50f, .50f, .50f}},
    // top face
    {{-1.0,  1.0,  1.0},  {.65f, .65f, .65f}},
    {{ 1.0,  1.0,  1.0},  {.65f, .65f, .65f}},
    {{-1.0,  1.0, -1.0},  {.60f, .60f, .60f}},
    {{ 1.0,  1.0, -1.0},  {.60f, .60f, .60f}},
    // bottom face
    {{ 1.0, -1.0,  1.0},  {.75f, .75f, .75f}},
    {{-1.0, -1.0,  1.0},  {.75f, .75f, .75f}},
    {{ 1.0, -1.0, -1.0},  {.70f, .70f, .70f}},
    {{-1.0, -1.0, -1.0},  {.70f, .70f, .70f}},
};


/* use two triangles sharing an edge for each face */
static const GLushort cubeConnectivity[] = {
     0, 1, 2,  2, 1, 3,	/* front */
     4, 5, 6,  6, 5, 7,	/* back */
     8, 9,10, 10, 9,11,	/* left */
    12,13,14, 14,13,15,	/* right */
    16,17,18, 18,17,19,	/* top */
    20,21,22, 22,21,23	/* bottom */
};

GLuint vbo[2];	/* vertex and index buffer names */
GLuint vao;		/* vertex array object */

static const char* vertex_shader_cube =
"#version 330 core\n"
"uniform mat4 modelView;\n"
"uniform mat4 projection;\n"
"in vec3 pos;\n"
"in vec4 clr;\n"
"out vec4 v_clr;\n"
"void main()\n"
"{\n"
"    v_clr = clr;\n"
"    gl_Position = projection * modelView * vec4(pos, 1.0);\n"
"}\n";

static const char* fragment_shader_cube =
"#version 330 core\n"
"in vec4 v_clr;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    color = v_clr;\n"
"}\n";


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void callback_Resize(GLFWwindow* win, int w, int h)
{
    //if needed sync size
}

int main(void)
{
    GLFWwindow* window8, * window10;
    GLuint vertex_shader, fragment_shader, program;
    GLint vpos_location = 0, vcol_location = 1;

    GLint locProjection;
    GLint locModelView;
    GLint locTime;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) exit(EXIT_FAILURE); //initializing GLFW

    window8 = glfwCreateWindow(wWidth, wHeight, "8-bit", NULL, NULL);
    glfwWindowHint(GLFW_RED_BITS, 10); //must be called before windows's creation
    glfwWindowHint(GLFW_GREEN_BITS, 10);
    glfwWindowHint(GLFW_BLUE_BITS, 10);
    glfwWindowHint(GLFW_ALPHA_BITS, 2);
    window10 = glfwCreateWindow(wWidth, wHeight, "10-bit", NULL, NULL);

    if (!window8 || !window10) { glfwTerminate(); exit(EXIT_FAILURE); }

    glfwSetKeyCallback(window8, key_callback);
    glfwSetKeyCallback(window10, key_callback);
    glfwSetFramebufferSizeCallback(window8, callback_Resize);

    glfwSetWindowPos(window8, 0, 32);
    glfwSetWindowPos(window10, wWidth, 32);

    glfwMakeContextCurrent(window8);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1); // ask the driver to enable synchronizing the buffer swaps to the *VBLANK of the display.

    // NOTE: OpenGL error checks have been omitted for brevity
    for (int w = 0; w < 2; w++) {
        if (w)glfwMakeContextCurrent(window8); else glfwMakeContextCurrent(window10);
        glEnable(GL_DEPTH_TEST); // set once and never change them, so there is no need to set them during the main loop

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeGeometry), cubeGeometry, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeConnectivity), cubeConnectivity, GL_STATIC_DRAW);

        program = glCreateProgram();

        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(vcol_location);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_cube, NULL);
        glCompileShader(vertex_shader);

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_cube, NULL);
        glCompileShader(fragment_shader);
  
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        locProjection = glGetUniformLocation(program, "projection");
        locModelView = glGetUniformLocation(program, "modelView");
        locTime = glGetUniformLocation(program, "time");
    }

    double timeCur = glfwGetTime();
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window8) && !glfwWindowShouldClose(window10))
    {
        double now = glfwGetTime();
        double timeDelta = now - timeCur;
        timeCur = now;

        for (int i = 0; i < 2; i++) {
            GLFWwindow* window;
            int width, height;

            window = i ? window10 : window8;
            glfwMakeContextCurrent(window);

            glfwGetFramebufferSize(window, &width, &height);
            float ratio = width / (float)height;
            float speed = 0.25f;
            float scale = 65.0f;
            model = glm::rotate(model, (float)(glm::half_pi<double>() * timeDelta * speed), glm::vec3(0.8f, 0.6f, 0.1f)); //
            projection = glm::perspective(glm::radians(scale), (float)width / (float)height, 0.1f, 10.0f);
            view = glm::translate(glm::vec3(0.0f, 0.0f, -4.0f));

            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(.3f, .3f, .3f, 1.f);

            glm::mat4 modelView = view * model;

            glUseProgram(program);

            glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(locModelView, 1, GL_FALSE, glm::value_ptr(modelView));
            glUniform1f(locTime, (GLfloat)timeCur);

            /* draw the cube */
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_SHORT, (void*)0);

            glBindVertexArray(0);

            glfwSwapBuffers(window);
        }
        glfwPollEvents();
    }

    glfwDestroyWindow(window8);
    glfwDestroyWindow(window10);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
