#coding=GB2312
#!/usr/bin/env python


print("Hello World!")

x = 3					# an integer stored (in variable x)
f = 3.1415926			# a floating real point (in variable f)
name = "Python"			# a string
z = complex(2, 3)		# (2+3i) a complex number. consists of real and imaginary part.
# del ���ɾ�����õ���һ�����ֶ���
# del var1[,var2[,var3[....,varN]]]]
del x

str1 = '''Hello World!'''
print(str1 * 2)          # Prints string two times
print(str1 + "TEST")     # Prints concatenated string

# Python�б�
# �洢��һ���б��е�ֵ����ʹ����Ƭ������([]��[:])���з��ʣ��б��0����λ��Ϊ���λ�ã�������-1 ������ �Ӻ�(+)�����б����������㣬�Ǻ�(*)���ظ���������
list1 = [ 'abcd', 786, 2.23, 'john', 70.2 ]
print(list1[2:])

# PythonԪ��
# Ԫ������һ�������������ͣ����������б�Ԫ����ʹ�õ����������ָ�ÿ��ֵ�������б�Ԫ���ֵ�Ƿ�����Բ�����С�
# �б��Ԫ��֮�����Ҫ�����ǣ��б��ǰ����ڷ�����[]�У��������ǵ�Ԫ�غʹ�С�ǿ��Ըı�ģ���Ԫ��Ԫ������������()�У����ܽ��и��¡�Ԫ����Ա���Ϊ��ֻ�����б�
tuple1 = ( 'abcd', 786, 2.23, 'john', 70.2 )

# Python�ֵ�
# һ�ֹ�ϣ�����͡����ǹ����ķ�ʽ��������Perl�й���������ϣ����-ֵ�ԡ��ֵ�ļ������Ǽ����κ�Python���ͣ���ͨ�������ֻ��ַ�������һ���棬����ֵ����������Python����
# �ֵ����ô�����������({})������ֵ�ɷ��䲢ʹ�÷�����([])�����ʡ�
# �ֵ���Ԫ��������
dict1 = {}
dict1['one'] = "This is one"
dict1[2] = "This is two"

tinydict = { 'name': 'john', 'code': 6734, 'dept': 'sales' }

print(dict1['one'])
print(dict1[2])
print(tinydict)
print(tinydict.keys())
print(tinydict.values())

# ��������ת��
# int(x [,base])        ת��xΪ������x���ַ����� base Ϊָ���Ļ���
# float(x)              ת��xΪһ��������
# complex(real [,imag]) ����һ������
# str(x)                ת������xΪ�ַ�����ʾ
# repr(x)               ת������xΪ���ʽ�ַ���
# eval(str)             ����һ���ַ�����������һ������
# tuple(s)              ת��sΪһ��Ԫ��
# list(s)               ת��sΪһ���б�
# set(s)                ת��sΪһ������
# dict(d)               ����һ���ֵ䡣 d������(����ֵ)Ԫ������
# frozenset(s)          ת��sΪ�䶳��
# chr(x)                ��һ���ַ�ת��Ϊ����
# unichr(x)             Unicode�ַ�ת��Ϊ����
# ord(x)                �����ַ���ת��Ϊ����ֵ
# hex(x)                ʮ�������ַ���ת��Ϊ����
# oct(x)                ת��������Ϊ�˽����ַ���

# Python��������Ա
# ��һ������Python�ĳ�Ա�����������Գ�Ա�����ַ������б��Ԫ�顣
# in        �����ָ����˳�����ҵ�������������Ϊtrue������Ϊ false
# not in    �����ָ����˳���в����ҵ�������������Ϊtrue������Ϊ false

# Python��ʶ������
# ��ʶ������Ƚ���������Ĵ洢��Ԫ��
# is        �����������������ͬ�Ķ��󣬼�����Ϊtrue��������Ϊ false
# is not    �������������ı�������ͬ�Ķ��󣬼�����Ϊfalse������ true

# range()����
# һ��������0��ʼ�� n-1 ������
for var in list(range(5)):
    print(var)

# �ַ�����ʽ������
print("My name is %s and weight is %d kg!" %('Zara', 21))

