// X�ļ� ��ʶ�� xof-�������ڼ���ĳ���汾��X�ļ�, 0302-�ļ������汾�źʹΰ汾��(3.2), 
// txt-�������ڼ��ص���Xģ�͵��ı��ļ����Ƕ������ļ�, 0032-��ʾ������ص��ļ�ʹ�õĸ���ֵλ��(32)
// Direct3D��׼ģ��, ���ļ����贴��ģ��
xof 0302txt 0032

// Material - ��Բ��ʵı�׼ģ��. ����ָ������ʩ���ڵ�������(�����)�Ĳ��ʡ�
Material UgpMat {					// Material 0.
	1.0;1.0;1.0;1.0;;				// Diffuse Color ���廷����ɫ
	0.0;							// Specular Power ��������
	0.0;0.0;0.0;;					// Specular color ������ɫ
	0.0;0.0;0.0;;					// Emissive color ������
	TextureFilename{"ugp.bmp";}		// Texture file name. �������ص�����ͼ���ļ���
}

// The square mesh data.
// Mesh ģ�� - ���� X �ļ��е���������, X ģ�Ϳ��԰����������.
// ���ڵ���ģ���ļ���ʹ�ò�ͬ�� mesh ģ�彫����ֳ�ͷ��������������������.
Mesh Square {
	4;								// Number of vertices. (1. ���嶥������)
	1.0; 1.0; 0.0;,					// Vertex 1  (2.�����x��y��z ����)
	-1.0; 1.0; 0.0;,				// Vertex 2
	-1.0; -1.0; 0.0;,				// Vertex 3
	1.0; -1.0; 0.0;					// Vertex 4
	
	// (3. ���붨���������ۻ�����)
	2;								// Number of triangles. (3.1.�����е������)
	3;0,1,2;,						// Triangle indices 1. (3.1.1.ÿ����۵�����������)
	3;0,2,3;;						// Triangle indices 2. (3.1.1.ÿ�������������� ��ʼ���������ʹ�õ���������Ŀ���������������)
	
	// (4. �������ԣ����磬ÿһ��۵Ĳ��ʡ���������)
	// ģ��ָ�������е��ĸ����ʹ�����ֲ���
	MeshMaterialList {
		1;							// Number of materials. (������Ŀ)
		2;							// Number of faces. (������ʩ�ӵ��������)
		0,							// Face 0 use material 0. (ÿ�����ռһ��)
		0;;							// Face 1 use material 0. (ÿ�ж���һ��ֵ���ø�ֵ����Ҫ�õĲ���)
		{UgpMat}					// Reference of material. (�漰�������в�������. ��һ������������Ϊ0, �ڶ���Ϊ1... ����ÿ����۶�ָ������������Ϊ0, ��Ϊ����ֻ��һ�����ʿ���)
	}
	
	// ���ø�ģ���� Direct3D �н�����ӳ�䵽������
	MeshTextureCoords {
		4;							// Number of vertices. (�����е�������)
		0.0; 0.0;,					// Vertex 1 tex coord. (���ŷָ����г�ÿ���������������)
		0.0; 1.0;,					// Vertex 2 tex coord.
		1.0; 1.0;,					// Vertex 3 tex coord.
		1.0; 0.0;;					// Vertex 4 tex coord.
	}
	
}	// End of Mesh Square