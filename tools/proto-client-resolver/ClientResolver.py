#-*- coding: UTF-8 -*-
import os
import codecs
import re

script_path = os.path.realpath(__file__)
script_dir = os.path.dirname(script_path)
print("script_path:" + script_path)
print("script_dir:" + script_dir)
proto_folder_path = script_dir + "\..\sources"
print("proto_folder_path:" + proto_folder_path)

def isProto(filepath) :
	if os.path.splitext(filepath)[1] == ".proto" :
		return True
	return False

def isStructProto(filepath) :
	if isProto(filepath) and "Struct" in filepath :
		# print("isStructProto")
		return True
	return False

folderCnt = 0
isInMessage = False
def isMessageHeader(linestr) :
	if "message" in linestr :
		# print("isMessage")
		global isInMessage
		global folderCnt
		isInMessage = True
		folderCnt = 0
		if "{" in linestr :
			folderCnt = folderCnt + 1
		return True
	return False

def isEnumHeader(linestr) :
	if "enum" in linestr :
		return True
	return False

def isInMessageStruct(linestr) :
	global isInMessage
	global folderCnt
	if "{" in linestr :
		folderCnt = folderCnt + 1
	elif "}" in linestr :
		folderCnt = folderCnt - 1
	if folderCnt == 0:
		isInMessage = False
	if (folderCnt > 0) and isInMessage:
		return True
	return False

def simplifyLine(linestr) :
	while linestr.find("\t") > -1:
		linestr = linestr.replace("\t", " ")
	while linestr.find("  ") > -1:
		linestr = linestr.replace("  ", " ")
	return linestr

def getMessageName(linestr):
	linestr = simplifyLine(linestr)
	result = re.search(r'message (\w+)',linestr)
	if result:
		return result.group(1)
	return None

def getMessageMember(linestr):
	message_member = {}
	linestr = simplifyLine(linestr)
	s = re.sub("[^\w]", " ", linestr).split()
	if len(s) < 3:
		return None
	message_member["member_modifier"] = s[0]
	message_member["member_type"] = s[1]
	message_member["member_name"] = s[2]
	if len(s) > 4:
		message_member["member_default"] = s[4]
	return message_member

def getEnumName(linestr):
	linestr = simplifyLine(linestr)
	result = re.search(r'enum (\w+)',linestr)
	if result:
		return result.group(1)
	return None

Enums = []
def isEnum(typestr):
	for e in Enums:
		if e == typestr:
			return True
	return False

def isBasicValue(member_type):
	if "uint4" == member_type or "uint8" == member_type or "uint16" == member_type or "uint32" == member_type \
	or "uint64" == member_type or "bytes" == member_type or "bool" == member_type:
		return True
	else :
		return isEnum(member_type)

def analyseMsgLine(linestr):
	linestr = simplifyLine(linestr)
	if result == re.search(r'message(\w+)',linestr):
		pass

struct_files = []
filelist = os.listdir(proto_folder_path)
for i in range(0,len(filelist)):
	path = os.path.join(proto_folder_path,filelist[i])
	# print("path:" + path)
	if isStructProto(path) :
		file = codecs.open(path, 'r', 'utf-8')
		file_content = file.readlines()
		struct_files.append(file_content)
		file.close()

for file in struct_files:
	for line in file:
		if isEnumHeader(line):
			Enums.append(getEnumName(line))

Message_Structs = []
for file in struct_files:
	message_name = ""
	for line in file:
		print("message_name:" + message_name)
		if isMessageHeader(line):
			message_name = getMessageName(line)
			Message_Struct = {"message_name":message_name,"message_members":[]}
			Message_Structs.append(Message_Struct)
		elif isInMessageStruct(line):
			member = getMessageMember(line)
			if member:
				for struct in Message_Structs:
					if struct["message_name"] == message_name:
						struct["message_members"].append(member)

# Ordered_Structs = []
# def sort_structs(structs):
# 	for struct in structs:
# 		isReferedByOthers = False
# 		for member in struct["message_members"]:
# 			isReferedByOthers = not isBasicValue(member["member_type"])
# 			if isReferedByOthers:
# 				for Ordered_Struct in Ordered_Structs:
# 					pass

proto_util_path = script_dir + r'\..\..\Client_4.18\LuaSource\util\protoresolver.lua'
f = codecs.open(proto_util_path, 'w', 'utf-8')
f.write("--region *.lua\n")
f.write("--Date\n")
f.write("\n")
f.write("ProtoResolver = {}")
f.write("\n")

for Message_Struct in Message_Structs:
	f.write("\n")
	f.write("function ProtoResolver.get" + Message_Struct["message_name"] + "( protoData )\n")
	f.write("\tif protoData == nil then return nil end\n")
	f.write("\n")
	f.write("\tlocal dataTable = {}\n")
	f.write("\n")
	for member in Message_Struct["message_members"]:
		if member["member_modifier"] == "optional":
			if isBasicValue(member["member_type"]):
				f.write("\tdataTable." + member["member_name"] + " = protoData." + member["member_name"] + "\n")
			else :
				f.write("\tdataTable." + member["member_name"] + " = ProtoResolver.get" + member["member_type"] + \
					"(protoData." + member["member_name"] + ")\n")
		elif member["member_modifier"] == "repeated":
			f.write("\n")
			f.write("\tdataTable." + member["member_name"] + " = {}\n")
			if isBasicValue(member["member_type"]) :
				f.write("\tfor k,v in ipairs(protoData." + member["member_name"] + ") do\n")
				f.write("\t\ttable.insert(dataTable." + member["member_name"] + ",v)\n")
			else :
				f.write("\tfor k,v in ipairs(protoData." + member["member_name"] + ") do\n")
				f.write("\t\tlocal data = ProtoResolver.get" + member["member_type"] + "(v)\n")
				f.write("\t\ttable.insert(dataTable." + member["member_name"] + ",data)\n")
			f.write("\tend\n")
	f.write("\n")
	f.write("\treturn dataTable\n")
	f.write("end\n")


f.write("\n")
f.write("\n")
f.write("return ProtoResolver\n")
f.write("--endregion\n\n\n\n")

f.close()