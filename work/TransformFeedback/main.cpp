#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/model_v2.h>
#include <utils/physics.h>

#include <bullet/btBulletDynamicsCommon.h>

#include <iostream>
#include <string>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();
float getDistance(glm::vec3 point1, glm::vec3 point2);
unsigned int loadTexture(const char *path);

// Vertex shader
const GLchar* vertexShaderSrc = R"glsl(
    #version 150 core

    in vec3 inValue;
    out vec3 outValue;

    void main()
    {
        outValue = vec3(inValue.x + 5, inValue.y + 10, inValue.z + 1);
    }
)glsl";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Compile shader
    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(shader);

    // Create program and specify transform feedback variables
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);

    const GLchar* feedbackVaryings[] = { "outValue" };
    glTransformFeedbackVaryings(program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

    glLinkProgram(program);
    glUseProgram(program);

    // Create VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vbo;
    GLuint tbo;
    
    // Create input VBO and vertex format
    glm::vec3 data[] = { glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(15.0f, 10.0f, 10.0f)}; //, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f };
    
    while(true)
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

        GLint inputAttrib = glGetAttribLocation(program, "inValue");
        glEnableVertexAttribArray(inputAttrib);
        glVertexAttribPointer(inputAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Create transform feedback buffer
        glGenBuffers(1, &tbo);
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), nullptr, GL_STATIC_READ);

        // Perform feedback transform
        glEnable(GL_RASTERIZER_DISCARD);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

        glBeginTransformFeedback(GL_POINTS);
            glDrawArrays(GL_POINTS, 0, sizeof(data));
        glEndTransformFeedback();

        glDisable(GL_RASTERIZER_DISCARD);

        glFlush();

        // Fetch and print results
        glm::vec3 feedback[2];
        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

//        printf("%f %f %f %f %f %f %f %f %f %f\n", feedback[0], feedback[1], feedback[2], feedback[3], feedback[4],
//                                                feedback[5], feedback[6], feedback[7], feedback[8], feedback[9]);
        
        printf("%f %f %f %f %f %f\n", feedback[0].x, feedback[0].y, feedback[0].z, feedback[1].x, feedback[1].y, feedback[1].z);
        
        int size = sizeof(data)/sizeof(data[0]);
        
        for (int i = 0; i<size; i++)
        {
            data[i] = feedback[i];
        }
    }

    glDeleteProgram(program);
    glDeleteShader(shader);

    glDeleteBuffers(1, &tbo);
    glDeleteBuffers(1, &vbo);

    glDeleteVertexArrays(1, &vao);

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// --------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

//////////////////////////////////////////
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
}


float getDistance(glm::vec3 point1, glm::vec3 point2)
{
    return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}


// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
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