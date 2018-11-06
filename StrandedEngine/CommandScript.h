#ifndef __COMMAND_SCRIPT_H__
#define __COMMAND_SCRIPT_H__

/*
 * ����ű�ϵͳ
 */

#define MAX_COMMAND_SIZE		32
#define MAX_LINE_SIZE			3072
#define MAX_PARAM_SIZE			2048

struct stCmdSVector
{
	stCmdSVector() : x(0), y(0), z(0)
	{}

	float x, y, z;
};

class CCommandScript
{
public:
	CCommandScript();
	~CCommandScript();

	// ���ؽű��ļ�
	bool LoadScriptFile(const char* filename);

	// ��ȡ����
	void ParseCommand(char* destCommand);
	// ��ȡ��Ч�ű��ļ��в�ͬ����ֵ(�ַ�����������)
	void ParseStringParam(char* destString);
	bool ParseBoolParam();
	int ParseIntParam();
	float ParseFloatParam();

	// ����ϵͳ
	void Shutdown();

	// �ƶ�����ʼλ��
	void MoveToStart();
	// �ƶ�����һ��
	void MoveToNextLine();

	// ���������ű��ļ��ĵ�ǰ�к�
	int GetCurrentLineNum();
	
	// ��ȡ�����ű��ļ��е�������
	int GetTotalLines();
	
	// �����в��ԣ�������һ���Ƿ���ע��('#'��ʼ)
	bool IsLineComment();

private:
	// �ű��ļ�������
	int totalScriptLines;
	// ��ǰ��λ��
	int currentLine;
	// ������ǰ���еĵ�ǰ�ַ�
	int currentLineChar;
	// �����ļ�����[���鹹�ɵ����飬ÿ���������һ��]
	char **m_script;
};

#endif
