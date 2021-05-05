
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include <glfwApp.hpp>

#include <shaderprogram.hpp>

BEGIN_APP_DECLARATION(KeyPressExample)
virtual void Initialize(const char *title, int win_width, int win_heigh, int opengl_major, int opengl_minor);
virtual void Display(bool auto_redraw);
virtual void Finalize(void);
// virtual void Resize(int width, int height);
void OnKey(int key, int scancode, int action, int mods);
GLuint program;
GLuint vao, vbo, ebo;
END_APP_DECLARATION()

DEFINE_APP(KeyPressExample, "Key Press Example", 800, 600, 3, 3)

void KeyPressExample::Finalize(void)
{

  // optional: de-allocate all resources once they've outlived their purpose:
  shaders::del_resource(this->vao, this->vbo, this->ebo);
  shaders::del_program(this->program);

  base::Finalize();
};

//----------------------------------------------------------------------------
//
// init
//

void KeyPressExample::Initialize(const char *title, int win_width, int win_heigh, int opengl_major, int opengl_minor)
{
  base::Initialize(title, win_width, win_heigh, opengl_major, opengl_minor);

  std::string vertexShaderSource = shaders::ReadShader_fromfile("media/vertx.vert");
  std::string fragmentShaderSource = shaders::ReadShader_fromfile("media/fragment.frag");
  this->program = shaders::create_link_program(vertexShaderSource, fragmentShaderSource);

  vertices_t vertices(3, 6);
              // 位置              // 颜色
  vertices << 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // 右下
             -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 左下
              0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f ;   // 顶部

  indices_t indices(1, 3);
  // note that we start from 0!
  indices << 0, 1, 2;        //  Triangle

  shaders::setup_vertex(this->vao, this->vbo, this->ebo,
                      //input
                      { {0, 3,0 }, {1, 3,3 } },
                      vertices, indices);
}

void KeyPressExample::OnKey(int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS)
  {
    switch (key)
    {
    case GLFW_KEY_M:
    {
      static GLenum mode = GL_FILL;

      mode = (mode == GL_FILL ? GL_LINE : GL_FILL);
      glPolygonMode(GL_FRONT_AND_BACK, mode);  //设置为线框模式
    }
    break;
    case GLFW_KEY_ESCAPE:
    {
      glfwSetWindowShouldClose(this->m_pWindow, true);
    }
    return;
    }
  }

  base::OnKey(key, scancode, action, mods);
}

void KeyPressExample::Display(bool auto_redraw)
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // draw our first triangle
  glUseProgram(this->program);
  glBindVertexArray(this->vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
  //glDrawArrays(GL_TRIANGLES, 0, 6);

  //指明我们从索引缓冲ebo渲染, 
  glDrawElements(GL_TRIANGLES, 3/*绘制顶点的个数*/, GL_UNSIGNED_INT, 0/*指定EBO中的偏移量*/);
  // glBindVertexArray(0); // no need to unbind it every time
  base::Display(auto_redraw);
}
