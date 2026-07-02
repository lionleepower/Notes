# 题目

Python 字典 `dict` 与 `Hash Table`：用 `key -> value` 快速查找、计数、记录位置和建立映射。

---

## 思路

算法题里经常需要快速判断一个元素是否出现过，或者快速找到某个元素对应的信息。  
如果每次都遍历数组，查找会变成 $O(n)$。  
==`dict` 底层基于 `Hash Table`==，平均可以把查找、插入、删除降到 $O(1)$。  
所以遇到“查重 / 计数 / 映射 / 记录位置”时，优先想到 `Hash Map`。

---

> [!tip]- 核心思路  
> Python 的 `dict` 本质是：
> 
> ```text
> key -> value
> ```
> 
> 也就是通过一个 `key` 快速找到对应的 `value`。
> 
> 常见用途：
> 
> - 计数：统计每个元素出现几次
>     
> - 查重：判断元素是否出现过
>     
> - 记录位置：记录某个值对应的下标
>     
> - 建立映射：把一种信息映射到另一种信息
>     
> 
> 平均复杂度：
> 
> - 查找：$O(1)$
>     
> - 插入：$O(1)$
>     
> - 删除：$O(1)$
>     
> 
> `dict` 适合算法题的原因：  
> 它可以用额外空间换时间，把原本需要遍历查找的问题，变成哈希查询。

---

> [!example]- 示例推演  
> 以 `Two Sum` 为例：
> 
> ```python
> nums = [2, 7, 11, 15]
> target = 9
> ```
> 
> 遍历过程：
> 
> ```text
> i = 0, x = 2
> need = 9 - 2 = 7
> 7 不在 pos 中
> pos[2] = 0
> 
> i = 1, x = 7
> need = 9 - 7 = 2
> 2 在 pos 中
> 返回 [pos[2], 1] = [0, 1]
> ```
> 
> 核心是：  
> 当前数 `x` 只需要判断 `target - x` 是否已经出现过。

---

> [!success]- C++/Python 实现
> 
> ## 基础用法
> 
> ```python
> d = {}
> d = dict()
> d = {"a": 1, "b": 2}
> ```
> 
> ## 安全访问
> 
> ```python
> d = {"a": 1, "b": 2}
> 
> print(d.get("a"))      # 1
> print(d.get("c"))      # None
> print(d.get("c", 0))   # 0
> ```
> 
> ## 计数模板
> 
> ```python
> count = {}
> 
> for x in nums:
>     count[x] = count.get(x, 0) + 1
> ```
> 
> ## 记录位置
> 
> ```python
> pos = {}
> 
> for i, x in enumerate(nums):
>     pos[x] = i
> ```
> 
> ## 分组 / 记录多个位置
> 
> ```python
> from collections import defaultdict
> 
> nums = [1, 2, 1, 3, 2]
> pos = defaultdict(list)
> 
> for i, x in enumerate(nums):
>     pos[x].append(i)
> 
> print(dict(pos))
> ```
> 
> 输出：
> 
> ```python
> {1: [0, 2], 2: [1, 4], 3: [3]}
> ```
> 
> ## Two Sum 模板
> 
> ```python
> class Solution:
>     def twoSum(self, nums: list[int], target: int) -> list[int]:
>         pos = {}
> 
>         for i, x in enumerate(nums):
>             need = target - x
> 
>             if need in pos:
>                 return [pos[need], i]
> 
>             pos[x] = i
> ```
> 
> 主流程：
> 
> - 遍历数组中的每个元素 `x`
>     
> - 计算当前需要的另一个数 `need = target - x`
>     
> - 如果 `need` 已经出现过，直接返回答案
>     
> - 否则记录当前数字的位置
>     
> 
> 关键变量：
> 
> - `pos`：记录数字对应的下标
>     
> - `x`：当前遍历到的数字
>     
> - `need`：当前数字需要匹配的另一个数
>     
> 
> 容易写错的位置：
> 
> - `Two Sum` 要先判断 `need in pos`，再存 `pos[x] = i`
>     
> - 否则可能把当前元素自己和自己配对
>     

---

> [!warning]- 易错点
> 
> - 访问不存在的 `key`：
>     
> 
> ```python
> d["x"]
> ```
> 
> 如果 `x` 不存在，会触发 `KeyError`。
> 
> 推荐：
> 
> ```python
> d.get("x", 0)
> ```
> 
> - `key` 必须是 `hashable`
>     
> 
> 可以作为 `key`：
> 
> ```python
> int
> str
> tuple
> ```
> 
> 不能作为 `key`：
> 
> ```python
> list
> dict
> set
> ```
> 
> 原因：  
> `Hash Table` 需要根据哈希值定位位置，可变对象内容会变化，哈希值不稳定。
> 
> - 计数时忘记初始化
>     
> 
> 不推荐：
> 
> ```python
> count[x] += 1
> ```
> 
> 如果 `x` 第一次出现，会报错。
> 
> 推荐：
> 
> ```python
> count[x] = count.get(x, 0) + 1
> ```
> 
> - `defaultdict` 打印结果不是普通字典
>     
> 
> ```python
> print(pos)
> ```
> 
> 可能显示：
> 
> ```python
> defaultdict(<class 'list'>, {...})
> ```
> 
> 可以转换：
> 
> ```python
> print(dict(pos))
> ```

---

> [!summary]- 复杂度  
> 时间复杂度：
> 
> $O(1)$
> 
> 空间复杂度：
> 
> $O(n)$
> 
> 为什么：
> 
> - `dict` 的查找、插入、删除平均是 $O(1)$
>     
> - 如果最多记录 $n$ 个不同元素，需要 $O(n)$ 额外空间
>     
> - 在算法题中，本质是用空间换时间
>     

---

## 一句话总结

字典 `dict` 本质是 `Hash Table`，用 `key -> value` 把查找、计数、记录位置这类问题优化成平均 $O(1)$ 查询。

---

## 模板归纳

遇到以下情况，优先想到 `Hash Map / dict`：

- 需要判断元素是否出现过
    
- 需要统计元素出现次数
    
- 需要记录元素下标
    
- 需要建立一一映射或分组映射
    
- 题目中出现“两个数是否匹配”“是否存在某个补数”
    
- 暴力做法中存在频繁查找，导致 $O(n^2)$
    

常用模板：

```python
# 判断是否出现
if x in d:
    ...

# 计数
count[x] = count.get(x, 0) + 1

# 记录位置
pos[x] = i

# 分组
from collections import defaultdict

groups = defaultdict(list)

for i, x in enumerate(nums):
    groups[x].append(i)

# Two Sum
pos = {}

for i, x in enumerate(nums):
    need = target - x

    if need in pos:
        return [pos[need], i]

    pos[x] = i
```