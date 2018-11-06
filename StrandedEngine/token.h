#ifndef __TOKEN_H__
#define __TOKEN_H__

/*
 * 令牌流
 * 分开的每一组文本称为一个令牌
 */
class CToken
{
public:
	CToken() : m_length(0), m_startIndex(0), m_endIndex(0), m_data(nullptr) {}
	~CToken() { Shutdown(); }

	// 返回该文件的头部
	void Reset() { m_startIndex = m_endIndex = 0; }

	void SetTokenStream(char *data);

	bool GetNextToken(char *buffer);
	bool GetNextToken(char *token, char *buffer);

	// 移动到文件的下一行
	bool MoveToNextLine(char *buffer);

	// 清理所有使用的对象
	void Shutdown();
private:
	// 整个文件的长度
	int m_length;
	// 跟踪文件的起始索引、终止索引
	int m_startIndex, m_endIndex;
	// 文件自身
	char *m_data;
};

#endif // __TOKEN_H__
