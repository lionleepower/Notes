下面这张表就是你考前最后一眼版。重点看**“看到题目怎么反应”**这一列。

# TTDS 考前最后知识汇总表

| 模块                            | 必须记住的核心                                              | 公式 / 关键词                                                                  | 看到题目怎么反应                                                             |
| ----------------------------- | ---------------------------------------------------- | ------------------------------------------------------------------------- | -------------------------------------------------------------------- |
| **IR 基本概念**                   | IR 是从大规模、非结构化文档中找到满足 information need 的材料            | document, query, relevance, information need                              | 问“IR vs Find/DB”时：IR 不是精确匹配，处理模糊需求、自然语言和相关性                          |
| **Precision**                 | 返回的文档里，有多少是相关的                                       | (P=\frac{TP}{TP+FP})                                                      | “误报成本高”时重视 Precision，例如 spam 自动删除、医学诊断直接行动                           |
| **Recall**                    | 所有相关文档里，找回了多少                                        | (R=\frac{TP}{TP+FN})                                                      | “不能漏掉”时重视 Recall，例如法律检索、misinformation moderation                    |
| **F1 / F-beta**               | Precision 和 Recall 的平衡                               | (F1=\frac{2PR}{P+R})                                                      | 想更重视 Recall 用 (F_\beta), (\beta>1)；更重视 Precision 用 (\beta<1)         |
| **AP / MAP**                  | AP 看 relevant documents 出现得早不早；MAP 是多个 query 的 AP 平均 | AP = relevant rank 上的 precision 平均                                        | 给 ranked list，先找所有 R 的位置，再算这些位置的 P@k，最后平均                            |
| **MRR**                       | 只关心第一个正确答案出现多早                                       | (RR=\frac{1}{rank}), (MRR=RR平均)                                           | QA / fact lookup / known-item search 用 MRR                           |
| **DCG / nDCG**                | graded relevance + 排名折扣                              | (DCG=rel_1+\sum_{i=2}^{k}\frac{rel_i}{\log_2 i}), (nDCG=\frac{DCG}{IDCG}) | 有 0/1/2/3 这种相关性等级时，用 nDCG                                            |
| **TF-IDF**                    | 文档内出现越多越重要；集合中越稀有越重要                                 | (tfidf=(1+\log tf)\times \log(N/df))                                      | 先算 df/idf，再算每个文档 query terms 的权重和                                    |
| **VSM**                       | 把 query 和 document 表示成向量，比较相似度                       | vector space, cosine similarity                                           | 问“文档如何表示”时：Bag-of-Words 向量，term 是维度                                  |
| **BM25**                      | TF 不无限增长；长文档要惩罚                                      | TF saturation, length normalization, IDF                                  | 比 TF-IDF 更强，因为处理重复词过多和长文档偏置                                          |
| **Language Model for IR**     | 好文档应该能“生成” query                                     | rank by (P(q                                                              | d))                                                                  |
| **Smoothing**                 | 避免 zero probability                                  | Jelinek-Mercer: document model + collection model                         | query term 不在 doc 里但在 collection 里，smoothing 后仍可非零                   |
| **Inverted Index**            | term → postings list of documents                    | dictionary, postings list                                                 | Boolean query 就做 posting lists 的 AND/OR/NOT                          |
| **Positional Index**          | term → docID + positions                             | phrase query, proximity query                                             | 问 phrase query：必须知道位置，普通 inverted index 不够                           |
| **Preprocessing**             | 把文本变成更适合匹配的 terms                                    | tokenisation, stopping, normalisation, stemming                           | 问优缺点：preprocessing 可提高匹配，但可能损失语义                                     |
| **Stopwords**                 | 高频功能词通常主题信息少                                         | the, is, of, to, from                                                     | 注意：`flights from A to B` 不能随便删 from/to                               |
| **Stemming**                  | 把不同词形合并                                              | connect, connected, connection → connect                                  | 好处提高 recall；风险 over-stemming 降低 precision                            |
| **PageRank**                  | 重要网页投出的链接更重要                                         | random surfer, damping factor, link analysis                              | 给图就按入链分数求和；问解释就说“长期随机访问概率”                                           |
| **Damping / Random Jump**     | 防止 dead end 和 spider trap                            | random jump                                                               | 页面没出链或小圈子互链时，需要随机跳转                                                  |
| **Anchor Text**               | 链接文字描述目标页面                                           | textual context of target page                                            | 目标页没写关键词，也可通过别人链接它的 anchor text 被识别                                  |
| **Web Search**                | Web 很大、质量参差、有 spam/SEO                               | massive data, SEO, spam, ads                                              | 问挑战：规模、质量控制、垃圾内容、动态变化                                                |
| **Text Classification**       | 把文本分到预定义类别                                           | binary, multi-class, multi-label                                          | 先判断任务类型，再说 feature + classifier + evaluation                         |
| **Rule-based Classification** | 手写规则分类                                               | keywords, dictionaries                                                    | 优点简单可解释；缺点 coverage 差、维护贵、不能理解上下文                                    |
| **Supervised Classification** | 用人工标注样本训练模型                                          | labelled data, features, classifier                                       | 标准流程：label data → extract features → train → predict                 |
| **Classification Features**   | 文本要变成向量                                              | BOW, TF-IDF, n-grams, embeddings                                          | 传统模型常用 BOW/TF-IDF；深度模型用 embeddings/PLM                               |
| **Accuracy 问题**               | 类别不平衡时 accuracy 可能骗人                                 | class imbalance                                                           | misinformation/spam/rare class 题：不要只用 accuracy，看 precision/recall/F1 |
| **Rocchio**                   | query 靠近 relevant docs，远离 non-relevant docs          | (q'=\alpha q+\beta avg(D_r)-\gamma avg(D_{nr}))                           | 计算题：先算 relevant 平均，再减 non-relevant 平均                                |
| **Relevance Feedback**        | 用户标注 relevant / non-relevant，系统改 query               | explicit feedback                                                         | 优点更贴近用户需求；缺点需要用户参与                                                   |
| **PRF**                       | 假设 top-k documents 是 relevant                        | pseudo/blind relevance feedback                                           | 优点自动；风险 query drift                                                  |
| **Query Drift**               | expansion terms 把 query 带偏                           | bad top-k docs                                                            | 如果初始结果错，扩展词也错，检索方向偏离原需求                                              |
| **LDA**                       | 每篇文档是 topics 的混合，每个 topic 是 words 的分布                | latent topic, document-topic, topic-word                                  | 问 latent：隐藏的 topic assignment/distributions，不是直接看到的 words            |
| **Unigram Model**             | 一个词分布生成所有词                                           | (P(w)=\prod P(w_i))                                                       | 不考虑 topic，不考虑词序                                                      |
| **Mixture of Unigrams**       | 每篇文档先选一个 topic，再生成所有词                                | one topic per document                                                    | 和 LDA 区别：LDA 一篇文档可混合多个 topics                                        |
| **Content Analysis**          | 分析文档里有哪些主题/内容                                        | coding, annotation, agreement                                             | 手工流程：读样本→定义 codes→标注→检查一致性→解决分歧→分析                                   |
| **Comparing Corpora**         | 找两个语料有什么不同                                           | word frequency, MI, chi-square, dominance score                           | 问“哪些词代表某类”：MI/chi-square 可找 discriminative words                     |
| **Cranfield Paradigm**        | 实验室式 IR evaluation                                   | documents, topics, relevance judgments, measure                           | 四件套一定背：collection, queries/topics, qrels, metric                     |
| **Pooling**                   | 多系统 top results 合并后人工判断                              | top-k pooling                                                             | 因为不能穷尽判断所有 document；未判断通常当 irrelevant                                |
| **Topic vs Query**            | topic 比 query 更完整                                    | title/query, description, narrative                                       | TREC topic 有 description/narrative；query 只是用户输入文本                    |
| **RAG**                       | 先检索，再让 LLM 基于证据回答                                    | retrieve → augment → generate                                             | 问 coding assistant：vector DB 放 local files, README, docs, tests      |
| **Zipf’s Law**                | 少数词极常见，大量词很稀有                                        | rank × frequency ≈ constant                                               | 用来解释 stopwords、rare terms、index 稀疏                                   |
| **Heap’s Law**                | collection 越大，vocabulary 越大                          | vocabulary growth                                                         | 问大规模索引：词表会随文档数增长                                                     |
| **Index Compression**         | 减少存储和 I/O                                            | delta encoding, v-byte                                                    | 好处：省空间、少 I/O；代价：需要解码计算                                               |
| **Delta Encoding**            | 存 docID 差值，不存完整 docID                                | gaps                                                                      | postings: 100, 105, 108 → 100, 5, 3                                  |
| **v-byte Encoding**           | 小数字用更少 bytes                                         | variable-byte                                                             | 和 delta encoding 常配合压缩 postings                                      |

# 最后 10 分钟只看这些

|题型|你应该立刻写什么|
|---|---|
|**问“为什么用 Recall”**|因为任务更怕 false negative，漏掉相关/有害内容代价高|
|**问“为什么不用 Accuracy”**|因为 class imbalance，模型可能全预测多数类也很高 accuracy|
|**问“BM25 比 TF-IDF 好在哪”**|TF saturation + document length normalization|
|**问“LM smoothing 为什么需要”**|避免 unseen query term 让 (P(q|
|**问“PageRank 是什么”**|random surfer 长期停留在某页面的概率，入链越重要分越高|
|**问“Rocchio 是什么”**|move query towards relevant docs and away from non-relevant docs|
|**问“PRF 风险”**|top-k 假设错会导致 query drift|
|**问“positional index 为什么需要”**|phrase query 需要词的位置和顺序|
|**问“LDA latent 是什么”**|hidden topics/topic assignments，不是直接观察到的 words|
|**问“Cranfield paradigm”**|documents + topics/queries + relevance judgments + evaluation metric|

# 考场答题模板

**如果是解释题：**

> The key idea is that ...  
> This is useful because ...  
> However, the limitation is ...  
> For example, ...

**如果是评价指标题：**

> In this task, false positives mean ..., while false negatives mean ...  
> Since the system cares more about avoiding ..., I would prioritize ...

**如果是模型比较题：**

> Model A assumes ..., while model B additionally handles ...  
> Therefore, model B is more suitable when ...

最后你只要记住一句话：

**检索题先想 ranking 和 relevance；分类题先想 TP/FP/FN；模型题先想它解决了哪个前一个模型的问题。**