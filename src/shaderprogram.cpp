

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include <shaderprogram.hpp>

namespace shaders
{
    std::basic_string<GLchar> ReadShader_fromfile(std::string filename)
    {
        std::basic_string<GLchar> out;
        std::ifstream file(filename);
        if (file.fail())
        {
            return out;
        }

        std::ostringstream oss;
        oss << file.rdbuf();
        out = oss.str();
        file.close();

        return out;
    };

    GLuint create_link_program(std::string &vert_glsl, std::string &frag_glsl)
    {
        //helper
        struct ShaderInfo
        {
            GLenum type;
            std::string shaderstr;
            GLuint shader;
        };

        auto LoadShaders = [](std::vector<ShaderInfo> &shaders) -> GLuint {
            if (shaders.empty() == true)
                return 0;

            GLuint prog = glCreateProgram();

            for (auto iter = shaders.begin(); iter != shaders.end(); iter++)
            {
                if (iter->type == GL_NONE)
                    continue;
                iter->shader = glCreateShader(iter->type);
                // const GLchar *source = entry->shaderstr.data();
                auto source = iter->shaderstr.c_str();
                glShaderSource(iter->shader, 1, &source, NULL);
                glCompileShader(iter->shader);

                // check for shader compile errors
                GLint success;
                glGetShaderiv(iter->shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    if constexpr (is_debug == true)
                    {
                        GLsizei len;
                        glGetShaderiv(iter->shader, GL_INFO_LOG_LENGTH, &len);

                        GLchar *log = new GLchar[len + 1];
                        glGetShaderInfoLog(iter->shader, len, &len, log);
                        std::cerr << "Shader compilation failed: " << log << std::endl;
                        delete[] log;
                    }

                    return 0;
                }
                // attach shaders
                glAttachShader(prog, iter->shader);
            }

            // link shaders
            glLinkProgram(prog);

            // check for linking errors
            GLint linked;
            glGetProgramiv(prog, GL_LINK_STATUS, &linked);
            if (!linked)
            {
                if constexpr (is_debug == true)
                {
                    GLsizei len;
                    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);

                    GLchar *log = new GLchar[len + 1];
                    glGetProgramInfoLog(prog, len, &len, log);
                    std::cerr << "Shader linking failed: " << log << std::endl;
                    delete[] log;
                }
                return 0;
            }
            //在把着色器对象链接到程序对象以后，记得删除着色器对象，我们不再需要它们了
            for (auto j = shaders.begin(); j != shaders.end() && j->type != GL_NONE; j++)
            {
                glDeleteShader(j->shader);
                j->shader = 0;
            }

            return prog;
        };

        auto shaders = std::vector<ShaderInfo>{
            {GL_VERTEX_SHADER, vert_glsl}, {GL_FRAGMENT_SHADER, frag_glsl}, {GL_NONE, ""}};

        GLuint program = LoadShaders(shaders);
        return program;
    };
    void del_program(GLuint prog)
    {
        glDeleteProgram(prog);
    };

    // set up vertex data (and buffer(s)) and configure vertex attributes
    //  return VAOID,vboid, eboid
    void setup_vertex(GLuint &VBO, GLuint &VAO, GLuint &EBO,
                      //input
                      vertexAttrib_t howto,
                      const vertices_t &vertices, const indices_t &indices,
                      GLenum vbo_usage, GLenum ebo_usage)
                      
    {
        // ------------------------------------------------------------------
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices_t::Scalar), vertices.data(), vbo_usage);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices_t::Scalar), indices.data(), ebo_usage);

        // 告诉OpenGL该如何解析顶点数据（应用到逐个顶点属性上）
        {
            auto vertices_stride = vertices.cols() * sizeof(vertices_t::Scalar);
            for(auto &hw: howto) {
                int vertex_attri_index = std::get<0>(hw);
                int ssize = std::get<1>(hw);
                int offset_index  = std::get<2>(hw);

                glVertexAttribPointer(vertex_attri_index, ssize, 
                                      GL_FLOAT, GL_FALSE, vertices_stride,
                                      (GLvoid*)(offset_index*sizeof(vertices_t::Scalar)) );
                glEnableVertexAttribArray(vertex_attri_index);
            }
        }

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);
        return;
    };

    void del_resource(GLuint &VAO, GLuint &VBO, GLuint &EBO)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    };

    // void display(myegl::context &ctx, GLuint array)
    // {
    //     glClear(GL_COLOR_BUFFER_BIT);

    //     glBindVertexArray(array /*VAOs[Triangles]*/);
    //     glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    //     eglSwapBuffers(ctx.display, ctx.surface);
    //     ;
    // };


};