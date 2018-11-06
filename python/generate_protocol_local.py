#!/usr/bin/python
# build_native.py
# Build native codes
# 
# Please use cocos console instead


import sys
import os, os.path
import shutil
import hashlib
from optparse import OptionParser

def generate(proto_type):
    
    proto_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "proto")
    xlsx_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../" + proto_type)
    src_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "src")
    python_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "python")
    pb_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "pb")
    
    if not os.path.exists(proto_dir):
        os.makedirs(proto_dir)
    else :
        shutil.rmtree(proto_dir)
        os.makedirs(proto_dir)

    if not os.path.exists(src_dir):
        os.makedirs(src_dir)
    else :
        shutil.rmtree(src_dir)
        os.makedirs(src_dir)

    if not os.path.exists(python_dir):
        os.makedirs(python_dir)
    else :
        shutil.rmtree(python_dir)
        os.makedirs(python_dir)

    if not os.path.exists(pb_dir):
        os.makedirs(pb_dir)
    else :
        shutil.rmtree(pb_dir)
        os.makedirs(pb_dir)

    for file in os.listdir(xlsx_dir):
        file_path = os.path.join(xlsx_dir,  file)
        command = 'python %s/x2p.py %s %s %s' % (os.path.dirname(os.path.realpath(__file__)), file_path, file.split(".")[0], proto_dir)
        if os.system(command) != 0:
            raise Exception("Generate proto fail!")

    for file in os.listdir(proto_dir):
        file_path = os.path.join(proto_dir,  file)
        command = 'protoc -I=%s --cpp_out=%s %s' % (proto_dir, src_dir, file_path)
        if os.system(command) != 0:
            raise Exception("Generate cpp fail!")

    for file in os.listdir(proto_dir):
        file_path = os.path.join(proto_dir,  file)
        command = 'protoc -I=%s --python_out=%s %s' % (proto_dir, python_dir, file_path)
        if os.system(command) != 0:
            raise Exception("Generate python fail!")

    command = 'python %s/p2m.py %s %s' % (os.path.dirname(os.path.realpath(__file__)), proto_dir, python_dir)
    if os.system(command) != 0:
        raise Exception("p2m fail!")

    for file in os.listdir(xlsx_dir):
        file_path = os.path.join(xlsx_dir,  file)
        command = 'python %s/x2pb.py %s %s %s' % (os.path.dirname(os.path.realpath(__file__)), file_path, file.split(".")[0], pb_dir)
        if os.system(command) != 0:
            raise Exception("Generate proto fail!")

    os.remove(os.path.join(os.path.dirname(os.path.realpath(__file__)), "msg.py"))
    os.remove(os.path.join(os.path.dirname(os.path.realpath(__file__)), "msg.pyc"))
    shutil.rmtree(proto_dir)
    shutil.rmtree(python_dir)
    shutil.rmtree(src_dir)
    os.makedirs(src_dir)
    command = 'protoc -I=%s --cpp_out=%s %s/localization.proto' % (os.path.dirname(os.path.realpath(__file__)), src_dir, os.path.dirname(os.path.realpath(__file__)))
    if os.system(command) != 0:
        raise Exception("Generate cpp fail!")

# -------------- main --------------
if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option("-t", "--type", dest="proto_type")
    (opts, args) = parser.parse_args()
    
    if opts.proto_type is None:
        opts.proto_type = 'translate'
    generate(opts.proto_type);
