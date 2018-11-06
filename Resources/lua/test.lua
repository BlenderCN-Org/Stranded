



-----------------------------------------------
--[[

-- Lua 弱引用 table
a = {}
b = {__mode = "k"}	-- table是key弱引用;包含"v"是value弱引用; 两个字符均存在为key/value弱引用
setmetatable(a, b)
key = {}
a[key] = 1
key = {}
a[key] = 2
collectgarbage()
for k, v in pairs(a) do
	print(v)
end

-- 备忘录(memoize)函数：

-- 用“空间换时间”是一种通用的程序运行效率优化手段，比如：对于一个普通的Server，它接受到的请求中包含Lua代码，
-- 每当其收到请求后都会调用Lua的loadstring函数来动态解析请求中的Lua代码，如果这种操作过于频率，
-- 就会导致Server的执行效率下降。要解决该问题，我们可以将每次解析的结果缓存到一个table中，
-- 下次如果接收到相同的Lua代码，就不需要调用loadstirng来动态解析了，而是直接从table中获取解析后的函数直接执行即可。
-- 这样在有大量重复Lua代码的情况下，可以极大的提高Server的执行效率。反之，如果有相当一部分的Lua代码只是出现一次，
-- 那么再使用这种机制，就将会导致大量的内存资源被占用而得不到有效的释放。在这种情况下，如果使用弱引用表，
-- 不仅可以在一定程度上提升程序的运行效率，内存资源也会得到有效的释放。

local results = {}
setmetatable(results, {_mode = "v"})	-- results表中的key是字符串形式的Lua代码
function mem_loadstring(s)
	local res = results[s]
	if res == nil then
		res = assert(loadstring(s))
		result[s] = res
	end
	return res
end

-- 类、继承
-----------------------------------------------------------------------------------
--
--	在这段代码中，我们可以将Account视为class的声明，如Java中的：
--	public class Account
--	{
--		public float blance = 0;
--		public Account(Account o);
--		public void deposite(float f);
--	}
--

--这里balance是一个公有的成员变量。
Account = {balance = 0}

--new可以视为构造函数
function Account:new(o)
	o = o or {}	-- 如果参数中没有提供table，则创建一个空的。
	-- 将新对象实例的metatable指向Account表(类)，这样就可以将其视为模板了。
	setmetatable(o, self)
	-- 在将Account的__index字段指向自己，以便新对象在访问Account的函数和字段时，可被直接重定向。
	self.__index = self
	-- 最后返回构造后的对象实例
	return o
end

-- deposite被视为Account类的公有成员函数
function Account:deposit(v)
	-- 这里的self表示对象实例本身
	self.balance = self.balance + v
end

function Account:withdraw(v)
	if v > self.balance then
		error("Insufficient funds")
	end
	self.balance = self.balance - v
end

-- 下面将派生出一个Account的子类，以使客户可以实现透支的功能。
SpecialAccount = Account:new()	-- 此时SpecialAccount仍然为Account的一个对象实例

---- 派生类SpecialAccount扩展出的方法。
-- 下面这些SpecialAccount中的方法代码(getLimit/withdraw)，一定要位于SpecialAccount被Account构造之后。
function SpecialAccount:getLimit()
	-- 此时的self将为对象实例。
	return self.limit or 0
end

-- SpecialAccount将为Account的子类，下面的方法withdraw可以视为SpecialAccount
-- 重写的Account中的withdraw方法，以实现自定义的功能。
function SepcialAccount:withdraw(v)
	-- 此时的self将为对象实例。
	if v - self.balance >= self:getLimit() then
		error("Insufficient funds")
	end
	self.balance = self.balance - v
end

-- 在执行下面的new方法时，table s的元表已经是SpecialAccount了，而不再是Account。
s = SpecialAccount:new{limit = 1000.00}
-- 在调用下面的deposit方法时，由于table s和SpecialAccount均未提供该方法，因此访问的仍然是
-- Account的deposit方法。
s:deposit(300)


-- 此时的withdraw方法将不再是Account中的withdraw方法，而是SpecialAccount中的该方法。
-- 这是因为Lua先在SpecialAccount(即s的元表)中找到了该方法。
s:withdraw(200.00)
print(s.balance)


-- 下面的代码创建两个Account的对象实例

-- 通过Account的new方法构造基于该类的示例对象。
a = Account:new()
--
--	这里需要具体解释一下，此时由于table a中并没有deposite字段，因此需要重定向到Account，
--	同时调用Account的deposite方法。在Account.deposite方法中，由于self(a对象)并没有balance
--	字段，因此在执行self.balance + v时，也需要重定向访问Account中的balance字段，其缺省值为0。
--	在得到计算结果后，再将该结果直接赋值给a.balance。此后a对象就拥有了自己的balance字段和值。
--	下次再调用该方法，balance字段的值将完全来自于a对象，而无需在重定向到Account了。
--
a:deposit(100.00)
print(a.balance)

b = Account:new()
b:deposit(200.00)
print(b.balance)

-- 私密性

-- 这里我们需要一个闭包函数作为类的创建工厂
function newAccount(initialBalance)
	-- 这里的self仅仅是一个普通的局部变量，其含义完全不同于前面示例中的self。
	-- 这里之所以使用self作为局部变量名，也是为了方便今后的移植。比如，以后
	-- 如果改为上面的实现方式，这里应用了self就可以降低修改的工作量了。
	local self = {balance = initialBalance}	-- 这里我们可以将self视为私有成员变量
	local withdraw = function(v) self.balance = self.balance - v end
	local deposit = function(v) self.balance = self.balance + v end
	local getBalance = function() return self.balance end
	-- 返回对象中包含的字段仅仅为公有方法。事实上，我们通过该种方式，不仅可以实现
	-- 成员变量的私有性，也可以实现方法的私有性，如：
	-- local privateFunction = function()	-- do something end
	-- 只要我们不在输出对象中包含该方法的字段即可。
	return{withdraw = withdraw, deposit = deposit, getBalance = getBalance}
end

-- 和前面两个示例不同的是，在调用对象方法时，不再需要self变量，因此我们可以直接使用点(.)，
-- 而不再需要使用冒号(:)操作符了。
acc1 = newAccount(100.00)
-- 在函数newAccount返回之后，该函数内的“非局部变量”表self就不再能被外部访问了，只能通过
-- 该函数返回的对象的方法来操作它们。
acc1.withdraw(40.00)
print(acc1.getBalance())

-----------------------------------------------------------------------------------

-- 该table用于存储所有已经声明过的全局变量名
local declaredNames = {}
local mt = {
	__newindex = function(table, name, value)
		-- 先检查新的名字是否已经声明过，如果存在，这直接通过rawset函数设置即可.
		if not declaredNames[name] then
			-- 再检查本次操作是否实在主程序或者C代码中完成的，如果是，就继续设置，否则报错.
			local w = debug.getinfo(2, "S").what
			if w ~= "main" and w ~= "C" then
				error("attempt to write a undeclared variable " .. name)
			end
			-- 在实际设置之前，更新一下declaredNames表，下次再设置时就无需检查了.
			declaredNames[name] = true
		end
		print("Setting " .. name .. " to " .. value)
		rawset(table, name, value)
	end,
	
	__index = function(_, name)
		if not declaredNames[name] then
			error("attempt to read undeclared variable " .. name)
		else
			return rawget(_, name)
		end
	end
}
-- Lua全局变量保存在常规表_G中
setmetatable(_G, mt)
a = 11
local kk = aa

-- 只读的table
function readOnly(t)
	local proxy = {}
	local mt = {
		__index = t,
		__newindex = function(t, k, v)
			error("attempt to update a read-only table")
		end
	}
	setmetatable(proxy, mt)
	return proxy
end

days = readOnly{"Sunday", "Monday", "Tuesady", "Wdenesday", "Thursday", "Friday", "Saturday"}
print(days[1])
days[2] = "Today"

-- 跟踪table的访问
t = {}	-- 原来的table
local _t = t	-- 保持对原有table的私有访问
t = {}	-- 创建代理
-- 创建元表
local mt = {
	__index = function(table, key)
		print("access to element " .. tostring(key))
		return _t[key]	-- 通过访问原来的表返回字段值
	end,
	
	__newindex = function(table, key, value)
		print("update of element " .. tostring(key) .. " to " .. tostring(value))
		_t[key] = value	-- 更新原来的table
	end
}
setmetatable(t, mt)

t[2] = "hello"
print(t[2])

-- 具有默认值的table
function setDefault(table, default)
	local mt = {__index = function() return default end}
	setmetatable(table, mt)
end

tab = {x = 10, y = 20}
print(tab.x, tab.z)
setDefault(tab, 0)
print(tab.x, tab.z)

-- table 访问的元方法 [针对修改table常规行为]
-- __index 元方法(访问table中不存在的字段)
Window = {}
Window.prototype = {x = 0, y = 0, width = 100, height = 100}
Window.mt = {}

function Window.new(o)
	setmetatable(o, Window.mt)
	return o
end

-- 将Window的元方法__index指向一个匿名函数
-- 匿名函数的参数table 和 key 取自于table.key。
-- 函数扩展性更强
Window.mt.__index = function(table, key) return Window.prototype[key] end
-- 或者，简洁表示方法效率更高
--Window.mt.__index = Window.prototype
-- 访问table时禁用__index元方法: rawget(table, key)	

-- test
w = Window.new{x = 10, y = 20}
print(w.width)
print(w.width1)

-- __newindex (不存在键的赋值)
-- 对元表进行赋值 避开元方法直接操作当前table的函数: rawset(table, key, value)

-- 库定义的元方法
Set = {}
local metatable = {} -- 元表

-- 根据参数列表中的值创建一个新的集合
function Set.new(l)
	local set = {}
	-- 将所有由该方法创建的集合的元表都指定到metatable
	setmetatable(set, metatable)
	for _, v in ipairs(l) do
		set[v] = true
	end
	return set
end

function Set.tostring(set)
	local l = {}
	for e in pairs(set) do
		l[#l + 1] = e
	end
	return "{" .. table.concat(l, ",") .. "}";
end

-- __tostring print函数调用来格式化输出
metatable.__tostring = Set.tostring

-- __metatable(保护元素)
metatable.__metatable = "new test"

-- 下面是测试代码:
s1 = Set.new{4, 5, 10}
print(s1)

s2 = Set.new{}
print(getmetatable(s2))
setmetatable(s2, {})

-- 关系类的元方法
Set = {}
local metatable = {} -- 元表

-- 根据参数列表中的值创建一个新的集合
function Set.new(l)
	local set = {}
	-- 将所有由该方法创建的集合的元表都指定到metatable
	setmetatable(set, metatable)
	for _, v in ipairs(l) do
		set[v] = true
	end
	return set
end

-- __le(小于等于)
metatable.__le = function(a, b)
	for k in pairs(a) do
		if not b[k] then return false end
	end
	return true
end
-- __lt(小于)
metatable.__lt = function(a, b) return a <= b and not (b <= a) end
-- __eq(等于)
metatable.__eq = function(a, b) return a <= b and b <= a end

-- 下面是测试代码:
s1 = Set.new{2, 4}
s2 = Set.new{4, 10, 2}
print(s1 <= s2)
print(s1 < s2)
print(s1 >= s1)
print(s1 > s1)

-- 算术类的元方法
Set = {}
local metatable = {} -- 元表

-- 根据参数列表中的值创建一个新的集合
function Set.new(l)
	local set = {}
	-- 将所有由该方法创建的集合的元表都指定到metatable
	setmetatable(set, metatable)
	for _, v in ipairs(l) do
		set[v] = true
	end
	return set
end

-- 取两个集合并集的函数
function Set.union(a, b)
	-- 防止s1 = s1 + 8
	if getmetatable(a) ~= metatable or getmetatable(b) ~= metatable then
		error("attempt to 'add' a set with a non-set value")
	end
	local res = Set.new{}
	for k in pairs(a) do
		res[k] = true
	end
	for k in pairs(b) do
		res[k] = true
	end
	return res
end

-- 取两个集合交集的函数
function Set.intersection(a, b)
	local res = Set.new{}
	for k in pairs(a) do
		res[k] = b[k]
	end
	return res
end

function Set.tostring(set)
	local l = {}
	for e in pairs(set) do
		l[#l + 1] = e
	end
	return "{" .. table.concat(l, ",") .. "}";
end

function Set.print(s)
	print(Set.tostring(s))
end

-- 最后将元方法加入到元表中， 这样当两个由Set.new方法创建出来的集合进行
-- 加运算时， 将被重定向到Set.union方法，乘法运算将被重定向到Set.intersection
-- __add(加法), __mul(乘法), __sub(减法), __div(除法), __unm(相反数),
-- __mod(取模), __pow(乘幂), __concat(描述连接操作符行为)
metatable.__add = Set.union
metatable.__mul = Set.intersection

-- 下面为测试代码
s1 = Set.new{10, 20, 30, 50}
s2 = Set.new{30, 1}
s3 = s1 + s2
Set.print(s3)
Set.print(s3 * s1)

-- StringBuilder
local t = {}
for line in io.lines() do
	t[#t + 1] = line .. "\n"
end
local s = table.concat(t)

-- concat 方法可以接受两个参数，因此上面的方式还可以改为：
local t = {}
for line in io.lines() do
	t[#t + 1] = line
end
local s = table.concat(t, "\n")

-- 包
function insert(bag, element)
	bag[element] = (bag[element] or 0) + 1
end

function remove(bag, element)
	local count = bag[element]
	bag[element] = (count and count > 1) and count - 1 or nil
end


-- 队列与双向队列
List = {}

function List.new()
	return {first = 0, last = -1}
end

function List.pushFront(list, value)
	local first = list.first - 1
	list.first = first
	list[first] = value
end

function List.pushBack(list, value)
	local last = list.last + 1
	list.last = last
	list[last] = value
end

function List.popFront(list)
	local first = list.first
	if first > list.last then
		error("List is empty")
	end
	local value = list[first]
	list[first] = nil
	list.first = first + 1
	return value
end

function List.popBack(list)
	local last = list.last
	if list.first > last then
		error("List is empty")
	end
	local value = list[last]
	list[last] = nil
	list.last = last - 1
	return value
end

-- 链表
list = nil
for i = 1, 10 do
	list = { next = list, value = i }
end

local l = list
while l do
	print(l.value)
	l = l.next
end

-- 错误处理

function foo()
	local a = 10
	print(a[2])
end

function errorHandle()
	print(debug.traceback())
end

if xpcall(foo, errorHandle) then
	print("This is OK.")
else
	print("This is error.")
end

local function IterateTable(tab)
	local count = 0
	for index, value in pairs(tab) do
		print(index, value)
		count = count + 1
	end
	return count
end

local file = io.open("d:\\program\\test_data.lua", "w")
if file ~= nil then
	file:write("-- Test Lua file")
	file:write(string.char(10))
	file:write(string.char(10))
	
	file:write(string.format("%s%s", "-- File created on: ", os.date()))
	file:write(string.char(10))
	file:write(string.char(10))
	
	file:write("print(\"hello world\")")
	io.close(file)
end

function GetLines(fileName)
    indx = 0
    myLines = {}
    for line in io.lines(string.format("%s%s", "c:/lua_scripts/", fileName)) do
        indx = indx + 1
        myLines[indx] = line
    end
    return indx, myLines --returns number of lines and line table
end

function GetValues(myString)
    num = 0
    values = {}
    if myString ~= nil then
        while string.find(myString,",") ~= nil do
            i,j = string.find(myString,",")
            num = num + 1
            values[num] = string.sub(myString,1, j-1)
            myString = string.sub(myString, j+1, string.len(myString))
        end
        num = num + 1
        values[num] = myString
    end
    return num, values
end

function LoadCharacters()
    myCharacters = {}
    numLines, allLines = GetLines("data1.csv")
    --load labels (the first line)
    count, myLabels = GetValues(allLines[1])
    --ignore line 1, it's got the labels
    for indx = 2, numLines do
        count, charHold = GetValues(allLines[indx])
        myCharacters[indx-1] = {}
        for indx2 = 1, count do
            myCharacters[indx-1][indx2] = charHold[indx2]
        end
    end
    --now print them
    for indx = 1, 3 do
        for indx2 = 1, table.getn(myLabels) do
            print(myLabels[indx2], myCharacters[indx][indx2])
        end
    end
end

--script to test
--GetLines("data1.csv")
--count, myCharacters = GetValues(myLines[1])
--for indx = 1,count do
--    print(myCharacters[indx])
--end

math.randomseed(tostring(os.time()):reverse():sub(1,6))

function Sort(theTable, direction)
	if direction ~= 1 then
		table.sort(theTable)
	else
		function Reverse(a, b)
			if a < b then
				return false
			else
				return true
			end
		end
		table.sort(theTable, Reverse)
	end
end

--获取可变参数个数方法
local function GetVarargNum( ... )
	arg = { ... }
	local count = 0
	for k, v in pairs(arg) do
		count = count + 1
	end
	return count
end

-- 获取可变参数中指定位置的参数(传入的参数为table)
local function GetTableArg( ... )
	arg = select("1", ...)	--返回第一个参数和其之后的所有参数
	print(arg[1], arg["nihao"])
end

test = {1}
test.nihao = 2
GetTableArg(test)

-- 获取可变参数中指定位置的参数(传入的参数不是table)
local function GetArg( ... )
	temp = { ... }
	arg = select("1", temp)	--返回第一个参数和其之后的所有参数
	print(arg[1], arg[2])
end

GetArg(1, 2)

--]]