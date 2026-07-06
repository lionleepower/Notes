# Python = 与链表指针理解

## 核心结论

> [!tip]- 一句话理解  
> Python 里的 `=` 不是“复制对象”，而是“让变量名绑定到某个对象”。
> 
> ```text
> 变量名 = 对象
> ```
> 
> 本质是：
> 
> ```text
> 让这个变量名指向这个对象
> ```
> 
> 所以：
> 
> ```python
> p0 = dummy
> ```
> 
> 不是复制一个新节点，而是让 `p0` 和 `dummy` 指向同一个 `ListNode` 对象。

---

## 1. Python 里的 = 到底是什么？

> [!tip]- 核心思路  
> 在 Python 中，变量更像是“名字”，对象才是真正存储数据的东西。
> 
> ```python
> a = [1, 2, 3]
> b = a
> ```
> 
> 这不是复制一个新列表，而是：
> 
> ```text
> a ─┐
>    ↓
>   [1, 2, 3]
>    ↑
> b ─┘
> ```
> 
> `a` 和 `b` 指向同一个 list 对象。
> 
> 所以：
> 
> ```python
> b.append(4)
> print(a)
> ```
> 
> 输出：
> 
> ```python
> [1, 2, 3, 4]
> ```
> 
> 因为你通过 `b` 修改的是同一个列表对象，`a` 当然也能看到变化。

---

## 2. 链表节点也是对象

> [!tip]- 核心思路  
> 链表节点 `ListNode` 本质上也是 Python 对象。
> 
> 所以：
> 
> ```python
> dummy = ListNode(0, head)
> p0 = dummy
> ```
> 
> 表示：
> 
> ```text
> dummy ─┐
>        ↓
>       [0] -> [1] -> [2] -> [3]
>        ↑
> p0 ────┘
> ```
> 
> `dummy` 和 `p0` 指向同一个哨兵节点。
> 
> 因此如果你写：
> 
> ```python
> p0.next = pre
> ```
> 
> 你是在修改这个哨兵节点的 `next`。
> 
> 因为 `dummy` 也指向这个节点，所以：
> 
> ```python
> dummy.next
> ```
> 
> 也会看到修改后的结果。

---

## 3. 为什么整数看起来不像这样？

> [!example]- 示例推演  
> 例如：
> 
> ```python
> a = 1
> b = a
> b = 2
> print(a)
> ```
> 
> 输出：
> 
> ```python
> 1
> ```
> 
> 这不是因为 `b = a` 复制了整数。
> 
> 真正过程是：
> 
> ```python
> a = 1
> b = a
> ```
> 
> 此时：
> 
> ```text
> a ─┐
>    ↓
>    1
>    ↑
> b ─┘
> ```
> 
> 然后：
> 
> ```python
> b = 2
> ```
> 
> 不是把对象 `1` 改成 `2`，而是让 `b` 改指向另一个对象：
> 
> ```text
> a -> 1
> b -> 2
> ```
> 
> 所以 `a` 没变。

---

## 4. 最关键的区别：换指向 vs 改对象

> [!tip]- 核心思路  
> 面试和写链表题时，最重要的是分清这两种操作：
> 
> ```python
> b = something
> ```
> 
> 这是变量换指向。
> 
> ```python
> b.attr = something
> ```
> 
> 这是修改对象内部属性。
> 
> 对链表来说：
> 
> ```python
> p0 = p0.next
> ```
> 
> 只是让 `p0` 这个变量移动到下一个节点，不改变链表结构。
> 
> ```python
> p0.next = pre
> ```
> 
> 是修改 `p0` 当前指向节点的 `next`，链表结构真的变了。

---

## 5. 链表中的三个典型操作

> [!tip]- 核心思路  
> 链表题里最容易混淆的就是下面三个操作。

### 5.1 `p0 = dummy`

> [!example]- 示例推演
> 
> ```python
> dummy = ListNode(0, head)
> p0 = dummy
> ```
> 
> 表示：
> 
> ```text
> dummy ─┐
>        ↓
>       [0] -> [1] -> [2] -> [3]
>        ↑
> p0 ────┘
> ```
> 
> `p0` 和 `dummy` 指向同一个节点。
> 
> 它不是复制节点。

### 5.2 `p0 = p0.next`

> [!example]- 示例推演
> 
> ```python
> p0 = p0.next
> ```
> 
> 表示让 `p0` 走到下一个节点。
> 
> 原来：
> 
> ```text
> dummy -> [0] -> [1] -> [2] -> [3]
>           p0
> ```
> 
> 执行后：
> 
> ```text
> dummy -> [0] -> [1] -> [2] -> [3]
>                  p0
> ```
> 
> 链表结构没有变化。
> 
> 只是 `p0` 这个变量换了指向。

### 5.3 `p0.next = pre`

> [!example]- 示例推演
> 
> ```python
> p0.next = pre
> ```
> 
> 表示修改 `p0` 当前指向节点的 `next`。
> 
> 如果原来：
> 
> ```text
> dummy -> [0] -> [1] -> [2] -> [3]
>                  p0
> ```
> 
> `pre` 指向 `[4]`，执行：
> 
> ```python
> p0.next = pre
> ```
> 
> 之后变成：
> 
> ```text
> dummy -> [0] -> [1] -> [4] -> ...
>                  p0
> ```
> 
> 这一步会真正改变链表结构。

---

## 6. 什么时候是复制？

> [!tip]- 核心思路  
> 普通赋值一般不是复制。
> 
> ```python
> b = a
> ```
> 
> 通常只是让 `b` 指向 `a` 指向的对象。
> 
> 常见对象包括：
> 
> - list
>     
> - dict
>     
> - set
>     
> - class object
>     
> - ListNode
>     
> - TreeNode
>     
> 
> 例如：
> 
> ```python
> node2 = node1
> arr2 = arr1
> dict2 = dict1
> ```
> 
> 它们都不是复制对象，而是共享同一个对象。

> [!success]- 显式复制写法  
> 如果真的要复制 list：
> 
> ```python
> b = a.copy()
> ```
> 
> 或者：
> 
> ```python
> b = a[:]
> ```
> 
> 如果要深拷贝：
> 
> ```python
> import copy
> b = copy.deepcopy(a)
> ```
> 
> 如果要复制链表节点，必须自己新建节点：
> 
> ```python
> new_node = ListNode(old_node.val)
> ```
> 
> 下面这种不是复制节点：
> 
> ```python
> new_node = old_node
> ```

---

## 7. 为什么链表题最后经常返回 `dummy.next`？

> [!tip]- 核心思路  
> 因为 `dummy` 一直留在最前面的哨兵节点位置。
> 
> 即使中间用 `p0` 移动、改线、反转，`dummy` 这个变量本身没有移动。
> 
> 所以最终链表入口就是：
> 
> ```python
> dummy.next
> ```
> 
> 特别是当头节点可能发生变化时，必须用 `dummy` 保护入口。
> 
> 例如反转头部：
> 
> ```text
> 原链表：
> dummy -> [0] -> [1] -> [2] -> [3]
> 
> 反转后：
> dummy -> [0] -> [2] -> [1] -> [3]
> ```
> 
> 此时真正链表入口已经不是原来的 `head`，而是：
> 
> ```python
> dummy.next
> ```

---

## 8. 链表区间反转模板

> [!tip]- 什么时候想到这个模板？  
> 看到这些特征，可以想到：
> 
> ```text
> dummy + p0 + 局部反转
> ```
> 
> 适用场景：
> 
> - 反转链表某一段
>     
> - 头节点可能变化
>     
> - 需要把局部反转后的链表重新接回原链表
>     
> - 题目出现 `left`、`right` 区间
>     

> [!success]- Python 实现
> 
> ```python
> class Solution(object):
>     def reverseBetween(self, head, left, right):
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

> [!tip]- 整体逻辑  
> 这段代码分成四步：
> 
> 第一步：建立哨兵节点。
> 
> ```python
> dummy = ListNode(0, head)
> p0 = dummy
> ```
> 
> `dummy` 负责保住最终链表入口。
> 
> `p0` 用来找到反转区间前一个节点。
> 
> 第二步：找到反转区间前驱。
> 
> ```python
> for _ in range(left - 1):
>     p0 = p0.next
> ```
> 
> 循环结束后，`p0` 指向第 `left - 1` 个节点。
> 
> 第三步：反转 `[left, right]` 区间。
> 
> ```python
> pre = None
> cur = p0.next
> 
> for _ in range(right - left + 1):
>     nxt = cur.next
>     cur.next = pre
>     pre = cur
>     cur = nxt
> ```
> 
> 反转结束后：
> 
> - `pre` 指向反转后区间的新头
>     
> - `cur` 指向反转区间后面的第一个节点
>     
> - `p0.next` 仍然指向反转前区间的旧头
>     
> 
> 第四步：重新接回链表。
> 
> ```python
> p0.next.next = cur
> p0.next = pre
> ```
> 
> `p0.next.next = cur`：
> 
> 让反转后的尾巴接回后半段。
> 
> `p0.next = pre`：
> 
> 让前半段接上反转后的新头。

---

## 9. 区间反转为什么先写 `p0.next.next = cur`？

> [!warning]- 易错点  
> 反转结束后，`p0.next` 仍然指向旧头。
> 
> 例如反转 `[2, 4]`：
> 
> ```text
> 原链表：
> dummy -> [1] -> [2] -> [3] -> [4] -> [5]
>          p0     old_head
> ```
> 
> 反转中间部分后，局部变成：
> 
> ```text
> [4] -> [3] -> [2]
>  pre          old_head
> 
> cur -> [5]
> ```
> 
> 此时：
> 
> ```python
> p0.next
> ```
> 
> 还是旧头 `[2]`。
> 
> 所以：
> 
> ```python
> p0.next.next = cur
> ```
> 
> 等价于：
> 
> ```text
> [2].next = [5]
> ```
> 
> 先把反转后的尾巴接回后半段。
> 
> 然后：
> 
> ```python
> p0.next = pre
> ```
> 
> 等价于：
> 
> ```text
> [1].next = [4]
> ```
> 
> 再把前半段接到新头。

---

## 10. 常见易错点

> [!warning]- 易错点  
> 易错点 1：以为 `p0 = dummy` 复制了节点。
> 
> 实际上只是两个变量指向同一个节点。
> 
> ---
> 
> 易错点 2：分不清 `p0 = p0.next` 和 `p0.next = xxx`。
> 
> ```python
> p0 = p0.next
> ```
> 
> 只是指针变量移动。
> 
> ```python
> p0.next = xxx
> ```
> 
> 才是修改链表结构。
> 
> ---
> 
> 易错点 3：忘记用 `dummy`。
> 
> 如果反转区间从头节点开始，原来的 `head` 会变。
> 
> 不用 `dummy` 会让头节点处理变复杂。
> 
> ---
> 
> 易错点 4：反转后接线顺序混乱。
> 
> 反转结束后要记住：
> 
> ```text
> pre = 反转后新头
> cur = 反转区间后面的第一个节点
> p0.next = 反转前旧头，也就是反转后尾巴
> ```
> 
> 所以接线是：
> 
> ```python
> p0.next.next = cur
> p0.next = pre
> ```

---

## 11. 面试速记口诀

> [!summary]- 模板归纳  
> Python 赋值口诀：
> 
> ```text
> 变量 = 另一个变量：共享对象，不复制。
> 
> 变量 = 新值：变量换指向。
> 
> 变量.属性 = 新值：修改对象内部。
> ```
> 
> 链表对应口诀：
> 
> ```text
> p0 = dummy：共享同一个节点。
> 
> p0 = p0.next：p0 往后走，链表不变。
> 
> p0.next = pre：修改 next，链表改变。
> ```
> 
> 区间反转口诀：
> 
> ```text
> dummy 保入口；
> p0 找前驱；
> pre cur 做反转；
> 旧头接后面；
> 新头接前面；
> 返回 dummy.next。
> ```

---

## 12. 复杂度

> [!summary]- 复杂度  
> 对链表区间反转：
> 
> 时间复杂度：
> 
> $$  
> O(n)  
> $$
> 
> 因为最多遍历链表一次。
> 
> 空间复杂度：
> 
> $$  
> O(1)  
> $$
> 
> 因为只使用了 `dummy`、`p0`、`pre`、`cur`、`nxt` 等常数个指针变量。