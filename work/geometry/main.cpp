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

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

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

btRigidBody* spheres[600];
glm::vec3 spheresLastPositions[600];
int sphereIndex = -1;

glm::vec3 verticesToDeform[600];
glm::vec3 hittingDirections[600];
int sphereDirCooldown = 0;
int index_vtd = 0;

// Lighting
glm::vec3 pointLightPositions[] = {
	glm::vec3( 5.0f, 10.0f, 10.0f ),
	glm::vec3( 5.0f, 10.0f, -10.0f ),
	glm::vec3( -5.0f, 10.0f, 10.0f ),
	glm::vec3( -5.0f, 10.0f, -10.0f )
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

int main()
{
    dirlightOn = false;
    pointlightsOn = false;
    flashlightOn = false;
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
    
    // Initialize verticesToDeform
    for (int i = 0; i<100; i++)
    {
        verticesToDeform[i] = glm::vec3(0.0f);
    }
    
//    for (int i = 0; i<100; i++)
//    {
//        printf("%lf, %lf, %lf", verticesToDeform[i].x, verticesToDeform[i].y, verticesToDeform[i].z);
//    }
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shader("geometry\\shaders\\explode.VERT", "geometry\\shaders\\explode.FRAG", "geometry\\shaders\\explode.GEO");
    // the Shader Program for the objects used in the application
    Shader object_shader("geometry\\shaders\\13_phong.vert", "geometry\\shaders\\14_ggx.frag");
    Shader deformShader("geometry\\shaders\\shader.VERT", "geometry\\shaders\\shader.FRAG"); //"geometry\\shaders\\shader.GEO");

    // load models
    // -----------
    Model nanosuitModel("C:\\Users\\Alessio\\Documents\\GitHub\\Progetto_RTGP\\models\\nanosuit\\nanosuit.obj");
    Model cubeModel("C:\\Users\\Alessio\\Documents\\GitHub\\Progetto_RTGP\\models\\highCube.obj");
    Model sphereModel("C:\\Users\\Alessio\\Documents\\GitHub\\Progetto_RTGP\\models\\sphere.obj");
    
    // dimensions and position of the static plane
    glm::vec3 plane_pos = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 plane_size = glm::vec3(200.0f, 0.1f, 200.0f);
    glm::vec3 plane_rot = glm::vec3(0.0f, 0.0f, 0.0f);
    
    btRigidBody* plane = bulletSimulation.createRigidBody(BOX, plane_pos, plane_size, plane_rot, 0.0f, 0.3f, 0.3f);
    
    GLint num_side = 1;
    // total number of the cubes
    GLint total_cubes = num_side*num_side;
    GLint i,j;
    
    glm::vec3 cube_pos;

    vector<glm::vec3> positions;
    // dimension of the cube
    glm::vec3 cube_size = glm::vec3(2.5f, 2.5f, 2.5f);
    // we set a small initial rotation for the cubes
    glm::vec3 cube_rot = glm::vec3(0.0f, 0.0f, 0.0f);

    // dimension of the bullets
    glm::vec3 sphere_size = glm::vec3(0.1f, 0.1f, 0.1f);
    
    btRigidBody* cube;

    // we create a 5x5 grid of rigid bodies
    for(i = 0; i < num_side; i++ )
    {
        for(j = 0; j < num_side; j++ )
        {
            cube_pos = glm::vec3((i - num_side)+3, -0.5f, (num_side - j));
            float mass = 9999999.0f;
//            glm::vec3 radius = glm::vec3(1.5f, 1.5f, 1.5f);
            cube = bulletSimulation.createRigidBody(BOX, cube_pos, cube_size, cube_rot, mass, 0.3f, 0.3f);
//            cube = bulletSimulation.createRigidBody(SPHERE, cube_pos, radius, cube_rot, mass, 0.3f, 0.3f);
        }
    }

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    projection = glm::perspective(45.0f, (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.05f, 100.0f);

    GLfloat maxSecPerFrame = 60.0f;
    
    float lineVertices[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };
    
    // line VAO
    unsigned int lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), &lineVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

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
        
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
//        processInput(window);
        // Check is an I/O event is happening
        glfwPollEvents();
        // we apply FPS camera movements
        apply_camera_movements();

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // we set the rendering mode
        if (wireframe)
        // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
        bulletSimulation.dynamicsWorld->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame), 0);
        
        /////////////////// OBJECTS ////////////////////////////////////////////////
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

        /////
        // STATIC PLANE
        GLint planeDiffuseLocation = glGetUniformLocation(object_shader.ID, "diffuseColor");
        glUniform3fv(planeDiffuseLocation, 1, planeMaterial);

        // The plane is static, so its Collision Shape is not subject to forces, and it does not move. Thus, we do not need to use dynamicsWorld to acquire the rototraslations, but we can just use directly glm to manage the matrices
        // if, for some reason, the plane becomes a dynamic rigid body, the following code must be modified
        glm::mat4 planeModelMatrix;
        glm::mat3 planeNormalMatrix;
        planeModelMatrix = glm::translate(planeModelMatrix, plane_pos);
        planeModelMatrix = glm::scale(planeModelMatrix, plane_size);
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(object_shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(object_shader.ID, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));

        // we render the plane
        cubeModel.Draw(object_shader);
        planeModelMatrix = glm::mat4(1.0f);
        
        
        // GET COLLISIONS POINTS
        bulletSimulation.dynamicsWorld->debugDrawWorld();
        ///one way to draw all the contact points is iterating over contact manifolds in the dispatcher:

        int numManifolds = bulletSimulation.dynamicsWorld->getDispatcher()->getNumManifolds();
        
        glm::vec3 contactPoint1, contactPoint2;
        double distance;
        
        for (i=0;i<numManifolds;i++)
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
        
        
        ///// Render the deformable objects
        // model and normal matrices
        glm::mat4 objModelMatrix;
        glm::mat3 objNormalMatrix;

        GLfloat matrix[16];
        btTransform transform;

        glm::vec3 obj_size;
        Model* objectModel;
        Shader* objectShader;

        GLint objDiffuseLocation = glGetUniformLocation(objectShader->ID, "diffuseColor");

        int num_cobjs = bulletSimulation.dynamicsWorld->getNumCollisionObjects();

        for(i = 1; i < num_cobjs; i++ )
        {
            if (i <= total_cubes)
            {
                objectModel = &cubeModel;
                objectShader = &deformShader;
                obj_size = cube_size;
                glUniform3fv(objDiffuseLocation, 1, diffuseColor);
            }
            else
            {
                objectModel = &sphereModel;
                objectShader = &object_shader;
                obj_size = sphere_size;
                glUniform3fv(objDiffuseLocation, 1, shootColor);
                
//                glm::vec4 sphereDirection = lastSpherePosition - spherePosition;
            }
            
            objectShader->use();
            
            // we pass projection and view matrices to the Shader Program
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

            // we determine the position in the Shader Program of the uniform variable
            GLint pointLightLocation = glGetUniformLocation(objectShader->ID, "pointLightPosition");
            GLint pointLightLocation1 = glGetUniformLocation(objectShader->ID, "pointLightPosition1");
            GLint kdLocation = glGetUniformLocation(objectShader->ID, "Kd");
            GLint alphaLocation = glGetUniformLocation(objectShader->ID, "alpha");
            GLint f0Location = glGetUniformLocation(objectShader->ID, "F0");

            // we assign the value to the uniform variable
            glUniform3fv(pointLightLocation, 1, glm::value_ptr(lightPos0));
            glUniform3fv(pointLightLocation1, 1, glm::value_ptr(lightPos1));
            glUniform1f(kdLocation, Kd);
            glUniform1f(alphaLocation, alpha);
            glUniform1f(f0Location, F0);
        
            btCollisionObject* obj = bulletSimulation.dynamicsWorld->getCollisionObjectArray()[i];
            
            btRigidBody* body = btRigidBody::upcast(obj);
            body->getMotionState()->getWorldTransform(transform);
            transform.getOpenGLMatrix(matrix);
            
//            glm::vec3 pos = glm::vec3(transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z());
//            
//            glm::vec3 direction = pos - spheresLastPositions[i-2];
//            
//            if (sphereDirCooldown == 0)
//            {
//                spheresDirections[i-2] = direction;
//                spheresLastPositions[i-2] = pos;
//                sphereDirCooldown = 500000;
//            }
            
            objModelMatrix = glm::make_mat4(matrix)*glm::scale(objModelMatrix, obj_size);
            objNormalMatrix = glm::inverseTranspose(glm::mat3(view*objModelMatrix));
            
            glUniformMatrix4fv(glGetUniformLocation(objectShader->ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(objModelMatrix));
            glUniformMatrix3fv(glGetUniformLocation(objectShader->ID, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(objNormalMatrix));
        
            if (index_vtd + 1 < 999 && (contactPoint1.x != 0.0f && contactPoint1.y != 0.0f && contactPoint1.z != 0.0f))
            {
                if (index_vtd == 0 || (index_vtd > 0 && verticesToDeform[index_vtd-1].x != contactPoint1.x
                                                     && verticesToDeform[index_vtd-1].y != contactPoint1.y 
                                                     && verticesToDeform[index_vtd-1].z != contactPoint1.z))
                {
                    hittingDirections[index_vtd] = glm::vec3(camera.Front.x, camera.Front.y, camera.Front.z);
                    verticesToDeform[index_vtd++] = glm::vec3(contactPoint1.x, contactPoint1.y, contactPoint1.z);
                    printf("VERTICE MESH WORLD: %lf %lf %lf\n", contactPoint1.x, contactPoint1.y, contactPoint1.z);
                    printf("%d\n", index_vtd);
                }
            }
            
            // renderizza il modello
            if (objectShader == &deformShader)
            {
                glUniform3fv(glGetUniformLocation(objectShader->ID, "impactPoints"), 1000, glm::value_ptr(verticesToDeform[0]));
                glUniform3fv(glGetUniformLocation(objectShader->ID, "hittingDirections"), 1000, glm::value_ptr(hittingDirections[0]));
                glUniform3fv(objDiffuseLocation, 1, diffuseColor);
                
                objectShader->setVec3("viewPos", camera.Position);
                
                // Setting Material Properties
                objectShader->setFloat("materialShininess", 32.0f);
                
                // directional light
                objectShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
                objectShader->setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
                objectShader->setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
                objectShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
                objectShader->setBool("dirLight.on", dirlightOn);
                // point light 1
                objectShader->setBool("pointLights[0].on", pointlightsOn);
                objectShader->setVec3("pointLights[0].position", pointLightPositions[0]);
                objectShader->setVec3("pointLights[0].ambient", pointLightColors[0] * 0.1f);
                objectShader->setVec3("pointLights[0].diffuse", pointLightColors[0]);
                objectShader->setVec3("pointLights[0].specular", pointLightColors[0]);
                objectShader->setFloat("pointLights[0].constant", 1.0f);
                objectShader->setFloat("pointLights[0].linear", 0.14f);
                objectShader->setFloat("pointLights[0].quadratic", 0.07f);
                // point light 2
                objectShader->setBool("pointLights[1].on", pointlightsOn);
                objectShader->setVec3("pointLights[1].position", pointLightPositions[1]);
                objectShader->setVec3("pointLights[1].ambient", pointLightColors[1] * 0.1f);
                objectShader->setVec3("pointLights[1].diffuse", pointLightColors[1]);
                objectShader->setVec3("pointLights[1].specular", pointLightColors[1]);
                objectShader->setFloat("pointLights[1].constant", 1.0f);
                objectShader->setFloat("pointLights[1].linear", 0.14f);
                objectShader->setFloat("pointLights[1].quadratic", 0.07f);
                // point light 3
                objectShader->setBool("pointLights[2].on", pointlightsOn);
                objectShader->setVec3("pointLights[2].position", pointLightPositions[2]);
                objectShader->setVec3("pointLights[2].ambient", pointLightColors[2] * 0.1f);
                objectShader->setVec3("pointLights[2].diffuse", pointLightColors[2]);
                objectShader->setVec3("pointLights[2].specular", pointLightColors[2]);
                objectShader->setFloat("pointLights[2].constant", 1.0f);
                objectShader->setFloat("pointLights[2].linear", 0.22f);
                objectShader->setFloat("pointLights[2].quadratic", 0.20f);
                // point light 4
                objectShader->setBool("pointLights[3].on", pointlightsOn);
                objectShader->setVec3("pointLights[3].position", pointLightPositions[3]);
                objectShader->setVec3("pointLights[3].ambient", pointLightColors[3] * 0.1f);
                objectShader->setVec3("pointLights[3].diffuse", pointLightColors[3]);
                objectShader->setVec3("pointLights[3].specular", pointLightColors[3]);
                objectShader->setFloat("pointLights[3].constant", 1.0f);
                objectShader->setFloat("pointLights[3].linear", 0.14f);
                objectShader->setFloat("pointLights[3].quadratic", 0.07f);
                // spotLight
                objectShader->setBool("spotLight.on", flashlightOn);
                objectShader->setVec3("spotLight.position", camera.Position);
                objectShader->setVec3("spotLight.direction", camera.Front);
                objectShader->setVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
                objectShader->setVec3("spotLight.diffuse", 0.4f, 0.4f, 0.4f);
                objectShader->setVec3("spotLight.specular", 0.4f, 0.4f, 0.4f);
                objectShader->setFloat("spotLight.constant", 1.0f);
                objectShader->setFloat("spotLight.linear", 0.09f);
                objectShader->setFloat("spotLight.quadratic", 0.032f);
                objectShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
                objectShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
                
//                objectShader->setVec3("contactPoint1", contactPoint1);
//                objectShader->setVec3("contactPoint2", contactPoint2);
//                objectShader->setFloat("distance", (float)distance);
            }
            else
                glUniform3fv(objDiffuseLocation, 1, shootColor);
            objectModel->Draw(*objectShader);
            objModelMatrix = glm::mat4(1.0f);
        }
        

        // configure transformation matrices
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.05f, 100.0f);
        view = camera.GetViewMatrix();;
        glm::mat4 model = glm::mat4(1.0f);
        shader.use();
        model = glm::translate(model, glm::vec3(0.0f, -0.8f, -10.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // add time component to geometry shader in the form of a uniform
        shader.setFloat("time", glfwGetTime());

        // draw model
        nanosuitModel.Draw(shader);

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
    
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        btVector3 pos, impulse;
        glm::vec3 radius = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec4 shoot;
        GLfloat shootInitialSpeed = 30.0f;
        btRigidBody* sphere;
        glm::mat4 unproject;
        
        sphere = bulletSimulation.createRigidBody(SPHERE, camera.Position, radius, rot, 1, 0.3f, 0.3f);
        shoot.x = camera.Front.x/SCR_WIDTH;
        shoot.y = camera.Front.y/SCR_HEIGHT;
        shoot.z = 1.0f;
        shoot.w = 1.0f;
        
        unproject = glm::inverse(projection*view);
        shoot = glm::normalize(unproject*shoot) * shootInitialSpeed;
        
        impulse = btVector3(shoot.x, shoot.y, shoot.z);
        sphere->applyCentralImpulse(impulse);
        
        spheres[++sphereIndex] = sphere;
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

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

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