#!/bin/bash

# Script di benchmarking per Password Decryption
# Confronta versione sequenziale vs parallela (con e senza WAIT) con diversi numeri di thread
# Analizza: Speedup, Execution Time, Scalability, Efficiency

set -e  # Exit on error

# Colori per output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Configurazione
BUILD_DIR="cmake-build-debug"
SEQUENTIAL_PROG="Password8Sequenziale"
PARALLEL_PROG="Password8ParallelRandomPassword"
PARALLEL_NOWAIT_PROG="Password8ParallelRandomPasswordNOWAIT"
RESULTS_DIR="benchmark_results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
CSV_FILE="${RESULTS_DIR}/benchmark_${TIMESTAMP}.csv"

# Array di numeri di thread da testare
THREAD_COUNTS=(1 2 4 8 16 20)

# Numero di esecuzioni per media (ridotto per test più veloci)
NUM_RUNS=1

echo -e "${BOLD}${CYAN}=================================================${NC}"
echo -e "${BOLD}${CYAN}  Password Decryption - Benchmark Suite${NC}"
echo -e "${BOLD}${CYAN}=================================================${NC}"
echo ""

# Crea directory per risultati
mkdir -p "${RESULTS_DIR}"

# Verifica che i programmi esistano
if [ ! -f "${BUILD_DIR}/${SEQUENTIAL_PROG}" ]; then
    echo -e "${RED}ERRORE: ${SEQUENTIAL_PROG} non trovato in ${BUILD_DIR}${NC}"
    echo "Compila prima il progetto!"
    exit 1
fi

if [ ! -f "${BUILD_DIR}/${PARALLEL_PROG}" ]; then
    echo -e "${RED}ERRORE: ${PARALLEL_PROG} non trovato in ${BUILD_DIR}${NC}"
    echo "Compila prima il progetto!"
    exit 1
fi

if [ ! -f "${BUILD_DIR}/${PARALLEL_NOWAIT_PROG}" ]; then
    echo -e "${RED}ERRORE: ${PARALLEL_NOWAIT_PROG} non trovato in ${BUILD_DIR}${NC}"
    echo "Compila prima il progetto!"
    exit 1
fi

# Abilita cancellazione OpenMP
export OMP_CANCELLATION=true

echo -e "${GREEN}✓ Tutti i programmi trovati${NC}"
echo -e "${GREEN}✓ OpenMP cancellation abilitata${NC}"
echo -e "${YELLOW}Numero di esecuzioni per test: ${NUM_RUNS}${NC}"
echo ""

# Inizializza CSV con metadata e header
echo "# Password Decryption - Benchmark Results" > "${CSV_FILE}"
echo "# Data: $(date)" >> "${CSV_FILE}"
echo "# Numero di run per test: ${NUM_RUNS}" >> "${CSV_FILE}"
echo "# Thread testati: ${THREAD_COUNTS[*]}" >> "${CSV_FILE}"
echo "# OpenMP Cancellation: ${OMP_CANCELLATION}" >> "${CSV_FILE}"
echo "#" >> "${CSV_FILE}"
echo "Version,Threads,AvgTime(s),StdDev(s),Speedup,Efficiency(%),Scalability,Passwords/sec" >> "${CSV_FILE}"

# Array per memorizzare i risultati
declare -A execution_times
declare -A execution_times_nowait
declare -A speedups
declare -A speedups_nowait
declare -A efficiencies
declare -A efficiencies_nowait
declare -A passwords_per_sec
declare -A passwords_per_sec_nowait

#################################################
# FASE 1: Esecuzione Sequenziale (Baseline)
#################################################

echo -e "${BOLD}${MAGENTA}[FASE 1] Esecuzione Sequenziale (Baseline)${NC}"
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

sequential_times=()
sequential_passwords=()

for run in $(seq 1 ${NUM_RUNS}); do
    echo -e "${YELLOW}Run ${run}/${NUM_RUNS}...${NC}"

    # Esegui e cattura output
    output=$(cd "${BUILD_DIR}" && ./${SEQUENTIAL_PROG} 2>&1)

    # Estrai tempo di esecuzione
    time=$(echo "$output" | grep "Tempo totale impiegato:" | awk '{print $4}')

    # Estrai password/secondo
    pass_sec=$(echo "$output" | grep "Password testate/secondo:" | awk '{print $3}')

    sequential_times+=($time)
    sequential_passwords+=($pass_sec)

    echo -e "  ${GREEN}Tempo: ${time}s${NC}"
done

# Calcola media e deviazione standard per sequenziale
seq_sum=0
seq_pass_sum=0
for t in "${sequential_times[@]}"; do
    seq_sum=$(echo "$seq_sum + $t" | bc -l)
done
for p in "${sequential_passwords[@]}"; do
    seq_pass_sum=$(echo "$seq_pass_sum + $p" | bc -l)
done

T_sequential=$(echo "scale=3; $seq_sum / ${NUM_RUNS}" | bc -l)
avg_seq_pass=$(echo "scale=0; $seq_pass_sum / ${NUM_RUNS}" | bc -l)

# Calcola deviazione standard
variance=0
for t in "${sequential_times[@]}"; do
    diff=$(echo "$t - $T_sequential" | bc -l)
    variance=$(echo "$variance + ($diff * $diff)" | bc -l)
done
std_dev=$(echo "scale=3; sqrt($variance / ${NUM_RUNS})" | bc -l)

echo ""
echo -e "${BOLD}${GREEN}Risultati Sequenziali:${NC}"
echo -e "  Tempo medio: ${BOLD}${T_sequential}s${NC} (±${std_dev}s)"
echo -e "  Password/sec: ${BOLD}${avg_seq_pass}${NC}"
echo ""

# CSV entry per sequenziale
echo "Sequential,1,${T_sequential},${std_dev},1.00,100.00,1.00,${avg_seq_pass}" >> "${CSV_FILE}"

execution_times[1]=$T_sequential
speedups[1]=1.00
efficiencies[1]=100.00
passwords_per_sec[1]=$avg_seq_pass

#################################################
# FASE 2: Esecuzioni Parallele (con WAIT)
#################################################

echo -e "${BOLD}${MAGENTA}[FASE 2] Esecuzioni Parallele (Standard)${NC}"
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

for threads in "${THREAD_COUNTS[@]}"; do
    echo -e "${BOLD}${BLUE}Testing con ${threads} threads${NC}"
    echo -e "${CYAN}────────────────────────────────────────────────${NC}"

    parallel_times=()
    parallel_passwords=()

    for run in $(seq 1 ${NUM_RUNS}); do
        echo -e "${YELLOW}Run ${run}/${NUM_RUNS}...${NC}"

        # Esegui programma parallelo con N thread
        output=$(cd "${BUILD_DIR}" && ./${PARALLEL_PROG} ${threads} 2>&1)

        # Estrai tempo di esecuzione
        time=$(echo "$output" | grep "Tempo totale impiegato:" | awk '{print $4}')

        # Estrai password/secondo
        pass_sec=$(echo "$output" | grep "Password testate/secondo:" | awk '{print $3}')

        parallel_times+=($time)
        parallel_passwords+=($pass_sec)

        echo -e "  ${GREEN}Tempo: ${time}s${NC}"
    done

    # Calcola media
    par_sum=0
    par_pass_sum=0
    for t in "${parallel_times[@]}"; do
        par_sum=$(echo "$par_sum + $t" | bc -l)
    done
    for p in "${parallel_passwords[@]}"; do
        par_pass_sum=$(echo "$par_pass_sum + $p" | bc -l)
    done

    T_parallel=$(echo "scale=3; $par_sum / ${NUM_RUNS}" | bc -l)
    avg_par_pass=$(echo "scale=0; $par_pass_sum / ${NUM_RUNS}" | bc -l)

    # Calcola deviazione standard
    variance=0
    for t in "${parallel_times[@]}"; do
        diff=$(echo "$t - $T_parallel" | bc -l)
        variance=$(echo "$variance + ($diff * $diff)" | bc -l)
    done
    std_dev=$(echo "scale=3; sqrt($variance / ${NUM_RUNS})" | bc -l)

    # Calcola metriche
    speedup=$(echo "scale=2; $T_sequential / $T_parallel" | bc -l)
    efficiency=$(echo "scale=2; ($speedup / $threads) * 100" | bc -l)
    scalability=$(echo "scale=2; $speedup / $threads" | bc -l)

    # Salva risultati
    execution_times[$threads]=$T_parallel
    speedups[$threads]=$speedup
    efficiencies[$threads]=$efficiency
    passwords_per_sec[$threads]=$avg_par_pass

    # Output con info aggiuntiva per 1 thread
    echo ""
    echo -e "${BOLD}${GREEN}Risultati con ${threads} threads:${NC}"
    echo -e "  Tempo medio: ${BOLD}${T_parallel}s${NC} (±${std_dev}s)"
    echo -e "  Speedup: ${BOLD}${speedup}x${NC}"
    echo -e "  Efficienza: ${BOLD}${efficiency}%${NC}"
    echo -e "  Scalabilità: ${BOLD}${scalability}${NC}"
    echo -e "  Password/sec: ${BOLD}${avg_par_pass}${NC}"

    # Mostra overhead OpenMP per 1 thread
    if [ $threads -eq 1 ]; then
        overhead=$(echo "scale=2; (($T_parallel - $T_sequential) / $T_sequential) * 100" | bc -l)
        echo -e "  ${YELLOW}⚠️  Overhead OpenMP: ${overhead}%${NC}"
        echo -e "  ${CYAN}(Differenza tra parallelo con 1 thread vs sequenziale puro)${NC}"
    fi

    echo ""

    # CSV entry
    echo "Parallel,${threads},${T_parallel},${std_dev},${speedup},${efficiency},${scalability},${avg_par_pass}" >> "${CSV_FILE}"
done

#################################################
# FASE 3: Esecuzioni Parallele NOWAIT
#################################################

echo -e "${BOLD}${MAGENTA}[FASE 3] Esecuzioni Parallele (NOWAIT)${NC}"
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

for threads in "${THREAD_COUNTS[@]}"; do
    echo -e "${BOLD}${BLUE}Testing NOWAIT con ${threads} threads${NC}"
    echo -e "${CYAN}────────────────────────────────────────────────${NC}"

    parallel_times=()
    parallel_passwords=()

    for run in $(seq 1 ${NUM_RUNS}); do
        echo -e "${YELLOW}Run ${run}/${NUM_RUNS}...${NC}"

        # Esegui programma parallelo NOWAIT con N thread
        output=$(cd "${BUILD_DIR}" && ./${PARALLEL_NOWAIT_PROG} ${threads} 2>&1)

        # Estrai tempo di esecuzione
        time=$(echo "$output" | grep "Tempo totale impiegato:" | awk '{print $4}')

        # Estrai password/secondo
        pass_sec=$(echo "$output" | grep "Password testate/secondo:" | awk '{print $3}')

        parallel_times+=($time)
        parallel_passwords+=($pass_sec)

        echo -e "  ${GREEN}Tempo: ${time}s${NC}"
    done

    # Calcola media
    par_sum=0
    par_pass_sum=0
    for t in "${parallel_times[@]}"; do
        par_sum=$(echo "$par_sum + $t" | bc -l)
    done
    for p in "${parallel_passwords[@]}"; do
        par_pass_sum=$(echo "$par_pass_sum + $p" | bc -l)
    done

    T_parallel_nowait=$(echo "scale=3; $par_sum / ${NUM_RUNS}" | bc -l)
    avg_par_pass_nowait=$(echo "scale=0; $par_pass_sum / ${NUM_RUNS}" | bc -l)

    # Calcola deviazione standard
    variance=0
    for t in "${parallel_times[@]}"; do
        diff=$(echo "$t - $T_parallel_nowait" | bc -l)
        variance=$(echo "$variance + ($diff * $diff)" | bc -l)
    done
    std_dev=$(echo "scale=3; sqrt($variance / ${NUM_RUNS})" | bc -l)

    # Calcola metriche
    speedup_nowait=$(echo "scale=2; $T_sequential / $T_parallel_nowait" | bc -l)
    efficiency_nowait=$(echo "scale=2; ($speedup_nowait / $threads) * 100" | bc -l)
    scalability_nowait=$(echo "scale=2; $speedup_nowait / $threads" | bc -l)

    # Salva risultati
    execution_times_nowait[$threads]=$T_parallel_nowait
    speedups_nowait[$threads]=$speedup_nowait
    efficiencies_nowait[$threads]=$efficiency_nowait
    passwords_per_sec_nowait[$threads]=$avg_par_pass_nowait

    # Output con info aggiuntiva
    echo ""
    echo -e "${BOLD}${GREEN}Risultati NOWAIT con ${threads} threads:${NC}"
    echo -e "  Tempo medio: ${BOLD}${T_parallel_nowait}s${NC} (±${std_dev}s)"
    echo -e "  Speedup: ${BOLD}${speedup_nowait}x${NC}"
    echo -e "  Efficienza: ${BOLD}${efficiency_nowait}%${NC}"
    echo -e "  Scalabilità: ${BOLD}${scalability_nowait}${NC}"
    echo -e "  Password/sec: ${BOLD}${avg_par_pass_nowait}${NC}"

    # Confronto con versione standard
    if [ -n "${execution_times[$threads]}" ]; then
        improvement=$(echo "scale=2; ((${execution_times[$threads]} - $T_parallel_nowait) / ${execution_times[$threads]}) * 100" | bc -l)
        if (( $(echo "$improvement > 0" | bc -l) )); then
            echo -e "  ${GREEN}✓ Miglioramento vs Standard: ${improvement}%${NC}"
        else
            abs_improvement=$(echo "scale=2; $improvement * -1" | bc -l)
            echo -e "  ${RED}✗ Peggioramento vs Standard: ${abs_improvement}%${NC}"
        fi
    fi

    echo ""

    # CSV entry
    echo "ParallelNOWAIT,${threads},${T_parallel_nowait},${std_dev},${speedup_nowait},${efficiency_nowait},${scalability_nowait},${avg_par_pass_nowait}" >> "${CSV_FILE}"
done

#################################################
# FASE 4: Riepilogo Risultati
#################################################

echo ""
echo -e "${BOLD}${CYAN}=================================================${NC}"
echo -e "${BOLD}${CYAN}  RIEPILOGO RISULTATI${NC}"
echo -e "${BOLD}${CYAN}=================================================${NC}"
echo ""

# Tabella formattata per versione Standard
echo -e "${BOLD}VERSIONE STANDARD:${NC}"
printf "${BOLD}%-10s %-12s %-10s %-12s %-15s${NC}\n" "Threads" "Time (s)" "Speedup" "Efficiency" "Pass/sec"
echo -e "${CYAN}─────────────────────────────────────────────────────────────────${NC}"

for threads in 1 "${THREAD_COUNTS[@]}"; do
    if [ $threads -eq 1 ]; then
        time=$T_sequential
        speedup="1.00"
        efficiency="100.00"
        pass_sec=$avg_seq_pass
    else
        time=${execution_times[$threads]}
        speedup=${speedups[$threads]}
        efficiency=${efficiencies[$threads]}
        pass_sec=${passwords_per_sec[$threads]}
    fi

    printf "%-10s %-12s %-10s %-12s %-15s\n" "$threads" "$time" "${speedup}x" "${efficiency}%" "$pass_sec"
done

echo ""
echo -e "${BOLD}VERSIONE NOWAIT:${NC}"
printf "${BOLD}%-10s %-12s %-10s %-12s %-15s${NC}\n" "Threads" "Time (s)" "Speedup" "Efficiency" "Pass/sec"
echo -e "${CYAN}─────────────────────────────────────────────────────────────────${NC}"

for threads in "${THREAD_COUNTS[@]}"; do
    time=${execution_times_nowait[$threads]}
    speedup=${speedups_nowait[$threads]}
    efficiency=${efficiencies_nowait[$threads]}
    pass_sec=${passwords_per_sec_nowait[$threads]}

    printf "%-10s %-12s %-10s %-12s %-15s\n" "$threads" "$time" "${speedup}x" "${efficiency}%" "$pass_sec"
done

# Trova il migliore speedup per entrambe le versioni
best_speedup=0
best_threads=1
best_version="Sequential"

for threads in "${THREAD_COUNTS[@]}"; do
    current_speedup=${speedups[$threads]}
    if [ -n "$current_speedup" ]; then
        result=$(echo "$current_speedup > $best_speedup" | bc -l)
        if [ "$result" -eq 1 ]; then
            best_speedup=$current_speedup
            best_threads=$threads
            best_version="Standard"
        fi
    fi

    current_speedup_nowait=${speedups_nowait[$threads]}
    if [ -n "$current_speedup_nowait" ]; then
        result=$(echo "$current_speedup_nowait > $best_speedup" | bc -l)
        if [ "$result" -eq 1 ]; then
            best_speedup=$current_speedup_nowait
            best_threads=$threads
            best_version="NOWAIT"
        fi
    fi
done

echo ""
echo -e "${BOLD}${GREEN}🏆 Migliore configurazione:${NC}"
echo -e "  Versione: ${BOLD}${best_version}${NC}"
echo -e "  ${BOLD}${best_threads} threads${NC} con speedup di ${BOLD}${best_speedup}x${NC}"
if [ "$best_version" = "NOWAIT" ]; then
    echo -e "  Efficienza: ${BOLD}${efficiencies_nowait[$best_threads]}%${NC}"
else
    echo -e "  Efficienza: ${BOLD}${efficiencies[$best_threads]}%${NC}"
fi
echo ""

# Aggiungi analisi al CSV
echo "#" >> "${CSV_FILE}"
echo "# ANALISI RISULTATI" >> "${CSV_FILE}"
echo "# Migliore configurazione: ${best_version} ${best_threads} threads (Speedup: ${best_speedup}x)" >> "${CSV_FILE}"
echo "#" >> "${CSV_FILE}"

echo -e "${GREEN}✓ Risultati salvati in: ${CSV_FILE}${NC}"
echo ""
echo -e "${CYAN}Per visualizzare i grafici, esegui:${NC}"
echo -e "  ${BOLD}python3 plot_benchmark.py ${CSV_FILE}${NC}"
echo ""

