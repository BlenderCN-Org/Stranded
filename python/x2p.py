# -*- coding:utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8') # for UnicodeEncodeError
import os
import xlrd
import re


av = sys.argv
if len(av) <4:
    print 'error: argv number: ', len(av)
    pass

indir = av[1]
msgname = av[2]
outdir = av[3]

#indir = 'D:/work/code/x2p/xlsx/soldierskill.xlsx'#av[1]#'D:/work/code/x2p/xlsx/army.xlsx'
#msgname ='soldierskill'#av[2]#'army'#av[2]
#outdir = 'D:/work/code/x2p/bin/'#av[3]#'D:/work/code/x2p/bin/'#av[3]

book = xlrd.open_workbook(indir, formatting_info=False)
sheet = book.sheet_by_index(0)

if os.path.exists(outdir) ==False:
    os.mkdir(outdir)

output = os.path.join(outdir, msgname + '.proto')
out_file = open(output, 'w')
out_file.writelines('package TableProto;\n')
out_file.writelines('message ' + msgname +' {\n')

for col in range(sheet.ncols):
    f_name = str()
    f_desc = str()
    f_type = 1 # 0 : string 1: int 2:list<int>
    for row in range(sheet.nrows):
        node = sheet.cell(row, col)

        if node.value is None:
            continue;

        if row == 0:
            f_name = node.value;
            continue

        if row == 1:
            f_desc = node.value;
            continue

        # check type
        if node.ctype == 1:
            sss = node.value
            if sss.find(';') != -1:
                # int;int
                f_type = 2
                break
            else:
                # string
                f_type = 0
        # int
        elif node.ctype == 2:
            if f_type != 0:
                f_type = 1
    # end for row

    # nothing
    if len(f_name) == 0:
        continue

    if f_type == 0:
        out_file.write('\toptional\tbytes m_')
    elif f_type == 1:
        out_file.write('\toptional\tuint32 m_')
    elif f_type == 2:
        out_file.write('\trepeated\tuint32 m_')

    out_file.write(f_name + '\t=' + str(col+1) + ';' + '\t// ' + f_desc +'\n')
#end for col
out_file.write('}\n\n\n')

out_file.write('message ' + msgname + 'vector{\n')
out_file.write('\trepeated\tint32 m_ids =1;\n')
out_file.write('\trepeated\t'+msgname+'\tm_datas=2;\n')
out_file.write('}\n')
out_file.close()
