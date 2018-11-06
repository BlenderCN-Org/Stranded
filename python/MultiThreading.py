#coding=GB2312
#!/usr/bin/env python

# 多线程package — threading. 包括调用多线程， 同步队列类Queue， Ctrl+c结束多线程。

import time
import threading

def printf(i):
    for x in xrange(5):
        time.sleep(1)
        print(i, end=' ')

def test():
    thread_list = []
    # 建立10个线程
    for i in xrange(10):
        sthread = threading.Thread(target = printf, args = str(i))
        # 设置守护线程（ setDeamon(True)），并用join（）让程序等所有线程结束了再退出（即去掉注释）
        #sthread.setDaemon(True)
        # 不要将这些线程设置为守护线程
        sthread.start()
        thread_list.append(sthread)

    #for i in xrange(10):
    #    thread_list[i].join()

if __name__ == '__main__':
    test()

# 调用10个守护线程（每个守护线程的timeout时间为1s）， 分别打印0~4， 每打印一个数pause x秒钟， x为0~4之间的randint值。
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
    # 建立10个线程
    for i in xrange(10):
        sthread = threading.Thread(target = printf, args = str(i))
        # 设置守护线程（ setDeamon(True)），并用join（）让程序等所有线程结束了再退出（即去掉注释）
        sthread.setDaemon(True)
        # 不要将这些线程设置为守护线程
        sthread.start()
        thread_list.append(sthread)

    for i in xrange(10):
        thread_list[i].join()

if __name__ == '__main__':
    test()


# 引入Queue， 带同步功能（enqueue和dequeue不用手动加锁）的queue类。

# proc函数处理一个thread的操作： 
#   1. dequeue 一个队头元素 
#   2. enqueue 5个threadid 
#   3. 重复执行两次步骤2（epoch<2）

# 注意proc函数中的最后Q.task_done()表示一个任务（一个dequeue的元素）已经结束；test( )中最后的Q.join()为等待队列为空才退出程序。

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
    # 最开始get到的1是在test（）中put进去的
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


# 程序接收ctrl + c后退出。程序每个thread打印100次threadid，直到ctrl+c退出。
# 更好的设计是在try，except后加finally块， 做到即便不ctrl+c也可以正常退出

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