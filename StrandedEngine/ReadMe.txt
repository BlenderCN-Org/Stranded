﻿========================================================================
    静态库：StrandedEngine 项目概述
========================================================================

应用程序向导已为您创建了此 StrandedEngine 库项目。

没有为此项目创建源文件。


StrandedEngine.vcxproj
    这是使用应用程序向导生成的 VC++ 项目的主项目文件，
    其中包含生成该文件的 Visual C++ 
    的版本信息，以及有关使用应用程序向导选择的平台、配置和项目功能的信息。

StrandedEngine.vcxproj.filters
    这是使用“应用程序向导”生成的 VC++ 项目筛选器文件。 
    它包含有关项目文件与筛选器之间的关联信息。 在 IDE 
    中，通过这种关联，在特定节点下以分组形式显示具有相似扩展名的文件。
    例如，“.cpp”文件与“源文件”筛选器关联。

/////////////////////////////////////////////////////////////////////////////
其他注释：

应用程序向导使用“TODO:”注释来指示应添加或自定义的源代码部分。

/////////////////////////////////////////////////////////////////////////////

场景管理技术:
1、状态管理
1.1 纹理状态转换
	多边形排序。
1.1.1 渲染缓存
	渲染缓存是在渲染系统中创建的一系列缓存。绘制几何图形时，不要直接将其绘制到屏幕上。实际上，要先将几何图形放到渲染缓存中。例如，在场景开始处要绘制一个房子模型。
如果渲染系统有10个渲染缓存，那么房子模型将被放置到第一个渲染缓存中，房子模型的纹理将用于确定几何图形的哪些部分可被放置到第一个渲染缓存中。进入到渲染缓存中的几何图形
的其他内容是使用相同纹理的其他多边形，而该纹理与第一个渲染相关。如果试图渲染一个有不同纹理图像的其他模型，将那个新对象放置到自由缓存中，在这种情况下，这个自由缓存可
是第二个渲染缓存。要重复这样做，直到所有缓存用完为止。出现这种情况时，就会发现程序中包含了大多数多边形的缓存，绑定该纹理图像，并将那个缓存中的所有几何图形绘制到屏幕
上。之后，通过将缓存设置为空来清除内存。现在可以将下一个对象放置到自由缓存中，一直。一旦将所有的场景放到渲染缓存中，就只需将所有包含集合图形的缓存渲染到屏幕上并为下
一帧清除每个缓存即可。使用许多已经设置好的大缓存存储一定量的几何图形。每次渲染模型时，都要查看渲染缓存中是否已经有共用纹理图像的多边形。如果有，那个模型的几何图形就
会被添加到已经存在的包含所有其他几何图形的缓存中。如果渲染缓存已经用完，就必须通过将包含多边形数目最多的缓存渲染到屏幕上，释放该缓存。有时候，会遇到这种情况，即有一
个包含共用纹理图像的几何图形缓存，但是该缓存已满。出现这种情况时，只能渲染该缓存(清除该缓存)并开始再次使用最初试图放置到该缓存的数据来填充它。使用的渲染缓存数目、渲
染缓存的大小以及在场景中正在渲染的几何图形的数目都将确定渲染缓存要被清除的次数。使用类似这样的系统，实际上是直到处理完场景或缓存已满并且需要通过将缓存内容渲染到屏幕
上而释放它或是自由缓存已经用完，并需要腾出空间时，才会将数据发送到硬件。
	在状态之间转换时，可编程的阴影器开销最大。
1.1.2 闭塞物和平截头体选择
	游戏中，有时会发现沿着某个方向进行观察，某些几何图形在视线之外。解决方法：进行平截头体选择处理。
	平截头体选择是从摄像机视角创建6个平面：左边一个平面、右边一个平面、顶部和底部两个平面、近端和远端两个平面。近端是按照距离摄像机可以观察物体的距离程度而确定的。
通过平面数学计算(ClassifyPoint()、 ClassifyPolygon())，可确定物体在平面那一侧。如果确定一个物体在所有的平面前方并不再任何一个平面后面，那么可以认为该对象可能在观察
视线中。该对象“可能”在观察视线中是因为像墙这样的物体会遮挡住从视线方向观察的对象。但是，该对象还是落在观察区域内。没有落在观察区域内的物体就会被“截去”，不绘制。
	对墙体多边形，使用“阻挡选择”技术处理。主要测试一个对象是否遮挡住了另一个对象。被遮挡住对象无需绘制。对墙体，可以通过创建构成墙的三角形的某一个平面完成。可使用
该平面测试对象是否落在墙前面还是墙后面。对象相对于墙的位置将决定对象是否可见。如果与对象同侧，那么就可以看到该对象，否则就不可以。(阻塞数目、位置以及使用那种多边形
进行阻塞)
1.1.3 八叉树和四叉树
	将场景分割成由多个娇小片构成的层状结构，可以快速遍历树以确定从某个角度可以看到特定的多边形。场景在一个树中，向下遍历树，没必要访问书上的每个叶子(节点)。
	八叉树是树形结构。先获取一系列多边形，并计算多边形的边界框。一旦得到边界框，就将其分解成8个更小的边界框。然后循环第1个边界框中的所有几何图形，并确定哪个几何图形
落在哪个较小的边界框(子节点)。处理完所有的三角形时，可以对所有的小边界框做相同的递归处理，也就是可以将娇小的边界框分解成更小的边界框。可以持续这样做，直到使用该函数
递归得到一定深度的递归层次，或是在节点上得到一定数量的多边形。
	分割最初的边界框时，会得到8个娇小的边界框。这样可以确定落在每个边界框中的几何图形。然后将这些小边界框分解成更小的边界框，并确定第一级的小边界框中的哪个三角形落
在第二级的小边界框中。
	四叉树和八叉树之间的主要差别在于八叉树分割成8个更小的立方体，而四叉树则是分割成4个更小的立方体(Quad=4, Oct=8)。通过将场景分割成许多小的边界框，就可以对边界框使
用平截头体选择技术，以确定场景中可见的几何图形。从第一个节点(根节点)开始，遍历整个树。如果某个节点不可见，那么意味着它没有通过平截头体测试，那么就忽略该节点及其所有
子节点，这些子节点是在该立方体中更小的立方体。

//////////////////////////////////////////////////////颜色混合 [SRC DST]////////////////////////////////////////////////////////////
以这个Sprite作为源，Sprite所在位置的其它像素作为目标，进行混合运算:

源的RGBA变量：Rs，Gs，Bs，As；
目标的RGBA： Rd，Gd，Bd，Ad；
源的各个运算因子： N_Rs，N_Gs，N_Bs，N_As；
目标的各个运算因子： N_Rd，N_Gd，N_Bd，N_Ad；
混合后的RGBA为：(Rs*N_Rs+ Rd* N_Rd，Gs*N_Gs+ Gd* N_Gd，Bs*N_Bs+ Bd* N_Bd，As*N_As+ Ad* N_Ad)

其中的运算因子包括：
GL_ONE：1.0
GL_ZERO：0.0
GL_SRC_ALPHA：源的Alpha值作为因子
GL_DST_ALPHA：目标Alpha作为因子
GL_ONE_MINUS_SRC_ALPHA：1.0减去源的Alpha值作为因子
GL_ ONE_MINUS_DST_ALPHA：1.0减去目标的Alpha值作为因子

常数	相关因子	融合因子结果
GL_ZERO	源因子或目的因子	(0,0,0,0)
GL_ONE	源因子或目的因子	(1,1,1,1)
GL_DST_COLOR	源因子	(Rd,Gd,Bd,Ad)
GL_SRC_COLOR	目的因子	(Rs,Gs,Bs,As)
GL_ONE_MINUS_DST_COLOR	源因子	(1,1,1,1)-(Rd,Gd,Bd,Ad)
GL_ONE_MINUS_SRC_COLOR	目的因子	(1,1,1,1)-(Rs,Gs,Bs,As)
GL_SRC_ALPHA	源因子或目的因子	(As,As,As,As)
GL_ONE_MINUS_SRC_ALPHA	源因子或目的因子	(1,1,1,1)-(As,As,As,As)
GL_DST_ALPHA	源因子或目的因子	(Ad,Ad,Ad,Ad)
GL_ONE_MINUS_DST_ALPHA	源因子或目的因子	(1,1,1,1)-(Ad,Ad,Ad,Ad)
GL_SRC_ALPHA_SATURATE	源因子	(f,f,f,1); f=min(As,1-Ad)

RM_ALPHA = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}
RM_ADD1 = {G_ONE, GL_ONE_MINUS_SRC_COLOR}
RM_ADD2 = {G_ONE, GL_ONE}
RM_ALPHAADD = {GL_SRC_ALPHA, GL_ONE}
RM_MODULATE = {GL_ZERO, GL_SRC_COLOR}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////