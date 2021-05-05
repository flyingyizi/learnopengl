#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <string>
#include <type_traits>

#include <glad/glad.h>

#include <Eigen/Core>
// 稠密矩阵的代数运算（逆，特征值等）
#include <Eigen/Dense>

#include <functional>
#include <tuple>

// vertex_attri_index : Specifies the index of the generic vertex attribute to be modified,it defined in vert-glsl
// size :Specifies the number of components per generic vertex attribute. Must be 1, 2, 3, 4.
// offset_index :Specifies a offset index of the first component of the first generic vertex attribute in the array
using vertexAttrib_t = std::initializer_list<std::tuple<int /*vertex_attri_index*/,
                                                        int /*size*/,
                                                        int /*offset_index*/>>;

// 必须是GLfloat，因为涉及到opengl规范强制约束
using vertices_t = Eigen::Matrix<GLfloat, -1, -1, Eigen::RowMajor>;
using indices_t = Eigen::Matrix<GLuint, -1, -1, Eigen::RowMajor>;
// Eigen::MatrixXd
namespace shaders
{
    std::basic_string<GLchar> ReadShader_fromfile(std::string filename);
    GLuint create_link_program(std::string &vert_glsl, std::string &frag_glsl);

    // vertices 每行代表一个vertex， 一个vertex可能包含一个或多个vertex attrib
    //  通过输入howto_do信息告诉gpu如何分析每个vertex attrib
    //usage参数指定了我们希望显卡如何管理给定的数据。它有三种形式：
    //   GL_STATIC_DRAW ：数据不会或几乎不会改变。 GL_DYNAMIC_DRAW：数据会被改变很多。
    //   GL_STREAM_DRAW ：数据每次绘制时都会改变。
    //howto 指定opengl 如何解析vertex attribute
    //
    // 注意： 由于我们构建了ebo，因此实际渲染时调用glDrawElements //指明我们从索引缓冲ebo渲染, 
    //         glDrawElements(GL_TRIANGLES, 6/*绘制顶点的个数*/, GL_UNSIGNED_INT, 0/*指定EBO中的偏移量*/);

    void setup_vertex(GLuint &VBO, GLuint &VAO, GLuint &EBO,
                      //input
                      vertexAttrib_t howto_do,
                      const vertices_t &vertices, const indices_t &indices,
                      GLenum vbo_usage = GL_STATIC_DRAW, GLenum ebo_usage = GL_STATIC_DRAW);

    void del_program(GLuint prog);
    void del_resource(GLuint &VAO, GLuint &VBO, GLuint &EBO);

    // 设置uniform 值, 当传递location name时，必须传递prog
    //fail return -1
    template <typename LOCT, typename Scaler>
    int uniform_set(LOCT loc, std::vector<Scaler> data,int prog=-1)
    {
        if (data.size() == 0 || data.size() > 4)
            return -1;

        int location = 0;

        if constexpr (std::is_same<LOCT, std::string>::value == true)
        {   if(prog==-1) return -1;
            location = glGetUniformLocation(prog, loc.c_str());
        }
        else if constexpr (std::is_integral<LOCT>::value == true)
        {
            location = loc;
        }
        else
        {
            return -1;
        }

        //glUniform1f
        if constexpr (std::is_same<Scaler, GLfloat>::value == true)
        {
            switch (data.size())
            {
            case 1:
                glUniform1f(location, data[0]);
                break;
            case 2:
                glUniform2f(location, data[0], data[1]);
                break;
            case 3:
                glUniform3f(location, data[0], data[1], data[2]);
                break;
            case 4:
                glUniform4f(location, data[0], data[1], data[2], data[3]);
                break;
            }
            return 0;
        }
        else if constexpr (std::is_same<Scaler, GLuint>::value == true)
        {
            switch (data.size())
            {
            case 1:
                glUniform1ui(location, data[0]);
                break;
            case 2:
                glUniform2ui(location, data[0], data[1]);
                break;
            case 3:
                glUniform3ui(location, data[0], data[1], data[2]);
                break;
            case 4:
                glUniform4ui(location, data[0], data[1], data[2], data[3]);
                break;
            }
            return 0;
        }
        else
        {
            return -1;
        }
    };

};

#ifdef _DEBUG
constexpr bool is_debug = true;
#else
constexpr bool is_debug = false;
#endif

#endif