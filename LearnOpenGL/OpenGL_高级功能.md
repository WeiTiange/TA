# OpenGL高级功能
#深度测试 #模板测试 #混合 #面剔除 

#### 深度测试(Depth Test)
- ###### 深度缓冲
	- 深度缓冲用于储存每个片段的**深度值**， 当深度测试(Depth Testing)开启时，OpenGL会将当前渲染片段的深度值与深度缓冲里的深度值进行对比，执行深度测试，如果如果测试通过，深度缓冲的值会更新为新的深度值，如果失败则该片段会被舍弃
	- 深度缓冲又窗口系统自动创建，以16、24或32位的float的形式储存他的深度值。**大部分系统中深度缓冲的精度是24位**
	- 深度缓冲在**片段着色器以及模板测试运行之后**，在**屏幕空间**中运行的
		- 屏幕空间坐标可以在片段着色器中通过gl_FragCoord直接访问
		- gl_FragCoord的x，y分量分别为片段的屏幕空间坐标 ((0,0)为左下角)。而z分量为片段的深度值，即需要与深度测试中需要的值
	- *提前深度测试(Early Depth Testing)：现在大部分GPU都提供的硬件特性，允许深度测试在片段着色器之前运行。当使用提前深度测试时，片段着色器无法写入深度值* 
- ###### 深度测试设置
	- 深度测试默认禁用，需要通过GL_DEPTH_TEST来启用
		- <mark>glEnable(GL_DEPTH_TEST);</mark>
	- 开启深度测试后，应该在每次渲染迭代前(while循环的一开始)清除深度缓冲，否则会使用上一次渲染迭代写入的深度值与当前渲染迭代的片段的深度值进行对比
		- <mark>glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);</mark>
	- 执行深度测试并丢弃相应的片段，但是**不**更新深度缓冲，即使用一个只读(Read-Only)深度缓冲。禁用深度缓冲的写入只需要设置他的深度掩码(Depth Mask)为GL_FALSE即可。**只有在开启深度测试时才有效**
		- <mark>glDepthMask(GL_FALSE)</mark>
- ###### 深度测试函数
	- 深度测试的比较运算符是可配置的。可以用来控制OpenGL什么时候该通过或丢弃一个片段，什么时候该更新深度缓冲。通过glDepthFunc来设置
		- <mark>glDepthFunc(GL_LESS)</mark>
	- 默认情况下使用GL_LESS，他会丢弃深度值大于等于当前深度缓冲值的所有片段

| 函数        | 描述                                       |
| ----------- | ------------------------------------------ |
| GL_ALWAYS   | **永远**通过深度测试                           |
| GL_NEVER    | **永远不**通过深度测试                         |
| GL_LESS     | 在片段深度值**小于**缓冲的深度值时通过测试     |
| GL_EQUAL    | 在片段深度值**等于**缓冲的深度值时通过测试     |
| GL_LEQUAL   | 在片段深度值**小于等于**缓冲的深度值时通过测试 |
| GL_GREATER  | 在片段深度值**大于**缓冲的深度值时通过测试     |
| GL_NOTEQUAL | 在片段深度值**不等于**缓冲的深度值时通过测试   |
| GL_GEQUAL   | 在片段深度值**大于等于**缓冲的深度值时通过测试                                           |
*注：越小的深度值意味着离摄像机越近*

- ###### 深度值精度
	- 深度值介于0到1之间。观察空间的z值可能是视锥体的近平面和远平面之间的任何值，需要把z值变换到\[0,1]的范围之间
		- **线性**
			- ![[OpenGL_深度值线性变换.png]]
				- near、far是在投影矩阵里设置的near和far值
			- ![[OpenGL_深度值线性图.png]]
			- *实践中几乎永远不会使用，因为线性的方程使得远处的物体和近处的物体使用同样的精度，会导致在物体距离摄像机很近的时候精准度不够*
		- **非线性**
			- 非线性方程是透视矩阵默认使用的方程
			- 与1/z成正比，在z值很小的时候提供非常高的精度，而在z值很远的时候提供更少的精度
			- ![[OpenGL_深度值非线性方程.png]]
			- ![[OpenGL_深度值非线性变换.png]]
- ###### 深度值的可视化
	- 由于使用非线性方程的关系(越近的片段精度越高，越远的越低)，片段的深度值会随着距离的增加而快速增加，从而使得大部分片段的颜色都趋近于白色，只有里相机非常近的片段才会显示灰色
		- ![[OpenGL_深度值非线性方程可视化.png]]
	- 将非线性方程的深度值变换为线性的深度值可以正确的可视化深度值
		- 先将深度值从\[0,1]范围重新变换到\[-1,1]范围的标准化设备坐标(裁剪空间)
			- <mark>float z = depth \* 2.0 - 1.0;</mark>
		- 然后使用获取到的z值，应用逆变换来获取线性的深度值
			- <mark>float linearDepth = (2.0 * near * far) / (far + near - z * (far - near));</mark>
		- 最后在main函数中将变换为线性的深度值除以far以正确的可视化深度值
			- <mark>float depth = linearizeDepth(gl_FragCoord.z)/far;</mark>
- ###### 深度冲突
	- 如果两个三角形或平面离得特别近，并且是平行的，深度缓冲会没有足够的精度来决定他们俩谁应该在上面，结果就是两个三角形不断地切换前后顺序，这个现象叫**深度冲突(Z-fighting)** 。当物体在远处时更明显 (因为非线性方程在远处的精度更是低上加低)。
	- 深度冲突不能完全避免，但可以通过技巧来减轻/规避
		- 永远不要吧多个物体摆的太靠近，以至于他们的一些三角形会重叠
		- 尽可能将近平面设置的远一些 (离摄像机更远)，越远的近平面会使得视锥体内的整体深度值精度越高。但是近平面太远会导致近处的物体被裁剪掉，需要微调来找到合适的近平面距离
		- 使用更高精度的深度缓冲，代价是牺牲掉一些性能。大部分深度缓冲的精度都是24位的，但是大部分的显卡都支持32位的深度缓冲，32位精度可以极大地提高精度。

#### 模板测试(Stencil Test)
- ###### 模板缓冲
	- 片段着色器处理完一个片段后会进行模板测试(Stencil Test)。模板测试和深度测试一样，都有可能会丢弃片段，被保留下来的片段会进行深度测试。
	- 模板测试使用模板缓冲(stencil buffer)来进行测试
		- 通常模板缓冲中每个模板值(stencil value)是8位的，所以每个像素/片段一共能有256种不同的模板值
		- *每个窗口库都需要配置一个模板缓冲，GLFW自动做了这件事，但是其他的库可能不会*
	- **模板测试的步骤**
		- 启用模板缓冲的写入
		- 渲染物体，更新模板缓冲的内容
		- 禁用模板缓冲的写入
		- 渲染(其他)物体，这次根据模板缓冲中已绘制的其他物体的片段来决定是否丢弃特定的片段
- ###### 模板测试设置
	- 通过GL_STENCIL_TEST来启用模板测试。开启后所有的渲染调用都会以某种方式影响着模板缓冲
		- <mark>glEnable(GL_STENCIL_TEST)</mark>
	- 在每次渲染迭代的开始清除模板缓冲，否则将会使用上次迭代的数据
		- <mark>glClear(GL_STENCIL_BUFFER_BIT)</mark>
	- 可以通过glStencilMask来关闭模板缓冲的写入，默认情况下设置的掩码所有位都为1，不影响输出，但如果设置为0X00，则写入缓冲的所有模板值都会变成0，和深度测试中的glDepthMask(GL_FALSE)一样
		- <mark>glStencilMask(0xFF); // 每一位写入模板缓冲时都保持原样 </mark>
		- <mark>glStencilMask(0x00); // 每一位在写入模板缓冲时都会变成0（禁用写入）</mark>
			- **禁用写入不仅会禁止模板缓冲的写入，还会阻止模板缓冲清空(使glClear(GL_STENCIL_BUFFER_BIT)无效)**
		- *大部分情况下只会使用0x00或者0xFF作为模板掩码(Stencil Mask)，但是可以自定义*
- ###### 模板测试函数
	- 有两个函数可以用来配置模板测试，让我们对模板缓冲应该通过还是失败，以及他应该如何影响模板缓冲有一定的控制
		- <mark>glStencilFunc(GLenum func, GLint ref, GLuint mask)</mark>
			- func：设置模板测试函数(Stencil Test Function)。这个测试函数将会应用到已储存的模板值上和glStencilFunc函数的**ref**值上
			- ref：设置了模板测试的参考值(Reference Value)。模板缓冲的内容将会与这个值进行比较
			- mask：设置一个掩码，它将会与参考值和储存的模板值在测试比较它们之前进行与(AND)运算。初始情况下所有位都为1
		- <mark>glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)</mark>
			- sfail：模板测试失败时采取的行为
			- dpfail：模板测试通过，但深度测试失败时采取的行为
			- dppass：模板测试和深度测试都通过时采取的行为

| 模板测试函数 | 描述                                             |
| ------------ | ------------------------------------------------ |
| GL_ALWAYS    | **永远**通过模板测试                                             |
| GL_NEVER     | **永远不**通过模板测试                           |
| GL_LESS      | 在片段模板值**小于**缓冲的模板值时通过测试       |
| GL_LEQUAL    | 在片段的模板值**小于等于**缓冲的模板值时通过测试 |
| GL_GREATER   | 在片段的模板值**大于缓冲**的模板值时通过测试     |
| GL_GEQUAL    | 在片段的模板值**大于等于**缓冲的模板值时通过测试 |
| GL_EQUAL     | 在片段的模板值**等于**缓冲的模板值时通过测试     |
| GL_NOTEQUAL  | 在片段的模板值**不等于**缓冲的模板值时通过测试   |

| glStencilOp函数行为 | 描述                                                                     |     |
| ------------------- | ------------------------------------------------------------------------ | --- |
| GL_KEEP             | 保存当前储存的模板值                                                     |     |
| GL_ZERO             | 将模板值设置为0                                                          |     |
| GL_REPLACE          | 将模板值设置为glStencilFunc函数的**ref**值                               |     |
| GL_INCR             | 如果模板值小于最大值则将模板值加一                                       |     |
| GL_INCR_WRAP        | 如果模板值小于最大值则将模板值加一，但如果模板值超过了最大值则归零       |     |
| GL_DECT             | 如果模板值大于最小值则将模板值减一                                       |     |
| GL_DECR_WRAP        | 如果模板值大于最小值则将模板值减一 ，但如果模板值小于0则将其设置为最大值 |     |
| GL_INVERT           | 按**位**翻转当前的模板缓冲值                                                                        |     |
*默认情况下glStencilOp是设置为(GL_KEEP, GL_KEEP,  GL_KEEP)的，即不论任何测试的结果是如何，模板缓冲都会保留他的值。默认的行为不会更新模板缓冲，所以如果想写入模板缓冲的话，需要至少对其中一个选项设置不同的值*

- ###### 模板测试应用
	- 物体轮廓(Object Outlining)，为每一个物体在周围创建一个有色边框。步骤如下：
		- 在绘制（需要添加轮廓的）物体**之前**，将模板函数设置为GL_ALWAYS，每当物体的片段被渲染时，将模板缓冲更新为1
		- 渲染物体
		- **禁用**模板写入以及深度测试
		- 将每个物体缩放一点点
		- 使用一个不同的片段着色器，输出一个单独的边框颜色
		- 再次绘制物体，但是在他们片段的模板值不等于1的时候才绘制
		- 再次启用模板写入和深度测试

#### 混合(Blending)
- ###### Alpha Cut
	- 在片段着色器中直接舍弃alpha值低于阈值的片段
		- <mark>if (texture.a < 0.1) <br>&emsp;discard; </mark>
	- 贴图的上和下，左和右会进行插值，导致贴图的四周会出现边框无法被丢弃，可以通过把贴图环绕方式改为**GL_CLAMP_TO_EDGE**来解决
- ###### Alpha Blend
	- 可以将半透明物体和物体后面的物体的颜色进行混合来达到半透明的效果
	- 通过GL_BLEND开启
		- <mark>glEnable(GL_BLEND)</mark>
	- **OpenGL的混合方程**
		- ![[OpenGL_混合方程.png]]
			- <span style="color:green">C<sub>source</sub></span> ：源颜色向量，源自纹理的颜色向量，**离相机更近的颜色**
			- <span style="color:green">F<sub>source</sub></span> ：源因子值，指定了alpha值对源颜色的影响
			- <span style="color:red">C<sub>destination</sub></span> ：目标颜色向量，当前储存在颜色缓冲中的颜色向量，即**背景(已经渲染过的片段)/里相机更远的颜色**
			- <span style="color:red">F<sub>destination</sub></span> ：目标因子值，指定了alpha值对目标颜色的影响
			- *只有当片段着色器运行完成，且所有测试都通过之后，混合方程才会应用到片段颜色输出与当前颜色缓冲中的值上*
	- **例子**：
		- ![[OpenGL_混合例子.png]]
		- 要将绿色方块绘制到红色方块的上面 (绿块离相机更近，是**源**，红块里相机更远，是**目标**)，结果为：
		-  ![[OpenGL_混合计算过程.png]]
		- ![[OpenGL_混合例子结果.png]]
	- 可以通过glBlendFunc来设置源和目标因子
		- <mark>glBlendFunc(GLenum sfactor, GLenum dfactor)</mark>
			- sfactor：源因子
			- dfactor：目标因子
	- 也可以通过glBlendFuncSeparate来将颜色和alpha设置为不同的选项
		- <mark>glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)</mark>
	- 还可以通过glBlendEquation来改变方程中源和目标之间的运算符
		- <mark>glBlendEquation(GLenum mode)</mark>
			- GL_FUNC_ADD：默认选项，将两个分量相加，Src + Dst
			- GL_FUNC_SUBTRACT：将两个分量相减，Src - Dst
			- GL_FUNC_REVERSE_SUBTRACT：将两个分量调换顺序并相减：Dst - Src

| 混合因子选项                | 值                        |
| --------------------------- | ------------------------- |
| GL_ZERO                     | 因子等于**0**                 |
| GL_ONE                      | 因子等于**1**                 |
| GL_SRC_COLOR                | 因子等于**源颜色**向量        |
| GL_ONE_MINUS_SRC_COLOR      | 因子等于**1-源颜色**向量      |
| GL_DST_COLOR                | 因子等于**目标颜色**向量      |
| GL_ONE_MINUS_DST_COLOR      | 因子等于**1-目标颜色**向量    |
| GL_SRC_ALPHA                | 因子等于**源alpha**分量       |
| GL_ONE_MINUS_SRC_ALPHA      | 因子等于**1-源alpha**分量     |
| GL_DST_ALPHA                | 因子等于**目标alpha**分量     |
| GL_ONE_MINUS_DST_ALPHA      | 因子等于**1-目标alpha**分量   |
| GL_CONSTANT_COLOR           | 因子等于**常数**向量          |
| GL_ONE_MINUS_CONSTANT_COLOR | 因子等于**1-常数**向量        |
| GL_CONSTANT_ALPHA           | 因子等于**常数向量alpha**分量 |
| GL_MINUS_CONSTANT_ALPHA     | 因子等于**1-常数向量alpha**分量                          |

- ###### 渲染半透明物体与不透明物体的顺序问题
	- 当绘制一个由不透明和透明物体的场景时，大体的原则是：
		- 先绘制所有**不透明**物体
		- 对所有透明物体**排序**
		- **按顺序**绘制所有透明的物体(从远到近)
			- 排序代码：
			- ![[OpenGL_透明物体排序1.png]]
			- ![[OpenGL_透明物体排序2.png]]
- ###### 其他问题
	- 如果在渲染半透明物体之前开启了模板测试，需要在渲染半透明物体之前禁用模板测试写入

#### 面剔除(Face Culling)
- 面剔除用来剔除**无法被摄像机看到**的面 (不面向摄像机的面)，这样可以减少非常大的性能开销
- OpenGL通过分析顶点数据的**环绕顺序** (Winding Order)来确定哪些面是**正向面** (Front Face)， 哪些面是**背向面** (Back Face)
- ###### 环绕顺序
	- 环绕顺序用于帮助OpenGL定义三角形的哪个面是正面哪个面是背面
	- 每组组成三角形的三个顶点的**定义顺序**即为该三角形的环绕顺序
	- ![[OpenGL_三角形环绕顺序.png]]
	- 一个三角形只有两种环绕顺序，**顺时针**和**逆时针**
	- 默认情况下**逆时针**顺序定义的顶点将会被处理为**正向**三角形
	- ![[OpenGL_观察者视角三角形正反面.png]]
	- 当一个三角形的两个面都是逆时针顺序定义的时候，**朝外**的一面是**逆时针**，而**朝向物体内部**的一面则是在渲染时是**顺时针**顺序。可以理解为在一张纸上画一个镂空的图案，面向自己的一面的图案是逆时针的，而当把纸水平翻转过来的时候，面向自己的图案就变成了顺时针，这样就可以区分哪面是正面，哪面是背面
- ###### 面剔除
	- 面剔除默认也是禁用状态，可以通过GL_CULL_FACE来开启
		- <mark>glEnable(GL_CULL_FACE)</mark>
	- 可以通过glCullFace来设置要剔除的是正面还是背面，**默认值是GL_BACK**
		- <mark>glCullFace( GL_BACK)</mark>
	- 还可以通过glFrontFace来告诉OpenGL哪个环绕顺序的面是正向面，**默认是GL_CCW**
		- <mark>glFrontFace(GL_CCW)</mark>

| glCullFace 选项   | 描述         |
| ----------------- | ------------ |
| GL_BACK           | 只剔除背向面 |
| GL_FRONT          | 只剔除正向面 |
| GL_FRONT_AND_BACK | 剔除正向面和背向面             |

| glFrontFace 选项 | 描述                               |
| ---------------- | ---------------------------------- |
| GL_CCW           | 将逆时针顺序顶点定义的面设为正向面 |
| GL_CW            | 将顺时针顺序顶点定义的面设为正向面                                   |

#### 帧缓冲(Frame Buffer)
- 帧缓冲是颜色缓冲、深度缓冲、模板缓冲等各种缓冲结合在一起的缓冲
- 一个完整的帧缓冲需要满足一下条件：
	- 附加**至少一个**缓冲(颜色、深度、模板)
	- 至少有一个**颜色附件**(Attachment)
	- 所有的附件都必须是**完整**的(保留了内存)
	- 每个缓冲都应该有**相同**的样本数
- ###### 创建帧缓冲
	- 通过glGenFramebuffers来创建帧缓冲对象
		- <mark>unsigned int fbo;</mark>
		- <mark>glGenFramebuffers(1, &fbo)</mark>
	- 再使用glBindFramebuffer来绑定帧缓冲
		- <mark>glBindFramebuffer(GL_FRAMEBUFFER, fbo)</mark>
	- 绑定之后所有的**读取**和**写入**帧缓冲的操作将会影响当前绑定的帧缓冲
	- 也可以使用<mark>GL_READ_FRAMEBUFFER</mark>或者<mark>GL_DRAW_FRAMEBUFFER</mark>来将一个帧缓冲分别绑定到读取目标和写入目标
		- 绑定到Read的帧缓冲将会使用在所有如glReadPixels的**读取**操作上
		- 绑定到Draw的帧缓冲将会被用作渲染、清楚等**写入**操作的目标上
		- *大部分情况不需要分开绑定*
	- 通过glCheckFramebufferStatus检查帧缓冲是否完整。他会检测**当前绑定**的帧缓冲。如果返回值是**GL_FRAMEBUFFER_COMPLETE**，帧缓冲就是完整的了
		- <mark>if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)</mark>
		- [返回值列表](**https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml**)
	- 当帧缓冲是完整的了之后，所有的渲染操作将会渲染到当前绑定的帧缓冲的附件中
	- 由于自定义的帧缓冲不是默认的帧缓冲，渲染指令不会对窗口的输出有任何影响，通过再次激活默认帧缓冲，将他绑定到0来解决
		- <mark>glBindFramebuffer(GL_FRAMEBUFFER, 0)</mark>
	- 在完成所有帧缓冲操作后删除该帧缓冲对象
		- <mark>glDeleteFramebuffers(1, &fbo)</mark>
- ###### 附件(Attachment)
	- 在完整性检查之前，需要给帧缓冲附加附件
	- 附件是一个内存位置，能够作为帧缓冲的一个缓冲，可以看做为一个图像
	- 创建附件有两个选项
		- **纹理**
		- **渲染缓冲对象(Renderbuffer Object)**
	- 渲染缓冲对象能为帧缓冲对象提供一些优化
	- 通常的规则是，如果不需要从一个缓冲中采样数据，那么对这个缓冲使用渲染缓冲对象，如果需要从缓冲中采样颜色或深度等数据，那么应该使用纹理附件
- ###### 纹理附件
	- 当把纹理附加给帧缓冲时，所有的渲染指令将会写入到这个纹理图像中，**可以在着色器中进行使用**
	- 创建纹理附件和创建一张普通的纹理对象差不多
		- <mark>unsigned int texture; <br>glGenTextures(1, &texture); <br>glBindTexture(GL_TEXTURE_2D, texture); <br>glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); <br>glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); <br>glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);</mark>
		- 与创建一张纹理对象的区别是
			- 纹理附件将**glTexImage2D的宽高设置为了屏幕的宽高**，因为这张纹理将要作为窗口的输出，需要和窗口一样大 (但这并不是必须的)
			- 将glTexImage2D的纹理实际**data参数设为了NULL**，因为填充这个纹理的数据将会在**渲染到帧缓冲之后**来进行
		- *如果想把屏幕渲染到更大或者更小的纹理上，需要在渲染到帧缓冲之前再调用一次glViewport并使用纹理的新维度作为参数，否则只有一小部分的纹理火屏幕会被渲染到这个纹理上*
		- 创建好纹理之后需要将纹理附件附加到帧缓冲上
			- <mark>glFramebufferTexture2D(target, attachment, textarget, texture, level);</mark>
				- `target`：帧缓冲的目标 (绘制，读取，或者都)
				- `attachment`：要附加的附件类型
					- GL_COLOR_ATTACHMENT0
					- GL_DEPTH_ATTACHMENT
					- GL_STENCIL_ATTACHMENT
					- *颜色附件可以有多个，通过类型末尾的数字设置*
				- `textarget`：附加的纹理类型
					- GL_TEXTURE_2D
					- GL_TEXTURE_CUBE_MAP_POSITIVE_X 
					- GL_TEXTURE_CUBE_MAP_NEGATIVE_X 
					- GL_TEXTURE_CUBE_MAP_POSITIVE_Y 
					- GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 
					- GL_TEXTURE_CUBE_MAP_POSITIVE_Z 
					- GL_TEXTURE_CUBE_MAP_NEGATIVE_Z`
				- `texture`：附加的纹理附件对象本身
				- `level`：Mipmap级别，**必须是0**
			- 可以将深度缓冲和模板缓冲附加到同一个单独的纹理中。纹理的每32位数值将包含24位的深度信息和8位的模板信息
				- <mark>glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL );<br> glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);</mark>
- ###### 渲染缓冲对象附件(Renderbuffer Object)
	- 渲染缓冲对象附件会将数据储存为OpenGL原生的渲染格式，他是为离屏渲染到帧缓冲优化过的
	- 渲染缓冲对象直接将所有的渲染数据储存在它的缓冲中，不会做任何针对纹理格式的转换
	- 渲染缓冲对象通常是只写的，不能读取。但是仍然可以用glReadPixels来读取他，但是是从当前绑定的帧缓冲中返回特定区域的像素而不是附加本身
	- 创建并绑定渲染缓冲对象
		- <mark>unsigned int rbo;<br>glGenRenderbuffers(1, &rbo);<br>glBindRenderbuffer(GL_RENDERBUFFER, rbo);</mark>
	- 因为我们只**需要**深度和模板值用于测试，而不需要对他们进行**采样**，所以通常是只写的渲染缓冲对象很适合用于深度和模板附件
	- 通过调用glRenderbufferStorage来创建深度和模板渲染缓冲对象
		- <mark>glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scrWidth, scrHeight);</mark>
	- 最后附加该渲染缓冲对象到当前帧缓冲
		- <mark>glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,  rbo);</mark>
- ###### 渲染到纹理
	- 要将场景绘制到纹理上需要以下步骤：
		- 将新的帧缓冲绑定为激活 的帧缓冲，和往常一样渲染场景
		- 绑定默认的帧缓冲
		- 绘制一个横跨整个屏幕的四边形，将帧缓冲的颜色缓冲作为他的纹理
- ###### 基于帧缓冲的一些后处理
	- 反相(Inversion)
		- 1 - 屏幕纹理的颜色值
		- ![[OpenGL_帧缓冲后处理_反相.png]]
	- 灰度(Grayscale)
		- 三种灰度算法
			- Lightness: 在R, G, B中取最大和最小的两个分量然后取他们俩的平均数
				- <mark>(max(R, G, B) + min(R, G, B)) / 2</mark>
			- Average: 将R, G, B的三个分量相加取平均数
				- <mark>(R + G + B) /3 </mark>
			- Luminosity: 由于人眼对绿色更敏感，而对蓝色不那么敏感，所以为了获取物理上更精确的效果，使用加权的通道，即**R的权重为0.2126，G的权重为0.7152，B的权重为0.0722**
				- <mark>0.2126 \* R + 0.7152 \* G + 0.0722 \* B</mark>
		- ![[OpenGL_帧缓冲后处理_灰度.png]]
	- **核效果**
		- 核(Kernel) / 卷积矩阵(Convolution Matrix)是一个类似矩阵的数值数组，这个数组的中心的值是当前的像素对应的核值，周围的核值是该像素周围的像素所对应的核值。核的每个值都与他所对应的像素相乘，并将结果相加，相加的结果为中心像素新的值
		- ![[OpenGL_核.png]]
		- 在上图的核中，当前像素的新值为
			- 2 \* P<sub>11</sub>  +   2 \* P<sub>12</sub>  +   2 \* P<sub>13</sub>  +   2 \* P<sub>21</sub>  +   2 \* P<sub>23</sub>  +   2 \* P<sub>31</sub>  +   2 \* P<sub>32</sub>  +   2 \* P<sub>33</sub>  +   (-15) \* P<sub>22</sub>
		- 大部分的核都是3X3核
		- 大部分的核的**所有权重加起来应该等于1**，如果不等于1，意味着新的像素值会比原来的像素值更亮 (大于1) 或者更暗 (小于1)
		- 周围像素距离中心像素的距离(采样点)可以自定义，越小的采样点最后的结果越细腻
			- ![[OpenGL_帧缓冲后处理_核偏移量.png]]
	- 核效果应用-**锐化**
		- ![[OpenGL_帧缓冲后处理_锐化核.png]]
		- ![[OpenGL_帧缓冲后处理_锐化.png]]
	- 核效果应用-**模糊**
		- 使用更高阶的高斯模糊的时候可以通过先渲染横向模糊到一个FBO，再将横向模糊的纹理在第二个FBO中进行纵向模糊，最终得到完整的高斯模糊。这样做可以极大的减少采样数以减少开销，越高阶的高斯模糊节约的性能越多
		- ![[OpenGL_帧缓冲后处理_模糊核.png]]
		- ![[OpenGL_帧缓冲后处理_模糊.png]]
	- 核效果应用-**边缘检测**
		- ![[OpenGL_帧缓冲后处理_边缘检测核.png]]
		- ![[OpenGL_帧缓冲后处理_边缘检测.png]]

#### Cubemap
- Cubemap是包含了6个2D纹理的纹理，每个2D纹理组成立方体的一个面
- ###### 创建Cubemap
	- <mark>unsigned int textureID; <br>glGenTextures(1, &textureID); <br>glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);</mark>
	- 由于Cubemap包含了6个面，需要对每一个面调用一次<mark>glTexImage2D</mark>函数，一共6次，每次在调用时设定该纹理对应的是Cubemap的哪个面
	- 可以通过循环从<mark>GL_TEXTURE_CUBE_MAP_POSITIVE_X</mark>开始历遍所有的方位
		- ![[OpenGL_Cubemap纹理历遍.png]]
		- textures_faces包含了所有的纹理目标并按顺序排列
	- 设置好6个面的纹理之后需要对Cubemap的**环绕**和**过滤**方式进行设置
		- 唯一与2D纹理不同的是，由于Cubemap是立体的，需要在环绕方式里**增加一个R坐标**来这是Cubemap的Z方向环绕方式

| 纹理目标                       | 方位 |
| ------------------------------ | ---- |
| GL_TEXTURE_CUBE_MAP_POSITIVE_X | 右   |
| GL_TEXTURE_CUBE_MAP_NEGATIVE_X | 左   |
| GL_TEXTURE_CUBE_MAP_POSITIVE_Y | 上   |
| GL_TEXTURE_CUBE_MAP_NEGATIVE_Y | 下   |
| GL_TEXTURE_CUBE_MAP_POSITIVE_Z | 后   |
| GL_TEXTURE_CUBE_MAP_NEGATIVE_Z | 前     |

- ###### 天空盒
	- 使用Cubemap的**顶点**位置作为UV来进行采样(立方体处于原点)
	- 不给天空盒创建M矩阵，以及**使用无视位移的V矩阵**，来使天空盒看起来很远
	- 使用**提前深度测试(Early Depth Test)**，直接不渲染所有被遮挡的部分，而不是在渲染完所有的片段之后再在深度测试中舍弃被遮挡的片段
		- [关于提前深度测试](https://zhuanlan.zhihu.com/p/72956611)
		- 为了让天空盒被其他所有物体遮挡，在**顶点着色器**中将<mark>gl_Position</mark>的**z**分量设为**w**，因为片段的深度值是由<mark>gl_Position</mark>的**z**分量除以**w**分量得到的，将**z**手动设置为**w**可以让天空盒的片段深度值永远是**最大值1**(离相机最远)
- ###### 环境映射-反射
	- 反射是的颜色，**根据观察者的视角**，或多或少的受环境颜色的影响
	- **反射需要**：
		- 视方向的反方向
		- 法线方向
	- 通过**视方向的反方向**和**法线方向**求得**观察方向沿法线方向的反射方向**，并使用该向量作为UV值在Cubemap上进行采样从而获得模型当前片段的镜面反射颜色
	- ![[OpenGL_环境映射_反射.png]]
- ###### 环境映射-折射
	- 折射是由于光线由于**传播介质的改变**而产生的**方向的改变**，折射通过斯涅尔定律 (Snell's Law) 来描述
	- ![[OpenGL_环境映射_折射示意图.png]]
	- 折射需要：
		- 视方向的反方向
		- 法线方向
	- 通过**视方向的反方向**和**法线方向**以及**当前传播介质和目标传播介质的折射率的比值**求得视方向在目标介质中的传播方向
	- ![[OpenGL_环境映射_折射.png]]

#### 高级数据
- ###### OpenGL的一些对缓冲的操作
	- 缓冲只是一个管理特定内存块的对象，在把他绑定到一个缓冲目标时，如GL_ARRAY_BUFFER、GL_ELEMENT_ARRAY_BUFFER，他才有意义
	- 在绑定完缓冲进行数据的填充时，可以把**data**设为**NULL**，这样可以预留特定大小的内存，但是在之后才对这个缓冲进行数据填充
	- 可以通过<mark>glBufferSubData</mark>来对一个缓冲的一部分进行数据填充，但在使用<mark>glBufferSubData</mark>之前必须先调用<mark>glBufferData</mark>来为缓冲分配足够的内存
		- <mark>// 范围： [24, 24 + sizeof(data)]<br>glBufferSubData(GL_ARRAY_BUFFER, 24, sizeof(data), &data);</mark>
	- 或者通过请求缓冲内存的指针，直接将数据复制到缓冲当中。通过调用<mark>glMapBuffer</mark>函数，OpenGL会返回当前绑定缓冲的内存指针。这样可以直接将数据映射到缓冲中而不用实现将他们储存到临时内存中
		- ![[OpenGL_缓冲内存指针.png]]
- ###### 分批顶点属性
	- 在之前的教程里都是将模型的顶点位置、法线方向、UV、顶点色等数据全部紧密的放置在一起，而另一种做法是将每个数据类型分批打包成个数组，**比如位置数据包含了模型所有的顶点的顶点位置**，**法线数据包含了模型所有的顶点的法线方向**
	- 这两种方法细化那个用哪个
		- ![[OpenGL_顶点数据分批1.png]]
		- ![[OpenGL_顶点数据分批2.png]]
- ###### 复制缓冲
	- 填充好数据之后，可以将缓冲里的数据复制给其他的缓冲，或者与其他缓冲共享数据，可以通过<mark>glCopyBufferSubData</mark>来复制缓冲中的数据
		- <mark>void glCopyBufferSubData(GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size);</mark>
		- `readtarget`：被复制数据的缓冲 (源)
		- `writetarget`：把数据复制过去的目标缓冲 (目标)
		- `readoffset`：读取数据的起始位置
		- `writeoffset`：写入数据的起始位置
			- 由于不能同时绑定两个同样类型的缓冲，如果**源和目标是同样的缓冲类型**，可以需要的缓冲对象绑定到<mark>GL_COPY_READ_BUFFER</mark>和<mark>GL_COPY_WRITE_BUFFER</mark>上，并把他们俩设置为源和目标，<mark>glCopyBufferSubData</mark>会从源目标的其实位置读取数据并在目标的起始位置写入数据
			- ![[OpenGL_复制缓冲1.png]]
		- 也可以只将`writetarget`缓冲绑定为新的缓冲目标类型
			- ![[OpenGL_复制缓冲2.png]]

#### 高级GLSL
- ###### GLSL的内建变量(Built-in Variable)
	- 除了顶点属性，uniform，和sampler以外，还可以通过前缀为**gl_** 的变量来读取/写入着色器之外的数据
	- 最常见的有一直在用的**gl_Position**和在可视化场景深度时用的**gl_FragCoord**
	- [全部的OpenGL内建变量](https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL))
- ###### 顶点着色器变量
	- **gl_Position**：输出变量，裁剪空间的顶点位置向量，想要在屏幕上输出颜色必须的步骤
	- **gl_PointSize**：输出变量，当图元渲染模式设为<mark>GL_POINTS</mark>时，可以在主程序中使用<mark>glPointSize</mark>来设置点的大小，也可以在顶点着色器中通过<mark>gl_PointSize</mark>来设置
		- 与在主程序中设置的区别是，在顶点着色器中可以对每个点设置不同的值
		- 在顶点着色器中修改点图元大小的功能默认是禁用的，需要使用<mark>GL_PROGRAM_POINT_SIZE</mark>来启用
			- <mark>glEnable(GL_PROGRAM_POINT_SIZE);</mark>
		- 使用例：点离相机越远就越大<br>![[OpenGL_gl_PointSize.png]]
	- **gl_VertexID**：输入变量，储存了正在绘制的顶点的ID
		- 使用<mark>glDrawElements</mark>进行索引渲染时，这个变量会储存正在绘制顶点的当前索引
		- 使用<mark>glDrawArrays</mark>进行渲染是，这个变量会储存从渲染调用开始的已处理的顶点数量
- 