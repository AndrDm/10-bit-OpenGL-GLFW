//========================================================================
// Simple GLFW example
// Rotated Cube from https://github.com/derhass/HelloCube
// Shaders from https://learnopengl.com/Lighting/Basic-Lighting
// Changed by AD 04-FEB-2024
//========================================================================

#include "..\include\glad_gl.h"
#include "..\include\glfw3.h"

#include "..\include\glm\mat4x4.hpp"
#include "..\include\glm\gtc\type_ptr.hpp"
#include "..\include\glm\gtx\transform.hpp"
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

// We use the following layout for vertex data
typedef struct {
    GLfloat pos[3]; // 3D cartesian coordinates
    GLfloat clr[4]; // RGBA (floats, because we need 10 bit rendering)
} Vertex;

static const Vertex cubeGeometry[] = {
    //  X     Y     Z       R     G     B     A
    // front face
    {{-1.0, -1.0,  1.0},  {.25f, .25f, .25f, 1.0f}},
    {{ 1.0, -1.0,  1.0},  {.23f, .23f, .23f, 1.0f}},
    {{-1.0,  1.0,  1.0},  {.20f, .20f, .20f, 1.0f}},
    {{ 1.0,  1.0,  1.0},  {.20f, .20f, .20f, 1.0f}},
    // back face
    {{ 1.0, -1.0, -1.0},  {.35f, .35f, .35f, 1.0f}},
    {{-1.0, -1.0, -1.0},  {.35f, .35f, .35f, 1.0f}},
    {{ 1.0,  1.0, -1.0},  {.30f, .30f, .30f, 1.0f}},
    {{-1.0,  1.0, -1.0},  {.30f, .30f, .30f, 1.0f}},
    // left  face
    {{-1.0, -1.0, -1.0},  {.45f, .45f, .45f, 1.0f}},
    {{-1.0, -1.0,  1.0},  {.45f, .45f, .45f, 1.0f}},
    {{-1.0,  1.0, -1.0},  {.40f, .40f, .40f, 1.0f}},
    {{-1.0,  1.0,  1.0},  {.40f, .40f, .40f, 1.0f}},
    // right face
    {{ 1.0, -1.0,  1.0},  {.55f, .55f, .55f, 1.0f}},
    {{ 1.0, -1.0, -1.0},  {.55f, .55f, .55f, 1.0f}},
    {{ 1.0,  1.0,  1.0},  {.50f, .50f, .50f, 1.0f}},
    {{ 1.0,  1.0, -1.0},  {.50f, .50f, .50f, 1.0f}},
    // top face
    {{-1.0,  1.0,  1.0},  {.65f, .65f, .65f, 1.0f}},
    {{ 1.0,  1.0,  1.0},  {.65f, .65f, .65f, 1.0f}},
    {{-1.0,  1.0, -1.0},  {.60f, .60f, .60f, 1.0f}},
    {{ 1.0,  1.0, -1.0},  {.60f, .60f, .60f, 1.0f}},
    // bottom face
    {{ 1.0, -1.0,  1.0},  {.75f, .75f, .75f, 1.0f}},
    {{-1.0, -1.0,  1.0},  {.75f, .75f, .75f, 1.0f}},
    {{ 1.0, -1.0, -1.0},  {.70f, .70f, .70f, 1.0f}},
    {{-1.0, -1.0, -1.0},  {.70f, .70f, .70f, 1.0f}},
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

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

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

static const char* vertex_shader_light =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"    FragPos = vec3(model * vec4(aPos, 1.0));\n"
"    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
"}\n";

static const char* fragment_shader_light =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"uniform vec3 objectColor;\n"
"void main()\n"
"{\n"
"    // ambient\n"
"    float ambientStrength = 0.1;\n"
"    vec3 ambient = ambientStrength * lightColor;\n"
"    // diffuse \n"
"    vec3 norm = normalize(Normal);\n"
"    vec3 lightDir = normalize(lightPos - FragPos);\n"
"    float diff = max(dot(norm, lightDir), 0.0);\n"
"    vec3 diffuse = diff * lightColor;\n"
"    // specular\n"
"    float specularStrength = 0.5;\n"
"    vec3 viewDir = normalize(viewPos - FragPos);\n"
"    vec3 reflectDir = reflect(-lightDir, norm);\n"
"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
"    vec3 specular = specularStrength * spec * lightColor;\n"
"    vec3 result = (ambient + diffuse + specular) * objectColor;\n"
"    FragColor = vec4(result, 1.0);\n"
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
    GLFWwindow* window8, * window10, *wControl;
    GLuint vertex_shader, fragment_shader, vertex_light, fragment_light, program, program_light;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) exit(EXIT_FAILURE); //initializing GLFW
   
    wControl = glfwCreateWindow(wWidth * 2, 100, "Control", NULL, NULL);
    window8 = glfwCreateWindow(wWidth, wHeight, "8-bit", NULL, NULL);
    glfwWindowHint(GLFW_RED_BITS, 10); //must be called before windows's creation
    glfwWindowHint(GLFW_GREEN_BITS, 10);
    glfwWindowHint(GLFW_BLUE_BITS, 10);
    glfwWindowHint(GLFW_ALPHA_BITS, 2);
    window10 = glfwCreateWindow(wWidth, wHeight, "10-bit", NULL, NULL);

    if (!window8 || !window10 || !wControl) { glfwTerminate(); exit(EXIT_FAILURE); }

    glfwSetKeyCallback(window8, key_callback);
    glfwSetKeyCallback(window10, key_callback);
    glfwSetKeyCallback(wControl, key_callback);
    glfwSetFramebufferSizeCallback(window8, callback_Resize);

    glfwSetWindowPos(window8, 0, 32);
    glfwSetWindowPos(window10, wWidth, 32);
    glfwSetWindowPos(wControl, 0, wHeight + 64);

    glfwMakeContextCurrent(wControl);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1); // ask the driver to enable synchronizing the buffer swaps to the *VSync of the display.

    nk = nk_glfw3_init(wControl, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

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

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, pos)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_cube, NULL);
        glCompileShader(vertex_shader);

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_cube, NULL);
        glCompileShader(fragment_shader);

        vertex_light = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_light, 1, &vertex_shader_light, NULL);
        glCompileShader(vertex_light);

        fragment_light = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_light, 1, &fragment_shader_light, NULL);
        glCompileShader(fragment_light);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        program_light = glCreateProgram();
        glAttachShader(program_light, vertex_light);
        glAttachShader(program_light, fragment_light);
        glLinkProgram(program_light);
    }

    double timeCur = glfwGetTime();
    glm::mat4 projection, view, model = glm::mat4(1.0f);
    float intensity = .5f, speed = .25f, scale = 65.0f;

    while (!glfwWindowShouldClose(window8) && !glfwWindowShouldClose(window10) && !glfwWindowShouldClose(wControl))
    {
        double now = glfwGetTime();
        double timeDelta = now - timeCur;
        timeCur = now;

        for (int i = 0; i < 2; i++) {
            GLFWwindow* window;
            int width, height;

            //---Control---
            glfwMakeContextCurrent(wControl);
            int widthControl, heightControl;
            glfwGetWindowSize(wControl, &widthControl, &heightControl);
            struct nk_rect area = nk_rect(0.f, 0.f, (float)widthControl, (float)heightControl);
            nk_glfw3_new_frame();
            bool need_reset = false;
            if (nk_begin(nk, "", area, 0)) {
                nk_layout_row_dynamic(nk, 32, 6);
                nk_label(nk, "Intensity", NK_TEXT_LEFT); nk_label(nk, "", NK_TEXT_LEFT);
                nk_label(nk, "Speed", NK_TEXT_LEFT); nk_label(nk, "", NK_TEXT_LEFT);
                nk_label(nk, "Scale", NK_TEXT_LEFT); nk_label(nk, "", NK_TEXT_LEFT);
                nk_slider_float(nk, 0.f, &intensity, 1.f, 0.01f); nk_labelf(nk, NK_TEXT_LEFT, "%0.2f", intensity);
                nk_slider_float(nk, 0.f, &speed, 1.f, 0.01f); nk_labelf(nk, NK_TEXT_LEFT, "%0.2f", speed);
                nk_slider_float(nk, 0.f, &scale, 100.f, 1.0f); nk_labelf(nk, NK_TEXT_LEFT, "%0.2f", scale);
            }
            nk_end(nk);
            nk_glfw3_render(NK_ANTI_ALIASING_ON);
            glfwSwapBuffers(wControl);

            window = i ? window10 : window8;
            glfwMakeContextCurrent(window);
            glfwGetFramebufferSize(window, &width, &height);
            float ratio = width / (float)height;

            model = glm::rotate(model, (float)(glm::half_pi<double>() * timeDelta * speed), glm::vec3(0.8f, 0.6f, 0.1f));
            projection = glm::perspective(glm::radians(scale), (float)width / (float)height, 0.1f, 10.0f);
            view = glm::translate(glm::vec3(0.0f, 0.0f, -4.0f));

            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(.3f, .3f, .3f, 1.f);

            glUseProgram(program_light);

            // Lighting
            glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
            glm::vec3 viewPos(0.0f, 0.0f, 10.0f);
            glm::vec3 lightColor(intensity, intensity, intensity);
            glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

            glUniform3fv(glGetUniformLocation(program_light, "objectColor"), 1, glm::value_ptr(objectColor));
            glUniform3fv(glGetUniformLocation(program_light, "lightColor"), 1, glm::value_ptr(lightColor));
            glUniform3fv(glGetUniformLocation(program_light, "lightPos"), 1, glm::value_ptr(lightPos));
            glUniform3fv(glGetUniformLocation(program_light, "viewPos"), 1, glm::value_ptr(viewPos));

            glUniformMatrix4fv(glGetUniformLocation(program_light, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(program_light, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(program_light, "model"), 1, GL_FALSE, glm::value_ptr(model));

            // Draw the cube
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

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
