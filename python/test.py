#!/usr/bin/env python

import sys
import re
import traceback



def math_str(key, real_key):
	if key.startswith('@'):
		key = key[1:]
		match = re.match("^" + key + "$", real_key)
		if match:
			return 1

	return 0

def main():
	ret = math_str("@list<\\s*const\\s*Item\\s*\\*.*>", "list<const Item *, std::allocator<const Item *> >")
	print("ret=", ret)

if __name__ == '__main__':
	try:
		main()
	except Exception as e:
		traceback.print_exc()
		sys.exit(1)