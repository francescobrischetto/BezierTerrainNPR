/*
//TODO: Add Description 

author: Francesco Brischetto mat. 958022

Thesis - a.a. 2021/2022
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

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>
// STB_IMAGE used for textures
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders and to load models
#include <utils/shader.h>
#include <utils/model.h>
#include <utils/terrain_model.h>
#include <utils/camera.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// dimensions of application's window
GLuint screenWidth = 1366 , screenHeight = 768 ;
GLfloat viewportResolution[] = {(GLfloat)screenWidth, (GLfloat)screenHeight};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_camera_movements();
void LoadTextureCubeSide(string path, string side_image, GLuint side_name);
GLint LoadTextureCube(string path);
GLint LoadTexture(const char* path);

//Styles
void ReddishStyle();
void BlackAndWhiteStyle();
void DirtStyle();
void GrassStyle();


// we initialize an array of booleans for each keybord key
bool keys[1024];
// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;
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
GLboolean spinning = GL_FALSE;
// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;
// near and far planes distance
GLfloat near = 0.1f;
GLfloat far = 10000.0f;
// clear color
GLfloat clearColor[] = {0.26f, 0.46f, 0.98f};

// dimensions of terrain and position of the camera
GLfloat terrainDimension = 500.0f;
// we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
Camera camera(glm::vec3(0.0f, 1.3f * terrainDimension,  1.0f * terrainDimension), terrainDimension * 0.15f, GL_TRUE);
// position of a pointlight
glm::vec3 lightPos0 = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.5 * terrainDimension);

//Terrain Generator Parameters
GLuint numPatches = 100;
GLuint generationSeed = 45;
GLuint consideredOctaves = 8;
GLfloat consideredFrequency = 3.0;
GLuint pointsToEvalXY = 7;
GLfloat displacementFactor = 0.01;

// Uniforms to pass to shaders
//User UI parameters
GLfloat warmColor[] = {1.0,1.0,1.0};
GLfloat coldColor[] = {0.0,0.0,0.0};
GLfloat strokeColor[] = {0.0,0.0,0.0};
GLfloat backgroundColor[] = {0.6,0.6,0.6};
GLuint shininessFactor = 30;
GLuint celShadingSize = 15;
// 0 - Cel Shading , 1 - Gooch Shading
GLuint shadingType = 0;
bool enableContours = true;
bool enableSuggestiveContours = true;
bool enableHatching = true;
bool enableGrassTexture = true;
// Contour and Suggestive Contour limit parameters 
GLfloat contourLimit = 1.5;
GLfloat suggestivecLimit = 1.5;
//Directional derivative of Radial Curvature Limit
GLfloat directionalDerivativeLimit = 0.005;
const string hatchTypes[] = {"standardLines.png", "dots.png", "wriggles.png"};
GLuint hatchIndex = 0;
const string grassTypes[] = {"grass1.jpg", "grass2.jpg", "dirt.jpg", "sand.jpg"};
GLuint grassIndex = 0;

//Styles we can switch in UI
typedef void (*PreloadedStyleFunction) ();
const PreloadedStyleFunction Styles[] = 
    {
        BlackAndWhiteStyle, 
        ReddishStyle, 
        DirtStyle,
        GrassStyle
    };
GLuint styleIndex = 0;



/////////////////// MAIN function ///////////////////////
int main()
{
  // Initialization of OpenGL context using GLFW
  glfwInit();
  // We set OpenGL specifications required for this application
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // we set if the window is resizable
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "MyProject", nullptr, nullptr);
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
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    // we enable Z test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glPatchParameteri(GL_PATCH_VERTICES, 16);
    //the "clear" color for the frame buffer
    glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);

    
    /////////////////// SHADER PROGRAMS ///////////////////////
    Shader skybox_shader = Shader("Shaders/skybox_vert.glsl", "Shaders/skybox_frag.glsl");
    //Shader illumination_shader = Shader("Shaders/basic.vert", "Shaders/basic.frag");
    Shader illumination_shader = Shader("Shaders/terrainBezierTessellation_vert.glsl", "Shaders/terrainBezierTessellation_frag.glsl",nullptr,"Shaders/terrainBezierTessellation_tcs.glsl","Shaders/terrainBezierTessellation_tes.glsl");

    //We apply the first style
    //Styles[styleIndex]();

    /////////////////// MODELS AND TEXTURES ///////////////////////
    Model cubeModel("../../models/cube.obj");
    TerrainModel terrainModel(numPatches, generationSeed, consideredOctaves, consideredFrequency);
    //TerrainModel terrainModel("../../models/teapot.bez");
    //Model terrainModel("../../models/stanford-bunny.obj");
    string textureToLoad = "./Textures/Hatches/";
    vector<GLuint> hatchTextures;
    for(int i=0; i < std::size(hatchTypes); i++){
        string loadTexture = textureToLoad + hatchTypes[i];
        hatchTextures.push_back(LoadTexture(loadTexture.c_str()));
    }
    string grassesToLoad = "./Textures/Grasses/";
    vector<GLuint> grassTextures;
    for(int i=0; i < std::size(grassTypes); i++){
        string loadTexture = grassesToLoad + grassTypes[i];
        grassTextures.push_back(LoadTexture(loadTexture.c_str()));
    }
    GLuint skyboxTexture = LoadTextureCube("Textures/Skyboxes/nprSky/"); 
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, near, far);
    // View matrix: the camera moves, so we just set to indentity now
    glm::mat4 view = glm::mat4(1.0f);
    // Model and Normal transformation matrices for the objects in the scene
    glm::mat4 terrainModelMatrix = glm::mat4(1.0f);
    glm::mat3 terrainNormalMatrix = glm::mat3(1.0f);

    /////////////////// IMGUI SETUP ///////////////////////
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 410");


    /////////////////// RENDERING LOOP ///////////////////////
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
        view = camera.GetViewMatrix();
        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // GUI Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // if animated rotation is activated, than we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        

        /////////////////// RENDERING OF THE OBJECTS IN THE SCENE ///////////////////////
        illumination_shader.Use();
        // Terrain Rendering
        terrainModelMatrix = glm::mat4(1.0f);
        terrainNormalMatrix = glm::mat3(1.0f);
        terrainModelMatrix = glm::translate(terrainModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        terrainModelMatrix = glm::rotate(terrainModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        //For Teapot
        //terrainModelMatrix = glm::rotate(terrainModelMatrix, glm::radians((GLfloat)90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        //terrainModelMatrix = glm::rotate(terrainModelMatrix, glm::radians((GLfloat)180.0), glm::vec3(0.0f, 1.0f, 0.0f));
        //terrainModelMatrix = glm::scale(terrainModelMatrix, glm::vec3(terrainDimension/2.0f));
        terrainModelMatrix = glm::scale(terrainModelMatrix, glm::vec3(terrainDimension));
        //terrainModelMatrix = glm::scale(terrainModelMatrix, glm::vec3(0.4f, 0.4f, 0.4f));
        terrainNormalMatrix = glm::inverseTranspose(glm::mat3(view*terrainModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(terrainModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(illumination_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(terrainNormalMatrix));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hatchTextures[hatchIndex]);
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "pointLightWorldPosition"), 1, glm::value_ptr(lightPos0));
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "cameraWorldPosition"), 1, glm::value_ptr(camera.Position));
        glUniform1f(glGetUniformLocation(illumination_shader.Program, "contourLimit"), contourLimit);
        glUniform1f(glGetUniformLocation(illumination_shader.Program, "suggestivecLimit"), suggestivecLimit);
        glUniform1f(glGetUniformLocation(illumination_shader.Program, "directionalDerivativeLimit"), directionalDerivativeLimit);
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "warmColor"), 1, warmColor);
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "coldColor"), 1, coldColor);
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "strokeColor"), 1, strokeColor);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "shininessFactor"), shininessFactor);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "celShadingSize"), celShadingSize);
        glUniform2fv(glGetUniformLocation(illumination_shader.Program, "viewportResolution"), 1, viewportResolution );
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "hatchTexture"), 0);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "shadingType"), shadingType);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "enableContours"), enableContours);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "enableSuggestiveContours"), enableSuggestiveContours);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "enableHatching"), enableHatching);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "enableGrassTexture"), enableGrassTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grassTextures[grassIndex]);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "grassTexture"), 1);
        
        terrainModel.Draw();
        
        // Skybox Rendering
        // we use the cube to attach the 6 textures of the environment map.
        // we render it after all the other objects, in order to avoid the depth tests as much as possible.
        // we will set, in the vertex shader for the skybox, all the values to the maximum depth. 
        // Thus, the environment map is rendered only where there are no other objects in the image (so, only on the background).
        // Thus, we set the depth test to GL_LEQUAL, in order to let the fragments of the background pass the depth test
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        skybox_shader.Use();
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
         // we pass projection and view matrices to the Shader Program of the skybox
        glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        // to have the background fixed during camera movements, we have to remove the translations from the view matrix
        // thus, we consider only the top-left submatrix, and we create a new 4x4 matrix
        view = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skybox_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(skybox_shader.Program, "backgroundColor"), 1, backgroundColor);
        glUniform1i(glGetUniformLocation(skybox_shader.Program, "skyboxCube"), 2);
        cubeModel.Draw();
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        
        // Render UI Window
        ImGui::Begin("Project Settings");
         // UI scaling
        ImGui::SetWindowFontScale( 1.2f );
        ImGui::NewLine();
        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("Load predefined Styles:");
        ImGui::SameLine();
        if( ImGui::Button( "Change Style" ) )
        {
            styleIndex++;
            styleIndex = styleIndex % std::size(Styles);
            Styles[styleIndex]();
            Model app(numPatches, generationSeed, consideredOctaves, consideredFrequency, pointsToEvalXY, pointsToEvalXY, displacementFactor);
            terrainModel.meshes.clear();
            for(int i=0; i<size(app.meshes); i++){
                    //terrainModel.meshes.push_back(Mesh(app.meshes[i].vertices,app.meshes[i].indices));
            }
        }
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("Terrain Settings:");
        ImGui::NewLine();
        ImGui::SliderInt("Squared Number of Bezier Patches", (int*)&numPatches, 15, 50 );
        ImGui::InputInt("Generation Seed", (int*)&generationSeed);
        ImGui::SliderInt("Octaves", (int*)&consideredOctaves, 1, 16);
        ImGui::SliderFloat("Frequency",&consideredFrequency, 1.0f, 30.0f);
        ImGui::SliderInt("Points evaluated (x-wise and y-wise)", (int*)&pointsToEvalXY, 4, 16);
        ImGui::SliderFloat("Displacement Factor",&displacementFactor, 0.01f, 0.03f);
        ImGui::NewLine();
        if( ImGui::Button( "Regenerate terrain with new parameters" ) )
        {
            Model app(numPatches, generationSeed, consideredOctaves, consideredFrequency, pointsToEvalXY, pointsToEvalXY, displacementFactor);
            terrainModel.meshes.clear();
            for(int i=0; i<size(app.meshes); i++){
                    //terrainModel.meshes.push_back(Mesh(app.meshes[i].vertices,app.meshes[i].indices));
            }
        }
        if( ImGui::Button( "Export terrain as OBJ" ) )
        {
            //Export OBJ
        } ImGui::SameLine();
        if( ImGui::Button( "Export Terrain as BEZ" ) )
        {
            //Export BEZ
        }
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("Shading Settings:");
        ImGui::NewLine();
        ImGui::RadioButton("Cel Shading", (int*)&shadingType, 0); ImGui::SameLine();
        ImGui::RadioButton("Gooch Shading", (int*)&shadingType, 1);
        ImGui::NewLine();
        ImGui::ColorEdit3("Warm Color", warmColor);
        ImGui::ColorEdit3("Cold Color", coldColor);
        ImGui::ColorEdit3("Background Color", backgroundColor);
        ImGui::NewLine();
        ImGui::SliderInt("Cel Size", (int*)&celShadingSize, 1, 20);
        ImGui::SliderInt("Shininess Factor", (int*)&shininessFactor, 1, 50);
        ImGui::SliderFloat3("Point Light Position",(float*)&lightPos0, 0.0f, terrainDimension);
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("Contours, Hatches and Textures Settings:");
        ImGui::ColorEdit3("Stroke Color", strokeColor);
        ImGui::Checkbox("Enable Contours", &enableContours);
        ImGui::SliderFloat("Contour Limit",&contourLimit, 1.0f, 10.0f);
        ImGui::Checkbox("Enable Suggestive Contours", &enableSuggestiveContours);
        ImGui::SliderFloat("Suggestive Contour Limit",&suggestivecLimit, 1.0f, 10.0f);;
        ImGui::SliderFloat("Directional Derivative Limit",&directionalDerivativeLimit, 0.001f, 0.300f);
        ImGui::Checkbox("Enable Hatching", &enableHatching);
        ImGui::SliderInt("Hatch Type", (int*)&hatchIndex, 0, std::size(hatchTypes)-1);
        ImGui::Checkbox("Enable Stylized Grass Texture", &enableGrassTexture);
        ImGui::SliderInt("Grass Type", (int*)&grassIndex, 0, std::size(grassTypes)-1);
        ImGui::NewLine();
        //ImGui::RadioButton()
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        
        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // Destroy UI Objects
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Program
    illumination_shader.Delete();
    skybox_shader.Delete();
    // we close and delete the created context
    glfwTerminate();
    return 0;
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
    if(keys[GLFW_KEY_UP])
        camera.ProcessKeyboard(UPDIR, deltaTime);
    if(keys[GLFW_KEY_DOWN])
        camera.ProcessKeyboard(DOWNDIR, deltaTime);
}

//////////////////////////////////////////
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if ( glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) ==  GLFW_PRESS) {
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

}

///////////////////////////////////////////
// load one side of the cubemap, passing the name of the file and the side of the corresponding OpenGL cubemap
void LoadTextureCubeSide(string path, string side_image, GLuint side_name)
{
    int w, h;
    unsigned char* image;
    string fullname;

    // full name and path of the side of the cubemap
    fullname = path + side_image;
    // we load the image file
    image = stbi_load(fullname.c_str(), &w, &h, 0, STBI_rgb);
    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;
    // we set the image file as one of the side of the cubemap (passed as a parameter)
    glTexImage2D(side_name, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);
}


//////////////////////////////////////////
// we load the 6 images from disk and we create an OpenGL cube map
GLint LoadTextureCube(string path)
{
    GLuint textureImage;

    // we create and activate the OpenGL cubemap texture
    glGenTextures(1, &textureImage);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureImage);

    // we load and set the 6 images corresponding to the 6 views of the cubemap
    // we use as convention that the names of the 6 images are "posx, negx, posy, negy, posz, negz", placed at the path passed as parameter
    // we load the images individually and we assign them to the correct sides of the cube map
    LoadTextureCubeSide(path, std::string("posx.jpg"), GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    LoadTextureCubeSide(path, std::string("negx.jpg"), GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    LoadTextureCubeSide(path, std::string("posy.jpg"), GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    LoadTextureCubeSide(path, std::string("negy.jpg"), GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    LoadTextureCubeSide(path, std::string("posz.jpg"), GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
    LoadTextureCubeSide(path, std::string("negz.jpg"), GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // we set how to consider the texture coordinates outside [0,1] range
    // in this case we have a cube map, so
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureImage;

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

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureImage;

}

void ReddishStyle(){
    
    numPatches = 100;
    generationSeed = 5400;
    consideredOctaves = 8;
    consideredFrequency = 18.0;
    pointsToEvalXY = 7;
    displacementFactor = 0.01;
    shadingType = 0;
    warmColor[0] = 1.0; warmColor[1] = 0.0; warmColor[2] = 0.0;
    coldColor[0] = 0.39; coldColor[1] = 0.91; coldColor[2] = 1.0;
    backgroundColor[0] = 0.78; backgroundColor[1] = 0.44; backgroundColor[2] = 0.0;
    celShadingSize = 9;
    shininessFactor = 22;
    lightPos0 = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.3 * terrainDimension);
    strokeColor[0] = 0.0; strokeColor[1] = 0.0; strokeColor[2] = 0.0;
    enableContours = true;
    enableSuggestiveContours = true;
    enableHatching = true;
    enableGrassTexture = true;
    hatchIndex = 0;
    grassIndex = 3;
    contourLimit = 2.1;
    suggestivecLimit = 3.1;
    directionalDerivativeLimit = 0.02;
}

void BlackAndWhiteStyle(){
    numPatches = 100;
    generationSeed = 786;
    consideredOctaves = 8;
    consideredFrequency = 10.0;
    pointsToEvalXY = 6;
    displacementFactor = 0.01;
    shadingType = 0;
    warmColor[0] = 1.0; warmColor[1] = 1.0; warmColor[2] = 1.0;
    coldColor[0] = 0.0; coldColor[1] = 0.0; coldColor[2] = 0.0;
    backgroundColor[0] = 0.6; backgroundColor[1] = 0.6; backgroundColor[2] = 0.6;
    celShadingSize = 20;
    shininessFactor = 16;
    lightPos0 = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.3 * terrainDimension);
    strokeColor[0] = 0.0; strokeColor[1] = 0.0; strokeColor[2] = 0.0;
    enableContours = true;
    enableSuggestiveContours = true;
    enableHatching = false;
    enableGrassTexture = true;
    hatchIndex = 0;
    grassIndex = 3;
    contourLimit = 4.3;
    suggestivecLimit = 5.2;
    directionalDerivativeLimit = 0.1;
}
void DirtStyle(){
    numPatches = 100;
    generationSeed = 786;
    consideredOctaves = 8;
    consideredFrequency = 10.0;
    pointsToEvalXY = 6;
    displacementFactor = 0.01;
    shadingType = 1;
    warmColor[0] = 0.91; warmColor[1] = 0.69; warmColor[2] = 0.03;
    coldColor[0] = 0.37; coldColor[1] = 0.28; coldColor[2] = 0.02;
    backgroundColor[0] = 0.0; backgroundColor[1] = 0.71; backgroundColor[2] = 0.95;
    celShadingSize = 20;
    shininessFactor = 3;
    lightPos0 = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.5 * terrainDimension);
    strokeColor[0] = 0.35; strokeColor[1] = 0.24; strokeColor[2] = 0.07;
    enableContours = true;
    enableSuggestiveContours = false;
    enableHatching = true;
    enableGrassTexture = true;
    hatchIndex = 0;
    grassIndex = 1;
    contourLimit = 6.5;
    suggestivecLimit = 5.2;
    directionalDerivativeLimit = 0.1;
}

void GrassStyle(){
    numPatches = 100;
    generationSeed = 8967;
    consideredOctaves = 6;
    consideredFrequency = 20.0;
    pointsToEvalXY = 7;
    displacementFactor = 0.01;
    shadingType = 0;
    warmColor[0] = 0.40; warmColor[1] = 0.91; warmColor[2] = 0.03;
    coldColor[0] = 0.05; coldColor[1] = 0.37; coldColor[2] = 0.02;
    backgroundColor[0] = 0.0; backgroundColor[1] = 0.42; backgroundColor[2] = 0.45;
    celShadingSize = 20;
    shininessFactor = 12;
    lightPos0 = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.5 * terrainDimension);
    strokeColor[0] = 0.19; strokeColor[1] = 0.17; strokeColor[2] = 0.16;
    enableContours = true;
    enableSuggestiveContours = true;
    enableHatching = true;
    enableGrassTexture = true;
    hatchIndex = 2;
    grassIndex = 0;
    contourLimit = 6;
    suggestivecLimit = 2.6;
    directionalDerivativeLimit = 0.08;
}