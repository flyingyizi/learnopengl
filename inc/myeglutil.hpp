#ifndef MYEGLUTIL_HPP
#define MYEGLUTIL_HPP

// #define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

struct egl_state_t
{
    EGLNativeDisplayType dpy; //native
    EGLNativeWindowType win;  //native , valid when in window mode
    EGLDisplay egl_dpy = EGL_NO_DISPLAY;
    EGLConfig cfg;
    EGLContext ctx = EGL_NO_CONTEXT;
    EGLSurface surf = EGL_NO_SURFACE;
    EGLint major, minor;
    int depth;
    // int width; //丢弃，需要时采用类似  eglQuerySurface( this->state.egl_dpy, this->state.surf,EGL_WIDTH,&width);获取
    // int height;

    bool inited = false;
};

// // 关联eglstate, 失败返回false
// /**
//  * 下面演示了 如何从gtk的widget获取需要nativewin和nativedisplay
//  * GtkWidget *widget;
//  * ...
//  * auto gdkwin = gtk_widget_get_window(m_widget);
//  *  EGLNativeWindowType  _win = GDK_WINDOW_XID(gdkwin);
//  *  EGLNativeDisplayType _dpy = GDK_WINDOW_XDISPLAY(gdkwin);
//  *  auto win_width = gtk_widget_get_allocated_width (widget);
//  *  auto win_height= gtk_widget_get_allocated_height (widget);
//  *
//  * 对原生xlib生成的，就更直接
//  *   EGLNativeDisplayType   _dpy = XOpenDisplay(NULL);
//  *   EGLNativeWindowType    _win = XCreateWindow(...)
//  * 对gtkmm4，下面是获取的例子
//  *    inline EGLNativeWindowType _get_XID(const Glib::RefPtr<Gdk::Surface> &surf)
//  *    {
//  *    #if defined(_WIN32) || defined(__CYGWIN__)
//  *    return (EGLNativeWindowType)GDK_SURFACE_HWND(surf->gobj());
//  *    #else
//  *    return GDK_SURFACE_XID(surf->gobj());
//  *    #endif
//  *    }
//  *
//  *    inline EGLNativeDisplayType _get_XDISPLAY(const Glib::RefPtr<Gdk::Surface> &surf)
//  *    {
//  *    #if defined(_WIN32) || defined(__CYGWIN__)
//  *    auto hwnd = (EGLNativeWindowType)GDK_SURFACE_HWND (surf->gobj());
//  *    return GetDC ( hwnd );
//  *    //GDK_WIN32_DISPLAY
//  *    #else
//  *    return GDK_SURFACE_XDISPLAY(surf->gobj());
//  *    #endif
//  *    }
//  * /
bool attach_eglstate4Win(EGLNativeWindowType _win, EGLNativeDisplayType _dpy,
                            int win_width, int win_height,
                            //output
                            egl_state_t &state);

bool prepare_PBuffer(int _width, int _height,
                     //output
                     egl_state_t &state);
void swapbuffers(const egl_state_t &state);

/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MYEGL_UTILITY__IMPLEMENTATION

void swapbuffers(const egl_state_t &state)
{
    eglSwapBuffers(state.egl_dpy, state.surf);
}

static bool init_eglenv(const std::vector<EGLint> &_config_attribs,
                        const NativeDisplayType &_dpy, const EGLDisplay &_egl_dpy,
                        //output
                        EGLConfig &_cfg,
                        EGLContext &_ctx)
{
    int i;

    if (*_config_attribs.rbegin() != EGL_NONE)
    {
        std::cout << "_config_attribs must be ended by EGL_NONE, fail"
                  << std::endl;
        return false;
    }

    auto iter = std::find(_config_attribs.begin(), _config_attribs.end(), EGL_RENDERABLE_TYPE);
    if (iter == _config_attribs.end())
    {
        eglBindAPI(EGL_OPENGL_ES_BIT);
    }
    else
    {
        switch (*std::next(iter))
        {
        case EGL_OPENGL_BIT:
            eglBindAPI(EGL_OPENGL_API);
            break;
        case EGL_OPENGL_ES_BIT:
        case EGL_OPENGL_ES2_BIT:
        case EGL_OPENGL_ES3_BIT:
            eglBindAPI(EGL_OPENGL_ES_API);
            break;
        case EGL_OPENVG_BIT:
            eglBindAPI(EGL_OPENVG_API);
            break;
        default:
            return false;
        }
    }

    EGLint majorVersion, minorVersion;
    if (!eglInitialize(_egl_dpy, &majorVersion, &minorVersion))
    {
        std::cout << "eglInitialize() failed\n"
                  << std::endl;
        return false;
    }
    //todo using eglQueryString to check egl extentions

    EGLint count;
    if (!eglChooseConfig(_egl_dpy, _config_attribs.data(), &_cfg, 1, &count) ||
        count == 0)
    {
        std::cout << "eglChooseConfig() failed\n"
                  << std::endl;
        return false;
    }

    _ctx = eglCreateContext(_egl_dpy, _cfg,
                            EGL_NO_CONTEXT, NULL);
    if (_ctx == EGL_NO_CONTEXT)
    {
        std::cout << "eglCreateContext() failed\n"
                  << std::endl;
        return false;
    }
    return true;
}

bool attach_eglstate4Win(EGLNativeWindowType _win, EGLNativeDisplayType _dpy,
                            int win_width, int win_height,
                            //output
                            egl_state_t &state)
{
    if (_dpy == EGL_NO_DISPLAY)
        return false;

    state.inited = false;
    std::vector<EGLint> config_attribs{
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, //EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        // EGL_SAMPLE_BUFFERS, 1,
        EGL_RENDERABLE_TYPE,  EGL_OPENGL_BIT,
        EGL_NONE};

    state.win = _win;
    state.dpy = _dpy;
    state.egl_dpy = eglGetDisplay(state.dpy);
    if (state.egl_dpy == EGL_NO_DISPLAY)
    {
        std::cout << "eglGetDisplay() failed\n"
                  << std::endl;
        return false;
    }
    if (false == init_eglenv(config_attribs, state.dpy, state.egl_dpy,
                             //output
                             state.cfg, state.ctx))
    {
        std::cout << "init_eglenv fail" << std::endl;
        return false;
    }

    state.surf = eglCreateWindowSurface(state.egl_dpy,
                                        state.cfg, state.win, NULL);

    if (state.surf == EGL_NO_SURFACE)
    {
        std::cout << "eglCreateWindowSurface() failed\n"
                  << std::endl;
        return false;
    }
    
    if (!eglMakeCurrent(state.egl_dpy,
                        state.surf, state.surf, state.ctx))
    {
        std::cout <<"state.egl_dpy:"<<state.egl_dpy <<
                        " .state.surf:"<<state.surf<< " .state.ctx:"<<state.ctx<<" ."<<win_width<<","<<win_height<<std::endl;
        std::cout << "eglMakeCurrent() failed\n"
                  << std::endl;
        return false;
    }
    // 官方文档提示它不再使用了
    // gtk_widget_set_double_buffered(GTK_WIDGET(w), FALSE); //否则与opengl冲突

    state.inited = true;

    return true;
}



#endif //MYEGL_UTILITY__IMPLEMENTATION

#endif