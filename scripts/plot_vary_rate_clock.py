import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from IPython import embed; 
import argparse
import datetime
import os
import tiga_common
import ruamel.yaml
from pandas.api.types import CategoricalDtype

plt.rcParams.update({'font.size': 12, "font.weight":"normal"})

cat_size_order = CategoricalDtype(
            ["logical-", "skeen-", "cwcs-", 
            "ntp-", "bad-clock-", "chrony-"],
            ordered=True
        ) 

region_categories =[
    "Local-1",
    "Local-2",
    "Local-3",
    "Remote",
    "All"
]

fig_size = (3.3,2)
x_categories = ["logical-", "skeen-", 
                "ntp-", "bad-clock-", "chrony-",
                "cwcs-"
                ]
x_category_labels = ["Logical", "Skeen", 
                    "NTP", "BadClock", "Chrony",
                    "Huygens"
                ]
x_category_colors = ['b','c','m','r', 'orange', 'dodgerblue', 'slategray', 'brown',  'limegreen']
x_category_markers =['s','P', 'D', '^','*', 'x', 'X', 'v', 'o']
x_category_markersizes =[5,5,5,5,7,7,7,5,5]
x_category_linestyles = ['solid', 'solid','solid', 'solid', 'solid', 'solid', 'solid','solid', 'solid']
# ['dashed',  'dashdot', 'dotted', 'solid', 'solid']
y_category = "50p"
y_category_label = '50p Latency ($10^3$ ms)'

y_range = [0, 1000]
x_label = r"Per-Coordinator Rate ($10^3$ txns/s)"
x_ranges =[0,25]
x_tick_interval = 5
y_tick_interval =200


def MyPlot(df, region_idx, show_legend=True, tag="Tag"):
    region_category = region_categories[region_idx]
    # Define your grid layout
    fig = plt.figure(figsize=fig_size)
    # Adjust the horizontal space between subplots
    plt.subplots_adjust(wspace=0.4)  # Increase the value to add more space
    ax = fig.add_subplot(1, 1, 1) 
    xmin = x_ranges[0]
    xmax = x_ranges[1]
    ymin = y_range[0]
    ymax = y_range[1] + 10

    x_ticks = np.arange(0, xmax+1, x_tick_interval)
    y_ticks = np.arange( 0, ymax, y_tick_interval)
    ax.set_ylim([ymin, ymax])
    ax.set_yticks(y_ticks)
    ax.set_xlim([xmin, xmax])
    ax.set_xticks(x_ticks)
    
    for j in range(len(x_category_labels)):
        x = df[df["Prefix"] == x_categories[j]]["Rate"] / 1000
        # x = df[df["Prefix"] == x_categories[j]]["Throughput"]
        x_category = x_categories[j]
        x_category_label = x_category_labels[j]
        y = df[df['Prefix'] == x_category][y_category]
        alpha = 1
        # Plot on the current axis
        ax.plot(x, y, color=x_category_colors[j],
                marker=x_category_markers[j],
                markersize= x_category_markersizes[j],
                linestyle=x_category_linestyles[j],
                label=x_category_label, alpha=alpha)
    ax.set_ylabel(f"{y_category_label}",  labelpad=1) 
    fig.text(0.5, -0.1, x_label, ha='center', va='center', fontsize=12)
    # Add a single legend for all subplots
    handles, labels = ax.get_legend_handles_labels()
    if show_legend:
        fig.legend(
            handles, labels, loc='upper center', 
            bbox_to_anchor=(0.5, 1.1), ncol= len(x_category_labels), 
            fancybox=False, shadow=False, 
            prop={'size': 12, 'weight': 'normal'}, frameon=False,
            handletextpad=0.3, columnspacing=1.5)

    # Save the figure
    with_legend = "-legend" if show_legend else ""
    output_file = f"{tag}-{region_category}{with_legend}.pdf"
    plt.savefig(
        f"{tiga_common.FIGS_PATH}/{output_file}", 
        bbox_inches='tight', dpi=1200)
    tiga_common.print_info(f"saved {tiga_common.FIGS_PATH}/{output_file}")


if __name__ == '__main__':
    parser = tiga_common.argparse.ArgumentParser(
        description='Process some integers.')
    parser.add_argument(
        '--tag',  
        type=str, 
        default = "Clock",
        help='Specify the tag')
    parser.add_argument(
        '--test_plan',  
        type=str, 
        default = tiga_common.TEST_PLAN_FILE,
        help='Specify the path of the test_plan yaml file')
    args = parser.parse_args()
    test_plan_file = args.test_plan
    print("test plan file: ", test_plan_file)

    stats_csv_files = [
        test_plan_file.split(".")[0]+f"-region-{r}.csv" 
        for r in range(len(tiga_common.REGION_PROXIES))
    ] + [
        test_plan_file.split(".")[0]+f"-region-all.csv" 
    ]
    dfs = [ 
        pd.read_csv(
            f"{tiga_common.SUMMARY_STATS_PATH}/{stats_csv_file}", 
            delimiter = "\t")
        for stats_csv_file in stats_csv_files
    ]
    summary_dfs = []
    for df in dfs:
        df = df.drop('BenchType', axis=1)
        df = df.drop('TestType', axis=1)
        summary_df = df.groupby(
            ['Prefix','Rate'],  
            as_index = False).median().round(2)
        summary_df["Prefix"] = summary_df["Prefix"].astype(cat_size_order)
        summary_dfs.append(summary_df)
    os.system(f"sudo mkdir -m777 -p {tiga_common.FIGS_PATH}")
    for region_idx in range(len(summary_dfs)):
        MyPlot(
            summary_dfs[region_idx], 
            region_idx, 
            show_legend=True, 
            tag=args.tag)
    
