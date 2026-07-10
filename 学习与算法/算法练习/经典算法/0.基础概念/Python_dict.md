# Python dict：字典语法与使用实例

Python 的 `dict` 用 `key → value` 建立映射，适合快速查找、计数、记录位置和分组。算法题出现“是否出现过”“出现几次”“值对应哪个位置”时，优先考虑字典。

---

## 思路

如果每次查找都遍历列表，单次查询需要 $O(n)$；`dict` 基于哈希表实现，查找、插入和删除平均可以做到 $O(1)$，本质是用额外空间换取更快查询。

> [!tip]- 核心思路
> 字典保存的是映射关系：
>
> ```text
> key → value
> ```
>
> 常见用途：
>
> - 查重：元素是否已经出现。
> - 计数：每个元素出现几次。
> - 定位：某个值对应哪个下标。
> - 分组：同一类元素放到同一个列表。
> - 缓存：输入对应已经计算出的结果。

---

## 1. 创建字典

> [!success]- 常见创建方式
> ```python
> empty1 = {}
> empty2 = dict()
>
> scores = {"Alice": 90, "Bob": 85}
> coordinates = dict(x=10, y=20)
> pairs = dict([("a", 1), ("b", 2)])
>
> keys = ["a", "b", "c"]
> initialized = dict.fromkeys(keys, 0)
> ```

字典的键不能重复。重复赋值会覆盖旧值：

```python
d = {"a": 1}
d["a"] = 2
print(d)  # {'a': 2}
```

---

## 2. 读取与判断

| 写法 | 作用 | 键不存在时 |
| --- | --- | --- |
| `d[key]` | 读取值 | 抛出 `KeyError` |
| `d.get(key)` | 安全读取 | 返回 `None` |
| `d.get(key, default)` | 带默认值读取 | 返回 `default` |
| `key in d` | 判断键是否存在 | 返回 `False` |

> [!example]- 安全访问
> ```python
> scores = {"Alice": 90}
>
> print(scores["Alice"])          # 90
> print(scores.get("Bob"))        # None
> print(scores.get("Bob", 0))     # 0
> print("Alice" in scores)         # True
> ```

`in` 默认检查键，不检查值。检查值需要写 `value in d.values()`，但这通常需要线性扫描。

---

## 3. 添加、更新与删除

> [!success]- 常用修改操作
> ```python
> d = {"a": 1, "b": 2}
>
> d["c"] = 3                 # 添加
> d["a"] = 10                # 更新
> d.update({"b": 20, "d": 4})
>
> value = d.pop("c")         # 删除并返回值
> missing = d.pop("x", None) # 不存在时返回默认值
> last_pair = d.popitem()     # 删除并返回最后加入的键值对
>
> del d["a"]                 # 键不存在会抛出 KeyError
> d.clear()                   # 清空字典
> ```

只在键不存在时写入默认值，可以使用：

```python
d.setdefault("items", []).append(1)
```

如果需要频繁分组，`defaultdict` 通常更清晰。

---

## 4. 遍历字典

> [!success]- 三种遍历方式
> ```python
> scores = {"Alice": 90, "Bob": 85}
>
> for name in scores:
>     print(name)
>
> for score in scores.values():
>     print(score)
>
> for name, score in scores.items():
>     print(name, score)
> ```

| 方法 | 得到的内容 |
| --- | --- |
| `d.keys()` | 所有键的动态视图 |
| `d.values()` | 所有值的动态视图 |
| `d.items()` | 所有 `(key, value)` 的动态视图 |

不要在遍历字典时直接改变它的大小。需要删除元素时，可以先复制键：

```python
for key in list(d):
    if should_remove(key):
        del d[key]
```

---

## 5. 键必须可哈希

字典需要通过键的哈希值定位位置，因此键必须是可哈希对象：生命周期内哈希值保持不变，并且能参与相等比较。

| 可以作为键 | 通常不能作为键 |
| --- | --- |
| `int`、`str`、`bytes` | `list` |
| 只含可哈希元素的 `tuple` | `dict` |
| `frozenset` | `set` |

> [!warning]- 元组不一定可哈希
> ```python
> valid = {(1, 2): "point"}
> invalid = {([1, 2], 3): "value"}  # TypeError
> ```
>
> 元组内部如果包含列表等不可哈希对象，整个元组也不能作为键。

---

## 6. 实例一：元素计数

> [!success]- 使用 `get`
> ```python
> def count_items(nums):
>     counts = {}
>
>     for value in nums:
>         counts[value] = counts.get(value, 0) + 1
>
>     return counts
>
>
> print(count_items([1, 2, 1, 3, 2, 1]))
> # {1: 3, 2: 2, 3: 1}
> ```

标准库也提供 `Counter`：

```python
from collections import Counter

counts = Counter([1, 2, 1, 3, 2, 1])
print(counts[1])  # 3
```

手写 `get` 模板更能体现哈希计数思路，`Counter` 更适合工程代码或允许使用标准库的场景。

---

## 7. 实例二：分组

> [!success]- 使用 `defaultdict(list)`
> ```python
> from collections import defaultdict
>
> def group_positions(nums):
>     positions = defaultdict(list)
>
>     for index, value in enumerate(nums):
>         positions[value].append(index)
>
>     return dict(positions)
>
>
> print(group_positions([1, 2, 1, 3, 2]))
> # {1: [0, 2], 2: [1, 4], 3: [3]}
> ```

访问不存在的键时，`defaultdict(list)` 会自动创建空列表，省去手动初始化。

---

## 8. 实例三：Two Sum

遍历当前数字 `value` 时，只需判断补数 `target - value` 是否已经出现。字典保存“数字 → 下标”。

> [!success]- 一次遍历
> ```python
> def two_sum(nums, target):
>     positions = {}
>
>     for index, value in enumerate(nums):
>         need = target - value
>
>         if need in positions:
>             return [positions[need], index]
>
>         positions[value] = index
>
>     return []
>
>
> print(two_sum([2, 7, 11, 15], 9))  # [0, 1]
> ```

> [!tip]- 为什么先查找再存当前下标
> 先判断 `need in positions`，可以保证补数来自之前的元素，避免在目标是当前值两倍时把同一个元素使用两次。

---

## 9. 字典推导式与合并

> [!success]- 常用写法
> ```python
> squares = {x: x * x for x in range(5)}
>
> left = {"a": 1, "b": 2}
> right = {"b": 20, "c": 3}
>
> merged1 = {**left, **right}
> merged2 = left | right  # Python 3.9+
> ```

键冲突时，后面的字典覆盖前面的值。上例中合并结果里的 `"b"` 是 `20`。

---

## 10. dict、set 与 list 怎么选

| 需求 | 推荐结构 |
| --- | --- |
| `key → value` 映射 | `dict` |
| 只判断元素是否存在或去重 | `set` |
| 按顺序保存并通过下标访问 | `list` |
| 统计次数 | `dict` 或 `Counter` |
| 每个键对应多个元素 | `defaultdict(list)` |

如果只需要记录“见过哪些元素”，使用 `set` 比保存无意义字典值更直接；需要附带次数、下标或其他信息时使用 `dict`。

---

## 11. 常见易错点

> [!warning]- 易错点
> - 用 `d[key]` 读取不存在的键，触发 `KeyError`。
> - 误以为 `value in d` 检查值；它实际检查键。
> - 使用 `list`、`dict` 或 `set` 作为键，触发 `TypeError`。
> - 计数时直接写 `counts[x] += 1`，却没有初始化。
> - 遍历字典时增加或删除键，触发运行时错误。
> - 用 `dict.fromkeys(keys, [])` 创建多个列表；所有键会共享同一个列表对象。
> - Two Sum 先存当前值再查补数，可能错误使用同一元素两次。

避免共享默认列表：

```python
safe = {key: [] for key in keys}
```

---

## 复杂度

> [!summary]- 平均复杂度
> - 查找、插入、更新、删除：平均 $O(1)$。
> - 遍历、复制字典：$O(n)$。
> - 额外空间：保存 $n$ 个键值对需要 $O(n)$。
>
> 哈希冲突严重时，单次操作最坏可能退化；算法分析通常使用平均复杂度 $O(1)$。

---

## 一句话总结

`dict` 用哈希表维护 `key → value` 映射，能把查重、计数、定位和分组中的频繁查找优化为平均 $O(1)$。

---

## 模板归纳

```python
# 安全读取
value = d.get(key, default)

# 计数
counts[x] = counts.get(x, 0) + 1

# 记录位置
positions[x] = index

# 分组
from collections import defaultdict

groups = defaultdict(list)
groups[key].append(value)

# Two Sum
positions = {}
for index, value in enumerate(nums):
    need = target - value
    if need in positions:
        return [positions[need], index]
    positions[value] = index
```

看到“是否出现、出现几次、对应位置、建立映射、按键分组”，优先想到 `dict`。
