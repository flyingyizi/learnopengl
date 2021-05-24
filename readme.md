
# opengl study：

基于GLAD + GLFW 构建opengl程序



## 渲染过程

![渲染过程](https://learnopengl-cn.github.io/img/01/04/pipeline.png)

- vertex shader :图形渲染管线的第一个部分是顶点着色器(Vertex Shader)，它把一个单独的顶点作为输入。顶点着色器主要的目的是把3D坐标转为另一种3D坐标（后面会解释），同时顶点着色器允许我们对顶点属性进行一些基本处理。

- shape assembly: 图元装配(Primitive Assembly)阶段将顶点着色器输出的所有顶点作为输入（如果是GL_POINTS，那么就是一个顶点），并所有的点装配成指定图元的形状；本节例子中是一个三角形。

- geometry shader: 图元装配阶段的输出会传递给几何着色器(Geometry Shader)。几何着色器把图元形式的一系列顶点的集合作为输入，它可以通过产生新顶点构造出新的（或是其它的）图元来生成其他形状。例子中，它生成了另一个三角形。

- rasterization stage: 几何着色器的输出会被传入光栅化阶段(Rasterization Stage)，这里它会把图元映射为最终屏幕上相应的像素，生成供片段着色器(Fragment Shader)使用的片段(Fragment)。在片段着色器运行之前会执行裁切(Clipping)。裁切会丢弃超出你的视图以外的所有像素，用来提升执行效率。

- fragment shader : 片段着色器的主要目的是计算一个像素的最终颜色，这也是所有OpenGL高级效果产生的地方。通常，片段着色器包含3D场景的数据（比如光照、阴影、光的颜色等等），这些数据可以被用来计算最终像素的颜色。在计算机图形中颜色被表示为有4个元素的数组：红色、绿色、蓝色和alpha(透明度)分量，通常缩写为RGBA。

- alpha and blending :在所有对应颜色值确定以后，最终的对象将会被传到最后一个阶段，我们叫做Alpha测试和混合(Blending)阶段。这个阶段检测片段的对应的深度（和模板(Stencil)）值（后面会讲），用它们来判断这个像素是其它物体的前面

其中“ vertex shader” 与 “fragment shader” 必须由用户提供， GPU中没有默认提供。

### 标准设备坐标 与屏幕空间坐标


一旦你的顶点坐标已经在顶点着色器中处理过，它们就应该是标准化设备坐标(Normalized Device Coordinates, NDC)。 标准化设备坐标是一个x、y和z值在-1.0到1.0的一小段空间。

屏幕空间坐标(Screen-space Coordinates)，这是使用你通过glViewport函数提供的数据，进行视口变换(Viewport Transform)完成的。所得的屏幕空间坐标又会被变换为片段输入到片段着色器中。

### VAO, VBO, EBO

- 顶点数组对象：Vertex Array Object，VAO
- 顶点缓冲对象：Vertex Buffer Object，VBO , 对应对象类型是GL_ARRAY_BUFFER
- 索引缓冲对象：Element Buffer Object，EBO或Index Buffer Object，IBO， 对应对象类型是GL_ELEMENT_ARRAY_BUFFER

### OPENGL如何解释GPU内存中的数据

OpenGL还不知道它该如何解释内存中的顶点数据，以及它该如何将顶点数据链接到顶点着色器的属性上。我们需要告诉OpenGL怎么做。

当我们特别谈论到顶点着色器的时候，每个输入变量也叫顶点属性(Vertex Attribute)

如果每个顶点对应一个顶点属性，每个顶点属性具有3个components(分量)，即VBO 数据希望被解析为下面的样子：
![](https://learnopengl-cn.github.io/img/01/04/vertex_attribute_pointer.png)

我们希望opengl按照上面的样子解析VBO数据，则通过`glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);`指令告诉opengl。

```c++
/**
Parameters
- index :Specifies the index of the generic vertex attribute to be modified.
        它是vertx-glsl中要填充属性(vertex attribute)的index
- size :Specifies the number of components per generic vertex attribute. Must be 1, 2, 3, 4. Additionally, the symbolic constant GL_BGRA is accepted by glVertexAttribPointer. The initial value is 4.
        
- type : Specifies the data type of each component in the array. The symbolic constants GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, and GL_UNSIGNED_INT are accepted by glVertexAttribPointer and glVertexAttribIPointer. Additionally GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_FIXED, GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV and GL_UNSIGNED_INT_10F_11F_11F_REV are accepted by glVertexAttribPointer. GL_DOUBLE is also accepted by glVertexAttribLPointer and is the only token accepted by the type parameter for that function. The initial value is GL_FLOAT.

- normalized :For glVertexAttribPointer, specifies whether fixed-point data values should be normalized (GL_TRUE) or converted directly as fixed-point values (GL_FALSE) when they are accessed.

- stride :Specifies the byte offset between consecutive generic vertex attributes. If stride is 0, the generic vertex attributes are understood to be tightly packed in the array. The initial value is 0.

- pointer :Specifies a offset of the first component of the first generic vertex attribute in the array in the data store of the buffer currently bound to the GL_ARRAY_BUFFER target. The initial value is 0.
*/
void glVertexAttribPointer(	GLuint index,
 	GLint size,
 	GLenum type,
 	GLboolean normalized,
 	GLsizei stride,
 	const void * pointer);
```

例子：
```c++
GLfloat data[] = {
  //position   //color        //texcoords
  1.0, 0.0,    0.5,0.5,0.5,   0.5,0.5,   
  0.0, 0.0,    0.2,0.8,0.0,   0.0,1.0,   
}

// 对应我们在GPU vert-glsl中定义的属性是
// #version 100
// attribute highp vec2 aPosition;
// attribute highp vec3 aColor;
// attribute highp vec2 aTexcoord;

// 我们需要下面的语句告诉gpu应该如何来解析
glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE,
                      7*sizeof(GLfloat), (GLvoid*)0);
glEnableVertexAttribArray(0);

glVertexAttribPointer(1, 3, GL_FLOAT,GL_FALSE,
                      7*sizeof(GLfloat), (GLvoid*)(2*sizeof(GLfloat)));
glEnableVertexAttribArray(1);

glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE,
                      7*sizeof(GLfloat), (GLvoid*)(5*sizeof(GLfloat)));
glEnableVertexAttribArray(2);                    
                      
```


## Textures纹理

我们已经了解到，我们可以为每个顶点添加颜色来增加图形的细节，从而创建出有趣的图像。但是，如果想让图形看起来更真实，我们就必须有足够多的顶点，从而指定足够多的颜色。这将会产生很多额外开销，因为每个模型都会需求更多的顶点，每个顶点又需求一个颜色属性。

推荐使用纹理，因为它可以让物体非常精细而不用指定额外的顶点。

例如对三角形上要贴个纹理：为了能够把纹理映射(Map)到三角形上，我们需要指定三角形的每个顶点各自对应纹理的哪个部分。这样每个顶点就会关联着一个纹理坐标(Texture Coordinate)，用来标明该从纹理图像的哪个部分采样（译注：采集片段颜色）。之后在图形的其它片段上进行片段插值(Fragment Interpolation)。

纹理坐标起始于(0, 0)，也就是纹理图片的左下角，终始于(1, 1)，即纹理图片的右上角。下面的图片展示了我们是如何把纹理坐标映射到三角形上的。

![纹理坐标演示](https://learnopengl-cn.github.io/img/01/06/tex_coords.png)


## 工具类 myglutil.hpp

该工具类对shader创建，顶点数据/纹理数据加载进行了封装，并且通过assimp库的底层支持，能支持第三方的顶点数据。 使用方法见该头文件中的说明。

同时src2中是一个 基于”gtkmm4 + myglutil.hpp“的使用例子。

## 工具类 myeglutil.hpp

使用egl时，比较麻烦的是对各种平台的EGLNativeWindowType，EGLNativeDisplayType获取。这个工具针对win32/linux平台提供了code sample，并且提供了创建上下文的封装。

