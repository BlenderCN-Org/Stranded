#ifndef __PROPERTY_SCRIPT_H__
#define __PROPERTY_SCRIPT_H__

/*
 * ���Խű�
 */

// ��ͬ���͵�����
enum enVarType
{
	NULL_VAR = 0,
	INT_VAR,
	FLOAT_VAR,
	STRING_VAR,
	VECTOR_VAR
};

// ʸ������ṹ
struct stVector
{
	stVector() : x(0), y(0), z(0)
	{}

	float x, y, z;
};

// ��������
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

	// ͨ��void*
	void SetData(int t, char* n, void* data);
	void SetData(int t, void* data);

	char* GetName() { return name; }
	int GetType() { return type; }

	int GetDataAsInt() { return intVal; }
	float GetDataAsFloat() { return floatVal; }
	char* GetDataAsString() { return stringVal; }
	const stVector& GetDataAsVector() const { return vecVal; }

private:
	// ������
	char name[128];
	// ��������
	int type;

	// ֧��ÿ�����͵ĳ�Ա����
	int intVal;
	float floatVal;
	char *stringVal;
	stVector vecVal;
};

// ���Խű�ϵͳ
class CPropertyScript
{
public:
	CPropertyScript();
	~CPropertyScript();

	// ���������ļ�
	bool LoadScriptFile(char* filename);

private:
	bool IncreaseVariableList();
	// �����ļ����ı�����
	void ParseNext(char* tempLine, char* varName);

public:
	// �ֶ���Ӻ����ñ���
	bool AddVariable(char* name, int t, void* val);
	bool SetVariable(char* name, int t, void* val);

	// ���ݱ�������ȡ����ֵ
	int GetVariableAsInt(char* name);
	float GetVariableAsFloat(char* name);
	char* GetVariableAsString(char* name);
	const stVector* GetVariableAsVector(char* name);

	void Shutdown();

private:
	// ��������
	CVariable* variableList;
	// �����е���������
	int m_totalVars;
	// �������ڶ�ȡ�ĵ�ǰ�ַ�������[��ȡ�ļ�ר��]
	int currentLineChar;
};

#endif
