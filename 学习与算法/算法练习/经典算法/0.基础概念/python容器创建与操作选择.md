
# Python List、Tuple、Dict：三种常用容器的创建、操作与选择

`list`、`tuple` 和 `dict` 都可以保存多个对象，但它们解决的问题不同：`list` 适合保存有顺序且需要修改的数据，`tuple` 适合表达固定结构，`dict` 适合根据键快速查找数据。

算法题中，连续遍历通常使用 `list`，固定状态或坐标常使用 `tuple`，计数、映射和快速查询通常使用 `dict`。

---

## 思路

程序经常需要保存多个数据，但不同场景对数据的要求不同。

- 数据需要增加、删除或修改时，使用 `list`
    
- 数据结构固定、不希望被修改时，使用 `tuple`
    
- 需要根据某个键快速找到对应值时，使用 `dict`
    

如果所有场景都使用 `list`，代码虽然可能运行，但数据含义会不够清晰，而且某些查询操作需要遍历整个列表，时间复杂度可能从 $O(1)$ 上升到 $O(n)$。

> [!tip]- 核心思路  
> `list` 管理有序可变数据，`tuple` 表达固定结构，`dict` 建立键和值之间的映射。
> 
> ```python
> nums = [1, 2, 3]
> point = (10, 20)
> counter = {"apple": 2, "banana": 1}
> ```
> 
> 判断依据是：数据是否需要修改，以及数据是通过位置访问还是通过名称或键访问。

---

## 1. 导入与创建

`list`、`tuple` 和 `dict` 都是 Python 内置类型，不需要额外导入。

> [!success]- 基础语法
> 
> ```python
> # List
> empty_list = []
> nums = [1, 2, 3]
> 
> # Tuple
> empty_tuple = ()
> point = (10, 20)
> single = (1,)
> 
> # Dict
> empty_dict = {}
> student = {
>     "name": "Leyan",
>     "score": 90,
> }
> ```

也可以使用对应的构造函数：

```python
empty_list = list()
empty_tuple = tuple()
empty_dict = dict()
```

三种容器都可以保存不同类型的对象：

```python
data_list = [1, "hello", True, None]
data_tuple = (1, "hello", True, None)

data_dict = {
    "number": 1,
    "text": "hello",
    "flag": True,
    "value": None,
}
```

它们对保存的数据类型通常没有统一要求。

==但字典的键必须是可哈希对象，常见可用类型包括==：

```python
data = {
    "name": "Leyan",
    1: "one",
    (10, 20): "point",
}
```

列表不能作为字典键：

```python
data = {
    [1, 2]: "value"
}
```

这会产生：

```text
TypeError: unhashable type: 'list'
```

---

## 2. 核心操作

### List

|操作|作用|时间复杂度或注意事项|
|---|---|---|
|`nums[i]`|根据下标访问元素|$O(1)$|
|`nums[i] = value`|修改指定位置|$O(1)$|
|`nums.append(x)`|在末尾添加元素|均摊 $O(1)$|
|`nums.pop()`|删除末尾元素|$O(1)$|
|`nums.insert(i, x)`|在指定位置插入|$O(n)$|
|`nums.pop(i)`|删除指定位置元素|$O(n)$|
|`x in nums`|判断元素是否存在|$O(n)$|
|`len(nums)`|获取元素数量|$O(1)$|

### Tuple

|操作|作用|时间复杂度或注意事项|
|---|---|---|
|`point[i]`|根据下标访问元素|$O(1)$|
|`x in point`|判断元素是否存在|$O(n)$|
|`len(point)`|获取元素数量|$O(1)$|
|`x, y = point`|元组解包|$O(n)$|
|`point[i] = value`|修改元素|不允许|
|`point + other`|创建新的元组|$O(n+m)$|

### Dict

|操作|作用|时间复杂度或注意事项|
|---|---|---|
|`data[key]`|根据键获取值|平均 $O(1)$|
|`data[key] = value`|添加或修改键值对|平均 $O(1)$|
|`data.get(key)`|安全获取值|平均 $O(1)$|
|`key in data`|判断键是否存在|平均 $O(1)$|
|`data.pop(key)`|删除键值对|平均 $O(1)$|
|`data.keys()`|获取所有键|返回动态视图|
|`data.values()`|获取所有值|返回动态视图|
|`data.items()`|获取键值对|常用于遍历|

> [!example]- 最小示例
> 
> ```python
> nums = [10, 20, 30]
> nums.append(40)
> 
> point = (100, 200)
> x, y = point
> 
> student = {
>     "name": "Leyan",
>     "score": 90,
> }
> student["score"] = 95
> 
> print(nums)
> print(x, y)
> print(student["score"])
> ```

输出：

```text
[10, 20, 30, 40]
100 200
95
```

---

## 3. 关键行为与边界

### List 是可变对象

列表创建后，可以修改内部元素：

```python
nums = [1, 2, 3]

nums[0] = 100
nums.append(4)

print(nums)
```

结果：

```text
[100, 2, 3, 4]
```

### Tuple 是不可变对象

元组创建后，不能替换、添加或删除其中的元素：

```python
point = (10, 20)
point[0] = 100
```

会产生：

```text
TypeError: 'tuple' object does not support item assignment
```

但是，==元组内部可以保存可变对象==：

```python
data = ([1, 2], "hello")

data[0].append(3)

print(data)
```

结果：

```text
([1, 2, 3], 'hello')
```

这里并没有替换 `data[0]`，而是修改了 `data[0]` 指向的列表。

### Dict 通过键访问，而不是通过位置访问

```python
student = {
    "name": "Leyan",
    "score": 90,
}

print(student["name"])
```

不能将字典理解为普通下标数组：

```python
print(student[0])
```

除非字典中真的存在键 `0`，否则会抛出 `KeyError`。

### 字典键不能重复

```python
data = {
    "name": "Alice",
    "name": "Bob",
}

print(data)
```

结果：

```text
{'name': 'Bob'}
```

后面的值会覆盖前面的值。

> [!tip]- 为什么这样做  
> `list` 和 `tuple` 通过整数下标定位元素，而 `dict` 使用哈希表，根据键计算存储位置，因此平均查询时间可以达到 $O(1)$。

> [!warning]- 边界条件
> 
> - 空列表写作 `[]`，空元组写作 `()`，空字典写作 `{}`。
>     
> - `{}` 表示空字典，不表示空集合。
>     
> - 单元素元组必须写成 `(value,)`。
>     
> - 列表和元组允许重复元素。
>     
> - 字典的键不能重复，但值可以重复。
>     
> - 字典键必须可哈希，列表、字典和集合通常不能作为键。
>     
> - 访问不存在的字典键时，`data[key]` 会报错，`data.get(key)` 默认返回 `None`。
>     
> - 三种容器都可以保存 `None`、负数、字符串或其他容器。
>     

---

## 4. 实例一：保存不同结构的数据

假设需要保存一组学生信息。学生列表会不断增加，因此外层使用 `list`；每个学生通过字段名访问信息，因此内部使用 `dict`；出生日期结构固定，因此使用 `tuple`。

> [!success]- 完整实现
> 
> ```python
> def add_student(students, name, score, birthday):
>     student = {
>         "name": name,
>         "score": score,
>         "birthday": birthday,
>     }
> 
>     students.append(student)
>     return students
> 
> 
> students = []
> 
> add_student(
>     students,
>     name="Leyan",
>     score=90,
>     birthday=(2002, 5, 20),
> )
> 
> add_student(
>     students,
>     name="Alice",
>     score=85,
>     birthday=(2001, 10, 12),
> )
> 
> for student in students:
>     year, month, day = student["birthday"]
> 
>     print(student["name"])
>     print(student["score"])
>     print(year, month, day)
> ```

这里三种数据结构分别承担不同职责：

```python
students = []
```

`students` 使用列表，因为学生数量可能不断增加。

```python
student = {
    "name": name,
    "score": score,
}
```

单个学生使用字典，因为需要通过 `"name"`、`"score"` 等字段访问信息。

```python
birthday = (2002, 5, 20)
```

日期使用元组，因为它具有固定的年、月、日结构。

最容易写错的是单元素元组：

```python
birthday = (2002)
```

这只是一个整数。

正确写法是：

```python
birthday = (2002,)
```

---

## 5. 实例二：两数之和

给定一个整数列表和目标值，寻找两个元素，使它们的和等于目标值。

输入数据使用 `list`，查询历史元素使用 `dict`，找到答案后使用 `tuple` 返回两个下标。

> [!success]- 完整实现
> 
> ```python
> def two_sum(nums, target):
>     seen = {}
> 
>     for index, value in enumerate(nums):
>         needed = target - value
> 
>         if needed in seen:
>             return seen[needed], index
> 
>         seen[value] = index
> 
>     return None
> 
> 
> nums = [2, 7, 11, 15]
> result = two_sum(nums, 9)
> 
> print(result)
> ```

输出：

```text
(0, 1)
```

其中：

```python
nums = [2, 7, 11, 15]
```

列表保存输入序列，并且允许通过下标访问元素。

```python
seen = {}
```

字典记录已经访问过的数字及其下标：

```python
{
    数字: 下标
}
```

例如：

```python
{
    2: 0,
    7: 1,
}
```

查询：

```python
if needed in seen:
```

平均时间复杂度为 $O(1)$。

返回：

```python
return seen[needed], index
```

等价于：

```python
return (seen[needed], index)
```

Python 会自动将多个返回值包装成元组。

> [!summary]- 当前实例复杂度
> 
> - 时间复杂度：$O(n)$，每个元素只被遍历一次，字典查询平均为 $O(1)$。
>     
> - 额外空间复杂度：$O(n)$，字典最多保存全部元素。
>     
> - 返回结果空间：$O(1)$，只返回两个下标。
>     

---

## 6. 与相近写法怎么选

|需求|推荐写法|原因|
|---|---|---|
|保存有顺序且需要修改的数据|`list`|支持添加、删除和修改|
|保存固定顺序的数据|`tuple`|不可变，结构语义更明确|
|根据键快速查询值|`dict`|平均查询复杂度为 $O(1)$|
|保存坐标|`tuple`|坐标通常具有固定结构|
|保存动态任务列表|`list`|任务可能增加或删除|
|统计元素出现次数|`dict`|可以建立元素到次数的映射|
|保存对象属性|`dict`|字段名比数字下标更清晰|
|函数返回多个固定结果|`tuple`|Python 多返回值本质上通常是元组|
|需要作为字典键|`tuple`|元组在元素均可哈希时可以作为键|
|需要频繁判断元素是否存在|`dict`|比列表的线性查找更高效|

### List 和 Tuple 怎么选

两者都具有顺序，并且都支持下标访问：

```python
values_list = [10, 20]
values_tuple = (10, 20)

print(values_list[0])
print(values_tuple[0])
```

核心区别是是否允许修改：

```python
values_list[0] = 100
```

允许。

```python
values_tuple[0] = 100
```

不允许。

当数据需要变化时使用列表，当数据具有固定含义时使用元组。

### List 和 Dict 怎么选

列表通过下标访问：

```python
student = ["Leyan", 23, 90]

print(student[0])
```

字典通过字段名访问：

```python
student = {
    "name": "Leyan",
    "age": 23,
    "score": 90,
}

print(student["name"])
```

字典通常更容易理解，因为字段含义直接写在键中。

### Tuple 和 Dict 怎么选

固定且较短的结构可以使用元组：

```python
point = (10, 20)
```

字段较多或字段含义不明显时，使用字典：

```python
point = {
    "x": 10,
    "y": 20,
}
```

---

## 7. 常见易错点

> [!warning]- 易错点
> 
> - 单元素元组遗漏逗号：
>     
> 
> ```python
> value = (1)     # int
> value = (1,)    # tuple
> ```
> 
> - 使用 `{}` 创建空集合：
>     
> 
> ```python
> value = {}      # dict
> value = set()   # set
> ```
> 
> - 直接访问不存在的字典键：
>     
> 
> ```python
> data = {}
> 
> print(data["count"])          # KeyError
> print(data.get("count", 0))   # 0
> ```
> 
> - 使用列表作为字典键：
>     
> 
> ```python
> data[[1, 2]] = "value"
> ```
> 
> 列表可变，因此不可哈希。可以改成元组：
> 
> ```python
> data[(1, 2)] = "value"
> ```
> 
> - 误以为元组内部对象全部不能变化：
>     
> 
> ```python
> data = ([1, 2], 3)
> data[0].append(4)
> ```
> 
> 元组不能替换元素，但其中的列表仍然可以被修改。
> 
> - 直接复制列表导致对象共享：
>     
> 
> ```python
> a = [1, 2, 3]
> b = a
> 
> b.append(4)
> 
> print(a)
> ```
> 
> 结果为：
> 
> ```text
> [1, 2, 3, 4]
> ```
> 
> `a` 和 `b` 指向同一个列表。需要浅拷贝时可以写：
> 
> ```python
> b = a.copy()
> ```
> 
> - 遍历字典时误以为得到的是值：
>     
> 
> ```python
> for item in data:
>     print(item)
> ```
> 
> 默认遍历的是键。遍历键和值应使用：
> 
> ```python
> for key, value in data.items():
>     print(key, value)
> ```

---

## 复杂度

> [!summary]- 复杂度
> 
> - `list` 下标访问：$O(1)$。
>     
> - `list` 末尾添加：均摊 $O(1)$。
>     
> - `list` 中间插入或删除：$O(n)$，因为后续元素需要移动。
>     
> - `list` 查找元素：$O(n)$，最坏需要遍历整个列表。
>     
> - `tuple` 下标访问：$O(1)$。
>     
> - `tuple` 查找元素：$O(n)$。
>     
> - `dict` 查询、插入和删除：平均 $O(1)$，最坏情况下可能退化为 $O(n)$。
>     
> - 遍历任意容器：$O(n)$。
>     
> - `list` 和 `tuple` 的空间复杂度通常为 $O(n)$。
>     
> - `dict` 的空间复杂度通常为 $O(n)$，但哈希表会保留额外空间。
>     

---

## 一句话总结

需要有序并可修改的数据时使用 `list`，需要固定结构时使用 `tuple`，需要通过键快速查询和建立映射关系时使用 `dict`。

---

## 模板归纳

看到以下信号时，优先想到对应容器：

- 出现“按顺序保存、追加、删除、修改”，优先想到 `list`
    
- 出现“坐标、固定状态、多个返回值”，优先想到 `tuple`
    
- 出现“计数、映射、缓存、快速查找”，优先想到 `dict`
    

```python
def solve(data):
    result = []
    mapping = {}

    for index, item in enumerate(data):
        if item in mapping:
            result.append((mapping[item], index))
        else:
            mapping[item] = index

    return result
```