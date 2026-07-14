# SQL 窗口函数：不折叠行的分组计算

窗口函数在**保留每一行**的前提下，给每行附上"它在组内的位次 / 累计值 / 邻居行的值"。当问题形如"每组前 N 条""每个 key 只留最新一条""和上一行比较"时，`GROUP BY` 无能为力（它把每组折叠成一行），窗口函数是标准解。

## 思路

`GROUP BY` 回答"每组的聚合值是多少"，代价是丢掉明细行；但很多问题**既要明细又要组内信息**：

- "每个班分数最高的学生是谁" —— 要的是学生这一**行**，不是 `MAX(score)` 这个**数**
- "每个用户最新一笔订单" —— `MAX(created_at)` 拿不回订单的其他字段（经典错误是再 JOIN 回去，又慢又容易撞并列）

窗口函数的核心机制：`OVER (PARTITION BY 分组键 ORDER BY 排序键)` 声明一个"窗口"——每行看着自己所在分组、按指定顺序，计算出自己的序号/排名/累计值，**行数不变，只是多了一列**。

## 1. 导入与创建

无需导入，是 SQL 语法本身。版本要求：SQLite ≥ 3.25、MySQL ≥ 8.0、PostgreSQL 全支持。

基本骨架：

```sql
函数名() OVER (
    PARTITION BY 分组列      -- 可省略：省略则全表是一个组
    ORDER BY 排序列          -- 排名类函数必写；聚合类不写则算"全组"，写了变"累计"
) AS 别名
```

## 2. 核心操作

| 操作 | 作用 | 注意事项 |
|---|---|---|
| `ROW_NUMBER()` | 组内唯一连续序号 1,2,3,4 | 并列时强行分先后，**顺序不确定**，需补唯一键 |
| `RANK()` | 排名，并列同名次、之后跳号 1,2,2,4 | 比赛名次语义 |
| `DENSE_RANK()` | 排名，并列同名次、不跳号 1,2,2,3 | "第几档"语义 |
| `LAG(col, n, 默认值)` | 取组内**上** n 行的值 | 第一行没有上一行，返回 NULL（或给的默认值） |
| `LEAD(col, n, 默认值)` | 取组内**下** n 行的值 | 末行同理 |
| `SUM/AVG/COUNT(col) OVER (...)` | 组内聚合但不折叠行 | 带 `ORDER BY` = 累计值；不带 = 全组同一个值 |

三个排名函数的差异（按分数降序）：

| 姓名 | 分数 | ROW_NUMBER | RANK | DENSE_RANK |
|---|---|:-:|:-:|:-:|
| 张三 | 95 | 1 | 1 | 1 |
| 李四 | 90 | 2 | 2 | 2 |
| 王五 | 90 | 3 | 2 | 2 |
| 赵六 | 85 | 4 | 4 | 3 |

## 3. 关键行为与边界

**最重要的一条：窗口函数不能出现在 WHERE 里。** SQL 的逻辑执行顺序是 `WHERE → GROUP BY → HAVING → 窗口函数 → ORDER BY`——WHERE 执行时窗口列还不存在。所以"取每组第一名"必须**套一层**：

```sql
-- ❌ 报错：misuse of window function
SELECT * FROM t WHERE ROW_NUMBER() OVER (...) = 1;

-- ✅ 子查询（或 CTE）先算出 rn，外层再过滤
SELECT * FROM (
    SELECT *, ROW_NUMBER() OVER (PARTITION BY grp ORDER BY val) AS rn
    FROM t
) WHERE rn = 1;
```

其他边界：

- `PARTITION BY` 省略 → 全表一个窗口（全表排名）
- `ROW_NUMBER` 遇并列时谁先谁后**不确定**：要可复现的结果，`ORDER BY` 末尾必须补能唯一定序的列（主键 id 是万能兜底）
- 窗口里的 `ORDER BY` 和语句最外层的 `ORDER BY` 是**两回事**：前者决定窗口内怎么算，后者决定结果集怎么显示

## 4. 实例一：每组取最快一行（Top-1 per group）

输入：benchmark 运行表 `runs(unknowns, ranks, threads, total_cores, time_sec)`。目标：同一问题规模下，"单线程模式"（threads = 1）和"多线程模式"（threads > 1）**各自**最快的一行。两个组、每组只要第一名——`PARTITION BY` 一个布尔表达式即可分出两组。

```sql
SELECT * FROM (
    SELECT
        ranks, threads, total_cores, time_sec,
        CASE WHEN threads = 1 THEN 'single' ELSE 'multi' END AS mode,
        ROW_NUMBER() OVER (
            PARTITION BY (threads = 1)                -- 布尔表达式也能当分组键
            ORDER BY time_sec ASC, total_cores ASC,
                     ranks ASC, threads ASC           -- 完整 tie-break 链保证可复现
        ) AS rn
    FROM runs
    WHERE unknowns = 1000000
)
WHERE rn = 1;                                          -- 返回 0/1/2 行
```

关键点：返回行数天然编码了"哪组有数据"（0 = 都没有，1 = 缺一组，2 = 齐全）；`ORDER BY` 链写满四级是为了并列时结果确定。最容易写错的位置：把 `WHERE rn = 1` 写进内层。

## 5. 实例二：每个 key 只留最新一条（按时间去重）

输入：订单表 `orders(id, user_id, amount, created_at)`，同一用户多条。目标：每个用户**最新**的那笔订单（整行）。这是窗口函数最高频的生产用法——`MAX(created_at) + JOIN` 的写法在时间戳并列时会翻倍出行，`ROW_NUMBER` 天然只留一行。

```sql
SELECT id, user_id, amount, created_at FROM (
    SELECT *,
        ROW_NUMBER() OVER (
            PARTITION BY user_id
            ORDER BY created_at DESC, id DESC   -- id 兜底：同一秒两笔也能唯一定序
        ) AS rn
    FROM orders
)
WHERE rn = 1;
```

## 6. 实例三：每组前 N 名——ROW_NUMBER 和 RANK 给出不同答案

输入：`employees(dept, name, salary)`。目标：每个部门工资**前 3**。这里选哪个函数是业务决策，不是语法偏好：

```sql
SELECT dept, name, salary FROM (
    SELECT *,
        RANK() OVER (PARTITION BY dept ORDER BY salary DESC) AS rk
    FROM employees
)
WHERE rk <= 3;
```

- `RANK() <= 3`：两人并列第 3 → **两人都进**（可能返回 4 行/组）
- `ROW_NUMBER() <= 3`：严格 3 行/组，但并列的第 3 名**随机淘汰一个**
- `DENSE_RANK() <= 3`：取"前三档"，档内人数不限

面试追问"前三名并列怎么办"时，答案是反问需求方要哪种语义。

## 7. 实例四：和上一行比较（LAG 算环比）

输入：`daily_sales(day, revenue)`。目标：每天的营收环比增量。没有 LAG 就得自连接 `ON a.day = b.day + 1`（日期有空洞就断）：

```sql
SELECT
    day,
    revenue,
    revenue - LAG(revenue, 1, 0) OVER (ORDER BY day) AS delta,   -- 第一天差值记 0
    SUM(revenue) OVER (ORDER BY day) AS running_total            -- 顺手：累计营收
FROM daily_sales;
```

注意 `SUM(...) OVER (ORDER BY day)`：**加了 ORDER BY 的聚合窗口是累计值**（截止本行），去掉 ORDER BY 就变成"全表总和"贴在每一行上——一个关键字改变整个语义。

## 8. 与相近写法怎么选

| 需求 | 推荐写法 | 原因 |
|---|---|---|
| 每组一个聚合数，不要明细 | `GROUP BY` | 天生干这个的 |
| 全表只要最值的那一行 | `ORDER BY ... LIMIT 1` | 最简单，无需窗口 |
| 每组各取 Top-1/Top-N | `ROW_NUMBER/RANK` + 外层过滤 | GROUP BY 拿不回整行 |
| 组数固定且很少（如 2 组） | 也可以每组一条 `WHERE ... LIMIT 1` 点查 | 可读性更高、缺组判断直白；单机小数据时两次往返成本为零 |
| 与相邻行比较 | `LAG/LEAD` | 自连接又慢又怕数据空洞 |
| 累计/滑动统计 | 聚合函数 + `OVER (ORDER BY ...)` | 一列搞定，无需自连接 |

## 9. 常见易错点

- 在 `WHERE` 里直接过滤窗口列 → 语法错误；必须子查询/CTE 包一层
- `ROW_NUMBER` 并列顺序不确定 → `ORDER BY` 末尾补主键，否则结果不可复现
- 忘写 `PARTITION BY` → 全表排名，每组不再从 1 开始
- 聚合窗口加/不加 `ORDER BY` 语义完全不同（累计 vs 全组）
- Top-N 场景没想清楚要 `ROW_NUMBER` 还是 `RANK` → 并列时多行或漏行

## 复杂度

每个分区内一次排序：O(n log n)；对比"每组最值再 JOIN 回原表"或自连接的 O(n²)/两遍扫描，窗口函数通常一遍扫描 + 排序完成。

## 一句话总结

窗口函数 = "保留每一行的 GROUP BY"：看到**每组前 N、每 key 留一条、和上一行比、累计值**这类信号，就写 `函数() OVER (PARTITION BY 组 ORDER BY 序)`，再套一层子查询过滤序号。

## 模板归纳

看到以下信号时，优先想到窗口函数：

- "每个 XX 的前 N 名 / 最新一条 / 最快一次"（分组 Top-N）
- "去重，但要保留整行"（ROW_NUMBER = 1）
- "环比 / 和上一条记录比"（LAG/LEAD）
- "累计到当前行的总和 / 移动平均"（聚合 + ORDER BY 窗口）

```sql
-- 可直接复用的最小模板：每组取 Top-1
SELECT * FROM (
    SELECT
        t.*,
        ROW_NUMBER() OVER (
            PARTITION BY <分组键>
            ORDER BY <业务排序键>, <主键兜底>
        ) AS rn
    FROM t
)
WHERE rn = 1;
```