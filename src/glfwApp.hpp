#ifndef GLFW_APP_HPP
#define GLFW_APP_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/**
 * prepare:
 *  $ vcpkg install glad[extensions]  glfw3
 *  in CmakeLists.txt add below commands:
 *      find_package(glad CONFIG REQUIRED)
 *      target_link_libraries(geo PRIVATE glad::glad)
 *      find_package(glfw3 CONFIG REQUIRED)
 *      target_link_libraries(geo PRIVATE glfw)
 *
*/




enum os_type_t
{
    win_os = 0,
    linux_os,
    mac_os
};
#ifdef _WIN32
#include <time.h>
constexpr os_type_t os_type = win_os;
#else
#include <sys/time.h>
#ifdef __APPLE__
constexpr os_type_t os_type = mac_os;
#else
constexpr os_type_t os_type = linux_os;
#endif
#endif

// 基于glad + glfw
class myGlfwApp_t
{
protected:
    inline myGlfwApp_t(void) {}
    virtual ~myGlfwApp_t(void) {}

    static myGlfwApp_t *s_app;
    GLFWwindow *m_pWindow;

    static void window_size_callback(GLFWwindow *window, int width, int height)
    {
        myGlfwApp_t *pThis = (myGlfwApp_t *)glfwGetWindowUserPointer(window);
        pThis->Resize(width, height);
    };
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        myGlfwApp_t *pThis = (myGlfwApp_t *)glfwGetWindowUserPointer(window);
        pThis->OnKey(key, scancode, action, mods);
    };
    static void char_callback(GLFWwindow *window, unsigned int codepoint)
    {
        myGlfwApp_t *pThis = (myGlfwApp_t *)glfwGetWindowUserPointer(window);
        pThis->OnChar(codepoint);
    };

public:
    void MainLoop(void);

    virtual void Initialize(const char *title = nullptr, int win_width = 800, int win_heigh = 600, int opengl_major = 3, int opengl_minor = 3)
    {
        if (glfwInit() != GLFW_TRUE)
        {
            std::cout << "glfwInit fail" << std::endl;
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if constexpr (os_type == mac_os)
        {
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //for Mac OS X
        }

        m_pWindow = glfwCreateWindow(win_width, win_heigh, title, NULL, NULL);
        glfwSetWindowUserPointer(m_pWindow, this);
        glfwSetWindowSizeCallback(m_pWindow, window_size_callback);
        glfwSetKeyCallback(m_pWindow, key_callback);
        glfwSetCharCallback(m_pWindow, char_callback);

        if (m_pWindow == nullptr)
        {
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(m_pWindow);

        glfwSetFramebufferSizeCallback(m_pWindow, /*framebuffer_size_callback*/ [](GLFWwindow *window, int width, int height) {
            glViewport(0, 0, width, height);
        });

        // init glad
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            glfwTerminate();
            return;
        }

        this->Resize(win_width, win_heigh);
    };

    virtual void Display(bool auto_redraw = true)
    {
        glfwSwapBuffers(m_pWindow);
    }

    virtual void Finalize(void) {}

    virtual void Resize(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    virtual void OnKey(int key, int scancode, int action, int mods)
    { /* NOTHING */
    }
    virtual void OnChar(unsigned int codepoint)
    { /* NOTHING */
    }
};

#define BEGIN_APP_DECLARATION(appclass)    \
    class appclass : public myGlfwApp_t    \
    {                                      \
    public:                                \
        typedef class myGlfwApp_t base;    \
        static myGlfwApp_t *Create(void)   \
        {                                  \
            return (s_app = new appclass); \
        }

#define END_APP_DECLARATION() \
    }                         \
    ;

// #ifdef _WIN32
// #define MAIN_DECL int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
// #else
#define MAIN_DECL int main(int argc, char **argv)
// #endif

#define DEFINE_APP(appclass, title, win_width, win_heigh, opengl_major, opengl_minor) \
    myGlfwApp_t *myGlfwApp_t::s_app;                                                  \
                                                                                      \
    void myGlfwApp_t::MainLoop(void)                                                  \
    {                                                                                 \
        do                                                                            \
        {                                                                             \
            Display();                                                                \
            glfwPollEvents();                                                         \
        } while (!glfwWindowShouldClose(m_pWindow));                                  \
        return;                                                                       \
    }                                                                                 \
                                                                                      \
    MAIN_DECL                                                                         \
    {                                                                                 \
        myGlfwApp_t *app = appclass::Create();                                        \
                                                                                      \
        app->Initialize(title, win_width, win_heigh, opengl_major, opengl_minor);     \
        app->MainLoop();                                                              \
        app->Finalize();                                                              \
        glfwTerminate();                                                              \
                                                                                      \
        return 0;                                                                     \
    }

#endif
