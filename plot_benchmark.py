#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import sys
import os
import numpy as np

def plot_benchmark(csv_file):
    """
    Plotta i risultati del benchmark da un file CSV salvando ogni grafico in file separati
    """
    # Leggi il CSV ignorando le linee di commento
    df = pd.read_csv(csv_file, comment='#')

    if df.empty:
        print("Errore: nessun dato valido trovato nel CSV")
        return
    
    # Separa i dati per versione
    df_sequential = df[df['Version'] == 'Sequential']
    df_parallel = df[df['Version'] == 'Parallel']
    df_parallel_nowait = df[df['Version'] == 'ParallelNOWAIT']

    # Tempo sequenziale di riferimento
    seq_time = df_sequential['AvgTime(s)'].iloc[0] if not df_sequential.empty else df_parallel[df_parallel['Threads'] == 1]['AvgTime(s)'].iloc[0]
    threads = df_parallel['Threads'].unique() if not df_parallel.empty else df_parallel_nowait['Threads'].unique()

    # Prepara il nome base per i file di output
    base_output = csv_file.replace('.csv', '')

    print(f"\n{'='*70}")
    print(f"Generazione grafici da: {os.path.basename(csv_file)}")
    print(f"{'='*70}\n")

    # 1. Execution Time
    fig1, ax1 = plt.subplots(figsize=(10, 6))
    if not df_sequential.empty:
        ax1.axhline(y=df_sequential['AvgTime(s)'].iloc[0], color='black', linestyle='--',
                    linewidth=2, alpha=0.7, label='Sequential')
    if not df_parallel.empty:
        ax1.plot(df_parallel['Threads'], df_parallel['AvgTime(s)'], 'o-', linewidth=2,
                markersize=8, color='#2E86AB', label='Parallel')
    if not df_parallel_nowait.empty:
        ax1.plot(df_parallel_nowait['Threads'], df_parallel_nowait['AvgTime(s)'], 's-',
                linewidth=2, markersize=8, color='#A23B72', label='Parallel NOWAIT')
    ax1.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax1.set_ylabel('Execution Time (s)', fontweight='bold', fontsize=12)
    ax1.set_title('Execution Time vs Threads', fontweight='bold', fontsize=14)
    ax1.grid(True, alpha=0.3)
    ax1.legend(fontsize=10)
    ax1.set_xticks(threads)
    plt.tight_layout()
    output1 = f"{base_output}_execution_time.png"
    plt.savefig(output1, dpi=300, bbox_inches='tight')
    print(f"✓ Salvato: {output1}")
    plt.close()

    # 2. Speedup
    fig2, ax2 = plt.subplots(figsize=(10, 6))
    if not df_parallel.empty:
        ax2.plot(df_parallel['Threads'], df_parallel['Speedup'], 'o-', linewidth=2,
                markersize=8, color='#2E86AB', label='Parallel')
    if not df_parallel_nowait.empty:
        ax2.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Speedup'], 's-',
                linewidth=2, markersize=8, color='#A23B72', label='Parallel NOWAIT')
    # Linea ideale
    ax2.plot(threads, threads, '--', linewidth=2, alpha=0.7, color='gray', label='Ideal Speedup')
    ax2.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax2.set_ylabel('Speedup', fontweight='bold', fontsize=12)
    ax2.set_title('Speedup vs Threads', fontweight='bold', fontsize=14)
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=10)
    ax2.set_xticks(threads)
    plt.tight_layout()
    output2 = f"{base_output}_speedup.png"
    plt.savefig(output2, dpi=300, bbox_inches='tight')
    print(f"✓ Salvato: {output2}")
    plt.close()

    # 3. Efficiency
    fig3, ax3 = plt.subplots(figsize=(10, 6))
    if not df_parallel.empty:
        ax3.plot(df_parallel['Threads'], df_parallel['Efficiency(%)'], 'o-', linewidth=2,
                markersize=8, color='#6A994E', label='Parallel')
    if not df_parallel_nowait.empty:
        ax3.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Efficiency(%)'], 's-',
                linewidth=2, markersize=8, color='#BC4B51', label='Parallel NOWAIT')
    ax3.axhline(y=100, color='gray', linestyle='--', linewidth=2, alpha=0.7, label='100% Efficiency')
    ax3.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax3.set_ylabel('Efficiency (%)', fontweight='bold', fontsize=12)
    ax3.set_title('Efficiency vs Threads', fontweight='bold', fontsize=14)
    ax3.grid(True, alpha=0.3)
    ax3.legend(fontsize=10)
    ax3.set_xticks(threads)
    plt.tight_layout()
    output3 = f"{base_output}_efficiency.png"
    plt.savefig(output3, dpi=300, bbox_inches='tight')
    print(f"✓ Salvato: {output3}")
    plt.close()

    # 4. Scalability
    fig4, ax4 = plt.subplots(figsize=(10, 6))
    if not df_parallel.empty:
        ax4.plot(df_parallel['Threads'], df_parallel['Scalability'], 'o-', linewidth=2,
                markersize=8, color='#2E86AB', label='Parallel')
    if not df_parallel_nowait.empty:
        ax4.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Scalability'], 's-',
                linewidth=2, markersize=8, color='#A23B72', label='Parallel NOWAIT')
    ax4.axhline(y=1.0, color='gray', linestyle='--', linewidth=2, alpha=0.7, label='Linear Scalability')
    ax4.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax4.set_ylabel('Scalability', fontweight='bold', fontsize=12)
    ax4.set_title('Scalability vs Threads', fontweight='bold', fontsize=14)
    ax4.grid(True, alpha=0.3)
    ax4.legend(fontsize=10)
    ax4.set_xticks(threads)
    plt.tight_layout()
    output4 = f"{base_output}_scalability.png"
    plt.savefig(output4, dpi=300, bbox_inches='tight')
    print(f"✓ Salvato: {output4}")
    plt.close()

    # 5. Passwords per secondo (Throughput)
    fig5, ax5 = plt.subplots(figsize=(10, 6))
    if not df_sequential.empty:
        ax5.axhline(y=df_sequential['Passwords/sec'].iloc[0], color='black', linestyle='--',
                    linewidth=2, alpha=0.7, label='Sequential')
    if not df_parallel.empty:
        ax5.plot(df_parallel['Threads'], df_parallel['Passwords/sec'], 'o-', linewidth=2,
                markersize=8, color='#8338EC', label='Parallel')
    if not df_parallel_nowait.empty:
        ax5.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Passwords/sec'], 's-',
                linewidth=2, markersize=8, color='#F18F01', label='Parallel NOWAIT')
    ax5.set_xlabel('Number of Threads', fontweight='bold', fontsize=12)
    ax5.set_ylabel('Passwords/sec', fontweight='bold', fontsize=12)
    ax5.set_title('Throughput vs Threads', fontweight='bold', fontsize=14)
    ax5.grid(True, alpha=0.3)
    ax5.legend(fontsize=10)
    ax5.set_xticks(threads)
    ax5.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{int(x):,}'))
    plt.tight_layout()
    output5 = f"{base_output}_throughput.png"
    plt.savefig(output5, dpi=300, bbox_inches='tight')
    print(f"✓ Salvato: {output5}")
    plt.close()

    # 6. Tabella riassuntiva comparativa
    fig6, ax6 = plt.subplots(figsize=(10, 8))
    ax6.axis('tight')
    ax6.axis('off')
    
    table_data = []

    # Aggiungi sequenziale se presente
    if not df_sequential.empty:
        row = df_sequential.iloc[0]
        table_data.append([
            'Sequential',
            f"{int(row['Threads'])}",
            f"{row['AvgTime(s)']:.2f}s",
            f"{row['Speedup']:.2f}x",
            f"{row['Efficiency(%)']:.1f}%",
            f"{row['Passwords/sec']:,.0f}"
        ])

    # Aggiungi parallel e nowait per ogni thread count
    for t in threads:
        if not df_parallel.empty:
            row_p = df_parallel[df_parallel['Threads'] == t]
            if not row_p.empty:
                row_p = row_p.iloc[0]
                table_data.append([
                    'Parallel',
                    f"{int(row_p['Threads'])}",
                    f"{row_p['AvgTime(s)']:.2f}s",
                    f"{row_p['Speedup']:.2f}x",
                    f"{row_p['Efficiency(%)']:.1f}%",
                    f"{row_p['Passwords/sec']:,.0f}"
                ])

        if not df_parallel_nowait.empty:
            row_nw = df_parallel_nowait[df_parallel_nowait['Threads'] == t]
            if not row_nw.empty:
                row_nw = row_nw.iloc[0]
                table_data.append([
                    'Parallel NOWAIT',
                    f"{int(row_nw['Threads'])}",
                    f"{row_nw['AvgTime(s)']:.2f}s",
                    f"{row_nw['Speedup']:.2f}x",
                    f"{row_nw['Efficiency(%)']:.1f}%",
                    f"{row_nw['Passwords/sec']:,.0f}"
                ])

    table = ax6.table(cellText=table_data,
                     colLabels=['Version', 'Threads', 'Time', 'Speedup', 'Efficiency', 'Throughput'],
                     cellLoc='center',
                     loc='center',
                     colWidths=[0.20, 0.12, 0.15, 0.15, 0.15, 0.23])

    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 2.0)

    # Colora header
    for i in range(6):
        table[(0, i)].set_facecolor('#4A5568')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    # Colora righe alternate
    for i in range(1, len(table_data) + 1):
        for j in range(6):
            if i % 2 == 0:
                table[(i, j)].set_facecolor('#F7FAFC')
            else:
                table[(i, j)].set_facecolor('#EDF2F7')
    
    ax6.set_title('Benchmark Summary Table\nPassword Decryption Parallel Comparison',
                  fontweight='bold', fontsize=14, pad=20)

    plt.tight_layout()
    output6 = f"{base_output}_summary_table.png"
    plt.savefig(output6, dpi=300, bbox_inches='tight')
    print(f"✓ Salvato: {output6}")
    plt.close()

    # 7. Grafico combinato (opzionale - mantiene la vista d'insieme)
    fig_combined, axes = plt.subplots(2, 3, figsize=(20, 12))
    fig_combined.suptitle(f'Benchmark Results - Password Decryption Parallel Comparison\n{os.path.basename(csv_file)}',
                 fontsize=16, fontweight='bold')

    # Execution Time
    ax = axes[0, 0]
    if not df_sequential.empty:
        ax.axhline(y=df_sequential['AvgTime(s)'].iloc[0], color='black', linestyle='--',
                    linewidth=2, alpha=0.7, label='Sequential')
    if not df_parallel.empty:
        ax.plot(df_parallel['Threads'], df_parallel['AvgTime(s)'], 'o-', linewidth=2,
                markersize=8, color='#2E86AB', label='Parallel')
    if not df_parallel_nowait.empty:
        ax.plot(df_parallel_nowait['Threads'], df_parallel_nowait['AvgTime(s)'], 's-',
                linewidth=2, markersize=8, color='#A23B72', label='Parallel NOWAIT')
    ax.set_xlabel('Number of Threads', fontweight='bold', fontsize=11)
    ax.set_ylabel('Execution Time (s)', fontweight='bold', fontsize=11)
    ax.set_title('Execution Time vs Threads', fontweight='bold', fontsize=12)
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.set_xticks(threads)

    # Speedup
    ax = axes[0, 1]
    if not df_parallel.empty:
        ax.plot(df_parallel['Threads'], df_parallel['Speedup'], 'o-', linewidth=2,
                markersize=8, color='#2E86AB', label='Parallel')
    if not df_parallel_nowait.empty:
        ax.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Speedup'], 's-',
                linewidth=2, markersize=8, color='#A23B72', label='Parallel NOWAIT')
    ax.plot(threads, threads, '--', linewidth=2, alpha=0.7, color='gray', label='Ideal Speedup')
    ax.set_xlabel('Number of Threads', fontweight='bold', fontsize=11)
    ax.set_ylabel('Speedup', fontweight='bold', fontsize=11)
    ax.set_title('Speedup vs Threads', fontweight='bold', fontsize=12)
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.set_xticks(threads)

    # Efficiency
    ax = axes[0, 2]
    if not df_parallel.empty:
        ax.plot(df_parallel['Threads'], df_parallel['Efficiency(%)'], 'o-', linewidth=2,
                markersize=8, color='#6A994E', label='Parallel')
    if not df_parallel_nowait.empty:
        ax.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Efficiency(%)'], 's-',
                linewidth=2, markersize=8, color='#BC4B51', label='Parallel NOWAIT')
    ax.axhline(y=100, color='gray', linestyle='--', linewidth=2, alpha=0.7, label='100% Efficiency')
    ax.set_xlabel('Number of Threads', fontweight='bold', fontsize=11)
    ax.set_ylabel('Efficiency (%)', fontweight='bold', fontsize=11)
    ax.set_title('Efficiency vs Threads', fontweight='bold', fontsize=12)
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.set_xticks(threads)

    # Scalability
    ax = axes[1, 0]
    if not df_parallel.empty:
        ax.plot(df_parallel['Threads'], df_parallel['Scalability'], 'o-', linewidth=2,
                markersize=8, color='#2E86AB', label='Parallel')
    if not df_parallel_nowait.empty:
        ax.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Scalability'], 's-',
                linewidth=2, markersize=8, color='#A23B72', label='Parallel NOWAIT')
    ax.axhline(y=1.0, color='gray', linestyle='--', linewidth=2, alpha=0.7, label='Linear Scalability')
    ax.set_xlabel('Number of Threads', fontweight='bold', fontsize=11)
    ax.set_ylabel('Scalability', fontweight='bold', fontsize=11)
    ax.set_title('Scalability vs Threads', fontweight='bold', fontsize=12)
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.set_xticks(threads)

    # Throughput
    ax = axes[1, 1]
    if not df_sequential.empty:
        ax.axhline(y=df_sequential['Passwords/sec'].iloc[0], color='black', linestyle='--',
                    linewidth=2, alpha=0.7, label='Sequential')
    if not df_parallel.empty:
        ax.plot(df_parallel['Threads'], df_parallel['Passwords/sec'], 'o-', linewidth=2,
                markersize=8, color='#8338EC', label='Parallel')
    if not df_parallel_nowait.empty:
        ax.plot(df_parallel_nowait['Threads'], df_parallel_nowait['Passwords/sec'], 's-',
                linewidth=2, markersize=8, color='#F18F01', label='Parallel NOWAIT')
    ax.set_xlabel('Number of Threads', fontweight='bold', fontsize=11)
    ax.set_ylabel('Passwords/sec', fontweight='bold', fontsize=11)
    ax.set_title('Throughput vs Threads', fontweight='bold', fontsize=12)
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.set_xticks(threads)
    ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{int(x):,}'))

    # Mini tabella
    ax = axes[1, 2]
    ax.axis('tight')
    ax.axis('off')

    mini_table_data = []
    if not df_sequential.empty:
        row = df_sequential.iloc[0]
        mini_table_data.append(['Seq', f"{int(row['Threads'])}", f"{row['AvgTime(s)']:.2f}s",
                                f"{row['Speedup']:.2f}x", f"{row['Efficiency(%)']:.1f}%"])

    for t in threads:
        if not df_parallel.empty:
            row_p = df_parallel[df_parallel['Threads'] == t]
            if not row_p.empty:
                row_p = row_p.iloc[0]
                mini_table_data.append(['Par', f"{int(row_p['Threads'])}", f"{row_p['AvgTime(s)']:.2f}s",
                                       f"{row_p['Speedup']:.2f}x", f"{row_p['Efficiency(%)']:.1f}%"])

        if not df_parallel_nowait.empty:
            row_nw = df_parallel_nowait[df_parallel_nowait['Threads'] == t]
            if not row_nw.empty:
                row_nw = row_nw.iloc[0]
                mini_table_data.append(['NW', f"{int(row_nw['Threads'])}", f"{row_nw['AvgTime(s)']:.2f}s",
                                       f"{row_nw['Speedup']:.2f}x", f"{row_nw['Efficiency(%)']:.1f}%"])

    mini_table = ax.table(cellText=mini_table_data,
                         colLabels=['Ver', 'Thr', 'Time', 'Speedup', 'Eff'],
                         cellLoc='center', loc='center',
                         colWidths=[0.15, 0.15, 0.25, 0.23, 0.22])
    mini_table.auto_set_font_size(False)
    mini_table.set_fontsize(9)
    mini_table.scale(1, 1.8)

    for i in range(5):
        mini_table[(0, i)].set_facecolor('#4A5568')
        mini_table[(0, i)].set_text_props(weight='bold', color='white')

    for i in range(1, len(mini_table_data) + 1):
        for j in range(5):
            if i % 2 == 0:
                mini_table[(i, j)].set_facecolor('#F7FAFC')
            else:
                mini_table[(i, j)].set_facecolor('#EDF2F7')

    ax.set_title('Summary Table\n(Seq=Sequential, Par=Parallel, NW=NOWAIT)',
                 fontweight='bold', fontsize=11, pad=20)

    plt.tight_layout()
    output_combined = f"{base_output}_combined.png"
    plt.savefig(output_combined, dpi=300, bbox_inches='tight')
    print(f"✓ Salvato: {output_combined}")
    plt.close()

    # Stampa statistiche
    print("\n" + "="*70)
    print("STATISTICHE BENCHMARK")
    print("="*70)

    if not df_sequential.empty:
        print(f"\n📊 SEQUENTIAL:")
        seq = df_sequential.iloc[0]
        print(f"   Tempo: {seq['AvgTime(s)']:.3f}s")
        print(f"   Throughput: {seq['Passwords/sec']:,} passwords/sec")

    if not df_parallel.empty:
        print(f"\n📊 PARALLEL (standard):")
        best_p = df_parallel.loc[df_parallel['Speedup'].idxmax()]
        print(f"   Migliore speedup: {best_p['Speedup']:.2f}x con {int(best_p['Threads'])} threads")
        print(f"   Tempo: {best_p['AvgTime(s)']:.3f}s")
        print(f"   Efficienza: {best_p['Efficiency(%)']:.1f}%")
        print(f"   Throughput: {best_p['Passwords/sec']:,} passwords/sec")

    if not df_parallel_nowait.empty:
        print(f"\n📊 PARALLEL NOWAIT:")
        best_nw = df_parallel_nowait.loc[df_parallel_nowait['Speedup'].idxmax()]
        print(f"   Migliore speedup: {best_nw['Speedup']:.2f}x con {int(best_nw['Threads'])} threads")
        print(f"   Tempo: {best_nw['AvgTime(s)']:.3f}s")
        print(f"   Efficienza: {best_nw['Efficiency(%)']:.1f}%")
        print(f"   Throughput: {best_nw['Passwords/sec']:,} passwords/sec")

    # Confronto diretto
    if not df_parallel.empty and not df_parallel_nowait.empty:
        print(f"\n🔄 CONFRONTO (miglior configurazione):")
        if best_nw['AvgTime(s)'] < best_p['AvgTime(s)']:
            improvement = ((best_p['AvgTime(s)'] - best_nw['AvgTime(s)']) / best_p['AvgTime(s)']) * 100
            print(f"   NOWAIT è {improvement:.1f}% più veloce di Parallel standard")
            print(f"   Differenza: {best_p['AvgTime(s)'] - best_nw['AvgTime(s)']:.3f}s")
        else:
            improvement = ((best_nw['AvgTime(s)'] - best_p['AvgTime(s)']) / best_nw['AvgTime(s)']) * 100
            print(f"   Parallel standard è {improvement:.1f}% più veloce di NOWAIT")
            print(f"   Differenza: {best_nw['AvgTime(s)'] - best_p['AvgTime(s)']:.3f}s")

    print("="*70)
    print(f"\n📁 File generati:")
    print(f"   1. {os.path.basename(output1)}")
    print(f"   2. {os.path.basename(output2)}")
    print(f"   3. {os.path.basename(output3)}")
    print(f"   4. {os.path.basename(output4)}")
    print(f"   5. {os.path.basename(output5)}")
    print(f"   6. {os.path.basename(output6)}")
    print(f"   7. {os.path.basename(output_combined)} (vista combinata)")
    print("="*70 + "\n")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # Cerca l'ultimo file CSV nella directory benchmark_results
        benchmark_dir = "benchmark_results"
        if os.path.exists(benchmark_dir):
            csv_files = sorted([f for f in os.listdir(benchmark_dir) if f.endswith('.csv')])
            if csv_files:
                csv_file = os.path.join(benchmark_dir, csv_files[-1])
                print(f"Usando l'ultimo file benchmark: {csv_file}")
            else:
                print("Errore: nessun file CSV trovato in benchmark_results/")
                print("Uso: python3 plot_benchmark.py <file.csv>")
                sys.exit(1)
        else:
            print("Errore: directory benchmark_results non trovata")
            print("Uso: python3 plot_benchmark.py <file.csv>")
            sys.exit(1)
    else:
        csv_file = sys.argv[1]
    
    if not os.path.exists(csv_file):
        print(f"Errore: file {csv_file} non trovato")
        sys.exit(1)
    
    plot_benchmark(csv_file)
