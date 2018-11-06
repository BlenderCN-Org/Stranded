#coding=GB2312
#!/usr/bin/env python


print("Hello World!")

x = 3					# an integer stored (in variable x)
f = 3.1415926			# a floating real point (in variable f)
name = "Python"			# a string
z = complex(2, 3)		# (2+3i) a complex number. consists of real and imaginary part.
# del 语句删除引用的那一个数字对象
# del var1[,var2[,var3[....,varN]]]]
del x

str1 = '''Hello World!'''
print(str1 * 2)          # Prints string two times
print(str1 + "TEST")     # Prints concatenated string

# Python列表
# 存储在一个列表中的值可以使用切片操作符([]和[:])进行访问：列表的0索引位置为起点位置，并在以-1 结束。 加号(+)号是列表中连接运算，星号(*)是重复操作符。
list1 = [ 'abcd', 786, 2.23, 'john', 70.2 ]
print(list1[2:])

# Python元组
# 元组是另一个序列数据类型，它类似于列表。元组中使用单个逗号来分隔每个值。不像列表，元组的值是放列在圆括号中。
# 列表和元组之间的主要区别是：列表是包含在方括号[]中，并且它们的元素和大小是可以改变的，而元组元素是括在括号()中，不能进行更新。元组可以被认为是只读的列表。
tuple1 = ( 'abcd', 786, 2.23, 'john', 70.2 )

# Python字典
# 一种哈希表类型。它们工作的方式就类似在Perl中关联数组或哈希、键-值对。字典的键可以是几乎任何Python类型，但通常是数字或字符串。另一方面，它的值可以是任意Python对象。
# 字典是用大括号括起来({})，并且值可分配并使用方括号([])来访问。
# 字典中元素是无序
dict1 = {}
dict1['one'] = "This is one"
dict1[2] = "This is two"

tinydict = { 'name': 'john', 'code': 6734, 'dept': 'sales' }

print(dict1['one'])
print(dict1[2])
print(tinydict)
print(tinydict.keys())
print(tinydict.values())

# 数据类型转换
# int(x [,base])        转换x为整数。x是字符串则 base 为指定的基数
# float(x)              转换x为一个浮点数
# complex(real [,imag]) 创建一个复数
# str(x)                转换对象x为字符串表示
# repr(x)               转换对象x为表达式字符串
# eval(str)             计算一个字符串，并返回一个对象
# tuple(s)              转换s为一个元组
# list(s)               转换s为一个列表
# set(s)                转换s为一个集合
# dict(d)               创建一个字典。 d必须是(键，值)元组序列
# frozenset(s)          转换s为冷冻集
# chr(x)                将一个字符转换为整数
# unichr(x)             Unicode字符转换为整数
# ord(x)                单个字符其转换为整数值
# hex(x)                十六进制字符串转换为整数
# oct(x)                转换整数成为八进制字符串

# Python操作符成员
# 在一个序列Python的成员操作符来测试成员，如字符串，列表或元组。
# in        如果在指定的顺序中找到变量，计算结果为true，否则为 false
# not in    如果在指定的顺序中不能找到变量，计算结果为true，否则为 false

# Python标识操作符
# 标识运算符比较两个对象的存储单元。
# is        如果操作符两侧是相同的对象，计算结果为true，否则结果为 false
# is not    如果操作符两侧的变量是相同的对象，计算结果为false，否则 true

# range()函数
# 一个迭代从0开始到 n-1 的整数
for var in list(range(5)):
    print(var)

# 字符串格式化操作
print("My name is %s and weight is %d kg!" %('Zara', 21))

