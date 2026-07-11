# Python self 与递归调用

局部函数递归直接使用函数名，实例方法递归通常使用 `self.方法名()`。判断依据不是“是否递归”，而是被调用对象定义在哪里、需要通过哪个命名空间访问。

---

## 思路

Python 调用函数时必须先找到对应的名字。局部函数名存在于当前或外层函数作用域中，可以直接调用；实例方法是类的属性，通常要先通过实例 `self` 找到它。

> [!tip]- 核心思路
> ```python
> def outer():
>     def dfs(node):
>         dfs(node.next)       # 局部函数：直接用名字
>
> class Solution:
>     def dfs(self, node):
>         self.dfs(node.next)  # 实例方法：通过 self 查找
> ```
>
> 先问“这个 `def` 写在哪一层”，再决定调用形式。

---

## 1. self 是什么

`self` 是当前实例对象。调用实例方法时，Python 会把点号左侧的实例自动传给方法的第一个参数。

> [!example]- 两种调用方式等价
> ```python
> class Counter:
>     def add(self, value):
>         return value + 1
>
>
> counter = Counter()
>
> print(counter.add(3))        # 4
> print(Counter.add(counter, 3))  # 4
> ```

`self` 不是 Python 关键字，但它是约定俗成的名称，不应随意替换。通过 `self.xxx` 可以访问当前实例的属性和方法。

---

## 2. 局部函数为什么不写 self

局部函数定义在另一个函数或方法内部，它的名字存在于局部作用域或闭包作用域中。

> [!success]- 局部递归函数
> ```python
> def factorial(n):
>     def dfs(x):
>         if x <= 1:
>             return 1
>         return x * dfs(x - 1)
>
>     return dfs(n)
>
>
> print(factorial(5))  # 120
> ```

这里的 `dfs` 不是对象属性，因此不能写成 `self.dfs()`。

---

## 3. 实例方法为什么写 self

定义在 `class` 代码块中的函数会成为类属性。通过实例访问时，它会绑定为实例方法。

> [!success]- 实例方法递归
> ```python
> class Solution:
>     def factorial(self, n):
>         if n <= 1:
>             return 1
>         return n * self.factorial(n - 1)
>
>
> solution = Solution()
> print(solution.factorial(5))  # 120
> ```

在方法体中直接写 `factorial(n - 1)`，Python 会按普通名字查找规则寻找局部或全局变量，但不会自动理解为“当前实例上的方法”，通常会触发 `NameError`。

---

## 4. LEGB 名字查找规则

Python 解析普通名字时，按照 LEGB 顺序查找：

| 层级 | 含义 | 示例 |
| --- | --- | --- |
| Local | 当前函数局部作用域 | 参数、局部变量、局部函数 |
| Enclosing | 外层函数作用域 | 闭包捕获的变量和函数 |
| Global | 当前模块全局作用域 | 模块顶层变量和函数 |
| Built-in | Python 内置作用域 | `len`、`sum`、`print` |

> [!tip]- LEGB 与属性查找不是一回事
> 裸名字 `dfs` 使用 LEGB；`self.dfs` 先通过 LEGB 找到 `self`，再在实例及其类上做属性查找。

类代码块会建立类命名空间，但方法运行时，类属性不会自动成为方法中的局部名字。因此实例方法通常需要 `self.method()` 或明确写 `ClassName.method(self, ...)`。

---

## 5. 三种定义位置对照

> [!success]- 全局函数、局部函数、实例方法
> ```python
> def global_countdown(n):
>     if n == 0:
>         return
>     global_countdown(n - 1)
>
>
> class Solution:
>     def use_local_function(self, n):
>         def local_countdown(x):
>             if x == 0:
>                 return
>             local_countdown(x - 1)
>
>         local_countdown(n)
>
>     def method_countdown(self, n):
>         if n == 0:
>             return
>         self.method_countdown(n - 1)
> ```

| 定义位置 | 递归调用方式 | 原因 |
| --- | --- | --- |
| 模块顶层 | `function()` | 名字在 Global 中 |
| 函数或方法内部 | `function()` | 名字在 Local 或 Enclosing 中 |
| `class` 内的实例方法 | `self.method()` | 方法是实例或类上的属性 |

---

## 6. 二叉树递归实例

下面两种写法都正确，区别是辅助函数放在哪里。

> [!example]- 局部函数写法
> ```python
> class Solution:
>     def max_depth(self, root):
>         def dfs(node):
>             if node is None:
>                 return 0
>             return 1 + max(dfs(node.left), dfs(node.right))
>
>         return dfs(root)
> ```

> [!example]- 实例方法写法
> ```python
> class Solution:
>     def max_depth(self, root):
>         if root is None:
>             return 0
>         return 1 + max(
>             self.max_depth(root.left),
>             self.max_depth(root.right),
>         )
> ```

局部函数便于捕获外层变量、隐藏辅助实现；实例方法便于被其他方法复用或单独测试。二叉树递归的状态设计可参考：[[5.1 最大深度、最小深度与路径总和]]。

---

## 7. classmethod 与 staticmethod

| 方法类型 | 第一个参数 | 常见调用方式 | 适用场景 |
| --- | --- | --- | --- |
| 实例方法 | `self` | `self.method()` | 需要实例状态 |
| 类方法 | `cls` | `cls.method()` | 需要类状态或构造替代方案 |
| 静态方法 | 无自动参数 | `ClassName.method()` | 逻辑属于该类，但不需要实例或类状态 |

> [!success]- 基础示例
> ```python
> class NumberTools:
>     base = 10
>
>     @classmethod
>     def add_base(cls, value):
>         return value + cls.base
>
>     @staticmethod
>     def is_even(value):
>         return value % 2 == 0
> ```

算法题中的 `Solution` 通常使用实例方法，因此一个方法调用同类中的另一个实例方法时写 `self.xxx()`。

---

## 8. self 与局部函数怎么选

| 需求 | 推荐写法 |
| --- | --- |
| 辅助逻辑只服务于当前方法 | 局部函数 |
| 需要直接捕获当前方法的局部状态 | 局部函数 |
| 多个方法需要复用辅助逻辑 | 实例方法 |
| 递归状态放在实例属性中 | 实例方法 |
| 辅助函数与实例状态完全无关 | 局部函数或静态方法 |

不要为了省略参数就把所有状态放到 `self` 上。在线评测可能复用同一个实例，如果没有在每次入口正确重置实例属性，会产生跨调用污染。

---

## 9. 常见易错点

> [!warning]- 易错点
> - 在实例方法内裸写同类方法名，触发 `NameError`。
> - 给局部函数写 `self.dfs()`，触发 `AttributeError`。
> - 定义实例方法时漏写第一个参数 `self`，调用时参数数量不匹配。
> - 把 `self` 当成关键字；它只是强约定名称。
> - 把递归结果保存在实例属性中，却忘记在每次公开方法调用前重置。
> - 混淆 `self.method` 与 `self.method()`：前者取得方法对象，后者才执行调用。

---

## 复杂度

> [!summary]- 复杂度
> `self.method()` 与局部函数调用方式本身不会改变算法的大 O 复杂度。递归复杂度取决于递归树：例如遍历二叉树每个节点一次，时间复杂度为 $O(n)$，递归栈空间为 $O(h)$，其中 $h$ 是树高。

---

## 一句话总结

局部函数和全局函数按 LEGB 直接查名字，实例方法是对象属性，要通过 `self` 访问；先看定义位置，再决定递归调用写法。

---

## 模板归纳

```python
# 局部辅助函数
class Solution:
    def solve(self, root):
        def dfs(node):
            if node is None:
                return
            dfs(node.left)
            dfs(node.right)

        dfs(root)


# 实例方法递归
class Solution:
    def dfs(self, node):
        if node is None:
            return
        self.dfs(node.left)
        self.dfs(node.right)
```

相关专题：[[5.1 最大深度、最小深度与路径总和]]、[[5.2 相同，对称，平衡]]。
