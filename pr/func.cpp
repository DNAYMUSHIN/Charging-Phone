// func.cpp
#include "func.h"
#include "globals.h"

string LoadShader(const char* filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Cannot open shader file: " << filename << std::endl;
        return "";
    }
    std::ostringstream s;
    s << in.rdbuf();
    return s.str();
}

GLFWwindow* InitAll(int w, int h, bool Fullscreen) {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(w, h, "Phone Charging Scene", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    WinWidth = w;
    WinHeight = h;

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);

    return window;
}

void EndAll() {
    glfwTerminate();
}

void window_size_callback(GLFWwindow* window, int width, int height) {
    WinWidth = width;
    WinHeight = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // оставляем пустым — в коде main используется опрос состояния клавиш
    // здесь можно добавить обработку нажатия клавиш если нужно
    (void)window; (void)key; (void)scancode; (void)action; (void)mods;
}
