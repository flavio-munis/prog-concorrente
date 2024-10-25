import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import json


def plotGraph(df, offset_value):

    # Set style
    sns.set_theme()
    sns.set_palette("husl")
    sns.set_style("white")

    # Create the plot
    plt.figure(figsize=(12, 8))

    # Plot lines for different batch sizes
    batch_sizes = df['batch_size'].unique()

    # Different markers for each batch size
    markers = ['o', 's', '^', 'D', 'v', '>', '<', 'p', '*']

    for batch_size, marker in zip(batch_sizes, markers):
        batch_data = df[df['batch_size'] == batch_size]
        plt.plot(batch_data['threads'], batch_data['median'], 
                 marker=marker, linewidth=2, markersize=8,
                 label=f'Batch Size {batch_size}')

    # Customize the plot
    plt.title(f'Performance by Thread Count and Batch Size (Offset={offset_value})', fontsize=14, pad=20)
    plt.xlabel('Number of Threads', fontsize=12)
    plt.ylabel('Median Time (seconds)', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(fontsize=10, title='Batch Sizes', title_fontsize=12)

    # Set x-axis ticks to show only the actual thread counts
    plt.xticks(df['threads'].unique())

    # Add minor gridlines
    plt.grid(True, which='minor', linestyle=':', alpha=0.4)

    # Adjust layout
    plt.tight_layout()

    # Show the plot
    plt.show()

# Function to analyze and print performance insights
def print_performance_insights(df, offset):
    print(f"\nPerformance Analysis by Offset: {offset}")
    print("-" * 50)

    # Best overall performance
    best_performance = df.loc[df['median'].idxmin()]
    print(f"Best Performance Configuration:")
    print(f"Threads: {best_performance['threads']}")
    print(f"Batch Size: {best_performance['batch_size']}")
    print(f"Median Time: {best_performance['median']:.6f} seconds")
    
    # Performance improvement from worst to best
    worst_performance = df['median'].max()
    improvement = ((worst_performance - best_performance['median']) / worst_performance) * 100
    print(f"\nPerformance Improvement:")
    print(f"Maximum improvement: {improvement:.2f}% from worst to best case")
    
    # Plot lines for different batch sizes
    batch_sizes = df['batch_size'].unique()

    # Thread scaling efficiency
    for batch_size in batch_sizes:
        batch_df = df[df['batch_size'] == batch_size]
        single_thread = batch_df[batch_df['threads'] == 1]['median'].iloc[0]
        max_threads = batch_df['threads'].max()
        max_thread_perf = batch_df[batch_df['threads'] == max_threads]['median'].iloc[0]
        speedup = single_thread / max_thread_perf
        
        print(f"\nBatch Size {batch_size}:")
        print(f"Thread Speedup ({max_threads} threads): {speedup:.2f}x")


def readJsonOut(path):
    with open(path, 'r') as fp:
        data = json.load(fp)

    return data


def main():
    results = readJsonOut("out.json")

    df = pd.DataFrame(results)

    unique_offsets = df['offset'].unique()

    for offset in unique_offsets:
        offset_df = df[df['offset'] == offset]
        print_performance_insights(offset_df, offset)
        plotGraph(offset_df, offset)


if __name__ == '__main__':
    main()
