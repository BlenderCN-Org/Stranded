#ifndef __COMMAND_SCRIPT_H__
#define __COMMAND_SCRIPT_H__

/*
 * 命令脚本系统
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

	// 加载脚本文件
	bool LoadScriptFile(const char* filename);

	// 提取命令
	void ParseCommand(char* destCommand);
	// 提取有效脚本文件中不同类型值(字符串、整数等)
	void ParseStringParam(char* destString);
	bool ParseBoolParam();
	int ParseIntParam();
	float ParseFloatParam();

	// 清理系统
	void Shutdown();

	// 移动到开始位置
	void MoveToStart();
	// 移动到下一行
	void MoveToNextLine();

	// 返回所处脚本文件的当前行号
	int GetCurrentLineNum();
	
	// 获取整个脚本文件中的总行数
	int GetTotalLines();
	
	// 进行行测试，测试这一行是否是注释('#'开始)
	bool IsLineComment();

private:
	// 脚本文件总行数
	int totalScriptLines;
	// 当前行位置
	int currentLine;
	// 所处当前行中的当前字符
	int currentLineChar;
	// 整个文件自身[数组构成的数组，每个数组代表一行]
	char **m_script;
};

#endif
