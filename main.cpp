#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;
bool started = false;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix, lMatrix, rMatrix, bMatrix;

//float player_x = 0;
//float player_rotate = 0;
//replace these with vectors

//Start at 0, 0, 0
glm::vec3 left_position = glm::vec3(-4.8, 0, 0);
glm::vec3 right_position = glm::vec3(4.8, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);

//dont go anywhere (yet) (stores how much they want to move)
glm::vec3 left_movement = glm::vec3(0, 0, 0);
glm::vec3 right_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

float player_speed = 4.0f; //player moves one unit per second
float ball_speed = 2.0f;
float changex = 1.0f;
float changey = 1.0f;

GLuint ballTextureID;
GLuint playerTextureID;

//when using images, you should use something that is 64x64, or 128x128, 1024x1024
//avoid odd resolutions and things that aren't multiplies of 16
GLuint LoadTexture(const char* filePath) { //returns ID number of a texture
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha); //loading the png file off the hard drive and into ram

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID); //create the texture ID
    glBindTexture(GL_TEXTURE_2D, textureID); //now bind that texture ID

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image); //here is the raw pixel data, put it onto the video card and heres the id number

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //use the nearest filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image); //freeing from our main memory since it's already on the video card memory
    return textureID; //now it returns the texture id
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl"); // Setting up the shader program
    //loads a shader program that can handle textured polygons

    viewMatrix = glm::mat4(1.0f);
    lMatrix = glm::mat4(1.0f); //applies this identity matrix to the triangle
    rMatrix = glm::mat4(1.0f);
    bMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);


    //this is the drawing of the object
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //program.SetColor(1.0f, 0.0f, 0.0f, 1.0f); //set the color to red

    glUseProgram(program.programID); //use this shader program

    glClearColor(0.0f, 0.2f, 0.2f, 1.0f); //whenever you go to draw something, use this background color
    glEnable(GL_BLEND);

    //good for setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    playerTextureID = LoadTexture("ikea.png"); //put in the address of the picture
    ballTextureID = LoadTexture("pog.png");
}

void ProcessInput() {

    left_movement = glm::vec3(0);
    right_movement = glm::vec3(0);
    ball_movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE: //both quit or close, stop game 
                gameIsRunning = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) { //.. now we are going to be looking for which key was it
                    case SDLK_SPACE:
                        started = true;
                        break;
                    }
                    break; //SDL_KEYDOWN

          
        }
    }

    //not inside the while loop
    //this is to help with a key being held down
    const Uint8* keys = SDL_GetKeyboardState(NULL); //pointer to array

    if (keys[SDL_SCANCODE_W]) { //if the left key is down rn (being held down_
        left_movement.y = 1.0f;
    }

    else if (keys[SDL_SCANCODE_S]) {
        left_movement.y = -1.0f;
    }

    if (keys[SDL_SCANCODE_UP]) { //if the left key is down rn (being held down_
      /*  if (right_position.y > 3.75 - (right_position.y/2))
            right_movement.y = 0.0f;
        else*/
            right_movement.y = 1.0f;
    }

    else if (keys[SDL_SCANCODE_DOWN]) {
           right_movement.y = -1.0f;
    }

    //makes sure that if both keys are held down, you aren't moving faster in some way
    //not move faster along a diagonal, simulates motion on a joystick
    if (glm::length(ball_movement) > 1.0f) {
        ball_movement = glm::normalize(ball_movement);
    }
}

float lastTicks = 0.0f; //time since last frame

//update only runs as fast as your computer can run it
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    //when someone misses the ball
    if (ball_position.x > 5 - .35 || ball_position.x < -5 + .35){
        left_position += 0;
        right_position += 0;
        ball_position += 0;
    }
    
    //game is still going
    else {
        //Add (direction * units per second * elapsed time) //add by how much they want to move by how fast they want to move (in deltaTime)
        //left player
        left_position += left_movement * player_speed * deltaTime;

        if (left_position.y >= 3.75 - .5) {
            left_position -= left_movement * player_speed * deltaTime;
        }
        if (left_position.y < -3.75 + .5) {
            left_position -= left_movement * player_speed * deltaTime;
        }

        lMatrix = glm::mat4(1.0f);
        lMatrix = glm::translate(lMatrix, left_position);
        lMatrix = glm::rotate(lMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        lMatrix = glm::scale(lMatrix, glm::vec3(1.0f, .5f, 1.0f));
        //reset to identity matrix, and translate it to new position


        //right player
        //Add (direction * units per second * elapsed time) //add by how much they want to move by how fast they want to move (in deltaTime)
        right_position += right_movement * player_speed * deltaTime;

        if (right_position.y >= 3.75 - .5) {
            right_position -= right_movement * player_speed * deltaTime;
        }
        if (right_position.y < -3.75 + .5) {
            right_position -= right_movement * player_speed * deltaTime;
        }

        rMatrix = glm::mat4(1.0f);
        rMatrix = glm::translate(rMatrix, right_position);
        rMatrix = glm::rotate(rMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        rMatrix = glm::scale(rMatrix, glm::vec3(1.0f, .5f, 1.0f));
        //reset to identity matrix, and translate it to new position


        //ball
        if (started == true) {
            if (ball_position.y > 3.75 - .45)
                changey = -1.0f;
            else if (ball_position.y < -3.75 + .45)
                changey = 1.0f;
            ball_movement.y = 1.0f * changey;

            //collision math
            float leftxdist = fabs(ball_position.x - left_position.x) - (1 / 2.0f);
            float leftydist = fabs(ball_position.y - left_position.y) - ((1 + 1) / 2.0f);
            float rightxdist = fabs(ball_position.x - right_position.x) - (1 / 2.0f);
            float rightydist = fabs(ball_position.y - right_position.y) - ((1 + 1) / 2.0f);

            if (rightxdist < 0 && rightydist < 0)
                changex = -1.0f;
            else if (leftxdist < 0 && leftydist < 0)
                changex = 1.0f;
            ball_movement.x = 1.0f * changex;

            ball_position += ball_movement * ball_speed * deltaTime;

            bMatrix = glm::mat4(1.0f);
            bMatrix = glm::translate(bMatrix, ball_position);
        }
        else
            ball_position += 0;
    }
    
}

void Render() {
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 }; //this are the vertices of the triangle
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glClear(GL_COLOR_BUFFER_BIT); //clear the color and use the background color


    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);


    program.SetModelMatrix(lMatrix); //here is the matrix to use when drawing the next thing
    glBindTexture(GL_TEXTURE_2D, playerTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(rMatrix); //here is the matrix to use when drawing the next thing
    glBindTexture(GL_TEXTURE_2D, playerTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(bMatrix); //here is the matrix to use when drawing the next thing
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(displayWindow); //swap window basically pushes it to the screen
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) { //this is happening as fast as the computer can handle (framerate)
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}