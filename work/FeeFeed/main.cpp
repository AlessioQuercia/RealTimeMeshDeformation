#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/shader.h>
#include <utils/shader_fee.h>
#include <utils/camera.h>
#include <utils/model_v2.h>
#include <utils/physics.h>
#include <vector>

#include <bullet/btBulletDynamicsCommon.h>

#include <iostream>
#include <string>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H


struct Character {
    GLuint     TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;       // Size of glyph
    glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
    GLuint     Advance;    // Offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
GLuint VAO, VBO;
void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();
float getDistance(glm::vec3 point1, glm::vec3 point2);
glm::vec3 checkCollisions();
void updateMeshes();
unsigned int loadTexture(const char *path);
int getHitModel(glm::vec3 hitPoint, glm::vec3* cubes_pos, glm::vec3* cubes_size);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

const unsigned int TEXTURE_WIDTH = 1024;
const unsigned int TEXTURE_HEIGHT = 1024;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
GLfloat lastX = SCR_WIDTH / 2.0;
GLfloat lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;
double cursorX, cursorY;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Bullet simulation
Physics bulletSimulation;

// we initialize an array of booleans for each keybord key
bool keys[1024];

// view and projection matrices (global because we need to use them in the keyboard callback)
glm::mat4 view, projection;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// Uniforms to be passed to shaders
// point light position
glm::vec3 lightPos0 = glm::vec3(5.0f, 10.0f, 10.0f);
glm::vec3 lightPos1 = glm::vec3(5.0f, 10.0f, -10.0f);

// weight for the diffusive component
GLfloat Kd = 3.0f;
// roughness index for Cook-Torrance shader
GLfloat alpha = 0.2f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0 = 0.9f;

// color of the falling objects
GLfloat diffuseColor[] = {0.2,0.2,0.2};
// color of the plane
GLfloat planeMaterial[] = {0.0,0.5,0.0};
// color of the bullets
GLfloat shootColor[] = {1.0,1.0,0.0};

glm::vec3 verticesToDeform[600];
glm::vec3 hittingDirections[600];
int sphereDirCooldown = 0;
int index_vtd = 0;

// Lighting
glm::vec3 pointLightPositions[] = {
	glm::vec3( 5.0f, 5.0f, 5.0f ),
	glm::vec3( 5.0f, 5.0f, -5.0f ),
	glm::vec3( -5.0f, 5.0f, 5.0f ),
	glm::vec3( -5.0f, 5.0f, -5.0f )
};

glm::vec3 pointLightColors[] = {
    glm::vec3(0.7f, 0.7f, 0.7f),
    glm::vec3(0.7f, 0.7f, 0.7f),
    glm::vec3(0.7f, 0.7f, 0.7f),
    glm::vec3(0.7f, 0.7f, 0.7f)
};

bool dirlightOn;
int dirlightCooldown;

bool pointlightsOn;
int pointlightsCooldown;

bool flashlightOn;
int flashlightCooldown;

bool shooting;
int shootingCooldown;

glm::vec3 contactPoint1, contactPoint2;

// dimensions and position of the static plane
glm::vec3 plane_pos = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 plane_size = glm::vec3(50.0f, 0.1f, 50.0f);
glm::vec3 plane_rot = glm::vec3(0.0f, 0.0f, 0.0f);

GLint num_side = 4;
// total number of the cubes
GLint total_cubes = num_side*num_side;

glm::vec3 cube_pos;

//vector<glm::vec3> directions;
//vector<glm::vec3> positions;
// dimension of the cube
glm::vec3 cube_size = glm::vec3(2.5f, 2.5f, 2.5f);
// we set a small initial rotation for the cubes
glm::vec3 cube_rot = glm::vec3(0.0f, 0.0f, 0.0f);

// dimension of the bullets
glm::vec3 sphere_size = glm::vec3(0.1f, 0.1f, 0.1f);

btRigidBody* cube;
glm::vec3 radius = glm::vec3(2.5f, 2.5f, 2.5f);
// float mass = 9999999.0f;
float mass = 0.0f;  // static

bool hit;
bool first;

int main()
{
    dirlightOn = false;
    pointlightsOn = false;
    flashlightOn = false;
    shooting = false;
    first = true;
    dirlightCooldown = 0;
    pointlightsCooldown = 0;
    flashlightCooldown = 0;
    shootingCooldown = 0;
    
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
    
    ///////////////// TEXTS ////////////////
    
    // Set OpenGL options
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile and setup the shader
    Shader shader("..\\shaders\\text.VERT", "..\\shaders\\text.FRAG");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    if (FT_New_Face(ft, "..\\..\\..\\fonts\\segoepr.ttf", 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl; 
        
    FT_Set_Pixel_Sizes(face, 0, 48);  
    
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl; 


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
      
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture, 
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    
        float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    }; 
    
    
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    
    // Initialize verticesToDeform
    for (int i = 0; i<100; i++)
    {
        verticesToDeform[i] = glm::vec3(0.0f);
    }
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    // the Shader Program for the objects used in the application
    Shader object_shader("..\\shaders\\13_phong.vert", "..\\shaders\\14_ggx.frag");
    Shader deformShader("..\\shaders\\shaderNM.VERT", "..\\shaders\\shaderNM.FRAG");   
    ShaderFee feedbackShader("..\\shaders\\feedback.VERT");
    Shader skyboxShader("..\\shaders\\skyboxV.VERT", "..\\shaders\\skyboxF.FRAG");
    
    vector<std::string> faces
    {
        "..\\resources\\skybox\\right.jpg",
        "..\\resources\\skybox\\left.jpg",
        "..\\resources\\skybox\\top.jpg",
        "..\\resources\\skybox\\bottom.jpg",
        "..\\resources\\skybox\\front.jpg",
        "..\\resources\\skybox\\back.jpg"
    };
    
    unsigned int cubemapTexture = loadCubemap(faces);

    // load models
    // -----------
    Model cubeModel("..\\..\\..\\models\\cube2\\cube.obj");
    Model planeModel("..\\..\\..\\models\\cube2\\cube.obj");
    Model sphereModel("..\\..\\..\\models\\sphere.obj", 1);
    
    Model cubes[total_cubes] = { 
                                // ITEMS
                                Model("..\\..\\..\\models\\cube2\\highCube.obj"),
                                Model("..\\..\\..\\models\\cube2\\highCube.obj"),
                                Model("..\\..\\..\\models\\cube2\\highCube.obj"),
                                Model("..\\..\\..\\models\\cube2\\cube.obj"),
                                Model("..\\..\\..\\models\\cube2\\cube.obj"),
                                Model("..\\..\\..\\models\\cube2\\cube.obj"),
                                Model("..\\..\\..\\models\\sphere\\veryHighSphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\veryHighSphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\veryHighSphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\veryHighSphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\veryHighSphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\sphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\sphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\sphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\sphere.obj", 1),
                                Model("..\\..\\..\\models\\sphere\\sphere.obj", 1)
                                };
                       
    glm::vec3 cubes_pos[total_cubes];
    glm::vec3 cubes_size[total_cubes];
    
    int xoff = 0;
    int zoff = 0;
    
    for (int i = 0; i<2; i++)
    {
        for (int j = 0; j<2; j++)
        {
            btRigidBody* plane = bulletSimulation.createRigidBody(BOX, glm::vec3(plane_pos.x - xoff, plane_pos.y, plane_pos.z + zoff), plane_size, plane_rot, 0.0f, 0.3f, 0.3f);
            xoff += 100;
        }
        xoff = 0;
        zoff += 100;
    }

    int cnt = 0;
    // we create a 5x5 grid of rigid bodies
    for(int i = 0; i < num_side; i++ )
    {
        for(int j = 0; j < num_side; j++ )
        {
            cubes_pos[cnt] = glm::vec3((i - num_side)*15, 1.6f, (num_side - j)*15);
            cubes_size[cnt] = cube_size;
                
            if (cubes[cnt].type == 1)
                cube = bulletSimulation.createRigidBody(SPHERE, cubes_pos[cnt], radius, cube_rot, mass, 0.3f, 0.3f);
            else
                cube = bulletSimulation.createRigidBody(BOX, cubes_pos[cnt], radius, cube_rot, mass, 0.3f, 0.3f);
            cnt++;
        }
    }

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    projection = glm::perspective(45.0f, (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100000.0f);

    GLfloat maxSecPerFrame = 60.0f;
    
    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int normalMap = loadTexture("..\\..\\..\\models\\cube2\\normalMap.png");
    unsigned int displacementMap = loadTexture("..\\..\\..\\models\\cube2\\displacementMap2.png");
    
    // load textures
    // -------------
    unsigned int cubeTexture = loadTexture("..\\..\\..\\textures\\high\\4k.jpg");
    unsigned int floorTexture = loadTexture("..\\..\\..\\textures\\ground_mud.jpg");
    
    // framebuffer configuration
    // -------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    GLuint tbo;

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &tbo);
    
    char fps[100];
    char* fps_text = "FPS: ";
    std::string fps_num;
    
    double startTime = glfwGetTime();
    int nbFrames = 0;
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        if (sphereDirCooldown > 0)
            sphereDirCooldown--;
        if (dirlightCooldown > 0)
            dirlightCooldown--;
        if (pointlightsCooldown > 0)
            pointlightsCooldown--;
        if (flashlightCooldown > 0)
            flashlightCooldown--;
        if (shootingCooldown > 0)
            shootingCooldown--;
        
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        double currentTime = glfwGetTime();
        nbFrames++;
        
        if (currentTime - startTime >= 1.0)
        {
            fps_num = std::to_string(nbFrames);
            startTime = glfwGetTime();
            nbFrames = 0;
        }
        
//        cout<<fps_num<<endl;

        // input
        // -----
//        processInput(window);
        // Check is an I/O event is happening
        glfwPollEvents();
        // we apply FPS camera movements
        apply_camera_movements();
        
        view = camera.GetViewMatrix();

        // render
        // ------
        
        // we set the rendering mode
        if (wireframe)
        // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
        bulletSimulation.dynamicsWorld->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame), 0);
        
        //////////////////////////    PRE - RENDER SCENE    //////////////////////////
        
        // 1 - Check whether there are new collision points. If yes, store them and their respective directions.
        
        glm::vec3 hitPoint = checkCollisions();
        
        // 2 - Update the vertices by capturing them as feedback
        
        if (hit)
        {
            if (first)
                first = false;
            
            int hitModel = getHitModel(hitPoint, cubes_pos, cubes_size);
      
            int ind = 0;
            int dim = 0;
            
            for (int i = 0; i<cubes[hitModel].meshes.size(); i++)
                dim += cubes[hitModel].meshes[i].vertices.size();

            glm::vec3 data[dim*2];
            
            for (int i = 0; i<cubes[hitModel].meshes.size(); i++)
            {
                for(int j=0; j<cubes[hitModel].meshes[i].vertices.size(); j++)
                {
                    data[ind++] = glm::vec3(cubes[hitModel].meshes[i].vertices[j].Position.x,
                        cubes[hitModel].meshes[i].vertices[j].Position.y, cubes[hitModel].meshes[i].vertices[j].Position.z);
                    data[ind++] = glm::vec3(cubes[hitModel].meshes[i].vertices[j].Normal.x,
                        cubes[hitModel].meshes[i].vertices[j].Normal.y, cubes[hitModel].meshes[i].vertices[j].Normal.z);
                }
            }
        
            glm::mat4 model;
            model = glm::translate(model, cubes_pos[hitModel]);
            model = glm::scale(model, cube_size);
                
            glBindVertexArray(vao);
            
            feedbackShader.use();
            feedbackShader.setMat4("model", model);
            feedbackShader.setMat4("view", view);
            feedbackShader.setMat4("projection", projection);
            feedbackShader.setVec3("hitPoint", hitPoint);
            feedbackShader.setVec3("hitDirection", camera.Front);
            
//            printf("HITPOINT: %f %f %f\n", hitPoint.x, hitPoint.y, hitPoint.z);
                    
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

            GLint inputAttrib = glGetAttribLocation(feedbackShader.ID, "position");
            glEnableVertexAttribArray(inputAttrib);
            glVertexAttribPointer(inputAttrib, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)0);
            GLint inputAttrib2 = glGetAttribLocation(feedbackShader.ID, "normal");
            glEnableVertexAttribArray(inputAttrib2);
            glVertexAttribPointer(inputAttrib2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)(sizeof(glm::vec3)));
            
            glm::vec3 feedback[dim*2];

            // Create transform feedback buffer
            glBindBuffer(GL_ARRAY_BUFFER, tbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(feedback), nullptr, GL_STATIC_READ);

            // Perform feedback transform
            glEnable(GL_RASTERIZER_DISCARD);

            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

            glBeginTransformFeedback(GL_POINTS);
                glDrawArrays(GL_POINTS, 0, sizeof(feedback));
            glEndTransformFeedback();

            glDisable(GL_RASTERIZER_DISCARD);

            glFlush();

            // Fetch and print results
            glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);
            
            int j = 0;
            for(int i = 0; i<dim*2; i+=2)
            {
                j = i/2;
                
                data[i] = feedback[i];
                data[i+1] = feedback[i+1];
            }
            
            cubes[hitModel].UpdateData(data);
        }
        
        // 3 - Render the scene
        
        //////////////////////////    RENDER SCENE    //////////////////////////
        
        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
        
        // clear all relevant buffers
        glClearColor(0.0f, 0.0f, 0.4f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // We "install" the selected Shader Program as part of the current rendering process
        object_shader.use();

        // we pass projection and view matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(object_shader.ID, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(object_shader.ID, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        // we determine the position in the Shader Program of the uniform variable
        GLint pointLightLocation = glGetUniformLocation(object_shader.ID, "pointLightPosition");
        GLint kdLocation = glGetUniformLocation(object_shader.ID, "Kd");
        GLint alphaLocation = glGetUniformLocation(object_shader.ID, "alpha");
        GLint f0Location = glGetUniformLocation(object_shader.ID, "F0");

        // we assign the value to the uniform variable
        glUniform3fv(pointLightLocation, 1, glm::value_ptr(lightPos0));
        glUniform1f(kdLocation, Kd);
        glUniform1f(alphaLocation, alpha);
        glUniform1f(f0Location, F0);
        
        ///// Render the deformable objects
        // model and normal matrices
        glm::mat4 objModelMatrix;
        glm::mat3 objNormalMatrix;

        GLfloat matrix[16];
        btTransform transform;

        glm::vec3 obj_size;
        Model* objectModel;
        Shader* objectShader;
        
        for(int i = 0; i < total_cubes; i++)
        {
            deformShader.use();
            glm::mat4 model;
            model = glm::translate(model, cubes_pos[i]);
            model = glm::scale(model, cubes_size[i]);
            deformShader.setMat4("model", model);
            deformShader.setMat4("view", view);
            deformShader.setMat4("projection", projection);
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, cubeTexture);
            deformShader.setInt("texture1", 1);

            deformShader.setVec3("viewPos", camera.Position);
            
            // Setting Material Properties
            deformShader.setFloat("materialShininess", 32.0f);
            
            // directional light
            deformShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
            deformShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
            deformShader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
            deformShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
            deformShader.setBool("dirLight.on", dirlightOn);
            // point light 1
            deformShader.setBool("pointLights[0].on", pointlightsOn);
            deformShader.setVec3("pointLights[0].position", pointLightPositions[0]);
            deformShader.setVec3("pointLights[0].ambient", pointLightColors[0] * 0.1f);
            deformShader.setVec3("pointLights[0].diffuse", pointLightColors[0]);
            deformShader.setVec3("pointLights[0].specular", pointLightColors[0]);
            deformShader.setFloat("pointLights[0].constant", 1.0f);
            deformShader.setFloat("pointLights[0].linear", 0.14f);
            deformShader.setFloat("pointLights[0].quadratic", 0.07f);
            // point light 2
            deformShader.setBool("pointLights[1].on", pointlightsOn);
            deformShader.setVec3("pointLights[1].position", pointLightPositions[1]);
            deformShader.setVec3("pointLights[1].ambient", pointLightColors[1] * 0.1f);
            deformShader.setVec3("pointLights[1].diffuse", pointLightColors[1]);
            deformShader.setVec3("pointLights[1].specular", pointLightColors[1]);
            deformShader.setFloat("pointLights[1].constant", 1.0f);
            deformShader.setFloat("pointLights[1].linear", 0.14f);
            deformShader.setFloat("pointLights[1].quadratic", 0.07f);
            // point light 3
            deformShader.setBool("pointLights[2].on", pointlightsOn);
            deformShader.setVec3("pointLights[2].position", pointLightPositions[2]);
            deformShader.setVec3("pointLights[2].ambient", pointLightColors[2] * 0.1f);
            deformShader.setVec3("pointLights[2].diffuse", pointLightColors[2]);
            deformShader.setVec3("pointLights[2].specular", pointLightColors[2]);
            deformShader.setFloat("pointLights[2].constant", 1.0f);
            deformShader.setFloat("pointLights[2].linear", 0.22f);
            deformShader.setFloat("pointLights[2].quadratic", 0.20f);
            // point light 4
            deformShader.setBool("pointLights[3].on", pointlightsOn);
            deformShader.setVec3("pointLights[3].position", pointLightPositions[3]);
            deformShader.setVec3("pointLights[3].ambient", pointLightColors[3] * 0.1f);
            deformShader.setVec3("pointLights[3].diffuse", pointLightColors[3]);
            deformShader.setVec3("pointLights[3].specular", pointLightColors[3]);
            deformShader.setFloat("pointLights[3].constant", 1.0f);
            deformShader.setFloat("pointLights[3].linear", 0.14f);
            deformShader.setFloat("pointLights[3].quadratic", 0.07f);
            // spotLight
            deformShader.setBool("spotLight.on", flashlightOn);
            deformShader.setVec3("spotLight.position", camera.Position);
            deformShader.setVec3("spotLight.direction", camera.Front);
            deformShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
            deformShader.setVec3("spotLight.diffuse", 0.7f, 0.7f, 0.7f);
            deformShader.setVec3("spotLight.specular", 0.9f, 0.9f, 0.9f);
            deformShader.setFloat("spotLight.constant", 1.0f);
            deformShader.setFloat("spotLight.linear", 0.09f);
            deformShader.setFloat("spotLight.quadratic", 0.032f);
            deformShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
            deformShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

            cubes[i].Draw(deformShader);
        }
            
        int x_offset = 0;
        int z_offset = 0;
        
        for (int i = 0; i<2; i++)
        {
            for (int j = 0; j<2; j++)
            {
                // The plane is static, so its Collision Shape is not subject to forces, and it does not move. Thus, we do not need to use dynamicsWorld to acquire the rototraslations, but we can just use directly glm to manage the matrices
                // if, for some reason, the plane becomes a dynamic rigid body, the following code must be modified
                glm::mat4 planeModelMatrix;
                planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(plane_pos.x - x_offset, plane_pos.y, plane_pos.z + z_offset));
                planeModelMatrix = glm::scale(planeModelMatrix, plane_size);
                glUniformMatrix4fv(glGetUniformLocation(object_shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
                
                deformShader.setMat4("model", planeModelMatrix);
                deformShader.setMat4("view", view);
                deformShader.setMat4("projection", projection);
            
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, floorTexture);
                deformShader.setInt("texture1", 1);

                // we render the plane
                planeModel.Draw(deformShader);
                planeModelMatrix = glm::mat4(1.0f);
                
                x_offset += 100;
            }
            
            x_offset = 0;
            z_offset += 100;
        }


        int num_cobjs = bulletSimulation.dynamicsWorld->getNumCollisionObjects();
        int ind = 0;

        for(int i = 1; i < num_cobjs; i++ )
        {
            if (i <= total_cubes)
            {
                continue;
            }
            else
            {
                objectModel = &sphereModel;
                objectShader = &object_shader;
                obj_size = sphere_size;
            }
            
            objectShader->use();
            GLint objDiffuseLocation = glGetUniformLocation(objectShader->ID, "diffuseColor");
            
            // we pass projection and view matrices to the Shader Program
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
            btCollisionObject* obj = bulletSimulation.dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            body->getMotionState()->getWorldTransform(transform);
            transform.getOpenGLMatrix(matrix);
            
            objModelMatrix = glm::make_mat4(matrix)*glm::scale(objModelMatrix, obj_size);
            objNormalMatrix = glm::inverseTranspose(glm::mat3(view*objModelMatrix));
            
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(objModelMatrix));
            glUniform3fv(objDiffuseLocation, 1, shootColor);
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(objModelMatrix));
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "viewMatrix   "), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix3fv(glGetUniformLocation(objectShader->ID, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(objNormalMatrix));
            
            objectModel->Draw(*objectShader);
            objModelMatrix = glm::mat4(1.0f);
        }
        
        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        if (shooting && shootingCooldown == 0)
        {
            shootingCooldown = 15;
            btVector3 pos, impulse;
            glm::vec3 radius = glm::vec3(0.2f, 0.2f, 0.2f);
            glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec4 shoot;
            GLfloat shootInitialSpeed = 30.0f;
            btRigidBody* sphere;
            glm::mat4 unproject;
            
            sphere = bulletSimulation.createRigidBody(SPHERE, camera.Position, sphere_size, rot, 1, 0.3f, 0.3f);
            shoot.x = camera.Front.x/SCR_WIDTH;
            shoot.y = camera.Front.y/SCR_HEIGHT;
            shoot.z = 1.0f;
            shoot.w = 1.0f;
            
//            directions.push_back(camera.Front);
//            positions.push_back(camera.Position);
            
            unproject = glm::inverse(projection*view);
            shoot = glm::normalize(unproject*shoot) * shootInitialSpeed;
            
            impulse = btVector3(shoot.x, shoot.y, shoot.z);
            sphere->applyCentralImpulse(impulse);
        }
        
        hit = false;
        
        strcpy(fps, fps_text);
        strcat(fps, fps_num.c_str());
        
        ///////////// DRAW FPS /////////////
        RenderText(shader, fps, SCR_WIDTH-100, SCR_HEIGHT-50, 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
//        glfwPollEvents();
    }

    glfwTerminate();
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
        
    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;
        
    // Press H to toggle the PointLights
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
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
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
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
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
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
    
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        shooting = true;
    }
    
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        shooting = false;
        shootingCooldown = 0;
    }
    
    // we keep trace of the pressed keys
    // with this method, we can manage 2 keys pressed at the same time:
    // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
    // using a boolean array, we can then check and manage all the keys pressed at the same time
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
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
    
    cursorX = xpos;
    cursorY = ypos;

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// --------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

//////////////////////////////////////////
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}


float getDistance(glm::vec3 point1, glm::vec3 point2)
{
    return sqrt( pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2) + pow(point1.z - point2.z, 2) );
}

glm::vec3 checkCollisions()
{
    glm::vec3 hitPoint = glm::vec3(0);
    
    // GET COLLISIONS POINTS
    bulletSimulation.dynamicsWorld->debugDrawWorld();
    ///one way to draw all the contact points is iterating over contact manifolds in the dispatcher:

    int numManifolds = bulletSimulation.dynamicsWorld->getDispatcher()->getNumManifolds();

    double distance;
    
    for (int i=0;i<numManifolds;i++)
    {
        btPersistentManifold* contactManifold = bulletSimulation.dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
//            btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
//            btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
    
        int numContacts = contactManifold->getNumContacts();
        
        for (int j=0;j<numContacts;j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getPositionWorldOnA().y() > -0.8f)
            {
//                    printf("DISTANCE: %lf\n", pt.getDistance());
//                    
                btVector3 ptA = pt.getPositionWorldOnA();
                btVector3 ptB = pt.getPositionWorldOnB();
//
//                    printf("WORLD POINT A: %lf %lf %lf\n", ptA.x(),ptA.y(),ptA.z());
//                    printf("WORLD POINT B: %lf %lf %lf\n", ptB.x(),ptB.y(),ptB.z());
                
                contactPoint1 = glm::vec3(ptA.x(), ptA.y(), ptA.z());
                contactPoint2 = glm::vec3(ptA.x(), ptA.y(), ptA.z());
                distance = pt.getDistance();
            }
        }

        //you can un-comment out this line, and then all points are removed
        contactManifold->clearManifold();	
    }
    
    if (index_vtd + 1 < 599 && (contactPoint1.x != 0.0f && contactPoint1.y != 0.0f && contactPoint1.z != 0.0f))
    {
        if (index_vtd == 0 || (index_vtd > 0 && verticesToDeform[index_vtd-1].x != contactPoint1.x
                                             && verticesToDeform[index_vtd-1].y != contactPoint1.y 
                                             && verticesToDeform[index_vtd-1].z != contactPoint1.z))
        {
            hittingDirections[index_vtd] = glm::vec3(camera.Front.x, camera.Front.y, camera.Front.z);
            verticesToDeform[index_vtd++] = glm::vec3(contactPoint1.x, contactPoint1.y, contactPoint1.z);
//                        printf("VERTICE MESH WORLD: %lf %lf %lf\n", contactPoint1.x, contactPoint1.y, contactPoint1.z);
//                        printf("%d\n", index_vtd);
            hitPoint = contactPoint1;
            hit = true;
        }
    }
    else if (index_vtd + 1 >= 599 && (contactPoint1.x != 0.0f && contactPoint1.y != 0.0f && contactPoint1.z != 0.0f))
    {
        index_vtd = 0;
    }
    
    return hitPoint;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * imagePath)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(imagePath, &width, &height, &nrComponents, 0);
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
        std::cout << "Texture failed to load at path: " << imagePath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int getHitModel(glm::vec3 hitPoint, glm::vec3* cubes_pos, glm::vec3* cubes_size)
{
    int hitModel = 0;
    
    for (int i = 0; i<total_cubes; i++)
    {
//        printf("HITPOINT: %f %f %f\n", hitPoint.x, hitPoint.y, hitPoint.z);
//        printf("CUBEPOS: %f %f %f\n", cubes_pos[i].x, cubes_pos[i].y, cubes_pos[i].z);
        float threshold = (cubes_size[i].x + cubes_size[i].y + cubes_size[i].z)/3;
        if (getDistance(hitPoint, cubes_pos[i]) < threshold + 5)
        {
//            printf("HIT MODEL: %d\n", i);
            hitModel = i;
        }
    }
    
    return hitModel;
}


unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
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