#include "PropertyScript.h"
#include <fstream>

using std::ifstream;

#pragma warning(push)
#pragma warning(disable:4996)
/*
 * @brief: 从文件中读取一整行内容，确定要处理的属性类型
 * @param:
		要加载属性到内存中的开始索引
		存储文件中正行文本的缓存
 */
static int DetermineType(int startIndex, char* buffer)
{
	int numComponents = 0;
	int type = NULL_VAR;
	// 是否包含小数点
	bool decimalFound = false;
	bool charFound = false;
	int index = startIndex;

	// Loop through the string and get information about it.
	while (index < (int)strlen(buffer))
	{
		// Since there are no new lines we auto add this if we get inside this loop.
		if (numComponents == 0)
			numComponents++;

		// Delimiter.分隔符
		if (buffer[index] == ' ')
			numComponents++;

		// Decimal
		if (buffer[index] == '.')
			decimalFound = true;

		// Character.
		if ((buffer[index] >= 'a' && buffer[index] <= 'z') || (buffer[index] >= 'A' && buffer[index] <= 'Z') || buffer[index] == '_')
			charFound = true;

		index++;
	}

	// If only one variable is shown after the name then it can be any type other than vector since vector should have 3.
	switch (numComponents)
	{
	case 1:
		// If there are any characters then it is a string.
		if (charFound)
			type = STRING_VAR;
		else
			type = INT_VAR;

		// If there is a decimal and no chars then it's a float.
		if (decimalFound == true && charFound == false)
			type = FLOAT_VAR;
		break;
	case 3:
		// Since vecs are just floats, if we see any characters in the group then it must be a string.
		if (charFound)
			type = STRING_VAR;
		else
			type = VECTOR_VAR;
		break;
	default:
		// If there are more than 1 word after the name then it must be a string since they can be spaces.
		if (numComponents > 0)
			type = STRING_VAR;
		break;
	}

	return type;
}

/*
 * @param:
		要设置的属性类型
		新属性的属性名
		赋给该属性的值
 */
void CVariable::SetData(int t, char * n, void * data)
{
	if (!name) return;

	// Set this variables name then set the type and data.
	memcpy(name, n, strlen(n));
	name[strlen(n)] = '\0';
	SetData(t, data);
}

/*
 * @param: 要设置的类型和数据
 */
void CVariable::SetData(int t, void * data)
{
	stVector *vec = nullptr;
	int len = 0;

	// Depending on the type will depend where the value is stored.
	switch (t)
	{
	case INT_VAR:
		intVal = (int)data;
		break;
	case FLOAT_VAR:
		floatVal = *(float*)data;
		break;
	case STRING_VAR:
		len = strlen((char*)data);
		stringVal = new char[len + 1];
		memcpy(stringVal, (char*)data, len);
		stringVal[len] = '\0';
		break;
	case VECTOR_VAR:
		vec = (stVector *)data;
		vecVal.x = vec->x;
		vecVal.y = vec->y;
		vecVal.z = vec->z;
		break;
	default:
		// If we get here then it is a NULL variable.
		return;
		break;
	}

	type = t;
}

CPropertyScript::CPropertyScript() : variableList(nullptr), m_totalVars(0), currentLineChar(0)
{

}

CPropertyScript::~CPropertyScript()
{
	// Release all resources.
	Shutdown();
}

/*
 * @param: 加载的脚本文件名
 * @return: 加载成功，返回true；否则，返回false
 */
bool CPropertyScript::LoadScriptFile(char * filename)
{
	int totalScriptLines = 0;
	ifstream input, input2;
	char tempLine[256];
	char varName[128];
	char param[3072];
	int type = 0;

	// Open the file to get the number of lines from it.
	input.open(filename);
	if (!input.is_open())
		return false;

	// Clear all previous data.
	Shutdown();

	// Open and get number of lines;
	while (!input.eof())
	{
		input.getline(tempLine, 256, '\n');
		totalScriptLines++;
	}

	input.close();

	// Open it this time to get the variables out.
	input2.open(filename);
	if (!input2.is_open())
		return false;

	// Loop through each line of the script and get all variables.
	for (int i = 0; i < totalScriptLines; ++i)
	{
		// Reset line counter to the beginning.
		currentLineChar = 0;

		// Read the entire line from the file.
		input2.getline(tempLine, 256, '\n');
		tempLine[strlen(tempLine)] = '\0';

		// Check if this is a comment. If not keep going.
		if (tempLine[0] != '#')
		{
			// Read the name then determine the type.
			ParseNext(tempLine, varName);
			type = DetermineType(currentLineChar, tempLine);

			/*
			 * Depending on the type will depend on how many words we need to read after the name. 
			 * For ints we need 1, vectors 3, string 1, etc. Once we get the data we convert int to the type
			 * we need and set it to the variable.
			 */
			if (type == INT_VAR)
			{
				if (IncreaseVariableList())
				{
					ParseNext(tempLine, param);

					variableList[m_totalVars].SetData(INT_VAR, varName, (void*)atoi(param));

					m_totalVars++;
				}
			}
			else if (type == FLOAT_VAR)
			{
				if (IncreaseVariableList())
				{
					float fVal = 0;
					ParseNext(tempLine, param);
					fVal = (float)atof(param);
					variableList[m_totalVars].SetData(FLOAT_VAR, varName, (void*)&fVal);

					m_totalVars++;
				}
			}
			else if (type == STRING_VAR)
			{
				if (IncreaseVariableList())
				{
					ParseNext(tempLine, param);

					variableList[m_totalVars].SetData(STRING_VAR, varName, (void*)param);

					m_totalVars++;
				}
			}
			else if (type == VECTOR_VAR)
			{
				if (IncreaseVariableList())
				{
					stVector vecVal;

					ParseNext(tempLine, param);

					vecVal.x = (float)atof(param);
					ParseNext(tempLine, param);
					vecVal.y = (float)atof(param);
					ParseNext(tempLine, param);
					vecVal.z = (float)atof(param);

					variableList[m_totalVars].SetData(VECTOR_VAR, varName, (void*)&vecVal);

					m_totalVars++;
				}
			}
		}
	}

	// Close file, return true.
	input2.close();
	return true;
}

// 增加 CVariable 链表数组
bool CPropertyScript::IncreaseVariableList()
{
	if (!variableList)
	{
		variableList = new CVariable[1];
		if (!variableList)
			return false;
	}
	else
	{
		CVariable *temp;
		temp = new CVariable[m_totalVars + 1];
		if (!temp)
			return false;

		memcpy(temp, variableList, sizeof(CVariable)*m_totalVars);

		delete[] variableList;
		variableList = temp;
	}

	return true;
}

/*
 * @brief: 提取文件中的一行，并获取改行开始到下一个分隔符之前的下一组文本
 *		使用类的当前字符计数，可从单行中提取多个值.
 * @param:
		要处理的行、存储出现在下一个分隔符之前的改组文本的地址
 */
void CPropertyScript::ParseNext(char * tempLine, char * varName)
{
	int commandSize = 0;
	int paramSize = 0;

	// Error checking.
	if (!tempLine || !varName) return;

	// Init string.
	varName[0] = '\0';

	// Loop until you find a space or newline.
	while (currentLineChar < (int)strlen(tempLine))
	{
		if(tempLine[currentLineChar] == ' ' || tempLine[currentLineChar] == '\n')
			break;

		// Save the text in the array.
		varName[paramSize] = tempLine[currentLineChar];

		paramSize++;
		currentLineChar++;
	}

	// End the string and move the line char past the next space. If no space then the system will move to the next line.
	varName[paramSize] = '\0';
	currentLineChar++;
}

/*
 * @brief: 手动为链表添加属性
 * @param:
 *		属性名、要添加的属性类型、要设置的属性值
 * @return:
 *		true - 成功; false - 增加链表数组遇到错误
 */
bool CPropertyScript::AddVariable(char * name, int t, void * val)
{
	// We can use this to see if the variable exist already.
	if (!SetVariable(name, t, val))
	{
		if (!IncreaseVariableList())
			return false;

		// Set the variables data then add to the counter.
		variableList[m_totalVars].SetData(t, name, val);
		m_totalVars++;
	}

	return true;
}

/*
 * @brief: 更改现有的属性
 * @param:
 *		属性名、要设置的属性类型、要设置的属性值
 * @return:
 *		true - 成功; false - 找不到要用的属性
 */
bool CPropertyScript::SetVariable(char * name, int t, void * val)
{
	// Loop through the list and compare names. If we find the variable set its data.
	for (int i = 0; i < m_totalVars; i++)
	{
		if (stricmp(variableList[i].GetName(), name) == 0)
		{
			variableList[i].SetData(t, val);
			return true;
		}
	}

	return false;
}

/*
 * @brief: 获取Int属性
 * @param: 属性名
 */
int CPropertyScript::GetVariableAsInt(char * name)
{
	 // Loop through the list and compare names. If we find the variable return its data.
	for (int i = 0; i < m_totalVars; ++i)
	{
		if (stricmp(variableList[i].GetName(), name) == 0)
			return variableList[i].GetDataAsInt();
	}

	return 0;
}

/*
* @brief: 获取float属性
* @param: 属性名
*/
float CPropertyScript::GetVariableAsFloat(char * name)
{
	// Loop through the list and compare names. If we find the variable return its data.
	for (int i = 0; i < m_totalVars; ++i)
	{
		if (stricmp(variableList[i].GetName(), name) == 0)
			return variableList[i].GetDataAsFloat();
	}

	return 0.0f;
}

/*
* @brief: 获取字符串属性
* @param: 属性名
*/
char * CPropertyScript::GetVariableAsString(char * name)
{
	// Loop through the list and compare names. If we find the variable return its data.
	for (int i = 0; i < m_totalVars; ++i)
	{
		if (stricmp(variableList[i].GetName(), name) == 0)
			return variableList[i].GetDataAsString();
	}

	return nullptr;
}

const stVector* CPropertyScript::GetVariableAsVector(char * name)
{
	// Loop through the list and compare names. If we find the variable return its data.
	for (int i = 0; i < m_totalVars; ++i)
	{
		if (stricmp(variableList[i].GetName(), name) == 0)
			return &(variableList[i].GetDataAsVector());
	}

	return nullptr;
}

/*
 * @brief: 释放系统使用的全部动态内存
 */
void CPropertyScript::Shutdown()
{
	// Delete list.
	if (variableList)
	{
		delete[] variableList;
		variableList = nullptr;
	}
}

#pragma warning(pop)
