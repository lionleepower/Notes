
cd /work/m25oc/m25oc/s2795693/petsc_omp

# 先跑 1.00M，确认脚本、输出、时间都正常
sbatch scripts/run_ex2_size_grid.sbatch

# 再跑中等规模
SCALES="medium-small medium" sbatch scripts/run_ex2_size_grid.sbatch

# very-large 先缩小配置网格，避免一上来跑太久
SCALES="very-large" RANKS="16 32 64 128" THREADS="1 2 4 8" sbatch scripts/run_ex2_size_grid.sbatch


默认只跑 small，这是故意保守的。结果会写到 results/size_grid/，日志会按规模分到 runs/size_grid/<scale>/。

我也做了 bash -n scripts/run_ex2_size_grid.sbatch，语法检查通过。没有实际提交 Slurm job。

