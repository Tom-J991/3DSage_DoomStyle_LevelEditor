//--------------------------By: 3DSage------------------------------------------
//---------------------------Modified-------------------------------------------
//               https://www.youtube.com/3dsage
//                         version 1.0

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
// TODO: Get Sleep function in other Operating Systems.

// Standard Libraries
#include <math.h>
#include <stdio.h>

// OpenGL Libraries.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // OpenGL Platform/Window/Input Abstraction Layer
#include <glad/glad.h> // OpenGL Bindings

#include <cglm/cglm.h> // OpenGL Maths for C

#define res        1                        //0=160x120 1=360x240 4=640x480
#define SW         160*res                  //screen width
#define SH         120*res                  //screen height
#define SW2        (SW/2)                   //half of screen width
#define SH2        (SH/2)                   //half of screen height
#define pixelScale 4/res                    //OpenGL pixel scale
#define GLSW       (SW*pixelScale)          //OpenGL window width
#define GLSH       (SH*pixelScale)          //OpenGL window height

// Textures
#include "textures/T_NUMBERS.h"
#include "textures/T_VIEW2D.h"
#include "textures/T_00.h"
#include "textures/T_01.h"
#include "textures/T_02.h"
#include "textures/T_03.h"
#include "textures/T_04.h"
#include "textures/T_05.h"
#include "textures/T_06.h"
#include "textures/T_07.h"
#include "textures/T_08.h"
#include "textures/T_09.h"
#include "textures/T_10.h"
#include "textures/T_11.h"
#include "textures/T_12.h"
#include "textures/T_13.h"
#include "textures/T_14.h"
#include "textures/T_15.h"
#include "textures/T_16.h"
#include "textures/T_17.h"
#include "textures/T_18.h"
#include "textures/T_19.h"
int numText=19;                          //number of textures
int numSect= 0;                          //number of sectors
int numWall= 0;                          //number of walls

//------------------------------------------------------------------------------
typedef struct
{
    union
    {
        struct
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
        };
        unsigned int rgb;
    };
} RGB;

typedef struct 
{
 int fr1,fr2;           //frame 1 frame 2, to create constant frame rate
}time; time T;

typedef struct 
{
 float cos[360];        //Save sin cos in values 0-360 degrees 
 float sin[360];
}math; math M;

typedef struct 
{
 int w,s,a,d;           //move up, down, left, rigth
 int sl,sr;             //strafe left, right 
 int m;                 //move up, down, look up, down
}keys; keys K;

typedef struct 
{
 int x,y,z;             //player position. Z is up
 int a;                 //player angle of rotation left right
 int l;                 //variable to look up and down
}player; player P;

typedef struct 
{
 int x1,y1;             //bottom line point 1
 int x2,y2;             //bottom line point 2
 int wt,u,v;            //wall texture and u/v tile
 int shade;             //shade of the wall
}walls; walls W[256];

typedef struct 
{
 int ws,we;             //wall number start and end
 int z1,z2;             //height of bottom and top 
 int d;                 //add y distances to sort drawing order
 int st,ss;             //surface texture, surface scale 
 int surf[SW];          //to hold points for surfaces
}sectors; sectors S[128];

typedef struct 
{
 int w,h;                             //texture width/height
 const unsigned char *name;           //texture name
}TexureMaps; TexureMaps Textures[64]; //increase for more textures

typedef struct
{
 int mx,my;        //rounded mouse position
 int addSect;      //0=nothing, 1=add sector
 int wt,wu,wv;     //wall    texture, uv texture tile
 int st,ss;        //surface texture, surface scale 
 int z1,z2;        //bottom and top height
 int scale;        //scale down grid
 int move[4];      //0=wall ID, 1=v1v2, 2=wallID, 3=v1v2
 int selS,selW;    //select sector/wall
}grid; grid G;

// Global Variables
GLFWwindow *window;

unsigned int indiceCount = 0;

unsigned int VAO;
unsigned int VBO[2];
unsigned int EBO;
unsigned int PBO[2];

unsigned int vertexShader, fragmentShader, shaderProgram;
mat4 view, projection;

unsigned int texture;

unsigned char *imageBuffer;
unsigned char *framebuffer[4];
size_t fbuffer_count = 4;
size_t buffer_size;
unsigned int activeFramebuffer = 0;

// Callbacks
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
static void mousePositionCallback(GLFWwindow *window, double xpos, double ypos);

// Forward Declarations
void start();
void shutdown();

void initSharedMemory();

void runGame();
void initGame();
void tick();
void render();
void cleanupGame();

void initOpenGL();
void startOpenGLRender();
void endOpenGLRender();
void cleanupOpenGL();

void drawPixel(unsigned char *framebuffer, const int x, const int y, const RGB color);
void drawLine(unsigned char *framebuffer, float x1, float y1, float x2, float y2, int r, int g, int b);
void drawNumber(unsigned char *framebuffer, int nx, int ny, int n);

void draw2D();
void darken();
int shade(int w);

void initGrid();

void save();
void load();

//------------------------------------------------------------------------------

// Entry Point
int main(int argc, char *argv[])
{
    /*glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(GLSW/2,GLSH/2);
    glutInitWindowSize(GLSW,GLSH);
    glutCreateWindow("Grid2D v1.0 by:3DSage");
    glPointSize(pixelScale);                        //pixel size
    gluOrtho2D(0,GLSW,0,GLSH);                      //origin bottom left
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(KeysDown);
    glutKeyboardUpFunc(KeysUp);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMoving);
    glutMainLoop();*/

    start();
    shutdown();

    return 0;
}

// Function Definitions
void start()
{
    initSharedMemory();
    initOpenGL();
    runGame();
}
void shutdown()
{
    cleanupOpenGL();
}

void initSharedMemory()
{
    buffer_size = SW * SH * 3;

    // Create Image Buffer.
    imageBuffer = (unsigned char *)calloc(buffer_size, sizeof(unsigned char));

    // Create Frame Buffers.
    for (int i = 0; i < fbuffer_count; ++i)
        framebuffer[i] = (unsigned char *)calloc(buffer_size, sizeof(unsigned char));
}

void initOpenGL()
{
    // Constants
    const char *vertexCode =
        "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aTexCoords;\n"
        "out vec2 texCoord;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "	texCoord = aTexCoords;\n"
        "	gl_Position = projection * view * vec4(aPos.xy, 0.0f, 1.0);\n"
        "}\n";
    const char *fragmentCode =
        "#version 330 core\n"
        "out vec4 fragColor;\n"
        "in vec2 texCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "	vec4 color = texture(texture1, texCoord);\n"
        "	fragColor = color;\n"
        "}\n";

    const float vertices[] =
    {
        0.0f, 0.0f,
        SW, 0.0f,
        SW, SH,
        0.0f, SH
    };
    const float texCoords[] =
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    const unsigned int indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };
    indiceCount = sizeof(indices) / sizeof(unsigned int);

    // Setup OpenGL Context.
    glfwInit();

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(GLSW, GLSH, "Grid2D v1.0 (modified) by:3DSage", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSwapInterval(0); // Disable VSync.

    // Setup Callbacks.
    glfwSetKeyCallback(window, keyCallback);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePositionCallback);

    // Initialize OpenGL Scene.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    //for alpha overlay darken buttons 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0);
    glClearDepth(1.0f);

    glDepthFunc(GL_LEQUAL);
    glViewport(0, 0, GLSW, GLSH);

    glPointSize(pixelScale);

    // Create Shaders.
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCode, NULL);
    glCompileShader(vertexShader);

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCode, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create Quad Geometry.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(2, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create Texture.
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SW, SH, 0, GL_RGB, GL_UNSIGNED_BYTE, imageBuffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create Pixel Buffers.
    glGenBuffers(2, PBO);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer_size, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer_size, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // Setup Perspective Matrices.
    glm_mat4_identity(view);
    glm_translate(view, (vec3) { 0.0f, 0.0f, -1.0f });

    glm_ortho(0.0f, SW, 0.0f, SH, 0.1f, 1000.0f, projection);

    // Set Shader Uniforms.
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);
}
void startOpenGLRender()
{
    // Copy active framebuffer to image buffer.
    memcpy(imageBuffer, framebuffer[activeFramebuffer], buffer_size);

    // Get Dual Pixel Buffer index.
    static int index = 0;
    int nextIndex = 0;

    index = (index + 1) % 2;
    nextIndex = (index + 1) % 2;

    // Update Texture with Pixel Buffer.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO[index]);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SW, SH, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Copy Image Buffer to Pixel Buffer.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBO[nextIndex]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer_size, 0, GL_STREAM_DRAW);
    unsigned char *dst = (unsigned char *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (dst)
    {
        unsigned char *ptr = (unsigned char *)dst;
        for (size_t i = 0; i < buffer_size; ++i)
        {
            *ptr = imageBuffer[i];
            ++ptr;
        }

        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
void endOpenGLRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw quad.
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_INT, 0);

    // Unbind.
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, texture);
}
void cleanupOpenGL()
{
    // Destroy.
    for (int i = 0; i < fbuffer_count; ++i)
    {
        free(framebuffer[i]);
        framebuffer[i] = 0;
    }

    free(imageBuffer);
    imageBuffer = 0;

    glDeleteTextures(1, &texture);
    glDeleteBuffers(2, PBO);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void runGame()
{
    initGame();

    // Game Loop.
    const double targetFPS = -1; // Max renders per frame. -1 = Disable render frame cap.
    const double targetTicks = 20.0; // Maximum updates per frame.
    double timeBetweenFrames = 1.0f / targetTicks;

    double now = 0.0;
    double lastTime = glfwGetTime();
    double deltaTime = 0.0;
    double timer = lastTime;

    int ticks = 0;
    int frames = 0;

    bool shouldRender = false;
    while (!glfwWindowShouldClose(window))
    {
        now = glfwGetTime();
        deltaTime += (now - lastTime) / timeBetweenFrames;
        lastTime = now;

        if (targetFPS == -1)
            shouldRender = true;
        else
            shouldRender = false;


        while (deltaTime >= 1.0)
        {
            tick();
            ticks++;
            shouldRender = true;

            deltaTime -= 1.0;
        }

        if (targetFPS != -1)
        {
            double timeToSleep = (1 - deltaTime) * 1000.0 / targetTicks;
            #ifdef _WIN32
            Sleep(timeToSleep);
            #endif
        }

        if (shouldRender)
        {
            startOpenGLRender();
            render();
            endOpenGLRender();
            frames++;
        }

        if (glfwGetTime() - timer > 1.0)
        {
            timer += 1.0;

            printf("%i ticks, %i fps\n", ticks, frames);

            ticks = 0;
            frames = 0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanupGame();
}

void initGrid()
{
    //define grid globals
    G.scale = 4;                //scale down grid
    G.selS = 0, G.selW = 0;       //select sector, walls
    G.z1 = 0;   G.z2 = 40;        //sector bottom top height
    G.st = 1;   G.ss = 4;         //sector texture, scale
    G.wt = 0;   G.wu = 1; G.wv = 1; //wall texture, u,v
}
void initGame()
{
    int x;

    initGrid();

    //init player
    P.x = 32 * 9; P.y = 48; P.z = 30; P.a = 0; P.l = 0;    //init player variables

    //store sin/cos in degrees
    for (x = 0; x < 360; x++)                         //precalulate sin cos in degrees
    {
        M.cos[x] = cos(x / 180.0 * M_PI);
        M.sin[x] = sin(x / 180.0 * M_PI);
    }

    //define textures
    Textures[0].name = T_00; Textures[0].h = T_00_HEIGHT; Textures[0].w = T_00_WIDTH;
    Textures[1].name = T_01; Textures[1].h = T_01_HEIGHT; Textures[1].w = T_01_WIDTH;
    Textures[2].name = T_02; Textures[2].h = T_02_HEIGHT; Textures[2].w = T_02_WIDTH;
    Textures[3].name = T_03; Textures[3].h = T_03_HEIGHT; Textures[3].w = T_03_WIDTH;
    Textures[4].name = T_04; Textures[4].h = T_04_HEIGHT; Textures[4].w = T_04_WIDTH;
    Textures[5].name = T_05; Textures[5].h = T_05_HEIGHT; Textures[5].w = T_05_WIDTH;
    Textures[6].name = T_06; Textures[6].h = T_06_HEIGHT; Textures[6].w = T_06_WIDTH;
    Textures[7].name = T_07; Textures[7].h = T_07_HEIGHT; Textures[7].w = T_07_WIDTH;
    Textures[8].name = T_08; Textures[8].h = T_08_HEIGHT; Textures[8].w = T_08_WIDTH;
    Textures[9].name = T_09; Textures[9].h = T_09_HEIGHT; Textures[9].w = T_09_WIDTH;
    Textures[10].name = T_10; Textures[10].h = T_10_HEIGHT; Textures[10].w = T_10_WIDTH;
    Textures[11].name = T_11; Textures[11].h = T_11_HEIGHT; Textures[11].w = T_11_WIDTH;
    Textures[12].name = T_12; Textures[12].h = T_12_HEIGHT; Textures[12].w = T_12_WIDTH;
    Textures[13].name = T_13; Textures[13].h = T_13_HEIGHT; Textures[13].w = T_13_WIDTH;
    Textures[14].name = T_14; Textures[14].h = T_14_HEIGHT; Textures[14].w = T_14_WIDTH;
    Textures[15].name = T_15; Textures[15].h = T_15_HEIGHT; Textures[15].w = T_15_WIDTH;
    Textures[16].name = T_16; Textures[16].h = T_16_HEIGHT; Textures[16].w = T_16_WIDTH;
    Textures[17].name = T_17; Textures[17].h = T_17_HEIGHT; Textures[17].w = T_17_WIDTH;
    Textures[18].name = T_18; Textures[18].h = T_18_HEIGHT; Textures[18].w = T_18_WIDTH;
    Textures[19].name = T_19; Textures[19].h = T_19_HEIGHT; Textures[19].w = T_19_WIDTH;
}
void cleanupGame()
{

}

int tickCount = 0;
void tick()
{
    //move up, down, left, right
    if (K.a == 1 && K.m == 0) { P.a -= 4; if (P.a < 0) { P.a += 360; } }
    if (K.d == 1 && K.m == 0) { P.a += 4; if (P.a > 359) { P.a -= 360; } }
    int dx = M.sin[P.a] * 10.0;
    int dy = M.cos[P.a] * 10.0;
    if (K.w == 1 && K.m == 0) { P.x += dx; P.y += dy; }
    if (K.s == 1 && K.m == 0) { P.x -= dx; P.y -= dy; }
    //strafe left, right
    if (K.sr == 1) { P.x += dy; P.y -= dx; }
    if (K.sl == 1) { P.x -= dy; P.y += dx; }
    //move up, down, look up, look down
    if (K.a == 1 && K.m == 1) { P.l -= 1; }
    if (K.d == 1 && K.m == 1) { P.l += 1; }
    if (K.w == 1 && K.m == 1) { P.z -= 4; }
    if (K.s == 1 && K.m == 1) { P.z += 4; }

    tickCount++;
}

void render()
{
    draw2D();
    darken();
}

void drawPixel(unsigned char *framebuffer, const int x, const int y, const RGB color) //draw a pixel at x/y with rgb
{
    if (x > SW - 1 || x < 0 || y > SH - 1 || y < 0) // Only draw pixel within buffer resolution.
        return;

    int xx = x * 3;
    int yy = y * 3;
    int index = xx + yy * SW;

    framebuffer[index++] = color.r;
    framebuffer[index++] = color.g;
    framebuffer[index++] = color.b;
}
void drawLine(unsigned char *framebuffer, float x1,float y1,float x2,float y2, int r,int g,int b)
{
    int n;
    float x=x2-x1;
    float y=y2-y1;
    float max=fabs(x); if(fabs(y)>max){ max=fabs(y);}
    x /= max; y /= max;
    for(n=0;n<max;n++)
    {
        drawPixel(framebuffer, x1, y1, (RGB) { r, g, b });
        x1+=x; y1+=y;
    }
}
void drawNumber(unsigned char *framebuffer, int nx,int ny, int n)
{
    int x,y;
    for(y=0;y<5;y++)
    { 
        int y2=((5-y-1)+5*n)*3*12;
        for(x=0;x<12;x++)
        { 
            int x2=x*3;
            if(T_NUMBERS[y2+x2]==0){ continue; }
            drawPixel(framebuffer, x + nx, y + ny, (RGB) { 255, 255, 255 });
        }
    }
}

void save() //save file
{
    int w, s;
    FILE *fp = fopen("./levels/level", "w");
    if (fp == NULL) { printf("Error opening level.\n"); return; }
    if (numSect == 0) { fclose(fp); return; } //nothing, clear file 

    fprintf(fp, "%i\n", numSect); //number of sectors 
    for (s = 0; s < numSect; s++)      //save sector
    {
        fprintf(fp, "%i %i %i %i %i %i\n", S[s].ws, S[s].we, S[s].z1, S[s].z2, S[s].st, S[s].ss);
    }

    fprintf(fp, "%i\n", numWall); //number of walls
    for (w = 0; w < numWall; w++)      //save walls
    {
        fprintf(fp, "%i %i %i %i %i %i %i %i\n", W[w].x1, W[w].y1, W[w].x2, W[w].y2, W[w].wt, W[w].u, W[w].v, W[w].shade);
    }
    fprintf(fp, "\n%i %i %i %i %i\n", P.x, P.y, P.z, P.a, P.l); //player position 
    fclose(fp);
}
void load()
{
    FILE *fp = fopen("./levels/level", "r");
    if (fp == NULL) { printf("Error opening level.\n"); return; }
    int s, w;

    fscanf(fp, "%i", &numSect);   //number of sectors 
    for (s = 0; s < numSect; s++)      //load all sectors
    {
        fscanf(fp, "%i", &S[s].ws);
        fscanf(fp, "%i", &S[s].we);
        fscanf(fp, "%i", &S[s].z1);
        fscanf(fp, "%i", &S[s].z2);
        fscanf(fp, "%i", &S[s].st);
        fscanf(fp, "%i", &S[s].ss);
    }
    fscanf(fp, "%i", &numWall);   //number of walls 
    for (s = 0; s < numWall; s++)      //load all walls
    {
        fscanf(fp, "%i", &W[s].x1);
        fscanf(fp, "%i", &W[s].y1);
        fscanf(fp, "%i", &W[s].x2);
        fscanf(fp, "%i", &W[s].y2);
        fscanf(fp, "%i", &W[s].wt);
        fscanf(fp, "%i", &W[s].u);
        fscanf(fp, "%i", &W[s].v);
        fscanf(fp, "%i", &W[s].shade);
    }
    fscanf(fp, "%i %i %i %i %i", &P.x, &P.y, &P.z, &P.a, &P.l); //player position, angle, look direction 
    fclose(fp);
}

void draw2D()
{
    int s,w,x,y,c;
    //draw background color
    for(y=0;y<120;y++)
    { 
        int y2=(SH-y-1)*3*160; //invert height, x3 for rgb, x15 for texture width
        for(x=0;x<160;x++)
        { 
            int pixel=x*3+y2;
            int r=T_VIEW2D[pixel+0]; 
            int g=T_VIEW2D[pixel+1]; 
            int b=T_VIEW2D[pixel+2];
            if(G.addSect>0 && y>48-8 && y<56-8 && x>144){ r=r>>1; g=g>>1; b=b>>1;} //darken sector button
            drawPixel(framebuffer[0], x, y, (RGB) { r, g, b });
        }
    }

    //draw sectors
    for(s=0;s<numSect;s++)
    {
        for(w=S[s].ws;w<S[s].we;w++)
        {
            if(s==G.selS-1) //if this sector is selected
            {
                //set sector to globals
                S[G.selS-1].z1=G.z1;
                S[G.selS-1].z2=G.z2;
                S[G.selS-1].st=G.st;
                S[G.selS-1].ss=G.ss;
                //yellow select
                if(G.selW==0)                   { c=80;} //all walls yellow
                else if(G.selW+S[s].ws-1==w)    { c=80; W[w].wt=G.wt; W[w].u=G.wu; W[w].v=G.wv;} //one wall selected
                else                            { c= 0;} //grey walls
            }
            else { c=0;} //sector not selected, grey

            drawLine(framebuffer[0], W[w].x1/G.scale,W[w].y1/G.scale, W[w].x2/G.scale,W[w].y2/G.scale,128+c,128+c,128-c);
            drawPixel(framebuffer[0], W[w].x1 / G.scale, W[w].y1 / G.scale, (RGB) { 255, 255, 255 });
            drawPixel(framebuffer[0], W[w].x2 / G.scale, W[w].y2 / G.scale, (RGB) { 255, 255, 255 });
        }
    } 
 
    //draw player
    int dx=M.sin[P.a]*12; 
    int dy=M.cos[P.a]*12; 
    drawPixel(framebuffer[0], P.x / G.scale, P.y / G.scale, (RGB) { 0, 255, 0 });
    drawPixel(framebuffer[0], (P.x + dx) / G.scale, (P.y + dy) / G.scale, (RGB) { 0, 175, 0 });

    //draw wall texture
    float tx=0, tx_stp=Textures[G.wt].w/15.0;
    float ty=0, ty_stp=Textures[G.wt].h/15.0;
    for(y=0;y<15;y++)
    { 
        tx=0;
        for(x=0;x<15;x++)
        { 
            int x2=(int)tx%Textures[G.wt].w; tx+=tx_stp;//*G.wu;
            int y2=(int)ty%Textures[G.wt].h; 
            int r=Textures[G.wt].name[(Textures[G.wt].h-y2-1)*3*Textures[G.wt].w+x2*3+0]; 
            int g=Textures[G.wt].name[(Textures[G.wt].h-y2-1)*3*Textures[G.wt].w+x2*3+1]; 
            int b=Textures[G.wt].name[(Textures[G.wt].h-y2-1)*3*Textures[G.wt].w+x2*3+2]; 
            drawPixel(framebuffer[0], x + 145, y + 105 - 8, (RGB) { r, g, b });
        } 
        ty+=ty_stp;//*G.wv;
    }
    //draw surface texture
    tx=0, tx_stp=Textures[G.st].w/15.0;
    ty=0, ty_stp=Textures[G.st].h/15.0;
    for(y=0;y<15;y++)
    { 
        tx=0;
        for(x=0;x<15;x++)
        { 
            int x2=(int)tx%Textures[G.st].w; tx+=tx_stp;//*G.ss;
            int y2=(int)ty%Textures[G.st].h; 
            int r=Textures[G.st].name[(Textures[G.st].h-y2-1)*3*Textures[G.st].w+x2*3+0]; 
            int g=Textures[G.st].name[(Textures[G.st].h-y2-1)*3*Textures[G.st].w+x2*3+1]; 
            int b=Textures[G.st].name[(Textures[G.st].h-y2-1)*3*Textures[G.st].w+x2*3+2]; 
            drawPixel(framebuffer[0], x + 145, y + 105 - 24 - 8, (RGB) { r, g, b });
        } 
        ty+=ty_stp;//*G.ss;
    }
    //draw numbers
    drawNumber(framebuffer[0], 140,90,G.wu);   //wall u
    drawNumber(framebuffer[0], 148,90,G.wv);   //wall v
    drawNumber(framebuffer[0], 148,66,G.ss);   //surface v
    drawNumber(framebuffer[0], 148,58,G.z2);   //top height
    drawNumber(framebuffer[0], 148,50,G.z1);   //bottom height
    drawNumber(framebuffer[0], 148,26,G.selS); //sector number
    drawNumber(framebuffer[0], 148,18,G.selW); //wall number
}

int dark = 0; //darken buttons
void darken()                       //draw a pixel at x/y with rgb
{
    int x,y, xs,xe, ys,ye;
    if(dark== 0){ return;}             //no buttons were clicked
    if(dark== 1){ xs= 0; xe=15; ys= 0/G.scale; ye=32/G.scale;} //save button
    if(dark== 2){ xs= 0; xe= 3; ys=96/G.scale; ye=128/G.scale;} //u left
    if(dark== 3){ xs= 4; xe= 8; ys=96/G.scale; ye=128/G.scale;} //u right
    if(dark== 4){ xs= 7; xe=11; ys=96/G.scale; ye=128/G.scale;} //v left
    if(dark== 5){ xs=11; xe=15; ys=96/G.scale; ye=128/G.scale;} //u right
    if(dark== 6){ xs= 0; xe= 8; ys=192/G.scale; ye=224/G.scale;} //u left
    if(dark== 7){ xs= 8; xe=15; ys=192/G.scale; ye=224/G.scale;} //u right
    if(dark== 8){ xs=0; xe= 7; ys=224/G.scale; ye=256/G.scale;} //Top left
    if(dark== 9){ xs=7; xe=15; ys=224/G.scale; ye=256/G.scale;} //Top right
    if(dark==10){ xs=0; xe= 7; ys=256/G.scale; ye=288/G.scale;} //Bot left
    if(dark==11){ xs=7; xe=15; ys=256/G.scale; ye=288/G.scale;} //Bot right
    if(dark==12){ xs=0; xe= 7; ys=352/G.scale; ye=386/G.scale;} //sector left
    if(dark==13){ xs=7; xe=15; ys=352/G.scale; ye=386/G.scale;} //sector right
    if(dark==14){ xs=0; xe= 7; ys=386/G.scale; ye=416/G.scale;} //wall left
    if(dark==15){ xs=7; xe=15; ys=386/G.scale; ye=416/G.scale;} //wall right
    if(dark==16){ xs=0; xe=15; ys=416/G.scale; ye=448/G.scale;} //delete
    if(dark==17){ xs=0; xe=15; ys=448/G.scale; ye=480/G.scale;} //load

    for(y=ys;y<ye;y++)
    {
        for(x=xs;x<xe;x++)
        {
            glColor4f(0,0,0,0.4);
            glBegin(GL_POINTS);
            glVertex2i(x*pixelScale+2+580,(120-y)*pixelScale);
            glEnd();
        }
    }
}

int shade(int w)    //automatic shading
{
    float ang = atan2f(W[w].y2 - W[w].y1, W[w].x2 - W[w].x1);
    ang = (ang * 180) / M_PI;      //radians to degrees
    if (ang < 0) { ang += 360; }    //correct negative
    int shade = ang;           //shading goes from 0-90-0-90-0
    if (shade > 180) { shade = 180 - (shade - 180); }
    if (shade > 90) { shade = 90 - (shade - 90); }
    return shade * 0.75;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Player Input.
    switch (action)
    {
        case GLFW_PRESS:
        {
            if (key == GLFW_KEY_W)
                K.w = true;
            if (key == GLFW_KEY_A)
                K.a = true;
            if (key == GLFW_KEY_S)
                K.s = true;
            if (key == GLFW_KEY_D)
                K.d = true;
            if (key == GLFW_KEY_COMMA)
                K.sl = true;
            if (key == GLFW_KEY_PERIOD)
                K.sr = true;
            if (key == GLFW_KEY_M)
                K.m = true;
        } break;
        case GLFW_RELEASE:
        {
            if (key == GLFW_KEY_W)
                K.w = false;
            if (key == GLFW_KEY_A)
                K.a = false;
            if (key == GLFW_KEY_S)
                K.s = false;
            if (key == GLFW_KEY_D)
                K.d = false;
            if (key == GLFW_KEY_COMMA)
                K.sl = false;
            if (key == GLFW_KEY_PERIOD)
                K.sr = false;
            if (key == GLFW_KEY_M)
                K.m = false;
        } break;
        default:
        {
        } return;
    }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    // Get mouse position and cast it to integer.
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    int x = (int)xPos;
    int y = (int)yPos;

    int s,w;
    //round mouse x,y
    G.mx=x/pixelScale; 
    G.my=SH-y/pixelScale;
    G.mx=((G.mx+4)>>3)<<3; 
    G.my=((G.my+4)>>3)<<3; //nearest 8th 

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        //2D view buttons only
        if(x>580) 
        {
            //2d 3d view buttons
            if(y>0 && y<32){ save(); dark=1;}		
            //wall texture
            if(y>32 && y<96){ if(x<610){ G.wt-=1; if(G.wt<0){ G.wt=numText;}} else{ G.wt+=1; if(G.wt>numText){ G.wt=0;}}} 	
            //wall uv
            if(y>96 && y<128) 
            { 
                if(x<595)       { dark=2; G.wu-=1; if(G.wu< 1){ G.wu= 1;}} 
                else if(x<610)  { dark=3; G.wu+=1; if(G.wu> 9){ G.wu= 9;}}
                else if(x<625)  { dark=4; G.wv-=1; if(G.wv< 1){ G.wv= 1;}} 
                else if(x<640)  { dark=5; G.wv+=1; if(G.wv> 9){ G.wv= 9;}}
            }
            //surface texture
            if(y>128 && y<192){ if(x<610){ G.st-=1; if(G.st<0){ G.st=numText;}} else{ G.st+=1; if(G.st>numText){ G.st=0;}}} 
            //surface uv
            if(y>192 && y<222) 
            { 
                if(x<610)   { dark=6; G.ss-=1; if(G.ss< 1){ G.ss= 1;}} 
                else        { dark=7; G.ss+=1; if(G.ss> 9){ G.ss= 9;}}
            }
            //top height
            if(y>222 && y<256)
            { 
                if(x<610){ dark=8; G.z2-=5; if(G.z2==G.z1){ G.z1-=5;}} 
                else     { dark=9; G.z2+=5;}
            }
            //bot height
            if(y>256 && y<288)
            { 
                if(x<610){ dark=10; G.z1-=5;} 
                else     { dark=11; G.z1+=5; if(G.z1==G.z2){ G.z2+=5;}}
            }
            //add sector
            if(y>288 && y<318){ G.addSect+=1; G.selS=0; G.selW=0; if(G.addSect>1){ G.addSect=0;}}
            //limit
            if(G.z1<0){ G.z1=0;} if(G.z1>145){ G.z1=145;}
            if(G.z2<5){ G.z2=5;} if(G.z2>150){ G.z2=150;}

            //select sector
            if(y>352 && y<386)
            { 
                G.selW=0; 
                if(x<610){ dark=12; G.selS-=1; if(G.selS<0){ G.selS=numSect;}} 
                else     { dark=13; G.selS+=1; if(G.selS>numSect){ G.selS=0;}}
                int s=G.selS-1;
                G.z1=S[s].z1; //sector bottom height
                G.z2=S[s].z2; //sector top height
                G.st=S[s].st; //surface texture
                G.ss=S[s].ss; //surface scale
                G.wt=W[S[s].ws].wt;
                G.wu=W[S[s].ws].u;
                G.wv=W[S[s].ws].v; 
                if(G.selS==0){ initGrid();} //defaults 
            }
            //select sector's walls 
            int snw=S[G.selS-1].we-S[G.selS-1].ws; //sector's number of walls
            if(y>386 && y<416)
            { 
                if(x<610) //select sector wall left
                { 
                    dark=14;
                    G.selW-=1; if(G.selW<0){ G.selW=snw;} 
                } 
                else //select sector wall right
                { 
                    dark=15;
                    G.selW+=1; if(G.selW>snw){ G.selW=0;} 
                }
                if(G.selW>0)
                {
                    G.wt=W[S[G.selS-1].ws+G.selW-1].wt; //printf("ws,%i,%i\n",G.wt, 1 );
                    G.wu=W[S[G.selS-1].ws+G.selW-1].u;
                    G.wv=W[S[G.selS-1].ws+G.selW-1].v; 
                }
            }
            //delete
            if(y>416 && y<448)
            { 
                dark=16;
                if(G.selS>0)
                {
                    int d=G.selS-1;                             //delete this one
	                //printf("%i before:%i,%i\n",d, numSect,numWall);
                    numWall-=(S[d].we-S[d].ws);                 //first subtract number of walls
                    for(x=d;x<numSect;x++){ S[x]=S[x+1];}       //remove from array
                    numSect-=1;                                 //1 less sector
                    G.selS=0; G.selW=0;                         //deselect
                    //printf("after:%i,%i\n\n",numSect,numWall);
                }
            }

            //load
            if(y>448 && y<480){ dark=17; load();}
        }
        //clicked on grid
        else
        {
            //init new sector
            if(G.addSect==1)
            { 
                S[numSect].ws=numWall;                                   //clear wall start
                S[numSect].we=numWall+1;                                 //add 1 to wall end
                S[numSect].z1=G.z1;
                S[numSect].z2=G.z2;
                S[numSect].st=G.st;
                S[numSect].ss=G.ss;
                W[numWall].x1=G.mx*G.scale; W[numWall].y1=G.my*G.scale;  //x1,y1 
                W[numWall].x2=G.mx*G.scale; W[numWall].y2=G.my*G.scale;  //x2,y2
                W[numWall].wt=G.wt;
                W[numWall].u=G.wu;
                W[numWall].v=G.wv;
                numWall+=1;                                              //add 1 wall
                numSect+=1;                                              //add this sector
                G.addSect=3;                                             //go to point 2
            }   
            //add point 2
            else if(G.addSect==3)
            {
                if(S[numSect-1].ws==numWall-1 && G.mx*G.scale<=W[S[numSect-1].ws].x1)
                {
                    numWall-=1; numSect-=1; G.addSect=0; 
                    printf("walls must be counter clockwise\n");
                    return;
                }

	            //point 2
                W[numWall-1].x2=G.mx*G.scale; W[numWall-1].y2=G.my*G.scale; //x2,y2
                //automatic shading 
                float ang = atan2f( W[numWall-1].y2-W[numWall-1].y1, W[numWall-1].x2-W[numWall-1].x1 );
                ang=(ang*180)/M_PI;      //radians to degrees
                if(ang<0){ ang+=360;}    //correct negative
                int shade=ang;           //shading goes from 0-90-0-90-0
                if(shade>180){ shade=180-(shade-180);}
                if(shade> 90){ shade= 90-(shade- 90);}
                W[numWall-1].shade=shade;

                //check if sector is closed
                if(W[numWall-1].x2==W[S[numSect-1].ws].x1 && W[numWall-1].y2==W[S[numSect-1].ws].y1)
                {     
                    W[numWall-1].wt=G.wt;
                    W[numWall-1].u=G.wu;
                    W[numWall-1].v=G.wv;
                    G.addSect=0; 
                }
                //not closed, add new wall
                else
                {
                    //init next wall
                    S[numSect-1].we+=1;                                      //add 1 to wall end
                    W[numWall].x1=G.mx*G.scale; W[numWall].y1=G.my*G.scale;  //x1,y1 
                    W[numWall].x2=G.mx*G.scale; W[numWall].y2=G.my*G.scale;  //x2,y2
                    W[numWall-1].wt=G.wt;
                    W[numWall-1].u=G.wu;
                    W[numWall-1].v=G.wv;
                    W[numWall].shade=0;
                    numWall+=1;                                              //add 1 wall
                }
            }
        }
    }

    //clear variables to move point
    for(w=0;w<4;w++){ G.move[w]=-1;}
 
    if(G.addSect==0 && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        //move point hold id 
        for(s=0;s<numSect;s++)
        { 
            for(w=S[s].ws;w<S[s].we;w++)
            {
                int x1=W[w].x1, y1=W[w].y1;
                int x2=W[w].x2, y2=W[w].y2;
                int mx=G.mx*G.scale, my=G.my*G.scale;
                if(mx<x1+3 && mx>x1-3 && my<y1+3 && my>y1-3){ G.move[0]=w; G.move[1]=1;}
                if(mx<x2+3 && mx>x2-3 && my<y2+3 && my>y2-3){ G.move[2]=w; G.move[3]=2;}
            }
        }
    } 

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){ dark=0;}
}
static void mousePositionCallback(GLFWwindow *window, double xpos, double ypos)
{
    // glutMotionFunc callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.

    int x = (int)xpos;
    int y = (int)ypos;

    if(x<580 && G.addSect==0 && G.move[0]>-1)
    {
        int Aw=G.move[0], Ax=G.move[1];
        int Bw=G.move[2], Bx=G.move[3];
        if(Ax==1){ W[Aw].x1=((x+16)>>5)<<5; W[Aw].y1=((GLSH-y+16)>>5)<<5;}
        if(Ax==2){ W[Aw].x2=((x+16)>>5)<<5; W[Aw].y2=((GLSH-y+16)>>5)<<5;}
        if(Bx==1){ W[Bw].x1=((x+16)>>5)<<5; W[Bw].y1=((GLSH-y+16)>>5)<<5;}
        if(Bx==2){ W[Bw].x2=((x+16)>>5)<<5; W[Bw].y2=((GLSH-y+16)>>5)<<5;}  
    }
}
