# Benchmarking Experiment Objectives

### Dissertation submission dates
# Report, Repository, **Presentation Slides**: 
***==14 August 2026, 4pm== 
## Final talks: w/b ==17 August 2026==

## Overall Project Objective

The overall objective is to build a reproducible benchmarking study to evaluate the PETSc OpenMP backend against traditional MPI-only execution on ARCHER2. The study will compare 2D five-point stencil and 3D seven-point stencil problems, focusing on scalability, communication patterns, OpenMP thread efficiency, and practical rank-thread configuration choices.

---

# Objective 1: Extend the Benchmark from 2D to 3D

The 2D case is based on a five-point stencil, where each grid point depends on itself and its four direct neighbours. The 3D case will use a seven-point stencil, where each grid point depends on itself and its six direct neighbours along the x, y, and z directions.

This extension allows the project to compare how PETSc behaves under different stencil dimensionalities and different communication patterns.

**Measurable targets:**

|Target|Description|
|---|---|
|2D benchmark|Implement or configure a 2D five-point stencil benchmark|
|3D benchmark|Implement or configure a 3D seven-point stencil benchmark|
|Correctness|Record solver convergence, residual norm, and iteration count|
|Comparison|Compare 2D and 3D results under the same benchmarking methodology|

---

# Objective 2: Test Larger Problem Sizes

The target problem sizes should include approximately 20 million and 40 million unknowns, where “unknowns” refers to the total number of grid points or degrees of freedom.

**Suggested problem sizes:**
## 2D five-point stencil

<<<<<<< Updated upstream
|Scale|Grid size|Unknowns|
|---|---|---|
|Small|`1000 × 1000`|1.00M|
|Medium-small|`2240 × 2240`|5.02M|
|Medium|`3160 × 3160`|9.99M|
|Large|`4500 × 4500`|20.25M|
|Very large|`6400 × 6400`|40.96M|
=======
| Problem type           | Approx. 20M unknowns | Approx. 40M unknowns |     |
| ---------------------- | -------------------: | -------------------: | --- |
| 2D five-point stencil  |        `4500 × 4500` |        `6400 × 6400` |     |
| 3D seven-point stencil |    `272 × 272 × 272` |    `345 × 345 × 345` |     |
>>>>>>> Stashed changes

## 3D seven-point stencil

| Scale        | Grid size         | Unknowns |
| ------------ | ----------------- | -------- |
| Small        | `100 × 100 × 100` | 1.00M    |
| Medium-small | `171 × 171 × 171` | 5.00M    |
| Medium       | `215 × 215 × 215` | 9.94M    |
| Large        | `272 × 272 × 272` | 20.12M   |
| Very large   | `342 × 342 × 342` | 40.00M   |
**Measurable targets:**

| Target           | Description                                                            |
| ---------------- | ---------------------------------------------------------------------- |
| Small cases 1m   | Include at least one small baseline case                               |
| Medium cases 5m  | Include at least one medium case                                       |
| Large cases 10 m | Include approximately 20M unknowns                                     |
| Very large cases | Include approximately 40M unknowns, if feasible                        |
| Stability        | Ensure runs complete successfully without memory or convergence issues |

---

# Objective 3: Compare MPI-only and Hybrid MPI+OpenMP Configurations

The experiment should test different rank-thread combinations at the same total core count, especially on a single ARCHER2 node with up to 128 cores.

**Example configurations:**

| Total cores |  MPI-only | Hybrid configurations                            |
| ----------: | --------: | ------------------------------------------------ |
|          32 |  `32 × 1` | `16 × 2`, `8 × 4`, `4 × 8`                       |
|          64 |  `64 × 1` | `32 × 2`, `16 × 4`, `8 × 8`, `4 × 16`            |
|         128 | `128 × 1` | `64 × 2`, `32 × 4`, `16 × 8`, `8 × 16`, `4 × 32` |

**Measurable targets:**

|Target|Description|
|---|---|
|MPI-only baseline|Test pure MPI configurations|
|Hybrid configurations|Test multiple MPI rank and OpenMP thread combinations|
|Fixed-core comparison|Compare configurations at the same total number of cores|
|Repetitions|Run each configuration at least 3 times|
|Variability|Report mean runtime and standard deviation|

---

# Objective 4: Measure Scalability
The main metrics should include runtime, speedup, parallel efficiency, and hybrid relative speedup.

**Metrics:**

| Metric                  | Definition                                             |
| ----------------------- | ------------------------------------------------------ |
| Runtime                 | `T(p)`                                                 |
| Speedup                 | `S(p) = T(1) / T(p)`                                   |
| Parallel efficiency     | `E(p) = S(p) / p`                                      |
| Hybrid relative speedup | `T(MPI-only) / T(hybrid)` at the same total core count |

**Measurable targets:**

|Target|Description|
|---|---|
|Runtime curves|Plot runtime against core count|
|Speedup curves|Plot speedup for both 2D and 3D problems|
|Efficiency curves|Plot parallel efficiency for both 2D and 3D problems|
|Hybrid comparison|Quantify whether hybrid execution improves over MPI-only|
|Practical threshold|Treat more than 5 percent improvement as meaningful and more than 10 percent as strong|

---

# ==Objective 5: Compare Communication Patterns between 2D and 3D Problems==
In the 2D case, each subdomain communicates with up to four neighbouring subdomains. In the 3D case, each subdomain communicates with up to six neighbouring subdomains across the faces of the local 3D block.

**Comparison points:**

|Aspect|2D five-point stencil|3D seven-point stencil|
|---|---|---|
|Stencil neighbours|4 direct neighbours|6 direct neighbours|
|Communication directions|Up to 4|Up to 6|
|Local computation|Proportional to local area|Proportional to local volume|
|Boundary communication|Proportional to boundary length|Proportional to boundary surface area|
|Communication object|Edges or lines|Faces or surfaces|

**Measurable targets:**

|Target|Description|
|---|---|
|Communication proxy|Estimate boundary-to-volume or communication-to-computation ratio|
|2D vs 3D comparison|Compare the proxy for both stencil types|
|Problem size effect|Analyse how larger problem sizes affect the communication-to-computation balance|
|Runtime explanation|Use communication pattern differences to explain performance trends|

---

# Objective 6: Use Profiling to Explain Performance Results

This should go beyond reporting runtime numbers and should identify the main performance bottlenecks.

**Suggested profiling tools:**

|Tool|Purpose|
|---|---|
|PETSc `-log_view`|PETSc-level timing and solver breakdown|
|Linaro MAP|Runtime profiling and MPI/OpenMP analysis|

**Measurable targets:**

|Target|Description|
|---|---|
|Representative profiling runs|Profile at least 4 key cases|
|Hotspot identification|Identify the top 3 expensive PETSc routines|
|OpenMP analysis|Measure useful OpenMP computation and OpenMP overhead|
|MPI analysis|Measure MPI communication or waiting time|
|Explanation|Explain at least two non-obvious or non-monotonic performance results|

Suggested profiling cases:

|Case|Purpose|
|---|---|
|2D MPI-only|Baseline comparison|
|2D hybrid MPI+OpenMP|OpenMP effect in 2D|
|3D MPI-only|3D baseline comparison|
|3D hybrid MPI+OpenMP|OpenMP effect in 3D|

---

# Objective 7: Identify Practical Rank-Thread Configuration Guidelines

The goal is not only to determine whether hybrid execution is faster, but also to identify when it is useful and when it introduces overhead.

**Measurable targets:**

|Target|Description|
|---|---|
|Best configuration table|Identify the fastest configuration for each problem type and size|
|Worst configuration table|Identify configurations that consistently perform poorly|
|Thread-count sensitivity|Compare 2, 4, 8, 16, and 32 threads per MPI rank|
|Practical recommendation|Produce 3 to 5 evidence-based configuration guidelines|
|Limitations|State when hybrid execution does not provide practical benefit|

---

# Objective 8: Ensure Reproducibility and Job-Ready Outputs
This is important both for the dissertation and for future job applications in HPC, scientific computing, and performance engineering.

**Measurable targets:**

|Deliverable|Description|
|---|---|
|Slurm scripts|Provide scripts for single-node and multi-node experiments|
|PETSc options|Record all PETSc command-line options|
|Environment record|Save modules, compiler version, MPI version, and PETSc configuration|
|Raw results|Store raw timing data in CSV or structured text files|
|Processed results|Store processed tables for runtime, speedup, and efficiency|
|Plotting scripts|Provide Python scripts for generating figures|
|Profiling summaries|Save PETSc logs and Linaro MAP summaries|
|Final report|Produce a technical report explaining methodology, results, and conclusions|

---

# Objective 9: Compare Different PETSc Matrix Representations

The ninth objective is to evaluate how different PETSc matrix representations affect MPI-only and hybrid MPI+OpenMP performance.

The current benchmark is based on stencil-derived sparse matrices. However, PETSc can represent and apply these operators in different ways. Comparing these matrix forms can help identify whether the OpenMP backend benefits depend on the matrix storage format, assembly strategy, and kernel implementation.


## Matrix forms to consider

| Matrix form              | PETSc type / approach                    | Purpose                                                        |
| ------------------------ | ---------------------------------------- | -------------------------------------------------------------- |
| Standard sparse matrix   | `MATAIJ` / `MATMPIAIJ`                   | Baseline compressed sparse row sparse matrix                   |
| Matrix-free operator     | `MATSHELL` or matrix-free PETSc approach | Tests performance without explicitly storing the sparse matrix |
| Structured-grid operator | DMDA-based matrix/operator               | Tests PETSc’s structured-grid support for stencil problems     |
| 2 st                     | SELL                                     |                                                                |
|                          |                                          |                                                                |


# Slovers
try the different one


Is more thread? 



---

## Measurable targets

| Target                     | Description                                                                          |
| -------------------------- | ------------------------------------------------------------------------------------ |
| Baseline matrix format     | Use `MATAIJ` / `MATMPIAIJ` as the main reference                                     |
| Alternative matrix formats | Test at least one or two additional matrix representations                           |
| Runtime comparison         | Compare solver runtime across matrix formats                                         |
| Memory comparison          | Record memory usage where possible                                                   |
| Kernel comparison          | Use PETSc `-log_view` to compare matrix-vector multiplication and solver setup costs |
| OpenMP sensitivity         | Check whether different matrix forms change OpenMP scaling behaviour                 |
| Applicability              | Identify which matrix forms are suitable for 2D and 3D stencil problems              |