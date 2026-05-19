# Revised Speaker Notes for `Leyan_project_presentation.tex`

## Slide 1: Title


大家好，我是 Leyan Li。我的项目题目是 *Benchmarking the PETSc OpenMP Backend*，导师是 Paul Bartholomew。这个项目关注 PETSc 新引入的 OpenMP backend，在现代 many-core HPC 平台上是否能够带来稳定而且有实际意义的性能提升。


Hello, I'm Leyan Li. My project is *Benchmarking the PETSc OpenMP Backend*, supervised by Paul Bartholomew. 

  

## Slide 2: Project Aim


这个项目的核心目标有三个。第一，比较 PETSc 的 OpenMP backend 和传统 MPI-only 执行方式。第二，找出 hybrid MPI+OpenMP 在什么情况下有效，什么情况下无效。第三，分析背后的原因，包括硬件拓扑、NUMA locality，以及 PETSc 内部 kernel 的并行化程度。当前主要目标平台是 ARCHER2，之后如果有必要也可能扩展到 Cirrus。可以把整个项目概括成一个核心问题：PETSc OpenMP backend 到底什么时候真正有用？
  

==This project has three main aims.== 
First, to compare the PETSc OpenMP backend with traditional MPI-only execution.

Second, to identify when hybrid MPI+OpenMP is beneficial and when it is not. 

Third, to explain the observed behaviour in terms of hardware topology, NUMA locality, and the degree of parallelisation inside PETSc kernels. The initial target platform is ARCHER2, with a possible extension to Cirrus if useful. 

The core question is: when is the PETSc OpenMP backend actually useful?

  

## Slide 3: Motivation: Flat MPI vs Hybrid MPI+OpenMP


这页用一个简单图示说明 flat MPI 和 hybrid MPI+OpenMP 的区别。flat MPI 中，每个节点上会有很多 MPI ranks，因此跨节点通信 endpoint 也更多。hybrid 的想法是减少 MPI ranks，让每个 rank 内部使用 OpenMP threads 共享内存。理论上这样可以减少 MPI 通信压力，并更好地利用节点内部的 shared-memory 结构。但是否真正有效，还要看 PETSc 的 backend 和具体硬件平台。


This slide illustrates the difference between flat MPI and hybrid MPI+OpenMP. 

In flat MPI, each node contains many MPI ranks, which can increase the number of communication endpoints. 

In the hybrid approach, fewer MPI ranks are used, and OpenMP threads provide shared-memory parallelism within each rank. 

In principle, this can reduce MPI communication pressure and better exploit the shared-memory structure inside a node. Whether this works in practice depends on PETSc's backend and the target hardware platform.

  

## Slide 4: Research Gap and Proposed Approach

已有研究说明 hybrid 并行有时会比纯 MPI 更好，但这种优势通常非常依赖具体应用和硬件。这里用 [1--3] 标出主要参考，完整来源放在后面的 references slide。对这个项目来说，关键问题是 OpenMP backend 是否有实际收益，最佳 rank-thread 配置是什么，以及关键 kernel 是否被有效并行化。我的方法是系统 benchmark，再结合 profiling 解释结果。重点不是预设 hybrid 一定更好，而是测量并解释什么时候它有用。

  
Previous work suggests that hybrid parallelism can be beneficial, but the advantage is highly application- and architecture-dependent. I mark the main references as [1--3] here and keep the full details for the references slide. 

For this project, the key questions are whether the OpenMP backend provides practical benefits, which rank-thread configurations work best, and whether important kernels are effectively parallelised. 

My approach is to ==benchmark systematically== and use ==profiling to explain the results.==

The aim is not to assume hybrid execution is better, but to measure and explain when it is useful.

  

## Slide 5: Preliminary Results: Single-Node Scaling

  

**中文**

这张图展示的是单节点上的初步 strong scaling 结果。最明显的现象是，性能不仅取决于 core count，更取决于具体的 rank-thread 配置。2 到 4 threads per rank 的配置整体最有希望，而更高的线程数往往效果明显变差。这说明适度 threading 可能通过减少 MPI overhead 带来收益，但过多线程又会引入新的开销。

**English**


==This figure shows the preliminary strong-scaling results on a single node.== 

The most obvious observation is that performance depends not only on the number of cores, but also on the specific rank-thread configuration. Configurations with around 2 to 4 threads per rank look the most promising, while higher thread counts perform substantially worse. 
This suggests that moderate threading may help by reducing MPI overhead, but excessive threading introduces additional costs.

  

## Slide 6: Initial Multi-Node Indication

  

**中文**

  

这里是一个固定总核数为 128 的初步多节点比较。固定 core count 的目的，是尽量隔离 node distribution 的影响，而不是混入总并行规模变化。可以看到，2-thread 配置在 1、2、4 个节点上仍然表现最好。相比之下，MPI-only 虽然比较稳定，但随着跨节点通信出现，并没有表现出同样明显的改进。这为我的假设提供了初步支持，也就是 moderate hybrid 配置在 multi-node 环境下可能更有优势。

  

**English**

  

==This slide shows an initial multi-node comparison with a fixed total core count of 128.== 
The fixed core count helps isolate the effect of node distribution, rather than mixing it with a change in total parallelism. The 2-thread configuration remains the strongest across 1, 2, and 4 nodes. 
By contrast, MPI-only is relatively stable, but it does not show the same clear improvement once inter-node communication is introduced. This gives preliminary support to the hypothesis that moderate hybrid configurations may become more advantageous in multi-node settings.

  

## Slide 7: Profiling Evidence: OpenMP Is Active

这页先单独展示 MAP overview。它说明 OpenMP backend 确实被启用了，所以 hybrid 路径不是完全没有发生作用。但同时，大量运行时间仍然停留在 main thread 上。这里的关键提醒是：OpenMP active 不等于 OpenMP effective。这说明 runtime 结果背后还有更细的原因，不能只看总时间快慢，还需要分析时间到底花在 MPI、OpenMP 还是 serial 部分。


This slide shows the MAP overview separately. 
It confirms that the OpenMP backend is active, so the hybrid path is not simply inactive. ==However, a large fraction of runtime still remains on the main thread==. The key point is that OpenMP active does not necessarily mean OpenMP effective. This shows that we cannot rely only on total runtime; we need to understand whether time is spent in MPI, OpenMP, or serial execution.

  

## Slide 8: Profiling Evidence: Kernel Bottlenecks


这页进一步看热点函数。`VecMDot` 是一个主要瓶颈，`PCApply` 也占据了相当一部分时间。重要的是，这些函数里仍然有明显的 main-thread 或 serial 执行部分。因此，高线程数没有线性提升并不意外，因为关键 kernel 没有充分利用线程级并行。



This slide looks more closely at hotspot functions. 
`VecMDot` is a major bottleneck, and `PCApply` also contributes substantially to runtime. Importantly, these routines still include significant main-thread or serial execution. Therefore, it is not surprising that high thread counts do not produce linear speedup, because key kernels are not fully exploiting thread-level parallelism.

  

## Slide 9: Planned Project Work

后续工作主要分为四部分。首先是进一步验证单节点结果，并扩展到更大的 problem sizes；如果有帮助，也会有限探索 PETSc 的不同 sparse matrix formats，比如 `AIJSELL` 或 `SELL`。第二是做更系统的多节点 benchmark。第三是 profiling 和有限的 kernel-level analysis 或 optimisation，重点关注主要 bottleneck。最后是把 benchmark 和 profiling 的结果整合成 dissertation 和 practical guidance。


The remaining work is organised into four parts. 
==First, I will validate the single-node results further and extend them to larger problem sizes; where useful, I will also explore alternative PETSc sparse matrix formats such as `AIJSELL` or `SELL`.== 
Second, I will carry out a more systematic multi-node benchmark study. Third, I will do profiling and limited kernel-level analysis or optimisation, focusing on the main bottlenecks. Finally, I will integrate the benchmarking and profiling results into the dissertation and practical guidance.

  

## Slide 10: Project Timeline


这张甘特图展示整体时间安排。四月到五月主要是 feasibility、presentation 和考试阶段。六月重点是 single-node refinement，同时开始 profiling 和初步写作。七月主要推进 multi-node benchmarking、profiling 和 kernel analysis。八月前半段集中做最终 synthesis 和 dissertation writing，目标是在 submission 和 final talk 前完成主要分析。


This Gantt chart shows the overall schedule. April to May is mainly the feasibility, presentation, and examination period. June focuses on single-node refinement, while profiling and initial writing also begin. July is mainly for multi-node benchmarking, profiling, and kernel analysis. The first half of August is for final synthesis and dissertation writing, with the aim of completing the main analysis before submission and the final talk.

  

## Slide 11: Key Risks and Mitigation


这里我只展示关键风险，而不是把 report 里的完整风险表全部放上来，因为演讲时间比较短。主要风险包括 HPC access 受限、profiling 工具不稳定、结果没有明显性能提升，以及时间压力。这里的 L 是 likelihood，I 是 impact，都是 1 到 5 分。即使某些风险发生，fallback 策略也能保证项目至少交付一个系统、可解释的 benchmark study。



Here I show only the key risks rather than the full risk table from the report,


. The main risks are limited HPC access, profiling instability, no clear performance improvement, and schedule pressure. Here L means likelihood and I means impact, both on a scale from 1 to 5. Even if some risks occur, the fallback strategy should still allow the project to deliver a systematic and explainable benchmark study.

  

## Slide 12: Conclusion


最后总结一下，这个项目要回答的是一个很具体但也很重要的问题，也就是 PETSc 的 OpenMP backend 在现代 HPC 平台上什么时候真正有用。我的初步结果显示 moderate hybrid configuration 比激进 threading 更有前景，而 profiling 也说明了潜在瓶颈在哪里。所以这个项目不仅有明确的问题，也已经有了初步证据和清晰的后续路线。

To conclude, this project addresses a specific but important question: when is the PETSc OpenMP backend genuinely useful on modern HPC platforms? 
My preliminary results suggest that moderate hybrid configurations are more promising than aggressive threading, and the profiling evidence also indicates where the main bottlenecks lie. So the project has not only a clear research question, but also initial evidence and a well-defined next step.

  

## Slide 13: Selected References


这页列出的是演讲中用到的核心参考，而不是完整 bibliography。第一篇支持 MPI+OpenMP 在 Cray EX 这类系统上的动机，第二篇和 PETSc hybrid optimisation 直接相关，第三个是 PETSc OpenMP kernels 的官方变化记录。


This slide lists the core references used in the presentation。

  

## Slide 14: Questions

我的汇报就到这里，谢谢大家，欢迎提问。


That concludes my presentation. Thank you very much, and I'm happy to take any questions.