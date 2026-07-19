#include "platform/GlfwWindow.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace hzl::platform
{
    GlfwWindow::GlfwWindow(int width, int height, const char* title)
    {
        glfwInitialized_ = glfwInit() == GLFW_TRUE;

        if (!glfwInitialized_)
        {
            std::cerr << "Failed to initialize GLFW.\n";
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if (window_ == nullptr)
        {
            std::cerr << "Failed to create GLFW window.\n";
            return;
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    }

    GlfwWindow::~GlfwWindow()
    {
        if (window_ != nullptr)
        {
            glfwDestroyWindow(window_);
        }

        if (glfwInitialized_)
        {
            glfwTerminate();
        }
    }

    bool GlfwWindow::isValid() const
    {
        return window_ != nullptr;
    }

    bool GlfwWindow::shouldClose() const
    {
        return glfwWindowShouldClose(window_) == GLFW_TRUE;
    }

    void GlfwWindow::requestClose()
    {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }

    void GlfwWindow::pollEvents() const
    {
        glfwPollEvents();
    }

    void GlfwWindow::swapBuffers() const
    {
        glfwSwapBuffers(window_);
    }

    bool GlfwWindow::isKeyPressed(int key) const
    {
        return glfwGetKey(window_, key) == GLFW_PRESS;
    }

    void GlfwWindow::framebufferSize(int& width, int& height) const
    {
        glfwGetFramebufferSize(window_, &width, &height);
    }

    float GlfwWindow::timeSeconds() const
    {
        return static_cast<float>(glfwGetTime());
    }

    void GlfwWindow::framebufferSizeCallback(
        GLFWwindow*,
        int width,
        int height
    )
    {
        glViewport(0, 0, width, height);
    }
}
