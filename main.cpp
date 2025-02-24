#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stack>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "demo.c"

int spin = 0;
GLuint demo_t;

struct Vertex {
    float x, y, z;
    float r, g, b;
    float u, v;
};

std::vector<Vertex> vertexBuffer;
GLuint VAO, VBO;
GLuint shaderProgram;
int drawMode;
std::stack<glm::mat4> matrixStack;
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

float currentR = 1.0f, currentG = 1.0f, currentB = 1.0f;
float currentU = 0.0f, currentV = 0.0f;

void myPushMatrix() {
    matrixStack.push(modelMatrix);
}

void myPopMatrix() {
    if (!matrixStack.empty()) {
        modelMatrix = matrixStack.top();
        matrixStack.pop();
    }
}

void myScalef(float x, float y, float z) {
    modelMatrix = glm::scale(modelMatrix, glm::vec3(x, y, z));
}

void myTranslatef(float x, float y, float z) {
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
}

void myRotatef(float angle, float x, float y, float z) {
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(x, y, z));
}

void myColor3f(float r, float g, float b) {
    currentR = r;
    currentG = g;
    currentB = b;
}

void myTexCoord2D(float u, float v) {
    currentU = u;
    currentV = v;
}

void myBegin(int type) {
    vertexBuffer.clear();
    drawMode = type;
}

void myVertex3f(float x, float y, float z) {
    glm::vec4 transformedVertex = modelMatrix * glm::vec4(x, y, z, 1.0f);
    vertexBuffer.push_back({transformedVertex.x, transformedVertex.y, transformedVertex.z, currentR, currentG, currentB, currentU, currentV});
}

void myEnd() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(Vertex), vertexBuffer.data(), GL_DYNAMIC_DRAW);

    // Activar textura antes de dibujar
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, demo_t);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glDrawArrays(drawMode, 0, vertexBuffer.size());
    glBindVertexArray(0);
}


GLuint Bon_create_texture(int bpp, int w, int h, const void *data) {
    GLuint tmp_txt;
    glGenTextures(1, &tmp_txt);
    glBindTexture(GL_TEXTURE_2D, tmp_txt);
  
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
    glTexImage2D(GL_TEXTURE_2D, 0, (bpp == 4) ? GL_RGBA : GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  
    return tmp_txt;
  }

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;
out vec3 FragColor;
out vec2 TexCoord;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragColor = aColor;
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 FragColor;
    in vec2 TexCoord;
    out vec4 FragOutput;
    
    uniform sampler2D texture1;
    
    void main() {
        vec4 texColor = texture(texture1, TexCoord);
        FragOutput = texColor * vec4(FragColor, 1.0);  // Multiplicar por color del vértice
    }
    )";
    

void checkShaderCompilation(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader Compilation Failed\n" << infoLog << std::endl;
    }
}

void checkShaderProgramLinking(GLuint program) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Program Linking Failed\n" << infoLog << std::endl;
    }
}

void setupShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompilation(fragmentShader);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkShaderProgramLinking(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void renderCube() {
    myBegin(GL_TRIANGLES);

    myTranslatef(0.0f, 0.0f, -2.0f);
    myRotatef(spin, 0.0f, 1.0f, 1.0f);

    // Cara frontal
    myColor3f(1.0f, 1.0f, 1.0f);
    myTexCoord2D(0.0f, 0.0f); myVertex3f(-0.5f, -0.5f, 0.5f);
    myTexCoord2D(1.0f, 0.0f); myVertex3f( 0.5f, -0.5f, 0.5f);
    myTexCoord2D(1.0f, 1.0f); myVertex3f( 0.5f,  0.5f, 0.5f);
    
    myTexCoord2D(0.0f, 0.0f); myVertex3f(-0.5f, -0.5f, 0.5f);
    myTexCoord2D(1.0f, 1.0f); myVertex3f( 0.5f,  0.5f, 0.5f);
    myTexCoord2D(0.0f, 1.0f); myVertex3f(-0.5f,  0.5f, 0.5f);

    // Cara trasera
    myTexCoord2D(1.0f, 0.0f); myVertex3f(-0.5f, -0.5f, -0.5f);
    myTexCoord2D(1.0f, 1.0f); myVertex3f(-0.5f,  0.5f, -0.5f);
    myTexCoord2D(0.0f, 1.0f); myVertex3f( 0.5f,  0.5f, -0.5f);
    
    myTexCoord2D(1.0f, 0.0f); myVertex3f(-0.5f, -0.5f, -0.5f);
    myTexCoord2D(0.0f, 1.0f); myVertex3f( 0.5f,  0.5f, -0.5f);
    myTexCoord2D(0.0f, 0.0f); myVertex3f( 0.5f, -0.5f, -0.5f);

    // Resto de caras...

    myEnd();
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Renderer", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewInit();
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);
    
    demo_t = Bon_create_texture(demo.bytes_per_pixel,demo.width,demo.height, demo.pixel_data);
    glBindTexture(GL_TEXTURE_2D, demo_t);

    setupShader();
    setupBuffers();
    
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    
    projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);
        
        myPushMatrix();
            renderCube();
        myPopMatrix();

        glfwSwapBuffers(window);
        glfwPollEvents();
        spin+=5;
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
