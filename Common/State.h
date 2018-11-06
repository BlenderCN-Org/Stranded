#ifndef __STATE_H__
#define __STATE_H__

/*
 * 1.所有的state应该被一个类（State Manager Class）管理:
	state之间的跳转和转换是非常复杂的，有时一些state可能要跳转的目标state有几十个，
	这个时候我们需要一个管理类（State Manager ）来统一的管理这些state的切换，
	例如目标state的初始化和申请跳转state的结束处理，以及一些state间共享数据的存储和处理。
	与其称这个Manager 为管理类，不如说是一个中间类，它实现了state之间的解隅，
	使得各个state之间不比知道target state的具体信息，而只要向Manager申请跳转就可以了。
	使得各个state的模块化更好，更加的灵活
 *
 * 2.所有的state都应该从一个state基类继承:
	既然state要教给一个manager来管理，那么自然的，这些state都应该从一个父类继承下来，
	这样manager并不需要知道很多子类的信息，一个最单纯的manager只要只要管理一个这样的基类的指针就可以了。
	另外，我们还可以统一的把state的一些共有的属性放在这里
 *
 * 3.state应该实现为一个singleton:
	state并不需要总是被申请，这样可能会造成管理上的混乱，state资源的申请也不应该可以任意进行，
	事实上，state的申请权限应该只有 Manager才有，并且有且只有一次。在这样的情况下，
	state的构造函数似乎应该被声明为protected or private ，而Manager应该被声明为state的友元
 *
 * 4.应该做一个state么？
	state可以说是if-else的一种替代品，极端的情况下面state可以让你的程序中if-else程序块消失得无影无踪，
	但是，这并不是银弹。state对于状态可预知的情况下非常有效，但是对于state不可预知，或者相似的state数量太多。
	过多的state会造成class的粒度过细，程序反而不简洁。在这样的情况下，你应该考虑使用if-else程序块来替代state。

	一种解决方法是我利用Manager传递给state一个state参数，让state有机会知道用户的操作意图，
	在这个例子里面是让state知道用户打算操作某一个节点，而state根据这个state参数来处理用户的操作，比如说，
	state得到的是用户操作的某一个点的index ,而state只要写
	points[index].moveTo(points[index].getX()+offset_x , points[index].getY()+offset_y)；
	就可以，从而避免了state过多出现的问题。
 */

class CContext;		// 前置声明
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
