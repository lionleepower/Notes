# PETSc 中不同矩阵类型的区别：AIJ、SBAIJ、SELL

## 1. 这段代码整体在做什么？

原始代码：

```c
PetscCall(MatCreate(PETSC_COMM_WORLD, &A));

PetscCall(MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, m * n, m * n));

PetscCall(MatSetFromOptions(A));

PetscCall(MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL));

PetscCall(MatSeqAIJSetPreallocation(A, 5, NULL));

PetscCall(MatSeqSBAIJSetPreallocation(A, 1, 5, NULL));

PetscCall(MatMPISBAIJSetPreallocation(A, 1, 5, NULL, 5, NULL));

PetscCall(MatMPISELLSetPreallocation(A, 5, NULL, 5, NULL));

PetscCall(MatSeqSELLSetPreallocation(A, 5, NULL));
```

这段代码的核心作用是：

> 创建一个 PETSc 稀疏矩阵 `A`，设置它的全局大小，然后根据运行时选择的矩阵类型，为不同的稀疏矩阵格式提前分配内存。

这里涉及三类主要矩阵格式：

```text
AIJ   = 普通 CSR 稀疏矩阵格式
SBAIJ = 对称 block CSR 稀疏矩阵格式
SELL  = Sliced ELLPACK 稀疏矩阵格式
```

同时，每种矩阵格式又分为：

```text
Seq = sequential，单进程版本
MPI = parallel，多进程分布式版本
```

所以代码中会看到：

```text
SeqAIJ      MPIAIJ
SeqSBAIJ    MPISBAIJ
SeqSELL     MPISELL
```

---

# 2. 为什么要创建矩阵？

在 PETSc 的 `ex2` 这类 PDE 离散问题中，我们通常要求解一个线性系统：

$$
A x = b
$$

其中：

- `A` 是矩阵，来自 PDE 离散化；
- `x` 是要求解的未知向量；
- `b` 是右端项。

如果二维网格大小是：

$$
m \times n
$$

那么总未知量数量是：

$$
N = m n
$$

所以矩阵大小就是：

$$
A \in \mathbb{R}^{N \times N}
$$

也就是：

$$
A \in \mathbb{R}^{(m n) \times (m n)}
$$

对应代码：

```c
PetscCall(MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, m * n, m * n));
```

意思是：

> 创建一个全局大小为 `(m*n) × (m*n)` 的矩阵。

---

# 3. `MatCreate` 在做什么？

代码：

```c
PetscCall(MatCreate(PETSC_COMM_WORLD, &A));
```

这行代码创建了一个 PETSc 矩阵对象 `A`。

注意，这时候还没有真正决定矩阵的具体存储格式。

也就是说，此时 `A` 只是一个抽象矩阵对象。

它可能之后变成：

```text
AIJ
SBAIJ
SELL
```

或者其他 PETSc 支持的矩阵格式。

`PETSC_COMM_WORLD` 表示这个矩阵存在于整个 MPI communicator 上。

如果你用 1 个进程运行，它就是单进程矩阵。

如果你用多个进程运行，它就是分布式矩阵。

例如：

```bash
./ex2
```

通常是单进程。

```bash
srun -n 4 ./ex2
```

就是 4 个 MPI processes 并行运行。

---

# 4. `MatSetSizes` 在做什么？

代码：

```c
PetscCall(MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, m * n, m * n));
```

这个函数设置矩阵大小。

函数形式可以粗略理解为：

```c
MatSetSizes(A, local_rows, local_cols, global_rows, global_cols);
```

这里写的是：

```c
MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, m * n, m * n);
```

意思是：

```text
local_rows  = PETSC_DECIDE
local_cols  = PETSC_DECIDE
global_rows = m * n
global_cols = m * n
```

其中 `PETSC_DECIDE` 的意思是：

> 每个 MPI process 分配多少行、多少列，由 PETSc 自动决定。

例如全局矩阵大小是：

$$
16 \times 16
$$

如果用 4 个 MPI processes，PETSc 可能让每个 process 拥有 4 行：

```text
process 0 owns rows 0  - 3
process 1 owns rows 4  - 7
process 2 owns rows 8  - 11
process 3 owns rows 12 - 15
```

所以你不用手动分配每个进程负责哪些行，PETSc 可以帮你处理。

---

# 5. `MatSetFromOptions` 为什么重要？

代码：

```c
PetscCall(MatSetFromOptions(A));
```

这行代码非常重要。

它的意思是：

> 允许用户在运行程序的时候，通过命令行选项设置矩阵类型。

例如：

```bash
./ex2 -mat_type aij
```

表示使用 AIJ 格式。

```bash
./ex2 -mat_type sbaij
```

表示使用 SBAIJ 格式。

```bash
./ex2 -mat_type sell
```

表示使用 SELL 格式。

所以代码本身不需要写死矩阵格式。

这很适合 benchmark，因为你可以用同一个程序测试不同矩阵存储格式的性能。

---

# 6. 为什么需要 Preallocation？

## 6.1 什么是 Preallocation？

Preallocation 的意思是：

> 在真正插入矩阵元素之前，提前告诉 PETSc 每一行大概会有多少个非零元素，让 PETSc 提前分配内存。

稀疏矩阵的特点是：

> 大部分元素都是 0，只有少量元素非零。

例如：

$$
A =
\begin{bmatrix}
4 & -1 & 0 & 0 \\
-1 & 4 & -1 & 0 \\
0 & -1 & 4 & -1 \\
0 & 0 & -1 & 4
\end{bmatrix}
$$

这个矩阵总共有：

$$
4 \times 4 = 16
$$

个位置。

但是非零元素只有 10 个。

所以没有必要存储所有 16 个位置，只需要存储非零元素。

---

## 6.2 如果不 Preallocate 会怎样？

如果不提前分配内存，PETSc 在插入矩阵元素的时候可能会不断扩容。

这就像你一开始只准备了一个很小的盒子，后来发现东西越来越多，就要不断换更大的盒子。

这样会带来额外开销：

```text
反复申请内存
反复移动数据
矩阵装配变慢
benchmark 结果不稳定
```

所以 PETSc 代码通常会显式写 preallocation。

---

# 7. 为什么这里总是出现数字 5？

很多地方都有：

```c
5
```

例如：

```c
MatSeqAIJSetPreallocation(A, 5, NULL);
```

这个 `5` 来自 2D five-point stencil。

在二维网格上，一个内部点通常连接 5 个位置：

```text
自己
上
下
左
右
```

例如二维网格：

```text
0   1   2   3
4   5   6   7
8   9   10  11
12  13  14  15
```

对于点 `5`，它连接：

```text
自己: 5
上:   1
下:   9
左:   4
右:   6
```

所以矩阵第 5 行会有这些非零元素：

```text
A[5,5]
A[5,1]
A[5,9]
A[5,4]
A[5,6]
```

也就是最多 5 个非零元素。

因此代码中写：

```c
5
```

意思是：

> 每一行大概最多有 5 个非零元素。

---

# 8. AIJ 是什么？

## 8.1 AIJ 的核心概念

AIJ 是 PETSc 中最常用的稀疏矩阵格式。

它基本等价于：

```text
CSR = Compressed Sparse Row
```

中文可以理解为：

> 按行压缩存储的稀疏矩阵格式。

AIJ 的思想是：

> 每一行只存储非零元素，不存储大量的 0。

---

## 8.2 AIJ 的例子

假设矩阵是：

$$
A =
\begin{bmatrix}
4 & -1 & 0 & 0 \\
-1 & 4 & -1 & 0 \\
0 & -1 & 4 & -1 \\
0 & 0 & -1 & 4
\end{bmatrix}
$$

如果用 dense matrix 存储，需要存 16 个数。

但是 AIJ 只存非零元素：

```text
row 0: col 0 = 4,  col 1 = -1
row 1: col 0 = -1, col 1 = 4, col 2 = -1
row 2: col 1 = -1, col 2 = 4, col 3 = -1
row 3: col 2 = -1, col 3 = 4
```

所以 AIJ 存储的是：

```text
每一行有哪些非零列
这些非零列对应的值是多少
```

---

## 8.3 AIJ 适合什么场景？

AIJ 是最通用的格式。

适合：

```text
普通稀疏矩阵
非对称矩阵
结构不规则矩阵
PDE 离散矩阵
默认 benchmark baseline
```

对于你的 PETSc benchmark 来说，AIJ 应该作为 baseline。

也就是说：

> 先测 AIJ，再用 SBAIJ 和 SELL 与它比较。

运行示例：

```bash
srun -n 4 ./ex2 -mat_type aij
```

---

# 9. AIJ 的 Preallocation

代码：

```c
PetscCall(MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL));
PetscCall(MatSeqAIJSetPreallocation(A, 5, NULL));
```

这里有两个版本：

```text
MatSeqAIJSetPreallocation = 单进程 AIJ 预分配
MatMPIAIJSetPreallocation = 多进程 AIJ 预分配
```

## 9.1 SeqAIJ

代码：

```c
MatSeqAIJSetPreallocation(A, 5, NULL);
```

意思是：

> 如果当前矩阵是单进程 AIJ，每一行预留大约 5 个非零元素空间。

---

## 9.2 MPIAIJ

代码：

```c
MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL);
```

MPIAIJ 更复杂，因为矩阵被分布在多个 MPI processes 上。

在 MPIAIJ 中，矩阵通常分成两部分：

```text
diagonal part
off-diagonal part
```

---

## 9.3 什么是 diagonal part 和 off-diagonal part？

假设全局矩阵大小是：

$$
8 \times 8
$$

用 2 个 MPI processes。

```text
process 0 owns rows 0 - 3
process 1 owns rows 4 - 7
```

对于 process 0 来说：

```text
本地 rows: 0 - 3
本地 columns: 0 - 3
远程 columns: 4 - 7
```

所以 process 0 上的矩阵可以分成：

```text
diagonal part     = 列也属于 process 0 的部分
off-diagonal part = 列属于其他 process 的部分
```

例如：

```text
A[3,3] 属于 diagonal part
A[3,4] 属于 off-diagonal part
```

因为 row 3 属于 process 0，但 column 4 属于 process 1。

---

## 9.4 `MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL)` 的含义

函数参数可以粗略理解为：

```c
MatMPIAIJSetPreallocation(A, d_nz, d_nnz, o_nz, o_nnz);
```

其中：

```text
d_nz  = diagonal part 每行预留多少非零元素
d_nnz = diagonal part 每行精确预留数组
o_nz  = off-diagonal part 每行预留多少非零元素
o_nnz = off-diagonal part 每行精确预留数组
```

所以：

```c
MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL);
```

意思是：

```text
diagonal part 每行预留 5 个非零元素
off-diagonal part 每行预留 5 个非零元素
```

严格来说，这是一种比较保守的写法。

因为对于 2D five-point stencil，一整行最多大概只有 5 个非零元素，不是 diagonal part 5 个加 off-diagonal part 5 个。

更精确的写法应该使用数组：

```text
d_nnz[]
o_nnz[]
```

分别告诉 PETSc 每一行在本地部分和远程部分各有多少个非零元素。

但是教学代码和 benchmark baseline 里经常使用这种简单写法，因为：

```text
代码更简单
不容易出错
预分配空间足够
适合快速测试不同矩阵格式
```

---

# 10. SBAIJ 是什么？

## 10.1 SBAIJ 的核心概念

SBAIJ 可以理解为：

```text
Symmetric Block AIJ
```

也就是：

> 对称 block 稀疏矩阵格式。

它适合存储对称矩阵。

如果矩阵满足：

$$
A_{ij} = A_{ji}
$$

那么它就是对称矩阵。

例如：

$$
A =
\begin{bmatrix}
4 & -1 & 0 & 0 \\
-1 & 4 & -1 & 0 \\
0 & -1 & 4 & -1 \\
0 & 0 & -1 & 4
\end{bmatrix}
$$

这个矩阵是对称的，因为：

$$
A_{0,1} = A_{1,0} = -1
$$

$$
A_{1,2} = A_{2,1} = -1
$$

$$
A_{2,3} = A_{3,2} = -1
$$

---

## 10.2 SBAIJ 为什么可以省空间？

对于对称矩阵，AIJ 会存上下三角的所有非零元素。

例如 AIJ 会存：

```text
A[0,1] = -1
A[1,0] = -1
```

但这两个值其实是重复信息，因为对称矩阵中：

$$
A_{0,1} = A_{1,0}
$$

所以 SBAIJ 可以只存一半，比如只存上三角部分。

完整矩阵是：

$$
A =
\begin{bmatrix}
4 & -1 & 0 & 0 \\
-1 & 4 & -1 & 0 \\
0 & -1 & 4 & -1 \\
0 & 0 & -1 & 4
\end{bmatrix}
$$

AIJ 存储：

```text
row 0: (0,4),  (1,-1)
row 1: (0,-1), (1,4),  (2,-1)
row 2: (1,-1), (2,4),  (3,-1)
row 3: (2,-1), (3,4)
```

SBAIJ 可以只存上三角：

```text
row 0: (0,4), (1,-1)
row 1: (1,4), (2,-1)
row 2: (2,4), (3,-1)
row 3: (3,4)
```

这样就减少了存储量。

---

# 11. SBAIJ 中的 block size 是什么？

代码：

```c
PetscCall(MatSeqSBAIJSetPreallocation(A, 1, 5, NULL));
PetscCall(MatMPISBAIJSetPreallocation(A, 1, 5, NULL, 5, NULL));
```

这里第二个参数是：

```c
1
```

这个 `1` 表示：

```text
block size = 1
```

也就是每个 block 是一个 scalar。

---

## 11.1 为什么普通 Poisson 问题里 block size 是 1？

如果每个网格点只有一个未知量：

$$
u_{i,j}
$$

那么每个点对应一个 scalar unknown。

所以 block size 是：

$$
\mathrm{bs} = 1
$$

---

## 11.2 什么时候 block size 会大于 1？

如果每个网格点有多个未知量，例如流体力学中每个点有：

```text
u = x 方向速度
v = y 方向速度
p = pressure
```

那么每个网格点有 3 个 unknowns：

$$
u_{i,j}, v_{i,j}, p_{i,j}
$$

这时候 block size 可能是：

$$
\mathrm{bs} = 3
$$

这类问题用 block format 可能更合适，因为矩阵天然具有 block structure。

---

# 12. SBAIJ 适合什么场景？

SBAIJ 适合：

```text
矩阵是对称的
矩阵来自 Laplacian / Poisson equation
矩阵是 symmetric positive definite, SPD
存在 block structure
希望减少内存占用
```

例如：

```text
Poisson equation
Laplacian operator
某些 finite element stiffness matrix
某些结构力学问题
```

但是 SBAIJ 不适合一般非对称矩阵。

如果矩阵不满足：

$$
A_{ij} = A_{ji}
$$

就不应该随便用 SBAIJ。

例如 convection-diffusion problem 中有方向性流动项，矩阵可能是非对称的：

$$
A_{ij} \neq A_{ji}
$$

这时候使用 SBAIJ 就可能不合适。

---

# 13. SELL 是什么？

## 13.1 SELL 的核心概念

SELL 可以理解为：

```text
Sliced ELLPACK
```

它是一种更规整的稀疏矩阵存储格式。

AIJ 是按行存储非零元素。

SELL 则是：

> 把矩阵的若干行切成一个 slice，然后让每个 slice 内部的存储更加整齐。

---

## 13.2 为什么需要 SELL？

现代 CPU 和 GPU 喜欢规整的内存访问。

AIJ 虽然通用，但是每一行非零元素数量可能不同：

```text
row 0: 2 nonzeros
row 1: 5 nonzeros
row 2: 5 nonzeros
row 3: 3 nonzeros
row 4: 5 nonzeros
```

这种不规则结构对 CPU vectorization、SIMD 和 GPU 并行不一定友好。

SELL 的思想是：

> 用一点 padding 换取更规整的数据布局。

---

## 13.3 SELL 的例子

假设一个 slice 包含 4 行：

```text
slice 0:
row 0: 2 nonzeros
row 1: 5 nonzeros
row 2: 5 nonzeros
row 3: 3 nonzeros
```

这个 slice 内最长的行有 5 个非零元素。

那么 SELL 可能把短行补齐到 5：

```text
row 0: 2 real nonzeros + 3 padding
row 1: 5 real nonzeros
row 2: 5 real nonzeros
row 3: 3 real nonzeros + 2 padding
```

这样做会浪费一点空间，但是好处是：

```text
数据排列更规整
内存访问更连续
更适合 SIMD
更适合 GPU
MatMult / SpMV 可能更快
```

---

# 14. SELL 适合什么场景？

SELL 适合：

```text
每行非零元素数量比较接近
stencil matrix
SpMV / MatMult 占主要时间
SIMD-friendly computation
GPU-friendly computation
```

你的 2D five-point stencil 就比较适合测试 SELL。

因为大部分内部点都是 5 个非零元素：

```text
center
up
down
left
right
```

也就是：

$$
5 \ \mathrm{nonzeros \ per \ row}
$$

边界点可能少一点，但整体结构仍然比较规则。

---

# 15. SELL 的 Preallocation

代码：

```c
PetscCall(MatMPISELLSetPreallocation(A, 5, NULL, 5, NULL));
PetscCall(MatSeqSELLSetPreallocation(A, 5, NULL));
```

含义是：

```text
如果是 MPISELL，就按 MPI SELL 格式预分配
如果是 SeqSELL，就按 sequential SELL 格式预分配
```

这里的 `5` 仍然来自 five-point stencil。

也就是告诉 PETSc：

> 每一行大概最多有 5 个非零元素。

---

# 16. Seq 和 MPI 的区别

PETSc 中常见矩阵类型有：

```text
SeqAIJ
MPIAIJ
SeqSBAIJ
MPISBAIJ
SeqSELL
MPISELL
```

这里的 `Seq` 和 `MPI` 是并行模式的区别。

---

## 16.1 Seq 是什么？

`Seq` 表示：

```text
sequential
```

也就是单进程矩阵。

如果你这样运行：

```bash
./ex2 -mat_type aij
```

通常使用的是：

```text
SeqAIJ
```

前提是你只启动了一个 MPI process。

---

## 16.2 MPI 是什么？

`MPI` 表示：

```text
parallel distributed matrix
```

也就是多进程分布式矩阵。

如果你这样运行：

```bash
srun -n 4 ./ex2 -mat_type aij
```

PETSc 会使用 MPI 版本的矩阵，例如：

```text
MPIAIJ
```

矩阵的不同 rows 会分布到不同 MPI processes 上。

---

# 17. 为什么代码里同时写 Seq 和 MPI preallocation？

因为同一个程序可能用单进程运行，也可能用多进程运行。

例如：

```bash
./ex2 -mat_type aij
```

可能使用 SeqAIJ。

而：

```bash
srun -n 4 ./ex2 -mat_type aij
```

可能使用 MPIAIJ。

如果代码只写：

```c
MatSeqAIJSetPreallocation(A, 5, NULL);
```

那么多进程情况下不够。

如果只写：

```c
MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL);
```

那么单进程情况下也不完整。

所以代码同时写：

```c
MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL);
MatSeqAIJSetPreallocation(A, 5, NULL);
```

这是一种兼容写法。

意思是：

> 无论当前运行是单进程还是多进程，都给对应矩阵格式设置合理的预分配。

---

# 18. 用 2D five-point stencil 串起来理解

假设二维网格是：

```text
4 × 4
```

总未知量数量：

$$
N = 4 \times 4 = 16
$$

矩阵大小：

$$
A \in \mathbb{R}^{16 \times 16}
$$

二维点映射到一维编号：

```text
0   1   2   3
4   5   6   7
8   9   10  11
12  13  14  15
```

对于内部点 `5`：

```text
上:   1
左:   4
自己: 5
右:   6
下:   9
```

所以矩阵第 5 行有：

```text
A[5,1]
A[5,4]
A[5,5]
A[5,6]
A[5,9]
```

也就是 5 个非零元素。

因此：

```c
MatSeqAIJSetPreallocation(A, 5, NULL);
```

中的 `5` 是合理的。

---

# 19. 2D stencil 对应的矩阵结构

对于二维 Laplacian，内部点通常满足：

$$
- u_{i-1,j} - u_{i+1,j} - u_{i,j-1} - u_{i,j+1} + 4u_{i,j} = f_{i,j}
$$

如果把二维坐标映射为一维编号：

$$
k = i n + j
$$

那么中心点是：

$$
u_{i,j} \rightarrow x_k
$$

上方点：

$$
u_{i-1,j} \rightarrow x_{k-n}
$$

下方点：

$$
u_{i+1,j} \rightarrow x_{k+n}
$$

左边点：

$$
u_{i,j-1} \rightarrow x_{k-1}
$$

右边点：

$$
u_{i,j+1} \rightarrow x_{k+1}
$$

所以第 `k` 行有：

$$
A_{k,k} = 4
$$

$$
A_{k,k-n} = -1
$$

$$
A_{k,k+n} = -1
$$

$$
A_{k,k-1} = -1
$$

$$
A_{k,k+1} = -1
$$

这就是 five-point stencil 的来源。

---

# 20. 三种矩阵格式的核心对比

| 矩阵类型 | 全称 / 含义 | 存储特点 | 适合场景 | 主要风险 |
|---|---|---|---|---|
| `AIJ` | CSR / Compressed Sparse Row | 按行存储非零元素 | 通用稀疏矩阵，baseline | 不一定是最快 |
| `SBAIJ` | Symmetric Block AIJ | 只存对称矩阵的一半，支持 block | 对称矩阵，SPD，多变量 PDE | 矩阵必须适合对称存储 |
| `SELL` | Sliced ELLPACK | 按 slice 规整存储，可能有 padding | stencil，SIMD，GPU，SpMV | 行长度差异大时浪费空间 |

---

# 21. 对 PETSc benchmark 的理解

对于你的 PETSc benchmark，不同矩阵格式可以理解为：

> 不同 sparse matrix storage backend。

它们会影响：

```text
内存占用
矩阵装配速度
MatMult / SpMV 性能
cache locality
SIMD/vectorization 效果
MPI communication pattern
OpenMP threaded kernel 表现
```

---

## 21.1 AIJ 作为 baseline

AIJ 是最基本、最通用的格式。

它回答的问题是：

> PETSc 默认 CSR 格式在当前问题上表现如何？

运行示例：

```bash
srun -n 4 ./ex2 -mat_type aij
```

报告中可以写：

```text
AIJ is used as the baseline sparse matrix format because it is PETSc's default general-purpose CSR-like format.
```

---

## 21.2 SBAIJ 测试对称存储的影响

SBAIJ 利用了矩阵对称性。

它回答的问题是：

> 如果矩阵是对称的，只存储一半矩阵后，是否能减少内存占用并改善性能？

运行示例：

```bash
srun -n 4 ./ex2 -mat_type sbaij
```

但是分析时要注意：

> SBAIJ 比 AIJ 快，不一定说明 OpenMP 更强，也可能是因为存储格式改变了。

可能原因包括：

```text
存储量减少
memory bandwidth 压力降低
cache locality 改善
MatMult kernel 不同
```

---

## 21.3 SELL 测试规整存储的影响

SELL 更适合规则 stencil 和向量化。

它回答的问题是：

> 对于规则 stencil problem，SELL 是否比 AIJ 更适合 SIMD / threaded MatMult？

运行示例：

```bash
srun -n 4 ./ex2 -mat_type sell
```

如果 SELL 性能更好，可能原因包括：

```text
行结构更规整
内存访问更连续
SIMD 利用率更好
SpMV kernel 更适合规则稀疏结构
```

如果 SELL 性能没有更好，可能原因包括：

```text
padding 带来额外开销
问题规模不够大
线程调度开销抵消收益
矩阵行结构没有充分匹配 SELL
```

---

# 22. 为什么不同矩阵格式会影响性能？

求解器中很重要的一步是矩阵向量乘法：

$$
y = A x
$$

这通常叫：

```text
MatMult
SpMV = Sparse Matrix-Vector Multiplication
```

在 Krylov solver 中，例如 CG、GMRES，反复需要做 SpMV。

所以矩阵格式会直接影响：

```text
每次 SpMV 读多少内存
访问 x 向量是否连续
cache miss 多不多
是否容易 SIMD vectorization
MPI 远程数据访问多不多
OpenMP 多线程负载是否均衡
```

因此，即使数学上的矩阵一样，不同存储格式也可能导致不同运行时间。

---

# 23. 一个关键区别：数学矩阵 vs 存储格式

数学上，AIJ、SBAIJ、SELL 可以表示同一个矩阵。

例如它们都可以表示：

$$
A x = b
$$

中的同一个 `A`。

但是计算机内部存储方式不同。

可以理解为：

```text
数学矩阵 = 内容
矩阵格式 = 内容在内存里的摆放方式
```

类似于同一本书可以：

```text
纸质书
PDF
EPUB
网页
```

内容一样，但读取方式和效率不同。

对于 PETSc benchmark 来说，你测的不只是数学问题，也是在测：

> 同一个数学问题，在不同 sparse matrix format 下的性能差异。

---

# 24. 代码逐行解释总结

## 24.1 创建矩阵

```c
PetscCall(MatCreate(PETSC_COMM_WORLD, &A));
```

创建矩阵对象 `A`。

---

## 24.2 设置矩阵大小

```c
PetscCall(MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, m * n, m * n));
```

设置矩阵全局大小为：

$$
(mn) \times (mn)
$$

---

## 24.3 从命令行读取矩阵类型

```c
PetscCall(MatSetFromOptions(A));
```

允许使用：

```bash
-mat_type aij
-mat_type sbaij
-mat_type sell
```

选择矩阵格式。

---

## 24.4 AIJ 预分配

```c
PetscCall(MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL));
PetscCall(MatSeqAIJSetPreallocation(A, 5, NULL));
```

如果矩阵类型是 AIJ，则按 five-point stencil 每行约 5 个非零元素预分配。

---

## 24.5 SBAIJ 预分配

```c
PetscCall(MatSeqSBAIJSetPreallocation(A, 1, 5, NULL));
PetscCall(MatMPISBAIJSetPreallocation(A, 1, 5, NULL, 5, NULL));
```

如果矩阵类型是 SBAIJ，则按：

```text
block size = 1
每个 block row 约 5 个非零 block
```

进行预分配。

---

## 24.6 SELL 预分配

```c
PetscCall(MatMPISELLSetPreallocation(A, 5, NULL, 5, NULL));
PetscCall(MatSeqSELLSetPreallocation(A, 5, NULL));
```

如果矩阵类型是 SELL，则按每行约 5 个非零元素进行预分配。

---

# 25. 报告中可以怎么写？

可以写：

```text
The benchmark considers multiple PETSc sparse matrix storage formats, including AIJ, SBAIJ, and SELL. AIJ is used as the default CSR-like baseline format. SBAIJ exploits symmetry and block structure, which may reduce storage and memory bandwidth requirements for symmetric stencil matrices. SELL stores rows in sliced ELLPACK form and may improve SIMD and threaded sparse matrix-vector multiplication for regular stencil patterns. Since the 2D five-point stencil produces at most five nonzero entries per interior row, all tested formats are preallocated with an estimated five nonzeros per row.
```

中文解释：

> 本 benchmark 比较了 PETSc 中多种稀疏矩阵存储格式，包括 AIJ、SBAIJ 和 SELL。AIJ 是默认的 CSR-like baseline 格式。SBAIJ 利用矩阵的对称性和 block structure，可能减少存储量和 memory bandwidth 压力。SELL 使用 sliced ELLPACK 的规整存储方式，对于规则 stencil matrix，可能改善 SIMD 和多线程 SpMV 性能。由于二维 five-point stencil 中内部点每行最多产生 5 个非零元素，因此代码对这些矩阵格式都使用每行约 5 个非零元素进行预分配。

---

# 术语表

| 术语 | 含义 |
|---|---|
| `Mat` | PETSc 中的矩阵对象 |
| `MatCreate` | 创建矩阵对象 |
| `MatSetSizes` | 设置矩阵的本地和全局大小 |
| `MatSetFromOptions` | 从命令行选项读取矩阵类型和相关设置 |
| `Preallocation` | 在插入矩阵元素前提前分配内存 |
| `Sparse Matrix` | 稀疏矩阵，大部分元素为 0 |
| `Dense Matrix` | 稠密矩阵，显式存储所有元素 |
| `AIJ` | PETSc 中的 CSR-like 稀疏矩阵格式 |
| `CSR` | Compressed Sparse Row，按行压缩存储 |
| `SBAIJ` | Symmetric Block AIJ，对称 block 稀疏矩阵格式 |
| `SELL` | Sliced ELLPACK，适合规整稀疏结构和 SIMD 的格式 |
| `Seq` | Sequential，单进程版本 |
| `MPI` | 多进程分布式版本 |
| `SpMV` | Sparse Matrix-Vector Multiplication，稀疏矩阵向量乘法 |
| `MatMult` | PETSc 中的矩阵向量乘法操作 |
| `Stencil` | 网格离散中一个点与邻居点的连接模式 |
| `five-point stencil` | 二维中连接自己、上、下、左、右的 stencil |
| `block size` | block sparse matrix 中每个 block 的大小 |
| `padding` | 为了对齐和规整存储而填充的额外位置 |
| `cache locality` | 数据访问是否有利于 CPU cache |
| `SIMD` | Single Instruction Multiple Data，单指令多数据并行 |
| `memory bandwidth` | 内存带宽，单位时间能从内存读取或写入的数据量 |

---

# 核心结论

1. `AIJ` 是最通用的 PETSc 稀疏矩阵格式，基本等价于 CSR，适合作为 benchmark baseline。

2. `SBAIJ` 适合对称矩阵，可以只存储一半矩阵，并且支持 block structure。

3. `SELL` 是更规整的 sliced ELLPACK 格式，适合规则 stencil、SIMD、GPU 和 threaded SpMV。

4. `Seq` 表示单进程矩阵，`MPI` 表示多进程分布式矩阵。

5. 代码中反复出现的 `5` 来自 2D five-point stencil，因为内部点最多连接 5 个位置。

6. `MatSetFromOptions` 允许通过命令行切换矩阵格式，例如：

```bash
-mat_type aij
-mat_type sbaij
-mat_type sell
```

7. Preallocation 不改变数学问题本身，但会显著影响矩阵装配性能和整体 benchmark 稳定性。

8. 不同矩阵格式表示的数学矩阵可以相同，但它们在内存中的存储方式不同，因此性能可能不同。

---

# 常见误区

## 误区 1：这些 preallocation 函数创建了多个矩阵

错误理解：

> 代码创建了 AIJ、SBAIJ、SELL 三个矩阵。

正确理解：

> 代码只创建了一个矩阵 `A`，但是为不同可能的矩阵类型都设置了预分配方式。

---

## 误区 2：AIJ、SBAIJ、SELL 是不同数学矩阵

错误理解：

> 使用不同 `-mat_type` 就是在求解不同的数学问题。

正确理解：

> 它们可以表示同一个数学矩阵，只是存储格式不同。

---

## 误区 3：SBAIJ 一定比 AIJ 快

错误理解：

> SBAIJ 少存一半，所以一定更快。

正确理解：

> SBAIJ 可能减少内存占用，但实际性能还取决于 kernel 实现、cache、通信、block size 和 solver 行为。

---

## 误区 4：SELL 一定比 AIJ 快

错误理解：

> SELL 更适合 SIMD，所以一定更快。

正确理解：

> SELL 适合行结构规整的矩阵。如果行长度差异很大，padding 可能浪费空间，性能不一定更好。

---

## 误区 5：MPIAIJ 里的两个 `5` 表示每行总共有 10 个非零元素

错误理解：

```c
MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL);
```

表示每行有 10 个非零元素。

正确理解：

> 第一个 `5` 是 diagonal part 的预留，第二个 `5` 是 off-diagonal part 的预留。这是对分布式矩阵两部分分别预分配，不是说数学上每行一定有 10 个非零元素。

---

## 误区 6：Preallocation 会改变计算结果

错误理解：

> 预分配不同会导致求解结果不同。

正确理解：

> Preallocation 只影响内存分配和装配效率，不应该改变数学矩阵和最终解。

---

# 自测问题

## 问题 1

为什么 2D five-point stencil 中每一行通常最多有 5 个非零元素？

答案要点：

```text
因为每个内部点连接自己、上、下、左、右五个位置。
```

---

## 问题 2

`MatSetFromOptions(A)` 的作用是什么？

答案要点：

```text
允许用户通过命令行选项设置矩阵类型，例如 -mat_type aij、-mat_type sbaij、-mat_type sell。
```

---

## 问题 3

AIJ 和 CSR 是什么关系？

答案要点：

```text
AIJ 是 PETSc 中的通用稀疏矩阵格式，基本等价于 compressed sparse row / CSR。
```

---

## 问题 4

为什么 SBAIJ 适合对称矩阵？

答案要点：

```text
因为对称矩阵满足 A_ij = A_ji，SBAIJ 可以只存储一半矩阵，从而减少存储量。
```

---

## 问题 5

SELL 为什么可能适合 stencil matrix？

答案要点：

```text
因为 stencil matrix 每行非零元素数量通常比较接近，SELL 的 sliced ELLPACK 存储方式更规整，可能改善 SIMD 和 SpMV 性能。
```

---

## 问题 6

SeqAIJ 和 MPIAIJ 的区别是什么？

答案要点：

```text
SeqAIJ 是单进程 AIJ 矩阵，MPIAIJ 是多进程分布式 AIJ 矩阵。
```

---

## 问题 7

`MatMPIAIJSetPreallocation(A, 5, NULL, 5, NULL)` 中两个 `5` 分别表示什么？

答案要点：

```text
第一个 5 表示 diagonal part 每行预留 5 个非零元素，第二个 5 表示 off-diagonal part 每行预留 5 个非零元素。
```

---

## 问题 8

为什么不同矩阵格式可能影响 Krylov solver 的性能？

答案要点：

```text
因为 Krylov solver 中反复执行 SpMV / MatMult，而矩阵存储格式会影响内存访问、cache locality、SIMD、MPI communication 和 OpenMP 线程效率。
```

---

## 问题 9

数学矩阵和矩阵存储格式有什么区别？

答案要点：

```text
数学矩阵描述的是线性系统本身，矩阵存储格式描述的是这个矩阵在计算机内存中的组织方式。
```

---

## 问题 10

在 PETSc benchmark 中，为什么 AIJ 通常应该作为 baseline？

答案要点：

```text
因为 AIJ 是最通用、最常用、最默认的 CSR-like 稀疏矩阵格式，适合作为其他格式的性能比较基准。
```