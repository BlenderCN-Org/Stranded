#include "CommandScript.h"
#include <fstream>

using std::ifstream;
using std::filebuf;
using std::ios;

#pragma warning(push)
#pragma warning(disable:4996)

CCommandScript::CCommandScript() : totalScriptLines(0), currentLine(0), currentLineChar(0), m_script(nullptr)
{

}

CCommandScript::~CCommandScript()
{
	Shutdown();
}

/*
 * @brief: 将整个脚本文件加载到内存中
 * @param: 脚本文件名
 */
bool CCommandScript::LoadScriptFile(const char* filename)
{
	ifstream input, input2;
	char tempLine[MAX_LINE_SIZE];

	// 确定整个文件的总行数
	input.open(filename);

	if (!input.is_open())
		return false;

	Shutdown();

	// Open and get number of lines.
	while (!input.eof())
	{
		input.getline(tempLine, MAX_LINE_SIZE, '\n');
		totalScriptLines++;
	}

	input.close();

	// Re-open.
	input2.open(filename);
	if (!input2.is_open())
		return false;

	// Allocate every line of text.
	m_script = new char*[totalScriptLines];

	// Read each line in.
	for (int i = 0; i < totalScriptLines; ++i)
	{
		m_script[i] = new char[MAX_LINE_SIZE + 1];
		input2.getline(m_script[i], MAX_LINE_SIZE, '\n');
	}

	input2.close();

	return true;
}

/*
 * @brief: 从脚本文件提取命令
 * @param: 在函数完成工作时保存的命令
 */
void CCommandScript::ParseCommand(char* destCommand)
{
	// This function will take the first word of the current line and save it in descCommand.

	int commandSize = 0;

	// If destcommand is NULL, or if we run out of lines, or at the end of the current line then we return.
	if (!destCommand)
		return;
	if (currentLine >= totalScriptLines)
		return;
	if (currentLineChar >= (int)strlen(m_script[currentLine]))
		return;

	// Init string.
	destCommand[0] = '\0';

	// Since commands start each line we can say that if there is a # at the start of the line, then this is a comment.
	if (IsLineComment())
	{
		destCommand[0] = '#';
		destCommand[0] = '\0';
		return;
	}

	// Loop through every character until you find a space or newline. That means we are at the end of a command.
	while (currentLineChar < (int)strlen(m_script[currentLine]))
	{
		if (m_script[currentLine][currentLineChar] == ' ' || m_script[currentLine][currentLineChar] == '\n')
			break;

		// Save the next in the array.
		destCommand[commandSize] = m_script[currentLine][currentLineChar];

		commandSize++;
		currentLineChar++;
	}

	// Skip next space or newline.
	currentLineChar++;
	destCommand[commandSize] = '\0';
}

/*
 * @brief: 获取文件中的下一组文本，直到遇到分隔符[空格符、换行符]为止
 */
void CCommandScript::ParseStringParam(char* destString)
{
	// This function will take a text inside " and " and save it to destString.

	int paramSize = 0;
	bool endQuoteFound = false;

	// If destcommand is NULL, or if we run out of lines, or at the end of the current line then we return.
	if (!destString)
		return;
	if (currentLine >= totalScriptLines)
		return;
	if (currentLineChar >= (int)strlen(m_script[currentLine]))
		return;

	// Initialize string.
	destString[0] = '\0';

	// Skip beginning quote.
	currentLineChar++;

	// Loop through every character until you find an end quote or newline. That means we are at the end of a string.
	while (currentLineChar < (int)strlen(m_script[currentLine]))
	{
		if (m_script[currentLine][currentLineChar] == '"')
		{
			endQuoteFound = true;
			break;
		}

		if(m_script[currentLine][currentLineChar] == '\n')
			break;

		// Save the next in the array.
		destString[paramSize] = m_script[currentLine][currentLineChar];

		paramSize++;
		currentLineChar++;
	}

	// Skip end quotes and next space or newline. In this system we don't allow strings to take up multiple lines. You can simple have multiple print string commands instead.
	if (endQuoteFound)
		currentLineChar += 2;
	else
		currentLineChar++;

	destString[paramSize] = '\0';
}

bool CCommandScript::ParseBoolParam()
{
	// This function will get the next text and will return true if this text is "true" or false if it is "false".

	char string[MAX_PARAM_SIZE];
	int paramSize = 0;

	// If we run out of lines, or at the end of the current line then we return.
	if (currentLine >= totalScriptLines)
		return false;
	if (currentLineChar >= (int)strlen(m_script[currentLine]))
		return false;

	// Loop through every character until you find a space or newline. That means we are at the end of a variable.
	while (currentLineChar < (int)strlen(m_script[currentLine]))
	{
		if(m_script[currentLine][currentLineChar] == ' ' || m_script[currentLine][currentLineChar] == '\n')
			break;

		// Save the text in the array.
		string[paramSize] = m_script[currentLine][currentLineChar];
		paramSize++;
		currentLineChar++;
	}

	// Skip next space or newline.
	currentLineChar++;
	string[paramSize] = '\0';

	if (stricmp(string, "true") == 0)
		return true;
	
	return false;
}

int CCommandScript::ParseIntParam()
{
	// This functio will take the next and convert it to a int that is returned.

	char string[MAX_PARAM_SIZE];
	int paramSize = 0;

	// If we run out of lines, or at the end of the current line then we return.
	if (currentLine >= totalScriptLines)
		return 0;
	if (currentLineChar >= (int)strlen(m_script[currentLine]))
		return 0;

	// Loop through every character until you find a space or newline. That means we are at the end of a variable.
	while (currentLineChar < (int)strlen(m_script[currentLine]))
	{
		if (m_script[currentLine][currentLineChar] == ' ' || m_script[currentLine][currentLineChar] == '\n')
			break;

		// Save the text in the array.
		string[paramSize] = m_script[currentLine][currentLineChar];
		paramSize++;
		currentLineChar++;
	}

	// Skip next space or newline.
	currentLineChar++;
	string[paramSize] = '\0';

	return atoi(string);
}

float CCommandScript::ParseFloatParam()
{
	// This function will take the next text and convert it to a float that is returned.

	char string[MAX_PARAM_SIZE];
	int paramSize = 0;

	// If we run out of lines, or at the end of the current line then we return.
	if (currentLine >= totalScriptLines)
		return 0.0f;
	if (currentLineChar >= (int)strlen(m_script[currentLine]))
		return 0.0f;

	// Loop through every character until you find a space or newline. That means we are at the end of a variable.
	while (currentLineChar < (int)strlen(m_script[currentLine]))
	{
		if (m_script[currentLine][currentLineChar] == ' ' || m_script[currentLine][currentLineChar] == '\n')
			break;

		// Save the text in the array.
		string[paramSize] = m_script[currentLine][currentLineChar];
		paramSize++;
		currentLineChar++;
	}

	// Skip next space or newline.
	currentLineChar++;
	string[paramSize] = '\0';

	return (float)atof(string);
}

void CCommandScript::Shutdown()
{
	if (m_script)
	{
		// Delete every line in the script file.
		for (int i = 0; i < totalScriptLines; ++i)
		{
			if (m_script[i])
			{
				delete[] m_script[i];
				m_script[i] = nullptr;
			}
		}

		delete[] m_script;
		m_script = nullptr;
	}

	// Reset variables.
	totalScriptLines = 0;
	currentLineChar = 0;
	currentLine = 0;
}

void CCommandScript::MoveToStart()
{
	currentLine = 0;
	currentLineChar = 0;
}

void CCommandScript::MoveToNextLine()
{
	currentLine++;
	currentLineChar = 0;
}

int CCommandScript::GetCurrentLineNum()
{
	return currentLine;
}

int CCommandScript::GetTotalLines()
{
	return totalScriptLines;
}

bool CCommandScript::IsLineComment()
{
	// If the first character of a line a # then the line must be commented out.
	if (m_script[currentLine][0] == '#')
		return true;

	return false;
}

#pragma warning(pop)
