#interview/database #interview/important

# MySQL 日志系统（undo log / redo log / binlog / 两阶段提交）

## 1. 这个问题在面试中考什么？

考三种日志各自的职责与协作：undo log 保原子性+MVCC、redo log 保持久性+崩溃恢复、binlog 用于备份和主从复制，以及三者如何靠**两阶段提交**保证 redo log 与 binlog 一致。是 InnoDB 面试的绝对核心。

## 2. 重要程度总览

|知识点|重要程度|一句话|
|---|---|---|
|三种日志是什么|必须掌握|undo(原子/MVCC) / redo(持久/恢复) / binlog(备份/复制)|
|redo log vs binlog|需要掌握|物理 vs 逻辑、循环写 vs 追加写、引擎层 vs Server 层|
|redo log vs undo log|需要掌握|事务后状态 vs 事务前状态|
|binlog 三种格式|需要掌握|STATEMENT / ROW / MIXED|
|redo log 文件与写入|需要掌握|循环写、write pos 与 checkpoint|
|为什么两阶段提交|需要掌握|保 redo/binlog 一致，避免主从数据不一致|
|两阶段提交过程|需要掌握|prepare 写 redo → 写 binlog → commit|
|undo log 如何实现 MVCC|了解即可|Read View + undo 版本链|
|为何有 binlog 还要 redo log|了解即可|MyISAM 无 crash-safe，redo 由 InnoDB 补上|
|主从复制原理与模型|了解即可|binlog 传输 + relay log 回放；同步/异步/半同步|
|组提交 / Buffer Pool|了解即可|合并刷盘减 IO / 缓存脏页|

---

## 3. MySQL 的三种日志是什么？

一句话：undo log 管回滚和 MVCC、redo log 管崩溃恢复、binlog 管备份和主从复制。

> [!important]- 必须掌握
> 日志系统第一题，必须能说清三种日志分别在哪一层、保证什么特性、用途是什么——这是理解事务和主从的地基。

> [!summary]- 核心结论
> undo log（回滚日志，InnoDB 层）：记录更新前的数据，实现原子性（回滚）和 MVCC（版本链）；redo log（重做日志，InnoDB 层，物理日志）：记录数据页做了什么修改，实现持久性和 crash-safe 崩溃恢复；binlog（归档日志，Server 层，逻辑日志）：记录所有表结构和数据变更，用于数据备份和主从复制。

> [!question]- 面试怎么答
> InnoDB 有三种关键日志。undo log 是回滚日志，事务修改数据前先把旧值记进 undo log，出错或 rollback 时用它回滚，保证原子性，同时它的版本链也是实现 MVCC 的基础。redo log 是重做日志，是物理日志，记录某个数据页做了什么修改，采用 WAL 先写日志再刷盘，即使宕机也能靠它把已提交的数据恢复出来，保证持久性和 crash-safe。binlog 是 Server 层的归档日志，是逻辑日志，记录所有的数据和表结构变更，不记查询，主要用于数据备份和主从复制。

> [!example]- 三种日志职责对照
> 
> - **undo log**：记「改之前长什么样」→ 回滚 + 快照读找旧版本。含 `trx_id`（谁改的）和 `roll_pointer`（串成版本链）。
> - **redo log**：记「对某数据页做了什么物理修改」→ 崩溃后重放恢复。配合 WAL 与后台线程刷脏页。
> - **binlog**：记「执行了什么变更」→ 复制到从库重放，实现主从一致。

> [!warning]- 常见误区
> 
> - **别把 redo log 和 undo log 搞反**：undo 保原子性（回滚），redo 保持久性（恢复）。
> - redo log 是 **InnoDB 引擎层**的、物理日志；binlog 是 **Server 层**的、逻辑日志，所有引擎都能用。
> - binlog 只记变更（DDL/DML），**不记录 select 等查询操作**。

> [!tip]- 记忆方法
> 「undo 往回看（旧值/回滚）、redo 往前冲（新值/恢复）、binlog 传出去（复制/备份）」。

> [!note]- 关键词
> undo(原子/MVCC/roll_pointer)、redo(持久/物理/WAL/crash-safe)、binlog(Server层/逻辑/备份/主从)

---

## 4. WAL 技术和 crash-safe 是什么？

一句话：WAL 先写日志再刷盘，redo log + WAL 让 MySQL 崩溃重启后已提交数据不丢，即 crash-safe。

> [!info]- 需要掌握
> 是理解 redo log 存在意义的关键概念，需要能一句话解释 WAL 和 crash-safe 的关系。

> [!summary]- 核心结论
> WAL（Write-Ahead Logging）指写操作不立刻落盘，而是先写 redo log、再由后台线程择机把脏页刷盘。发生更新时 InnoDB 先改内存并标记脏页，同时把修改记进 redo log，此时更新即完成。redo log + WAL 使 MySQL 具备 crash-safe：即使异常重启，已提交的记录也不会丢。

> [!example]- 具体例子
> 更新一行：改 Buffer Pool 内存页（标脏）→ 写 redo log →（返回成功）→ 后台线程稍后把脏页刷盘。若刷盘前宕机，重启时用已落盘的 redo log 重放，数据不丢。

> [!note]- 关键词
> WAL 先写日志、脏页、后台刷盘、crash-safe、已提交不丢

---

## 5. redo log 与 binlog 的区别？

一句话：redo log 是引擎层物理日志、循环写、用于崩溃恢复；binlog 是 Server 层逻辑日志、追加写、用于备份和复制。

> [!info]- 需要掌握
> 高频对比题，必须能从适用对象、格式、写入方式、用途四个维度区分。

> [!summary]- 核心结论
> 适用对象：binlog 是 Server 层实现、所有引擎可用，redo log 是 InnoDB 独有；格式：redo log 是物理日志（某表空间某数据页某偏移做了什么修改），binlog 是逻辑日志；写入方式：binlog 追加写、保存全量日志、写满新建文件不覆盖，redo log 循环写、空间固定、写满从头覆盖、只存未刷盘的脏页日志；用途：binlog 用于备份恢复和主从复制，redo log 用于掉电等故障恢复。

> [!example]- 对比表
> 
> |维度|redo log|binlog|
> |---|---|---|
> |所在层|InnoDB 引擎层|Server 层（所有引擎可用）|
> |日志类型|物理日志（页+偏移+修改）|逻辑日志|
> |写入方式|循环写（固定空间，写满覆盖）|追加写（写满建新文件，全量保留）|
> |保存内容|未刷盘的脏页日志|所有变更的全量日志|
> |用途|掉电/崩溃恢复（crash-safe）|备份恢复、主从复制|

> [!warning]- 常见误区
> redo log 空间有限、**循环写**，所以只能做崩溃恢复不能做归档；要做数据归档/回滚到任意时间点靠的是 **binlog（追加写全量保留）**。

> [!note]- 关键词
> 引擎层 vs Server层、物理 vs 逻辑、循环写 vs 追加写、恢复 vs 备份复制

---

## 6. redo log 与 undo log 的区别？

一句话：redo log 记事务完成后的新状态，undo log 记事务开始前的旧状态。

> [!info]- 需要掌握
> 一句话对比题，抓住「事务后 vs 事务前」即可。

> [!summary]- 核心结论
> redo log 记录的是此次事务**完成后**的数据状态（用于崩溃后把数据前滚到最新）；undo log 记录的是此次事务**开始前**的数据状态（用于回滚和快照读找旧版本）。

> [!note]- 关键词
> redo=事务后新状态/前滚、undo=事务前旧状态/回滚

---

## 7. binlog 的三种格式？

一句话：STATEMENT（记 SQL，默认）、ROW（记行变化结果）、MIXED（两者自动混用）。

> [!info]- 需要掌握
> 常见追问，需要能说出三种格式及各自的缺陷。

> [!summary]- 核心结论
> STATEMENT（默认）：记录每条修改 SQL，从库重放该 SQL；缺陷是遇到 `uuid()`、`now()` 等动态函数，主从执行结果不同会导致数据不一致。ROW：记录每行最终被改成什么样，无动态函数问题；缺陷是批量 update 会为每行生成一条记录，binlog 文件过大。MIXED：自动在 STATEMENT 和 ROW 之间选择。

> [!warning]- 常见误区
> STATEMENT 省空间但**动态函数场景会主从不一致**；ROW 安全但**批量更新会让 binlog 暴涨**——两者各有取舍，MIXED 是折中。

> [!note]- 关键词
> STATEMENT(记SQL/动态函数不一致)、ROW(记行结果/文件大)、MIXED(自动选择)

---

## 8. redo log 的文件结构与写入过程？

一句话：一组两个文件（logfile0/logfile1）循环写，用 write pos 记写入位置、checkpoint 记擦除位置。

> [!info]- 需要掌握
> 需要能讲清「循环写 + write pos 追上 checkpoint 会阻塞刷脏页」这个机制。

> [!summary]- 核心结论
> InnoDB 有 1 个 redo log 组，由 logfile0 和 logfile1 两个等大文件组成，以循环（环形）方式写：写满 logfile0 切到 logfile1，再写满又回到 logfile0。用 `write pos` 表示当前写入位置，`checkpoint` 表示可擦除位置。当 write pos 追上 checkpoint 说明 redo log 满了，MySQL 会阻塞、先把 Buffer Pool 脏页刷盘、擦除旧 redo log 记录、推进 checkpoint，然后恢复运行。

> [!example]- 具体例子
> 环形跑道：write pos 是「正在写的笔」，checkpoint 是「橡皮擦」。笔追上橡皮擦 = 没地方写了 → 停下来刷脏页、擦旧记录、橡皮擦往前挪腾出空间。

> [!warning]- 常见误区
> redo log 不是直接写磁盘——先写 **redo log buffer**，再由（正常关闭 / buffer 过半 / 后台每秒 / 事务提交等时机）持久化到磁盘。追加顺序写把随机写变顺序写，提升性能。

> [!note]- 关键词
> logfile0/1、循环环形写、write pos、checkpoint、写满阻塞刷脏页

---

## 9. 为什么需要两阶段提交？

一句话：redo log 和 binlog 独立刷盘可能半成功，不一致会导致主从数据不一致，两阶段提交保证两份日志一致。

> [!info]- 需要掌握
> 极高频题，必须能举出「只写一个日志就宕机」导致主从不一致的两个反例。

> [!summary]- 核心结论
> 事务提交要把 redo log 和 binlog 都刷盘，两者是独立逻辑、可能出现半成功。因为 redo log 影响主库数据、binlog 影响从库数据，若只写一个就宕机，主从就会不一致。两阶段提交把提交拆成 prepare 和 commit 两阶段，中间穿插写 binlog，保证两份日志要么都生效要么都不生效。

> [!example]- 两个不一致反例
> 
> - **只写了 redo，没写 binlog**：重启后 redo 能恢复主库为新值，但 binlog 没这条 → 从库还是旧值，主从不一致。
> - **只写了 binlog，没写 redo**：重启后 redo 没这条、事务无效、主库是旧值，但 binlog 有这条被复制到从库 → 从库是新值，主从不一致。

> [!warning]- 常见误区
> 两阶段提交解决的是「**redo log 与 binlog 的一致性**」，进而保证主从一致，不是解决单机事务原子性（那是 undo/redo 的事）。

> [!note]- 关键词
> redo/binlog 独立刷盘、半成功、主从不一致、prepare+commit、内部 XA

---

## 10. 两阶段提交的过程是怎样的？

一句话：prepare 写 redo（置 prepare 并刷盘）→ 写 binlog 刷盘 → commit 置 redo 为 commit。

> [!info]- 需要掌握
> 与上题配套，必须能按 prepare/commit 两阶段把顺序讲清楚，并知道以 binlog 是否写成功为准。

> [!summary]- 核心结论
> MySQL 用内部 XA 事务，binlog 作协调者、存储引擎作参与者。prepare 阶段：把内部 XA 事务 ID 写入 redo log、状态置为 prepare，并把 redo log 持久化到磁盘；commit 阶段：把 XA 事务 ID 写入 binlog 并持久化到磁盘，再调引擎接口把 redo log 状态置为 commit（此状态无需强制落盘，write 到 page cache 即可）。只要 binlog 写盘成功，即使 redo log 还是 prepare 也算事务成功。

> [!example]- 崩溃恢复判定
> 重启扫描 redo log，遇到 prepare 状态就拿 XID 去 binlog 查：
> 
> - binlog 中**没有**该 XID → redo 刷了但 binlog 没刷 → **回滚**。
> - binlog 中**有**该 XID → 两者都刷了 → **提交**。 即 prepare 状态的事务，以「binlog 里有没有对应 XID」决定提交还是回滚，从而保证一致。

> [!warning]- 常见误区
> commit 阶段把 redo 置为 commit 时**不强制刷盘**——只要 binlog 已落盘，即便 redo 还停在 prepare，崩溃恢复也会判定为成功并提交。判断基准是 binlog。

> [!note]- 关键词
> 内部 XA、binlog 协调者、prepare 写 redo 刷盘、写 binlog、commit、以 binlog XID 为准

---

## 11. 主从复制是怎么实现的？（了解）

一句话：主库写 binlog，从库 IO 线程拉取写入 relay log，SQL 线程回放 relay log 更新数据。

> [!note]- 了解即可
> 知道 binlog 传输 + relay log 回放的三步流程，以及三种复制模型的取舍即可。

> [!summary]- 核心结论
> 复制的本质是把主库 binlog 传到从库重放。主库收到提交请求后先写 binlog、再提交事务并返回成功；从库的 IO 线程连主库 log dump 线程拉取 binlog，写入本地 relay log（中继日志）；从库的回放线程读 relay log 回放 binlog，更新自己的数据，最终主从一致。

> [!example]- 三种复制模型
> 
> - **同步复制**：主库要等所有从库都复制成功才返回——性能和可用性都差，基本不用。
> - **异步复制（默认）**：主库不等 binlog 同步就返回——主库宕机可能丢数据。
> - **半同步复制**：只要一部分（如任意一个）从库复制成功就返回——兼顾性能与安全，主库宕机至少有一个从库有最新数据。

> [!note]- 关键词
> binlog 传输、IO 线程、relay log、SQL 回放线程、同步/异步(默认)/半同步

---

## 12. 两阶段提交的问题与组提交（了解）

一句话：两阶段提交每事务两次 fsync + 锁竞争激烈，组提交靠合并刷盘缓解。

> [!note]- 了解即可
> 属于深入优化，知道「问题=IO高+锁竞争」和「组提交=合并 binlog 刷盘」即可。

> [!summary]- 核心结论
> 两阶段提交两个问题：① 磁盘 IO 高——`sync_binlog=1` 和 `innodb_flush_log_at_trx_commit=1` 时每个事务至少两次 fsync（redo 一次、binlog 一次）；② 锁竞争激烈——早期用 `prepare_commit_mutex` 保证多事务两个日志提交顺序一致，高并发下争锁严重。组提交（group commit）把多个事务的 binlog 刷盘合并成一次，减少 IO。

> [!example]- 组提交三阶段
> prepare 阶段不变，把 commit 阶段拆成：flush（多事务按序把 binlog 从 cache 写入文件，不刷盘）→ sync（对 binlog 文件做一次 fsync，多事务合并刷盘）→ commit（各事务按序做 InnoDB commit）。每阶段一个队列、队首事务当 leader 领导整队，锁只保护每个队列，锁粒度减小、各阶段可并发，从而提效。

> [!note]- 关键词
> 两次 fsync、prepare_commit_mutex 锁竞争、组提交、flush/sync/commit、leader、合并刷盘

---

## 13. Buffer Pool 的作用与内容（了解）

一句话：Buffer Pool 是 InnoDB 的内存缓存，缓存数据页、索引页、Undo 页等，改数据先改缓存标脏页再异步刷盘。

> [!note]- 了解即可
> 知道「缓存加速读写 + 脏页异步刷盘」和「除数据/索引页外还缓存 Undo 页等」即可。

> [!summary]- 核心结论
> Buffer Pool 主要作用是缓存：读数据时命中缓存直接读；改数据时若在缓存中就直接改缓存页并标记为脏页（内存与磁盘不一致），不立即刷盘，由后台线程择机刷。InnoDB 申请一片连续内存按页划分成缓存页，除缓存索引页和数据页外，还缓存 Undo 页、插入缓存、自适应哈希索引、锁信息等。开启事务更新前记的 undo log 就写入 Buffer Pool 的 Undo 页面。

> [!note]- 关键词
> 内存缓存、缓存页、脏页异步刷盘、Undo 页/插入缓存/自适应哈希/锁信息

---

## 14. 最终速记版

> [!tip]- 一分钟速记
> 
> - **三日志**：undo(原子/MVCC/旧值/版本链) + redo(持久/物理/循环写/crash-safe) + binlog(Server层/逻辑/追加写/备份主从)。
> - **redo vs binlog**：引擎层 vs Server层、物理 vs 逻辑、循环写 vs 追加写、恢复 vs 复制。
> - **redo vs undo**：事务后新状态 vs 事务前旧状态。
> - **binlog 格式**：STATEMENT(动态函数不一致) / ROW(文件大) / MIXED。
> - **redo 文件**：logfile0/1 循环写，write pos 追上 checkpoint 就阻塞刷脏页。
> - **两阶段提交**：为保 redo/binlog 一致→主从一致；prepare 写 redo→写 binlog→commit，恢复时以 binlog 有无 XID 定提交/回滚。
> - **复制模型**：同步(不用) / 异步(默认/可能丢) / 半同步(折中)。
> - **优化**：组提交合并 binlog 刷盘减 IO；Buffer Pool 缓存脏页异步刷盘。

---

## 15. 自测问题

> [!question]- 点开自测
> 
> 1. 三种日志分别在哪一层、保证什么、用途是什么？
> 2. WAL 和 crash-safe 是什么关系？
> 3. redo log 和 binlog 在写入方式（循环写/追加写）上的区别导致了用途上的什么差异？
> 4. binlog 三种格式各自的缺陷是什么？
> 5. redo log 满了（write pos 追上 checkpoint）会发生什么？
> 6. 举两个「只写一个日志就宕机」导致主从不一致的例子。
> 7. 两阶段提交的完整过程是什么？崩溃恢复时凭什么决定提交还是回滚？
> 8. 同步、异步、半同步复制各有什么优缺点？
> 9. 组提交把 commit 阶段拆成哪三步？为什么能减少 IO？

---

## 16. 关联

- [[MySQL事务与隔离级别]]（undo log 版本链 + Read View 实现 MVCC；redo/undo 支撑 ACID）
- [[MySQL常见面试题]]（update 执行过程正是 undo→redo→binlog→两阶段提交）
- [[InnoDB与MyISAM]]（只有 InnoDB 有 redo/undo，才具备事务和 crash-safe）
- [[MySQL慢查询与优化]]（sync_binlog、innodb_flush_log_at_trx_commit 与磁盘 IO 调优）

---
