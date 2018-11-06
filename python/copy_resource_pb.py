#!/usr/bin/python
# build_native.py
# Build native codes
# 
# Please use cocos console instead


import sys
import os, os.path
import shutil
import hashlib
import filecmp

from optparse import OptionParser

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
# -------------- main --------------
if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-s", "--sourcepath", dest="source_path")
    parser.add_option("-t", "--targetpath", dest="targetpath_path")
    (opts, args) = parser.parse_args()
    
    if opts.source_path is None:
        resource_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../table/x2p")
        if not os.path.exists(resource_dir):
            print 'The Resource Dir %s do not exist!' % (resource_dir)
            exit(0)
        else:
            opts.source_path = resource_dir

    command = 'python %s/generate_protocol.py' % (opts.source_path)
    if os.system(command) != 0:
        raise Exception("Generate proto fail!")

    opts.targetpath_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../Resources/res/data")
    copyFiles(os.path.join(opts.source_path, "pb"), opts.targetpath_path)

    opts.targetpath_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../Classes/Protocol")
    copyFiles(os.path.join(opts.source_path, "src"), opts.targetpath_path)

    command = 'python %s/generate_protocol_local.py' % (opts.source_path)
    if os.system(command) != 0:
        raise Exception("Generate proto fail!")

    opts.targetpath_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../Resources/res/localization")
    copyFiles(os.path.join(opts.source_path, "pb"), opts.targetpath_path)

    opts.targetpath_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../Classes/Protocol")
    copyFiles(os.path.join(opts.source_path, "src"), opts.targetpath_path)

    shutil.rmtree(os.path.join(opts.source_path, "pb"))
    shutil.rmtree(os.path.join(opts.source_path, "src"))
