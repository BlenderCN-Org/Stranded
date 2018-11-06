// X文件 标识符 xof-程序正在加载某个版本的X文件, 0302-文件的主版本号和次版本号(3.2), 
// txt-程序正在加载的是X模型的文本文件，非二进制文件, 0032-表示程序加载的文件使用的浮点值位数(32)
// Direct3D标准模板, 该文件不需创建模板
xof 0302txt 0032

// Material - 针对材质的标准模板. 用于指定可以施加在单个表面(多边形)的材质。
Material UgpMat {					// Material 0.
	1.0;1.0;1.0;1.0;;				// Diffuse Color 定义环境颜色
	0.0;							// Specular Power 发射能量
	0.0;0.0;0.0;;					// Specular color 镜面颜色
	0.0;0.0;0.0;;					// Emissive color 反射量
	TextureFilename{"ugp.bmp";}		// Texture file name. 与材质相关的纹理图像文件名
}

// The square mesh data.
// Mesh 模板 - 定义 X 文件中的完整网格, X 模型可以包含多个网格.
// 可在单个模型文件中使用不同的 mesh 模板将网格分成头网格、上身网格、下身网格.
Mesh Square {
	4;								// Number of vertices. (1. 定义顶点总数)
	1.0; 1.0; 0.0;,					// Vertex 1  (2.顶点的x、y、z 坐标)
	-1.0; 1.0; 0.0;,				// Vertex 2
	-1.0; -1.0; 0.0;,				// Vertex 3
	1.0; -1.0; 0.0;					// Vertex 4
	
	// (3. 必须定义网格的外观或多边形)
	2;								// Number of triangles. (3.1.网格中的外观数)
	3;0,1,2;,						// Triangle indices 1. (3.1.1.每个外观的三角形索引)
	3;0,2,3;;						// Triangle indices 2. (3.1.1.每个三角形索引行 开始代表该外观使用的三角形数目，后面才是索引号)
	
	// (4. 其他属性，比如，每一外观的材质、纹理坐标)
	// 模板指明网格中的哪个外观使用那种材质
	MeshMaterialList {
		1;							// Number of materials. (材质数目)
		2;							// Number of faces. (将材质施加的外观总数)
		0,							// Face 0 use material 0. (每个外观占一行)
		0;;							// Face 1 use material 0. (每行定义一个值，用该值引用要用的材质)
		{UgpMat}					// Reference of material. (涉及到的所有材质链表. 第一个材质索引号为0, 第二个为1... 这里每个外观都指定材质索引号为0, 因为这里只有一个材质可用)
	}
	
	// 利用该模板在 Direct3D 中将纹理映射到网格上
	MeshTextureCoords {
		4;							// Number of vertices. (网格中的索引数)
		0.0; 0.0;,					// Vertex 1 tex coord. (逗号分隔，列出每个顶点的纹理坐标)
		0.0; 1.0;,					// Vertex 2 tex coord.
		1.0; 1.0;,					// Vertex 3 tex coord.
		1.0; 0.0;;					// Vertex 4 tex coord.
	}
	
}	// End of Mesh Square