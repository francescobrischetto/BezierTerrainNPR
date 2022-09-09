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
void replaceBy(TerrainModel*& foo, TerrainModel* bar);

// Predefined Styles
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
// position of a pointlight
glm::vec3 lightPosition = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.5 * terrainDimension);
glm::vec3 cameraPosition = glm::vec3(0.0f, 1.3f * terrainDimension,  1.0f * terrainDimension);
GLfloat cameraSpeed = terrainDimension * 0.15f; 
// we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
Camera camera(cameraPosition , cameraSpeed, GL_TRUE);
glm::vec3 cameraOrientation = camera.Front;
glm::vec3 cameraInitialPosition = cameraPosition;
glm::vec3 cameraInitialOrientation = cameraOrientation;

//Terrain Generator Parameters
GLuint numPatches = 100;
GLuint generationSeed = 45;
GLuint consideredOctaves = 8;
GLfloat consideredFrequency = 3.0;

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
// Contour and Suggestive Contour limit parameters 
GLfloat contourLimit = 0.1;
//Directional derivative of Radial Curvature Limit
GLfloat directionalDerivativeLimit = 12;

//Stores the Model to be displayed and changed dynamically during run-time
TerrainModel terrainModel;
bool showingTerrain = true;

//Styles we can switch in UI
typedef void (*PreloadedStyleFunction) ();
const PreloadedStyleFunction Styles[] = 
    {
        BlackAndWhiteStyle, 
        ReddishStyle, 
        GrassStyle
    };
const string StylesPrettyNames[] = 
    {
        "Black and White Style", 
        "Reddish Style", 
        "Grass Style"
    };    
GLuint styleIndex = 0;

// UI Tabs manager
int switchTabs = 0;

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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Non Photorealistic Rendering - Thesis", nullptr, nullptr);
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
    Styles[styleIndex]();

    /////////////////// MODELS AND TEXTURES ///////////////////////
    Model cubeModel("../../models/cube.obj");
    terrainModel = TerrainModel(numPatches, generationSeed, consideredOctaves, consideredFrequency);
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
        if(showingTerrain){
            terrainModelMatrix = glm::mat4(1.0f);
            terrainNormalMatrix = glm::mat3(1.0f);
            terrainModelMatrix = glm::translate(terrainModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
            terrainModelMatrix = glm::rotate(terrainModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
            terrainModelMatrix = glm::scale(terrainModelMatrix, glm::vec3(terrainDimension));
        }else{
            terrainModelMatrix = glm::mat4(1.0f);
            terrainModelMatrix = glm::rotate(terrainModelMatrix, glm::radians((GLfloat)90.0), glm::vec3(1.0f, 0.0f, 0.0f));
            terrainModelMatrix = glm::rotate(terrainModelMatrix, glm::radians((GLfloat)180.0), glm::vec3(0.0f, 1.0f, 0.0f));
            terrainModelMatrix = glm::scale(terrainModelMatrix, glm::vec3(terrainDimension/4.0f));
        }
        terrainNormalMatrix = glm::inverseTranspose(glm::mat3(view*terrainModelMatrix));

        // Uniforms passed to the shaders
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(terrainModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(illumination_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(terrainNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "pointLightWorldPosition"), 1, glm::value_ptr(lightPosition));
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "cameraWorldPosition"), 1, glm::value_ptr(camera.Position));
        glUniform1f(glGetUniformLocation(illumination_shader.Program, "contourLimit"), contourLimit);
        glUniform1f(glGetUniformLocation(illumination_shader.Program, "directionalDerivativeLimit"), directionalDerivativeLimit);
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "warmColor"), 1, warmColor);
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "coldColor"), 1, coldColor);
        glUniform3fv(glGetUniformLocation(illumination_shader.Program, "strokeColor"), 1, strokeColor);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "shininessFactor"), shininessFactor);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "celShadingSize"), celShadingSize);
        glUniform2fv(glGetUniformLocation(illumination_shader.Program, "viewportResolution"), 1, viewportResolution );
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "shadingType"), shadingType);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "enableContours"), enableContours);
        glUniform1i(glGetUniformLocation(illumination_shader.Program, "enableSuggestiveContours"), enableSuggestiveContours);
        
        // Draw call for the terrain
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
        // Draw call for the background skybox
        cubeModel.Draw();
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        
        // Render UI Window
        ImGui::Begin("Project Settings",0, ImGuiWindowFlags_AlwaysAutoResize);
         // UI scaling
        ImGui::SetWindowFontScale( 1.4f );
        
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
        if (ImGui::Button("Application", ImVec2(110.0f, 50.0f)))         switchTabs = 0;
        ImGui::SameLine();
		if (ImGui::Button("Style", ImVec2(110.0f, 50.0f)))               switchTabs = 1;
        ImGui::SameLine();
        if (ImGui::Button("Terrain", ImVec2(110.0f, 50.0f)))             switchTabs = 2;
        ImGui::SameLine();
        if (ImGui::Button("Shading", ImVec2(110.0f, 50.0f)))             switchTabs = 3;
        ImGui::SameLine();
        if (ImGui::Button("Camera", ImVec2(110.0f, 50.0f)))              switchTabs = 4;
        ImGui::SameLine();
        if (ImGui::Button("Contours", ImVec2(110.0f, 50.0f)))            switchTabs = 5;
        ImGui::Separator();
        switch (switchTabs) {
        case 0:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Application Info:");
            ImGui::PopStyleColor();
            ImGui::NewLine();
            ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
            ImGui::NewLine();
            
            break;
        case 1:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Style Settings:");
            ImGui::PopStyleColor();
            ImGui::NewLine();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::Text(StylesPrettyNames[styleIndex].c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();

            if( ImGui::Button( "Change Style" ) )
            {
                styleIndex++;
                styleIndex = styleIndex % std::size(Styles);
                Styles[styleIndex]();
                showingTerrain = true;
                terrainModel = TerrainModel(numPatches, generationSeed, consideredOctaves, consideredFrequency);
            }
            ImGui::NewLine();
            ImGui::Separator();
            break;
        case 2:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Terrain Settings:");
            ImGui::PopStyleColor();
            ImGui::NewLine();
            ImGui::SliderInt("Patches", (int*)&numPatches, 80, 120 );
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Number of Bezier Patches to use in x and y direction to generate the terrain.");
            ImGui::InputInt("Seed", (int*)&generationSeed);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Overall seed of the terrain generation (used for noise function).");
            ImGui::SliderInt("Octaves", (int*)&consideredOctaves, 1, 16);
            ImGui::SliderFloat("Frequency",&consideredFrequency, 1.0f, 12.0f);
            ImGui::NewLine();
            if( ImGui::Button( "Regenerate terrain" ) )
            {
                showingTerrain = true;
                camera.Position = cameraPosition;
                // Reloading the mesh
                terrainModel = TerrainModel(numPatches, generationSeed, consideredOctaves, consideredFrequency);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Generate the terrain using above settings.");
            ImGui::SameLine();
            if( ImGui::Button( "Load Teapot" ) )
            {
                showingTerrain = false;
                Styles[0]();
                camera.Position = glm::vec3(0,350,770);
                // Loading teapot from disk (expressed with bezier surfaces)
                terrainModel = TerrainModel("../../models/teapot.bez");
                
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Load Teapot model expressed with bezier surfaces.");
            ImGui::NewLine();
            ImGui::Separator();
            break;
        case 3:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Shading Settings:");
            ImGui::PopStyleColor();
            ImGui::NewLine();
            ImGui::RadioButton("Cel Shading", (int*)&shadingType, 0); ImGui::SameLine();
            ImGui::RadioButton("Gooch Shading", (int*)&shadingType, 1);ImGui::SameLine();
            ImGui::RadioButton("Uniform Color", (int*)&shadingType, 2);
            ImGui::NewLine();
            ImGui::ColorEdit3("Warm Color", warmColor);
            ImGui::ColorEdit3("Cold Color", coldColor);
            ImGui::ColorEdit3("Background Color", backgroundColor);
            ImGui::NewLine();
            ImGui::SliderInt("Cel Size", (int*)&celShadingSize, 1, 20);
            ImGui::SliderInt("Shininess Factor", (int*)&shininessFactor, 1, 50);
            ImGui::NewLine();
            ImGui::Text( "Point Light Current Position x:%f y:%f z:%f", lightPosition.x, lightPosition.y, lightPosition.z );
            ImGui::SliderFloat3("Point Light Position",(float*)&lightPosition, 0.0f, terrainDimension);
            ImGui::NewLine();
            ImGui::Separator();
            break;
        case 4:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Camera Settings:");
            ImGui::PopStyleColor();
            ImGui::NewLine();
            ImGui::Text( "Camera Current Position x:%f y:%f z:%f", camera.Position.x, camera.Position.y, camera.Position.z );
            ImGui::SliderFloat3("Camera Position",(float*)&cameraPosition, 0.0f, terrainDimension);
            ImGui::SliderFloat3("Camera Orientation",(float*)&cameraOrientation, -1.0f, 1.0f);
            if( ImGui::Button( "Move Camera" ) )
            {
                camera.Position = cameraPosition;
                camera.Front = cameraOrientation;
            }
            ImGui::NewLine();
            ImGui::Separator();
            break;
        case 5:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Contours Settings:");
            ImGui::PopStyleColor();
            ImGui::NewLine();
            ImGui::ColorEdit3("Stroke Color", strokeColor);
            ImGui::Checkbox("Enable Contours", &enableContours);
            ImGui::SliderFloat("Contour Limit",&contourLimit, 0.01f, 0.5f);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Parameter that enlarge or shrinks the contours.");
            ImGui::Checkbox("Enable Suggestive Contours", &enableSuggestiveContours);
            ImGui::SliderFloat("Directional Derivative Limit",&directionalDerivativeLimit, 3, 20);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Parameter that increases or decreases the regions to be considered as suggestive contours.");
            ImGui::NewLine();
            ImGui::Separator();
            break;
        }     
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
    consideredFrequency = 4.0;
    shadingType = 0;
    warmColor[0] = 1.0; warmColor[1] = 0.0; warmColor[2] = 0.0;
    coldColor[0] = 0.39; coldColor[1] = 0.91; coldColor[2] = 1.0;
    backgroundColor[0] = 1; backgroundColor[1] = 0.4; backgroundColor[2] = 0.2;
    celShadingSize = 9;
    shininessFactor = 22;
    strokeColor[0] = 0.0; strokeColor[1] = 0.0; strokeColor[2] = 0.0;
    enableContours = true;
    enableSuggestiveContours = true;
    contourLimit = 0.3;
    directionalDerivativeLimit = 12;
    lightPosition = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.5 * terrainDimension);
    camera.Position = cameraInitialPosition;
    camera.Front = cameraInitialOrientation;
}

void BlackAndWhiteStyle(){
    numPatches = 100;
    generationSeed = 45;
    consideredOctaves = 8;
    consideredFrequency = 3.0;
    shadingType = 0;
    warmColor[0] = 1.0; warmColor[1] = 1.0; warmColor[2] = 1.0;
    coldColor[0] = 0.0; coldColor[1] = 0.0; coldColor[2] = 0.0;
    backgroundColor[0] = 0.6; backgroundColor[1] = 0.6; backgroundColor[2] = 0.6;
    celShadingSize = 20;
    shininessFactor = 16;
    strokeColor[0] = 0.0; strokeColor[1] = 0.0; strokeColor[2] = 0.0;
    enableContours = true;
    enableSuggestiveContours = true;
    contourLimit = 0.1;
    directionalDerivativeLimit = 12;
    lightPosition = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.5 * terrainDimension);
    camera.Position = cameraInitialPosition;
    camera.Front = cameraInitialOrientation;
}

void GrassStyle(){
    numPatches = 100;
    generationSeed = 8967;
    consideredOctaves = 6;
    consideredFrequency = 2.0;
    shadingType = 0;
    warmColor[0] = 0.40; warmColor[1] = 0.91; warmColor[2] = 0.03;
    coldColor[0] = 0.05; coldColor[1] = 0.37; coldColor[2] = 0.02;
    backgroundColor[0] = 0.0; backgroundColor[1] = 0.42; backgroundColor[2] = 0.45;
    celShadingSize = 20;
    shininessFactor = 12;
    strokeColor[0] = 0.19; strokeColor[1] = 0.17; strokeColor[2] = 0.16;
    enableContours = true;
    enableSuggestiveContours = true;
    contourLimit = 0.05;
    directionalDerivativeLimit = 8;
    lightPosition = glm::vec3(0.5 * terrainDimension, 2 * terrainDimension, 0.5 * terrainDimension);
    camera.Position = cameraInitialPosition;
    camera.Front = cameraInitialOrientation;
}

void replaceBy(TerrainModel*& foo, TerrainModel* bar)
{
  delete foo;
  foo = bar;
}