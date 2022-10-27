#include <iostream>
#include <vector>
#include <algorithm>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

// for texture 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FileSystem.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"

#include "shader.h"
#include "camera.h"

// for data 
#include "data.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// square number 
const int amount = 512000;
Square square_container[amount];

// last used number 
int last_unused_index = 0;

int findUnusedIndex() {
    // find last use index of particle 
    for (int i=last_unused_index; i<amount; i++) {
        if (square_container[i].life < 0) {
            last_unused_index = i;
            return i;
        }
    }

    // Check the index upside down
    for (int i=0; i<last_unused_index; i++) {
        if (square_container[i].life < 0) {
            last_unused_index = i;
            return i;
        } 
    }

    // if not, return 0
    return 0;
}

void sortSquare() {
    std::sort(&square_container[0], &square_container[amount]);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;


    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// MAIN FUNC
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif 

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // window config setting 
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // shader load 
    Shader shader("../shader/vertex_shader.glsl", "../shader/fragment_shader.glsl");
    
    static float* g_square_position_size_data = new float[amount * 3];

    // binding Vertex Array Object 
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // for vertex buffer
    unsigned int square_vertex_buffer;
    glGenBuffers(1, &square_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // for position buffer 
    unsigned int square_position_buffer;
    glGenBuffers(1, &square_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * 3 * sizeof(float), NULL, GL_STREAM_DRAW);


    // initialization vertex data 
    for (int i=0; i<amount; i++) {
        square_container[i].life = -1.0f;
        square_container[i].cameradistance = -1.0f;
    }

    float lastFrame = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int new_squares = (int)(deltaTime * 10000.0);
        if (new_squares > (int)(0.016f * 10000.0)) 
            new_squares = (int)(0.016f * 10000.0);

        for (int i=0; i<new_squares; i++) {
            int square_idx = findUnusedIndex();
            square_container[square_idx].life = 3.0f;
            square_container[square_idx].pos = glm::vec3(0, 0, -20.0f);

            float spread = 5.5f;
            glm::vec3 maindir = glm::vec3(0.0f, -10.0f, 0.0f);
            // direction + alpha random value 
            glm::vec3 randomdir = glm::vec3(
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f
            );

            square_container[square_idx].speed = maindir * randomdir * spread;
            square_container[square_idx].size = (rand() % 10000) / 29000.0f + 0.1f;
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 camera_position(glm::inverse(view)[3]);

        int squares_counter = 0;
        for (int i=0; i<amount; i++) {
            Square& s = square_container[i];

            if (s.life > 0.0f) {
                s.life -= deltaTime;
                if (s.life > 0) {
                    s.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
                    s.pos += s.speed * (float)deltaTime;
                    s.cameradistance = glm::length2(s.pos - camera_position);

                    g_square_position_size_data[3 * squares_counter + 0] = s.pos.x;
                    g_square_position_size_data[3 * squares_counter + 1] = s.pos.y;
                    g_square_position_size_data[3 * squares_counter + 2] = s.pos.z;
                }
                else {
                    s.cameradistance = -1.0f;
                }
                squares_counter++;
            }
        }

        // rendering 
        sortSquare();

        glBindBuffer(GL_ARRAY_BUFFER, square_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, amount * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, squares_counter * sizeof(float) * 3, g_square_position_size_data);

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, square_vertex_buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, square_position_buffer);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

        glVertexAttribDivisor(0, 0);
        glVertexAttribDivisor(1, 1);

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 36, squares_counter);

        glfwSwapBuffers(window);
        glfwPollEvents();
    
    }

    delete[] g_square_position_size_data;
    glDeleteBuffers(1, &square_vertex_buffer);
    glDeleteBuffers(1, &square_position_buffer);
    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    return 0;
}
