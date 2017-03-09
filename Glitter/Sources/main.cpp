// Local Headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/ext.hpp"          // to print vectors and matrices


// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <iostream>

#define RED 1.0f, 0.0f, 0.0f            // define macros for convenience
#define BLUE 0.0f, 0.0f, 1.0f
#define GREEN 0.0f, 1.0f, 0.0f
#define PURPLE 1.0f, 0.0f, 1.0f

// device coordinates
#define LEFT -0.5f
#define TOP 0.5f
#define RIGHT 0.5f
#define BOTTOM -0.5f
#define MIDDLE 0.0f
#define NEAR 0.5f
#define FAR -0.5f

const GLchar* vertexSource =
"#version 330 core\n"             // glsl version
"layout (location = 0) in vec3 position;"
"layout (location = 1) in vec3 color;"               // and 3 values for color
"out vec3 Color;"                 // will pass color along pipeline
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 projection;"         // uniform = the same for all vertices
"void main()"
"{"
"    Color = color;"              // just pass color along without modifying it
"    gl_Position = projection * view * model * vec4(position, 1.0f);"   // gl_Position is special variable for final position
"}";                                                    // must be in homogeneous coordinates -- put in 0 for z and 1 for w
// multiply by model matrix to transform
const GLchar* fragmentSource =
"#version 330 core\n"
"in vec3 Color;"
"out vec4 outColor;"
"void main()"
"{"
"    outColor = vec4(Color, 1.0);"
"}";

// Triangle and Color vertex data
GLfloat vertices [] = {
  LEFT, BOTTOM, NEAR, BLUE,
  RIGHT, BOTTOM, NEAR, BLUE,
  MIDDLE, TOP, MIDDLE, BLUE,
  LEFT, BOTTOM, NEAR, RED,
  LEFT, BOTTOM, FAR, RED,
  MIDDLE, TOP, MIDDLE, RED,
  RIGHT, BOTTOM, NEAR, GREEN,
  RIGHT, BOTTOM, FAR, GREEN,
  MIDDLE, TOP, MIDDLE, GREEN,
  LEFT, BOTTOM, FAR, PURPLE,
  RIGHT, BOTTOM, FAR, PURPLE,
  MIDDLE, TOP, MIDDLE, PURPLE,
};
// Camera vertex data
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraBack = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 cameraDown  = glm::vec3(0.0f, -1.0f,  0.0f);
glm::vec3 cameraLeft  = glm::vec3(-1.0f, 0.0f,  0.0f);
glm::vec3 cameraRight  = glm::vec3(1.0f, 0.0f,  0.0f);

//Camera speed
GLfloat cameraSpeed = 0.1f;
//Gaze direction
glm::vec3 Gaze;

bool keys[1024];

bool controlIsPressed(GLFWwindow* window) {
	return glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
}

// callback for keyboard input
// move camera when arrow keys are pressed, rotate it when arrow keys are pressed with control
void key_callback(GLFWwindow* mWindow, int key, int scancode, int action, int mods);



int main(int argc, char * argv[]) {
  // Load GLFW and Create a Window
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  auto mWindow = glfwCreateWindow(200, 200, "CISC3620", nullptr, nullptr);
  // Check for Valid Context
  if (mWindow == nullptr) {
    fprintf(stderr, "Failed to Create OpenGL Context");
    return EXIT_FAILURE;
  }

  // callbacks
  glfwSetKeyCallback(mWindow, key_callback);
  
  // Create Context and Load OpenGL Functions
  glfwMakeContextCurrent(mWindow);
  gladLoadGL();
  fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
  
  // Create Vertex Array Object: this will store all the information about the vertex data that we are about to specify
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  
  // Create a Vertex Buffer Object and copy the vertex data to it
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

  
  // Create and compile the vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  // Create and compile the fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);
  
  // Specify the layout of the vertex data
  // position
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),(GLvoid*)0);  // attribute location, # values, value type, normalize?, stride, offset
  glEnableVertexAttribArray(0);
  // color
  GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  
  // Rendering Loop
  while (glfwWindowShouldClose(mWindow) == false) {
    // Calculate deltatime of current frame
    GLfloat currentFrame = glfwGetTime();
    //deltaTime = currentFrame - lastFrame;
    //lastFrame = currentFrame;
      
    // Background Fill Color
    glClearColor(0.85f, 0.65f, 0.65f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT);
      
    //View
    glm::mat4 view;
    view = glm::lookAt(cameraPos, cameraPos + cameraBack, cameraUp);
    // Projection
    glm::mat4 projection;
    projection = glm::perspective(45.0f, (GLfloat)200/(GLfloat)200, 0.1f, 100.0f);
    // Get the uniform locations
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    // Pass the matrices to the shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
      
    //Model matrix
    GLint modelTransform = glGetUniformLocation(shaderProgram, "model");
    glm::mat4 model;
    glUniformMatrix4fv(modelTransform, 1, GL_FALSE, glm::value_ptr(model));
    
      
    //draw triangles
    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(vertices[0]));
      
      
    // Flip Buffers and Draw
    glfwSwapBuffers(mWindow);
    glfwPollEvents();
  }   glfwTerminate();
  
  // clean up
  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  
  return EXIT_SUCCESS;
}
//moves camera position when buttons are pressed
void key_callback(GLFWwindow* mWindow, int key, int scancode, int action, int mods){

    std::cout << (mods == GLFW_MOD_SHIFT) << std::endl;
    
        if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
            if (mods == GLFW_MOD_SHIFT){
                std::cout << "rotating camera Right\n"; // rotate camera
                //Print camera location to terminal
                std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
                //Then gaze vector
                Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
                std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
                //Then up vector
                std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
                //do same for the other if-else statements
            }else{
                std::cout << "moving camera Right\n";  // move camera
                cameraPos += cameraSpeed * cameraRight;
                std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
                Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
                std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
                std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
            }
        }
        if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
            if (mods == GLFW_MOD_SHIFT){
                std::cout << "rotating camera Left\n"; // rotate camera
                //cameraPos += cameraSpeed * cameraLeft;
                std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
                Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
                std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
                std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
            }else{
                std::cout << "moving camera Left\n";  // move camera
                cameraPos += cameraSpeed * cameraLeft;
                std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
                Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
                std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
                std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
            }
        }
        if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
            if (mods == GLFW_MOD_SHIFT){
                std::cout << "rotating camera Up\n"; // rotate camera
            }else{
                std::cout << "moving camera Up\n";  // move camera
                cameraPos += cameraSpeed * cameraUp;
                std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
                Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
                std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
                std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
            }
        }
        if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
            if (mods == GLFW_MOD_SHIFT){
                std::cout << "rotating camera Down\n"; // rotate camera
            }else{
                std::cout << "moving camera Down\n";  // move camera
                cameraPos += cameraSpeed * cameraDown;
                std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
                Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
                std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
                std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
            }
        }
        if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
            if (mods == GLFW_MOD_SHIFT){
            std::cout << "moving camera Forward\n";  // move camera
            cameraPos += cameraSpeed * cameraFront;
            std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
            Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
            std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
            std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
                
            }
        }
        if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
            if (mods == GLFW_MOD_SHIFT){
            std::cout << "moving camera Back\n";  // move camera
            cameraPos += cameraSpeed * cameraBack;
            std::cout<< "camera location now: " << glm::to_string(cameraPos)<<std::endl;
            Gaze = (glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos)/glm::length((glm::vec3(0.0f, 0.0f, 0.0f)-cameraPos));
            std::cout<< "gaze direction now: " << glm::to_string(Gaze)<<std::endl;
            std::cout<< "camera up direction now: " << glm::to_string(cameraUp);
            }
        }
}
