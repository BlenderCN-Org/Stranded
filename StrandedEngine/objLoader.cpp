#include "objLoader.h"

stObjModel* LoadOBJModel(const char* fileName)
{
	FILE *file = nullptr;
	char *data = nullptr;
	CToken lexer, tempLex;
	char tempLine[512];
	char token[512];

	// Open file for input.
#pragma warning(push)
#pragma warning(disable:4996)
	file = fopen(fileName, "r");
#pragma warning(pop)
	if (!file) return nullptr;

	// Get the length of the file.
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Read in all data from the file.
	data = new char[(length + 1) * sizeof(char)];
	if (!data) return nullptr;
	fread(data, length, 1, file);
	data[length] = '\0';

	// Close the file when we are doen.
	fclose(file);

	// Set our file to our lexer. 使用缓存数据设置 CToken 对象的令牌流
	lexer.SetTokenStream(data);

	// No longer need.
	delete[] data;
	data = nullptr;

	//////////////////////////////////////////////////////////////////////////

	// (确定文件有效)
	bool validFile = false;

	// Look for the word. 循环所有令牌并查找 Wavefront 单词，标识是否为 Wavefront OBJ 文件
	while (lexer.GetNextToken(token))
	{
		if (strcmp(token, "Wavefront") == 0)
		{
			validFile = true;
			break;
		}
	}

	if (!validFile) return nullptr;

	// Reset for next pass. (将令牌流重设到开始位置)
	lexer.Reset();

	//////////////////////////////////////////////////////////////////////////

	// Used to get the total number of each declared in a file.
	// Since faces uses indices these number could be different.
	// 确定文件中的顶点、法线、纹理坐标和外观的总数
	int totalVertices = 0, totalNormals = 0, totalTexC = 0, totalFaces = 0;

	// Get the first (or next) line.
	while (lexer.MoveToNextLine(tempLine))
	{
		// Set line to the temp lexer.
		tempLex.SetTokenStream(tempLine);

		// Read the new line character.
		lexer.GetNextToken(NULL);

		// If something was set to the temp lex then we keep going.
		if(!tempLex.GetNextToken(token))
			continue;

		// If the first token of the line is a v, vn, vt, or f
		// increment the respective counter.
		if (strcmp(token, "v") == 0)
			totalVertices++;
		else if (strcmp(token, "vn") == 0)
			totalNormals++;
		else if (strcmp(token, "vt") == 0)
			totalTexC++;
		else if (strcmp(token, "f") == 0)
			totalFaces++;

		token[0] = '\0';
	}

	// Allocate temp space.
	float *verts = new float[totalVertices * 3];
	float *norms = new float[totalNormals * 3];
	float *texC = new float[totalTexC * 3];
	int *faces = new int[totalFaces * 9];
	int vIndex = 0, nIndex = 0, tIndex = 0, fIndex = 0, index = 0;

	// Move to the beginning of the file.
	lexer.Reset();

	//////////////////////////////////////////////////////////////////////////

	// Do it all again but this time we get the data.
	while (lexer.MoveToNextLine(tempLine))
	{
		// Set to temp lex, read past newline, get token.
		tempLex.SetTokenStream(tempLine);
		lexer.GetNextToken(NULL);

		if(!tempLex.GetNextToken(token))
			continue;

		// If v then we get the vertex x, y, z.
		if (strcmp(token, "v") == 0)
		{
			// Get the x and save it.
			tempLex.GetNextToken(token);
			verts[vIndex] = (float)atof(token);
			vIndex++;

			// Get the y and save it.
			tempLex.GetNextToken(token);
			verts[vIndex] = (float)atof(token);
			vIndex++;

			// Get the z and save it.
			tempLex.GetNextToken(token);
			verts[vIndex] = (float)atof(token);
			vIndex++;
		}
		// Else If vn then we get the normal x, y, z.
		else if (strcmp(token, "vn") == 0)
		{
			// Get the x and save it.
			tempLex.GetNextToken(token);
			norms[nIndex] = (float)atof(token);
			nIndex++;

			// Get the y and save it.
			tempLex.GetNextToken(token);
			norms[nIndex] = (float)atof(token);
			nIndex++;

			// Get the z and save it.
			tempLex.GetNextToken(token);
			norms[nIndex] = (float)atof(token);
			nIndex++;
		}
		// Else If vt then we get the tex coord u, v.
		else if (strcmp(token, "vt") == 0)
		{
			// Get the u and save it.
			tempLex.GetNextToken(token);
			texC[tIndex] = (float)atof(token);
			tIndex++;

			// Get the v and save it.
			tempLex.GetNextToken(token);
			texC[tIndex] = (float)atof(token);
			tIndex++;
		}
		// Else If f then get each vertex 3 indices set.
		else if (strcmp(token, "f") == 0)
		{
			// Load for each vertex (3 in a triangle).
			for (int i = 0; i < 3; i++)
			{
				// Get first set. Get the length of it.
				tempLex.GetNextToken(token);
				int len = strlen(token);

				// Extract indices.
				for (int s = 0; s < len + 1; s++)
				{
					char buff[64];

					// If this is not a / or if not the end.
					if (token[s] != '/' && s < len)
					{
						buff[index] = token[s];
						index++;
					}
					else
					{
						// Else end string then convert.
						buff[index] = '\0';
						faces[fIndex] = (int)atoi(buff);
						fIndex++;
						index = 0;
					}
				}
			}
		}

		token[0] = '\0';
	}

	// No longer need.
	lexer.Shutdown();

	//////////////////////////////////////////////////////////////////////////

	// Create the model object by allocating.
	stObjModel *model = new stObjModel;
	if (!model) return nullptr;

	memset(model, 0, sizeof(stObjModel));

	// Save face count.[模型总外观数]
	model->m_numFaces = totalFaces;

	// Reset temp counters.
	vIndex = 0, nIndex = 0, tIndex = 0, fIndex = 0, index = 0;

	// Allocate data for each part of the model.
	// 根据顶点总数(总外观数*3)和每块数据的成分总数分配模型对象(顶点、纹理坐标、法线等)中的每个数组
	model->m_pVertices = new float[totalFaces * 3 * 3];
	if (totalNormals)
		model->m_pNormals = new float[totalFaces * 3 * 3];
	if (totalTexC)
		model->m_pTexCoords = new float[totalFaces * 3 * 2];

	// Loop through and fill in our model.
	for (int f = 0; f < totalFaces * 9; f += 3)
	{
		// Get vertex.
		model->m_pVertices[vIndex + 0] = verts[(faces[f] - 1) * 3 + 0];
		model->m_pVertices[vIndex + 1] = verts[(faces[f] - 1) * 3 + 1];
		model->m_pVertices[vIndex + 2] = verts[(faces[f] - 1) * 3 + 2];

		vIndex += 3;

		// Get Texture coordinates.
		if (model->m_pTexCoords)
		{
			model->m_pTexCoords[tIndex + 0] = texC[(faces[f + 1] - 1) * 2 + 0];
			model->m_pTexCoords[tIndex + 1] = texC[(faces[f + 1] - 1) * 2 + 1];

			tIndex += 2;
		}

		// We do the same with the normal data.
		if (model->m_pNormals)
		{
			model->m_pNormals[nIndex + 0] = norms[(faces[f + 2] - 1) * 3 + 0];
			model->m_pNormals[nIndex + 1] = norms[(faces[f + 2] - 1) * 3 + 1];
			model->m_pNormals[nIndex + 2] = norms[(faces[f + 2] - 1) * 3 + 2];

			nIndex += 3;
		}
	}

	// Delete temp data.
	delete[] verts;
	delete[] norms;
	delete[] texC;
	delete[] faces;

	return model;
}

/*
 * @param: 正在试图释放的 stObjModel 对象的地址
 */
void FreeOBJModel(stObjModel* model)
{
	if (!model) return;

	// Release all resources.
	if (model->m_pVertices)
	{
		delete[] model->m_pVertices;
		model->m_pVertices = nullptr;
	}

	if (model->m_pNormals)
	{
		delete[] model->m_pNormals;
		model->m_pNormals = nullptr;
	}

	if (model->m_pTexCoords)
	{
		delete[] model->m_pTexCoords;
		model->m_pTexCoords = nullptr;
	}

	delete model;
	model = nullptr;
}
