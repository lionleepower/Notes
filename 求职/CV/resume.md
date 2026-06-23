# LI Leyan | 李乐岩

Email: [leyanpower@gmail.com](mailto:leyanpower@gmail.com) / [3073751449@qq.com](mailto:3073751449@qq.com)  
Phone: +86 18805506898  
GitHub: [https://github.com/lionleepower](https://github.com/lionleepower)

---

## Education

**University of Edinburgh**  
**MSc High Performance Computing with Data Science**  
Sep 2025 – Present, Expected Sep 2026  
Edinburgh, United Kingdom

Relevant Topics: Parallel Computing, MPI Programming, OpenMP, Performance Engineering, Distributed Systems, High Performance Data Analytics

**University of Liverpool**  
**BSc Computer Science, First Class Honours**  
Sep 2023 – Jun 2025  
Liverpool, United Kingdom

**Xi’an Jiaotong-Liverpool University**  
**BSc Information and Computer Science**  
Sep 2021 – Jun 2023  
Suzhou, China

---

## Technical Skills

**Programming Languages:** C/C++, Python, SQL, JavaScript, HTML, CSS

**High Performance Computing:** MPI, OpenMP, PETSc, Slurm, ARCHER2, parallel program benchmarking, strong scaling analysis, hybrid MPI+OpenMP execution

**Data Analysis and Machine Learning:** PyTorch, ePyMARL, NumPy, Pandas, Matplotlib, experiment logging, performance visualisation

**Systems and Tools:** Linux, Git, Bash scripting, VS Code, REST API integration

---

## Project Experience

### PETSc Benchmark and Performance Analysis Suite

GitHub: [https://github.com/lionleepower/petsc-benchmark-suite](https://github.com/lionleepower/petsc-benchmark-suite)

**Project Description:**  
Developed a reproducible benchmarking and analysis framework for evaluating PETSc linear solver performance under different MPI and hybrid MPI+OpenMP configurations. The project focuses on large-scale performance experiments, automated job submission, runtime log processing, and scalability analysis on the ARCHER2 supercomputing system.

**Technical Stack:**  
C, PETSc, MPI, OpenMP, Slurm, Bash, Python, Pandas, Matplotlib, Linux, ARCHER2

**Work Completed:**

- Built an automated benchmarking workflow for PETSc solver experiments, covering compilation, job submission, runtime logging, result collection, and post-processing.
    
- Designed Slurm job scripts to run strong scaling and hybrid MPI+OpenMP experiments across multiple process and thread configurations on ARCHER2.
    
- Implemented Python analysis scripts to parse raw runtime logs, generate structured CSV datasets, and calculate speedup, parallel efficiency, and runtime trends.
    
- Compared MPI-only and hybrid MPI+OpenMP execution models to evaluate how communication overhead, thread-level parallelism, and process placement affect solver scalability.
    
- Analysed performance bottlenecks in large-scale sparse linear solver experiments, identifying cases where hybrid execution may reduce MPI communication but introduce thread-level overhead.
    
- Organised the project into a reproducible research workflow with clear documentation, enabling repeated experiments and consistent performance comparison.
    

---

### Value Function Factorisation in Multi-Agent Actor-Critic Methods

**Project Description:**  
Investigated value function factorisation methods in cooperative multi-agent reinforcement learning by integrating VDN-style and QMIX-style critic architectures into Multi-Agent Proximal Policy Optimization. The project aimed to study whether factorised critics can improve coordination, learning stability, and policy performance in multi-agent environments.

**Technical Stack:**  
Python, PyTorch, ePyMARL, PyMARL, Reinforcement Learning, MAPPO, VDN, QMIX, Matrix Game, Predator–Prey

**Work Completed:**

- Implemented custom MAPPO, MAPPO-VDN, and MAPPO-QMIX variants within the ePyMARL framework.
    
- Modified critic network structures to support value decomposition, including additive VDN-style factorisation and monotonic QMIX-style mixing.
    
- Designed and conducted experiments in Matrix Game and Predator–Prey environments to evaluate convergence speed, coordination behaviour, and final policy performance.
    
- Built experiment logging and result analysis workflows to compare different algorithms using metrics such as test return, convergence stability, and coordination quality.
    
- Analysed the limitations of applying value factorisation to actor-critic methods, including instability caused by critic approximation error and non-stationarity in multi-agent training.
    
- Documented implementation details, algorithmic design choices, experimental results, and limitations in a full academic dissertation.
    

---

### String Art Generator Based on Radon Transform

**Summer Undergraduate Research Programme**  
Jun 2024 – Aug 2024

**Project Description:**  
Developed an algorithmic image-to-string-art generation system using Radon transform techniques. The project converts input images into string-based artwork by extracting structural features and optimising line placement.

**Technical Stack:**  
Python, Image Processing, Radon Transform, NumPy, OpenCV, Algorithm Design

**Work Completed:**

- Designed an image preprocessing pipeline including grayscale conversion, contrast enhancement, and feature extraction.
    
- Applied Radon transform techniques to identify dominant image structures and guide string placement.
    
- Implemented parameter control mechanisms to adjust visual style, line density, contrast, and generation quality.
    
- Built an interactive workflow for testing different parameter settings and improving visual output.
    

---

## Internship Experience

### Anhui Esou Information Technology Co., Ltd

**Front-end Developer Intern**  
Aug 2023 – Sep 2023  
Anhui, China

**Work Completed:**

- Developed a parking management interface using HTML, CSS, and JavaScript.
    
- Implemented dynamic UI components including real-time vehicle count monitoring and weather information display.
    
- Integrated front-end pages with backend REST APIs to support data display and interaction.
    
- Collaborated with backend engineers to debug interface issues and improve page usability.
    

---

## Publication

**Li, Leyan.** “Convolutional Neural Networks Based Medical Image Analysis.”  
International Conference on Engineering Management, Information Technology and Intelligence, EMITI 2024.