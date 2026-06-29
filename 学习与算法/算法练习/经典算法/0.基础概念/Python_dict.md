# Python 字典 Dict 与 Hash Table

## 1. 字典要解决什么问题？

Python 的 `dict` 本质是一个 `Hash Table`，用来维护：

```text
key -> value
```

也就是通过一个 `key` 快速找到对应的 `value`。

在算法题里，字典最常用于：

- 计数
    
- 判断元素是否出现过
    
- 记录元素位置
    
- 建立映射关系
    

## 2. 核心思路

`dict` 的核心优势是平均情况下可以做到：

- 查找：$O(1)$
    
- 插入：$O(1)$
    
- 删除：$O(1)$
    

所以当题目需要频繁判断“某个元素是否出现过”或“某个元素对应什么值”时，优先考虑 `Hash Map`。

例如 `Two Sum`：

```python
nums = [2, 7, 11, 15]
target = 9
```

遍历到 `7` 时，只需要判断：

```python
target - 7 = 2
```

是否已经在字典中即可。

## 3. 关键规则 / 判断条件

字典的 `key` 必须是不可变类型，也就是 `hashable`。

可以作为 `key`：

```python
int
str
tuple
```

不能作为 `key`：

```python
list
dict
set
```

原因是 `key` 需要通过哈希值定位位置，而可变对象内容会变化，哈希值不稳定。

## 4. 代码模板

### 4.1 创建字典

```python
d = {}

d = dict()

d = {"a": 1, "b": 2}
```

### 4.2 访问元素

```python
d = {"a": 1, "b": 2}

print(d["a"])
```

注意：如果 `key` 不存在，会报错。

```python
print(d["c"])  # KeyError
```

### 4.3 安全访问 get

```python
d = {"a": 1, "b": 2}

print(d.get("a"))      # 1
print(d.get("c"))      # None
print(d.get("c", 0))   # 0
```

`get(key, default)` 的含义是：

```text
如果 key 存在，返回对应 value
如果 key 不存在，返回 default
```

### 4.4 添加 / 修改

```python
d = {"a": 1}

d["c"] = 3     # 添加
d["a"] = 10    # 修改
```

### 4.5 删除

```python
d = {"a": 1, "b": 2}

del d["a"]

value = d.pop("b")
```

如果不确定 `key` 是否存在，可以写：

```python
d.pop("x", 0)
```

这样 `x` 不存在时不会报错，而是返回默认值 `0`。

## 5. 遍历方式

### 遍历 key

```python
for k in d:
    print(k)
```

### 遍历 key 和 value

```python
for k, v in d.items():
    print(k, v)
```

### 只遍历 value

```python
for v in d.values():
    print(v)
```

## 6. 算法题核心用法

### 6.1 计数

普通写法：

```python
count = {}

for x in nums:
    if x in count:
        count[x] += 1
    else:
        count[x] = 1
```

更简洁写法：

```python
count = {}

for x in nums:
    count[x] = count.get(x, 0) + 1
```

小例子：

```python
nums = [1, 2, 1, 3, 2, 1]

count = {}

for x in nums:
    count[x] = count.get(x, 0) + 1

print(count)
```

结果：

```python
{1: 3, 2: 2, 3: 1}
```

### 6.2 判断是否存在

```python
if x in d:
    ...
```

这是算法题里最常见的 $O(1)$ 判断。

### 6.3 Two Sum 模板

```python
class Solution:
    def twoSum(self, nums: list[int], target: int) -> list[int]:
        pos = {}

        for i, x in enumerate(nums):
            need = target - x

            if need in pos:
                return [pos[need], i]

            pos[x] = i
```

核心逻辑：

- `pos[x] = i`：记录数字 `x` 出现的位置
    
- `need = target - x`：当前数字需要的另一个数
    
- 如果 `need in pos`，说明之前已经见过答案
    

## 7. defaultdict

`defaultdict` 可以自动给不存在的 `key` 创建默认值。

常见用法：

```python
from collections import defaultdict

pos = defaultdict(list)
```

含义：

```text
当 key 不存在时，自动创建一个空 list
```

等价于：

```python
pos = {}

if key not in pos:
    pos[key] = []

pos[key].append(value)
```

### 例子：记录每个数字出现的位置

```python
from collections import defaultdict

nums = [1, 2, 1, 3, 2]

pos = defaultdict(list)

for i, x in enumerate(nums):
    pos[x].append(i)

print(dict(pos))
```

结果：

```python
{1: [0, 2], 2: [1, 4], 3: [3]}
```

## 8. 为什么结果是 `{1: [0, 2], 2: [1, 4], 3: [3]}`？

原数组：

```python
nums = [1, 2, 1, 3, 2]
```

`enumerate(nums)` 会产生：

```python
(0, 1)
(1, 2)
(2, 1)
(3, 3)
(4, 2)
```

每一步执行：

```python
pos[x].append(i)
```

过程如下：

```text
i = 0, x = 1
pos[1].append(0)
pos = {1: [0]}

i = 1, x = 2
pos[2].append(1)
pos = {1: [0], 2: [1]}

i = 2, x = 1
pos[1].append(2)
pos = {1: [0, 2], 2: [1]}

i = 3, x = 3
pos[3].append(3)
pos = {1: [0, 2], 2: [1], 3: [3]}

i = 4, x = 2
pos[2].append(4)
pos = {1: [0, 2], 2: [1, 4], 3: [3]}
```

所以最终结果是：

```python
{1: [0, 2], 2: [1, 4], 3: [3]}
```

## 9. 不用 defaultdict 的写法

```python
nums = [1, 2, 1, 3, 2]

pos = {}

for i, x in enumerate(nums):
    if x not in pos:
        pos[x] = []

    pos[x].append(i)

print(pos)
```

这和 `defaultdict(list)` 的效果一样，只是代码更长。

## 10. 复杂度分析

对于常见字典操作：

时间复杂度：

$$  
O(1)  
$$

空间复杂度：

$$  
O(n)  
$$

解释：查找、插入、删除平均是 $O(1)$；如果记录了最多 $n$ 个不同元素，空间复杂度就是 $O(n)$。

## 11. 核心结论

- `dict` 本质是 `Hash Table`
    
- 核心结构是 `key -> value`
    
- 查找、插入、删除平均都是 $O(1)$
    
- 算法题里常用来计数、查重、记录位置、建立映射
    
- `get()` 适合安全访问
    
- `defaultdict(list)` 适合分组和记录多个位置
    
- `Two Sum` 是最经典的 `Hash Map` 题型
    

## 12. 常见误区

- 用不存在的 `key` 直接访问，导致 `KeyError`
    
- 把 `list` 当作字典的 `key`
    
- 写计数时忘记初始化
    
- `Two Sum` 中先存当前数字再判断，可能错误使用同一个元素
    
- 使用 `defaultdict` 后，打印结果时看到的是 `defaultdict(...)`，可以用 `dict(pos)` 转成普通字典
    

---

# 最后速记版

## dict 基本模型

```text
key -> value
```

## 计数模板

```python
count[x] = count.get(x, 0) + 1
```

## 判断存在

```python
if x in d:
    ...
```

## 记录位置

```python
pos[x] = i
```

## 分组 / 记录多个位置

```python
from collections import defaultdict

pos = defaultdict(list)

for i, x in enumerate(nums):
    pos[x].append(i)
```

## Two Sum

```python
pos = {}

for i, x in enumerate(nums):
    need = target - x

    if need in pos:
        return [pos[need], i]

    pos[x] = i
```

## 一句话总结

字典就是用空间换时间，把原本需要遍历查找的问题，变成 $O(1)$ 的哈希查询。