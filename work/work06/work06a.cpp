/*
Es05b: shadow rendering with shadow mapping technique
- this examole applies a VERY basic shadow mapping (with a lot of aliasing/shadow "acne"): Es06a will present also a shader with adaptive bias to avoid shadow "acne", and a final shader with PCF to smooth shadow borders

N.B. 1) The presented shader swapping method is used in OpenGL 3.3 .
        For OpenGL 4.x, shader subroutines can be used : http://www.geeks3d.com/20140701/opengl-4-shader-subroutines-introduction-3d-programming-tutorial/

N.B. 2) the application considers only a directional light. In case of more lights, and/or of different nature, the code must be modifies

N.B. 3) see https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
            http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/#basic-shadowmap
            https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/common-techniques-to-improve-shadow-depth-maps
        for further details


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

// Loader for OpenGL extensions
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

// classes developed during lab lectures to manage shaders, to load models, and for FPS camera
// in this example, the Model and Mesh classes support texturing
#include <utils/shader_v1.h>
#include <utils/model_v2.h>
#include <utils/camera.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

// dimensions of application's window
GLuint screenWidth = 800, screenHeight = 600;

// the rendering steps used in the application
enum render_passes{ SHADOWMAP, RENDER};

// callback functions for keyboard and mouse events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();

// setup of Shader Programs for the 3 shaders used in the application
void SetupShaders();
// delete Shader Programs whan application ends
void DeleteShaders();
// print on console the name of current shader
void PrintCurrentShader(int shader);

// in this application, we have isolated the models rendering using a function, which will be called in each rendering step
void RenderObjects(Shader &shader, Model &planeModel, Model &cubeModel, Model &sphereModel, Model &bunnyModel, GLint render_pass, GLuint depthMap);

// load image from disk and create an OpenGL texture
GLint LoadTexture(const char* path);

// we initialize an array of booleans for each keybord key
bool keys[1024];

// we set the initial position of mouse cursor in the application window
GLfloat lastX = 400, lastY = 300;
// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// enum data structure to manage indices for shaders swapping
enum available_ShaderPrograms{ SHADOW_BASE, SHADOW_BIAS, SHADOW_PCF_FINAL};
// strings with shaders names to print the name of the current one on console
const char * print_available_ShaderPrograms[] = { "SHADOW_BASE", "SHADOW_BIAS", "SHADOW_PCF_FINAL"};

// index of the current shader (= 0 in the beginning)
GLuint current_program = SHADOW_BASE;
// a vector for all the Shader Programs used and swapped in the application
vector<Shader> shaders;

// view matrix
glm::mat4 view;

// we create a camera. We pass the initial position as a paramenter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
Camera camera(glm::vec3(0.0f, 0.0f, 7.0f), GL_TRUE);

// in this example, we consider a directional light. We pass the direction of incoming light as an uniform to the shaders
glm::vec3 lightDir0 = glm::vec3(1.0f, 1.0f, 1.0f);

// weight for the diffusive component
GLfloat Kd = 3.0f;
// roughness index for Cook-Torrance shader
GLfloat alpha = 0.2f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0 = 0.9f;

// vector for the textures IDs
vector<GLint> textureID;

// UV repetitions
GLfloat repeat = 1.0;

/////////////////// MAIN function ///////////////////////
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
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RGP_work06a", nullptr, nullptr);
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

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    //the "clear" color for the frame buffer
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    // we create the Shader Program for the creation of the shadow map
    Shader shadow_shader("23_shadowmap.vert", "23_shadowmap.frag");
    // we create the Shader Programs used in the application for the application of the shadow map in the final rendering
    SetupShaders();

    // we load the images and store them in a vector
    textureID.push_back(LoadTexture("../../../textures/UV_Grid_Sm.png"));
    textureID.push_back(LoadTexture("../../../textures/SoilCracked.png"));

    // we load the model(s) (code of Model class is in include/utils/model_v2.h)
    Model cubeModel("../../../models/cube.obj");
    Model sphereModel("../../../models/sphere.obj");
    Model bunnyModel("../../../models/bunny_lp.obj");
    Model planeModel("../../../models/plane.obj");

    // we print on console the name of the first shader used
    PrintCurrentShader(current_program);

    /////////////////// CREATION OF BUFFER FOR THE  DEPTH MAP /////////////////////////////////////////
    // buffer dimension: too large -> performance may slow down if we have many lights; too small -> strong aliasing
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMapFBO;
    // we create a Frame Buffer Object: the first rendering step will render to this buffer, and not to the real frame buffer
    glGenFramebuffers(1, &depthMapFBO);
    // we create a texture for the depth map
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    // in the texture, we will save only the depth data of the fragments. Thus, we specify that we need to render only depth in the first rendering step
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // imposto di "clampare" per coordinate fuori dallo spazio [0,1]
    // we set to clamp the uv coordinates outside [0,1] to the color of the border
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    GLfloat borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // we bind the depth map FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    // we set that we are not calculating nor saving color data
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ///////////////////////////////////////////////////////////////////


    // Projection matrix of the camera: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);

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

        /////////////////// STEP 1 - SHADOW MAP: RENDERING OF SCENE FROM LIGHT POINT OF VIEW ////////////////////////////////////////////////
        // we set view and projection matrix for the rendering using light as a camera
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        GLfloat near_plane = -10.0f, far_plane = 10.0f;
        GLfloat frustumSize = 5.0f;
        // for a directional light, the projection is orthographic. For point lights, we should use a perspective projection
        lightProjection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, near_plane, far_plane);
        // the light is directional, so technically it has no position. We need a view matrix, so we consider a position on the the direction vector of the light
        lightView = glm::lookAt(lightDir0, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        // transformation matrix for the light
        lightSpaceMatrix = lightProjection * lightView;
        /// We "install" the  Shader Program for the shadow mapping creation
        shadow_shader.Use();
        // we pass the transformation matrix as uniform
        glUniformMatrix4fv(glGetUniformLocation(shadow_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        // we set the viewport for the first rendering step = dimensions of the depth texture
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        // we activate the FBO for the depth map rendering
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // we render the scene, using the shadow shader
        RenderObjects(shadow_shader, planeModel, cubeModel, sphereModel, bunnyModel, SHADOWMAP, depthMap);

        /////////////////// STEP 2 - SCENE RENDERING FROM CAMERA ////////////////////////////////////////////////

        // we get the view matrix from the Camera class
        view = camera.GetViewMatrix();

        // we activate back the standard Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, than we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        // we set the viewport for the final rendering step
        glViewport(0, 0, width, height);

        // We "install" the selected Shader Program as part of the current rendering process. We pass to the shader the light transformation matrix, and the depth map rendered in the first rendering step
        shaders[current_program].Use();

        // we pass projection and view matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        // we determine the position in the Shader Program of the uniform variables
        GLint lightDirLocation = glGetUniformLocation(shaders[current_program].Program, "lightVector");
        GLint kdLocation = glGetUniformLocation(shaders[current_program].Program, "Kd");
        GLint alphaLocation = glGetUniformLocation(shaders[current_program].Program, "alpha");
        GLint f0Location = glGetUniformLocation(shaders[current_program].Program, "F0");

        // we assign the value to the uniform variables
        glUniform3fv(lightDirLocation, 1, glm::value_ptr(lightDir0));
        glUniform1f(kdLocation, Kd);
        glUniform1f(alphaLocation, alpha);
        glUniform1f(f0Location, F0);

        // we render the scene
        RenderObjects(shaders[current_program], planeModel, cubeModel, sphereModel, bunnyModel, RENDER, depthMap);

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    DeleteShaders();
    shadow_shader.Delete();
    // chiudo e cancello il contesto creato
    glfwTerminate();
    return 0;
}


//////////////////////////////////////////
// we render the objects. We pass also the current rendering step, and the depth map generated in the first step, which is used by the shaders of the second step
void RenderObjects(Shader &shader, Model &planeModel, Model &cubeModel, Model &sphereModel, Model &bunnyModel, GLint render_pass, GLuint depthMap)
{
    // For the second rendering step -> we pass the shadow map to the shaders
    if (render_pass==RENDER)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        GLint shadowLocation = glGetUniformLocation(shader.Program, "shadowMap");
        glUniform1i(shadowLocation, 2);
    }
    // we pass the needed uniforms
    GLint textureLocation = glGetUniformLocation(shader.Program, "tex");
    GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");

    // PLANE
    // we activate the texture of the plane
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureID[1]);
    glUniform1i(textureLocation, 1);
    glUniform1f(repeatLocation, 80.0);

    /*
      we create the transformation matrix by defining the Euler's matrices

      N.B.) the last defined is the first applied

      We need also the matrix for normals transformation, which is the inverse of the transpose of the 3x3 submatrix (upper left) of the modelview. We do not consider the 4th column because we do not need translations for normals.
      An explanation (where XT means the transpose of X, etc):
        "Two column vectors X and Y are perpendicular if and only if XT.Y=0. If We're going to transform X by a matrix M, we need to transform Y by some matrix N so that (M.X)T.(N.Y)=0. Using the identity (A.B)T=BT.AT, this becomes (XT.MT).(N.Y)=0 => XT.(MT.N).Y=0. If MT.N is the identity matrix then this reduces to XT.Y=0. And MT.N is the identity matrix if and only if N=(MT)-1, i.e. N is the inverse of the transpose of M.
    */
    glm::mat4 planeModelMatrix;
    glm::mat3 planeNormalMatrix;
    planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
    planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
    planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
    // we render the plane
    planeModel.Draw(shader);

    // SPHERE
    // we activate the texture of the object
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glUniform1i(textureLocation, 0);
    glUniform1f(repeatLocation, repeat);

    glm::mat4 sphereModelMatrix;
    glm::mat3 sphereNormalMatrix;
    sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(-3.0f, 1.0f, 0.0f));
    sphereModelMatrix = glm::rotate(sphereModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
    sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
    sphereNormalMatrix = glm::inverseTranspose(glm::mat3(view*sphereModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));

    // we render the sphere
    sphereModel.Draw(shader);

    // CUBE
    glm::mat4 cubeModelMatrix;
    glm::mat3 cubeNormalMatrix;
    cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
    cubeModelMatrix = glm::rotate(cubeModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
    cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
    cubeNormalMatrix = glm::inverseTranspose(glm::mat3(view*cubeModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(cubeModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(cubeNormalMatrix));

    // we render the cube
    cubeModel.Draw(shader);

    // BUNNY
    glm::mat4 bunnyModelMatrix;
    glm::mat3 bunnyNormalMatrix;
    bunnyModelMatrix = glm::translate(bunnyModelMatrix, glm::vec3(3.0f, 1.0f, 0.0f));
    bunnyModelMatrix = glm::rotate(bunnyModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
    bunnyModelMatrix = glm::scale(bunnyModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
    bunnyNormalMatrix = glm::inverseTranspose(glm::mat3(view*bunnyModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyNormalMatrix));

    // we render the bunny
    bunnyModel.Draw(shader);

}

//////////////////////////////////////////
// we load the image from disk and we create an OpenGL texture
GLint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);
    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if (channels==4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    // we set how to consider UVs outside [0,1] range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    return textureImage;
}

//////////////////////////////////////////
// we create and compile shaders (code of Shader class is in include/utils/shader_v1.h), and we add them to the list of available shaders
void SetupShaders()
{
    // we create the Shader Programs (code in shader_v1.h)
    Shader shader1("24_phong_tex_shadow.vert", "24a_ggx_tex_shadow_acne.frag");
    shaders.push_back(shader1);
}

/////////////////////////////////////////
// we delete all the Shaders Programs
void DeleteShaders()
{
    for(GLuint i = 0; i < shaders.size(); i++)
        shaders[i].Delete();
}

//////////////////////////////////////////
// we print on console the name of the currently used shader
void PrintCurrentShader(int shader)
{
    std::cout << "Current shader:" << print_available_ShaderPrograms[shader]  << std::endl;

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

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    // pressing a key between 1 and 3, we change the shader applied to the models
    if(key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        // "1" to "3" -> ASCII codes from 49 to 51
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 3
        // we subtract 1 to have indices from 0 to 2 in the shaders list
        current_program = (key-'0'-1);
        PrintCurrentShader(current_program);
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

      // offset of mouse cursor position
      GLfloat xoffset = xpos - lastX;
      GLfloat yoffset = lastY - ypos;

      // the new position will be the previous one for the next frame
      lastX = xpos;
      lastY = ypos;

      // we pass the offset to the Camera class instance in order to update the rendering
      camera.ProcessMouseMovement(xoffset, yoffset);

}
