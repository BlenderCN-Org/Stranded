#ifndef __STATE_H__
#define __STATE_H__

/*
 * 1.���е�stateӦ�ñ�һ���ࣨState Manager Class������:
	state֮�����ת��ת���Ƿǳ����ӵģ���ʱһЩstate����Ҫ��ת��Ŀ��state�м�ʮ����
	���ʱ��������Ҫһ�������ࣨState Manager ����ͳһ�Ĺ�����Щstate���л���
	����Ŀ��state�ĳ�ʼ����������תstate�Ľ��������Լ�һЩstate�乲�����ݵĴ洢�ʹ���
	��������Manager Ϊ�����࣬����˵��һ���м��࣬��ʵ����state֮��Ľ��磬
	ʹ�ø���state֮�䲻��֪��target state�ľ�����Ϣ����ֻҪ��Manager������ת�Ϳ����ˡ�
	ʹ�ø���state��ģ�黯���ã����ӵ����
 *
 * 2.���е�state��Ӧ�ô�һ��state����̳�:
	��ȻstateҪ�̸�һ��manager��������ô��Ȼ�ģ���Щstate��Ӧ�ô�һ������̳�������
	����manager������Ҫ֪���ܶ��������Ϣ��һ�������managerֻҪֻҪ����һ�������Ļ����ָ��Ϳ����ˡ�
	���⣬���ǻ�����ͳһ�İ�state��һЩ���е����Է�������
 *
 * 3.stateӦ��ʵ��Ϊһ��singleton:
	state������Ҫ���Ǳ����룬�������ܻ���ɹ����ϵĻ��ң�state��Դ������Ҳ��Ӧ�ÿ���������У�
	��ʵ�ϣ�state������Ȩ��Ӧ��ֻ�� Manager���У���������ֻ��һ�Ρ�������������£�
	state�Ĺ��캯���ƺ�Ӧ�ñ�����Ϊprotected or private ����ManagerӦ�ñ�����Ϊstate����Ԫ
 *
 * 4.Ӧ����һ��stateô��
	state����˵��if-else��һ�����Ʒ�����˵��������state��������ĳ�����if-else�������ʧ����Ӱ���٣�
	���ǣ��Ⲣ����������state����״̬��Ԥ֪������·ǳ���Ч�����Ƕ���state����Ԥ֪���������Ƶ�state����̫�ࡣ
	�����state�����class�����ȹ�ϸ�����򷴶�����ࡣ������������£���Ӧ�ÿ���ʹ��if-else����������state��

	һ�ֽ��������������Manager���ݸ�stateһ��state��������state�л���֪���û��Ĳ�����ͼ��
	�����������������state֪���û��������ĳһ���ڵ㣬��state�������state�����������û��Ĳ���������˵��
	state�õ������û�������ĳһ�����index ,��stateֻҪд
	points[index].moveTo(points[index].getX()+offset_x , points[index].getY()+offset_y)��
	�Ϳ��ԣ��Ӷ�������state������ֵ����⡣
 */

class CContext;		// ǰ������
class CState
{
public:
	CState();
	virtual ~CState();

	virtual void OperationInterface(CContext*) = 0;
	virtual void OperationChangeState(CContext*) = 0;

protected:
	bool ChangeState(CContext* con, CState* st);

};

class CConcreteStateA : public CState
{
public:
	CConcreteStateA();
	virtual ~CConcreteStateA();

	void OperationInterface(CContext* con);
	void OperationChangeState(CContext* con);
};

class CConcreteStateB : public CState
{
public:
	CConcreteStateB();
	virtual ~CConcreteStateB();

	void OperationInterface(CContext* con);
	void OperationChangeState(CContext* con);
};

#endif
