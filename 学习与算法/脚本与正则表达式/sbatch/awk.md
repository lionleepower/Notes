
```
awk 是“逐行读取文本，然后按条件处理每一行”的工具。
```

最常见形式是：

```
awk '条件 { 动作 }' 文件名
```

例如：

```
awk 'NR > 1 { print }' runtime_results.csv
```

意思是：

```
从第 2 行开始，打印每一行
```

---

**1. 基本结构**

```
awk 'pattern { action }' file
```

中文理解：

```
如果这一行满足 pattern，就执行 action
```

例子：

```
awk '$14 == "completed" { print }' runtime_results.csv
```

意思是：

```
如果第 14 列是 completed，就打印这一行
```

如果没有条件，默认每行都执行：

```
awk '{ print }' runtime_results.csv
```

如果没有动作，默认打印整行：

```
awk '$14 == "completed"' runtime_results.csv
```

等价于：

```
awk '$14 == "completed" { print }' runtime_results.csv
```

---

**2. 指定分隔符**

默认情况下，`awk` 用空格或 tab 分列。

CSV 是逗号分隔，所以要写：

```
awk -F, '{ print $1, $2 }' runtime_results.csv
```

这里：

```
-F,
```

表示 field separator 是逗号。

也可以写成：

```
awk -F',' '{ print $1, $2 }' runtime_results.csv
```

---

**3. 常用内置变量**

```
NR
```

当前行号，从 1 开始。

```
awk 'NR == 1 { print }' file.csv
```

打印第 1 行。

```
awk 'NR > 1 { print }' file.csv
```

跳过第 1 行，打印第 2 行之后。

```
$1, $2, $3
```

第 1 列、第 2 列、第 3 列。

```
awk -F, '{ print $1, $12 }' runtime_results.csv
```

打印第 1 列和第 12 列。

```
$0
```

整行。

```
awk '{ print $0 }' file.txt
```

打印整行。

```
NF
```

当前行有多少列。

```
awk -F, '{ print NF }' runtime_results.csv
```

打印每一行的列数。

---

**4. BEGIN 和 END**

`BEGIN`：在读取文件之前执行。

```
awk 'BEGIN { print "start" } { print }' file.txt
```

`END`：所有行处理完之后执行。

```
awk '{ count++ } END { print count }' file.txt
```

意思是统计总行数。

你的脚本里有：

```
awk -F, 'NR > 1 && $14 == "completed" { count++ } END { print count + 0 }' "${INPUT_CSV}"
```

意思是：

```
跳过表头，只统计第 14 列是 completed 的行，最后打印数量
```

---

**5. 条件判断**

等于：

```
$14 == "completed"
```

不等于：

```
$14 != "completed"
```

大于：

```
$12 > 100
```

小于：

```
$12 < 100
```

并且：

```
NR > 1 && $14 == "completed"
```

或者：

```
$1 == "small" || $1 == "medium"
```

---

**6. print 和 printf**

简单打印用 `print`：

```
awk -F, '{ print $1, $12 }' runtime_results.csv
```

输出字段之间默认用空格隔开。

想控制格式，用 `printf`：

```
awk -F, '{ printf "%s %.2f\n", $1, $12 }' runtime_results.csv
```

这里：

```
%s = 字符串
%d = 整数
%f = 小数
%.2f = 保留 2 位小数
\n = 换行
```

例如：

```
awk -F, 'NR > 1 { printf "%s runtime=%.2f\n", $1, $12 }' runtime_results.csv
```

可能输出：

```
small runtime=245.70
small runtime=248.00
```

---

**7. 变量和计算**

awk 里可以直接定义变量：

```
awk -F, 'NR > 1 { runtime = $12 + 0; print runtime }' runtime_results.csv
```

`+ 0` 是常见写法，意思是把字符串转成数字。

计算 CU：

```
awk -F, 'NR > 1 { cu = $5 * $12 / 3600; print cu }' runtime_results.csv
```

你的脚本里：

```
runtime = $12 + 0
nodes = $5 + 0
estimated_cu = nodes * runtime / 3600
```

意思是：

```
runtime_seconds 转成数字
nodes 转成数字
计算 estimated CU
```

---

**8. gsub 替换**

```
gsub(查找内容, 替换内容)
```

例如：

```
gsub(/\r/, "")
```

意思是删除当前行里的所有 `\r`。

如果指定某一列：

```
gsub(/\r/, "", $15)
```

意思是只删除第 15 列里的 `\r`。

---

**9. 常用模板**

统计 completed 行数：

```
awk -F, 'NR > 1 && $14 == "completed" { count++ } END { print count + 0 }' runtime_results.csv
```

打印 completed 行：

```
awk -F, 'NR > 1 && $14 == "completed" { print }' runtime_results.csv
```

打印某几列：

```
awk -F, 'NR > 1 { print $1, $8, $7, $12 }' runtime_results.csv
```

计算新列：

```
awk -F, 'NR > 1 { cu = $5 * $12 / 3600; print $1, cu }' runtime_results.csv
```

生成 Markdown 表行：

```
awk -F, 'NR > 1 {
  printf "| %s | %d | %.4g |\n", $1, $9, $12
}' runtime_results.csv
```

---

你现在最需要记住这几个就够了：

```
awk -F, '条件 { 动作 }' 文件
```

```
NR
```

当前第几行。

```
$1, $2, $14
```

第几列。

```
BEGIN { ... }
END { ... }
```

开始前、结束后执行。

```
count++
```

计数加 1。

```
printf
```

按格式输出。