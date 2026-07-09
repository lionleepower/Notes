#python/scope #python/oop #binary-tree #status/done

# 递归调用要不要写 self.

一句话：**局部函数**递归直接用函数名，**类方法**递归必须写 `self.方法名`。

## 思路
两段代码都在"调用自己"，区别只在于——被调用的名字**是不是挂在实例上**。挂在实例上的（方法）走不了普通变量查找，必须显式 `self.`。

> [!tip]- 核心思路：LEGB 管不到 self
> Python 查名字遵循 **LEGB**：Local → Enclosing → Global → Built-in。
> - 局部函数 `f`（`def` 写在方法体内）是**局部/闭包名字**，在 Enclosing 层能查到 → 直接写 `f(...)`。
> - 类方法 `isSameTree`（`def` 写在 class 体内）**不参与 LEGB**，它是 `Solution` 类的属性，只能通过实例 `self.` 或类 `Solution.` 访问 → 必须写 `self.isSameTree(...)`。
> 
> 记住一句：**方法名不是全局函数名**，直接裸写会 `NameError`。

> [!example]- 两种情况对照
> ```python
> class Solution:
>     def rightSideView(self, root):
>         ans = []
>         def f(node, depth):        # 局部函数 → 局部名字
>             if node is None: return
>             if depth == len(ans): ans.append(node.val)
>             f(node.right, depth+1) # ✅ 直接用 f，Enclosing 查得到
>             f(node.left,  depth+1)
>         f(root, 0)                 # ✅ 同作用域直接调
>
>     def isSameTree(self, p, q):    # 类方法 → 挂在实例上
>         if p is None or q is None: return p is q
>         return (p.val == q.val
>                 and self.isSameTree(p.left,  q.right)   # ✅ 必须 self.
>                 and self.isSameTree(p.right, q.left))
>         #      直接写 isSameTree(...) → ❌ NameError
> ```

> [!warning]- 易错点
> - **裸写方法名递归** → `NameError: name 'isSameTree' is not defined`。方法名不在 Global 里。
> - **给局部函数硬加 self.** → 局部函数没挂在实例上，`self.f` 反而报 `AttributeError`。
> - **判断依据搞反**：不是看"要不要递归"，而是看 `def` 写在**哪一层**——class 体内=方法(要 self)，函数体内=局部(不要 self)。

一句话总结：`self.` 是"到实例上找方法"的开关；局部函数不在实例上，按普通变量查找即可，所以不写也不能写。

## 关联
`[[二叉树的右视图]]` `[[相同的树]]` `[[对称二叉树]]`（均为二叉树递归；本条是它们共用的语言底座）

## 模板归纳
**看到递归调用，先问：这个被调用的名字定义在哪一层？**
- 写在 `class` 里（第一个参数是 `self`）→ 递归写 `self.xxx()`
- 写在某个函数/方法**内部**（闭包）→ 递归直接写 `xxx()`
- 写在模块顶层（普通全局函数）→ 也直接写 `xxx()`（在 Global 层查得到）