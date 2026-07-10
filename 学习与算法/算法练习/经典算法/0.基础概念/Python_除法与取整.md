# Python 除法与取整

Python 的 `/` 执行真除法，`//` 执行向下取整除法，`%` 计算与整除配套的余数。算法题中求中点、批次数和向上取整时，需要特别注意负数方向和整除边界。

---

## 思路

除法题最常见的错误，是把 `//` 理解成“直接去掉小数”。它实际取不大于真实商的最大整数，也就是向负无穷方向取整。

> [!tip]- 核心思路
> ```python
> a / b       # 真除法，结果通常是 float
> a // b      # 向下取整除法
> a % b       # 余数
> divmod(a, b)# 同时得到 (a // b, a % b)
> ```
>
> 当 `b != 0` 时始终满足：
>
> $$a = (a // b) \times b + (a \% b)$$

---

## 1. 四种基础操作

| 写法 | 含义 | 示例结果 |
| --- | --- | --- |
| `5 / 2` | 真除法 | `2.5` |
| `5 // 2` | 向下取整除法 | `2` |
| `5 % 2` | 余数 | `1` |
| `divmod(5, 2)` | 同时求商和余数 | `(2, 1)` |

> [!success]- 基础示例
> ```python
> print(5 / 2)          # 2.5
> print(4 / 2)          # 2.0
> print(5 // 2)         # 2
> print(5 % 2)          # 1
> print(divmod(5, 2))   # (2, 1)
> ```

即使刚好整除，`/` 通常也返回浮点数。需要整数商时使用 `//`，不要先用 `/` 再随意转成 `int`。

---

## 2. // 是向下取整，不是截断

正数时，向下取整和截断小数看起来相同；负数时差异明显。

> [!example]- 负数整除
> ```python
> print(-5 / 2)   # -2.5
> print(-5 // 2)  # -3
> print(int(-5 / 2))  # -2
> ```

```text
floor(-2.5) = -3    # 向负无穷方向
trunc(-2.5) = -2    # 向 0 方向
```

`//` 使用 `floor` 规则，`int(float_value)` 使用向零截断规则，两者不能混用。

---

## 3. 余数的符号

Python 让余数与除数具有相同符号，并保持整除恒等式成立。

> [!example]- 不同符号组合
> ```python
> print(5 // 2, 5 % 2)      # 2 1
> print(-5 // 2, -5 % 2)    # -3 1
> print(5 // -2, 5 % -2)    # -3 -1
> print(-5 // -2, -5 % -2)  # 2 -1
> ```

例如 `-5 // 2 == -3`，所以余数必须是 `1`：

$$-5 = (-3) \times 2 + 1$$

---

## 4. 向下、向上和向零取整

| 目标 | 推荐写法 |
| --- | --- |
| 向下取整 | `math.floor(x)` |
| 向上取整 | `math.ceil(x)` |
| 向零截断 | `math.trunc(x)` 或 `int(x)` |
| 四舍五入 | `round(x)` |

> [!success]- `math` 模块
> ```python
> import math
>
> x = -2.5
> print(math.floor(x))  # -3
> print(math.ceil(x))   # -2
> print(math.trunc(x))  # -2
> print(round(x))       # -2
> ```

Python 的 `round()` 在恰好位于两个整数中间时采用“舍入到偶数”的规则，例如 `round(2.5) == 2`、`round(3.5) == 4`，不能把它简单理解为传统的“四舍五入”。

---

## 5. 正整数除法的向上取整

当 `a >= 0`、`b > 0` 时：

$$\left\lceil \frac{a}{b} \right\rceil = \frac{a + b - 1}{b}\text{ 向下取整}$$

对应代码：

```python
result = (a + b - 1) // b
```

> [!example]- 计算批次数
> ```python
> def batches(total, capacity):
>     return (total + capacity - 1) // capacity
>
>
> print(batches(10, 3))  # 4
> print(batches(9, 3))   # 3
> print(batches(0, 3))   # 0
> ```

不能写成 `a // b + 1`，因为刚好整除时会多算一次。

---

## 6. 更通用的整数向上取整

当除数 `b > 0` 时，可以使用：

```python
result = -(-a // b)
```

它不会先转成浮点数，适合大整数，也能处理负的被除数。

> [!example]- 与 `math.ceil` 对照
> ```python
> import math
>
> for a in [-7, -1, 0, 1, 7]:
>     integer_result = -(-a // 3)
>     math_result = math.ceil(a / 3)
>     print(integer_result, math_result)
> ```

算法题中数量和容量通常都是正整数，此时 `(a + b - 1) // b` 最直观；符号不确定时，先明确题目要求和除数符号。

---

## 7. 实例一：二分查找中点

> [!success]- 常见中点写法
> ```python
> left = 0
> right = 10
>
> mid = left + (right - left) // 2
> print(mid)  # 5
> ```

Python 整数不会像固定宽度整数那样溢出，因此 `(left + right) // 2` 也安全；使用 `left + (right - left) // 2` 有利于形成跨语言通用习惯。

需要取右中位数时可以写：

```python
mid = left + (right - left + 1) // 2
```

---

## 8. 实例二：计算完成任务所需时间

每小时最多处理 `speed` 个任务，一堆 `pile` 需要的小时数是：

```python
(pile + speed - 1) // speed
```

> [!success]- 汇总所有批次
> ```python
> def total_hours(piles, speed):
>     return sum((pile + speed - 1) // speed for pile in piles)
>
>
> print(total_hours([3, 6, 7, 11], 4))  # 8
> ```

速度、容量或每批处理量不能为 `0`。二分答案时，这类下界通常从 `1` 开始。

---

## 9. 实例三：商和余数拆分

`divmod()` 适合需要同时使用商和余数的场景，例如把总秒数拆成分钟和秒。

> [!success]- 时间单位转换
> ```python
> total_seconds = 367
> minutes, seconds = divmod(total_seconds, 60)
>
> print(minutes)  # 6
> print(seconds)  # 7
> ```

它等价于：

```python
minutes = total_seconds // 60
seconds = total_seconds % 60
```

---

## 10. 整数运算与浮点运算怎么选

| 需求 | 推荐方式 |
| --- | --- |
| 需要真实比例或小数 | `/` |
| 求整数商和余数 | `//`、`%` 或 `divmod()` |
| 正整数批次数 | `(a + b - 1) // b` |
| 大整数精确向上取整 | `-(-a // b)`，其中 `b > 0` |
| 对浮点数按方向取整 | `math.floor()`、`math.ceil()` |

能用整数表达的问题尽量保持整数运算，避免大整数转成浮点数后产生精度损失。

---

## 11. 常见易错点

> [!warning]- 易错点
> - 把 `//` 当成向零截断，忽略负数结果。
> - 用 `a // b + 1` 做向上取整，刚好整除时多算一次。
> - 用 `/` 得到浮点数后再 `int()`，既改变取整方向，也可能损失精度。
> - 忘记检查除数为 `0`，触发 `ZeroDivisionError`。
> - 在速度或容量二分中把左边界设为 `0`，随后发生除零。
> - 误以为 `%` 的结果总是非负；除数为负时余数也可能为负。
> - 把 `round()` 当成固定的传统四舍五入。

---

## 复杂度

> [!summary]- 复杂度
> 在常见算法题的整数运算模型中，单次 `/`、`//`、`%` 和 `divmod()` 通常记为 $O(1)$，额外空间为 $O(1)$。严格来说，Python 支持任意精度整数，数字位数非常大时，运算成本会随位数增长。

---

## 一句话总结

`/` 求真实商，`//` 向下取整，`%` 给出配套余数；正整数向上取整使用 `(a + b - 1) // b`，并始终检查负数方向和除数为零的边界。

---

## 模板归纳

```python
# 真除法
ratio = a / b

# 整数商与余数
quotient, remainder = divmod(a, b)

# 正整数向上取整：a >= 0, b > 0
ceil_value = (a + b - 1) // b

# 更通用的整数向上取整：b > 0
ceil_value = -(-a // b)

# 左中位数与右中位数
left_mid = left + (right - left) // 2
right_mid = left + (right - left + 1) // 2
```

看到“每批最多处理多少”“需要几轮完成”“求中点”“拆分商和余数”，优先想到整数除法模板。
