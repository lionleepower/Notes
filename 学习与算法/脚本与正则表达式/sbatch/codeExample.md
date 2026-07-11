```code
#!/usr/bin/env bash

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

##SBATCH --exclusive

  

set -euo pipefail

  

module purge

module load PrgEnv-gnu

module load cray-mpich

  

# Always write outputs under the project root.

PROJECT_ROOT="/work/m25oc/m25oc/s2795693/petsc_omp"

cd "${PROJECT_ROOT}"

  

RUN_ROOT="${PROJECT_ROOT}/runs/size_grid"

RES_DIR="${PROJECT_ROOT}/results/size_grid"

mkdir -p "${RUN_ROOT}" "${RES_DIR}"

  

# PETSc paths.

ROOT="/mnt/lustre/a2fs-work4/work/m25oc/m25oc/s2795693/petsc_omp"

export PETSC_DIR="${ROOT}/src/petsc"

export PETSC_ARCH="arch-omp-opt"

  

SIZE_TABLE="${SIZE_TABLE:-${PROJECT_ROOT}/scripts/ex2_problem_sizes.csv}"

  

# Start conservatively by default. Override at submission time, for example:

#   SCALES="large very-large" sbatch scripts/run_ex2_size_grid.sbatch

#   SCALES="small medium-small medium large very-large" REPEAT=3 sbatch scripts/run_ex2_size_grid.sbatch

SCALES="${SCALES:-small}"

REPEAT="${REPEAT:-1}"

  

# Single ARCHER2 node has 128 physical cores.

MAX_CORES="${MAX_CORES:-128}"

RANKS="${RANKS:-1 2 4 8 16 32 64 128}"

THREADS="${THREADS:-1 2 4 8 16 32 64 128}"

  

read -r -a RANKS_LIST <<< "${RANKS}"

read -r -a THREADS_LIST <<< "${THREADS}"

  

export OMP_PLACES=cores

export OMP_PROC_BIND=close

  

OUTCSV="${RES_DIR}/size_grid_ex2_${PETSC_ARCH}_job${SLURM_JOB_ID}.csv"

echo "timestamp,jobid,nodelist,nodes,scale,m,n,unknowns,ranks,threads,total_cores,rep,time_sec,iterations,error_norm,logfile" > "${OUTCSV}"

  

ts() { date -Iseconds; }

  

want_scale() {

  local needle="$1"

  local scale

  for scale in ${SCALES}; do

    [[ "${scale}" == "${needle}" ]] && return 0

  done

  return 1

}

  

echo "============================================================"

echo "PETSc ex2 problem-size x rank x thread grid study"

echo "JobID=${SLURM_JOB_ID}"

echo "PROJECT_ROOT=${PROJECT_ROOT}"

echo "RUN_ROOT=${RUN_ROOT}"

echo "RES_DIR=${RES_DIR}"

echo "PETSC_DIR=${PETSC_DIR}"

echo "PETSC_ARCH=${PETSC_ARCH}"

echo "SIZE_TABLE=${SIZE_TABLE}"

echo "SCALES=${SCALES}"

echo "REPEAT=${REPEAT}  MAX_CORES=${MAX_CORES}"

echo "RANKS=${RANKS}"

echo "THREADS=${THREADS}"

echo "NodeList=${SLURM_JOB_NODELIST:-UNKNOWN}"

echo "CSV=${OUTCSV}"

echo "============================================================"

  

if [[ ! -f "${SIZE_TABLE}" ]]; then

  echo "[ERROR] SIZE_TABLE does not exist: ${SIZE_TABLE}"

  exit 1

fi

  

# Build ex2 once.

cd "${PETSC_DIR}/src/ksp/ksp/tutorials"

make -j PETSC_DIR="${PETSC_DIR}" PETSC_ARCH="${PETSC_ARCH}" ex2

cd "${PROJECT_ROOT}"

  

ran_any_scale=0

  

while IFS=, read -r SCALE M N UNKNOWNS; do

  [[ "${SCALE}" == "scale" ]] && continue

  [[ -z "${SCALE}" ]] && continue

  

  if ! want_scale "${SCALE}"; then

    continue

  fi

  

  ran_any_scale=1

  RUN_DIR="${RUN_ROOT}/${SCALE}"

  mkdir -p "${RUN_DIR}"

  

  echo "============================================================"

  echo "[SCALE] ${SCALE}: M=${M} N=${N} unknowns=${UNKNOWNS}"

  echo "Logs under: ${RUN_DIR}"

  echo "============================================================"

  

  for R in "${RANKS_LIST[@]}"; do

    (( R > MAX_CORES )) && continue

  

    MAX_T=$(( MAX_CORES / R ))

    (( MAX_T < 1 )) && continue

  

    for T in "${THREADS_LIST[@]}"; do

      (( T > MAX_T )) && continue

  

      P=$(( R * T ))

  

      export OMP_NUM_THREADS="${T}"

      export SRUN_CPUS_PER_TASK="${T}"

  

      echo "------------------------------------------------------------"

      echo "[CONFIG] scale=${SCALE} ranks=${R} threads=${T} total_cores=${P}"

      echo "  OMP_NUM_THREADS=${OMP_NUM_THREADS} SRUN_CPUS_PER_TASK=${SRUN_CPUS_PER_TASK}"

      echo "------------------------------------------------------------"

  

      for rep in $(seq 1 "${REPEAT}"); do

        LOG="${RUN_DIR}/ex2_${PETSC_ARCH}_${SCALE}_r${R}_t${T}_p${P}_m${M}_n${N}_job${SLURM_JOB_ID}_rep${rep}.log"

  

        echo "[RUN] $(ts) scale=${SCALE} r=${R} t=${T} p=${P} rep=${rep}"

        echo "  logfile=${LOG}"

  

        srun --nodes=1 --ntasks="${R}" --cpus-per-task="${T}" --exact \

             --hint=nomultithread --distribution=block:block \

             "${PETSC_DIR}/src/ksp/ksp/tutorials/ex2" \

             -m "${M}" -n "${N}" -ksp_converged_reason -log_view \

          | tee "${LOG}"

  

        TIMESEC="$(awk '/Time \(sec\):/{t=$3} END{print t}' "${LOG}")"

        ITER="$(awk '/Norm of error/{iter=$NF} END{print iter}' "${LOG}")"

        ERRNORM="$(awk '/Norm of error/{err=$(NF-3)} END{print err}' "${LOG}")"

  

        if [[ -z "${TIMESEC}" ]]; then

          echo "[ERROR] No 'Time (sec):' parsed from ${LOG} (scale=${SCALE} r=${R} t=${T} rep=${rep})."

          echo "[ERROR] Aborting to avoid corrupting CSV."

          exit 1

        fi

  

        echo "$(ts),${SLURM_JOB_ID},${SLURM_JOB_NODELIST:-UNKNOWN},${SLURM_JOB_NUM_NODES},${SCALE},${M},${N},${UNKNOWNS},${R},${T},${P},${rep},${TIMESEC},${ITER},${ERRNORM},${LOG}" >> "${OUTCSV}"

      done

    done

  done

done < "${SIZE_TABLE}"

  

if (( ran_any_scale == 0 )); then

  echo "[ERROR] No matching scales found in ${SIZE_TABLE} for SCALES='${SCALES}'."

  exit 1

fi

  

echo "============================================================"

echo "Completed"

echo "CSV written: ${OUTCSV}"

echo "Logs under: ${RUN_ROOT}"

echo "============================================================"
```