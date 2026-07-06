# Python 赋值语义与链表反转

Python 中 = 不是复制对象，而是让变量名指向对象；链表反转的核心是修改节点对象的 `next` 指针。

---

## 思路

链表题容易迷惑，是因为变量和节点对象不是一回事。

`p0 = dummy` 只是让 `p0` 和 `dummy` 指向同一个节点；`p0 = p0.next` 是移动变量；`p0.next = xxx` 才是修改链表结构。

理解这个之后，`dummy.next` 为什么能作为返回值就清楚了。

---

> [!tip]- 核心思路
> 
> ## 1. Python 里的 `=` 不是复制对象
> 
> ```python
> p0 = dummy
> ```
> 
> 含义不是复制一个新节点，而是：
> 
> ```text
> p0 和 dummy 指向同一个 ListNode 对象
> ```
> 
> 图示：
> 
> ```text
> dummy ─┐
>        ↓
>      [0] -> [1] -> [2] -> [3]
>        ↑
> p0 ────┘
> ```
> 
> 所以如果执行：
> 
> ```python
> p0.next = xxx
> ```
> 
> 修改的是 `[0]` 这个节点对象的 `next`。
> 
> 因为 `dummy` 也指向 `[0]`，所以 `dummy.next` 也会看到变化。
> 
> ---
> 
> ## 2. 变量移动和修改节点不是一回事
> 
> ```python
> p0 = p0.next
> ```
> 
> 这是让变量 `p0` 改指向下一个节点。
> 
> 它不会修改链表结构。
> 
> ```text
> dummy -> [0] -> [1] -> [2] -> [3]
>           p0
> 
> p0 = p0.next
> 
> dummy -> [0] -> [1] -> [2] -> [3]
>                  p0
> ```
> 
> ---
> 
> ```python
> p0.next = pre
> ```
> 
> 这是修改 `p0` 当前指向的节点对象的 `next`。
> 
> 它会改变链表结构。
> 
> ```text
> 修改前：
> dummy -> [0] -> [1] -> [2] -> [3]
>                  p0
> 
> 执行 p0.next = pre
> 
> 修改后：
> dummy -> [0] -> [1] -> pre
>                  p0
> ```
> 
> ---
> 
> ## 3. 为什么返回 `dummy.next`
> 
> `dummy` 是哨兵节点 Sentinel Node。
> 
> 它的作用是稳定保存整条链表的入口。
> 
> ```text
> dummy -> 真正链表头节点 -> ...
> ```
> 
> 反转过程中，`p0` 会移动到中间施工，但 `dummy` 一直留在最前面。
> 
> 所以最后：
> 
> ```python
> return dummy.next
> ```
> 
> 返回的是最终真实链表的头节点。
> 
> 不能返回 `p0`，因为 `p0` 只是反转区间前一个节点，不一定是链表头。

---

> [!example]- 示例推演  
> 以 `head = [1,2,3,4,5]`，`left = 2`，`right = 4` 为例。
> 
> 初始：
> 
> ```text
> dummy -> 1 -> 2 -> 3 -> 4 -> 5
> ```
> 
> 找到 `p0`：
> 
> ```text
> dummy -> 1 -> 2 -> 3 -> 4 -> 5
>          p0
> ```
> 
> 反转 `2 -> 3 -> 4` 后：
> 
> ```text
> dummy -> 1      4 -> 3 -> 2      5
>          p0     pre        尾巴   cur
> ```
> 
> 接回链表：
> 
> ```python
> p0.next.next = cur
> p0.next = pre
> ```
> 
> 得到：
> 
> ```text
> dummy -> 1 -> 4 -> 3 -> 2 -> 5
> ```
> 
> 最后返回：
> 
> ```python
> return dummy.next
> ```
> 
> 也就是：
> 
> ```text
> 1 -> 4 -> 3 -> 2 -> 5
> ```

---

> [!success]- C++/Python 实现
> 
> ## Python：反转链表 II
> 
> ```python
> # Definition for singly-linked list.
> # class ListNode(object):
> #     def __init__(self, val=0, next=None):
> #         self.val = val
> #         self.next = next
> 
> class Solution(object):
>     def reverseBetween(self, head, left, right):
>         """
>         :type head: Optional[ListNode]
>         :type left: int
>         :type right: int
>         :rtype: Optional[ListNode]
>         """
>         dummy = ListNode(0, head)
>         p0 = dummy
> 
>         for _ in range(left - 1):
>             p0 = p0.next
> 
>         pre = None
>         cur = p0.next
> 
>         for _ in range(right - left + 1):
>             nxt = cur.next
>             cur.next = pre
>             pre = cur
>             cur = nxt
> 
>         p0.next.next = cur
>         p0.next = pre
> 
>         return dummy.next
> ```
> 
> 主流程：
> 
> - 用 `dummy` 保存链表入口，避免头节点变化带来的特殊判断
>     
> - 用 `p0` 找到反转区间前一个节点
>     
> - 用 `pre / cur / nxt` 反转 `[left, right]` 区间
>     
> - 反转后把中间段重新接回原链表
>     
> 
> 关键变量：
> 
> - `dummy`：哨兵节点，始终在真实链表头节点前面
>     
> - `p0`：反转区间前一个节点
>     
> - `pre`：反转后区间的新头
>     
> - `cur`：反转区间后面的第一个节点
>     
> - `p0.next`：反转前区间的第一个节点，反转后变成区间尾
>     
> 
> 最容易写错的位置：
> 
> ```python
> p0.next.next = cur
> p0.next = pre
> ```
> 
> 第一行：原来的区间头现在变成尾巴，要接到后半段。
> 
> 第二行：区间前一个节点要接到反转后的新头。

---

> [!warning]- 易错点
> 
> ## 1. 误以为 `p0 = dummy` 是复制节点
> 
> 错误理解：
> 
> ```text
> p0 是 dummy 的副本
> ```
> 
> 正确理解：
> 
> ```text
> p0 和 dummy 指向同一个节点对象
> ```
> 
> Python 中普通赋值通常不会复制对象。
> 
> ---
> 
> ## 2. 分不清 `p0 = ...` 和 `p0.next = ...`
> 
> ```python
> p0 = p0.next
> ```
> 
> 是变量换指向，不改链表。
> 
> ```python
> p0.next = pre
> ```
> 
> 是修改节点对象的 `next`，会改链表结构。
> 
> ---
> 
> ## 3. 误以为应该返回 `p0`
> 
> `p0` 是施工指针，只表示反转区间前一个节点。
> 
> 如果 `left = 3`：
> 
> ```text
> 1 -> 2 -> 3 -> 4 -> 5
> ```
> 
> 此时 `p0` 是节点 `2`。
> 
> 如果返回 `p0`，会得到：
> 
> ```text
> 2 -> 4 -> 3 -> 5
> ```
> 
> 节点 `1` 丢失。
> 
> 所以必须返回：
> 
> ```python
> return dummy.next
> ```
> 
> ---
> 
> ## 4. 反转区间长度写反
> 
> 正确：
> 
> ```python
> for _ in range(right - left + 1):
> ```
> 
> 错误：
> 
> ```python
> for _ in range(left - right + 1):
> ```
> 
> 因为 `[left, right]` 的节点数量是：
> 
> $$  
> right - left + 1  
> $$
> 
> ---
> 
> ## 5. 接回顺序不能反
> 
> 正确：
> 
> ```python
> p0.next.next = cur
> p0.next = pre
> ```
> 
> 如果先写：
> 
> ```python
> p0.next = pre
> ```
> 
> 那么 `p0.next` 就不再指向原来的区间头，后面会找不到区间尾。

---

> [!summary]- 复杂度  
> 时间复杂度：$O(n)$
> 
> 空间复杂度：$O(1)$
> 
> 为什么：
> 
> - 最多遍历链表一次
>     
> - 反转时只修改节点的 `next` 指针
>     
> - 只使用 `dummy / p0 / pre / cur / nxt` 这些常数级变量
>     

---

## 一句话总结

Python 里的 =是让变量指向对象；链表反转真正改变结构的是 `.next = ...`，而 `dummy.next` 始终是最终真实链表的入口。

---

