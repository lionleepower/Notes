# Common Q&A for the Presentation

  

## 1. Why did you choose PETSc for this project?


我选择 PETSc 是因为它是科学计算和并行线性求解领域中非常有代表性的库，被广泛使用，而且它最近引入了 OpenMP backend，这使它既有实际价值，也有研究价值。研究 PETSc 的表现不仅能回答一个具体的软件问题，也可能为实际用户提供配置建议。


I chose PETSc because it is a ==highly representative and widely used library in scientific computing and parallel linear solvers.== Its recently introduced OpenMP backend makes it both practically relevant and academically interesting. Studying PETSc can answer a concrete software question while also providing useful configuration guidance for real users.

  

## 2. Why start with ARCHER2?



ARCHER2 是一个非常适合起步的平台，因为它是现代 many-core NUMA 架构，节点内部的拓扑结构复杂，正好适合研究 MPI-only 和 hybrid MPI+OpenMP 在真实 HPC 系统上的表现差异。如果时间和资源允许，后续也可以扩展到 Cirrus，用来判断趋势是否依赖具体平台。


ARCHER2 is a good starting platform because it is a ==modern many-core NUMA system with a complex node-level topology==. That makes it well suited for studying the difference between MPI-only and hybrid MPI+OpenMP execution on a ==realistic HPC machine==. If time and resources allow, the study could later extend to Cirrus to assess whether the trends are platform-dependent.

  

## 3. Why are 2 to 4 threads per rank more promising than higher thread counts?



从初步结果来看，适度 threading 能减少 MPI rank 数量，从而降低通信开销，但如果线程数过高，就会引入更多同步、内存带宽竞争，以及 cache 和 NUMA locality 问题。另外，profiling 也说明 PETSc 的关键 kernel 还没有被充分并行化，所以更多线程并不一定能转化为更多性能收益。



The preliminary results suggest that moderate threading ==reduces the number of MPI ranks and therefore lowers communication overhead.== However, high thread counts introduce more synchronisation, memory bandwidth contention, and cache or NUMA locality issues. 

Profiling also shows that the key PETSc kernels are ==not yet fully parallelised==, so more threads do not necessarily translate into better performance.

  

## 4. Does this mean MPI-only is a bad choice?


不一定。MPI-only 仍然是一个非常重要而且稳定的 baseline。我的目标不是证明 hybrid 一定更好，而是找出在什么条件下 hybrid 更有优势，以及什么时候 MPI-only 仍然是更合适的选择。

  
==Not necessarily==. ==MPI-only remains a very important and stable baselin==e. 
My goal is not to prove that hybrid execution is always better, but to identify the conditions under which hybrid is advantageous and when MPI-only is still the more appropriate choice.

  

## 5. What if your final results show no clear performance improvement?


这仍然是一个有价值的结果。因为这说明 PETSc 的 OpenMP backend 至少在当前架构、当前配置或当前 kernel 实现下，并不能稳定提供收益。这样的负结果同样可以帮助解释限制因素，并为后续开发或使用提供指导。


That would still be a valuable result. It would show that, at least under the current architecture, configuration, or kernel implementation, the PETSc OpenMP backend does not provide consistent benefits. Such a negative result is still useful because it helps explain the limiting factors and can guide future development or usage.

  
## 6. How do you make sure your results are fair and reproducible?


我会使用统一的问题设置、统一的软件环境，并对关键配置进行重复实验。同时，我会报告 runtime、speedup 和 efficiency，而不是只给出单次最好结果。这样可以减少偶然波动带来的误导。



I ensure fairness and reproducibility by using the ==same problem setup==, the same software environment, and repeated runs for key configurations. 

I also report runtime, speedup, and efficiency rather than only the best single run. This reduces the risk of drawing conclusions from random variation.

  

## 7. Why not test every possible configuration exhaustively?

理论上可以，但在项目时间和 HPC 资源有限的情况下，完全穷举并不现实。更合理的做法是先通过 preliminary results 缩小到有代表性的配置范围，再在这个范围内做更系统的分析。


==In theory that would be possible==, but with limit==ed project time and HPC resources==, exhaustive exploration is not realistic. 

A more practical approach is to use the preliminary results to ==narrow the search to representative configurations== and then study those systematically.

  
## 8. Why is profiling necessary if you already have runtime results?


因为 runtime 只能告诉我们“哪个更快”，但不能解释“为什么更快或者为什么更慢”。profiling 能帮助我区分时间花在 MPI、OpenMP 还是串行部分，也能定位像 VecMDot 和 PCApply 这样的关键 bottleneck。



Runtime results can tell us which configuration is faster, but they do not explain why. 
Profiling helps separate time spent in MPI, OpenMP, and serial regions, and it also identifies important bottlenecks such as VecMDot and PCApply.

  

## 9. Are your current results general, or only valid for one benchmark?


目前的结果主要基于 PETSc 的 `ex2` benchmark，所以它们首先反映的是这个 benchmark 的行为。后续工作会尝试通过更大的 problem sizes、更多节点以及可能的其他 solver 或设置，来判断这些趋势有多普遍。



At the moment, the results are primarily based on PETSc's `ex2` benchmark, so they first reflect the behaviour of that benchmark. 
Later work will test larger problem sizes, more nodes, and possibly other solvers or settings to assess how general these trends really are.

  

## ==10. What is the main contribution of your project?==


我认为主要贡献有两个。第一，是对 PETSc OpenMP backend 在目标 HPC 平台上进行系统、可复现的性能评估，当前以 ARCHER2 为主要起点。第二，是将 benchmark 结果和 profiling 证据结合起来，解释性能趋势背后的原因，而不仅仅是报告数字。


I see two main contributions. 
First, ==a systematic and reproducible performance evaluation of the PETSc OpenMP backend on target HPC platforms==, starting with ARCHER2. 

Second, ==combining benchmark results with profiling evidence to explain the reasons behind the observed trends==, rather than only reporting performance numbers.

  

## 11. What optimisation are you hoping to explore later?

如果时间允许，我希望进一步分析像 `VecMDot` 这样的热点 kernel，看看它们为什么没有充分利用 OpenMP，以及是否可以通过更好的并行化或执行方式改进整体性能。

  
If time allows, I would like to ==investigate hotspot kernels== such as `VecMDot` in more detail, to understand why they do not exploit OpenMP effectively and whether better parallelisation or execution strategies could improve overall performance.

## 12. What is your minimum viable outcome if some risks happen?


最小可交付成果是一个系统的单节点 benchmark study，包含 MPI-only 和一组代表性的 hybrid 配置，并结合基本 profiling 或 timing analysis 给出解释。即使更深入的多节点测试或优化受限，这样的结果依然可以回答核心研究问题的一部分。


The minimum viable outcome is a ==systematic single-node benchmark study including MPI-only and a representative set of hybrid configurations==, supported by basic profiling or timing analysis. Even if more extensive multi-node testing or optimisation is limited, this would still address an important part of the core research question.

  

## 13. Why consider alternative PETSc sparse matrix formats such as AIJSELL or SELL?


因为 sparse matrix format 会影响内存访问模式、cache locality 和向量化机会，而这些因素也会影响 OpenMP backend 的表现。如果只测试默认格式，可能会把 matrix storage 的限制误认为是 OpenMP 本身的限制。所以我会把它作为有限探索，而不是主线工作：如果初步结果显示格式可能影响性能，就测试 `AIJSELL` 或 `SELL` 来判断趋势是否依赖矩阵格式。



Sparse matrix formats can affect ==memory access pattern==s, cache locality, and opportunities for vectorisation, all of which can influence the behaviour of the OpenMP backend.

If only the default format is tested, a limitation caused by matrix storage could be mistaken for a limitation of OpenMP itself. I therefore treat this as a limited exploration rather than the main focus: if preliminary results suggest that the format matters, I will test `AIJSELL` or `SELL` to see whether the observed trends depend on the matrix format.