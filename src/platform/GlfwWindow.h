#pragma once

struct GLFWwindow;

namespace hzl::platform
{
    class GlfwWindow
    {
    public:
        GlfwWindow(int width, int height, const char* title);
        ~GlfwWindow();

        GlfwWindow(const GlfwWindow&) = delete;
        GlfwWindow& operator=(const GlfwWindow&) = delete;

        bool isValid() const;
        bool shouldClose() const;
        void requestClose();
        void pollEvents() const;
        void swapBuffers() const;
        bool isKeyPressed(int key) const;
        void framebufferSize(int& width, int& height) const;
        float timeSeconds() const;

    private:
        static void framebufferSizeCallback(
            GLFWwindow* window,
            int width,
            int height
        );

        GLFWwindow* window_ = nullptr;
        bool glfwInitialized_ = false;
    };
}
