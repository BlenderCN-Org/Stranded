#coding=GB2312
#!/usr/bin/env python

# ���߳�package �� threading. �������ö��̣߳� ͬ��������Queue�� Ctrl+c�������̡߳�

import time
import threading

def printf(i):
    for x in xrange(5):
        time.sleep(1)
        print(i, end=' ')

def test():
    thread_list = []
    # ����10���߳�
    for i in xrange(10):
        sthread = threading.Thread(target = printf, args = str(i))
        # �����ػ��̣߳� setDeamon(True)��������join�����ó���������߳̽��������˳�����ȥ��ע�ͣ�
        #sthread.setDaemon(True)
        # ��Ҫ����Щ�߳�����Ϊ�ػ��߳�
        sthread.start()
        thread_list.append(sthread)

    #for i in xrange(10):
    #    thread_list[i].join()

if __name__ == '__main__':
    test()

# ����10���ػ��̣߳�ÿ���ػ��̵߳�timeoutʱ��Ϊ1s���� �ֱ��ӡ0~4�� ÿ��ӡһ����pause x���ӣ� xΪ0~4֮���randintֵ��
import time
import threading
import random

def printf(i):
    randime = random.randint(1, 5)
    for x in xrange(5):
        time.sleep(randime)
        print('T' + str(i), randtime)

def test():
    thread_list = []
    # ����10���߳�
    for i in xrange(10):
        sthread = threading.Thread(target = printf, args = str(i))
        # �����ػ��̣߳� setDeamon(True)��������join�����ó���������߳̽��������˳�����ȥ��ע�ͣ�
        sthread.setDaemon(True)
        # ��Ҫ����Щ�߳�����Ϊ�ػ��߳�
        sthread.start()
        thread_list.append(sthread)

    for i in xrange(10):
        thread_list[i].join()

if __name__ == '__main__':
    test()


# ����Queue�� ��ͬ�����ܣ�enqueue��dequeue�����ֶ���������queue�ࡣ

# proc��������һ��thread�Ĳ����� 
#   1. dequeue һ����ͷԪ�� 
#   2. enqueue 5��threadid 
#   3. �ظ�ִ�����β���2��epoch<2��

# ע��proc�����е����Q.task_done()��ʾһ������һ��dequeue��Ԫ�أ��Ѿ�������test( )������Q.join()Ϊ�ȴ�����Ϊ�ղ��˳�����

import time
import threading
import random
import queue

Q = queue.Queue()

def proc(threadid, epoch):
    while True:
        time.sleep(1)
        try:
            ele = Q.get()
            print("Thread " + str(threadid) + " get element " + str(ele))
        except queue.Empty:
            print("Thread " + str(threadid) + " get empty queue")
            continue
        if int(epoch) < 2:
            for i in xrange(5):
                Q.put(threadid)
            epoch = int(epoch) + 1

        Q.task_done()

def test():
    # �ʼget����1����test������put��ȥ��
    Q.put(1)
    thread_list = []
    for i in xrange(3):
        args = [str(i), str(0)]
        sthread = threading.Thread(target = proc, args = args)
        sthread.setDaemon(True)
        sthread.start()
        thread_list.append(sthread)
    Q.join()
     
if __name__ == '__main__':
    test()


# �������ctrl + c���˳�������ÿ��thread��ӡ100��threadid��ֱ��ctrl+c�˳���
# ���õ��������try��except���finally�飬 �������㲻ctrl+cҲ���������˳�

import time
import threading

def printf(i):
    for x in xrange(5):
        time.sleep(1)
        print(i, end=' ')

def test():
    for i in xrange(10):
        sthread = threading.Thread(target = printf, args = str(i))
        sthread.setDaemon(True)
        sthread.start()

    try:
        while 1:
            time.sleep(1)
    except KeyboardInterrupt:
        print("exit")

if __name__ == '__main__':
    test()