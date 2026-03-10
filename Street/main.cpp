#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

#include <cmath>
#include <cstdlib>
#include <ctime>


gps::Window myWindow;


glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::vec3 spotDir(0.0f, -1.0f, 0.0f); 
GLint spotDirLoc, cutOffLoc, outerCutOffLoc;

glm::vec3 windDir = glm::normalize(glm::vec3(1.0f, 0.0f, 0.4f)); 
float windStrength = 1.5f;  
bool windOn = true;


glm::vec3 wingL_pos = glm::vec3(0.58326f, 0.38957f, 0.005897f);
glm::vec3 wingR_pos = glm::vec3(0.58058f, 0.38957f, 0.010732f);


glm::vec3 lightDir;
glm::vec3 lightColor;


const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
GLuint shadowMapFBO, depthMapTexture;
gps::Shader depthMapShader;

glm::mat4 lightSpaceTrMatrix;
GLint lightSpaceTrLoc, shadowMapLoc;

glm::vec3 rainCenter(0.0f, 0.0f, 0.0f); 
float rainRadius = 10.0f;
float rainTopY = 22.0f;
float rainBottomY = -2.0f;


glm::vec3 pointLightPosLocal;


glm::vec3 pointLightPosBlender(0.751144f, 0.315282f, -0.501908f);
                
glm::vec3 pointLightColor(2.0f, 1.8f, 1.4f);   


GLint pointLightPosLoc, pointLightColorLoc;
GLint attConstantLoc, attLinearLoc, attQuadraticLoc;




GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;


gps::Camera myCamera(
    glm::vec3(0.0f, 1.0f, 3.0f),
    glm::vec3(0.0f, 1.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.03f;
GLboolean pressedKeys[1024];


glm::vec3 sceneTranslate(0.0f);
glm::vec3 sceneScale(1.0f);
float sceneYaw = 0.0f;
float scenePitch = 0.0f;
float sceneRoll = 0.0f;

float groundY = 0.0f;      
float minCameraY = 1.0f;   

//rain
struct Drop { glm::vec3 pos; float len; float speed; };

std::vector<Drop> rain;
GLuint rainVAO = 0, rainVBO = 0;
gps::Shader rainShader;

glm::vec3 fogCenterCPU;   
float fogRadiusCPU;
float fogHMinCPU, fogHMaxCPU;

float frand01() { return rand() / (float)RAND_MAX; }




bool autoRotate = false;
bool autoTour = false;
float lastTime = 0.0f;

bool editScene = false;


bool night = false;
float sunIntensity = 1.0f;
float sunTarget = 1.0f;
glm::vec3 sunBaseColor = glm::vec3(1.5f); 




gps::Model3D sceneModel;
gps::Model3D skyModel;


gps::Model3D tinkerBody;
gps::Model3D wingL;
gps::Model3D wingR;





float lastX = 512;
float lastY = 384;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float sensitivity = 0.1f;


gps::Shader myBasicShader;

int polygonMode = 0; 
int shadingMode = 1; 
GLint shadingModeLoc;

GLint materialAmbientLoc, materialDiffuseLoc, materialSpecularLoc, materialShininessLoc;






GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)



void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        shadingMode = 1 - shadingMode; 
        myBasicShader.useShaderProgram();
        glUniform1i(shadingModeLoc, shadingMode);
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_M) editScene = !editScene;



    
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) pressedKeys[key] = true;
        else if (action == GLFW_RELEASE) pressedKeys[key] = false;
    }

    
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_R) autoRotate = !autoRotate;
        if (key == GLFW_KEY_T) autoTour = !autoTour;

        
        if (key == GLFW_KEY_1) polygonMode = 0; 
        if (key == GLFW_KEY_2) polygonMode = 1; 
        if (key == GLFW_KEY_3) polygonMode = 2; 
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_N) {
        night = !night;
        sunTarget = night ? 0.15f : 1.0f; 
    }

}



void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw);

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}


void processMovement()
{
    glm::vec3 oldPos = myCamera.getPosition();
    glm::vec3 nextPos = oldPos;

    glm::vec3 forward = glm::normalize(myCamera.getFront());
    glm::vec3 right = glm::normalize(myCamera.getRight());

    glm::vec3 desiredMove(0.0f);
    if (pressedKeys[GLFW_KEY_W]) desiredMove += forward * cameraSpeed;
    if (pressedKeys[GLFW_KEY_S]) desiredMove -= forward * cameraSpeed;
    if (pressedKeys[GLFW_KEY_D]) desiredMove += right * cameraSpeed;
    if (pressedKeys[GLFW_KEY_A]) desiredMove -= right * cameraSpeed;

    nextPos += desiredMove;

    
    if (nextPos.y < groundY + minCameraY)
        nextPos.y = groundY + minCameraY;

    myCamera.setPosition(nextPos);



    float current = (float)glfwGetTime();
    float dt = current - lastTime;
    lastTime = current;

    if (editScene) {
        float tSpeed = 0.05f;
        float rSpeed = 1.0f;
        float sSpeed = 0.01f;

        if (pressedKeys[GLFW_KEY_I]) sceneTranslate.z -= tSpeed;
        if (pressedKeys[GLFW_KEY_K]) sceneTranslate.z += tSpeed;
        if (pressedKeys[GLFW_KEY_J]) sceneTranslate.x -= tSpeed;
        if (pressedKeys[GLFW_KEY_L]) sceneTranslate.x += tSpeed;
        if (pressedKeys[GLFW_KEY_U]) sceneTranslate.y += tSpeed;
        if (pressedKeys[GLFW_KEY_O]) sceneTranslate.y -= tSpeed;

        if (pressedKeys[GLFW_KEY_LEFT])  sceneYaw -= rSpeed;
        if (pressedKeys[GLFW_KEY_RIGHT]) sceneYaw += rSpeed;
        if (pressedKeys[GLFW_KEY_UP])    scenePitch += rSpeed;
        if (pressedKeys[GLFW_KEY_DOWN])  scenePitch -= rSpeed;
        if (pressedKeys[GLFW_KEY_Z])     sceneRoll -= rSpeed;
        if (pressedKeys[GLFW_KEY_X])     sceneRoll += rSpeed;

        if (pressedKeys[GLFW_KEY_KP_ADD] || pressedKeys[GLFW_KEY_EQUAL]) sceneScale += glm::vec3(sSpeed);
        if (pressedKeys[GLFW_KEY_KP_SUBTRACT] || pressedKeys[GLFW_KEY_MINUS]) sceneScale -= glm::vec3(sSpeed);
        sceneScale = glm::max(sceneScale, glm::vec3(0.05f));
    }

    if (autoRotate) sceneYaw += 30.0f * dt;

    if (autoTour) {
        float t = (float)glfwGetTime();
        float R = 5.0f;
        glm::vec3 pos(R * cos(t), 1.5f, R * sin(t));

        myCamera = gps::Camera(pos, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    // tranzitie zi/noapte
    sunIntensity += (sunTarget - sunIntensity) * (1.0f - exp(-dt * 2.5f));
    glm::vec3 sunColor = sunBaseColor * sunIntensity;
    myBasicShader.useShaderProgram();
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(sunColor));

    
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}


glm::mat4 computeLightSpaceTrMatrix()
{
    glm::vec3 lightPos = glm::normalize(lightDir) * 20.0f; 
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

    float near_plane = 0.1f, far_plane = 80.0f;
    glm::mat4 lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);

    return lightProjection * lightView;
}





void initOpenGLWindow()
{
    myWindow.Create(1024, 768, "OpenGL Project Core");
}


void setWindowCallbacks()
{
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}


void initShadowMap()
{
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void initOpenGLState()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_PROGRAM_POINT_SIZE); 
    glPointSize(5.0f);

}



void initModels()
{
    sceneModel.LoadModel("models/MapUntitled.obj");
    skyModel.LoadModel("models/Sky.obj");

    
    tinkerBody.LoadModel("models/TinkerBell_Body.obj"); 
    wingL.LoadModel("models/Wing_L.obj");
    wingR.LoadModel("models/Wing_R.obj");

}






void initShaders()
{
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    depthMapShader.loadShader("shaders/depth.vert", "shaders/depth.frag");
    rainShader.loadShader("shaders/rain.vert", "shaders/rain.frag");


}

void initRain(int N = 6000) {
    rain.resize(N);
    for (auto& d : rain) {
        
        float a = frand01() * 6.2831853f;
        float r = sqrt(frand01()) * rainRadius;
        float x = rainCenter.x + r * cos(a);
        float z = rainCenter.z + r * sin(a);

        float y = rainTopY + frand01() * 3.0f; 
        d.pos = glm::vec3(x, y, z);

        d.len = 0.12f + frand01() * 0.10f;
        d.speed = 1.2f + frand01() * 1.0f;
    }

    glGenVertexArrays(1, &rainVAO);
    glGenBuffers(1, &rainVBO);
    glBindVertexArray(rainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);

    
    glBufferData(GL_ARRAY_BUFFER, rain.size() * 2 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}




void initUniforms()
{
    myBasicShader.useShaderProgram();

    materialAmbientLoc = glGetUniformLocation(myBasicShader.shaderProgram, "materialAmbient");
    materialDiffuseLoc = glGetUniformLocation(myBasicShader.shaderProgram, "materialDiffuse");
    materialSpecularLoc = glGetUniformLocation(myBasicShader.shaderProgram, "materialSpecular");
    materialShininessLoc = glGetUniformLocation(myBasicShader.shaderProgram, "materialShininess");

    
    glUniform3f(materialAmbientLoc, 0.20f, 0.20f, 0.20f); // Ka
    glUniform3f(materialDiffuseLoc, 1.00f, 1.00f, 1.00f); // Kd
    glUniform3f(materialSpecularLoc, 0.60f, 0.60f, 0.60f); // Ks
    glUniform1f(materialShininessLoc, 32.0f);              // Ns

    GLint fogEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled");
    GLint fogColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogColor");
    GLint fogStartLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogStart");
    GLint fogEndLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogEnd");
    GLint fogCenterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogCenter");
    GLint fogRadiusLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogRadius");
    GLint fogHMinLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogHeightMin");
    GLint fogHMaxLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogHeightMax");

    fogCenterCPU = glm::vec3(1.9181f, 0.41626f, -5.2744f);
    fogRadiusCPU = 4.0f;
    fogHMinCPU = 0.0f;
    fogHMaxCPU = 2.0f;


    
    glUniform1i(fogEnabledLoc, 1);

    
    glUniform3f(fogColorLoc, 0.72f, 0.72f, 0.78f);

    
    glUniform1f(fogStartLoc, 2.0f);
    glUniform1f(fogEndLoc, 12.0f);

    
    glm::vec3 fogCenterOpenGL(1.9181f, 0.41626f, -5.2744f);
    glUniform3fv(fogCenterLoc, 1, glm::value_ptr(fogCenterOpenGL));

    
    glUniform1f(fogRadiusLoc, 4.0f);

   
    glUniform1f(fogHMinLoc, 0.0f);
    glUniform1f(fogHMaxLoc, 2.0f);



    shadingModeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "shadingMode");
    glUniform1i(shadingModeLoc, shadingMode);


    spotDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotDir");
    cutOffLoc = glGetUniformLocation(myBasicShader.shaderProgram, "cutOff");
    outerCutOffLoc = glGetUniformLocation(myBasicShader.shaderProgram, "outerCutOff");

    
    glUniform3fv(spotDirLoc, 1, glm::value_ptr(spotDir));
    glUniform1f(cutOffLoc, glm::cos(glm::radians(12.0f)));
    glUniform1f(outerCutOffLoc, glm::cos(glm::radians(20.0f)));


    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width /
        (float)myWindow.getWindowDimensions().height,
        0.1f, 200.0f);

    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    lightColor = glm::vec3(1.5f);
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    
    pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPos");
    pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");
    attConstantLoc = glGetUniformLocation(myBasicShader.shaderProgram, "attConstant");
    attLinearLoc = glGetUniformLocation(myBasicShader.shaderProgram, "attLinear");
    attQuadraticLoc = glGetUniformLocation(myBasicShader.shaderProgram, "attQuadratic");
    
   
    pointLightColor = glm::vec3(6.0f, 5.4f, 4.2f);
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));


     
    pointLightPosLocal = glm::vec3(pointLightPosBlender.x, -pointLightPosBlender.z, -pointLightPosBlender.y);

   
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    glUniform1f(attConstantLoc, 1.0f);
    glUniform1f(attLinearLoc, 0.25f);
    glUniform1f(attQuadraticLoc, 0.80f);

}


void drawRain() {
    
    std::vector<glm::vec3> lines;
    lines.reserve(rain.size() * 2);

    for (auto& d : rain) {
        glm::vec3 p0 = d.pos;
        float t = (float)glfwGetTime();
        float gust = 1.0f + 0.35f * sin(t * 0.8f);
        glm::vec3 wind = windOn ? (windDir * windStrength * gust) : glm::vec3(0.0f);

        
        glm::vec3 fallDir = glm::normalize(glm::vec3(wind.x, -4.0f, wind.z)); 
        glm::vec3 p1 = d.pos + fallDir * d.len;

        
        
            lines.push_back(p0);
            lines.push_back(p1);
        
    }

    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(glm::vec3), lines.data());

    rainShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); 

    glBindVertexArray(rainVAO);
    glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}


void updateRain(float dt);
void drawRain();



void renderScene()
{

    static float last = (float)glfwGetTime();
    float now = (float)glfwGetTime();
    float dt = now - last;
    last = now;

    updateRain(dt);

    // 1) DEPTH PASS
    lightSpaceTrMatrix = computeLightSpaceTrMatrix();

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    depthMapShader.useShaderProgram();
    if (polygonMode == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (polygonMode == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (polygonMode == 2) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    model = glm::mat4(1.0f);
    model = glm::translate(model, sceneTranslate);
    model = glm::rotate(model, glm::radians(sceneYaw), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(scenePitch), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(sceneRoll), glm::vec3(0, 0, 1));
    model = glm::scale(model, sceneScale);

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    glDisable(GL_CULL_FACE);
    sceneModel.Draw(depthMapShader);

    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    // 2) NORMAL PASS
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 5);


   
    glm::mat4 sceneM(1.0f);
    sceneM = glm::translate(sceneM, sceneTranslate);
    sceneM = glm::rotate(sceneM, glm::radians(sceneYaw), glm::vec3(0, 1, 0));
    sceneM = glm::rotate(sceneM, glm::radians(scenePitch), glm::vec3(1, 0, 0));
    sceneM = glm::rotate(sceneM, glm::radians(sceneRoll), glm::vec3(0, 0, 1));
    sceneM = glm::scale(sceneM, sceneScale);

    
    glm::vec3 lampPosWorld = glm::vec3(sceneM * glm::vec4(pointLightPosLocal, 1.0f));
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(lampPosWorld));

    
    glm::mat4 R(1.0f);
    R = glm::rotate(R, glm::radians(sceneYaw), glm::vec3(0, 1, 0));
    R = glm::rotate(R, glm::radians(scenePitch), glm::vec3(1, 0, 0));
    R = glm::rotate(R, glm::radians(sceneRoll), glm::vec3(0, 0, 1));

    glm::vec3 spotDirLocal(0.0f, -1.0f, 0.0f); 
    glm::vec3 spotDirWorld = glm::normalize(glm::mat3(R) * spotDirLocal);
    glUniform3fv(spotDirLoc, 1, glm::value_ptr(spotDirWorld));



    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sceneM));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * sceneM));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "receiveShadows"), 1);

    sceneModel.Draw(myBasicShader);

    

    glDisable(GL_CULL_FACE);
    float t = (float)glfwGetTime();
    float speed = 3.0f;                 
    float amplitude = glm::radians(3.0f); 
    float flap = sin(t * speed) * amplitude;

    glm::mat4 ML = sceneM;
    ML = glm::translate(ML, wingL_pos);
    ML = glm::rotate(ML, flap, glm::vec3(0, 1, 0));
    ML = glm::translate(ML, -wingL_pos);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ML));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * ML));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    wingL.Draw(myBasicShader);

    

    glm::mat4 MR = sceneM;
    MR = glm::translate(MR, wingR_pos);
    MR = glm::rotate(MR, -flap, glm::vec3(0, 1, 0));
    MR = glm::translate(MR, -wingR_pos);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(MR));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * MR));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    wingR.Draw(myBasicShader);

    drawRain();

   
    myBasicShader.useShaderProgram();

    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


    glEnable(GL_CULL_FACE);

    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "receiveShadows"), 0);




    
    glDepthMask(GL_FALSE);
    glm::mat4 skyM = sceneM;   
 
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(skyM));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * skyM));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    skyModel.Draw(myBasicShader);
    glDepthMask(GL_TRUE);


}

bool insideFogZone(const glm::vec3& p) {
   
    glm::vec3 d = p - fogCenterCPU;
    if (glm::dot(d, d) > fogRadiusCPU * fogRadiusCPU) return false;
    if (p.y < fogHMinCPU || p.y > fogHMaxCPU + 2.0f) return false; 
    return true;
}

void updateRain(float dt) {
    float t = (float)glfwGetTime();

    
    float gust = 1.0f + 0.35f * sin(t * 0.8f);

    glm::vec3 wind = windOn ? (windDir * windStrength * gust) : glm::vec3(0.0f);

    for (auto& d : rain) {
        
        d.pos.y -= d.speed * dt;

        
        d.pos.x += wind.x * dt;
        d.pos.z += wind.z * dt;

       
        glm::vec2 dxz(d.pos.x - rainCenter.x, d.pos.z - rainCenter.z);
        if (d.pos.y < rainBottomY || glm::dot(dxz, dxz) > rainRadius * rainRadius) {
            float a = frand01() * 6.2831853f;
            float r = sqrt(frand01()) * rainRadius;
            d.pos.x = rainCenter.x + r * cos(a);
            d.pos.z = rainCenter.z + r * sin(a);
            d.pos.y = rainTopY + frand01() * 0.5f;
        }
    }
}









// cleanup
void cleanup()
{
    myWindow.Delete();
}


int main(int argc, const char* argv[])
{
    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initShadowMap();
    initModels();
    //initColliders();

    initShaders();
    initUniforms();
    srand((unsigned)time(nullptr));
    //rainRadius = domeRadius - 1.0f;
    //rainTopY = domeCenter.y + domeRadius - 2.0f;
    rainRadius = 17.0f;
    rainTopY = 17.0f;
    rainBottomY = -10.0f;


    initRain();

    setWindowCallbacks();

    // 🔥 FPS MODE (lock mouse)
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glCheckError();

    while (!glfwWindowShouldClose(myWindow.getWindow()))
    {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();
    return EXIT_SUCCESS;
}
