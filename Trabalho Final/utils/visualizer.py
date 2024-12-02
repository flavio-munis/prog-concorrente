import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import json

def readJsonOut(path):
    with open(path, 'r') as fp:
        data = json.load(fp)
    return data

import matplotlib.pyplot as plt
import json


def load_data(sequential_data, concurrent_data):
    offsets = []
    seq_medians = []
    conc_medians = []

    for seq, conc in zip(sequential_data, concurrent_data):
        offsets.append(seq['offset'])
        seq_medians.append(seq['median'])
        conc_medians.append(conc['median'] if 'median' in conc else conc['mediam'])

    return offsets, seq_medians, conc_medians


def calculate_improvement(seq_medians, conc_medians, offsets):

    improvements = []

    for seq, conc, offset in zip(seq_medians, conc_medians, offsets): 
        best = min([seq, conc])
        worst = max([seq, conc])
        result = (worst - best) / worst * 100
        print(f'Improvement For Offset : {offset}')
        print('-------------------------------------------')
        print(f'Sequential Median Time: {seq:.6f}')
        print(f'Concurrent Median Time: {conc:.6f}\n')
        print(f'Improvement Best to Worst: {result:.2f}%\n')
        improvements.append(result)

    return improvements


def plot_comparison(offsets, seq_medians, conc_medians, improvements):
    # Set style
    sns.set_theme()
    sns.set_palette("husl")
    sns.set_style("white")

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 12))

    # Execution time comparison
    ax1.loglog(offsets, seq_medians, 'b-o', label='Sequential')
    ax1.loglog(offsets, conc_medians, 'r-o', label='Concurrent')
    ax1.set_xlabel('Offset')
    ax1.set_ylabel('Execution Time (s)')
    ax1.set_title('Sequential vs Concurrent Execution Time')
    ax1.grid(True)
    ax1.legend()

    # Improvement percentage
    ax2.semilogx(offsets, improvements, 'g-o')
    ax2.set_xlabel('Offset')
    ax2.set_ylabel('Improvement (%)')
    ax2.set_title('Performance Improvement')
    ax2.grid(True)

    plt.tight_layout()
    plt.show()


def main():
    # Read both files
    results_seq = readJsonOut("seq.json")
    results_conc = readJsonOut("conc.json")

    offsets, seq_medians, conc_medians = load_data(results_seq, results_conc)
    improvements = calculate_improvement(seq_medians, conc_medians, offsets)
    plot_comparison(offsets, seq_medians, conc_medians, improvements)

if __name__ == '__main__':
    main()
