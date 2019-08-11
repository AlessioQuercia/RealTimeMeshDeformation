#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/model.h>

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
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

// Last mouse cursor positions
float lastX, lastY;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Lighting
glm::vec3 pointLightPositions[] = {
	glm::vec3( 0.7f,  0.2f,  2.0f),
	glm::vec3( 2.3f, -3.3f, -7.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3( 0.0f,  0.0f, -3.0f)
};

glm::vec3 pointLightColors[] = {
    glm::vec3(0.9f, 0.9f, 0.9f),
    glm::vec3(0.9f, 0.9f, 0.9f),
    glm::vec3(0.9f, 0.9f, 0.9f),
    glm::vec3(0.9f, 0.9f, 0.9f)
};

float cube[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
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

bool wireframe;
int cooldown;

bool dirlightOn;
int dirlightCooldown;

bool pointlightsOn;
int pointlightsCooldown;

bool flashlightOn;
int flashlightCooldown;

int main()
{
    wireframe = false;
    dirlightOn = false;
    pointlightsOn = false;
    flashlightOn = false;
    cooldown = 0;
    dirlightCooldown = 0;
    pointlightsCooldown = 0;
    flashlightCooldown = 0;
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
//    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
//    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
//    
//    SCR_WIDTH = mode->width;
//    SCR_HEIGHT = mode->height;
//
//    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
//    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
//    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
//    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    
    lastX = SCR_WIDTH/2;
    lastY = SCR_HEIGHT/2;

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
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
//    glDepthMask(GL_FALSE);   // if we need to keep the depth-buffer values that would be discarded
    glDepthFunc(GL_LESS);  

    // build and compile our shader program
    // ------------------------------------
    Shader lightingShader("lighting\\shaders\\multipleLights.VERT", "lighting\\shaders\\multipleLights.FRAG");
    Shader lampShader("lighting\\shaders\\lamp.VERT", "lighting\\shaders\\lamp.FRAG");
    Shader modelShader("lighting\\shaders\\multipleLights.VERT", "lighting\\shaders\\multipleLights.FRAG");

    // load models
    // -----------
    Model nanosuitModel("C:\\Users\\Alessio\\Documents\\GitHub\\Progetto_RTGP\\models\\nanosuit\\nanosuit.obj");
    
    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object 
    // so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 
    
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    
    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture("lighting\\resources\\textures\\container2.png");
    unsigned int specularMap = loadTexture("lighting\\resources\\textures\\container2_specular.png");
//    unsigned int emissionMap = loadTexture("lighting\\resources\\textures\\container2_emission.jpg");

    // shader configuration
    // --------------------
    lightingShader.use(); 
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
//    lightingShader.setInt("material.emission", 2);
    
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame; 
        
        // Update View Transform
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        
        // Update Projection Transform
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); 
        
        if (cooldown > 0)
            cooldown--;
        if (dirlightCooldown > 0)
            dirlightCooldown--;
        if (pointlightsCooldown > 0)
            pointlightsCooldown--;
        if (flashlightCooldown > 0)
            flashlightCooldown--;
        
        // input
        // -----
        processInput(window);
        
        // rendering commands here
        // -----
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Update Light Position
        pointLightPositions[0].x = 1.0f + sin(glfwGetTime()) * 2.0f;
        pointLightPositions[0].y = sin(glfwGetTime() / 2.0f) * 1.0f;
        pointLightPositions[1].x = 1.0f + sin(glfwGetTime()) * 2.0f;
        pointLightPositions[1].y = sin(glfwGetTime() / 2.0f) * 1.0f;
        pointLightPositions[2].x = 1.0f + sin(glfwGetTime()) * 2.0f;
        pointLightPositions[2].y = sin(glfwGetTime() / 2.0f) * 1.0f;
        pointLightPositions[3].x = 1.0f + sin(glfwGetTime()) * 2.0f;
        pointLightPositions[3].y = sin(glfwGetTime() / 2.0f) * 1.0f;
        
        // Draw the cube object
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        /*
           Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
           the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
           by defining light types as classes and set their values in there, or by using a more efficient uniform approach
           by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
        */
        // directional light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.specular", 0.05f, 0.05f, 0.05f);
        lightingShader.setBool("dirLight.on", dirlightOn);
        // point light 1
        lightingShader.setBool("pointLights[0].on", pointlightsOn);
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", pointLightColors[0] * 0.1f);
        lightingShader.setVec3("pointLights[0].diffuse", pointLightColors[0]);
        lightingShader.setVec3("pointLights[0].specular", pointLightColors[0]);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.14f);
        lightingShader.setFloat("pointLights[0].quadratic", 0.07f);
        // point light 2
        lightingShader.setBool("pointLights[1].on", pointlightsOn);
        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", pointLightColors[1] * 0.1f);
        lightingShader.setVec3("pointLights[1].diffuse", pointLightColors[1]);
        lightingShader.setVec3("pointLights[1].specular", pointLightColors[1]);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.14f);
        lightingShader.setFloat("pointLights[1].quadratic", 0.07f);
        // point light 3
        lightingShader.setBool("pointLights[2].on", pointlightsOn);
        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setVec3("pointLights[2].ambient", pointLightColors[2] * 0.1f);
        lightingShader.setVec3("pointLights[2].diffuse", pointLightColors[2]);
        lightingShader.setVec3("pointLights[2].specular", pointLightColors[2]);
        lightingShader.setFloat("pointLights[2].constant", 1.0f);
        lightingShader.setFloat("pointLights[2].linear", 0.22f);
        lightingShader.setFloat("pointLights[2].quadratic", 0.20f);
        // point light 4
        lightingShader.setBool("pointLights[3].on", pointlightsOn);
        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        lightingShader.setVec3("pointLights[3].ambient", pointLightColors[3] * 0.1f);
        lightingShader.setVec3("pointLights[3].diffuse", pointLightColors[3]);
        lightingShader.setVec3("pointLights[3].specular", pointLightColors[3]);
        lightingShader.setFloat("pointLights[3].constant", 1.0f);
        lightingShader.setFloat("pointLights[3].linear", 0.14f);
        lightingShader.setFloat("pointLights[3].quadratic", 0.07f);
        // spotLight
        lightingShader.setBool("spotLight.on", flashlightOn);
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        
        // View/Projection transformations
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        
        // world transformation
        model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap); 
        
//        // bind emission map
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, emissionMap);
        
        // render the cubes
        glBindVertexArray(cubeVAO);
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw the lamp object
        lampShader.use();
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        lampShader.setBool("on", pointlightsOn);
        
        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightVAO);
        for (unsigned int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lampShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
        // Use the nanosuit model program shader
        modelShader.use();
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setFloat("material.shininess", 32.0f);

        /*
           Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
           the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
           by defining light types as classes and set their values in there, or by using a more efficient uniform approach
           by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
        */
        // directional light
        modelShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        modelShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        modelShader.setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
        modelShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
        modelShader.setBool("dirLight.on", dirlightOn);

        // point light 1
        modelShader.setBool("pointLights[0].on", pointlightsOn);
        modelShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        modelShader.setVec3("pointLights[0].ambient", pointLightColors[0] * 0.1f);
        modelShader.setVec3("pointLights[0].diffuse", pointLightColors[0]);
        modelShader.setVec3("pointLights[0].specular", pointLightColors[0]);
        modelShader.setFloat("pointLights[0].constant", 1.0f);
        modelShader.setFloat("pointLights[0].linear", 0.14f);
        modelShader.setFloat("pointLights[0].quadratic", 0.07f);
        // point light 2
        modelShader.setBool("pointLights[1].on", pointlightsOn);
        modelShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        modelShader.setVec3("pointLights[1].ambient", pointLightColors[1] * 0.1f);
        modelShader.setVec3("pointLights[1].diffuse", pointLightColors[1]);
        modelShader.setVec3("pointLights[1].specular", pointLightColors[1]);
        modelShader.setFloat("pointLights[1].constant", 1.0f);
        modelShader.setFloat("pointLights[1].linear", 0.14f);
        modelShader.setFloat("pointLights[1].quadratic", 0.07f);
        // point light 3
        modelShader.setBool("pointLights[2].on", pointlightsOn);
        modelShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        modelShader.setVec3("pointLights[2].ambient", pointLightColors[2] * 0.1f);
        modelShader.setVec3("pointLights[2].diffuse", pointLightColors[2]);
        modelShader.setVec3("pointLights[2].specular", pointLightColors[2]);
        modelShader.setFloat("pointLights[2].constant", 1.0f);
        modelShader.setFloat("pointLights[2].linear", 0.22f);
        modelShader.setFloat("pointLights[2].quadratic", 0.20f);
        // point light 4
        modelShader.setBool("pointLights[3].on", pointlightsOn);
        modelShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        modelShader.setVec3("pointLights[3].ambient", pointLightColors[3] * 0.1f);
        modelShader.setVec3("pointLights[3].diffuse", pointLightColors[3]);
        modelShader.setVec3("pointLights[3].specular", pointLightColors[3]);
        modelShader.setFloat("pointLights[3].constant", 1.0f);
        modelShader.setFloat("pointLights[3].linear", 0.14f);
        modelShader.setFloat("pointLights[3].quadratic", 0.07f);
        // spotLight
        modelShader.setBool("spotLight.on", flashlightOn);
        modelShader.setVec3("spotLight.position", camera.Position);
        modelShader.setVec3("spotLight.direction", camera.Front);
        modelShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        modelShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        modelShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        modelShader.setFloat("spotLight.constant", 1.0f);
        modelShader.setFloat("spotLight.linear", 0.09f);
        modelShader.setFloat("spotLight.quadratic", 0.032f);
        modelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
        modelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        
        // View/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        
        // Render the nanosuit model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.75f, -10.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        modelShader.setMat4("model", model);
        nanosuitModel.Draw(modelShader);

        
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
    
    // Press H to toggle the PointLights
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && pointlightsCooldown == 0)
    {
        if (pointlightsOn)
        {
            pointlightsOn = false;
            pointlightsCooldown = 200;
        }
        else
        {
            pointlightsOn = true;
            pointlightsCooldown = 200;
        }
    }
    
    // Press G to toggle the DirLight
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && dirlightCooldown == 0)
    {
        if (dirlightOn)
        {
            dirlightOn = false;
            dirlightCooldown = 200;
        }
        else
        {
            dirlightOn = true;
            dirlightCooldown = 200;
        }
    }
    
    // Press F to toggle the FlashLight
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && flashlightCooldown == 0)
    {
        if (flashlightOn)
        {
            flashlightOn = false;
            flashlightCooldown = 200;
        }
        else
        {
            flashlightOn = true;
            flashlightCooldown = 200;
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

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * imagePath)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    char path[] = "C:\\Users\\Alessio\\Documents\\GitHub\\Progetto_RTGP\\work\\";
    
    char imgPath[strlen(path) + strlen(imagePath)] = "";
    strcat(imgPath, path);
    strcat(imgPath, imagePath);
    
    printf("%s\n", imgPath);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(imgPath, &width, &height, &nrComponents, 0);
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
        std::cout << "Texture failed to load at path: " << imgPath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}