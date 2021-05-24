#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include <gtkmmApp.hpp>

constexpr int SCR_WIDTH = 800;
constexpr int SCR_HEIGHT = 600;

#include <locale.h>
#include <locale>

class mydraw_t : public drawarea_base_t
{
public:
   // member
   shaders::prog_t lightingShader_prog, lightCubeShader_prog;
   shaders::model_t cubeModel, planeModel;

public:
   mydraw_t(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder)
       : drawarea_base_t(cobject, builder)
   {
      auto keyevent = Gtk::EventControllerKey::create();
      keyevent->signal_key_pressed().connect(sigc::mem_fun(*this, &mydraw_t::on_selfkeypressed), false);
      this->add_controller(keyevent);

      auto drag = Gtk::GestureDrag::create();
      drag->set_button(GDK_BUTTON_PRIMARY);
      drag->signal_drag_begin().connect(sigc::mem_fun(*this, &mydraw_t::on_selfdrag_begin), false);
      drag->signal_drag_update().connect(sigc::mem_fun(*this, &mydraw_t::on_selfdrag_update));
      drag->signal_drag_end().connect(sigc::mem_fun(*this, &mydraw_t::on_selfdrag_end), false);
      this->add_controller(drag);
   };

private:
   void on_selfdrag_begin(double start_x, double start_y)
   {
      // update last x/y
      drawarea_base_t::on_mousemove(start_x, start_y);
   }
   void on_selfdrag_update(double offset_x, double offset_y)
   {
      //camera 使用右手系， 而物理屏幕坐标是左上为原点
      float xoffset = offset_x;
      float yoffset = -offset_y; // reversed since y-coordinates go from bottom to top
      thecamera.ProcessMouseMovement(xoffset, yoffset);
      // std::cout << "mouse_x:" << mouse_x << " mouse_x:" << mouse_y << std::endl;
      this->queue_render();
   }
   void on_selfdrag_end(double offset_x, double offset_y)
   {
      on_selfdrag_update(offset_x, offset_y);
   }
   gboolean on_selfkeypressed(guint keyval, guint keycode, Gdk::ModifierType state)
   {
      // std::cout << "receive keyval:" << keyval << std::endl;
      const auto &sym = keyval;
      if (sym == GDK_KEY_Escape || sym == GDK_KEY_q || sym == GDK_KEY_Q)
      {
         //gtk_main_quit();
         return GDK_EVENT_STOP;
      }

      if (sym == GDK_KEY_W || sym == GDK_KEY_w)
         this->thecamera.ProcessKeyboard(cv::Camera_Movement::FORWARD, deltaTime);
      else if (sym == GDK_KEY_S || sym == GDK_KEY_s)
         this->thecamera.ProcessKeyboard(cv::Camera_Movement::BACKWARD, deltaTime);
      else if (sym == GDK_KEY_A || sym == GDK_KEY_a)
         this->thecamera.ProcessKeyboard(cv::Camera_Movement::LEFT, deltaTime);
      else if (sym == GDK_KEY_D || sym == GDK_KEY_d)
         this->thecamera.ProcessKeyboard(cv::Camera_Movement::RIGHT, deltaTime);
      else if (sym == GDK_KEY_M || sym == GDK_KEY_m)
      {
         static GLenum mode = GL_FILL;
         mode = (mode == GL_FILL ? GL_LINE : GL_FILL);
         this->make_current();
         glPolygonMode(GL_FRONT_AND_BACK, mode); //设置为线框模式
         std::cout << "toggle polygmode" << std::endl;
      }
      else
      {
         return GDK_EVENT_PROPAGATE;
      }

      this->queue_render();
      return GDK_EVENT_STOP;
   };

private:
   bool on_selfrender(const Glib::RefPtr<Gdk::GLContext> &context)
   {
      // per-frame time logic
      // --------------------
      auto currentFrame = get_frame_clock()->get_fps();
      this->deltaTime = currentFrame - this->lastFrame;
      this->lastFrame = currentFrame;

      // std::cout << "---trigger on_selfrender;" << std::endl;
      try
      {
         throw_if_error();
         // glViewport(0, 0, this->get_width(), this->get_height());

         // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClearColor(0.5, 0.5, 0.5, 1.0);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         //   glClear (GL_COLOR_BUFFER_BIT);

         // be sure to activate shader when setting uniforms/drawing objects
         lightingShader_prog.use();
         // view/projection transformations

         auto projection = trans::perspective(radians(thecamera.Zoom),
                                              (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                              0.1f, 100.0f);
         auto view = thecamera.GetViewMatrix();
         Eigen::Affine3f model;
         model = Eigen::Translation3f(-1.0f, 0.0f, -1.0f); //* Eigen::Scaling(1.0f, 1.0f, 1.0f);
         lightingShader_prog.uniform_set<Eigen::Matrix4f>({
             {"projection", projection},
             {"view", view},
             {"model", model.matrix()},
         });
         // render the cube
         cubeModel.Draw(lightingShader_prog);

         model = Eigen::Translation3f(2.0f, 0.0f, 0.0f);
         lightingShader_prog.uniform_set<Eigen::Matrix4f>({
             {"model", model.matrix()},
         });
         // render the cube
         cubeModel.Draw(lightingShader_prog);

         //floor
         lightingShader_prog.uniform_set<Eigen::Matrix4f>({
             {"model", Eigen::Matrix4f::Identity()},
         });
         // render the cube
         planeModel.Draw(lightingShader_prog);
         // swapbuffers(this->m_state);
         // base::OnDraw();
         // std::cout << "draw" << std::endl;
         glFlush();

         // for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
         // {
         //    //Process/log the error.
         //    std::cout << "opengl error:" << err << std::endl;
         // }

         return true;
      }
      catch (const Gdk::GLError &gle)
      {
         std::cout << "An error occurred in the render callback of the GLArea" << std::endl;
         std::cout << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
         return false;
      }
   };


   void on_selfrealize()
   {
      this->lastX = this->get_width() / 2.0f;
      this->lastY = this->get_height() / 2.0f;

      glEnable(GL_DEPTH_TEST);
      thecamera = cv::camera_t(Eigen::Vector3f(0.0f, 0.0f, 3.0f));

      // build and compile our shader zprogram
      // ------------------------------------
      auto r = lightingShader_prog.load(
          R"(
            #version 330 core
            // #version 330
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec2 aTexCoords;

            out vec2 TexCoords;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main()
            {
               TexCoords = aTexCoords;
               gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
         )",
          R"(
            #version 330 core
            // #version 330
            out vec4 FragColor;

            in vec2 TexCoords;

            uniform sampler2D texture1;

            void main()
            {
               FragColor = texture(texture1, TexCoords);
            }

      )");

      assert(r == true);

      try
      {
         this->throw_if_error();
      }
      catch (const Gdk::GLError &gle)
      {
         std::cout << "An error occured making the context current during realize:" << std::endl;
         std::cout << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
      }

      std::vector<GLfloat> cubeNative{
          // positions          // texture Coords
          -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
          0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
          -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
          -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
          0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
          0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
          0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
          -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

          -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
          -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
          -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

          0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
          0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
          0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
          0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
          0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
          0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
          0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

          -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
          -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
          -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

      std::vector<GLfloat> planeNative{
          // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
          5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
          -5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
          -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

          5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
          -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
          5.0f, -0.5f, -5.0f, 2.0f, 2.0f};

      cubeModel.loadNativeData(5,
                               {
                                   {shaders::vertexType_t::POSITION_ENU, 0},
                                   {shaders::vertexType_t::TEXCOORD_ENU, 3},
                               },
                               cubeNative,
                               {
                                   {aiTextureType_HEIGHT, "../../resources/textures/marble.jpg"},
                               });

      planeModel.loadNativeData(5,
                                {
                                    {shaders::vertexType_t::POSITION_ENU, 0},
                                    {shaders::vertexType_t::TEXCOORD_ENU, 3},
                                },
                                planeNative,
                                {
                                    {aiTextureType_HEIGHT, "../../resources/textures/metal.png"},
                                });

      lightingShader_prog.use();
      lightingShader_prog.uniform_set<GLuint>({
          {"texture1", 0},
      });
      // glBindAttribLocation(lightingShader_prog.progid, 0, "aPos");
      // glBindAttribLocation(lightingShader_prog.progid, 1, "aTexCoords");

      std::cout << "---trigger on_selfrealize end;" << std::endl;
   }

   void on_selfunrealize()
   {
      std::cout << "---trigger on_selfunrealize end;" << std::endl;
      // if (gtk_gl_area_get_error(this->gobj()) != NULL)
      //    return;
      // //   glDeleteProgram (program);
   }
};

BEGIN_APP_DECLARATION(KeyPressExample)
virtual void _initialize(const char *title, int win_width,
                         int win_heigh);
gboolean on_appkeypressed(guint keyval, guint keycode, Gdk::ModifierType state);

Gtk::AspectFrame *m_aspectFrame;
mydraw_t *m_drawarea;
Glib::RefPtr<Gtk::EventControllerKey> m_keyevent;

END_APP_DECLARATION()

DEFINE_APP(KeyPressExample, "Key Press Example", SCR_WIDTH, SCR_HEIGHT, 3, 3)

gboolean KeyPressExample::on_appkeypressed(guint keyval, guint keycode, Gdk::ModifierType state)
{
   return m_keyevent->forward(*m_drawarea);
}

void KeyPressExample::_initialize(const char *title, int win_width,
                                  int win_heigh)
{
   set_title(title);
   this->set_size_request(win_width, win_heigh);
   this->set_default_size(win_width, win_heigh);
   // gtk_window_set_position(GTK_WINDOW(this->m_widget), GTK_WIN_POS_CENTER);

   auto a = m_builder->get_widget<Gtk::Window>("main_window"); //

   m_aspectFrame = m_builder->get_widget<Gtk::AspectFrame>("mainwindow_aspectframe1"); //
   assert(m_aspectFrame != nullptr);
   // m_aspectFrame->set_ratio( 9.0f / 16.0f);
   m_aspectFrame->set_expand();
   m_drawarea = Gtk::Builder::get_widget_derived<mydraw_t>(m_builder, "drawarea");
   assert(m_drawarea != nullptr);

   m_keyevent = Gtk::EventControllerKey::create();
   m_keyevent->signal_key_pressed().connect(sigc::mem_fun(*this, &KeyPressExample::on_appkeypressed), false);
   this->add_controller(m_keyevent);
};

// inline EGLNativeWindowType XID(const Glib::RefPtr<Gdk::Surface> &surf)
// {
// #ifndef _WIN32
//    return GDK_SURFACE_XID(surf->gobj());
// #else
//    return (EGLNativeWindowType)GDK_SURFACE_HWND(surf->gobj());
// #endif
// }

// inline EGLNativeDisplayType XDISPLAY(const Glib::RefPtr<Gdk::Surface> &surf)
// {
// #ifndef _WIN32
//    return GDK_SURFACE_XDISPLAY(surf->gobj());
// #else
//    auto hwnd = (EGLNativeWindowType)GDK_SURFACE_HWND (surf->gobj());
//    return GetDC ( hwnd );
//    //GDK_WIN32_DISPLAY
// #endif
// }

// {

// }