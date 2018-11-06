import os
import sys
reload(sys)
sys.setdefaultencoding('utf-8') # for UnicodeEncodeError

av = sys.argv
if len(av) <3:
    print 'error: argv number: ', len(av)
    pass

dir_in = av[1]
dir_py = av[2]
if dir_py[-1] == '\\':
    len = len(dir_py)
    dir_py = dir_py[0:len-1]
dir_py = dir_py.replace("\\", '/')

msg_names = []
for parent,dirnames,filenames in os.walk(dir_in):
    for filename in filenames:
        m_name = filename.split('.')[0]
        msg_names.append(m_name)

msg_f = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), "msg.py"), 'w')
msg_f.write('# -*- coding:utf-8 -*-\n')
msg_f.write('import sys\n')
msg_f.write('sys.path.append(\''+ dir_py +'\')\n')
for n in msg_names:
    msg_f.write('from '+ n + '_pb2 import '+ n +','+n+'vector'+'\n')
msg_f.write('\n\ndef get_obj(name):\n')

#
for n in msg_names:
    msg_f.write('\tif name == \''+n+'\': return '+n+'()\n')
    msg_f.write('\tif name == \''+n+'vector\': return '+n+'vector()\n')

msg_f.close()