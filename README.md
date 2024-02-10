# 10-bit-OpenGL-GLFW
How to use 10-bit Display Mode using OpenGL/GLFW.

Was tested on the following configuration:

Graphic Adapter — NVIDIA Quadro RTX 4000
Monitor— EIZO RadiForce RX340

The Monitor shall be connected to Display Port.

The following settings needs to be changed:

![](assets/Settings.png)

The trick is to set 10 bit before OpenGL Window's creation:

```c
    wImage8 = glfwCreateWindow(ImageWidth, ImageHeight, "8-bit", NULL, NULL);

    glfwWindowHint(GLFW_RED_BITS, 10); //must be called before windows's creation
    glfwWindowHint(GLFW_GREEN_BITS, 10);
    glfwWindowHint(GLFW_BLUE_BITS, 10);
    glfwWindowHint(GLFW_ALPHA_BITS, 2);
    wImage10 = glfwCreateWindow(ImageWidth, ImageHeight, "10-bit", NULL, NULL);
```

And use GL_RGB10 for textures

```c
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_SHORT, pixelsU16);
```

There are four small projects, each one will open two windows - one in 8 bit mode, and another one in 10 bit, so you can compare both visually to check that banding effect disappeared on 10 bit.

#### Demo-10 bit

This one will show gradient ramp

![](assets/01_demo_app.png)

or real x-ray image:

![](assets/01_demo_app_01.png)

The settings will allow to change Min/Max (Center/Width) Levels and measure DDLs.

![](assets/nuklear.png)

#### Simple-10 bit

Just triangle from GLFW "Hello World":

![](assets/02_simple_app.png)

#### Cube-10 bit

Rotating Cube with gradients:

![](assets/03_cube_app.png)

#### CubeLight-10 bit

Rotating cube with lighting

![](assets/04_cube_light_app.png)
