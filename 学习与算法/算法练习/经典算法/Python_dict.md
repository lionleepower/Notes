**一、字典本质（你先建立模型）**  
Python 的 dict 本质是：  
**key → value** **的哈希映射（****Hash Table****）**  
特点：

- 查找：O(1)
- 插入：O(1)
- key 必须是 **不可变类型（****hashable****）**
    - ✅ int / str / tuple
    - ❌ list / dict
 
**二、最基本用法（必须会）**  
**1.** **创建**
 
==d = {}==
 
==d = dict()==
 
==d = {"a": 1, "b": 2}==
 
**2.** **访问**
 
==d["a"] # 1==  
==⚠️== ==如果== ==key== ==不存在== ==→== ==报错==
 
**3.** **安全访问（很重要）**
 
==d.get("a") # 1==  
==d.get("c") # None==  
==d.get("c", 0) # 0====（默认值）==
 
**4.** **添加** **/** **修改**
 
d["c"] = 3  
d["a"] = 10 # 修改
 
**5.** **删除**
 
del d["a"]
 
d.pop("b") # 返回 value  
d.pop("x", 0) # 不存在也不会报错
 
**三、遍历（面试常考）**
 
# 遍历 key  
for k in d:  
print(k)
 
# key + value  
for k, v in d.items():  
print(k, v)
 
# 只 value  
for v in d.values():  
print(v)
 
**四、算法题核心用法（重点）**  
你刷 LeetCode 真正用到的是这几个：
 
**1.** **计数（最常见）**
 
count = {}
 
for x in nums:  
if x in count:  
count[x] += 1  
else:  
count[x] = 1  
👉 更简洁：
 
count[x] = count.get(x, 0) + 1
 
**2.** **判断存在（****O(1)****）**
 
if x in d:  
...
 
**3.** **哈希表解题（经典** **Two Sum****）**
 
nums = [2,7,11,15]  
target = 9
 
d = {}
 
for i, x in enumerate(nums):  
if target - x in d:  
return [d[target - x], i]  
d[x] = I
   

**为什么算法题特别爱用它**  
经典场景：**分组** **/** **记录位置**  
==pos = defaultdict(list)==￼
 
from collections import defaultdict  
pos = defaultdict(list)  
含义是：  
**当** **key** **不存在时，自动创建一个空** **list** **作为默认值**  
等价逻辑是：
 
pos = {}
 
if key not in pos:  
pos[key] = []  
pos[key].append(value)  
但 defaultdict 帮你自动做了这一步。
 
**例子：记录每个数字出现的位置**
 
from collections import defaultdict  
pos = defaultdict(list)
 
nums = [1,2,1,3,2]  
for i, x in enumerate(nums):  
pos[x].append(i)
 
# 结果：  
==# {1: [0,2], 2: [1,4], 3: [3]}==￼￼  
**结果为什么是** **{1: [0,2], 2: [1,4], 3: [3]}**  
我们一步一步拆开看（这是关键）
 
nums = [1,2,1,3,2]  
你在做的是：  
**记录每个数字出现的位置（索引）**
 
**逐步执行过程**
 
for i, x in enumerate(nums):  
enumerate(nums) 会产生：
 
(0, 1)  
(1, 2)  
(2, 1)  
(3, 3)  
(4, 2)
 
**然后这一句：**
 
pos[x].append(i)  
我们一行一行看：
 
**第****1****步**
 
i=0, x=1
 
pos[1].append(0)  
👉 pos = {1: [0]}
 
**第****2****步**
 
i=1, x=2  
👉 pos = {1: [0], 2: [1]}
 
**第****3****步**
 
i=2, x=1  
👉 pos = {1: [0,2], 2: [1]}
 
**第****4****步**
 
i=3, x=3  
👉 pos = {1: [0,2], 2: [1], 3: [3]}
 
**第****5****步**
 
i=4, x=2  
👉 pos = {1: [0,2], 2: [1,4], 3: [3]}
 
**如果不用** **defaultdict****（你会写成这样）**
 
pos = {}
 
for i, x in enumerate(nums):  
if x not in pos:  
pos[x] = []  
pos[x].append(i)