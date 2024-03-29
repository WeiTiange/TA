// ----------包含文件----------
// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// 数学
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// 图片处理
#include <LearnOpenGL/stb_image.h>
// Shader类
#include <LearnOpenGL/Shader_s.h>
// 摄像机类
#include <LearnOpenGL/Camera.h>
// 模型类
#include <LearnOpenGL/Model.h>
// C++
#include <iostream>
#include <random>
// ---------------------------


// ----------命名空间----------
using namespace std;
// ---------------------------


// ----------创建函数----------
// 调整窗口大小回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// 处理用户输入
void processInput(GLFWwindow *window);
// 用户输入(按一次只触发一次)
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
// 计算摄像机俯仰角和偏航角
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
// 滚轮调整FOV来进行缩放
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
// 纹理加载
unsigned int loadTexture(char const *path, bool gammaCorrection);
// 设置FBO纹理颜色缓冲附件
void FBOTextureAttach(GLuint texAttach, GLint internalType, GLenum format, int number);
// 检查FBO完整性
void CheckFBO(string FBOname);
// 线性插值
float lerp(float a, float b, float f);
// 渲染
void renderCube();
void renderQuad();
// ---------------------------

// ----------全局参数----------
// 窗口大小
unsigned int scrWidth = 1920;
unsigned int scrHeight = 1080;
// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
// DeltaTime
float deltaTime = 0.0f;     // 渲染当前帧与上一帧的时间差
float lastTime = 0.0f;      // 完成渲染上一帧的时间
// 鼠标参数
bool firstMouse = true;           // 是否是第一次打开窗口
float lastX = scrWidth / 2.0f;    // 上一帧鼠标X轴位置
float lastY = scrHeight / 2.0f;   // 上一帧鼠标Y轴位置
// 摄像机参数
float yaw = -90.0f; // 0.0f的偏航角会指向正右方向，所以需要把他转到正前方
float pitch = 0.0f;
float fov = 45.0f;
float maxFov = 45.0f;
// // 光源位置
// glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
// 聚光灯开关
float spotLightSwitch = 1.0;
// HDR曝光
float exposure = 1.0f;
// ---------------------------


int main() {
    // -----初始化GLFW-----
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                    // 使用OpenGL3.x版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                    // 使用OpenGLx.3版本
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);    // 使用Core模式
    glfwWindowHint(GLFW_SAMPLES, 4);                                  // 设置MSAA子采样点数

    // -----创建窗口对象-----
    GLFWwindow *window = glfwCreateWindow(scrWidth, scrHeight, "LearnOpenGL", NULL, NULL);    // 设置窗口宽高、名称
    glfwMakeContextCurrent(window);                                                           // 设置OpenGLContext
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // -----注册窗口变化监听-----
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);                        // 注册窗口大小函数
    glfwSetCursorPosCallback(window, mouse_callback);                                         // 注册鼠标输入函数
    glfwSetScrollCallback(window, scroll_callback);                                           // 注册鼠标滚轮函数

    // -----设置视口-----
    glViewport(0, 0, scrWidth, scrHeight);

    // -----模型加载-----
    Model Cube("./Assets/Mesh/TestScene/Cube.obj");
    Model Sphere("./Assets/Mesh/TestScene/Sphere.obj");
    Model Plane("./Assets/Mesh/TestScene/Plane.obj");

    // -----创建Shader-----
    Shader GeometryShader("./src/MyCode/29_SSAO/Shaders/Geometry.vert", "./src/MyCode/29_SSAO/Shaders/Geometry.frag");
    Shader SSAOShader("./src/MyCode/29_SSAO/Shaders/SSAO.vert", "./src/MyCode/29_SSAO/Shaders/SSAO.frag");
    Shader SSAOBlurShader("./src/MyCode/29_SSAO/Shaders/SSAOBlur.vert", "./src/MyCode/29_SSAO/Shaders/SSAOBlur.frag");
    Shader LightingShader("./src/MyCode/29_SSAO/Shaders/Lighting.vert", "./src/MyCode/29_SSAO/Shaders/Lighting.frag");


    // -----贴图加载-----
    unsigned int T_box_diffuse = loadTexture("./Assets/T_container_Diffuse.png", true);
    unsigned int T_box_specular = loadTexture("./Assets/T_container_specular.png", true);

    // -----绑定贴图通道-----
    GeometryShader.use();
    GeometryShader.setInt("Diffuse", 0);
    GeometryShader.setInt("Specular", 1);
    SSAOShader.use();
    SSAOShader.setInt("gPositionDepth", 0);
    SSAOShader.setInt("gNormal", 1);
    SSAOShader.setInt("texNoise", 2);
    SSAOBlurShader.use();
    SSAOBlurShader.setInt("SSAO", 0);
    LightingShader.use();
    LightingShader.setInt("gPositionDepth", 0);
    LightingShader.setInt("gNormal", 1);
    LightingShader.setInt("gColorSpec", 2);
    LightingShader.setInt("SSAO", 3);


    // -----模型世界空间位置-----
    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-2.0, -2.0, -2.0));
    objectPositions.push_back(glm::vec3(0.0, -2.0, -2.0));
    objectPositions.push_back(glm::vec3(2.0, -2.0, -2.0));
    objectPositions.push_back(glm::vec3(-2.0, -2.0, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -2.0, 0.0));
    objectPositions.push_back(glm::vec3(2.0, -2.0, 0.0));
    objectPositions.push_back(glm::vec3(-2.0, -2.0, 2.0));
    objectPositions.push_back(glm::vec3(0.0, -2.0, 2.0));
    objectPositions.push_back(glm::vec3(2.0, -2.0, 2.0));

    // -----灯光信息-----
    glm::vec3 lightPos = glm::vec3(1.0, 1.0, 1.0);
    glm::vec3 lightColor = glm::vec3(3.0, 3.0, 3.0);
    // -----延迟渲染G-buffer-----
    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // 创建片段位置、法线、颜色+镜面强度颜色缓冲
    GLuint gPositionDepth, gNormal, gColorSpec;
    glGenTextures(1, &gPositionDepth);
    glGenTextures(1, &gNormal);
    glGenTextures(1, &gColorSpec);
    FBOTextureAttach(gPositionDepth, GL_RGBA16F, GL_RGBA, 0);
    FBOTextureAttach(gNormal, GL_RGB16F, GL_RGB, 1);
    FBOTextureAttach(gColorSpec, GL_RGBA, GL_RGBA, 2);
    GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);
    // 创建深度缓冲RBO
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scrWidth, scrHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    CheckFBO("gBuffer");

    // -----SSAO Buffer-----
    unsigned int ssaoFBO, ssaoBlurFBO;
    unsigned int ssaoColorBuffer, ssaoBlurColorBuffer;
    glGenFramebuffers(1, &ssaoFBO); glGenFramebuffers(1, &ssaoBlurFBO);
    
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glGenTextures(1, &ssaoColorBuffer);
    FBOTextureAttach(ssaoColorBuffer, GL_RED, GL_RED, 0);
    CheckFBO("ssaoFBO");

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoBlurColorBuffer);
    FBOTextureAttach(ssaoBlurColorBuffer, GL_RED, GL_RED, 0);
    CheckFBO("ssaoBlurFBO");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // -----生成SSAO半球核-----
    uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    default_random_engine generator;
    vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 64; ++i) {
        // 每个采样点在半球核里的位置
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        // 根据序号向核心靠拢
        float scale = float(i) / 64.0;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    // -----生成SSAO随机旋转值-----
    vector<glm::vec3> ssaoNoise;
    for (int i = 0; i < 16; i++) {
        // 切线空间中绕z轴旋转
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise.push_back(noise);
    }

    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGBA, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // -----OpenGL渲染设置-----
    // 深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // 模板测试
    // glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // 开启混合
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 开启面剔除
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // 开启MSAA
    glEnable(GL_MULTISAMPLE);
    // 渲染设置
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window)) {
        // 处理输入
        processInput(window);
        glfwSetKeyCallback(window, key_callback);

        // 渲染指令
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // DeltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;

        // MVP矩阵
        glm::mat4 V = camera.GetViewMatrix();
        glm::mat4 P = glm::perspective(glm::radians(camera.Zoom), (float)scrWidth/(float)scrHeight, 0.1f, 100.0f);
        
        // Pass 1 几何处理阶段
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GeometryShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, T_box_diffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, T_box_specular);
        GeometryShader.setMat4("V", V);
        GeometryShader.setMat4("P", P);
        for (int i = 0; i < objectPositions.size(); i++) {
            glm::mat4 M = glm::mat4(1.0f);
            M = glm::translate(M, objectPositions[i]);
            M = glm::scale(M, glm::vec3(1.0f));
            GeometryShader.setMat4("M", M);
            GeometryShader.setVec3("Color", glm::vec3(0.9));
            Sphere.Draw(GeometryShader);
        }
        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, glm::vec3(0.0f, -3.0f, 0.0f));
        GeometryShader.setMat4("M", M);
        GeometryShader.setVec3("Color", glm::vec3(0.9));
        Plane.Draw(GeometryShader);

        // Pass 2 SSAO计算
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SSAOShader.use();
        // 发送采样点数据给Shader
        for (int i = 0; i < 64; i++) {
            SSAOShader.setVec3("samples[" + to_string(i) + "]", ssaoKernel[i]);
        }
        SSAOShader.setMat4("P", P);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        renderQuad();

        // Pass 3 Blur SSAO
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SSAOBlurShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        renderQuad();











        // Pass 4 光照计算阶段
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        LightingShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionDepth);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gColorSpec);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
        glm::vec3 lightPosVS = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0f));
        LightingShader.setVec3("camPos", camera.Position);
        LightingShader.setVec3("lightPos", lightPosVS);
        LightingShader.setVec3("lightCol", lightColor);
        LightingShader.setMat4("P", P);
        renderQuad();

        

        glfwSwapBuffers(window);    // 交换缓冲
        glfwPollEvents();           // 检查并调用事件
    }

    // 关闭窗口
    glfwTerminate();
    return 0;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                                // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// ----------设置函数----------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        spotLightSwitch *= -1.0f;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 加载纹理贴图
unsigned int loadTexture(char const *path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
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

// 设置帧缓冲纹理附件
void FBOTextureAttach(GLuint texAttach, GLint internalType, GLenum format, int number)
{
    // glGenTextures(1, &texAttach);
    glBindTexture(GL_TEXTURE_2D, texAttach);
    glTexImage2D(GL_TEXTURE_2D, 0, internalType, scrWidth, scrHeight, 0, format, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + number, GL_TEXTURE_2D, texAttach, 0);
}

// 检查FBO完整性
void CheckFBO(string FBOname) {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << FBOname << "not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}