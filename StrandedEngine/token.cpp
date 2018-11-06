#include "token.h"
#include <string.h>

#pragma warning(push)
#pragma warning(disable:4996)

/*
 * @brief: 检查字符是否为有效的标识符类型
 * @return: 是，返回true(真)；否则，返回false(假)
 */
static bool IsValidIdentifier(char c)
{
	// It is valid if it falls within one of these ranges.
	if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '"' || c == '/' || c == '(' || c == ')' || c == '-' || c == '.')
		return true;

	return false;
}

void CToken::SetTokenStream(char *data)
{
	Shutdown();

	m_length = strlen(data);
	m_data = new char[(m_length+1) * sizeof(char)];
	
	strcpy(m_data, data);
	
	m_data[m_length] = '\0';
}

/*
 * @brief: 返回文件中的令牌
 * @param:保存令牌的缓存
 */
bool CToken::GetNextToken(char *buffer)
{
	bool inString = false;
	m_startIndex = m_endIndex;

	while (m_startIndex < m_length && ((m_data[m_startIndex] == ' ' || m_data[m_startIndex] == '\t') || inString))
	{
		if (m_data[m_startIndex] == '"')
			inString = !inString;
		m_startIndex++;
	}

	m_endIndex = m_startIndex + 1;

	if (m_startIndex < m_length)
	{
		bool valid = true;

		if (IsValidIdentifier(m_data[m_startIndex]))
		{
			while (IsValidIdentifier(m_data[m_endIndex]) || m_data[m_endIndex] == '.')
				m_endIndex++;
		}
		else
			valid = false;

		if (buffer != nullptr)
		{
			if (valid)
			{
				strncpy(buffer, m_data + m_startIndex, m_endIndex - m_startIndex);

				buffer[m_endIndex - m_startIndex] = '\0';

				if (strcmp(buffer, "\n") == 0)
					buffer[0] = '\0';
			}
			else
				buffer[0] = '\0';
		}

		return true;
	}

	return false;
}

/*
 * @brief: 返回文件中的令牌
 * @param: 保存令牌的缓存、保存下一个令牌的缓存
 */
bool CToken::GetNextToken(char *token, char *buffer)
{
	char tok[256];

	while (GetNextToken(tok))
	{
		if (stricmp(tok, token) == 0)
			return GetNextToken(buffer);
	}

	return false;
}

bool CToken::MoveToNextLine(char *buffer)
{
	if (m_startIndex < m_length && m_endIndex < m_length)
	{
		m_startIndex = m_endIndex;

		while (m_endIndex < m_length && (m_data[m_endIndex] != '\n' && m_data[m_endIndex] != '\r' && m_data[m_endIndex] != '\0'))
			m_endIndex++;

		if (m_endIndex - m_startIndex >= 511)
			return false;

		if (buffer != nullptr)
		{
			strncpy(buffer, m_data + m_startIndex, m_endIndex - m_startIndex);
			buffer[m_endIndex - m_startIndex] = '\0';
		}
	}
	else
		return false;

	return true;
}

void CToken::Shutdown()
{
	if (m_data)
	{
		delete[] m_data;
		m_data = nullptr;
	}

	m_length = m_startIndex = m_endIndex = 0;
}

#pragma warning(pop)
