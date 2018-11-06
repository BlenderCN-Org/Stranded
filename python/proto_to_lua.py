#!/usr/bin/env python

import os
import sys
import shutil
import traceback
from optparse import OptionParser


def copySingleFile(sourceFile, targetDir, file):
    targetFile = os.path.join(targetDir,  file)
    if os.path.isfile(sourceFile):
        if not os.path.exists(targetDir):
            os.makedirs(targetDir)
        if not os.path.exists(targetFile) or(os.path.exists(targetFile) and (not filecmp.cmp(sourceFile, targetFile))):
            fileHandleTarget = open(targetFile, "wb")
            fileHandleSource = open(sourceFile, "rb")
            fileHandleTarget.write(fileHandleSource.read())
            fileHandleTarget.close()
            fileHandleSource.close()

def copyFiles(sourceDir, targetDir):
    for file in os.listdir(sourceDir):
        sourceFile = os.path.join(sourceDir,  file)
        targetFile = os.path.join(targetDir,  file)
        if os.path.isfile(sourceFile):
            if not os.path.exists(targetDir):
                os.makedirs(targetDir)
            if not os.path.exists(targetFile) or(os.path.exists(targetFile) and (not filecmp.cmp(sourceFile, targetFile))):
                fileHandleTarget = open(targetFile, "wb")
                fileHandleSource = open(sourceFile, "rb")
                fileHandleTarget.write(fileHandleSource.read())
                fileHandleTarget.close()
                fileHandleSource.close()
        if os.path.isdir(sourceFile):
            copyFiles(sourceFile, targetFile)

def generate(proto_type, sourceDir):
    proto_dir = os.path.join(sourceDir, "proto")
    xlsx_dir = os.path.join(sourceDir, "../" + proto_type)
    #src_dir = os.path.join(sourceDir, "src")
    #python_dir = os.path.join(sourceDir, "python")
    #pb_dir = os.path.join(sourceDir, "pb")
    
    if not os.path.exists(proto_dir):
        os.makedirs(proto_dir)
    else :
        shutil.rmtree(proto_dir)
        os.makedirs(proto_dir)

    '''
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
    '''

    for file in os.listdir(xlsx_dir):
        file_path = os.path.join(xlsx_dir,  file)
        if file.find("DS_Store") > 0:
            os.remove(file_path)
            continue
        command = 'python %s/x2p.py %s %s %s' % (sourceDir, file_path, file.split(".")[0], proto_dir)
        if os.system(command) != 0:
            raise Exception("Generate proto fail!")

    '''
    for file in os.listdir(proto_dir):
        file_path = os.path.join(proto_dir,  file)
        command = 'protoc -I=%s --python_out=%s %s' % (proto_dir, python_dir, file_path)
        if os.system(command) != 0:
            raise Exception("Generate python fail!")

    command = 'python %s/p2m.py %s %s' % (sourceDir, proto_dir, python_dir)
    if os.system(command) != 0:
        raise Exception("p2m fail!")

    for file in os.listdir(xlsx_dir):
        file_path = os.path.join(xlsx_dir,  file)
        command = 'python %s/x2pb.py %s %s %s' % (sourceDir, file_path, file.split(".")[0], pb_dir)
        if os.system(command) != 0:
            raise Exception("Generate proto fail!")

    os.remove(os.path.join(sourceDir, "msg.py"))
    os.remove(os.path.join(sourceDir, "msg.pyc"))
    '''
    #shutil.rmtree(proto_dir)
    #shutil.rmtree(python_dir)

def CopyCppProto(sourceDir, targetDir):
    for file in os.listdir(sourceDir):
        if file.find("git") > 0:
            continue
        elif file.find("DS_Store") > 0:
            continue
        elif file.find(".proto") > 0:
            file_path = os.path.join(sourceDir,  file)
            copySingleFile(file_path, targetDir, file)
        sourceFile = os.path.join(sourceDir,  file)
        if os.path.isdir(sourceFile):
            CopyCppProto(sourceFile, targetDir)

def GenerateLua(sourceDir):
    proto_dir = os.path.join(sourceDir, "proto")
    protolua_dir = os.path.join(sourceDir, "protolua")

    if not os.path.exists(protolua_dir):
        os.makedirs(protolua_dir)
    else:
        shutil.rmtree(protolua_dir)
        os.makedirs(protolua_dir)

    for file in os.listdir(proto_dir):
        file_path = os.path.join(proto_dir,  file)
        if file.find("git") > 0:
            continue
        elif file.find("DS_Store") > 0:
            continue
        elif file.find(".proto") > 0:
            command = 'protoc --lua_out=%s --plugin=protoc-gen-lua=\"%s/plugin/protoc-gen-lua.sh\" -I=%s/proto %s' % (protolua_dir, sourceDir, sourceDir, file_path)
            if os.system(command) != 0:
                raise Exception("Generate lua fail!")

    
    msg_names = []
    for parent,dirnames,filenames in os.walk(proto_dir):
        for filename in filenames:
            m_name = filename.split('.')[0]
            msg_names.append(m_name)

    msg_f = open(os.path.join(protolua_dir, "protobufdef.lua"), 'w')
    msg_f.write("package.path = package.path .. \';./lua/protobuf/?.lua;./lua/protobuf/protolua/?.lua\'\n")
    msg_f.write("package.cpath = package.cpath .. \';./lua/protobuf/?.so;./lua/protobuf/protolua/?.so\'\n")
    msg_f.write("\n\n")

    for n in msg_names:
        msg_f.write("require(\'" + n + "_pb\')\n")

    msg_f.close()

    os.remove(os.path.join(sourceDir, "plugin/plugin_pb2.pyc"))
    shutil.rmtree(proto_dir)

def main():
    parser = OptionParser()
    parser.add_option("-s", "--sourcepath", dest="source_path")
    parser.add_option("-t", "--targetpath", dest="targetpath_path")
    parser.add_option("-y", "--type", dest="proto_type")
    (opts, args) = parser.parse_args()
    
    pyFileDir = os.path.dirname(os.path.realpath(__file__))

    if opts.source_path is None:
        resource_dir = os.path.join(pyFileDir, "../../../../table/x2p")
        if not os.path.exists(resource_dir):
            print 'The Resource Dir %s do not exist!' % (resource_dir)
            exit(0)
        else:
            opts.source_path = resource_dir

    print("1. generate proto for excel!")
    ################ xlsx
    if opts.proto_type is None:
        opts.proto_type = 'xlsx'
    generate(opts.proto_type, opts.source_path)

    opts.targetpath_path = os.path.join(pyFileDir, "proto")
    if not os.path.exists(opts.targetpath_path ):
        os.makedirs(opts.targetpath_path )
    else :
        shutil.rmtree(opts.targetpath_path )
        os.makedirs(opts.targetpath_path )
    copyFiles(os.path.join(opts.source_path, "proto"), opts.targetpath_path)

    print("2. generate proto for translate!")
    ################ translate
    opts.proto_type = 'translate'
    generate(opts.proto_type, opts.source_path);

    copyFiles(os.path.join(opts.source_path, "proto"), opts.targetpath_path)
    
    shutil.rmtree(os.path.join(opts.source_path, "proto"))
    
    print("3. copy cpp proto!")
    ############### cpp proto
    opts.source_path = os.path.join(pyFileDir, "../../../../protomsg")
    CopyCppProto(opts.source_path, opts.targetpath_path)

    print("4. generate lua for proto!")
    ############### proto to lua
    GenerateLua(pyFileDir)

    opts.targetpath_path = os.path.join(pyFileDir, "../../../Resources/res/lua/protobuf/protolua")
    if not os.path.exists(opts.targetpath_path ):
        os.makedirs(opts.targetpath_path )
    else :
        shutil.rmtree(opts.targetpath_path )
        os.makedirs(opts.targetpath_path )
    copyFiles(os.path.join(pyFileDir, "protolua"), opts.targetpath_path)

    shutil.rmtree(os.path.join(pyFileDir, "protolua"))

    ##########################

    print '---------------------------------'
    print 'Generating lua succeeds.'
    print '---------------------------------'

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        traceback.print_exc()
        sys.exit(1)
