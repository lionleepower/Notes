#python/list #python/syntax #status/review

> [!info] 知识图谱
> 总览：[[0.0 Patterns 算法知识图谱]] · 相关应用：[[5.2 前中后序遍历]]

# Python list 的 append 与 +=：加「一个元素」还是加「一串元素」

`append` 把右边的对象**整个**当作一个元素塞进去；`+=` 把右边的对象**拆开**，逐个塞进去。选错会得到嵌套 list、被拆碎的字符串，或者一个 `TypeError`。

---

## 思路

算法题里 `list` 几乎只做两件事：**收集答案**和**维护路径**。这两件事都在往 list 里加东西，但「加什么」不一样：

- 收集单个值（`ans.append(root.val)`）→ 加的是**一个元素**
- 合并子递归返回的 list（`ans += self.dfs(root.left)`）→ 加的是**一串元素**

如果不区分，`ans.append(self.dfs(root.left))` 会得到 `[[1, 2], 5, [3]]` 这种嵌套结构，答案格式直接错；反过来 `ans += root.val` 会 `TypeError`，因为 `int` 不可迭代。

更深一层：`+=` 是**原地修改**，`p = p + x` 是**新建对象**。这个差别决定了别名（aliasing）问题，也决定了回溯题要不要手动撤销。

---

> [!tip]- 核心思路
> 右边是**一个值** → `append`；右边是**一个可迭代对象**且你想摊平 → `+=`。
>
> ```python
> p = []
> p.append([1, 2])   # [[1, 2]]   整个塞进去，长度 +1
>
> p = []
> p += [1, 2]        # [1, 2]     拆开塞进去，长度 +2
> ```
>
> 判断依据：**`len(p)` 增加 1 就是 `append`，增加 `len(x)` 就是 `+=`。**

---

## 1. 导入与创建

`list` 是内置类型，不需要导入。

> [!success]- 基础语法
> ```python
> empty = []
> empty = list()
>
> initialized = [1, 2, 3]
> matrix = [[0] * n for _ in range(m)]   # 二维，不要写 [[0] * n] * m
> ```
>
> `[[0] * n] * m` 是错的：外层乘法只是把**同一个内层 list 的引用**复制 m 份，改一行会改全部。

---

## 2. 核心操作

| 操作 | 作用 | 复杂度与限制 |
| --- | --- | --- |
| `p.append(x)` | 把 `x` 作为**一个元素**追加 | $O(1)$ 均摊；`x` 可以是任何对象 |
| `p += x` | 把 `x` 中每个元素追加（等价 `extend`） | $O(k)$；`x` **必须可迭代** |
| `p.extend(x)` | 同 `+=` | $O(k)$ |
| `p = p + x` | **新建** list，`p` 指向新对象 | $O(n + k)$；`x` 必须也是 list |
| `p[:]` / `list(p)` | 浅拷贝，得到新对象 | $O(n)$ |

> [!example]- 最小示例
> ```python
> p = []
> p.append(5)        # [5]
> p += [6, 7]        # [5, 6, 7]
> p.append([8, 9])   # [5, 6, 7, [8, 9]]
> p += "ab"          # [5, 6, 7, [8, 9], 'a', 'b']   字符串被拆成字符
> p += 10            # TypeError: 'int' object is not iterable
> ```

---

## 3. 关键行为与边界

`+=` 和 = + 看起来只差一个符号，但一个原地改、一个新建对象，这是所有别名 bug 的源头。

> [!tip]- 为什么这样做
> `p += x` 触发 `list.__iadd__`，它内部就是 `extend`：**在原对象上追加**，`p` 的身份（`id`）不变。
>
> `p = p + x` 触发 `list.__add__`：**造一个新 list**，再让变量名 `p` 指向它，旧对象原封不动。
>
> ```python
> a = []
> b = a
> a += [1]        # a is b → True，b 也变成 [1]
>
> a = []
> b = a
> a = a + [1]     # a is b → False，a = [1] 而 b 仍是 []
> ```
>
> 不变量：**只有 `append` / `+=` / `extend` 会让「所有指向这个 list 的变量」一起看到变化。**

> [!warning]- 边界条件
> - `p += x` 中 `x` 必须可迭代。`int` 会 `TypeError`；`str` 会被**逐字符拆开**（最隐蔽的坑）。
> - `p = p + x` 要求两边都是 list，`p + "ab"` 直接 `TypeError`（比 `+=` 反而更安全）。
> - `p += []` 合法，什么也不加；`p.append([])` 会加进一个空 list，`len(p)` 变成 1。
> - 函数参数默认值不要写 `def f(p=[])`：默认值只创建一次，`append` 会跨调用累积。

---

## 4. 实例一：递归收集答案（二叉树中序遍历）

输入是一棵树，目标是返回**一维**的值列表。左右子树的递归返回的是 list，根节点的值是单个 int——两者必须用不同的写法加进答案。

> [!success]- 完整实现
> ```python
> class Solution:
>     def inorderTraversal(self, root):
>         if not root:
>             return []
>
>         ans = []
>         ans += self.inorderTraversal(root.left)   # 左子树返回 list → 摊平合并
>         ans.append(root.val)                      # 根是单个 int → 追加一个元素
>         ans += self.inorderTraversal(root.right)
>         return ans
> ```

主流程：递归返回的永远是「这棵子树的中序结果」，父节点把左结果、自己、右结果按顺序拼起来。

关键变量：`ans` 是当前子树的答案容器，每层递归各有一个。

最易写错处：
- `ans.append(self.inorderTraversal(root.left))` → 得到 `[[...], 5, [...]]`，嵌套了。
- `ans += root.val` → `TypeError`，`int` 不可迭代。想用 `+=` 就得写 `ans += [root.val]`。

---

## 5. 实例二：回溯路径（原地 append + pop）

回溯要在**一条 path 上反复进出**，这时必须用原地修改，并且靠 `pop` 撤销。

> [!success]- 完整实现
> ```python
> class Solution:
>     def subsets(self, nums):
>         res = []
>         path = []
>
>         def dfs(i):
>             if i == len(nums):
>                 res.append(path[:])      # 存快照，必须拷贝
>                 return
>
>             dfs(i + 1)                   # 不选 nums[i]
>
>             path.append(nums[i])         # 选 nums[i]
>             dfs(i + 1)
>             path.pop()                   # 恢复现场
>
>         dfs(0)
>         return res
> ```

主流程：`path` 全程只有一个对象，`append` 进入分支、`pop` 退出分支。

最易写错处：`res.append(path)` 存的是**引用**，`path` 后续被 `pop` 改空，`res` 里所有元素会一起变成 `[]`。必须 `path[:]` 或 `list(path)` 存副本。

> [!summary]- 当前实例复杂度
> - 时间：$O(n \cdot 2^n)$，共 $2^n$ 个子集，每次拷贝 path 是 $O(n)$。
> - 空间：递归栈 $O(n)$，`path` $O(n)$；返回结果 $O(n \cdot 2^n)$ 不计入额外空间。

---

## 6. 与相近写法怎么选

| 需求 | 推荐写法 | 原因 |
| --- | --- | --- |
| 加一个值 | `p.append(x)` | $O(1)$，语义最直白 |
| 合并另一个 list | `p += x` / `p.extend(x)` | 原地，$O(k)$，不产生新对象 |
| 需要保留原 list 不变 | `q = p + x` | 新建对象，`p` 不受影响 |
| 回溯中共享一条路径 | `append` + `pop` | 原地，配合撤销，空间 $O(n)$ |
| 回溯中不想手动撤销 | `dfs(path + [x])` | 每层拿到独立副本，但每次 $O(n)$ 拷贝 |
| 把快照存进结果集 | `res.append(path[:])` | 切片拷贝，切断引用 |

---

## 7. 常见易错点

> [!warning]- 易错点
> **1. 用 `append` 合并 list，得到嵌套结构**
> ```python
> ans.append(dfs(left))   # ✗ [[1, 2], ...]
> ans += dfs(left)        # ✓ [1, 2, ...]
> ```
>
> **2. 用 `+=` 加单个值**
> ```python
> ans += root.val         # ✗ TypeError，int 不可迭代
> ans += [root.val]       # ✓
> ans.append(root.val)    # ✓ 更推荐
> ```
>
> **3. `+=` 一个字符串，被拆成字符**
> ```python
> words += "abc"          # ✗ ['a', 'b', 'c']
> words.append("abc")     # ✓ ['abc']
> ```
>
> **4. 结果集里存了引用而不是快照**
> ```python
> res.append(path)        # ✗ path 之后被改，res 全跟着变
> res.append(path[:])     # ✓
> ```
>
> **5. 混淆 += 和 = + 导致别名 bug**
> `b = a` 之后，`a += [1]` 会连带改动 `b`；`a = a + [1]` 不会。
>
> **6. `append` / `+=` / `pop` 的返回值是 `None`**
> ```python
> p = p.append(1)         # ✗ p 变成 None
> p.append(1)             # ✓ 原地修改，不要接收返回值
> ```
> 同理 `p = p.sort()` 也是错的，应该 `p.sort()` 或 `q = sorted(p)`。

---

## 复杂度

> [!summary]- 复杂度
> - `append`：均摊 $O(1)$。CPython 的 list 是动态数组，容量不足时按比例扩容，摊到每次操作是常数。
> - `+=` / `extend`：$O(k)$，$k$ 是右边的元素个数，只拷贝 $k$ 个引用。
> - `p = p + x`：$O(n + k)$，要把左右两边全部拷进新对象。**在循环里这样写会退化成 $O(n^2)$。**
> - `path[:]`：$O(n)$ 浅拷贝，只复制引用，不递归复制元素。
> - 额外空间：`append` / `+=` 不产生新 list；`+` 和切片都会新建一个 $O(n)$ 的对象。

---

## 一句话总结

`append` 让 `len` 加 1、`+=` 让 `len` 加 `len(x)`，两者都是**原地修改**（会被所有别名看到），而 `p = p + x` 是**新建对象**——决定用哪个，只需要问「右边是一个值，还是一串值？」以及「我要不要影响原对象？」

---

## 模板归纳

看到以下信号时，回到这条判断链：

- 递归函数返回的是 list，要往上层合并 → `+=`
- 要加的是单个 `int` / `str` / `TreeNode` → `append`
- 结果里出现了意料之外的嵌套 `[[...]]` → 用错成 `append` 了
- 结果里所有元素长得一样或全是空 → 存了引用没拷贝，改成 `path[:]`
- 报 `TypeError: 'int' object is not iterable` → 用错成 `+=` 了
- 改了 `a` 结果 `b` 也变了 → `+=` 的原地语义 + 别名

```python
# 最小模板：递归收集答案
def collect(node):
    if not node:
        return []

    ans = []
    ans += collect(node.left)    # 一串 → +=
    ans.append(node.val)         # 一个 → append
    ans += collect(node.right)
    return ans


# 最小模板：回溯维护路径
def backtrack(i, path, res):
    if done(i):
        res.append(path[:])      # 快照必须拷贝
        return

    path.append(choice)          # 进入分支
    backtrack(i + 1, path, res)
    path.pop()                   # 恢复现场
```