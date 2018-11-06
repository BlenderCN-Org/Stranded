#coding=GB2312
#!/usr/bin/env python

# Given a collection of distinct numbers, return all possible permutations.
# For example,
# [1,2,3] have the following permutations:
# [
#  [1,2,3],
#  [1,3,2],
#  [2,1,3],
#  [2,3,1],
#  [3,1,2],
#  [3,2,1]
#]

#
#递归（Recursion）

#记传入数组为nums，若nums的长度不大于1，则直接返回[nums]

#遍历nums，从中抽取一个数num，递归计算剩余数字组成的数组n，然后将num与结果合并

class Permutations(object):
    def permute(self, nums):
        """
        :type nums: List[int]
        :rtype: List[List[int]]
        """
        if len(nums) <= 1: return [nums]
        ans = []
        for i, num in enumerate(nums):
            n = nums[:i] + nums[i+1:]
            for y in self.permute(n):
                ans.append([num] + y)
        return ans