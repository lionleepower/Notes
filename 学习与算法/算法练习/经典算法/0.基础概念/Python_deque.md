# Python deque：双端队列语法与使用实例

`collections.deque` 是支持两端高效插入、删除的数据结构。算法题需要“从左端弹出”、维护队列或单调队列时，优先使用 `deque`，不要用 `list.pop(0)`。

---

## 思路

普通 `list` 适合随机访问和尾部操作，但从头部插入或删除会移动后面的所有元素，时间复杂度是 $O(n)$。

`deque` 是 `double-ended queue`（双端队列），左右两端的添加和删除平均都是 $O(1)$，因此适合 BFS、普通队列、固定长度窗口和单调队列。

> [!tip]- 核心思路
> 把 `deque` 想成一条两端都能开口的队伍：
>
> ```text
> 左端                                右端
>   ← popleft()  [1, 2, 3]  pop() →
>   → appendleft()          append() ←
> ```
>
> 最常见的普通队列写法是：
>
> ```python
> q.append(x)      # 右端入队
> x = q.popleft()  # 左端出队
> ```
>
> 这样满足先进先出（FIFO）。

---

## 1. 导入与创建

> [!success]- 基础语法
> ```python
> from collections import deque
>
> q = deque()                    # 空 deque
> q = deque([1, 2, 3])           # 从可迭代对象创建
> q = deque("abc")               # deque(['a', 'b', 'c'])
> q = deque([1, 2, 3], maxlen=3) # 固定最大长度
> ```
>
> `maxlen` 创建后不能修改。固定长度的 `deque` 已满时，再从一端加入元素，会自动丢弃另一端最旧的元素。

```python
from collections import deque

q = deque([1, 2, 3], maxlen=3)
q.append(4)

print(q)  # deque([2, 3, 4], maxlen=3)
```

---

## 2. 两端添加和删除

| 操作 | 作用 | 平均复杂度 |
| --- | --- | --- |
| `q.append(x)` | 右端添加 `x` | $O(1)$ |
| `q.appendleft(x)` | 左端添加 `x` | $O(1)$ |
| `q.pop()` | 删除并返回右端元素 | $O(1)$ |
| `q.popleft()` | 删除并返回左端元素 | $O(1)$ |

> [!example]- 示例推演
> ```python
> from collections import deque
>
> q = deque([2, 3])
> q.append(4)       # deque([2, 3, 4])
> q.appendleft(1)   # deque([1, 2, 3, 4])
>
> right = q.pop()   # right = 4，q = deque([1, 2, 3])
> left = q.popleft()# left = 1，q = deque([2, 3])
> ```

> [!warning]- 空队列不能直接弹出
> `pop()` 和 `popleft()` 在 `deque` 为空时都会抛出 `IndexError`。不确定是否为空时，先写：
>
> ```python
> if q:
>     x = q.popleft()
> ```

---

## 3. 批量添加

> [!success]- `extend` 与 `extendleft`
> ```python
> from collections import deque
>
> q = deque([2, 3])
> q.extend([4, 5])
> print(q)  # deque([2, 3, 4, 5])
>
> q.extendleft([0, 1])
> print(q)  # deque([1, 0, 2, 3, 4, 5])
> ```

`extend(iterable)` 按原顺序逐个添加到右端。

`extendleft(iterable)` 会把元素逐个添加到左端，所以最终顺序与传入顺序相反。若希望左侧最终仍是 `[0, 1]`，可以写：

```python
q.extendleft(reversed([0, 1]))
```

---

## 4. 查看、旋转、清空和转换

> [!success]- 常用操作
> ```python
> from collections import deque
>
> q = deque([1, 2, 3, 4])
>
> first = q[0]       # 1，查看左端
> last = q[-1]       # 4，查看右端
> size = len(q)      # 4
> exists = 3 in q    # True
>
> q.rotate(1)        # deque([4, 1, 2, 3])，向右旋转一步
> q.rotate(-2)       # deque([2, 3, 4, 1])，向左旋转两步
>
> items = list(q)    # 转成 list
> q.clear()          # 清空
> ```

`q[0]` 和 `q[-1]` 访问两端很快，但 `deque` 不适合频繁随机访问中间位置。需要按下标遍历、切片或二分查找时，通常应使用 `list`。

> [!warning]- `deque` 不支持切片
> 下面的写法会报错：
>
> ```python
> q[1:3]
> ```
>
> 确实需要切片时，先转换：
>
> ```python
> list(q)[1:3]
> ```

---

## 5. 实例一：把 deque 当普通队列

队列遵循先进先出（FIFO）：右端入队，左端出队。

> [!success]- 队列模板
> ```python
> from collections import deque
>
> q = deque()
>
> q.append("A")
> q.append("B")
> q.append("C")
>
> while q:
>     current = q.popleft()
>     print(current)
> ```
>
> 输出顺序是 `A`、`B`、`C`。

如果改成 `q.pop()`，就变成后进先出（LIFO），此时用法更像栈。

---

## 6. 实例二：BFS 最短步数

无权图中求最少经过多少条边，常用 BFS。`deque` 保存“已经发现、等待处理”的节点。

> [!success]- 无权图最短距离
> ```python
> from collections import deque
>
> def shortest_distance(graph, start, target):
>     q = deque([(start, 0)])
>     visited = {start}
>
>     while q:
>         node, distance = q.popleft()
>
>         if node == target:
>             return distance
>
>         for neighbor in graph[node]:
>             if neighbor not in visited:
>                 visited.add(neighbor)
>                 q.append((neighbor, distance + 1))
>
>     return -1
>
>
> graph = {
>     "A": ["B", "C"],
>     "B": ["A", "D"],
>     "C": ["A", "D"],
>     "D": ["B", "C", "E"],
>     "E": ["D"],
> }
>
> print(shortest_distance(graph, "A", "E"))  # 3
> ```

> [!tip]- 为什么入队时就加入 `visited`
> 节点第一次被发现时就是 BFS 能找到的最短路径。入队时立即标记，可以防止同一节点被多个相邻节点重复加入队列。

二叉树按层遍历的完整模板可参考：[[5.4 层序遍历|二叉树层序遍历（BFS）模板]]。

---

## 7. 实例三：固定长度的最近记录

设置 `maxlen` 后，`deque` 可以自动保留最近的 $k$ 条数据。

> [!success]- 保留最近 3 次成绩
> ```python
> from collections import deque
>
> recent_scores = deque(maxlen=3)
>
> for score in [70, 80, 90, 100]:
>     recent_scores.append(score)
>
> print(recent_scores)  # deque([80, 90, 100], maxlen=3)
> ```

这种写法适合最近访问记录、日志缓存、移动平均所需的固定窗口等场景。

---

## 8. 实例四：滑动窗口最大值（单调队列）

只用普通队列无法快速知道窗口最大值。可以让 `deque` 保存元素下标，并保证对应值从队首到队尾单调递减；这样队首永远是当前窗口最大值的下标。

> [!success]- 单调队列模板
> ```python
> from collections import deque
>
> def max_sliding_window(nums, k):
>     if not nums or k <= 0:
>         return []
>
>     q = deque()  # 保存下标，对应的 nums 值单调递减
>     ans = []
>
>     for right, value in enumerate(nums):
>         # 队首下标已经离开窗口 [right - k + 1, right]
>         while q and q[0] <= right - k:
>             q.popleft()
>
>         # 队尾较小的值不可能再成为后续窗口最大值
>         while q and nums[q[-1]] <= value:
>             q.pop()
>
>         q.append(right)
>
>         if right >= k - 1:
>             ans.append(nums[q[0]])
>
>     return ans
>
>
> nums = [1, 3, -1, -3, 5, 3, 6, 7]
> print(max_sliding_window(nums, 3))  # [3, 3, 5, 5, 6, 7]
> ```

> [!tip]- 为什么保存下标而不是只保存值
> 下标既能判断队首元素是否离开窗口，也能通过 `nums[index]` 比较大小。只保存值时，遇到重复元素会难以判断应该删除哪一个。

> [!summary]- 复杂度
> 每个下标最多入队一次、出队一次，所以总时间复杂度是 $O(n)$；队列最多保存 $k$ 个下标，空间复杂度是 $O(k)$。

---

## 9. deque 与 list 怎么选

| 需求 | 推荐结构 | 原因 |
| --- | --- | --- |
| 尾部添加、尾部删除 | `list` 或 `deque` | 两者平均都是 $O(1)$ |
| 左端添加、左端删除 | `deque` | `appendleft()`、`popleft()` 平均是 $O(1)$ |
| 按下标随机访问 | `list` | `list[index]` 是 $O(1)$ |
| 切片、排序 | `list` | `deque` 不支持切片，也没有 `sort()` |
| BFS、普通队列 | `deque` | 右进左出都是 $O(1)$ |
| 单调队列、两端维护窗口 | `deque` | 两端都需要高效删除 |

> [!warning]- 不要用 `list.pop(0)` 模拟队列
> ```python
> q = [1, 2, 3]
> q.pop(0)  # O(n)，后面的元素需要整体前移
> ```
>
> 推荐写法：
>
> ```python
> from collections import deque
>
> q = deque([1, 2, 3])
> q.popleft()  # O(1)
> ```

---

## 10. 常见易错点

> [!warning]- 易错点
> - 忘记导入：使用前需要 `from collections import deque`。
> - 把 `popleft` 写成 `pop(0)`：`deque.popleft()` 不接收下标参数。
> - 空队列直接弹出：`pop()` 和 `popleft()` 都会抛出 `IndexError`。
> - 混淆两端：普通 FIFO 队列通常是 `append()` 配合 `popleft()`。
> - 误解 `extendleft()`：它会反转传入元素的最终顺序。
> - 把 `deque` 当数组使用：中间位置访问较慢，也不支持切片。
> - 单调队列只存值：通常应存下标，才能同时判断过期和比较大小。

---

## 一句话总结

需要高效操作序列两端时使用 `deque`；算法题里最常见的固定组合是 `append()` 右端入队、`popleft()` 左端出队。

---

## 模板归纳

看到以下信号，优先想到 `deque`：

- BFS、层序遍历、无权图最短步数
- 先进先出的任务处理
- 需要频繁从序列左端删除
- 只保留最近 $k$ 条记录
- 滑动窗口中需要维护最大值或最小值
- 元素需要从左右两端进入或退出

最常用模板：

```python
from collections import deque

q = deque()

# 普通队列：FIFO
q.append(item)
item = q.popleft()

# 查看两端但不删除
left = q[0]
right = q[-1]

# 安全处理
while q:
    item = q.popleft()
```
