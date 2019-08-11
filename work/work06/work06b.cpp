/*
work06b

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2018/2019
Master degree in Computer Science
Universita' degli Studi di Milano
*/

/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/

// Std. Includes
#include <string>

// Loader estensioni OpenGL
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders, to load models, for FPS camera, and for physical simulation
// in this example, the Model and Mesh classes support texturing
#include <utils/shader_v1.h>
#include <utils/model_v2.h>
#include <utils/cameraProf.h>
#include <utils/physics.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <bullet/btBulletDynamicsCommon.h>

Physics bulletSimulation;


// Dimensioni della finestra dell'applicazione
GLuint screenWidth = 1366, screenHeight = 768;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// callback functions for keyboard and mouse events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();

// we initialize an array of booleans for each keybord key
bool keys[1024];

// we set the initial position of mouse cursor in the application window
GLfloat lastX = screenWidth/2.0, lastY = screenHeight/2.0;

// we will use these value to "pass" the cursor position to the keyboard callback, in order to determine the bullet trajectory
double cursorX,cursorY;

// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// view and projection matrices (global because we need to use them in the keyboard callback)
glm::mat4 view, projection;

// we create a camera. We pass the initial position as a paramenter to the constructor. In this case, we use a "floating" camera (we pass false as last parameter)
Camera camera(glm::vec3(0.0f, 0.0f, 9.0f), GL_FALSE);

// Uniforms to be passed to shaders
// point light position
glm::vec3 lightPos0 = glm::vec3(5.0f, 10.0f, 10.0f);

// weight for the diffusive component
GLfloat Kd = 3.0f;
// roughness index for Cook-Torrance shader
GLfloat alpha = 0.2f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0 = 0.9f;

// color of the falling objects
GLfloat diffuseColor[] = {0.3,0.3,0.3};
// color of the plane
GLfloat planeMaterial[] = {0.0,0.5,0.0};
// color of the bullets
GLfloat shootColor[] = {1.0,1.0,0.0};

////////////////// MAIN function ///////////////////////
int main()
{
  // Initialization of OpenGL context using GLFW
  glfwInit();
  // We set OpenGL specifications required for this application
  // In this case: 3.3 Core
  // It is possible to raise the values, in order to use functionalities of OpenGL 4.x
  // If not supported by your graphics HW, the context will not be created and the application will close
  // N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
  // in relation also to the values indicated in these GLFW commands
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // we set if the window is resizable
  // glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RGP_work06b", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // we disable the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetCursorPos(window, screenWidth/2,screenHeight/2);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
//    int width, height;
//    glfwGetFramebufferSize(window, &width, &height);
//    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    //the "clear" color for the frame buffer
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    // the Shader Program for the objects used in the application
    Shader object_shader("13_phong.vert", "14_ggx.frag");

    // we load the model(s) (code of Model class is in include/utils/model_v2.h)
    Model cubeModel("../../../models/cube.obj");
    Model sphereModel("../../../models/sphere.obj");

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
    glm::vec3 cube_size = glm::vec3(2.5f, 1.5f, 0.6f);
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
            cube_pos = glm::vec3((i - num_side)+3, 1.0f, (num_side - j));
            float mass = 10000.0f;
            cube = bulletSimulation.createRigidBody(BOX, cube_pos, cube_size, cube_rot, mass, 0.3f, 0.3f);
        }
    }

    // Projection matrix: FOV angle, aspect ratio, near and far planes
    projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);

    GLfloat maxSecPerFrame = 60.0f;
    
    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
      // we determine the time passed from the beginning
      // and we calculate time difference between current frame rendering and the previous one
      GLfloat currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // Check is an I/O event is happening
      glfwPollEvents();
      // we apply FPS camera movements
      apply_camera_movements();
      // View matrix (=camera): position, view direction, camera "up" vector
      // in this example, it has been defined as a global variable (we need it in the keyboard callback function)
      view = camera.GetViewMatrix();

      // we "clear" the frame and z buffer
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
      object_shader.Use();

      // we pass projection and view matrices to the Shader Program
      glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
      glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

      // we determine the position in the Shader Program of the uniform variable
      GLint pointLightLocation = glGetUniformLocation(object_shader.Program, "pointLightPosition");
      GLint kdLocation = glGetUniformLocation(object_shader.Program, "Kd");
      GLint alphaLocation = glGetUniformLocation(object_shader.Program, "alpha");
      GLint f0Location = glGetUniformLocation(object_shader.Program, "F0");

      // we assign the value to the uniform variable
      glUniform3fv(pointLightLocation, 1, glm::value_ptr(lightPos0));
      glUniform1f(kdLocation, Kd);
      glUniform1f(alphaLocation, alpha);
      glUniform1f(f0Location, F0);

      /////
      // STATIC PLANE
      GLint planeDiffuseLocation = glGetUniformLocation(object_shader.Program, "diffuseColor");
      glUniform3fv(planeDiffuseLocation, 1, planeMaterial);

      // The plane is static, so its Collision Shape is not subject to forces, and it does not move. Thus, we do not need to use dynamicsWorld to acquire the rototraslations, but we can just use directly glm to manage the matrices
      // if, for some reason, the plane becomes a dynamic rigid body, the following code must be modified
      glm::mat4 planeModelMatrix;
      glm::mat3 planeNormalMatrix;
      planeModelMatrix = glm::translate(planeModelMatrix, plane_pos);
      planeModelMatrix = glm::scale(planeModelMatrix, plane_size);
      planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
      glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
      glUniformMatrix3fv(glGetUniformLocation(object_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));

      // we render the plane
      cubeModel.Draw(object_shader);
      planeModelMatrix = glm::mat4(1.0f);

      /////
      GLint objDiffuseLocation = glGetUniformLocation(object_shader.Program, "diffuseColor");
      //glUniform3fv(objDiffuseLocation, 1, diffuseColor);

      // model and normal matrices
      glm::mat4 objModelMatrix;
      glm::mat3 objNormalMatrix;
      
      GLfloat matrix[16];
      btTransform transform;
      
      glm::vec3 obj_size;
      Model* objectModel;
      
      int num_cobjs = bulletSimulation.dynamicsWorld->getNumCollisionObjects();

      for(i = 1; i < num_cobjs; i++ )
      {
          if (i <= total_cubes)
          {
              objectModel = &cubeModel;
              obj_size = cube_size;
              glUniform3fv(objDiffuseLocation, 1, diffuseColor);
          }
          else
          {
              objectModel = &sphereModel;
              obj_size = sphere_size;
              glUniform3fv(objDiffuseLocation, 1, shootColor);
          }
          
//          printf("%d\n", bulletSimulation.dynamicsWorld->getDispatcher()->getNumManifolds());
          
          
            int numManifolds = bulletSimulation.dynamicsWorld->getDispatcher()->getNumManifolds();
//            for (int i=0;i<numManifolds;i++)
//            {
//                btPersistentManifold* contactManifold =  bulletSimulation.dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
//                btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
//                btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
//
//                int numContacts = contactManifold->getNumContacts();
//                for (int j=0;j<numContacts;j++)
//                {
//                    btManifoldPoint& pt = contactManifold->getContactPoint(j);
//                    if (pt.getDistance()<0.f)
//                    {
//                        const btVector3& ptA = pt.getPositionWorldOnA();
//                        const btVector3& ptB = pt.getPositionWorldOnB();
//                        const btVector3& normalOnB = pt.m_normalWorldOnB;
//                        printf("%d\n",pt.getPositionWorldOnA().getX());
//                    }
//                }
//            }


//          if (numManifolds >0)
//          {
//              btPersistentManifold* contactManifold =  bulletSimulation.dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(numManifolds-1);  
//              int numContacts = contactManifold->getNumContacts();  
//              if (numContacts > 0)
//              {
//                  btManifoldPoint& pt = contactManifold->getContactPoint(numContacts-1);
//                  if (pt.getDistance() < 0.f)
//                  {
//                      const btVector3& ptA = pt.getPositionWorldOnA();
//                      const btVector3& ptB = pt.getPositionWorldOnB();
//                      const btVector3& normalOnB = pt.m_normalWorldOnB;
//                      printf("WORLD ON A: (%f, %f, %f)\n",pt.getPositionWorldOnA().getX(), 
//                                                pt.getPositionWorldOnA().getY(), pt.getPositionWorldOnA().getZ());
//                      printf("WORLD ON B: (%f, %f, %f)\n",pt.getPositionWorldOnB().getX(), 
//                                                pt.getPositionWorldOnB().getY(), pt.getPositionWorldOnB().getZ());
//                  }
//              }
//          }
          
          btCollisionObject* obj = bulletSimulation.dynamicsWorld->getCollisionObjectArray()[i];
          btRigidBody* body = btRigidBody::upcast(obj);
          body->getMotionState()->getWorldTransform(transform);
          transform.getOpenGLMatrix(matrix);
          objModelMatrix = glm::make_mat4(matrix)*glm::scale(objModelMatrix, obj_size);
          objNormalMatrix = glm::inverseTranspose(glm::mat3(view*objModelMatrix));
          glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(objModelMatrix));
          glUniformMatrix3fv(glGetUniformLocation(object_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(objNormalMatrix));

          // renderizza il modello
          objectModel->Draw(object_shader);
          objModelMatrix = glm::mat4(1.0f);
      }

      // Faccio lo swap tra back e front buffer
      glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    object_shader.Delete();
    bulletSimulation.Clear();

    // we close and delete the created context
    glfwTerminate();
    return 0;
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

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;
    
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        btVector3 pos, impulse;
        glm::vec3 radius = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec4 shoot;
        GLfloat shootInitialSpeed = 15.0f;
        btRigidBody* sphere;
        glm::mat4 unproject;
        
        sphere = bulletSimulation.createRigidBody(SPHERE, camera.Position, radius, rot, 1, 0.3f, 0.3f);
        shoot.x = camera.Front.x/screenWidth; //(cursorX/screenWidth) * 2 - 1;
        shoot.y = camera.Front.y/screenHeight; //-(cursorY/screenHeight) * 2 + 1;
        shoot.z = 1.0f;
        shoot.w = 1.0f;
        
        unproject = glm::inverse(projection*view);
        shoot = glm::normalize(unproject*shoot) * shootInitialSpeed;
        
        impulse = btVector3(shoot.x, shoot.y, shoot.z);
        sphere->applyCentralImpulse(impulse);
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

//////////////////////////////////////////
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
      // we move the camera view following the mouse cursor
      // we calculate the offset of the mouse cursor from the position in the last frame
      // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // we save the current cursor position in 2 global variables, in order to use the values in the keyboard callback function
    cursorX = xpos;
    cursorY = ypos;

    // offset of mouse cursor position
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    // the new position will be the previous one for the next frame
    lastX = xpos;
    lastY = ypos;

    // we pass the offset to the Camera class instance in order to update the rendering
    camera.ProcessMouseMovement(xoffset, yoffset);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
