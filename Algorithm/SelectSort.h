#include "commonInclude.h"

// selection sort
/*
* ������� for ѭ�������� i=q ��ѭ��ʱ�������� list[q] <= list[r], q<r<n. ���ţ�ִ�к���ѭ���� i>q����ʱ��
* list[0]��list[q]�����ݲ��䡣��ˣ��������forִ�е����һ��ѭ��ʱ(��i=n-2֮��)����list[0]<=list[1]<=...<=list[n-1].
*/

/*
* �ٶ���ǰ��Сֵ��list[i]�У�������list[i+1], list[i+2],...,list[n-1]�Ƚϣ�ֻҪ�ҵ���Сֵ������������Сֵ.
* ������list[n-1]�����ҳ���С����.
*/
void sort(int list[], int n)
{
	int i, j, min, temp;
	for (i = 0; i < n - 1; i++)
	{
		min = i;
		for (j = i + 1; j < n; j++)
			if (list[j] < list[min])
				min = j;

		COMM_SWAP(list[i], list[min], temp);
	}
}