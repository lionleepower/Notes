# Python 链表指针与区间反转

Python 链表题中的“指针”本质是保存节点对象引用的变量。移动变量不会改变链表，修改节点的 `next` 才会改变链表结构。

---

## 思路

链表区间反转最容易出错的地方不是反转本身，而是分不清变量和节点对象，以及反转后如何把三段链表重新接好。

> [!tip]- 核心思路
> 三种写法必须分清：
>
> ```python
> p0 = dummy      # p0 与 dummy 指向同一节点
> p0 = p0.next    # 移动变量，链表结构不变
> p0.next = pre   # 修改节点属性，链表结构改变
> ```
>
> 区间反转只需记住三个连接点：区间前驱、反转后的新头、反转后的新尾。

Python 变量绑定、浅拷贝和深拷贝的基础见：[[Python_赋值与拷贝]]。

---

## 1. 链表节点与变量

> [!success]- 节点定义
> ```python
> class ListNode:
>     def __init__(self, val=0, next=None):
>         self.val = val
>         self.next = next
> ```

执行下面的代码时，没有产生新的节点：

```python
dummy = ListNode(0, head)
p0 = dummy
```

```text
dummy ─┐
       ↓
      [0] → [1] → [2] → [3]
       ↑
p0 ────┘
```

`dummy` 与 `p0` 是两个变量，但它们保存的是同一个节点引用。

---

## 2. 移动变量与修改链表

| 写法 | 发生了什么 | 链表是否改变 |
| --- | --- | --- |
| `p0 = dummy` | 两个变量指向同一节点 | 否 |
| `p0 = p0.next` | `p0` 移动到下一节点 | 否 |
| `p0.next = node` | 修改当前节点的后继 | 是 |
| `node = ListNode(x)` | 创建新节点并绑定变量 | 原链表暂时不变 |

> [!example]- 移动变量
> ```text
> 执行前：dummy → [0] → [1] → [2]
>                 p0
>
> p0 = p0.next
>
> 执行后：dummy → [0] → [1] → [2]
>                        p0
> ```

> [!example]- 修改 `next`
> ```text
> p0 指向 [1]，pre 指向 [3]
>
> p0.next = pre
>
> 链表局部变为：[1] → [3]
> ```

---

## 3. 为什么使用哨兵节点 dummy

哨兵节点放在真实头节点前面，可以把“修改头节点”和“修改中间节点”统一成同一种接线操作。

> [!tip]- `dummy` 的作用
> ```python
> dummy = ListNode(0, head)
> ```
>
> 无论反转区间是否从第一个节点开始，最终真实头节点始终是：
>
> ```python
> dummy.next
> ```

没有 `dummy` 时，反转 `[1, right]` 会改变 `head`，需要额外判断；有了 `dummy`，区间前驱始终存在。

---

## 4. 完整反转一个链表

> [!success]- 三指针反转模板
> ```python
> def reverse_list(head):
>     pre = None
>     cur = head
>
>     while cur:
>         nxt = cur.next
>         cur.next = pre
>         pre = cur
>         cur = nxt
>
>     return pre
> ```

| 变量 | 职责 |
| --- | --- |
| `pre` | 已反转部分的头节点 |
| `cur` | 当前准备反转的节点 |
| `nxt` | 暂存未反转部分的入口，防止断链后丢失 |

> [!warning]- 为什么必须先保存 `nxt`
> 执行 `cur.next = pre` 后，原来的后继引用会被覆盖。如果没有提前保存 `cur.next`，就无法继续访问未处理部分。

---

## 5. 区间反转的四个步骤

目标：反转链表中第 `left` 到第 `right` 个节点，位置从 `1` 开始。

> [!tip]- 固定流程
> 1. 建立 `dummy`，用 `p0` 找到区间前驱。
> 2. 反转 `[left, right]`，一共处理 `right - left + 1` 个节点。
> 3. 让原区间头接到后半段。
> 4. 让区间前驱接到反转后的新头。

> [!example]- 反转 `[2, 4]`
> ```text
> 原链表：
> dummy → 1 → 2 → 3 → 4 → 5
>         p0   old_head
>
> 局部反转后：
> dummy → 1    4 → 3 → 2    5
>         p0   pre     尾   cur
>
> 接回后：
> dummy → 1 → 4 → 3 → 2 → 5
> ```

---

## 6. 区间反转代码

> [!success]- 完整实现
> ```python
> class ListNode:
>     def __init__(self, val=0, next=None):
>         self.val = val
>         self.next = next
>
>
> def reverse_between(head, left, right):
>     if head is None or left == right:
>         return head
>
>     dummy = ListNode(0, head)
>     p0 = dummy
>
>     for _ in range(left - 1):
>         p0 = p0.next
>
>     pre = None
>     cur = p0.next
>
>     for _ in range(right - left + 1):
>         nxt = cur.next
>         cur.next = pre
>         pre = cur
>         cur = nxt
>
>     p0.next.next = cur
>     p0.next = pre
>
>     return dummy.next
> ```

关键状态：

- `p0`：区间前驱，反转过程中保持不动。
- `pre`：反转完成后的区间新头。
- `cur`：反转区间后面的第一个节点。
- `p0.next`：反转前的区间头，反转后成为区间尾。

---

## 7. 为什么接线顺序不能反

正确顺序是：

```python
p0.next.next = cur
p0.next = pre
```

> [!tip]- 接线推导
> 反转结束后，`p0.next` 仍然指向原区间头，而原区间头现在是区间尾。
>
> ```python
> p0.next.next = cur
> ```
>
> 先让区间尾连接后半段。然后：
>
> ```python
> p0.next = pre
> ```
>
> 再让区间前驱连接区间新头。

如果先执行 `p0.next = pre`，`p0.next` 就不再指向原区间头，随后无法通过它找到区间尾。

---

## 8. 运行实例

> [!example]- 构造、反转并输出
> ```python
> def build_list(values):
>     dummy = ListNode()
>     tail = dummy
>     for value in values:
>         tail.next = ListNode(value)
>         tail = tail.next
>     return dummy.next
>
>
> def to_list(head):
>     values = []
>     while head:
>         values.append(head.val)
>         head = head.next
>     return values
>
>
> head = build_list([1, 2, 3, 4, 5])
> head = reverse_between(head, 2, 4)
> print(to_list(head))  # [1, 4, 3, 2, 5]
> ```

---

## 9. 常见易错点

> [!warning]- 易错点
> - 以为 `p0 = dummy` 复制了节点：它们只是共享同一节点对象。
> - 忘记保存 `nxt = cur.next`：修改 `cur.next` 后会丢失未反转部分。
> - 区间长度少写 `+ 1`：闭区间节点数是 `right - left + 1`。
> - 找错区间前驱：`p0` 应从 `dummy` 出发移动 `left - 1` 次。
> - 把两句接线顺序写反，导致找不到反转后的区间尾。
> - 返回原来的 `head`：当 `left == 1` 时头节点已经改变，应返回 `dummy.next`。

---

## 复杂度

> [!summary]- 复杂度
> - 时间复杂度：$O(n)$，寻找前驱和反转区间最多遍历链表一次。
> - 额外空间复杂度：$O(1)$，只使用常数个节点引用。

---

## 一句话总结

链表变量移动不会改结构，`.next = ...` 才会改结构；区间反转用 `dummy` 保入口、`p0` 找前驱、三指针反转，再按“旧头接后段、新头接前段”的顺序接回。

---

## 模板归纳

看到“头节点可能改变”或“反转链表某一区间”时：

```python
dummy = ListNode(0, head)
p0 = dummy

for _ in range(left - 1):
    p0 = p0.next

pre = None
cur = p0.next

for _ in range(right - left + 1):
    nxt = cur.next
    cur.next = pre
    pre = cur
    cur = nxt

p0.next.next = cur
p0.next = pre

return dummy.next
```

速记：`dummy` 保入口，`p0` 找前驱，`pre / cur / nxt` 做反转，旧头接后段，新头接前段。
