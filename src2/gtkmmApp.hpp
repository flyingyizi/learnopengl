#ifndef GTKMMWINDOW_APP_HPP
#define GTKMMWINDOW_APP_HPP

#pragma push_macro("WIN32")
#undef WIN32
#include <gtkmm.h>
#pragma pop_macro("WIN32")

#include <epoxy/gl.h>
#include <myglutil.hpp>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

// #if defined(_WIN32) || defined(__CYGWIN__)
// #include <gdk/win32/gdkwin32.h>
// #include <epoxy/wgl.h>
// #else
// #include <gdk/x11/gdkx.h>
// #include <epoxy/glx.h>
// #endif
#include "gdk/gdkkeysyms.h"



#include <gtk/gtk.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <sys/time.h>
#include <vector>

// base class for opengl drawing
class drawarea_base_t : public Gtk::GLArea
{
public:
    // member
    cv::camera_t thecamera;
    float deltaTime = 0.0f; // 当前帧与上一帧的时间差
    float lastFrame = 0.0f; // 上一帧的时间

    float lastX = 0.f; //鼠标最后所在位置
    float lastY = 0.f;

    int win_lastXpos = 0; //Win最后所在位置
    int win_lastYpos = 0;

    bool is_using_es = false;

public:
    drawarea_base_t(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder)
        : Gtk::GLArea(cobject)
    {
       assert(cobject!=nullptr && builder!=nullptr);

        this->set_hexpand();
        this->set_vexpand();
        this->set_size_request();
        this->signal_realize().connect(sigc::mem_fun(*this, &drawarea_base_t::realize));
        this->signal_unrealize().connect(sigc::mem_fun(*this, &drawarea_base_t::unrealize));
        this->signal_render().connect(sigc::mem_fun(*this, &drawarea_base_t::render), false);
        // this->set_auto_render(true);
    };

public:
    virtual void on_selfrealize() = 0;
    virtual bool on_selfrender(const Glib::RefPtr<Gdk::GLContext> &context) = 0;
    virtual void on_selfunrealize() = 0;

protected:
    bool render(const Glib::RefPtr<Gdk::GLContext> &context)
    {
        return on_selfrender(context);
    }
    void realize()
    {
        // //gtkmm default provided
        // this->on_realize();

        try
        {
        this->make_current();
            this->throw_if_error();
        }
        catch(const Glib::Error &e) {
            std::cout << "An error occured making the context current during realize:" << std::endl;
        }
        // catch (const Gdk::GLError &gle)
        // {
        //     std::cout << "An error occured making the context current during realize:" << std::endl;
        //     std::cout << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
        // }

        is_using_es = this->get_context()->get_use_es();

        if (is_using_es == true)
        {
            std::cout << "env is opengl es" << std::endl;
        }
        else
        {
            std::cout << "env is opengl" << std::endl;
        }
        glEnable(GL_DEPTH_TEST);

        this->on_selfrealize();
    };
    void unrealize()
    {
        // //gtkmm default provided
        // this->on_unrealize();

        try
        {
        this->make_current();
            this->throw_if_error();
        }
        catch(const Glib::Error &e) {
            std::cout << "An error occured making the context current during realize:" << std::endl;
        }

        on_selfunrealize();

    };

    //
    virtual void on_mousemove(double x, double y)
    {
      // 记录鼠标位置
      static bool firstMouse = true;
      if (firstMouse)
      {
         this->lastX = x;
         this->lastY = y;
         firstMouse = false;
      }

      this->lastX = x;
      this->lastY = y;

    }
};




#define BEGIN_APP_DECLARATION(appclass)                        \
    class appclass : public Gtk::Window                        \
    {                                                          \
    public:                                                    \
        Glib::RefPtr<Gtk::Builder> m_builder;                  \
        typedef class myGtkmmApp_t base;                       \
        appclass(BaseObjectType *cobject,                      \
                 const Glib::RefPtr<Gtk::Builder> &builder,    \
                 const char *title, int win_width,             \
                 int win_heigh, int opengl_major,              \
                 int opengl_minor)                             \
            : Gtk::Window(cobject), m_builder(builder)         \
        {                                                      \
            _initialize(title, win_width,                      \
                        win_heigh);                            \
            this->signal_show().connect([&]() { on_show(); }); \
        };

#define END_APP_DECLARATION() \
    }                         \
    ;

#define DEFINE_APP(appclass, title, win_width, win_heigh, opengl_major, opengl_minor)           \
    int main(int argc, char *argv[])                                                            \
    {                                                                                           \
        std::string _id = std::string("com.example.") + #appclass;                              \
        auto app = Gtk::Application::create(_id.c_str());                                       \
        auto on_app_activate = [&app]() {                                                       \
            /*static_cast<void>(appclass());*/                                                  \
            auto refBuilder = Gtk::Builder::create();                                           \
            try                                                                                 \
            {                                                                                   \
                refBuilder->add_from_file("../../resources/gladeui/basic.glade");               \
            }                                                                                   \
            catch (const Glib::Error &ex)                                                       \
            {                                                                                   \
                std::cout << "Error: " << ex.what() << std::endl;                               \
                return;                                                                         \
            }                                                                                   \
            auto pWin = Gtk::Builder::get_widget_derived<appclass>(refBuilder, "main_window",   \
                                                                   title, win_width, win_heigh, \
                                                                   opengl_major, opengl_minor); \
            pWin->signal_hide().connect([pWin]() { delete pWin; });                             \
            app->add_window(*pWin);                                                             \
            pWin->show();                                                                       \
        };                                                                                      \
        app->signal_activate().connect(on_app_activate);                                        \
        return app->run(argc, argv);                                                            \
    }



#endif
