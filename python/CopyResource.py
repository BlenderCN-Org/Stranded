#coding=GB2312
#!/usr/bin/env python

import sys
import os, os.path
import shutil
import hashlib
import filecmp
from optparse import OptionParser

def CalcMD5(filepath):
	with open(filepath,'rb') as f:
		md5obj = hashlib.md5()
		md5obj.update(f.read())
		hash = md5obj.hexdigest()
		return hash

def copyFiles(sourceDir, targetDir):
	if os.path.exists(sourceDir) and ((sourceDir.find("Default") > 0) or (sourceDir.find("spine") > 0)):
		return
	for file in os.listdir(sourceDir):
		sourceFile = os.path.join(sourceDir, file)
		targetFile = os.path.join(targetDir, file)
		if os.path.isfile(sourceFile):
			if not os.path.exists(targetDir):
				os.makedirs(targetDir)
			if not os.path.exists(targetFile) or (os.path.exists(targetFile) and (not filecmp.cmp(sourceFile, targetFile))):
				fileHandleTarget = open(targetFile, "wb")
				fileHandleSource = open(sourceFile, "rb")
				fileHandleTarget.write(fileHandleSource.read())
				fileHandleTarget.close()
				fileHandleSource.close()
		if os.path.isdir(sourceFile):
			copyFiles(sourceFile, targetFile)

# -------------- main --------------
if __name__ = '__main__':

	parser = OptionParser()
	parser.add_option("-s", "--sourcepath", dest="source_path")
	parser.add_option("-t", "--targetpath", dest="targetpath_path")
	(opts, args) = parser.parse_args()

	if opts.source_path is None:
		resource_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../ui/Resources/res")
		if not os.path.exists(resource_dir):
			print 'The Resource Dir %s do not exist!' % (resource_dir)
			exit(0)
		else:
		opts.source_path = resource_dir

	if opts.targetpath_path is None:
		opts.targetpath_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../Resources/res")

	copyFiles(opts.source_path, opts.targetpath_path)