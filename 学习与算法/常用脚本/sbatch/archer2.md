# Bash、CSV 与 Slurm 脚本学习笔记：从问题到可运行 Benchmark

## 0. 这份笔记要解决什么问题？

我现在学习的脚本不是普通的“打印 Hello World”，而是一个用于 HPC benchmark 的自动化脚本。

它的核心目标是：

```text
自动测试不同问题规模、不同 MPI ranks、不同 OpenMP threads 下 PETSc ex2 程序的性能
````

也就是说，脚本要帮我完成这些重复劳动：

```text
读取问题规模 CSV
    ↓
筛选要运行的 scale
    ↓
遍历 MPI ranks
    ↓
遍历 OpenMP threads
    ↓
过滤非法核心数组合
    ↓
启动 PETSc ex2
    ↓
保存每次运行的日志
    ↓
从日志中提取性能数据
    ↓
写入结果 CSV
```


---

# Part 1：这个脚本到底在自动化什么？

## 1. 为什么需要脚本？

如果我只测试一次 PETSc ex2，命令可能长这样：

```bash
export OMP_NUM_THREADS=4

srun --nodes=1 \
     --ntasks=2 \
     --cpus-per-task=4 \
     --exact \
     /path/to/ex2 \
     -m 1000 \
     -n 1000 \
     -ksp_converged_reason \
     -log_view
```

这表示：

```text
运行一个 1000 × 1000 的问题
启动 2 个 MPI ranks
每个 rank 使用 4 个 OpenMP threads
总共使用 8 个 CPU cores
```

如果只有一次运行，可以手动输入。

但 benchmark 通常不是一次运行，而是很多组合：

```text
scale = small, medium, large
ranks = 1, 2, 4, 8, 16, 32, 64, 128
threads = 1, 2, 4, 8, 16, 32, 64, 128
repeat = 1, 2, 3, ...
```

手动运行会有几个问题：

- 容易漏掉某些组合
    
- 容易把参数复制错
    
- 不方便重复实验
    
- 不方便保存日志
    
- 不方便生成统一 CSV
    
- 很难保证实验可复现
    

所以脚本的真正作用是：

```text
把“重复、机械、容易出错”的 benchmark 流程自动化
```

---

## 2. 这个脚本可以拆成哪几层？

当前脚本可以分成 8 层：

```text
第 1 层：Slurm 资源申请
第 2 层：安全模式与环境加载
第 3 层：路径与变量配置
第 4 层：读取用户可覆盖参数
第 5 层：读取 CSV 问题规模
第 6 层：生成合法 ranks × threads 组合
第 7 层：运行 PETSc ex2 并保存日志
第 8 层：解析日志并写入结果 CSV
```

学习顺序应该是：

```text
先学 Bash 基础
    ↓
再学 CSV 读取
    ↓
再学循环和参数组合
    ↓
再学 Slurm 资源映射
    ↓
最后再看完整生产脚本
```

---

# Part 2：脚本开头为什么要这样写？

## 3. `#!/usr/bin/env bash` 是什么？

脚本第一行是：

```bash
#!/usr/bin/env bash
```

这叫 shebang。

它告诉系统：

```text
请使用 bash 来解释执行这个脚本
```

为什么不用：

```bash
#!/bin/bash
```

而用：

```bash
#!/usr/bin/env bash
```

因为 `env` 会从当前环境的 `PATH` 里寻找 `bash`，可移植性更好。

简单理解：

```text
#!/usr/bin/env bash
= 用当前系统能找到的 bash 来运行这个脚本
```

---

## 4. `#SBATCH` 为什么看起来像注释，但又有用？

脚本中有很多行：

```bash
#SBATCH --account=m25oc-s2795693
#SBATCH --partition=standard
#SBATCH --qos=standard
#SBATCH --job-name=petsc_ex2_size_grid
#SBATCH --output=%x.%j.out
#SBATCH --error=%x.%j.err
#SBATCH --nodes=1
#SBATCH --time=06:00:00
#SBATCH --hint=nomultithread
#SBATCH --distribution=block:block
```

对于普通 Bash 来说，`#` 开头确实是注释。

但是如果脚本通过 `sbatch` 提交：

```bash
sbatch scripts/run_ex2_size_grid.sbatch
```

Slurm 会读取这些特殊注释。

所以：

```text
直接 bash script.sbatch：
    #SBATCH 被 Bash 当作普通注释

使用 sbatch script.sbatch：
    #SBATCH 被 Slurm 当作资源申请指令
```

---

## 5. 常见 SBATCH 参数是什么意思？

|参数|含义|
|---|---|
|`--account=m25oc-s2795693`|使用哪个项目账户计费|
|`--partition=standard`|提交到 standard 分区|
|`--qos=standard`|使用 standard 服务质量|
|`--job-name=petsc_ex2_size_grid`|作业名称|
|`--output=%x.%j.out`|标准输出文件|
|`--error=%x.%j.err`|标准错误文件|
|`--nodes=1`|申请 1 个节点|
|`--time=06:00:00`|最长运行 6 小时|
|`--hint=nomultithread`|倾向使用物理核心，不使用硬件超线程|
|`--distribution=block:block`|控制任务和核心的分布方式|

其中：

```text
%x = job name
%j = job ID
```

所以：

```bash
#SBATCH --output=%x.%j.out
```

可能生成：

```text
petsc_ex2_size_grid.123456.out
```

---

## 6. `##SBATCH --exclusive` 为什么前面有两个 `#`？

脚本中有：

```bash
##SBATCH --exclusive
```

这不是有效的 Slurm 指令。

因为 Slurm 只识别：

```bash
#SBATCH
```

不识别：

```bash
##SBATCH
```

所以这行相当于被注释掉了。

它的作用是：

```text
暂时保留这个选项，但当前不启用
```

如果改成：

```bash
#SBATCH --exclusive
```

意思就是：

```text
独占整个节点
```

这在正式 benchmark 中可能更稳定，但也会占用更多资源。

---

# Part 3：为什么脚本要开启安全模式？

## 7. `set -euo pipefail` 是什么？

脚本中有：

```bash
set -euo pipefail
```

这是 Bash 脚本中非常重要的安全设置。

它由三部分组成：

```text
-e
-u
-o pipefail
```

---

## 8. `set -e`：命令失败就停止

```bash
set -e
```

表示：

```text
只要某条命令失败，脚本就立刻停止
```

例如：

```bash
set -e

cp missing_file.txt output.txt
echo "Finished"
```

如果 `missing_file.txt` 不存在，`cp` 会失败。

此时：

```bash
echo "Finished"
```

不会继续执行。

在 benchmark 脚本中，这很重要。

因为如果 PETSc 已经运行失败，脚本就不应该继续写入一行看似正常的 CSV。

---

## 9. `set -u`：使用未定义变量就停止

```bash
set -u
```

表示：

```text
如果读取了一个没有定义过的变量，脚本直接报错
```

例如：

```bash
set -u

echo "${RESULT_DIR}"
```

如果 `RESULT_DIR` 没定义，脚本会停止。

这可以防止变量拼写错误。

例如我本来想写：

```bash
"${THREADS}"
```

但误写成：

```bash
"${THREDS}"
```

没有 `set -u` 时，它可能被当成空字符串。

有 `set -u` 时，它会立刻报错。

---

## 10. `pipefail`：管道中任何一步失败都算失败

普通管道：

```bash
command_a | command_b
```

默认情况下，Bash 往往只看最后一个命令 `command_b` 是否成功。

但是 benchmark 脚本中常有：

```bash
srun ... | tee "${LOG}"
```

如果 `srun` 失败，但 `tee` 成功写了文件，整个管道可能看起来像成功。

所以需要：

```bash
set -o pipefail
```

这样：

```text
管道中任意一步失败，整个管道都算失败
```

---

## 11. 为什么 benchmark 脚本特别需要安全模式？

因为 benchmark 最怕的不是直接报错，而是：

```text
程序失败了
    ↓
脚本继续跑
    ↓
日志不完整
    ↓
CSV 里写入空值或错误值
    ↓
后续画图时以为结果是真的
```

所以：

```bash
set -euo pipefail
```

可以理解为：

```text
宁可早停，也不要生成脏数据
```

---

# Part 4：路径和目录为什么要单独设置？

## 12. 为什么先设置 `PROJECT_ROOT`？

脚本中有：

```bash
PROJECT_ROOT="/work/m25oc/m25oc/s2795693/petsc_omp"
cd "${PROJECT_ROOT}"
```

意思是：

```text
明确项目根目录
然后进入项目根目录
```

这样后面的路径都可以基于项目根目录构造。

如果不这么做，脚本从不同目录提交时，可能会找不到文件。

例如：

```bash
sbatch scripts/run_ex2_size_grid.sbatch
```

和：

```bash
cd scripts
sbatch run_ex2_size_grid.sbatch
```

当前工作目录可能不同。

所以正式脚本最好先固定：

```bash
cd "${PROJECT_ROOT}"
```

---

## 13. 为什么变量展开要写成 `"${PROJECT_ROOT}"`？

推荐写法：

```bash
cd "${PROJECT_ROOT}"
```

不推荐：

```bash
cd $PROJECT_ROOT
```

原因是 Bash 会对未加引号的变量进行 word splitting 和 glob expansion。

例如：

```bash
PROJECT_ROOT="/some/path with space/project"
```

如果写：

```bash
cd $PROJECT_ROOT
```

Bash 可能把它拆成多个参数：

```text
/some/path
with
space/project
```

如果写：

```bash
cd "${PROJECT_ROOT}"
```

它会被保留为一个完整路径。

结论：

```text
普通变量展开，默认使用 "${variable}"
```

---

## 14. 为什么要创建 RUN_ROOT 和 RES_DIR？

脚本中有：

```bash
RUN_ROOT="${PROJECT_ROOT}/runs/size_grid"
RES_DIR="${PROJECT_ROOT}/results/size_grid"
mkdir -p "${RUN_ROOT}" "${RES_DIR}"
```

含义是：

```text
RUN_ROOT：保存每次运行的原始日志
RES_DIR：保存汇总后的结果 CSV
```

为什么要分开？

```text
日志文件：
    适合排查错误
    保存完整输出
    文件较大

结果 CSV：
    适合统计
    适合绘图
    文件较小
```

不要只保存 CSV。

因为 CSV 只保留最终提取出来的数据，一旦某一行异常，就需要回到原始日志查原因。

---

# Part 5：默认值与外部覆盖

## 15. `${SCALES:-small}` 是什么？

脚本中有：

```bash
SCALES="${SCALES:-small}"
REPEAT="${REPEAT:-1}"
MAX_CORES="${MAX_CORES:-128}"
RANKS="${RANKS:-1 2 4 8 16 32 64 128}"
THREADS="${THREADS:-1 2 4 8 16 32 64 128}"
```

其中：

```bash
${SCALES:-small}
```

意思是：

```text
如果 SCALES 已经存在且非空，就使用原来的值
否则使用 small
```

所以：

```bash
SCALES="${SCALES:-small}"
```

可以拆成两步理解：

```text
第一步：
    ${SCALES:-small} 决定应该展开成什么文本

第二步：
    SCALES="..." 把这个文本赋值给 SCALES
```

注意：

```text
:- 不是赋值
= 才是赋值
```

---

## 16. 为什么要允许外部覆盖？

默认运行：

```bash
sbatch scripts/run_ex2_size_grid.sbatch
```

此时：

```text
SCALES = small
REPEAT = 1
```

如果想运行更大的规模，可以提交时覆盖：

```bash
SCALES="large very-large" \
REPEAT=3 \
sbatch scripts/run_ex2_size_grid.sbatch
```

这表示：

```text
只为这次 sbatch 命令临时设置 SCALES 和 REPEAT
```

这样不用修改脚本本身。

这对 benchmark 很重要，因为不同实验可以复用同一份脚本，只改变外部变量。

---

## 17. `${x:-v}`、`${x-v}`、`${x:=v}` 有什么区别？

|写法|含义|
|---|---|
|`${x:-v}`|如果 `x` 未设置或为空，使用 `v`|
|`${x-v}`|只有 `x` 未设置时，使用 `v`；如果 `x=""`，保留空字符串|
|`${x:=v}`|如果 `x` 未设置或为空，把 `v` 赋值给 `x`，然后使用它|

示例：

```
x=""
printf '<%s>\n' "${x:-fallback}"
```


输出：

```text
<fallback>
<>
```

在学习和写 benchmark 脚本时，通常优先使用：

```bash
${x:-default}
```

因为空字符串往往也代表没有提供有效输入。

---

# Part 6：为什么要把字符串转成数组？

## 18. `RANKS="1 2 4 8"` 是字符串，不是数组

脚本中：

```bash
RANKS="${RANKS:-1 2 4 8 16 32 64 128}"
THREADS="${THREADS:-1 2 4 8 16 32 64 128}"
```

这里的 `RANKS` 和 `THREADS` 本质上是字符串：

```text
"1 2 4 8 16 32 64 128"
```

为了后面更清楚地遍历，脚本把它们转成数组：

```bash
read -r -a RANKS_LIST <<< "${RANKS}"
read -r -a THREADS_LIST <<< "${THREADS}"
```

---

## 19. `read -r -a RANKS_LIST <<< "${RANKS}"` 怎么理解？

这句可以拆成三部分：

```bash
read -r -a RANKS_LIST <<< "${RANKS}"
```

第一部分：

```bash
read
```

表示从输入中读取内容。

第二部分：

```bash
-r
```

表示不要把反斜杠当成转义符。

第三部分：

```bash
-a RANKS_LIST
```

表示读入数组 `RANKS_LIST`。

第四部分：

```bash
<<< "${RANKS}"
```

叫 here-string，意思是：

```text
把右边这段字符串当作输入喂给 read
```

如果：

```bash
RANKS="1 2 4 8"
```

执行：

```bash
read -r -a RANKS_LIST <<< "${RANKS}"
```

就得到：

```bash
RANKS_LIST[0]="1"
RANKS_LIST[1]="2"
RANKS_LIST[2]="4"
RANKS_LIST[3]="8"
```

后面就可以写：

```bash
for R in "${RANKS_LIST[@]}"; do
  echo "${R}"
done
```

---

## 20. 为什么不直接 `for R in ${RANKS}`？

这样也能跑：

```bash
for R in ${RANKS}; do
  echo "${R}"
done
```

但转成数组更清楚：

```text
RANKS：
    用户传入的原始字符串

RANKS_LIST：
    Bash 内部使用的可遍历数组
```

这让脚本结构更明确，也更适合后续扩展。

---

# Part 7：CSV 文件到底负责什么？

## 21. 什么是 CSV？

CSV 是 Comma-Separated Values，意思是逗号分隔值。

例如：

```csv
scale,m,n,unknowns
small,1000,1000,1000000
medium,2000,2000,4000000
large,4000,4000,16000000
```

第一行是 header，也就是表头：

```csv
scale,m,n,unknowns
```

后面每一行是一条数据。

---

## 22. PETSc ex2 的问题规模怎么表示？

脚本读取的是：

```bash
SIZE_TABLE="${SIZE_TABLE:-${PROJECT_ROOT}/scripts/ex2_problem_sizes.csv}"
```

CSV 中每一行类似：

```csv
small,1000,1000,1000000
```

四个字段分别是：

|字段|含义|
|---|---|
|`scale`|问题规模名称|
|`m`|二维网格第一个维度|
|`n`|二维网格第二个维度|
|`unknowns`|未知量总数|

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

所以：

$$  
\text{unknowns} = 1000 \times 1000 = 1{,}000{,}000  
$$

---

## 23. 为什么不直接把 M 和 N 写死在脚本里？

如果只测试一个规模，可以写：

```bash
M=1000
N=1000
```

但如果有很多规模，写死会变得很乱：

```bash
M1=1000
N1=1000

M2=2000
N2=2000

M3=4000
N3=4000
```

使用 CSV 后，结构变成：

```text
CSV 文件：
    只负责描述实验输入

Bash 脚本：
    只负责读取输入并执行实验
```

优点是：

- 添加规模时只改 CSV
    
- 不需要改主脚本
    
- 实验配置更清晰
    
- 更容易复现实验
    
- 更容易检查输入是否正确
    

---

# Part 8：Bash 如何逐行读取 CSV？

## 24. `while IFS=, read -r SCALE M N UNKNOWNS; do` 是什么？

脚本中核心结构是：

```bash
while IFS=, read -r SCALE M N UNKNOWNS; do
  ...
done < "${SIZE_TABLE}"
```

它的意思是：

```text
从 SIZE_TABLE 文件中逐行读取
每一行用逗号分隔
分别放入 SCALE、M、N、UNKNOWNS
```

例如读取：

```csv
small,1000,1000,1000000
```

会得到：

```text
SCALE    = small
M        = 1000
N        = 1000
UNKNOWNS = 1000000
```

---

## 25. `IFS=,` 是什么？

`IFS` 是 Internal Field Separator。

它决定 Bash 用什么符号切分字段。

默认情况下，Bash 按空格、制表符等切分。

但 CSV 是逗号分隔，所以这里写：

```bash
IFS=,
```

完整写法：

```bash
IFS=, read -r SCALE M N UNKNOWNS
```

意思是：

```text
本次 read 使用逗号作为字段分隔符
```

---

## 26. `done < "${SIZE_TABLE}"` 是什么？

这是输入重定向。

```bash
done < "${SIZE_TABLE}"
```

意思是：

```text
把 SIZE_TABLE 文件内容喂给整个 while 循环
```

可以理解为：

```text
打开 CSV 文件
    ↓
每次读取一行
    ↓
read 拆成多个变量
    ↓
执行循环体
    ↓
继续读取下一行
```

---

## 27. 为什么要跳过表头？

CSV 第一行是：

```csv
scale,m,n,unknowns
```

如果不跳过，脚本会把它当成真实问题规模。

所以有：

```bash
[[ "${SCALE}" == "scale" ]] && continue
```

等价于：

```bash
if [[ "${SCALE}" == "scale" ]]; then
  continue
fi
```

意思是：

```text
如果当前行是表头，就跳过这一轮循环
```

---

## 28. `continue` 是什么？

`continue` 表示：

```text
跳过当前这一轮循环剩下的内容，直接进入下一轮循环
```

例子：

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

---

# Part 9：如何筛选指定的问题规模？

## 29. 为什么需要 `want_scale()`？

CSV 里可能有很多规模：

```text
small
medium-small
medium
large
very-large
```

但某次实验可能只想运行：

```bash
SCALES="small large"
```

所以脚本定义了函数：

```bash
want_scale() {
  local needle="$1"
  local scale
  for scale in ${SCALES}; do
    [[ "${scale}" == "${needle}" ]] && return 0
  done
  return 1
}
```

它的作用是：

```text
判断当前 CSV 行的 SCALE 是否在用户指定的 SCALES 中
```

---

## 30. `return 0` 和 `return 1` 为什么反直觉？

在 Bash 中：

```text
0 表示成功
非 0 表示失败
```

这和很多编程语言中的 `true = 1` 不一样。

所以：

```bash
return 0
```

表示：

```text
找到了，匹配成功
```

而：

```bash
return 1
```

表示：

```text
没找到，匹配失败
```

后面调用：

```bash
if ! want_scale "${SCALE}"; then
  continue
fi
```

意思是：

```text
如果当前 SCALE 不在用户想要的 SCALES 中，就跳过
```

---

# Part 10：MPI ranks、OpenMP threads 和 CPU cores 是什么关系？

## 31. 总核心数怎么计算？

在 MPI + OpenMP 混合并行中：

```text
R = MPI ranks 数量
T = 每个 rank 的 OpenMP threads 数量
P = 总 CPU cores 数量
```

关系是：

$$  
P = R \times T  
$$

例如：

$$  
R = 4  
$$

$$  
T = 8  
$$

那么：

$$  
P = 4 \times 8 = 32  
$$

意思是：

```text
启动 4 个 MPI 进程
每个 MPI 进程使用 8 个 OpenMP 线程
总共需要 32 个 CPU cores
```

---

## 32. 为什么不能测试所有 ranks × threads 组合？

ARCHER2 单节点有 128 个物理核心。

脚本中：

```bash
MAX_CORES="${MAX_CORES:-128}"
````

要求所有组合满足：

$$R \times T \leq \mathrm{MAX_CORES}$$

其中：

- $R$ 表示 MPI ranks 数量
    
- $T$ 表示每个 rank 使用的 OpenMP threads 数量
    
- $\mathrm{MAX_CORES}$ 表示当前最多允许使用的 CPU cores 数量
    

例如：

```text
R = 64
T = 4
```

则总核心数为：

$$P = R \times T$$

代入数值：

$$P = 64 \times 4 = 256$$

因为：

$$256 > 128$$

所以这个组合在单节点上不合法，必须跳过。

---

## 33. `MAX_T=$(( MAX_CORES / R ))` 是怎么推导出来的？

合法条件是：

$$R \times T \leq \mathrm{MAX_CORES}$$

为了求在给定 $R$ 的情况下，$T$ 最大能取多少，需要把两边同时除以 $R$：

$$T \leq \frac{\mathrm{MAX_CORES}}{R}$$

所以最大合法线程数是：

$$T_{\mathrm{max}} = \left\lfloor \frac{\mathrm{MAX_CORES}}{R} \right\rfloor$$

其中：

- $T_{\mathrm{max}}$ 表示当前 $R$ 下允许的最大 thread 数
    
- $\lfloor x \rfloor$ 表示向下取整，也就是 floor
    
- 因为 Bash 的整数除法会自动舍去小数部分，所以正好对应这个公式
    

在 Bash 中写成：

```bash
MAX_T=$(( MAX_CORES / R ))
```

Bash 默认使用整数除法，所以小数部分会被舍去。

例如：

```text
MAX_CORES = 128
R = 16
```

则：

$$T_{\mathrm{max}} = \left\lfloor \frac{128}{16} \right\rfloor$$

也就是：

$$T_{\mathrm{max}} = 8$$

所以当 `R=16` 时，`T` 最多只能是 `8`。



---

## 34. 如何跳过非法组合？

脚本中：

```bash
(( T > MAX_T )) && continue
```

意思是：

```text
如果 T 大于当前 R 对应的最大合法线程数，就跳过
```

等价写法：

```bash
if (( T > MAX_T )); then
  continue
fi
```

---

## 35. 为什么用 `P=$(( R * T ))` 而不是加法？

正确：

```bash
P=$(( R * T ))
```

错误：

```bash
P=$(( R + T ))
```

因为每个 MPI rank 都有 `T` 个 threads。

例如：

```text
R = 4
T = 8
```

不是：

$$  
P = 4 + 8 = 12  
$$

而是：

$$  
P = 4 \times 8 = 32  
$$

---

# Part 11：Slurm 参数如何对应 MPI + OpenMP？

## 36. `srun` 中的资源参数怎么理解？

脚本核心运行命令：

```bash
srun --nodes=1 --ntasks="${R}" --cpus-per-task="${T}" --exact \
     --hint=nomultithread --distribution=block:block \
     "${PETSC_DIR}/src/ksp/ksp/tutorials/ex2" \
     -m "${M}" -n "${N}" -ksp_converged_reason -log_view
```

核心映射如下：

|概念|Bash 变量|Slurm 参数|
|---|---|---|
|节点数|固定 1|`--nodes=1`|
|MPI ranks|`R`|`--ntasks="${R}"`|
|每个 rank 分配的 CPU cores|`T`|`--cpus-per-task="${T}"`|
|每个 rank 使用的 OpenMP threads|`T`|`OMP_NUM_THREADS="${T}"`|
|总核心数|`P=R*T`|脚本提前检查|

---

## 37. 为什么 `OMP_NUM_THREADS` 和 `--cpus-per-task` 都要设置？

脚本中：

```bash
export OMP_NUM_THREADS="${T}"
export SRUN_CPUS_PER_TASK="${T}"
```

同时 `srun` 中也有：

```bash
--cpus-per-task="${T}"
```

这两类设置作用不同。

### `--cpus-per-task`

这是告诉 Slurm：

```text
每个 MPI task 请分配 T 个 CPU cores
```

### `OMP_NUM_THREADS`

这是告诉 OpenMP runtime：

```text
每个 MPI rank 内部请创建 T 个 OpenMP threads
```

两者应该匹配。

如果只设置：

```bash
export OMP_NUM_THREADS=4
```

但没有：

```bash
--cpus-per-task=4
```

可能导致：

```text
4 个 OpenMP threads 争抢 1 个 CPU core
```

这叫 oversubscription。

如果只设置：

```bash
--cpus-per-task=4
```

但没有：

```bash
OMP_NUM_THREADS=4
```

可能导致：

```text
Slurm 分配了 4 个 cores
但程序只启动 1 个 thread
资源被浪费
```

---

## 38. `OMP_PLACES=cores` 和 `OMP_PROC_BIND=close` 是什么？

脚本中：

```bash
export OMP_PLACES=cores
export OMP_PROC_BIND=close
```

这两个变量控制 OpenMP 线程绑定。

### `OMP_PLACES=cores`

意思是：

```text
OpenMP 线程以 CPU cores 为绑定位置
```

### `OMP_PROC_BIND=close`

意思是：

```text
尽量把同一个 MPI rank 内部的 OpenMP threads 放在相近的 cores 上
```

在 HPC benchmark 中，线程绑定很重要。

因为如果线程乱跑，性能数据会更不稳定。

---

# Part 12：日志和结果 CSV 是怎么生成的？

## 39. 输出 CSV 的表头为什么先写？

脚本中：

```bash
OUTCSV="${RES_DIR}/size_grid_ex2_${PETSC_ARCH}_job${SLURM_JOB_ID}.csv"

echo "timestamp,jobid,nodelist,nodes,scale,m,n,unknowns,ranks,threads,total_cores,rep,time_sec,iterations,error_norm,logfile" > "${OUTCSV}"
```

这里的 `>` 表示覆盖写入。

意思是：

```text
创建一个新的结果 CSV
并先写入表头
```

如果文件已经存在，旧内容会被清空。

---

## 40. 每次运行的日志文件名为什么这么长？

脚本中：

```bash
LOG="${RUN_DIR}/ex2_${PETSC_ARCH}_${SCALE}_r${R}_t${T}_p${P}_m${M}_n${N}_job${SLURM_JOB_ID}_rep${rep}.log"
```

这个文件名包含：

|字段|含义|
|---|---|
|`PETSC_ARCH`|PETSc 构建类型|
|`SCALE`|问题规模|
|`r${R}`|MPI ranks|
|`t${T}`|OpenMP threads|
|`p${P}`|总核心数|
|`m${M}`|网格维度 m|
|`n${N}`|网格维度 n|
|`job${SLURM_JOB_ID}`|Slurm 作业 ID|
|`rep${rep}`|重复实验编号|

这样设计的好处是：

```text
只看文件名，就知道这次日志对应哪种实验配置
```

---

## 41. `tee "${LOG}"` 是什么？

脚本中：

```bash
srun ... | tee "${LOG}"
```

`tee` 的作用是：

```text
一边把输出显示在终端
一边把输出写入日志文件
```

如果不用 `tee`，只写：

```bash
srun ... > "${LOG}"
```

输出只会进入文件，不会显示在当前作业输出中。

使用 `tee` 更方便观察运行过程。

---

## 42. `awk` 如何从日志中提取数据？

脚本中：

```bash
TIMESEC="$(awk '/Time \(sec\):/{t=$3} END{print t}' "${LOG}")"
ITER="$(awk '/Norm of error/{iter=$NF} END{print iter}' "${LOG}")"
ERRNORM="$(awk '/Norm of error/{err=$(NF-3)} END{print err}' "${LOG}")"
```

`awk` 的基本思想是：

```text
逐行扫描文件
    ↓
找到匹配某个 pattern 的行
    ↓
提取某些字段
    ↓
最后输出结果
```

例如：

```bash
awk '/Time \(sec\):/{t=$3} END{print t}' "${LOG}"
```

意思是：

```text
找到包含 Time (sec): 的行
把第 3 个字段保存到 t
文件读完后输出 t
```

---

## 43. `$NF` 是什么？

在 `awk` 中：

```text
NF = 当前行字段数量
$NF = 当前行最后一个字段
$(NF-3) = 倒数第 4 个字段
```

例如一行文本：

```text
Norm of error 1.23e-08 iterations 5
```

字段编号大概是：

|编号|字段|
|--:|---|
|`$1`|`Norm`|
|`$2`|`of`|
|`$3`|`error`|
|`$4`|`1.23e-08`|
|`$5`|`iterations`|
|`$6`|`5`|

此时：

```text
$NF = $6 = 5
$(NF-2) = $4 = 1.23e-08
```

注意：

```text
具体该取哪个字段，必须根据真实日志格式确认
```

不能只看变量名猜。

---

## 44. 为什么要检查 `TIMESEC` 是否为空？

脚本中：

```bash
if [[ -z "${TIMESEC}" ]]; then
  echo "[ERROR] No 'Time (sec):' parsed from ${LOG} (scale=${SCALE} r=${R} t=${T} rep=${rep})."
  echo "[ERROR] Aborting to avoid corrupting CSV."
  exit 1
fi
```

`-z` 表示：

```text
字符串长度为 0
```

也就是变量为空。

如果 `TIMESEC` 为空，说明：

```text
日志里没有成功提取到 Time (sec):
```

可能原因：

- PETSc 运行失败
    
- 日志格式变化
    
- `awk` 匹配规则写错
    
- `srun` 没启动成功
    
- 日志文件不完整
    

所以脚本选择立刻停止。

这是一种很重要的防御式写法：

```text
宁愿中止，也不要把空数据写进 CSV
```

---

# Part 13：完整脚本的运行逻辑

## 45. 主循环整体结构是什么？

可以把生产脚本简化成下面这个骨架：

```bash
while IFS=, read -r SCALE M N UNKNOWNS; do
  [[ "${SCALE}" == "scale" ]] && continue
  [[ -z "${SCALE}" ]] && continue

  if ! want_scale "${SCALE}"; then
    continue
  fi

  for R in "${RANKS_LIST[@]}"; do
    (( R > MAX_CORES )) && continue

    MAX_T=$(( MAX_CORES / R ))
    (( MAX_T < 1 )) && continue

    for T in "${THREADS_LIST[@]}"; do
      (( T > MAX_T )) && continue

      P=$(( R * T ))

      for rep in $(seq 1 "${REPEAT}"); do
        run_one_case
        parse_log
        append_csv
      done
    done
  done
done < "${SIZE_TABLE}"
```

它其实就是四层循环：

```text
第 1 层：scale
第 2 层：MPI ranks
第 3 层：OpenMP threads
第 4 层：repeat
```

---

## 46. 假设有多少次运行？

如果：

```text
SCALES = small medium
RANKS = 1 2
THREADS = 1 2 4
MAX_CORES = 4
REPEAT = 2
```

合法条件是：

$$  
R \times T \leq 4  
$$

候选组合：

|R|T|P|是否合法|
|--:|--:|--:|---|
|1|1|1|合法|
|1|2|2|合法|
|1|4|4|合法|
|2|1|2|合法|
|2|2|4|合法|
|2|4|8|非法|

每个 scale 有 5 个合法组合。

两个 scale：

```text
small
medium
```

每个配置重复 2 次：

$$  
2 \times 5 \times 2 = 20  
$$

所以总共会运行 20 次。

---

# Part 14：初学者应该怎么练？

## 47. 第一步：只练 Bash 语法，不碰 Slurm

在 WSL 本地先练：

```bash
bash -n script.sh
```

含义：

```text
只检查语法，不真正执行
```

如果没有输出，通常说明语法没问题。

但注意：

```text
bash -n 只能检查语法
不能检查逻辑
```

例如：

```bash
P=$(( R + T ))
```

语法正确，但逻辑错误。

---

## 48. 第二步：用 `bash -x` 看变量怎么展开

运行：

```bash
bash -x script.sh
```

含义：

```text
执行脚本时，把每一步展开后的命令打印出来
```

适合检查：

- 变量有没有正确读取
    
- 循环有没有按预期执行
    
- 哪些组合被跳过
    
- 命令最终展开成什么样
    
- CSV 是否正确读取
    

---

## 49. 第三步：写 dry run 脚本

Dry run 的意思是：

```text
只打印计划执行的命令，不真正运行 PETSc
```

例如：

```bash
DRY_RUN=1 bash local_size_grid_practice.sh
```

这样可以先检查：

- scale 是否筛选正确
    
- ranks 和 threads 是否正确组合
    
- 非法组合是否跳过
    
- 总运行次数是否符合预期
    
- 日志路径是否合理
    
- CSV 输出是否合理
    

---

## 50. 第四步：只用 tiny 问题真实运行

如果本地已经有可运行的 `ex2`，可以先用极小规模测试：

```bash
SCALES="tiny" \
RANKS="1 2" \
THREADS="1" \
MAX_CORES=2 \
REPEAT=1 \
bash local_size_grid_practice.sh
```

本地学习目标不是测真实性能，而是确认：

```text
脚本逻辑确实能跑通
```

正式性能测试再放到 ARCHER2。

---

## 51. 第五步：提交 ARCHER2 小规模测试

先不要一上来跑所有规模。

推荐顺序：

```text
small + REPEAT=1
    ↓
small medium + REPEAT=1
    ↓
large + REPEAT=1
    ↓
large very-large + REPEAT=3
```

例如：

```bash
SCALES="small" \
REPEAT=1 \
RANKS="1 2 4 8" \
THREADS="1 2 4 8" \
sbatch scripts/run_ex2_size_grid.sbatch
```

确认无误后再扩大规模。

---

# Part 15：最小本地练习脚本

## 52. 这个脚本用于理解逻辑，不运行 PETSc

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

read -r -a RANKS_LIST <<< "${RANKS}"
read -r -a THREADS_LIST <<< "${THREADS}"

mkdir -p "$(dirname "${PLAN_CSV}")"

printf 'scale,m,n,unknowns,ranks,threads,total_cores,repeat\n' > "${PLAN_CSV}"

want_scale() {
  local needle="$1"
  local scale

  for scale in ${SCALES}; do
    [[ "${scale}" == "${needle}" ]] && return 0
  done

  return 1
}

while IFS=, read -r SCALE M N UNKNOWNS; do
  [[ "${SCALE}" == "scale" ]] && continue
  [[ -z "${SCALE}" ]] && continue

  if ! want_scale "${SCALE}"; then
    continue
  fi

  for R in "${RANKS_LIST[@]}"; do
    (( R > MAX_CORES )) && continue

    MAX_T=$(( MAX_CORES / R ))
    (( MAX_T < 1 )) && continue

    for T in "${THREADS_LIST[@]}"; do
      (( T > MAX_T )) && continue

      P=$(( R * T ))

      for REP in $(seq 1 "${REPEAT}"); do
        printf 'Would run: scale=%s m=%s n=%s ranks=%s threads=%s cores=%s repeat=%s\n' \
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

## 53. 最小 CSV 文件

保存为：

```text
tutorial/examples/local_problem_sizes.csv
```

内容：

```csv
scale,m,n,unknowns
tiny,8,8,64
small,100,100,10000
medium,200,300,60000
```

---

## 54. 练习命令

检查语法：

```bash
bash -n tutorial/examples/my_size_grid_practice.sh
```

运行默认配置：

```bash
bash tutorial/examples/my_size_grid_practice.sh
```

覆盖参数运行：

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

追踪执行过程：

```bash
bash -x tutorial/examples/my_size_grid_practice.sh
```

---

# Part 16：从本地练习脚本升级到 ARCHER2 生产脚本

## 55. 本地脚本缺少什么？

本地练习脚本只做：

```text
读取 CSV
筛选 scale
生成合法组合
打印计划
写入 plan CSV
```

生产脚本还需要：

```text
SBATCH 资源申请
module 环境加载
PETSc 路径
make 编译 ex2
srun 启动程序
tee 保存日志
awk 解析日志
错误检查
结果 CSV 汇总
```

---

## 56. 生产脚本的核心升级点

本地脚本中只是：

```bash
printf 'Would run ...\n'
```

生产脚本中替换成：

```bash
export OMP_NUM_THREADS="${T}"
export SRUN_CPUS_PER_TASK="${T}"

srun --nodes=1 --ntasks="${R}" --cpus-per-task="${T}" --exact \
     --hint=nomultithread --distribution=block:block \
     "${PETSC_DIR}/src/ksp/ksp/tutorials/ex2" \
     -m "${M}" -n "${N}" -ksp_converged_reason -log_view \
  | tee "${LOG}"
```

然后解析日志：

```bash
TIMESEC="$(awk '/Time \(sec\):/{t=$3} END{print t}' "${LOG}")"
ITER="$(awk '/Norm of error/{iter=$NF} END{print iter}' "${LOG}")"
ERRNORM="$(awk '/Norm of error/{err=$(NF-3)} END{print err}' "${LOG}")"
```

最后追加 CSV：

```bash
echo "$(ts),${SLURM_JOB_ID},${SLURM_JOB_NODELIST:-UNKNOWN},${SLURM_JOB_NUM_NODES},${SCALE},${M},${N},${UNKNOWNS},${R},${T},${P},${rep},${TIMESEC},${ITER},${ERRNORM},${LOG}" >> "${OUTCSV}"
```

---

# Part 17：术语表

|术语|英文|含义|
|---|---|---|
|Bash|Bourne Again Shell|常用 Shell 脚本语言|
|Shell Script|Shell Script|由命令、变量、循环、判断组成的自动化脚本|
|Shebang|`#!/usr/bin/env bash`|指定脚本解释器|
|Slurm|Simple Linux Utility for Resource Management|HPC 集群作业调度系统|
|SBATCH Directive|`#SBATCH`|Slurm 批处理作业资源申请指令|
|sbatch|`sbatch`|提交 Slurm 批处理作业|
|srun|`srun`|在 Slurm 分配的资源中启动程序|
|CSV|Comma-Separated Values|逗号分隔的文本表格|
|Header|Header Row|CSV 第一行表头|
|Parameter Expansion|`${variable}`|Bash 变量展开|
|Default Value|`${x:-v}`|变量为空或未定义时使用默认值|
|Override|Override|从命令行临时覆盖脚本默认值|
|IFS|Internal Field Separator|Bash 字段分隔符|
|Here-string|`<<<`|把字符串作为输入传给命令|
|Array|Array|Bash 数组|
|MPI Rank|MPI Rank|一个 MPI 进程|
|OpenMP Thread|OpenMP Thread|一个 OpenMP 线程|
|Hybrid Parallelism|MPI + OpenMP|MPI 进程与 OpenMP 线程结合使用|
|Oversubscription|Oversubscription|创建的线程数超过实际分配的 CPU cores|
|Log File|Log File|保存程序完整输出的日志文件|
|Result CSV|Result CSV|保存提取后关键结果的表格|
|awk|`awk`|按行和字段处理文本的命令行工具|
|tee|`tee`|同时显示输出并写入文件|
|Dry Run|Dry Run|只打印计划，不真正执行程序|
|`set -e`|Exit on Error|命令失败时退出|
|`set -u`|Unset Variable Error|使用未定义变量时报错|
|`pipefail`|Pipeline Failure Detection|管道中任意命令失败则整体失败|

---

# Part 18：核心结论

## 1. 学脚本不要从背语法开始，而要从问题开始

这份脚本要解决的问题是：

```text
如何自动、可复现地运行大量 benchmark 配置
```

语法只是为这个目标服务。

---

## 2. CSV 负责输入，Bash 负责逻辑

推荐设计是：

```text
CSV：
    存放问题规模

Bash：
    读取 CSV
    生成实验组合
    执行命令
    汇总结果
```

不要把所有问题规模硬编码在脚本里。

---

## 3. MPI ranks 和 OpenMP threads 的总核心数必须用乘法

核心公式是：

$$P = R \times T$$

其中：

```text
R = MPI ranks
T = OpenMP threads per rank
P = total CPU cores
````

也就是说：

```text
总核心数 = MPI 进程数 × 每个进程使用的 OpenMP 线程数
```

例如：

```text
R = 4
T = 8
```

则：

$$P = 4 \times 8 = 32$$

所以这个配置表示：

```text
启动 4 个 MPI ranks
每个 rank 使用 8 个 OpenMP threads
总共需要 32 个 CPU cores
```

合法条件是：

$$R \times T \leq \mathrm{MAX_CORES}$$

其中：

```text
MAX_CORES = 当前允许使用的最大 CPU 核心数
```

例如在 ARCHER2 单节点上，如果最多使用 128 个物理核心，那么必须满足：

$$R \times T \leq 128$$

如果：

```text
R = 64
T = 4
```

则：

$$R \times T = 64 \times 4 = 256$$

因为：

$$256 > 128$$

所以这个组合不合法，脚本应该跳过。


---

## 4. Slurm 分配资源和 OpenMP 使用资源必须一致

如果：

```bash
OMP_NUM_THREADS="${T}"
```

通常也要有：

```bash
--cpus-per-task="${T}"
```

否则可能出现：

```text
线程争抢核心
或者
核心分配了但没用上
```

---

## 5. 日志和 CSV 都要保留

```text
日志：
    用于排查问题
    保存完整证据

CSV：
    用于统计分析
    用于画图
```

两者不能互相替代。

---

## 6. 本地练 Bash，ARCHER2 练 Slurm

WSL 本地适合练：

```text
CSV 读取
Bash 循环
变量展开
参数过滤
dry run
bash -n
bash -x
```

ARCHER2 才适合测试：

```text
#SBATCH
module
srun
真实节点资源
真实性能
```

---

# Part 19：常见误区

## 误区 1：把 CSV 表头当成数据

错误：

```bash
while IFS=, read -r SCALE M N UNKNOWNS; do
  echo "${SCALE}"
done < "${SIZE_TABLE}"
```

正确：

```bash
[[ "${SCALE}" == "scale" ]] && continue
```

---

## 误区 2：把总核心数写成加法

错误：

```bash
P=$(( R + T ))
```

正确：

```bash
P=$(( R * T ))
```

---

## 误区 3：只设置 `OMP_NUM_THREADS`

错误：

```bash
export OMP_NUM_THREADS=4

srun --ntasks=2 ...
```

缺少：

```bash
--cpus-per-task=4
```

可能导致 oversubscription。

---

## 误区 4：只设置 `--cpus-per-task`

错误：

```bash
srun --ntasks=2 --cpus-per-task=4 ...
```

如果没有：

```bash
export OMP_NUM_THREADS=4
```

程序可能仍然只启动 1 个 OpenMP thread。

---

## 误区 5：以为 `bash -n` 能检查逻辑

```bash
bash -n script.sh
```

只能检查语法，不能检查逻辑。

例如：

```bash
P=$(( R + T ))
```

语法是对的，但逻辑是错的。

---

## 误区 6：变量不加双引号

不推荐：

```bash
cd $PROJECT_ROOT
```

推荐：

```bash
cd "${PROJECT_ROOT}"
```

因为加双引号可以避免空格拆词和通配符展开问题。

---

## 误区 7：日志解析变量命名不准确

如果提取的是：

```text
Norm of error
```

变量名最好叫：

```bash
ERRNORM
```

不要随便叫：

```bash
ITER
```

变量名应该表达真实含义。

---

# Part 20：自测问题

## Bash 基础

1. `#!/usr/bin/env bash` 的作用是什么？
    
2. 为什么普通变量展开推荐写成 `"${VAR}"`？
    
3. `${x:-v}` 和 `${x-v}` 有什么区别？
    
4. `set -euo pipefail` 分别解决什么问题？
    
5. `read -r -a arr <<< "${text}"` 中，`-a` 和 `<<<` 分别是什么意思？
    

---

## CSV 读取

1. `IFS=, read -r SCALE M N UNKNOWNS` 中，`IFS=,` 的作用是什么？
    
2. `done < "${SIZE_TABLE}"` 是什么重定向？
    
3. 为什么要跳过 CSV 表头？
    
4. `continue` 在循环中起什么作用？
    

---

## 参数组合

1. 如果 `R=8`，`T=4`，总核心数是多少？
    
2. 为什么是 `P=$(( R * T ))`，不是 `P=$(( R + T ))`？
    
3. 如果 `MAX_CORES=128`，`R=32`，最大合法 `T` 是多少？
    
4. 为什么 `(( T > MAX_T )) && continue` 可以过滤非法组合？
    

---

## Slurm 与 OpenMP

1. `--ntasks="${R}"` 对应什么？
    
2. `--cpus-per-task="${T}"` 对应什么？
    
3. `OMP_NUM_THREADS="${T}"` 对应什么？
    
4. 如果 `OMP_NUM_THREADS=8`，但 `--cpus-per-task=1`，可能发生什么？
    
5. 如果 `--cpus-per-task=8`，但 `OMP_NUM_THREADS=1`，可能发生什么？
    

---

## 日志与结果

1. 为什么要保存完整日志？
    
2. 为什么还需要单独生成 summary CSV？
    
3. `tee "${LOG}"` 的作用是什么？
    
4. `awk '/Time \(sec\):/{t=$3} END{print t}' "${LOG}"` 的逻辑是什么？
    
5. 为什么解析不到 `TIMESEC` 时应该直接退出？
    

---

# Part 21：推荐学习路线

## 阶段 1：只看脚本骨架

先不要纠结每个细节，只记住：

```text
读取 CSV
筛选 scale
遍历 ranks
遍历 threads
过滤非法组合
运行程序
保存日志
解析结果
写入 CSV
```

---

## 阶段 2：本地重写最小 dry run 脚本

目标不是跑 PETSc，而是生成计划：

```text
scale, m, n, ranks, threads, total_cores, repeat
```

---

## 阶段 3：手动预测输出

给定：

```text
SCALES = tiny medium
RANKS = 1 2
THREADS = 1 2 4
MAX_CORES = 4
REPEAT = 2
```

先手动算出合法组合，再运行脚本检查是否一致。

---

## 阶段 4：用 `bash -x` 观察展开过程

运行：

```bash
bash -x tutorial/examples/my_size_grid_practice.sh
```

重点观察：

```text
SCALE 如何变化
R 如何变化
T 如何变化
P 如何计算
continue 在哪里触发
```

---

## 阶段 5：再看完整 Slurm 脚本

最后再回到生产脚本：

```text
scripts/run_ex2_size_grid.sbatch
```

此时每一段就能对应到前面的概念，而不是一坨看不懂的 Bash 咒语。