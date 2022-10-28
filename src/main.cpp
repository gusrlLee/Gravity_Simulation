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

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// square number 
const int amount = 512000;
Square square_container[amount];
glm::vec3 square_position_buffer[amount];

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

    // shader load 
    Shader shader("../shader/vertex_shader.glsl", "../shader/fragment_shader.glsl");

    float offset = 0.5f;
    int index = 0;
    for (int z=0; z<80; z++) {
        for (int y=0; y<80; y++) {
            for (int x=0; x<80; x++) {
                glm::vec3 position;
                position.x = (float)(x - (80/2)) * offset;
                position.y = (float)(y - (80/2)) * offset;
                position.z = (float)(z - (80/2)) * offset;
                square_position_buffer[index] = position;
                index++;
            }
        }
    }

    // binding VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // binding vertex buffer 
    unsigned int square_vertex_data_buffer;
    glGenBuffers(1, &square_vertex_data_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_vertex_data_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // define vertex buffer 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);

    unsigned int square_position_data_buffer;
    glGenBuffers(1, &square_position_data_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_position_data_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * amount, &square_position_buffer[0], GL_DYNAMIC_DRAW);

    // define vertex buffer 
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glVertexAttribDivisor(1, 1); 

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, amount);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &square_vertex_data_buffer);
    glDeleteBuffers(1, &square_position_data_buffer);

    glfwTerminate();
    return 0;
}
