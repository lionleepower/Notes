# CSV、Bash 与 Slurm Size-Grid 批处理脚本

## 1. 学习目标

在 PETSc benchmark 项目中，我们希望测试不同问题规模、不同 MPI 进程数和不同 OpenMP 线程数对程序性能的影响。

最终需要理解并能够自行编写：

```text
scripts/run_ex2_size_grid.sbatch
```

这个脚本的核心任务是：

1. 从 CSV 文件中读取多个问题规模；
    
2. 使用 Bash 循环遍历输入参数；
    
3. 生成合法的 MPI rank 与 OpenMP thread 组合；
    
4. 在 ARCHER2 上使用 `srun` 启动 PETSc 程序；
    
5. 将每次运行的完整输出保存为日志；
    
6. 从日志中提取关键数据；
    
7. 把结果汇总为一个新的 CSV 文件。
    

不要直接背诵整个 Slurm 脚本。更好的学习方式是将它拆分为几个独立层次：

```text
CSV 输入文件
    ↓
Bash 读取与过滤
    ↓
生成合法的 rank-thread 参数组合
    ↓
使用 mpirun 或 srun 启动程序
    ↓
保存日志并提取结果
```

前三个步骤可以安全地在 WSL 中练习。

涉及以下内容时，需要在 ARCHER2 上测试：

```text
#SBATCH
module
srun
```

---

## 2. 为什么需要批处理脚本？

假设我们希望测试以下三种问题规模：

|scale|m|n|unknowns|
|---|--:|--:|--:|
|small|1000|1000|1000000|
|medium|2000|2000|4000000|
|large|4000|4000|16000000|

同时，我们希望测试以下 MPI ranks：

```text
1 2 4 8
```

以及以下 OpenMP threads：

```text
1 2 4 8
```

如果手动运行，每一种组合都要输入一次命令。

例如：

```bash
export OMP_NUM_THREADS=1
srun --nodes=1 --ntasks=1 --cpus-per-task=1 --exact \
  /path/to/ex2 -m 1000 -n 1000 -ksp_converged_reason -log_view
```

然后再修改参数：

```bash
export OMP_NUM_THREADS=2
srun --nodes=1 --ntasks=1 --cpus-per-task=2 --exact \
  /path/to/ex2 -m 1000 -n 1000 -ksp_converged_reason -log_view
```

这种方式存在几个问题：

- 容易漏掉某些组合；
    
- 容易错误地复制参数；
    
- 不方便重复多次测试；
    
- 不方便统一保存结果；
    
- 很难保证实验可复现。
    

批处理脚本的作用，就是自动化执行这些重复工作。

---

# 第一部分：使用 CSV 描述 benchmark 输入

## 3. 什么是 CSV？

CSV 是 **Comma-Separated Values** 的缩写，即“逗号分隔值”。

它是一种简单的文本表格格式。每一行表示一条记录，不同字段之间使用逗号分隔。

例如：

```csv
scale,m,n,unknowns
small,1000,1000,1000000
medium,2000,2000,4000000
large,4000,4000,16000000
```

第一行通常称为 **header**，即表头：

```csv
scale,m,n,unknowns
```

后续每一行表示一个问题规模。

---

## 4. PETSc ex2 中的问题规模

项目中的问题规模文件是：

```text
scripts/ex2_problem_sizes.csv
```

其中一行可能是：

```csv
small,1000,1000,1000000
```

四个字段分别表示：

|字段|含义|
|---|---|
|`scale`|人类可读的问题规模名称|
|`m`|二维网格的第一个维度|
|`n`|二维网格的第二个维度|
|`unknowns`|线性系统中的未知数总量|

对于二维网格：

$$  
\text{unknowns} = m \times n  
$$

例如：

$$  
m = 1000  
$$

$$  
n = 1000  
$$

因此：

$$  
\text{unknowns} = 1000 \times 1000 = 1{,}000{,}000  
$$

CSV 中的：

```csv
small,1000,1000,1000000
```

表示：

```text
运行一个 1000 × 1000 的二维网格问题
总共有 1,000,000 个未知量
将这个规模命名为 small
```

---

## 5. 为什么将输入参数放在 CSV 中？

也可以直接把参数写死在脚本中：

```bash
M=1000
N=1000
```

但是，如果需要测试多个问题规模，脚本会变得很难维护。

例如：

```bash
M1=1000
N1=1000

M2=2000
N2=2000

M3=4000
N3=4000
```

使用 CSV 文件后，输入数据和执行逻辑可以分离：

```text
CSV 文件负责描述测试数据
Bash 脚本负责读取并执行测试
```

这样做有几个优点：

- 增加问题规模时，只需要修改 CSV；
    
- 不需要修改主脚本；
    
- 输入参数更加直观；
    
- 实验配置更容易保存和复现；
    
- 可以单独检查数据是否正确。
    

---

# 第二部分：使用 Bash 读取 CSV

## 6. Bash 中的 while read 循环

下面的 Bash 代码可以逐行读取 CSV 文件：

```bash
while IFS=, read -r scale m n unknowns; do
  printf '%s: %s x %s = %s unknowns\n' \
    "${scale}" "${m}" "${n}" "${unknowns}"
done < scripts/ex2_problem_sizes.csv
```

需要逐部分理解。

---

## 7. while 循环的基本结构

最简单的 Bash `while` 循环结构是：

```bash
while 条件; do
  循环体
done
```

例如：

```bash
count=1

while (( count <= 3 )); do
  echo "${count}"
  count=$(( count + 1 ))
done
```

输出：

```text
1
2
3
```

在读取文件时，条件部分可以写成：

```bash
read -r scale m n unknowns
```

只要 `read` 还可以成功读取下一行，循环就会继续。

---

## 8. IFS 是什么？

`IFS` 是 **Internal Field Separator** 的缩写。

它决定 Bash 应该使用什么字符切分一行文本。

默认情况下，Bash 通常按照空格或制表符切分字段。

例如：

```bash
read -r a b c <<< "apple banana orange"
```

此时：

```text
a = apple
b = banana
c = orange
```

但是 CSV 使用逗号分隔：

```text
small,1000,1000,1000000
```

因此需要临时将字段分隔符设置为逗号：

```bash
IFS=,
```

完整写法：

```bash
IFS=, read -r scale m n unknowns
```

读取：

```text
small,1000,1000,1000000
```

之后变量内容为：

```text
scale    = small
m        = 1000
n        = 1000
unknowns = 1000000
```

---

## 9. 为什么使用 read -r？

命令：

```bash
read -r
```

中的 `-r` 表示：

```text
不要将反斜杠 \ 解释为转义字符
```

例如，如果路径中出现：

```text
some\path
```

使用 `read -r` 可以保留原始内容。

在读取文件时，通常推荐始终使用：

```bash
read -r
```

这是一种更加安全的写法。

---

## 10. 文件重定向：done < file.csv

循环的最后一行是：

```bash
done < scripts/ex2_problem_sizes.csv
```

其中 `<` 是输入重定向符号。

它表示：

```text
将 scripts/ex2_problem_sizes.csv 的内容作为 while 循环的输入
```

可以将其理解为：

```text
打开 CSV 文件
    ↓
逐行送入 read
    ↓
每读取一行，执行一次循环体
```

---

## 11. printf 的作用

循环中的输出命令是：

```bash
printf '%s: %s x %s = %s unknowns\n' \
  "${scale}" "${m}" "${n}" "${unknowns}"
```

`printf` 与 C 语言中的 `printf()` 类似。

格式字符串：

```text
%s: %s x %s = %s unknowns\n
```

其中：

- `%s` 表示插入一个字符串；
    
- `\n` 表示换行；
    
- `\` 表示当前命令尚未结束，下一行仍然属于同一条命令。
    

如果变量内容为：

```text
scale    = small
m        = 1000
n        = 1000
unknowns = 1000000
```

输出为：

```text
small: 1000 x 1000 = 1000000 unknowns
```

---

## 12. 为什么变量通常写成 "${variable}"？

推荐写法：

```bash
"${scale}"
```

而不是：

```bash
$scale
```

双引号可以避免变量内容中包含空格时发生意外切分。

例如：

```bash
name="small test"
```

如果写成：

```bash
printf '%s\n' $name
```

Bash 可能将它拆成两个参数：

```text
small
test
```

如果写成：

```bash
printf '%s\n' "${name}"
```

则会保留为一个完整字符串：

```text
small test
```

因此，在一般情况下，读取变量时推荐使用：

```bash
"${variable}"
```

---

## 13. CSV 表头的问题

CSV 文件的第一行通常是：

```csv
scale,m,n,unknowns
```

如果直接读取，Bash 也会将它视为普通数据。

第一次循环中：

```text
scale    = scale
m        = m
n        = n
unknowns = unknowns
```

但是这不是实际问题规模，所以需要跳过。

生产脚本中使用：

```bash
[[ "${SCALE}" == "scale" ]] && continue
```

---

## 14. 条件判断 [[ ... ]]

Bash 中可以使用：

```bash
[[ 条件 ]]
```

进行判断。

例如：

```bash
name="small"

if [[ "${name}" == "small" ]]; then
  echo "This is the small case."
fi
```

在 CSV 脚本中：

```bash
[[ "${SCALE}" == "scale" ]]
```

表示判断：

```text
变量 SCALE 的内容是否等于字符串 scale
```

如果相等，说明当前行是表头。

---

## 15. && 表示什么？

代码：

```bash
[[ "${SCALE}" == "scale" ]] && continue
```

等价于：

```bash
if [[ "${SCALE}" == "scale" ]]; then
  continue
fi
```

`&&` 表示：

```text
只有左侧命令执行成功时，才执行右侧命令
```

在 Bash 中：

```text
条件成立
```

通常对应退出状态码：

```text
0
```

因此：

```bash
[[ "${SCALE}" == "scale" ]] && continue
```

表示：

```text
如果 SCALE 等于 scale，则执行 continue
```

---

## 16. continue 的作用

`continue` 表示：

```text
跳过当前这一轮循环剩余的内容，直接进入下一轮循环
```

例如：

```bash
for x in 1 2 3 4; do
  [[ "${x}" == "2" ]] && continue
  echo "${x}"
done
```

输出：

```text
1
3
4
```

数字 `2` 被跳过。

在 CSV 读取过程中：

```bash
[[ "${SCALE}" == "scale" ]] && continue
```

可以跳过表头。

---

## 17. 一个完整的 CSV 读取示例

创建文件：

```text
tutorial/examples/local_problem_sizes.csv
```

内容如下：

```csv
scale,m,n,unknowns
tiny,8,8,64
small,100,100,10000
medium,200,300,60000
```

编写脚本：

```bash
#!/usr/bin/env bash

while IFS=, read -r SCALE M N UNKNOWNS; do
  [[ "${SCALE}" == "scale" ]] && continue

  printf 'scale=%s, m=%s, n=%s, unknowns=%s\n' \
    "${SCALE}" "${M}" "${N}" "${UNKNOWNS}"
done < tutorial/examples/local_problem_sizes.csv
```

输出：

```text
scale=tiny, m=8, n=8, unknowns=64
scale=small, m=100, n=100, unknowns=10000
scale=medium, m=200, n=300, unknowns=60000
```

---

# 第三部分：默认值与环境变量覆盖

## 18. 为什么需要默认值？

脚本需要允许两种运行方式。

第一种：直接运行，使用默认设置。

```bash
bash tutorial/examples/local_size_grid_practice.sh
```

第二种：临时修改部分参数。

```bash
SCALES="small medium" REPEAT=3 \
bash tutorial/examples/local_size_grid_practice.sh
```

为了同时支持这两种方式，可以使用 Bash 的默认值语法。

---

## 19. 默认值语法 ${VARIABLE:-default}

示例：

```bash
SCALES="${SCALES:-small}"
REPEAT="${REPEAT:-1}"
```

其中：

```bash
"${SCALES:-small}"
```

表示：

```text
如果 SCALES 已经存在并且非空，则使用原来的 SCALES
否则使用默认值 small
```

类似地：

```bash
"${REPEAT:-1}"
```

表示：

```text
如果 REPEAT 已经存在并且非空，则使用原来的 REPEAT
否则使用默认值 1
```

---

## 20. 默认值示例

脚本：

```bash
#!/usr/bin/env bash

SCALES="${SCALES:-small}"
REPEAT="${REPEAT:-1}"

printf 'SCALES=%s\n' "${SCALES}"
printf 'REPEAT=%s\n' "${REPEAT}"
```

直接运行：

```bash
bash demo.sh
```

输出：

```text
SCALES=small
REPEAT=1
```

覆盖变量：

```bash
SCALES="small medium" REPEAT=3 bash demo.sh
```

输出：

```text
SCALES=small medium
REPEAT=3
```

---

## 21. 命令前面的变量赋值是什么意思？

命令：

```bash
SCALES="small medium" REPEAT=3 bash demo.sh
```

表示：

```text
只为这一次 bash demo.sh 命令临时设置环境变量
```

它不会永久修改当前终端的变量。

执行结束后，可以检查：

```bash
echo "${SCALES}"
```

通常不会得到刚才临时设置的值。

这种写法适合 benchmark，因为可以快速运行不同配置，而不需要修改脚本文件。

---

## 22. 本地 Bash 与 Slurm 中的覆盖方式

本地运行：

```bash
SCALES="small medium" REPEAT=3 \
bash tutorial/examples/local_size_grid_practice.sh
```

提交 ARCHER2 作业：

```bash
SCALES="small medium" REPEAT=3 \
sbatch scripts/run_ex2_size_grid.sbatch
```

两者逻辑类似：

```text
命令前设置环境变量
    ↓
脚本启动时读取变量
    ↓
如果变量存在，覆盖默认值
```

---

## 23. 常用的用户可控参数

生产脚本中可能包含：

```bash
SCALES="${SCALES:-small}"
REPEAT="${REPEAT:-1}"
RANKS="${RANKS:-1 2 4 8}"
THREADS="${THREADS:-1 2 4 8}"
MAX_CORES="${MAX_CORES:-8}"
```

这些变量分别表示：

|变量|含义|
|---|---|
|`SCALES`|选择哪些问题规模|
|`REPEAT`|每一种配置重复运行多少次|
|`RANKS`|测试哪些 MPI rank 数量|
|`THREADS`|测试哪些 OpenMP thread 数量|
|`MAX_CORES`|当前实验允许使用的最大 CPU 核心数|

---

# 第四部分：生成合法的 MPI rank-thread 参数组合

## 24. MPI ranks 与 OpenMP threads 的关系

在 MPI 与 OpenMP 混合并行中：

- MPI 使用多个进程；
    
- 每个 MPI 进程内部可以创建多个 OpenMP 线程。
    

假设：

```text
MPI ranks = R
每个 rank 的 OpenMP threads = T
```

那么总核心需求为：

$$  
P = R \times T  
$$

其中：

|符号|含义|
|---|---|
|$R$|MPI rank 数量|
|$T$|每个 MPI rank 的 OpenMP thread 数量|
|$P$|总共占用的 CPU cores 数量|

---

## 25. 具体例子

如果：

$$  
R = 2  
$$

$$  
T = 4  
$$

那么：

$$  
P = R \times T = 2 \times 4 = 8  
$$

表示：

```text
启动 2 个 MPI 进程
每个 MPI 进程启动 4 个 OpenMP 线程
总共需要 8 个 CPU cores
```

可以画成：

```text
MPI rank 0
├── OpenMP thread 0
├── OpenMP thread 1
├── OpenMP thread 2
└── OpenMP thread 3

MPI rank 1
├── OpenMP thread 0
├── OpenMP thread 1
├── OpenMP thread 2
└── OpenMP thread 3
```

总线程数量：

```text
4 + 4 = 8
```

---

## 26. 为什么不能测试所有组合？

假设当前最多允许使用：

```bash
MAX_CORES=8
```

候选 MPI ranks 为：

```bash
RANKS_LIST=(1 2 4 8)
```

候选 OpenMP threads 为：

```bash
THREADS_LIST=(1 2 4 8)
```

并不是所有组合都合法。

例如：

```text
R = 4
T = 8
```

需要的 CPU cores 数量为：

$$  
P = 4 \times 8 = 32  
$$

但是：

$$  
32 > 8  
$$

因此这个组合必须跳过。

---

## 27. Bash 数组

Bash 中可以使用数组保存多个候选值：

```bash
RANKS_LIST=(1 2 4 8)
THREADS_LIST=(1 2 4 8)
```

遍历数组：

```bash
for R in "${RANKS_LIST[@]}"; do
  echo "${R}"
done
```

输出：

```text
1
2
4
8
```

其中：

```bash
"${RANKS_LIST[@]}"
```

表示：

```text
取出数组中的所有元素，并逐个传递给循环
```

---

## 28. Bash 中的整数运算

Bash 中可以使用：

```bash
$(( 数学表达式 ))
```

执行整数运算。

例如：

```bash
A=4
B=2
C=$(( A * B ))

echo "${C}"
```

输出：

```text
8
```

在参数组合脚本中：

```bash
P=$(( R * T ))
```

表示计算：

$$  
P = R \times T  
$$

---

## 29. 推导最大合法线程数

对于给定的 MPI ranks 数量 $R$，总核心数必须满足：

$$  
R \times T \leq \text{MAX_CORES}  
$$

为了求合法的最大线程数 $T$，两边同时除以 $R$：

$$  
T \leq \frac{\text{MAX_CORES}}{R}  
$$

因此：

$$  
T_{\max} = \left\lfloor \frac{\text{MAX_CORES}}{R} \right\rfloor  
$$

在 Bash 中写为：

```bash
MAX_T=$(( MAX_CORES / R ))
```

由于 Bash 默认执行整数除法，小数部分会自动舍去。

例如：

```text
MAX_CORES = 8
R = 2
```

则：

$$  
T_{\max} = \frac{8}{2} = 4  
$$

因此合法的线程数可以是：

```text
1 2 4
```

但不能是：

```text
8
```

---

## 30. 使用 continue 跳过非法组合

代码：

```bash
(( T > MAX_T )) && continue
```

表示：

```text
如果 T 大于最大合法线程数 MAX_T
则跳过当前组合
```

其中：

```bash
(( 数学条件 ))
```

用于执行算术判断。

等价写法：

```bash
if (( T > MAX_T )); then
  continue
fi
```

---

## 31. 完整的 rank-thread 组合生成代码

```bash
#!/usr/bin/env bash

MAX_CORES=8
RANKS_LIST=(1 2 4 8)
THREADS_LIST=(1 2 4 8)

for R in "${RANKS_LIST[@]}"; do
  MAX_T=$(( MAX_CORES / R ))

  for T in "${THREADS_LIST[@]}"; do
    (( T > MAX_T )) && continue

    P=$(( R * T ))

    printf 'R=%s, T=%s, P=%s\n' "${R}" "${T}" "${P}"
  done
done
```

输出：

```text
R=1, T=1, P=1
R=1, T=2, P=2
R=1, T=4, P=4
R=1, T=8, P=8
R=2, T=1, P=2
R=2, T=2, P=4
R=2, T=4, P=8
R=4, T=1, P=4
R=4, T=2, P=8
R=8, T=1, P=8
```

---

## 32. 参数组合表

当：

```text
MAX_CORES = 8
RANKS      = 1 2 4 8
THREADS    = 1 2 4 8
```

所有组合如下：

|MPI ranks $R$|threads $T$|总核心数 $P = R \times T$|是否合法|
|--:|--:|--:|---|
|1|1|1|合法|
|1|2|2|合法|
|1|4|4|合法|
|1|8|8|合法|
|2|1|2|合法|
|2|2|4|合法|
|2|4|8|合法|
|2|8|16|非法|
|4|1|4|合法|
|4|2|8|合法|
|4|4|16|非法|
|4|8|32|非法|
|8|1|8|合法|
|8|2|16|非法|
|8|4|32|非法|
|8|8|64|非法|

---

## 33. 为什么只使用总核心数相同的组合进行对比？

假设总共使用 8 个 CPU cores，可以测试：

|MPI ranks|OpenMP threads per rank|总核心数|
|--:|--:|--:|
|8|1|8|
|4|2|8|
|2|4|8|
|1|8|8|

这些配置分别代表：

```text
纯 MPI：
8 ranks × 1 thread

混合并行：
4 ranks × 2 threads
2 ranks × 4 threads

偏 OpenMP：
1 rank × 8 threads
```

由于总核心数保持相同，性能差异更容易解释。

如果一个配置使用 2 个 cores，另一个配置使用 8 个 cores，那么速度差异可能只是因为资源更多，而不是因为并行模型更好。

因此，固定总核心数进行对比是一种更公平的实验设计。

---

# 第五部分：筛选指定的问题规模

## 34. 为什么需要 SCALES？

CSV 文件中可能包含很多问题规模：

```csv
scale,m,n,unknowns
tiny,8,8,64
small,1000,1000,1000000
medium,2000,2000,4000000
large,4000,4000,16000000
```

但某一次实验可能只想运行：

```text
small medium
```

因此，可以通过环境变量设置：

```bash
SCALES="small medium"
```

---

## 35. 一个简单的规模筛选函数

```bash
scale_selected() {
  local current_scale="$1"

  for selected_scale in ${SCALES}; do
    if [[ "${current_scale}" == "${selected_scale}" ]]; then
      return 0
    fi
  done

  return 1
}
```

调用：

```bash
if ! scale_selected "${SCALE}"; then
  continue
fi
```

逻辑如下：

```text
读取 CSV 中的一个 scale
    ↓
检查它是否出现在 SCALES 中
    ↓
如果没有出现，则跳过当前 CSV 行
```

---

## 36. return 0 与 return 1

在 Bash 中：

```text
0 表示成功
非 0 表示失败
```

这与很多编程语言中的布尔值习惯不同。

函数：

```bash
return 0
```

表示：

```text
找到了匹配的 scale
```

函数：

```bash
return 1
```

表示：

```text
没有找到匹配的 scale
```

例如：

```bash
SCALES="small medium"

if scale_selected "small"; then
  echo "selected"
fi
```

输出：

```text
selected
```

---

# 第六部分：本地命令与 Slurm 命令

## 37. 本地 WSL 中的 MPI 命令

在 WSL 中，如果已经安装 MPI 并编译了 PETSc `ex2`，可以使用：

```bash
export OMP_NUM_THREADS=2

mpirun -n 2 \
  /path/to/ex2 \
  -m 8 \
  -n 8 \
  -ksp_converged_reason \
  -log_view
```

这表示：

```text
启动 2 个 MPI ranks
每个 rank 使用 2 个 OpenMP threads
运行 8 × 8 的问题
输出 PETSc 求解器收敛信息
输出 PETSc 性能日志
```

总核心需求为：

$$  
P = 2 \times 2 = 4  
$$

---

## 38. ARCHER2 中的 Slurm 命令

在 ARCHER2 上，通常使用 `srun`：

```bash
export OMP_NUM_THREADS=2

srun \
  --nodes=1 \
  --ntasks=2 \
  --cpus-per-task=2 \
  --exact \
  /path/to/ex2 \
  -m 1000 \
  -n 1000 \
  -ksp_converged_reason \
  -log_view
```

概念映射如下：

|概念|Bash 变量|Slurm 参数|
|---|---|---|
|MPI ranks|`R`|`--ntasks="${R}"`|
|每个 rank 的 CPU cores|`T`|`--cpus-per-task="${T}"`|
|每个 rank 的 OpenMP threads|`T`|`OMP_NUM_THREADS="${T}"`|
|总核心数|`P=R*T`|由脚本提前验证|

---

## 39. 为什么 --cpus-per-task 和 OMP_NUM_THREADS 都要设置？

以下两项看起来相似，但作用不同：

```bash
--cpus-per-task="${T}"
```

以及：

```bash
export OMP_NUM_THREADS="${T}"
```

`--cpus-per-task` 是告诉 Slurm：

```text
请为每个 MPI task 分配 T 个 CPU cores
```

`OMP_NUM_THREADS` 是告诉 OpenMP runtime：

```text
请让每个 MPI rank 创建 T 个 OpenMP threads
```

如果只设置：

```bash
OMP_NUM_THREADS=4
```

但没有设置：

```bash
--cpus-per-task=4
```

Slurm 可能只为每个 MPI task 分配一个 CPU core。

结果可能是：

```text
4 个线程争抢 1 个核心
```

这称为 **oversubscription**，即过度订阅。

反过来，如果只设置：

```bash
--cpus-per-task=4
```

但没有设置：

```bash
OMP_NUM_THREADS=4
```

那么 Slurm 虽然预留了 4 个 CPU cores，但程序可能只创建 1 个线程。

结果是：

```text
分配了资源，但没有充分使用
```

因此，两者通常需要保持一致。

---

## 40. --exact 的作用

Slurm 命令中：

```bash
--exact
```

表示：

```text
让当前 srun step 精确使用请求的资源
```

在一个较大的 batch allocation 中，如果需要依次运行多个小型测试，`--exact` 可以减少不同测试之间的资源分配歧义。

---

## 41. local command 与 Slurm command 的对应关系

本地 WSL：

```bash
export OMP_NUM_THREADS="${T}"

mpirun -n "${R}" \
  "${EX2_BIN}" \
  -m "${M}" \
  -n "${N}" \
  -ksp_converged_reason \
  -log_view
```

ARCHER2：

```bash
export OMP_NUM_THREADS="${T}"

srun \
  --nodes=1 \
  --ntasks="${R}" \
  --cpus-per-task="${T}" \
  --exact \
  "${EX2_BIN}" \
  -m "${M}" \
  -n "${N}" \
  -ksp_converged_reason \
  -log_view
```

两者的核心概念相同：

```text
选择 MPI ranks
    ↓
设置每个 rank 的 OpenMP threads
    ↓
运行指定大小的 PETSc 问题
```

差别在于：

```text
WSL 使用 mpirun
ARCHER2 使用 Slurm 管理的 srun
```

---

# 第七部分：SBATCH 指令

## 42. 什么是 #SBATCH？

Slurm batch 脚本中可能包含：

```bash
#SBATCH --nodes=1
#SBATCH --time=06:00:00
#SBATCH --output=%x.%j.out
```

这些行看起来像 Bash 注释，因为它们以：

```text
#
```

开头。

对于普通 Bash 来说，它们确实只是注释。

但是，当脚本通过：

```bash
sbatch scripts/run_ex2_size_grid.sbatch
```

提交时，`sbatch` 会读取这些特殊指令。

---

## 43. 常见的 #SBATCH 参数

### 43.1 请求节点数量

```bash
#SBATCH --nodes=1
```

表示：

```text
申请 1 个计算节点
```

---

### 43.2 设置最大运行时间

```bash
#SBATCH --time=06:00:00
```

表示：

```text
最长允许运行 6 小时
```

格式为：

```text
小时:分钟:秒
```

如果作业超过这个时间，Slurm 会终止作业。

---

### 43.3 设置输出文件

```bash
#SBATCH --output=%x.%j.out
```

其中：

|占位符|含义|
|---|---|
|`%x`|Slurm job name|
|`%j`|Slurm job ID|

例如：

```text
job name = run_ex2_size_grid
job ID   = 123456
```

输出文件可能为：

```text
run_ex2_size_grid.123456.out
```

---

## 44. 为什么不能直接在 WSL 中运行生产 sbatch 脚本？

如果执行：

```bash
bash scripts/run_ex2_size_grid.sbatch
```

Bash 会忽略：

```bash
#SBATCH ...
```

因为它们只是注释。

但是脚本执行到后续内容时，仍然可能失败：

```bash
module load ...
srun ...
```

原因是 WSL 通常没有 ARCHER2 上的：

```text
module
srun
```

因此：

```text
本地 WSL 用于学习 Bash 逻辑
ARCHER2 用于测试 Slurm 相关内容
```

---

# 第八部分：脚本安全设置

## 45. set -euo pipefail

生产脚本通常在开头包含：

```bash
set -euo pipefail
```

它用于减少脚本静默失败的风险。

---

## 46. -e：命令失败时立即停止

```bash
set -e
```

表示：

```text
如果某条命令失败，则停止执行脚本
```

例如：

```bash
set -e

cp missing_file.txt output.txt
echo "Finished"
```

由于文件不存在：

```bash
cp missing_file.txt output.txt
```

执行失败。

因此：

```bash
echo "Finished"
```

不会继续执行。

这对于 benchmark 很重要，因为失败的程序不应该继续生成看似正常的结果行。

---

## 47. -u：使用未定义变量时立即停止

```bash
set -u
```

表示：

```text
如果脚本读取了一个尚未定义的变量，则立即报错
```

例如：

```bash
set -u

echo "${RESULT_DIR}"
```

如果 `RESULT_DIR` 从未定义，脚本会停止。

这可以避免拼写错误。

例如，原本想写：

```bash
"${THREADS}"
```

但错误写成：

```bash
"${THREDS}"
```

在没有 `set -u` 时，错误可能不容易发现。

在使用 `set -u` 时，脚本会立刻报告未定义变量。

---

## 48. pipefail：管道前部失败时也停止

Bash 管道示例：

```bash
command_a | command_b
```

默认情况下，管道是否成功通常取决于最后一个命令：

```text
command_b
```

即使：

```text
command_a
```

失败，整个管道仍然可能被视为成功。

使用：

```bash
set -o pipefail
```

之后，只要管道中的任意命令失败，整个管道就会失败。

例如：

```bash
set -o pipefail

grep "missing pattern" file.txt | awk '{print $1}'
```

如果 `grep` 没有匹配到内容，管道会被视为失败。

---

## 49. 为什么 benchmark 脚本需要这些设置？

benchmark 的主要风险不是明显报错，而是：

```text
程序已经失败
但脚本仍然继续运行
最后写出了不可信的 CSV 数据
```

例如：

```text
PETSc 执行失败
    ↓
日志文件不完整
    ↓
awk 没有匹配到数据
    ↓
CSV 中写入空白值
    ↓
后续绘图时误以为这是正常结果
```

因此：

```bash
set -euo pipefail
```

是一种基础的防御性编程方式。

---

# 第九部分：为什么日志文件和结果 CSV 要分开？

## 50. 两种输出文件的职责不同

每次 PETSc 运行时，程序会输出大量信息。

完整输出应该保存为日志：

```text
raw log
```

但是最终绘图通常只需要少量字段：

```text
summary CSV
```

例如：

|scale|m|n|ranks|threads|total_cores|repeat|runtime|
|---|--:|--:|--:|--:|--:|--:|--:|
|small|1000|1000|2|4|8|1|0.452|
|small|1000|1000|4|2|8|1|0.391|

---

## 51. 为什么不能只保存 CSV？

如果只保存 CSV，出现异常时很难调查。

例如：

```text
某一行 runtime 为空
```

可能有多种原因：

- PETSc 程序失败；
    
- `srun` 启动失败；
    
- 日志格式发生变化；
    
- `awk` 匹配规则写错；
    
- 求解器没有收敛；
    
- 输出文件被覆盖；
    
- 文件路径错误。
    

如果保留完整日志，就可以重新检查。

因此推荐：

```text
完整日志负责保留证据
结果 CSV 负责方便分析
```

---

## 52. 使用重定向保存日志

可以将程序输出保存到文件：

```bash
srun ... > "${LOG}" 2>&1
```

其中：

```bash
> "${LOG}"
```

表示：

```text
将标准输出 stdout 写入 LOG 文件
```

而：

```bash
2>&1
```

表示：

```text
将标准错误 stderr 也合并到标准输出
```

最终：

```text
普通输出
错误信息
PETSc log_view 输出
```

都会进入同一个日志文件。

---

# 第十部分：使用 awk 从日志中提取数据

## 53. awk 是什么？

`awk` 是一种常用于文本处理的命令行工具。

它适合处理：

```text
逐行读取文本
    ↓
寻找匹配内容
    ↓
提取某些字段
    ↓
执行简单统计
```

例如，文件：

```text
apple 10
banana 20
orange 30
```

执行：

```bash
awk '{print $1}' file.txt
```

输出第一列：

```text
apple
banana
orange
```

执行：

```bash
awk '{print $2}' file.txt
```

输出第二列：

```text
10
20
30
```

---

## 54. $NF 是什么意思？

在 `awk` 中：

```text
NF
```

表示：

```text
当前行的字段数量
```

因此：

```text
$NF
```

表示：

```text
当前行的最后一个字段
```

例如：

```text
Norm of error 1.23e-08
```

字段为：

|字段编号|内容|
|--:|---|
|`$1`|`Norm`|
|`$2`|`of`|
|`$3`|`error`|
|`$4`|`1.23e-08`|

此时：

```text
NF = 4
```

因此：

```text
$NF = $4 = 1.23e-08
```

---

## 55. 解析 Norm of error

示例：

```bash
ERROR_NORM="$(awk '/Norm of error/{value=$NF} END{print value}' "${LOG}")"
```

逐部分解释：

```bash
awk '规则' "${LOG}"
```

表示：

```text
让 awk 读取日志文件 LOG
```

规则：

```awk
/Norm of error/
```

表示：

```text
只处理包含 Norm of error 的行
```

动作：

```awk
{value=$NF}
```

表示：

```text
将当前行的最后一个字段保存到变量 value
```

最后：

```awk
END{print value}
```

表示：

```text
文件读取结束后，输出最后一次保存的 value
```

完整过程：

```text
逐行读取日志
    ↓
寻找包含 Norm of error 的行
    ↓
取出最后一列
    ↓
读取完成后输出最终值
```

---

## 56. 注意变量名称必须准确

原始示例中使用：

```bash
ITER="$(awk '/Norm of error/{iter=$NF} END{print iter}' "${LOG}")"
```

这段代码可以执行，但是变量命名容易令人误解。

它匹配的是：

```text
Norm of error
```

提取出的通常是误差范数，而不是迭代次数。

更加清晰的命名是：

```bash
ERROR_NORM="$(awk '/Norm of error/{value=$NF} END{print value}' "${LOG}")"
```

如果需要提取迭代次数，应该根据实际日志中的迭代信息编写另一个匹配规则。

基本原则是：

```text
变量名称应该准确表达其中保存的数据
```

否则，后续分析时很容易错误解释结果。

---

## 57. 为什么使用最后一次匹配结果？

代码：

```awk
{value=$NF}
END{print value}
```

不会立即输出每次匹配，而是不断覆盖变量：

```text
第一次匹配：value = 第一次结果
第二次匹配：value = 第二次结果
第三次匹配：value = 第三次结果
```

文件读取结束后：

```awk
END{print value}
```

输出最后一次匹配结果。

这样做适用于日志中可能多次出现同一个关键词，而我们只需要最终值的情况。

---

## 58. 使用 grep 与 awk 的区别

查找某个关键词：

```bash
grep "Norm of error" "${LOG}"
```

提取最后一列：

```bash
awk '/Norm of error/{print $NF}' "${LOG}"
```

提取最后一次匹配的最后一列：

```bash
awk '/Norm of error/{value=$NF} END{print value}' "${LOG}"
```

可以理解为：

|工具|适合的任务|
|---|---|
|`grep`|查找包含某个关键词的行|
|`awk`|查找行并进一步提取字段|
|`sed`|按规则替换或转换文本|

---

# 第十一部分：将实验结果写入 CSV

## 59. 写入 CSV 表头

首先创建结果 CSV，并写入表头：

```bash
printf 'scale,m,n,unknowns,ranks,threads,total_cores,repeat,error_norm\n' \
  > "${RESULT_CSV}"
```

其中：

```text
>
```

表示：

```text
覆盖写入文件
```

如果文件已经存在，旧内容会被清空。

---

## 60. 追加结果行

每一次测试完成后，追加一行：

```bash
printf '%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
  "${SCALE}" \
  "${M}" \
  "${N}" \
  "${UNKNOWNS}" \
  "${R}" \
  "${T}" \
  "${P}" \
  "${REP}" \
  "${ERROR_NORM}" \
  >> "${RESULT_CSV}"
```

其中：

```text
>>
```

表示：

```text
追加到文件末尾
```

区别如下：

|符号|含义|
|---|---|
|`>`|覆盖文件|
|`>>`|在文件末尾追加内容|

---

# 第十二部分：理解嵌套循环

## 61. 完整实验需要几层循环？

一个典型 benchmark 可能包含四层循环：

```text
第一层：问题规模 scale
第二层：MPI ranks R
第三层：OpenMP threads T
第四层：重复实验编号 REP
```

结构如下：

```bash
while IFS=, read -r SCALE M N UNKNOWNS; do
  for R in ${RANKS}; do
    for T in ${THREADS}; do
      for REP in $(seq 1 "${REPEAT}"); do
        echo "Run benchmark"
      done
    done
  done
done < "${SIZE_TABLE}"
```

---

## 62. 一个具体例子

假设：

```text
SCALES  = small medium
RANKS   = 1 2
THREADS = 1 2
REPEAT  = 2
```

暂时忽略非法组合，共有：

$$  
2 \times 2 \times 2 \times 2 = 16  
$$

次运行。

解释：

```text
2 个问题规模
× 2 种 MPI rank 数量
× 2 种 thread 数量
× 每种配置重复 2 次
= 16 次运行
```

如果存在非法组合，则实际运行次数会减少。

---

## 63. 为什么要重复运行？

性能测试通常会受到噪声影响，例如：

- 节点上的系统活动；
    
- 缓存状态；
    
- 文件系统状态；
    
- CPU 频率变化；
    
- 网络通信抖动；
    
- 运行时初始化开销。
    

因此，同一种配置通常需要重复多次：

```bash
REPEAT=3
```

之后可以计算：

- 平均值；
    
- 中位数；
    
- 最小值；
    
- 最大值；
    
- 标准差。
    

---

# 第十三部分：Dry Run

## 64. 什么是 dry run？

`dry run` 表示：

```text
只打印计划执行的命令
但不真正启动程序
```

例如：

```bash
DRY_RUN=1
```

脚本中：

```bash
if [[ "${DRY_RUN}" == "1" ]]; then
  printf 'Would run: %q ' "${CMD[@]}"
  printf '\n'
else
  "${CMD[@]}"
fi
```

Dry run 特别适合检查：

- 参数组合是否正确；
    
- 是否过滤了非法组合；
    
- 文件路径是否正确；
    
- `srun` 参数是否匹配；
    
- 总运行次数是否符合预期。
    

---

## 65. 为什么应该先 dry run？

假设错误地生成了：

```text
1000 个组合
```

如果直接提交作业，可能浪费大量计算资源。

如果先执行：

```bash
DRY_RUN=1 bash tutorial/examples/local_size_grid_practice.sh
```

就可以在本地确认：

```text
到底会运行哪些命令
```

因此推荐顺序：

```text
语法检查
    ↓
dry run
    ↓
检查生成的计划 CSV
    ↓
tiny 本地测试
    ↓
ARCHER2 小规模测试
    ↓
正式 benchmark
```

---

# 第十四部分：WSL 本地练习顺序

## 66. 进入仓库根目录

```bash
cd ~/leyan/jobSkill/petsc-benchmark-suite/s2795693
```

后续命令都从仓库根目录运行。

---

## 67. Exercise 1：检查 Bash 语法

```bash
bash -n tutorial/examples/local_size_grid_practice.sh
```

其中：

```text
-n
```

表示：

```text
只检查语法，不真正执行脚本
```

如果没有任何输出，通常说明 Bash 语法有效。

需要注意：

```text
语法正确
```

不代表：

```text
逻辑一定正确
```

例如，以下代码语法没有问题：

```bash
P=$(( R + T ))
```

但如果目标是计算总核心数，它的逻辑是错误的。

正确写法应为：

```bash
P=$(( R * T ))
```

---

## 68. Exercise 2：生成一个最小执行计划

```bash
bash tutorial/examples/local_size_grid_practice.sh
```

默认情况下，脚本运行在 dry-run 模式。

它应该：

- 打印计划执行的命令；
    
- 生成一个小型 plan CSV；
    
- 不真正启动 PETSc；
    
- 不占用大量计算资源。
    

---

## 69. Exercise 3：覆盖输入参数

运行：

```bash
SCALES="small medium" \
RANKS="1 2" \
THREADS="1 2 4" \
MAX_CORES=4 \
REPEAT=2 \
bash tutorial/examples/local_size_grid_practice.sh
```

在执行前，先手动预测合法组合。

条件：

$$  
P = R \times T \leq 4  
$$

候选组合：

|$R$|$T$|$P = R \times T$|是否合法|
|--:|--:|--:|---|
|1|1|1|合法|
|1|2|2|合法|
|1|4|4|合法|
|2|1|2|合法|
|2|2|4|合法|
|2|4|8|跳过|

因此，每个 scale 有：

```text
5 个合法 rank-thread 组合
```

有两个 scale：

```text
small
medium
```

每个配置重复两次：

```text
REPEAT=2
```

总运行计划数量为：

$$  
2 \times 5 \times 2 = 20  
$$

---

## 70. Exercise 4：追踪 Bash 实际执行过程

```bash
bash -x tutorial/examples/local_size_grid_practice.sh
```

其中：

```text
-x
```

表示：

```text
在执行前打印展开后的命令
```

例如，原始代码：

```bash
P=$(( R * T ))
```

追踪输出中可能显示：

```text
+ P=8
```

`bash -x` 适合检查：

- 循环是否按照预期执行；
    
- 变量是否成功读取；
    
- 条件判断是否正确；
    
- 哪些组合被跳过；
    
- 命令最终展开为什么形式。
    

---

## 71. Exercise 5：可选的本地真实运行

只有当本地 `ex2` 已经可以运行时，才执行：

```bash
DRY_RUN=0 \
EX2_BIN=/absolute/path/to/ex2 \
SCALES="tiny" \
SIZE_TABLE=tutorial/examples/local_problem_sizes.csv \
RANKS="1 2" \
THREADS="1" \
MAX_CORES=2 \
bash tutorial/examples/local_size_grid_practice.sh
```

重点：

```text
只使用 tiny 问题规模
```

不要意外地在 WSL 中运行：

```text
1,000,000 unknowns
```

甚至更大的生产问题。

本地练习的目标是理解脚本，不是测试 ARCHER2 的真实性能。

---

# 第十五部分：一个可以自行重写的最小本地脚本

## 72. 最小可运行版本

下面的脚本不会运行 PETSc，只会生成执行计划。

保存为：

```text
tutorial/examples/my_size_grid_practice.sh
```

内容：

```bash
#!/usr/bin/env bash

set -euo pipefail

SIZE_TABLE="${SIZE_TABLE:-tutorial/examples/local_problem_sizes.csv}"
SCALES="${SCALES:-tiny small}"
RANKS="${RANKS:-1 2 4}"
THREADS="${THREADS:-1 2 4}"
MAX_CORES="${MAX_CORES:-4}"
REPEAT="${REPEAT:-1}"
PLAN_CSV="${PLAN_CSV:-tutorial/examples/my_size_grid_plan.csv}"

printf 'scale,m,n,unknowns,ranks,threads,total_cores,repeat\n' \
  > "${PLAN_CSV}"

scale_selected() {
  local current_scale="$1"

  for selected_scale in ${SCALES}; do
    if [[ "${current_scale}" == "${selected_scale}" ]]; then
      return 0
    fi
  done

  return 1
}

while IFS=, read -r SCALE M N UNKNOWNS; do
  [[ "${SCALE}" == "scale" ]] && continue

  if ! scale_selected "${SCALE}"; then
    continue
  fi

  for R in ${RANKS}; do
    MAX_T=$(( MAX_CORES / R ))

    for T in ${THREADS}; do
      (( T > MAX_T )) && continue

      P=$(( R * T ))

      for REP in $(seq 1 "${REPEAT}"); do
        printf 'scale=%s, m=%s, n=%s, ranks=%s, threads=%s, cores=%s, repeat=%s\n' \
          "${SCALE}" "${M}" "${N}" "${R}" "${T}" "${P}" "${REP}"

        printf '%s,%s,%s,%s,%s,%s,%s,%s\n' \
          "${SCALE}" "${M}" "${N}" "${UNKNOWNS}" \
          "${R}" "${T}" "${P}" "${REP}" \
          >> "${PLAN_CSV}"
      done
    done
  done
done < "${SIZE_TABLE}"

printf 'Plan written to %s\n' "${PLAN_CSV}"
```

---

## 73. 最小本地 CSV

保存为：

```text
tutorial/examples/local_problem_sizes.csv
```

内容：

```csv
scale,m,n,unknowns
tiny,8,8,64
small,16,16,256
medium,32,32,1024
```

---

## 74. 运行最小脚本

首先检查语法：

```bash
bash -n tutorial/examples/my_size_grid_practice.sh
```

然后运行：

```bash
bash tutorial/examples/my_size_grid_practice.sh
```

覆盖参数：

```bash
SCALES="tiny medium" \
RANKS="1 2" \
THREADS="1 2 4" \
MAX_CORES=4 \
REPEAT=2 \
bash tutorial/examples/my_size_grid_practice.sh
```

查看计划 CSV：

```bash
cat tutorial/examples/my_size_grid_plan.csv
```

---

# 第十六部分：从本地脚本替换为 ARCHER2 脚本

## 75. 本地脚本缺少什么？

最小本地脚本只负责：

```text
读取输入
筛选 scale
生成合法组合
打印计划
写入 CSV
```

生产脚本还需要增加：

1. `#SBATCH` 资源申请；
    
2. ARCHER2 module 环境；
    
3. PETSc 程序路径；
    
4. 日志目录；
    
5. `OMP_NUM_THREADS`；
    
6. `srun`；
    
7. 日志重定向；
    
8. `awk` 解析；
    
9. 结果 CSV；
    
10. 失败检查。
    

---

## 76. 将打印命令替换为 srun

本地计划脚本中可能有：

```bash
printf 'Would run R=%s T=%s P=%s\n' "${R}" "${T}" "${P}"
```

在 ARCHER2 中，可以逐步替换为：

```bash
export OMP_NUM_THREADS="${T}"

srun \
  --nodes=1 \
  --ntasks="${R}" \
  --cpus-per-task="${T}" \
  --exact \
  "${EX2_BIN}" \
  -m "${M}" \
  -n "${N}" \
  -ksp_converged_reason \
  -log_view \
  > "${LOG}" 2>&1
```

---

## 77. 一个生产脚本的逻辑骨架

下面的代码用于理解整体结构，不应在不了解项目路径时直接提交：

```bash
#!/usr/bin/env bash

#SBATCH --nodes=1
#SBATCH --time=06:00:00
#SBATCH --output=%x.%j.out

set -euo pipefail

SIZE_TABLE="${SIZE_TABLE:-scripts/ex2_problem_sizes.csv}"
SCALES="${SCALES:-small}"
RANKS="${RANKS:-1 2 4 8}"
THREADS="${THREADS:-1 2 4 8}"
MAX_CORES="${MAX_CORES:-8}"
REPEAT="${REPEAT:-1}"

EX2_BIN="${EX2_BIN:-/path/to/ex2}"
LOG_DIR="${LOG_DIR:-logs}"
RESULT_CSV="${RESULT_CSV:-results/ex2_size_grid.csv}"

mkdir -p "${LOG_DIR}"
mkdir -p "$(dirname "${RESULT_CSV}")"

printf 'scale,m,n,unknowns,ranks,threads,total_cores,repeat,error_norm\n' \
  > "${RESULT_CSV}"

while IFS=, read -r SCALE M N UNKNOWNS; do
  [[ "${SCALE}" == "scale" ]] && continue

  for R in ${RANKS}; do
    MAX_T=$(( MAX_CORES / R ))

    for T in ${THREADS}; do
      (( T > MAX_T )) && continue

      P=$(( R * T ))

      for REP in $(seq 1 "${REPEAT}"); do
        LOG="${LOG_DIR}/${SCALE}_r${R}_t${T}_rep${REP}.log"

        export OMP_NUM_THREADS="${T}"

        srun \
          --nodes=1 \
          --ntasks="${R}" \
          --cpus-per-task="${T}" \
          --exact \
          "${EX2_BIN}" \
          -m "${M}" \
          -n "${N}" \
          -ksp_converged_reason \
          -log_view \
          > "${LOG}" 2>&1

        ERROR_NORM="$(
          awk '/Norm of error/{value=$NF} END{print value}' "${LOG}"
        )"

        printf '%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
          "${SCALE}" "${M}" "${N}" "${UNKNOWNS}" \
          "${R}" "${T}" "${P}" "${REP}" "${ERROR_NORM}" \
          >> "${RESULT_CSV}"
      done
    done
  done
done < "${SIZE_TABLE}"

printf 'Completed. Results written to %s\n' "${RESULT_CSV}"
```

---

# 第十七部分：阅读生产脚本的推荐顺序

## 78. 不要从第一行机械地读到最后一行

阅读：

```text
scripts/run_ex2_size_grid.sbatch
```

时，推荐按照以下顺序标注。

### 78.1 资源申请

寻找：

```bash
#SBATCH ...
```

需要回答：

```text
申请了多少节点？
最长运行时间是多少？
Slurm 输出写到哪里？
```

---

### 78.2 失败处理

寻找：

```bash
set -euo pipefail
```

需要回答：

```text
脚本在什么情况下会立即停止？
为什么 benchmark 不应该静默失败？
```

---

### 78.3 ARCHER2 环境

寻找：

```bash
module ...
```

以及：

```bash
PETSC_DIR=...
PETSC_ARCH=...
EX2_BIN=...
```

需要回答：

```text
程序从哪里加载？
ex2 的真实路径是什么？
依赖哪些 module？
```

---

### 78.4 用户可控参数

寻找：

```bash
SCALES="${SCALES:-...}"
REPEAT="${REPEAT:-...}"
RANKS="${RANKS:-...}"
THREADS="${THREADS:-...}"
```

需要回答：

```text
默认测试哪些规模？
默认重复几次？
如何临时覆盖默认值？
```

---

### 78.5 输出目录与 CSV 表头

寻找：

```bash
mkdir -p ...
printf '...' > "${RESULT_CSV}"
```

需要回答：

```text
日志保存在哪里？
汇总结果保存在哪里？
CSV 中有哪些列？
```

---

### 78.6 问题规模循环

寻找：

```bash
while IFS=, read -r SCALE M N UNKNOWNS; do
```

需要回答：

```text
CSV 的每一列分别进入哪个变量？
如何跳过 header？
如何筛选 scale？
```

---

### 78.7 rank-thread 参数循环

寻找：

```bash
for R in ...
for T in ...
MAX_T=$(( MAX_CORES / R ))
(( T > MAX_T )) && continue
P=$(( R * T ))
```

需要回答：

```text
为什么部分组合必须跳过？
P 代表什么？
MAX_T 是如何推导出来的？
```

---

### 78.8 srun 启动

寻找：

```bash
export OMP_NUM_THREADS="${T}"

srun \
  --ntasks="${R}" \
  --cpus-per-task="${T}" \
  ...
```

需要回答：

```text
MPI ranks 如何映射到 Slurm 参数？
OpenMP threads 如何映射到环境变量？
为什么 --cpus-per-task 与 OMP_NUM_THREADS 应保持一致？
```

---

### 78.9 awk 解析

寻找：

```bash
awk ...
```

需要回答：

```text
脚本从日志中提取了哪些字段？
匹配了哪些关键词？
$NF 表示什么？
提取的是最后一次匹配还是第一次匹配？
```

---

### 78.10 写入结果与最终检查

寻找：

```bash
printf ... >> "${RESULT_CSV}"
```

以及脚本末尾的提示信息。

需要回答：

```text
每一次运行写入几行？
是否保留原始日志？
是否检查结果字段为空？
```

---

# 第十八部分：调试方法

## 79. 使用 bash -n 检查语法

```bash
bash -n script.sh
```

适合发现：

- 缺少 `done`；
    
- 缺少 `fi`；
    
- 引号没有关闭；
    
- 括号没有关闭；
    
- Bash 语法拼写错误。
    

---

## 80. 使用 bash -x 查看变量展开

```bash
bash -x script.sh
```

适合发现：

- 变量为空；
    
- 条件判断错误；
    
- 循环次数错误；
    
- 文件路径错误；
    
- 命令实际展开结果错误。
    

---

## 81. 使用 echo 或 printf 插入检查点

例如：

```bash
printf 'DEBUG: SCALE=%s R=%s T=%s P=%s\n' \
  "${SCALE}" "${R}" "${T}" "${P}"
```

可以帮助确认脚本当前运行到哪里。

正式运行前，可以保留必要的进度信息，但不应输出过多无用内容。

---

## 82. 检查生成的 CSV

查看前几行：

```bash
head "${RESULT_CSV}"
```

查看最后几行：

```bash
tail "${RESULT_CSV}"
```

统计总行数：

```bash
wc -l "${RESULT_CSV}"
```

如果 CSV 包含一行 header，则：

$$  
\text{实验运行次数} = \text{CSV 总行数} - 1  
$$

---

## 83. 检查日志文件

列出日志：

```bash
ls -lh logs/
```

查找错误：

```bash
grep -i "error" logs/*.log
```

查找误差范数：

```bash
grep "Norm of error" logs/*.log
```

---

# 第十九部分：术语表

|术语|英文全称或形式|含义|
|---|---|---|
|CSV|Comma-Separated Values|使用逗号分隔字段的文本表格格式|
|Header|Header Row|CSV 文件中的表头|
|Bash|Bourne Again Shell|常见的 Shell 脚本语言|
|Shell Script|Shell Script|由命令和控制结构组成的自动化脚本|
|Environment Variable|Environment Variable|可以传递给脚本或程序的外部变量|
|Default Value|Default Value|用户未指定参数时使用的默认设置|
|Override|Override|使用外部输入覆盖脚本中的默认设置|
|IFS|Internal Field Separator|Bash 中用于切分字段的分隔符|
|Input Redirection|`<`|将文件内容作为命令输入|
|Output Redirection|`>`|覆盖写入文件|
|Append Redirection|`>>`|在文件末尾追加内容|
|stderr Merge|`2>&1`|将标准错误合并到标准输出|
|MPI Rank|MPI Rank|一个 MPI 进程|
|OpenMP Thread|OpenMP Thread|MPI 进程内部的共享内存线程|
|Hybrid Parallelism|MPI + OpenMP|同时使用 MPI 与 OpenMP 的混合并行|
|Oversubscription|Oversubscription|创建的线程数量超过实际分配的 CPU cores|
|Slurm|Simple Linux Utility for Resource Management|HPC 集群常用的作业调度系统|
|sbatch|`sbatch`|向 Slurm 提交批处理作业|
|srun|`srun`|在 Slurm 分配的资源中启动程序|
|SBATCH Directive|`#SBATCH`|由 `sbatch` 解析的资源申请指令|
|Dry Run|Dry Run|只生成或打印命令，不真正执行程序|
|Log|Log File|保存一次运行完整输出的日志文件|
|awk|`awk`|面向文本字段的命令行处理工具|
|`$NF`|Last Field in awk|当前行的最后一个字段|
|`continue`|`continue`|跳过当前循环的剩余部分|
|`set -e`|Exit on Error|命令失败后停止脚本|
|`set -u`|Error on Unset Variable|使用未定义变量时停止脚本|
|`pipefail`|Pipeline Failure Detection|管道中任意命令失败时，将整个管道视为失败|

---

# 第二十部分：核心结论

## 84. CSV 负责描述输入，Bash 负责执行逻辑

推荐将问题规模单独写在：

```text
scripts/ex2_problem_sizes.csv
```

脚本只负责：

```text
读取
筛选
遍历
运行
记录
```

这样可以减少硬编码。

---

## 85. 总核心数由 MPI ranks 与 threads 共同决定

核心公式为：

$$  
P = R \times T  
$$

其中：

```text
R = MPI ranks
T = 每个 rank 的 OpenMP threads
P = 总 CPU cores
```

所有合法组合必须满足：

$$  
R \times T \leq \text{MAX_CORES}  
$$

---

## 86. Slurm 资源与 OpenMP 设置需要匹配

当：

```bash
OMP_NUM_THREADS="${T}"
```

时，通常也应该设置：

```bash
--cpus-per-task="${T}"
```

否则可能出现资源浪费或 oversubscription。

---

## 87. 本地练习与 ARCHER2 测试需要分开

WSL 可以练习：

```text
CSV
Bash 循环
参数过滤
dry run
bash -n
bash -x
```

ARCHER2 才能测试：

```text
#SBATCH
module
srun
节点资源分配
真实 benchmark 性能
```

---

## 88. 原始日志与结果 CSV 都需要保留

日志负责：

```text
排查错误
保留完整证据
重新解析结果
```

CSV 负责：

```text
汇总关键指标
后续统计
绘图
对比实验结果
```

两者不能互相替代。

---

# 第二十一部分：常见误区

## 89. 将 CSV 表头当成数据执行

错误：

```bash
while IFS=, read -r SCALE M N UNKNOWNS; do
  echo "${SCALE}"
done < "${SIZE_TABLE}"
```

这会把：

```text
scale
```

也当作问题规模。

正确做法：

```bash
[[ "${SCALE}" == "scale" ]] && continue
```

---

## 90. 错误地将总核心数写成加法

错误：

```bash
P=$(( R + T ))
```

正确：

```bash
P=$(( R * T ))
```

因为每个 rank 都拥有 `T` 个线程。

---

## 91. 只设置 OMP_NUM_THREADS，不申请对应 cores

错误：

```bash
export OMP_NUM_THREADS=4

srun --ntasks=2 ...
```

缺少：

```bash
--cpus-per-task=4
```

可能导致线程争抢 CPU cores。

---

## 92. 只申请 cpus-per-task，不设置 OMP_NUM_THREADS

错误：

```bash
srun --ntasks=2 --cpus-per-task=4 ...
```

如果 OpenMP runtime 默认只创建一个线程，额外分配的 CPU cores 可能不会被使用。

---

## 93. 在 WSL 中直接运行生产规模

本地学习时，应优先使用：

```text
tiny
```

例如：

```text
8 × 8
16 × 16
32 × 32
```

不要意外运行 ARCHER2 生产规模。

---

## 94. 认为 bash -n 可以检查逻辑正确性

```bash
bash -n script.sh
```

只能检查语法。

它不能发现：

```bash
P=$(( R + T ))
```

这种逻辑错误。

需要结合：

```bash
bash -x script.sh
```

以及手动验证输出。

---

## 95. 使用含义错误的变量名称

如果提取的是：

```text
Norm of error
```

不应将变量命名为：

```bash
ITER
```

更加合理的是：

```bash
ERROR_NORM
```

变量名称必须帮助读者理解数据含义。

---

## 96. 只保存汇总 CSV，不保存日志

汇总 CSV 适合绘图，但不足以排查错误。

每次运行都应保留：

```text
完整日志
```

否则出现异常数据时，很难判断原因。

---

# 第二十二部分：自测问题

## 97. CSV 与 Bash 读取

1. CSV 是什么？为什么适合保存 benchmark 输入？
    
2. `IFS=,` 的作用是什么？
    
3. `read -r scale m n unknowns` 中的四个变量如何获得数据？
    
4. `done < file.csv` 的作用是什么？
    
5. 为什么需要跳过 CSV header？
    
6. `continue` 会对循环产生什么影响？
    

---

## 98. 默认值与环境变量

1. `${SCALES:-small}` 是什么意思？
    
2. 下面两条命令有什么区别？
    

```bash
bash script.sh
```

```bash
SCALES="small medium" bash script.sh
```

3. 命令前面的环境变量赋值是否会永久修改当前 Shell？
    
4. 如何将 `REPEAT` 临时设置为 `3`？
    

---

## 99. rank-thread 参数组合

1. 如果 `R=4`、`T=2`，总核心数是多少？
    
2. 如果 `MAX_CORES=8`、`R=4`，最大合法线程数是多少？
    
3. 为什么 `R=4`、`T=4` 在 `MAX_CORES=8` 时不合法？
    
4. `MAX_T=$(( MAX_CORES / R ))` 对应的数学推导是什么？
    
5. 为什么对比混合并行配置时，通常应保持总核心数一致？
    

---

## 100. Slurm 与 OpenMP

1. `--ntasks` 对应什么概念？
    
2. `--cpus-per-task` 对应什么概念？
    
3. `OMP_NUM_THREADS` 对应什么概念？
    
4. 为什么 `--cpus-per-task` 和 `OMP_NUM_THREADS` 通常应该相等？
    
5. 什么是 oversubscription？
    
6. WSL 中的 `mpirun` 与 ARCHER2 中的 `srun` 在概念上有什么对应关系？
    

---

## 101. 日志与文本解析

1. 为什么应该同时保存 raw log 和 summary CSV？
    
2. `>` 与 `>>` 有什么区别？
    
3. `2>&1` 的作用是什么？
    
4. `awk` 中的 `$NF` 表示什么？
    
5. 下面命令提取的是什么？
    

```bash
awk '/Norm of error/{value=$NF} END{print value}' "${LOG}"
```

6. 为什么将上述结果命名为 `ITER` 可能不够准确？
    

---

## 102. 综合练习

假设：

```text
SCALES  = small medium
RANKS   = 1 2 4
THREADS = 1 2 4
MAX_CORES = 4
REPEAT = 3
```

回答以下问题：

1. 每个 scale 有哪些合法的 rank-thread 组合？
    
2. 每个 scale 有多少个合法组合？
    
3. 两个 scale 总共需要执行多少次 benchmark？
    
4. 如果 CSV 中包含一行 header，最终结果 CSV 应该有多少行？
    
5. 如果只想测试 `medium`，提交命令应该如何写？
    

参考答案：

合法组合为：

|$R$|$T$|$P$|
|--:|--:|--:|
|1|1|1|
|1|2|2|
|1|4|4|
|2|1|2|
|2|2|4|
|4|1|4|

每个 scale 有：

$$  
6  
$$

个合法组合。

总 benchmark 次数为：

$$  
2 \times 6 \times 3 = 36  
$$

加上一行 CSV header 后，总行数为：

$$  
36 + 1 = 37  
$$

只测试 `medium`：

```bash
SCALES="medium" \
sbatch scripts/run_ex2_size_grid.sbatch
```

---

# 第二十三部分：最终检查清单

在尝试自己重写脚本前，确认能够独立解释以下内容：

-  CSV 文件中每一列的含义；
    
-  `IFS=, read -r` 如何读取一行 CSV；
    
-  为什么必须跳过 header；
    
-  `${VARIABLE:-default}` 如何提供默认值；
    
-  如何通过命令前的环境变量覆盖默认设置；
    
-  Bash 数组和 `for` 循环如何工作；
    
-  总核心数公式 $P = R \times T$；
    
-  `MAX_T=$(( MAX_CORES / R ))` 的推导；
    
-  `continue` 如何跳过非法组合；
    
-  `OMP_NUM_THREADS`、`--ntasks` 与 `--cpus-per-task` 的对应关系；
    
-  `#SBATCH` 指令由谁读取；
    
-  为什么 WSL 与 ARCHER2 的测试范围不同；
    
-  `set -euo pipefail` 的作用；
    
-  为什么日志和汇总 CSV 需要同时保留；
    
-  `awk` 如何从日志中提取最后一列；
    
-  如何使用 `bash -n` 和 `bash -x` 调试脚本。
    

当这些内容都可以用自己的语言解释后，就可以开始不参考原脚本，重写一个简化版本的 `run_ex2_size_grid.sbatch`。