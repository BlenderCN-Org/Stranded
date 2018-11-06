#ifndef __TOKEN_H__
#define __TOKEN_H__

/*
 * ������
 * �ֿ���ÿһ���ı���Ϊһ������
 */
class CToken
{
public:
	CToken() : m_length(0), m_startIndex(0), m_endIndex(0), m_data(nullptr) {}
	~CToken() { Shutdown(); }

	// ���ظ��ļ���ͷ��
	void Reset() { m_startIndex = m_endIndex = 0; }

	void SetTokenStream(char *data);

	bool GetNextToken(char *buffer);
	bool GetNextToken(char *token, char *buffer);

	// �ƶ����ļ�����һ��
	bool MoveToNextLine(char *buffer);

	// ��������ʹ�õĶ���
	void Shutdown();
private:
	// �����ļ��ĳ���
	int m_length;
	// �����ļ�����ʼ��������ֹ����
	int m_startIndex, m_endIndex;
	// �ļ�����
	char *m_data;
};

#endif // __TOKEN_H__
