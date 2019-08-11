#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <utils/shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utils/camera.h>

#include <iostream>
#include <string>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

// Functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

// Last mouse cursor positions
float lastX, lastY;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float vertices[] = {
    -0.55f, 0.95f, 0.0f,  // top right
    -0.55f, 0.55f, 0.0f,  // bottom right
    -0.95f, 0.55f, 0.0f,  // bottom left
    -0.95f, 0.95f, 0.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
}; 

float thirdTriangle[] = {
    0.6f, 0.6f, 0.0f,  // left
    0.9f, 0.6f, 0.0f,  // right
    0.75f, 0.9f, 0.0f   // top 
};

float fourthTriangle[] = {
    // positions         // colors
    -0.9f,  0.2f, 0.0f,  1.0f, 0.0f, 0.0f,    // bottom left
    -0.6f,  0.9f, 0.0f,  0.0f, 1.0f, 0.0f,    // top
    -0.3f,  0.2f, 0.0f,  0.0f, 0.0f, 1.0f     // bottom right 
}; 

float rect[] = {
    // positions          // colors           // texture coords
     0.5f,  0.1f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.9f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.9f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.1f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
};

float cube[] = {
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f
};

glm::vec3 cubePositions[] = {
  glm::vec3( 0.0f, -0.5f,  0.0f), 
  glm::vec3( 2.0f,  5.0f, -15.0f), 
  glm::vec3(-1.5f, -2.2f, -2.5f),  
  glm::vec3(-3.8f, -2.0f, -12.3f),  
  glm::vec3( 2.4f, -0.4f, -3.5f),  
  glm::vec3(-1.7f,  3.0f, -7.5f),  
  glm::vec3( 1.3f, -2.0f, -2.5f),  
  glm::vec3( 1.5f,  2.0f, -2.5f), 
  glm::vec3( 1.5f,  0.2f, -1.5f), 
  glm::vec3(-1.3f,  1.0f, -1.5f)  
};

unsigned int containerTex;
unsigned int faceTex;

unsigned int VBOs[5];

unsigned int VAOs[5];

unsigned int EBOs[2];

unsigned int vertexShader;
unsigned int vertexShaderMixed;

unsigned int fragmentShaderOrange;
unsigned int fragmentShaderYellow;
unsigned int fragmentShaderMixed;

unsigned int shaderProgramOrange;
unsigned int shaderProgramYellow;
unsigned int shaderProgramMixed;

bool wireframe;
int cooldown;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos; // the position variable has attribute position 0\n"
"out vec4 vertexColor; // specify a color output to the fragment shader\n"
"void main()\n"
"{\n"
    "gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor\n"
    "vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color\n"
"}\n\0";

const char *vertexShaderSourceMixed = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
"   ourColor = aColor;\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 ourColor; // we set this variable in the OpenGL code.\n"
"void main()\n"
"{\n"
    "FragColor = ourColor;\n"
"}\n\0";
    
const char *fragmentShaderSourceMixed = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n"
"}\n\0";

int main()
{
    wireframe = false;
    cooldown = 0;
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
//    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    // glfw windowed full screen creation
    // --------------------
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    
    SCR_WIDTH = mode->width;
    SCR_HEIGHT = mode->height;

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "LearnOpenGL", monitor, NULL);
    
    lastX = mode->width/2;
    lastY = mode->height/2;

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Hide and capture the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Each time the mouse moves, the cursor position is updated in xpos and ypos
    glfwSetCursorPosCallback(window, mouse_callback); 
    
    // Each time the mouse scrolls, the camera zooms in/out
    glfwSetScrollCallback(window, scroll_callback); 

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Create a VAO buffer (Vertex Array Object) which stores attribute calls for each vertex in each VBO (so they can be called once and
    // take from the VAO when needed)
    glGenVertexArrays(5, VAOs);  
    
    // Create a VBO buffer (Vertex Buffer Object) which stores vertices
    glGenBuffers(5, VBOs);
    
    // Create a EBO buffer (Element Buffer Object) which stores elements (indices of vertices)
    glGenBuffers(2, EBOs);
    
    // Set the first two triangles
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAOs[0]);
    
    // Bind the VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);  
    
    // Insert the vertices into it
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind the EBO buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    
    // Insert the elements (indices) into it
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Tell OpenGL how it should interpret the vertex data taken from the bound VBO
    // First arg: which vertex we want to configure (the one at location = 0)
    // Second arg: size of the vertex attribute (vec3 has 3 values)
    // Third arg: type of data
    // Fourth arg: if we want the data to be normalized
    // Fifth arg: stride between each vertex
    // Sixth arg: offset of where the position data begins in the buffer (of type void*)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // Enable the Vertex attributes (which are disabled by default)
    glEnableVertexAttribArray(0);
    
    // Set the third triangle
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(thirdTriangle), thirdTriangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Set the fourth triangle
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(fourthTriangle), fourthTriangle, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Set the second rectangle
    glBindVertexArray(VAOs[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
    
    // Bind the EBO buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
    
    // Insert the elements (indices) into it
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    
    // Set the cube
    glBindVertexArray(VAOs[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[4]);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object 
    // so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 
    
    // Print maximum number of vertex attributes supported by the hardware
//    int nrAttributes;
//    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
//    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

    Shader firstShader("test\\shaders\\vertShaderSrc.VERT", "test\\shaders\\fragShaderSrc.FRAG");
    Shader mixedShader("test\\shaders\\vertShaderSrcMixed.VERT", "test\\shaders\\fragShaderSrcMixed.FRAG");
    Shader texShader("test\\shaders\\vertShaderSrcTex.VERT", "test\\shaders\\fragShaderSrcTex.FRAG");
    
    // Store the texture(s) we want to generate in an unsigned int array
    glGenTextures(1, &containerTex);  
    
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, containerTex);  
    
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load an image
    int width, height, nrChannels;
    unsigned char *data = stbi_load("C:\\Users\\Alessio\\Documents\\GitHub\\Progetto_RTGP\\work\\test\\resources\\textures\\container.jpg",
                                        &width, &height, &nrChannels, 0); 
    
    if (data)
    {
        // Generate the texture by attaching it to the bound texture object
        // First arg: texture target
        // Second arg: mipmap level
        // Third arg: texture format
        // Fourth arg: texture width
        // Fifth arg: texture heigth
        // Sixth arg: this arg should always be 0
        // Seventh arg: format of the source image
        // Eigth arg: datatype of the source image
        // Ninth arg: source image data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    
    // Free the image memory
    stbi_image_free(data);
    
    // Store the texture(s) we want to generate in an unsigned int array
    glGenTextures(1, &faceTex);  
    
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, faceTex);  
    
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_set_flip_vertically_on_load(true); 
    data = stbi_load("C:\\Users\\Alessio\\Documents\\GitHub\\Progetto_RTGP\\work\\test\\resources\\textures\\awesomeface.png", 
                        &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
//    glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
//    glm::mat4 trans = glm::mat4(1.0f);  // Identity Matrix (diagonal contains 1s)
//    trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
//    vec = trans * vec;
//    std::cout << vec.x << vec.y << vec.z << std::endl;

    // Model Matrix (used to transform from Local Coords to World Coords)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
    
    // View Matrix (used to transform from World Coords to View Coords)
    glm::mat4 view = glm::mat4(1.0f);
    // note that we're translating the scene in the reverse direction of where we want to move
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f)); 
    
    // Projection Matrix (used to transform from View Coords to Clip Coords)
    glm::mat4 projection;
    
    float ratio = mode->width / mode->height;
    projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    
    glEnable(GL_DEPTH_TEST); 
    
    float variableScaleCube = 0.8;
    bool variableScaleIncreasing = false;
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame; 
        
        // Update View Transform
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        
        ratio = mode->width / mode->height;
        
        // Update Projection Transform
        projection = glm::perspective(glm::radians(camera.Zoom), ratio, 0.1f, 100.0f); 
        
        if (cooldown > 0)
            cooldown--;
        // input
        // -----
        processInput(window);
        
        float timeValue = glfwGetTime();
        
        // rendering commands here
        // -----
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        float variableOffset = tan(timeValue)/2 + 0.5f;
        
        // draw our first triangle
        // Tell to the GPU that every Shader and Rendering call from now on will use this program object
        // now when we draw the triangle we first use the vertex and orange fragment shader from the first program
        firstShader.use();
        firstShader.setFloat("yOffset", variableOffset);
        int modelLoc = glGetUniformLocation(firstShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        int viewLoc = glGetUniformLocation(firstShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(firstShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        // Get the uniform parameter "ourColor" and set it to red
        float variableColor = (sin(timeValue) * 2) + 0.5f;
        // Get the uniform parameter "ourColor" and set it to red (variable)
        firstShader.setUniform4f("ourColor", variableColor, 0.0f, 0.0f, 1.0f);
        
        glBindVertexArray(VAOs[0]); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time 
        
        // Get the uniform parameter "ourColor" and set it to yellow (variable)
        firstShader.setUniform4f("ourColor", variableColor, variableColor, 0.0f, 1.0f);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);	// this call should output a yellow triangle
        
        // Update the second rectangle
        rect[3] = variableColor;
        rect[13] = variableColor;
        rect[21] = variableColor;
        rect[27] = variableColor;
        rect[28] = variableColor;
        glBindVertexArray(VAOs[3]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);  
        glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
        
        // Bind the EBO buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
        
        // Insert the elements (indices) into it
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        // Draw the second rectangle
        texShader.use();
        texShader.setMat4("model", model);
        texShader.setMat4("view", view);
        texShader.setMat4("projection", projection);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, faceTex);
        glUniform1i(glGetUniformLocation(texShader.ID, "texture1"), 0); // set it manually
        texShader.setInt("texture2", 1); // or with shader class
        
        float variableScale = (sin(timeValue) / 2) + 0.5f;
        
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(0.0f, 2.0f, 0.0f));
        trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
        trans = glm::scale(trans, glm::vec3(variableScale, variableScale, variableScale));
        
        texShader.setMat4("transform", trans);

        glBindVertexArray(VAOs[3]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
        
        // Update the fourth triangle
        fourthTriangle[4] = variableColor;
        fourthTriangle[11] = variableColor;
        fourthTriangle[18] = variableColor;
        // Set the fourth triangle
        // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAOs[2]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);  
        glBufferData(GL_ARRAY_BUFFER, sizeof(fourthTriangle), fourthTriangle, GL_STATIC_DRAW);
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);
        
        variableOffset = tan(timeValue)/5 + 0.5f;
        
        // Draw the fourth triangle
        mixedShader.use();
        mixedShader.setFloat("xOffset", variableOffset);
        mixedShader.setMat4("model", model);
        mixedShader.setMat4("view", view);
        mixedShader.setMat4("projection", projection);
        glBindVertexArray(VAOs[2]);
        glDrawArrays(GL_TRIANGLES, 0, 3);	// this call should output a mixed triangle
        
        // Draw the cube
        texShader.use();
//        glm::mat4 model1 = glm::mat4(1.0f);
//        model1 = glm::translate(model1, glm::vec3(0.0f, -1.0f, 0.0f));
//        float angle = 20.0f; 
//        model1 = glm::rotate(model1, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
//        texShader.setMat4("model", model1);
//        texShader.setMat4("view", view);
//        texShader.setMat4("projection", projection);
//        
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, containerTex);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, faceTex);
//        glUniform1i(glGetUniformLocation(texShader.ID, "texture1"), 0); // set it manually
//        texShader.setInt("texture2", 1); // or with shader class

        // Update the cube
        cube[3] = variableColor;
        cube[13] = variableColor;
        cube[21] = variableColor;
        cube[27] = variableColor;
        cube[28] = variableColor;
        cube[36] = variableColor;
        cube[37] = variableColor;
        cube[43] = variableColor;
        cube[45] = variableColor;
        
        glBindVertexArray(VAOs[4]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[4]);  
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
        
        if (variableScaleCube >= 0.8)
            variableScaleIncreasing = false;
        if (variableScaleCube <= 0.2)
            variableScaleIncreasing = true;
        
        if (variableScaleIncreasing)
            variableScaleCube+=0.001;
        else
            variableScaleCube-=0.001;
            
        trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.0f));
        trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
        trans = glm::scale(trans, glm::vec3(variableScaleCube, variableScaleCube, variableScaleCube));
        
        texShader.setMat4("transform", trans);

        glBindVertexArray(VAOs[4]);
        
        for(unsigned int i = 0; i < 10; i++)
        {
          glm::mat4 model1 = glm::mat4(1.0f);
          model1 = glm::translate(model1, cubePositions[i]);
          float angle = 20.0f * i; 
          if (angle == 0) angle = 90.0f;
          model1 = glm::rotate(model1, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
          texShader.setMat4("model", model1);

          glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    // Press Escape to close the window
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
    // Press X to toggle wireframe polygons
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && cooldown == 0)
    {
        if (wireframe)
        {
            wireframe = false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            cooldown = 400;
        }
        else
        {
            wireframe = true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            cooldown = 400;
        }
    }
    
    // Press W to move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
        
    // Press S to move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        
    // Press A to move left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
        
    // Press D to move right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}