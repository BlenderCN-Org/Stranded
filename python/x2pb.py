import sys
reload(sys)
sys.setdefaultencoding('utf-8') # for UnicodeEncodeError
import os
import xlrd
import msg

av = sys.argv
if len(av) <3:
    print 'error: argv number: ', len(av)
    pass

for val in av:
    if len(val) == 0:
        print 'error: argv is null'
        pass

#dir_xlsx = 'D:/work/project_slg/project/tools/x2p/xlsx/soldier.xlsx'#av[1]#'D:/work/code/x2p/xlsx/army.xlsx'
#msg_name ='soldier'#av[2]#'army'#av[2]
#dir_bin = 'D:/work/project_slg/project/tools/x2p/bin/'#av[3]#'D:/work/code/x2p/bin/'#av[3]
#print dir_xlsx
dir_xlsx=av[1]
msg_name=av[2]
dir_bin =av[3]

# bin dir
if os.path.exists(dir_bin) ==False:
    os.mkdir(dir_bin)

# open xls
book = xlrd.open_workbook(dir_xlsx, formatting_info=False)
sheet = book.sheet_by_index(0)

# bin
bin = open(os.path.join(dir_bin, msg_name+'.pb'), 'wb')

# for each node
vector = msg.get_obj(msg_name+'vector')
for line in range(sheet.nrows):
    # head and dec
    if line < 2:
        continue

    # data
    id = 0
    #obj = msg.get_obj(msg_name)
    obj = vector.m_datas.add()
    for col in range(sheet.ncols):
        if len(sheet.cell(0, col).value ) == 0:
            break

        try:
            f_name = 'm_' + sheet.cell(0, col).value
            node = sheet.cell(line, col)
            f_type = sheet.cell(line, col).ctype
            f_val = sheet.cell(line, col).value
            msg_val = getattr(obj, f_name)

            # get key
            if id == 0:
                id = int(f_val)
            if isinstance(msg_val, int):
                setattr(obj, f_name, int(f_val))
            elif isinstance(msg_val, str):
                if f_type == 2:
                    int_v = int(f_val)
                    str_v = str(int_v)
                    setattr(obj, f_name, str_v)
                else:
                    setattr(obj, f_name, str(f_val))
            else:
                str_v = f_val
                if f_type != 1:
                    int_v = int(f_val)
                    str_v = str(int_v)

                str_v = str_v.replace('_', ';')
                split_val = str_v.split(';')
                for sv in split_val:
                    if sv:
                        msg_val.append(int(sv))
                        
        except:
            info=sys.exc_info()
            print 'error: ',dir_xlsx, 'col:', col, 'line:', line, info
            
    # push obj
    #bin.write(obj.SerializeToString())
    ids = getattr(vector, 'm_ids')
    ids.append(id);
bin.write(vector.SerializeToString())
bin.close();
