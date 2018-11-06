#ifndef __PROPERTY_SCRIPT_H__
#define __PROPERTY_SCRIPT_H__

/*
 * 属性脚本
 */

// 不同类型的属性
enum enVarType
{
	NULL_VAR = 0,
	INT_VAR,
	FLOAT_VAR,
	STRING_VAR,
	VECTOR_VAR
};

// 矢量对象结构
struct stVector
{
	stVector() : x(0), y(0), z(0)
	{}

	float x, y, z;
};

// 单个属性
class CVariable
{
public:
	CVariable() : type(0), floatVal(0), intVal(0), stringVal(0)
	{
		name[0] = '\0';
		vecVal.x = vecVal.y = vecVal.z = 0;
	}

	~CVariable()
	{
		if (stringVal)
		{
			delete[] stringVal;
			stringVal = nullptr;
		}
	}

	// 通用void*
	void SetData(int t, char* n, void* data);
	void SetData(int t, void* data);

	char* GetName() { return name; }
	int GetType() { return type; }

	int GetDataAsInt() { return intVal; }
	float GetDataAsFloat() { return floatVal; }
	char* GetDataAsString() { return stringVal; }
	const stVector& GetDataAsVector() const { return vecVal; }

private:
	// 属性名
	char name[128];
	// 属性类型
	int type;

	// 支持每种类型的成员变量
	int intVal;
	float floatVal;
	char *stringVal;
	stVector vecVal;
};

// 属性脚本系统
class CPropertyScript
{
public:
	CPropertyScript();
	~CPropertyScript();

	// 加载属性文件
	bool LoadScriptFile(char* filename);

private:
	bool IncreaseVariableList();
	// 解析文件中文本单词
	void ParseNext(char* tempLine, char* varName);

public:
	// 手动添加和设置变量
	bool AddVariable(char* name, int t, void* val);
	bool SetVariable(char* name, int t, void* val);

	// 根据变量名获取变量值
	int GetVariableAsInt(char* name);
	float GetVariableAsFloat(char* name);
	char* GetVariableAsString(char* name);
	const stVector* GetVariableAsVector(char* name);

	void Shutdown();

private:
	// 属性链表
	CVariable* variableList;
	// 链表中的属性总数
	int m_totalVars;
	// 跟踪正在读取的当前字符计数器[读取文件专用]
	int currentLineChar;
};

#endif
