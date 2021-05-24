
#ifndef MYGL_UTIL_HPP
#define MYGL_UTIL_HPP

/**
 * 使用说明：
 *  1. 引入该头文件前，应确保有合适的gl头文件，例如`#include <epoxy/gl.h>`, 或`#include <glad/glad.h>`
 *     或任何其他你希望使用的gl
 *  2. 该头文件在任意一个c++ source文件中,添加下面的语句则代表引入了源文件，否则只是起header file作用
 *          #ifdef MYGLUTILITY__IMPLEMENTATION
 *          #include<myglutil.hpp>
 * 3. 准备 shaders::prog_t prog; 成员。
 * 4. 通过prog.load(...)编译shader程序， 并通过prog.use()切换不同shader
 * 5. 准备 shaders::model_t cubeModel 成员。
 * 6. 通过 cubeModel.loadNativeData/load 加载顶点数据。 以后可以在需要渲染时，
 *    调用 cubeModel.Draw(lightingShader_prog)进行渲染
 * 7. 准备 cv::camera_t thecamera 成员。该成员提供了view matrix进行坐标转换
 * 8   
 * **/

/**
 * 坐标转换使用说明：
 * 典型变换过程包括 ` gl_Position = projection * view * model * vec4(aPos, 1.0f);`  
 * 1. model matrix: 这个通过 Eigen::Translation3f， Eigen::AngleAxisf组合就可以，这里没有特意为它们封装
 * 2. view matrix。 提供camera_t类支持。 
 * 3. projection matrix： 提供trans::perspective 方法支持
 * 
 * 
 * **/
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// // // glad, include glad *before* glfw
// #include <glad/glad.h>



#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

//Eigen 中有定义Success，和X11中的定义冲突
#pragma push_macro("Success")
#undef Success
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#pragma pop_macro("Success")


#ifdef _DEBUG
constexpr bool is_debug = true;
#else
constexpr bool is_debug = false;
#endif

      inline GLfloat radians(GLfloat degrees) 
      {
         return degrees * static_cast<GLfloat>(0.01745329251994329576923690768489f);
      };


template <class T, class U>
struct IsSameType
{
    enum
    {
        result = false
    };
};

template <class T>
struct IsSameType<T, T>
{
    enum
    {
        result = true
    };
};

namespace shaders
{
    class prog_t
    {
    public:
        GLuint progid = 0;

        ~prog_t() { this->del_program(); };
        bool use();

        bool loadFromPath(const std::filesystem::path &vert_path, const std::filesystem::path &frag_path);
        bool load(const std::string &vert_content, const std::string &frag_content);

        /**
         * usage example:
         *   lightingShader_prog.use();
         *       {
         *           lightingShader_prog.uniform_set<Eigen::Vector3f>(
         *               {{"light.position", lightPos},
         *               {"material.specular", Eigen::Vector3f(0.5f, 0.5f, 0.5f)}});
         * 
         **/
        template <typename T>
        void uniform_set(std::initializer_list<std::tuple<std::string /*name*/, T>> lists)
        {
            for (auto &v : lists)
            {
                GLuint loc = glGetUniformLocation(this->progid, std::get<0>(v).c_str());
                if (loc == -1)
                {
                    std::cout << std::get<0>(v) << ": uniform is not in program[" << this->progid << "]" << std::endl;
                    continue;
                }
                static_assert(IsSameType<T, Eigen::Matrix4f>::result ||
                                  IsSameType<T, Eigen::Vector3f>::result ||
                                  std::is_integral<T>::value ||
                                  IsSameType<T, GLfloat>().result,
                              "-not supported-");

                if constexpr (IsSameType<T, Eigen::Vector3f>().result)
                {
                    auto &&d = std::get<1>(v);
                    glUniform3f(loc, d.x(), d.y(), d.z());
                }
                else if constexpr (IsSameType<T, Eigen::Matrix4f>::result)
                {
                    glUniformMatrix4fv(loc, 1, GL_FALSE, std::get<1>(v).data());
                }
                else if constexpr (IsSameType<T, GLfloat>::result)
                {
                    glUniform1f(loc, std::get<1>(v));
                }
                else if constexpr (std::is_integral<T>::value)
                {
                    glUniform1i(loc, std::get<1>(v));
                }
            }
            return;
        };

    private:
        GLuint create_link_program(std::string vert_glsl, std::string frag_glsl);
        void del_program();

    private:
        std::basic_string<GLchar> readShader_fromfile(const std::filesystem::path &_path);
    };

    // 顶点数据类型
    enum class vertexType_t
    {
        POSITION_ENU,
        NORMAL_ENU,
        TEXCOORD_ENU,
        TANGENT_ENU,
        BITTANGENT_ENU
    };

    class mesh_t
    {
        
    public:
        struct vertex_t //= struct //__Vertex
        {
            // position
            Eigen::Vector3f Position;
            // normal
            Eigen::Vector3f Normal;
            // texCoords
            Eigen::Vector2f TexCoords;
            // tangent
            Eigen::Vector3f Tangent;
            // bitangent
            Eigen::Vector3f Bitangent;
        };

        struct texture_t //= struct //__Texture
        {
            GLuint id;
            std::string type;
            std::string path;
        };

    public:
        // mesh Data
        std::vector<vertex_t> vertices;
        std::vector<GLuint> indices;
        std::vector<texture_t> textures;
        GLuint VAO;
        // constructor
        mesh_t(const std::vector<vertex_t> &_vertices, const std::vector<GLuint> &_indices,
               const std::vector<texture_t> &_textures);

        // render the mesh
        void Draw(shaders::prog_t &shader);

    private:
        // render data
        GLuint VBO, EBO;

        // initializes all the buffer objects/arrays
        void setupMesh();
    };

    /**
     * 可加载原始数据，或任何assimp支持的规范化模型数据。
     * 加载的顶点数据属性位置，固定为以下属性位置。因此在书写GLSL时填写属性位置，或在其他地方指定属性位置时必须按照
     * 下面的属性位置约定进行编码，该约定固定编码在mesh_t.setupMesh()中： 
     *        0: vertex positon;        1:vertex normals; 
     *        2: vextex texture coords; 3: vertex tangent
     *        4: vertex bitangent
     * 同时约定：上述这些顶点数据，除了`texture coords是vec2外，其他顶点数据都是vec3
    */
    class model_t
    {
        struct boneInfo_t //= struct __BoneInfo
        {
            /*
        	For uniquely indentifying the bone and
		    for indexing bone transformation in shaders
            */
            int id;
            /*
		    map from bone name to offset matrix.
		    offset matrix transforms bone from bone space to local space
	        */
            Eigen::Matrix4f offset;
        };

    public:
        // model data
        std::vector<mesh_t::texture_t> textures_loaded; // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
        std::vector<mesh_t> meshes;
        std::string directory;
        bool gammaCorrection;

        // constructor, expects a filepath to a 3D model.
        model_t(bool gamma = false) : gammaCorrection(gamma){};

        // draws the model, and thus all its meshes
        void Draw(shaders::prog_t &shader);

        auto &GetOffsetMatMap() { return m_OffsetMatMap; }
        int &GetBoneCount() { return m_BoneCount; }

        // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
        void loadModel(const std::filesystem::path &path);
        /**
        * 加载, 绑定原始vbo数据到model。
        * cols: vertices 每行代表一个vertex.列数由cols传递
        *                一个vertex可能包含一个或多个vertex attrib
        * howto_do: 通过输入howto_do信息告诉gpu如何分析每个vertex attrib
        * 例如， vertexNative的数据应按照下面解释
        *       // positions          // normals           // texture coords
        *       -0.5f,-0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
        *       0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
        *       ...
        *      对应构造的howto_do应是如下数据
        *      {
        *          {shaders::vertexType_t::POSITION_ENU, 0},
        *          {shaders::vertexType_t::NORMAL_ENU, 3},
        *          {shaders::vertexType_t::TEXCOORD_ENU, 6},
        *      },
        * */
        using vertexAttrib_t = std::initializer_list<std::tuple<vertexType_t /*顶点数据类型*/,
                                                                long /*该顶点数据在一行中的起始位置*/>>;
        bool loadNativeData(int cols,
                            vertexAttrib_t howto_do,
                            const std::vector<GLfloat> &vertexNative,
                            std::initializer_list<std::tuple<aiTextureType /*纹理类型*/,
                                                             std::string /*纹理图片路径*/>>
                                _texturesNative = {},
                            const std::vector<GLuint> &_indices = std::vector<GLuint>{});

    private:
        std::unordered_map<std::string, boneInfo_t> m_OffsetMatMap;
        int m_BoneCount = 0;

        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats
        // this process on its children nodes (if any).
        void processNode(aiNode *node, const aiScene *scene);

        void SetVertexBoneDataToDefault(mesh_t::vertex_t &vertex);

        mesh_t processMesh(aiMesh *mesh, const aiScene *scene);

        void SetVertexBoneData(mesh_t::vertex_t &vertex, int boneID, float weight);

        void ExtractBoneWeightForVertices(std::vector<mesh_t::vertex_t> &vertices,
                                          aiMesh *mesh, const aiScene *scene);

        unsigned int TextureFromFile(const std::string &filename,
                                     const std::string &directory, bool gamma = false);
        // checks all material textures of a given type and loads the textures if they're not loaded yet.
        // the required info is returned as a Texture struct.
        std::vector<mesh_t::texture_t> loadMaterialTextures(aiMaterial *mat,
                                                            aiTextureType type, std::string typeName);
    };

};

namespace trans
{
    // 通过反射关系，从入射/发现求出反射向量
    // lightdir : 入射向量， 必须是单位向量, 光源 ->反射点
    // normal :  法线 。  必须是单位向量
    // 结果：  反射向量
    Eigen::Vector3f reflect(Eigen::Vector3f lightdir, Eigen::Vector3f normal);
    // eigen 实现 右手系范围是[-1,1] glm:: perspective 函数，"返回结果.data()" 作为传递uniform的参数
    // 例子: auto projection = trans::perspective<GLfloat>(...);
    //       glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, projection.data());
    //
    // fovy : Field of View,视野，通常设置为45度
    // aspect： 视口宽高比
    // near : 就是投影面，其值是投影面距离视点的距离，
    // far  : 是视景体的后截面，其值是后截面距离视点的距离。far 和 near 的差值，就是视景体的深度。
    Eigen::Matrix<GLfloat, 4, 4> perspective(GLfloat fovy, GLfloat aspectRatio, GLfloat zNear, GLfloat zFar);
    //正交变换
    Eigen::Matrix<GLfloat, 4, 4, Eigen::RowMajor> ortho(GLfloat left,
                                                        GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

};

namespace cv
{
    // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
    enum class Camera_Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    // eye: 在世界坐标中摄像机位置. 眼睛在哪儿
    // center: 在世界坐标中场景原点. 、看哪儿
    // up:  在世界坐标中上向量(Up Vector). 头顶朝哪儿
    //
    // eigen 实现 glm:: lookAt 函数，"返回结果.data()" 作为传递uniform的参数
    // 例子: auto view_eigen=trans::lookAt( Eigen::Vector3f(camX, 0.0f, camZ),... );
    //       glUniformMatrix4fv(glGetUniformLocation(prog, "view"), 1, GL_FALSE,  view_eigen.data());
    Eigen::Matrix<GLfloat, 4, 4> lookAt(const Eigen::Vector3f &eye,
                                        const Eigen::Vector3f &center, const Eigen::Vector3f &up);

    // An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
    class camera_t
    {
        constexpr static GLfloat YAW = -90.0f;
        constexpr static GLfloat PITCH = 0.0f;
        constexpr static GLfloat SPEED = 5.5f;
        constexpr static GLfloat SENSITIVITY = 0.1f;
        constexpr static GLfloat ZOOM = 45.0f;

    public:
        // camera Attributes
        Eigen::Vector3f Position;
        Eigen::Vector3f Front;
        Eigen::Vector3f Up;
        Eigen::Vector3f Right;
        Eigen::Vector3f WorldUp;
        // euler Angles
        GLfloat Yaw, Pitch; // 是角度值，非弧度值
        // camera options
        GLfloat MovementSpeed = SPEED;
        GLfloat MouseSensitivity = SENSITIVITY;
        GLfloat Zoom = ZOOM;

        // constructor with vectors
        camera_t(Eigen::Vector3f position = Eigen::Vector3f(0.0f, 0.0f, 0.0f),
                 Eigen::Vector3f up = Eigen::Vector3f(0.0f, 1.0f, 0.0f),
                 GLfloat yaw = YAW, GLfloat pitch = PITCH);

        // returns the view matrix calculated using Euler Angles and the LookAt Matrix
        Eigen::Matrix<GLfloat, 4, 4> GetViewMatrix();

        // processes input received from any keyboard-like input system. Accepts input parameter in the form
        // of camera defined ENUM (to abstract it from windowing systems)
        void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);

        // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
        void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);

        // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
        void ProcessMouseScroll(GLfloat yoffset);

    private:
        // calculates the front vector from the Camera's (updated) Euler Angles
        void updateCameraVectors();
    };

};

namespace assimp_helper
{
    static inline Eigen::Matrix4f ConvertAiMatrixToEigenFormat(const aiMatrix4x4 &from)
    {
        Eigen::Matrix4f to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to << from.a1, from.a2, from.a3, from.a4,
            from.b1, from.b2, from.b3, from.b4,
            from.c1, from.c2, from.c3, from.c4,
            from.d1, from.d2, from.d3, from.d4;
        return to;
    }

};

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(MYGLUTILITY__IMPLEMENTATION) 

// Eigen::MatrixXd
namespace shaders
{
    bool prog_t::use()
    {
        if (this->progid == 0)
            return false;

        glUseProgram(this->progid);
        return true;
    };

    bool prog_t::loadFromPath(const std::filesystem::path &vert_path, const std::filesystem::path &frag_path)
    {
        // reload, delete previous
        if (this->progid != 0)
        {
            glDeleteProgram(this->progid);
        }

        auto v_str = readShader_fromfile(vert_path);
        if (v_str.size() == 0)
            return false;
        auto f_str = readShader_fromfile(frag_path);
        if (v_str.size() == 0)
            return false;

        return load(v_str, f_str);
    };
    bool prog_t::load(const std::string &vert_content, const std::string &frag_content)
    {
        this->progid = create_link_program(vert_content, frag_content);
        if (this->progid == 0)
            return false;

        return true;
    };

    GLuint prog_t::create_link_program(std::string vert_glsl, std::string frag_glsl)
    {
        //helper
        struct _shaderInfo
        {
            GLenum type;
            std::string shaderstr;
            GLuint shader;
        };

        auto LoadShaders = [](std::vector<_shaderInfo> &shaders) -> GLuint
        {
            if (shaders.empty() == true)
                return 0;

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
                    // if constexpr (is_debug == true)
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
            }

            GLuint prog = glCreateProgram();
            if (prog == 0)
            {
                std::cout << "glCreateProgram fail. " << __func__ << std::endl;
                return 0;
            }
            for (auto iter = shaders.begin(); iter != shaders.end(); iter++)
            {
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
                // if constexpr (is_debug == true)
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

        auto shaders = std::vector<_shaderInfo>{
            {GL_VERTEX_SHADER, vert_glsl}, {GL_FRAGMENT_SHADER, frag_glsl}};

        GLuint program = LoadShaders(shaders);
        return program;
    };
    void prog_t::del_program()
    {
        glDeleteProgram(this->progid);
    };

    std::basic_string<GLchar> prog_t::readShader_fromfile(const std::filesystem::path &_path)
    {
        std::basic_string<GLchar> out;
        std::ifstream file(_path);
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

};

namespace trans
{
    // 通过反射关系，从入射/发现求出反射向量
    // lightdir : 入射向量， 必须是单位向量, 光源 ->反射点
    // normal :  法线 。  必须是单位向量
    // 结果：  反射向量
    Eigen::Vector3f reflect(Eigen::Vector3f lightdir, Eigen::Vector3f normal) //得到反射方向
    {
        // L     N     R
        //       |
        //      /|
        //  \  / |    /
        //   \/  |   /
        //    \  |  /
        //     \ | /
        // -----\|/----------------------
        // 构造菱形
        lightdir = -lightdir;
        Eigen::Vector3f result;
        auto cos_theta = lightdir.dot(normal);
        result = (2 * cos_theta * normal - lightdir);
        result.normalize();
        return result;
    }

    // eigen 实现 右手系范围是[-1,1] glm:: perspective 函数，"返回结果.data()" 作为传递uniform的参数
    // 例子: auto projection = trans::perspective<GLfloat>(...);
    //       glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, projection.data());
    //
    // fovy : Field of View,视野，通常设置为45度
    // aspect： 视口宽高比
    // near : 就是投影面，其值是投影面距离视点的距离，
    // far  : 是视景体的后截面，其值是后截面距离视点的距离。far 和 near 的差值，就是视景体的深度。
    Eigen::Matrix<GLfloat, 4, 4> perspective(GLfloat fovy, GLfloat aspectRatio, GLfloat zNear, GLfloat zFar)
    {
        Eigen::Matrix<GLfloat, 4, 4> Result;
        //[formula](http://openglbook.com/the-book/chapter-4-entering-the-third-dimension/)

        //     [ xScale   0                 0                        0               ]
        // P = [   0    yScale              0                        0               ]
        //     [   0      0    -(zFar+zNear)/(zFar-zNear) -2*zNear*zFar/(zFar-zNear) ]
        //     [   0      0                -1                        0               ]
        // yScale = cot(fovY/2)
        // xScale = yScale/aspectRatio
        auto yScale = 1.0f / std::tan(fovy / 2.0f);
        auto xScale = yScale / aspectRatio;
        Result << xScale, 0, 0, 0,
            0, yScale, 0, 0,
            0, 0, -(zFar + zNear) / (zFar - zNear), -2 * zNear * zFar / (zFar - zNear),
            0, 0, -1, 0;
        return Result;

        // GLfloat const tanHalfFovy = std::tan(fovy / 2.0f);
        // Result = Eigen::Matrix<GLfloat, 4, 4>::Zero();
        // Result(0, 0) = 1.0f / (aspect * tanHalfFovy);
        // Result(1, 1) = 1.0f / (tanHalfFovy);
        // Result(2, 2) = -(zFar + zNear) / (zFar - zNear);
        // Result(2, 3) = -1.0f;
        // Result(3, 2) = -(2.0f * zFar * zNear) / (zFar - zNear);
        // return Result.transpose();
    };
    //正交变换
    Eigen::Matrix<GLfloat, 4, 4, Eigen::RowMajor> ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
    {
        Eigen::Matrix<GLfloat, 4, 4, Eigen::RowMajor> Result;
        Result = Eigen::Matrix<GLfloat, 4, 4, Eigen::RowMajor>::Identity();
        Result(0, 0) = 2.0f / (right - left);
        Result(1, 1) = 2.0f / (top - bottom);
        Result(2, 2) = -2.0f / (zFar - zNear);
        Result(0, 3) = -(right + left) / (right - left);
        Result(1, 3) = -(top + bottom) / (top - bottom);
        Result(2, 3) = -(zFar + zNear) / (zFar - zNear);
        return Result;
    };

};

namespace cv
{
    // eye: 在世界坐标中摄像机位置. 眼睛在哪儿
    // center: 在世界坐标中场景原点. 、看哪儿
    // up:  在世界坐标中上向量(Up Vector). 头顶朝哪儿
    //
    // eigen 实现 glm:: lookAt 函数，"返回结果.data()" 作为传递uniform的参数
    // 例子: auto view_eigen=trans::lookAt( Eigen::Vector3f(camX, 0.0f, camZ),... );
    //       glUniformMatrix4fv(glGetUniformLocation(prog, "view"), 1, GL_FALSE,  view_eigen.data());
    Eigen::Matrix<GLfloat, 4, 4> lookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up)
    {
        // 得到相机坐标系(u-v-n) ,该坐标系是左手坐标系统。而opgen相机坐标系是右手系
        Eigen::Vector3f n(center - eye); // n 代表 opgen相机坐标系"+Z轴"反向
        n.normalize();

        Eigen::Vector3f u(n.cross(up)); // u 代表 opgen相机坐标系"+X轴"
        u.normalize();

        Eigen::Vector3f v(u.cross(n)); //  v 代表 opgen相机坐标系"+Y轴"
        v.normalize();

        Eigen::Matrix<GLfloat, 4, 4> Result;
        Result << u.x(), u.y(), u.z(), -u.dot(eye),
            v.x(), v.y(), v.z(), -v.dot(eye),
            -n.x(), -n.y(), -n.z(), n.dot(eye),
            0, 0, 0, 1;
        return Result;
    };

    // constructor with vectors
    camera_t::camera_t(Eigen::Vector3f position,
                       Eigen::Vector3f up,
                       GLfloat yaw, GLfloat pitch)
    {
        Front = Eigen::Vector3f(0.0f, 0.0f, -1.0f);
        Position = position;

        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    Eigen::Matrix<GLfloat, 4, 4> camera_t::GetViewMatrix()
    {
        return lookAt(Position, Eigen::Vector3f(Position + Front), Up);
    };

    // processes input received from any keyboard-like input system. Accepts input parameter in the form
    // of camera defined ENUM (to abstract it from windowing systems)
    void camera_t::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        // std::cout << "backward" << std::endl;
        GLfloat velocity = MovementSpeed * deltaTime;
        if (direction == Camera_Movement::FORWARD)
        {
            Position += Front * velocity;
        }
        else if (direction == Camera_Movement::BACKWARD)
        {
            Position -= Front * velocity;
        }
        else if (direction == Camera_Movement::LEFT)
        {
            Position -= Right * velocity;
        }
        else if (direction == Camera_Movement::RIGHT)
        {
            Position += Right * velocity;
        }
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void camera_t::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        this->Yaw += xoffset;
        this->Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void camera_t::ProcessMouseScroll(GLfloat yoffset)
    {
        Zoom -= (GLfloat)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    // calculates the front vector from the Camera's (updated) Euler Angles
    void camera_t::updateCameraVectors()
    {
        auto radians = [](GLfloat degrees) -> GLfloat
        {
            return degrees * static_cast<GLfloat>(0.01745329251994329576923690768489f);
        };

        auto eulerAngle_to_vector = [](GLfloat pitch /*俯仰角*/, GLfloat yaw /*偏航角*/) -> Eigen::Vector3f
        {
            //右手坐标系
            auto radians = [](GLfloat v) -> GLfloat
            {
                return (v * M_PI) / 180;
            };
            pitch = radians(pitch);
            yaw = radians(yaw);
            //
            Eigen::Vector3f front;
            front.y() = (std::sin(pitch));

            front.x() = (std::cos(yaw) * std::cos(pitch));
            front.z() = (std::sin(yaw) * std::cos(pitch));
            front.normalize();
            return front;
        };

        this->Front = eulerAngle_to_vector(this->Pitch, this->Yaw);

        // u cross v:  cross product 含义是u、v 所构成平面的法向
        // also re-calculate the Right and Up vector
        auto c = this->Front.cross(this->WorldUp);
        c.normalize();
        this->Right = c; // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        auto cc = this->Right.cross(this->Front);
        cc.normalize();
        this->Up = cc;
    }

};

namespace shaders
{

    // constructor
    mesh_t::mesh_t(const std::vector<vertex_t> &_vertices, const std::vector<GLuint> &_indices, const std::vector<texture_t> &_textures)
    {
        this->vertices = _vertices;
        this->indices = _indices;
        this->textures = _textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void mesh_t::Draw(shaders::prog_t &shader)
    {
        // bind appropriate textures
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        GLuint normalNr = 1;
        GLuint heightNr = 1;
        for (unsigned int i = 0; i < this->textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string &name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to stream
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to stream
            else if (name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to stream

            // now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.progid, (name + number).c_str()), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // draw mesh
        glBindVertexArray(this->VAO);
        if (this->indices.size() == 0)
        {
            glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
        }
        else
        {
            glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

    // initializes all the buffer objects/arrays
    void mesh_t::setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &(this->VAO));
        glGenBuffers(1, &(this->VBO));
        glGenBuffers(1, &(this->EBO));

        glBindVertexArray(this->VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex_t), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, Bitangent));

        glBindVertexArray(0);
    }

    // draws the model, and thus all its meshes
    void model_t::Draw(shaders::prog_t &shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void model_t::loadModel(const std::filesystem::path &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
        // check for errors
        if (nullptr == scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        auto absolute = std::filesystem::absolute(path);
        this->directory = absolute.parent_path().string();

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    bool model_t::loadNativeData(int cols,
                                 vertexAttrib_t howto_do,
                                 const std::vector<GLfloat> &vertexNative,
                                 std::initializer_list<std::tuple<aiTextureType /*纹理类型*/,
                                                                  std::string /*纹理图片路径*/>>
                                     _texturesNative,
                                 const std::vector<GLuint> &_indices)
    {
        //check
        if (vertexNative.size() % cols != 0)
        {
            std::cout << "invalid cols occours when calling " << __func__ << std::endl;
            return false;
        }
        for (auto &&t : howto_do)
        {
            if (std::get<1>(t) > cols)
            {
                std::cout << "invalid howto_do" << std::endl;
                return false;
            }
        }

        long end_ind = 0;
        for (auto &&t : howto_do)
        {
            auto _t_type = std::get<0>(t);
            auto ind = std::get<1>(t);
            switch (_t_type)
            {
            case vertexType_t::POSITION_ENU:
            case vertexType_t::NORMAL_ENU:
            case vertexType_t::TANGENT_ENU:
            case vertexType_t::BITTANGENT_ENU:
                end_ind = std::max(end_ind, ind + 3);
                break;
            case vertexType_t::TEXCOORD_ENU:
                end_ind = std::max(end_ind, ind + 2);
                break;
            default:;
            }
        }
        if (end_ind > cols)
        {
            std::cout << "invalid howto_do occours when calling " << __func__ << std::endl;
            return false;
        }

        const auto rows = vertexNative.size() / cols;

        std::vector<mesh_t::vertex_t> vertices;
        {
            for (int i = 0; i < rows; i++)
            {
                mesh_t::vertex_t v;

                auto start_i = i * cols;

                for (auto &&t : howto_do)
                {
                    auto oft = start_i + std::get<1>(t);
                    auto m = Eigen::Vector3f(vertexNative[oft], vertexNative[oft + 1], vertexNative[oft + 2]);
                    switch (std::get<0>(t))
                    {
                    case vertexType_t::POSITION_ENU:
                        v.Position = m;
                        break;
                    case vertexType_t::NORMAL_ENU:
                        v.Normal = m;
                        break;
                    case vertexType_t::TEXCOORD_ENU:
                        v.TexCoords = m.head(2);
                        break;
                    case vertexType_t::TANGENT_ENU:
                        v.Tangent = m;
                        break;
                    case vertexType_t::BITTANGENT_ENU:
                        v.Bitangent = m;
                        break;
                    default:
                        break;
                    }
                }
                vertices.push_back(v);
            }
        }

        std::vector<mesh_t::texture_t> textures;
        {
            auto aiTextureType_to_str = [](aiTextureType type_) -> std::string
            {
                switch (type_)
                {
                case aiTextureType_DIFFUSE:
                    return "texture_diffuse";
                    break;
                case aiTextureType_SPECULAR:
                    return "texture_specular";
                    break;
                case aiTextureType_HEIGHT:
                    return "texture_normal";
                    break;
                case aiTextureType_AMBIENT:
                    return "texture_height";
                    break;
                default:
                    return "";
                }
            };
            //todo check whether already loaded in textures_loaded
            for (auto &&text : _texturesNative)
            {
                mesh_t::texture_t t;
                t.type = aiTextureType_to_str(std::get<0>(text));
                auto pth = std::filesystem::path(std::get<1>(text));
                t.path = pth.string();
                if (std::filesystem::is_directory(pth))
                {
                    std::cout << t.path << ": is a directory. not a valid filepath" << std::endl;
                    continue;
                }
                t.id = this->TextureFromFile(pth.filename().string(), pth.parent_path().string());
                textures.push_back(t);
            }
        }

        auto mesh = mesh_t(vertices, _indices, textures);
        this->meshes.push_back(mesh);
        return true;
    };

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats
    // this process on its children nodes (if any).
    void model_t::processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    void model_t::SetVertexBoneDataToDefault(mesh_t::vertex_t &vertex)
    {
        // todo
        // for (int i = 0; i < MAX_BONE_WEIGHTS; i++)
        // {
        //     vertex.m_BoneIDs[i] = -1;
        //     vertex.m_Weights[i] = 0.0f;
        // }
    }

    mesh_t model_t::processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<mesh_t::vertex_t> vertices;
        std::vector<unsigned int> indices;
        std::vector<mesh_t::texture_t> textures;

        auto getVec = [](const aiVector3D &vec) -> Eigen::Vector3f
        {
            return Eigen::Vector3f(vec.x, vec.y, vec.z);
        };

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            mesh_t::vertex_t vertex;
            SetVertexBoneDataToDefault(vertex);
            vertex.Position = getVec(mesh->mVertices[i]);
            vertex.Normal = getVec(mesh->mNormals[i]);

            if (mesh->mTextureCoords[0])
            {
                vertex.TexCoords << mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y;
            }
            else
            {
                vertex.TexCoords << 0.0f, 0.0f;
            }
            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<mesh_t::texture_t> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<mesh_t::texture_t> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<mesh_t::texture_t> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        std::vector<mesh_t::texture_t> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        return mesh_t(vertices, indices, textures);
    }

    void model_t::SetVertexBoneData(mesh_t::vertex_t &vertex, int boneID, float weight)
    {
        //todo
        // for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
        // {
        //     if (vertex.m_BoneIDs[i] < 0)
        //     {
        //         vertex.m_Weights[i] = weight;
        //         vertex.m_BoneIDs[i] = boneID;
        //         break;
        //     }
        // }
    }

    void model_t::ExtractBoneWeightForVertices(std::vector<mesh_t::vertex_t> &vertices, aiMesh *mesh, const aiScene *scene)
    {
        auto &boneInfoMap = m_OffsetMatMap;
        int &boneCount = m_BoneCount;

        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfo_t newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = assimp_helper::ConvertAiMatrixToEigenFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo;
                boneID = boneCount;
                boneCount++;
            }
            else
            {
                boneID = boneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    unsigned int model_t::TextureFromFile(const std::string &filename,
                                          const std::string &directory, bool gamma)
    {
        auto path = std::filesystem::path(directory);
        path /= filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path.string().c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<mesh_t::texture_t> model_t::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<mesh_t::texture_t> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            { // if texture hasn't been loaded already, load it
                mesh_t::texture_t texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

};
#endif //#ifdef  MYGLUTILITY__IMPLEMENTATION 

#endif