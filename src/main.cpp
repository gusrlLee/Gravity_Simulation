#include <iostream>
#include <vector>
#include <algorithm>

// for OpenGL 
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// for texture 
// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FileSystem.h"

// math 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"

// loader 
#include "shader.h"
#include "camera.h"
#include "model.h"

// for data 
#include "data.h"

// window size 
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera 
Camera camera(glm::vec3(0.0f, 5.0f, 30.0f));

// mouse Cursor 
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// time 
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// light Position 
glm::vec3 light_position(0.0f, 50.0f, 40.0f);

// squre amount 
const int amount = 3000000;
// position array 
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

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(const char *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int main() {
    // GL init 
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif 

    // create window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "My_Project", NULL, NULL);
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

    // glad init 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // for 3D 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Cube Map Shader 
    Shader cube_map_shader("../shader/CubeMap/CubeMap_vs.glsl", "../shader/CubeMap/CubeMap_fs.glsl");
    // teapot Shader 
    Shader teapot_shader("../shader/Teapot/teapot_vs.glsl", "../shader/Teapot/teapot_fs.glsl");
    Shader shader("../shader/vertex_shader.glsl", "../shader/fragment_shader.glsl");

    // init square
    for (int i=0; i<amount; i++) {
        square_container[i].life = -1.0f;
        square_container[i].up = 0;
    }

    // binding VAO
    unsigned int pVAO;
    glGenVertexArrays(1, &pVAO);
    glBindVertexArray(pVAO);

    // binding vertex buffer 
    unsigned int square_vertex_data_buffer;
    glGenBuffers(1, &square_vertex_data_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_vertex_data_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // define vertex buffer 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));

    unsigned int square_position_data_buffer;
    glGenBuffers(1, &square_position_data_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, square_position_data_buffer);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * amount, &square_position_buffer[0], GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * amount, NULL, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);

    Model teapot(FileSystem::getPath("resource/obj/teapot/teapot.obj"));

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    
    std::vector<std::string> faces;

    // load CubeMap texture 
    faces.push_back(FileSystem::getPath("resource/texture/right.jpg"));
    faces.push_back(FileSystem::getPath("resource/texture/left.jpg"));
    faces.push_back(FileSystem::getPath("resource/texture/top.jpg"));
    faces.push_back(FileSystem::getPath("resource/texture/bottom.jpg"));
    faces.push_back(FileSystem::getPath("resource/texture/front.jpg"));
    faces.push_back(FileSystem::getPath("resource/texture/back.jpg")); 

    unsigned int cubmap_texture = loadCubemap(faces);

    // for water box 
    unsigned int water_texture;
    glGenTextures(1, &water_texture);
    glBindTexture(GL_TEXTURE_2D, water_texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* texture = stbi_load(FileSystem::getPath("resource/texture/bottom.jpg").c_str(), &width, &height, &nrChannels, 0);
    if ( texture ) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
        glGenerateMipmap(GL_TEXTURE_2D);
    }    
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(texture);



    shader.use();
    shader.setInt("water_texture", 0);

    // for teapot 
    teapot_shader.use();
    teapot_shader.setInt("skybox", 0);
    teapot_shader.setInt("material.diffuse", 0);
    teapot_shader.setInt("material.specular", 1);

    // for cube map 
    cube_map_shader.use();
    cube_map_shader.setInt("skybox", 0);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        int new_square = (int)(deltaTime * 10000.0);
        if (new_square > (int)(0.016f * 10000.0))
            new_square = (int)(0.016f * 10000.0);


        float spread = 1.5f;
        for (int i=0; i<new_square; i++) {
            int square_idx = findUnusedIndex();
            square_container[square_idx].life = 5.0f;
            square_container[square_idx].pos = glm::vec3(9.0f, 1.5f, 0.0f);
            square_container[square_idx].up = 0;

            glm::vec3 main_dir = glm::vec3(5.0f, 1.0f, 0.0f);
            glm::vec3 rand_dir = glm::vec3(
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f,
                    (rand()%2000 - 1000.0f)/1000.0f
            );

            square_container[square_idx].speed = main_dir + rand_dir * spread;
        }

        int square_counter = 0;
        for (int i=0; i<amount; i++) {
            Square &sq = square_container[i];

            if (sq.life > 0.0f) {
                sq.life -= deltaTime;
                if (sq.life > 0.0f) {
                    sq.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
                    sq.pos += sq.speed * (float)deltaTime;

                    square_position_buffer[square_counter] = sq.pos;
                }
                square_counter++;
            }
        }


        shader.use();

        // vertex shader 
        shader.setMat4("model",model);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // fragment shader 
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", light_position);

        glBindVertexArray(pVAO);
        // draw plane 
        glBindBuffer(GL_ARRAY_BUFFER, square_position_data_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * amount, NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, square_counter * sizeof(glm::vec3), square_position_buffer);

        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, square_position_data_buffer);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
        glVertexAttribDivisor(3, 1); 

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, water_texture);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, square_counter);

        teapot_shader.use();
        teapot_shader.setVec3("light.position", light_position);
        teapot_shader.setVec3("viewPos", camera.Position);
        teapot_shader.setVec3("lightPos", light_position);
        teapot_shader.setVec3("lightColor", glm::vec3(0.0f, 1.0f, 1.0f));
        teapot_shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        teapot_shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        teapot_shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        teapot_shader.setFloat("material.shininess", 64.0f);
        teapot_shader.setMat4("projection", projection);
        teapot_shader.setMat4("view", view);
        glm::mat4 teapot_model = glm::mat4(1.0f);
        teapot_model = glm::translate(teapot_model, glm::vec3(0.0f, 0.0f, 0.0f));
        teapot_model = glm::scale(teapot_model, glm::vec3(1.0f, 1.0f, 1.0f));
        teapot_model = glm::rotate(teapot_model, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        teapot_shader.setMat4("model", teapot_model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubmap_texture);
        teapot.Draw(teapot_shader);
        glBindVertexArray(0);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);
        cube_map_shader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        cube_map_shader.setMat4("view", view);
        cube_map_shader.setMat4("projection", projection);

        // skybox cube 
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubmap_texture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;

}
