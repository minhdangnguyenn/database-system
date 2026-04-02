"""
plot_benchmark.py
─────────────────────────────────────────────────────────────────────────────
Reads  benchmark_results.csv  produced by benchmark.cpp and generates two
chart files:

  1. benchmark_time_ms.png   — time_ms   vs capacity, grouped by algorithm
  2. benchmark_ns_per_op.png — ns_per_op vs capacity, grouped by algorithm

Usage:
  python3 plot_benchmark.py                        # default: benchmark_results.csv
  python3 plot_benchmark.py my_results.csv         # custom path

CSV schema expected:
  name,type,capacity,operations,key_range,time_ms,ns_per_op
  (type is either "O1" or "NAIVE")
─────────────────────────────────────────────────────────────────────────────
"""

import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np

# ── style ──────────────────────────────────────────────────────────────────
plt.style.use("seaborn-v0_8-whitegrid")
plt.rcParams.update({
    "figure.dpi":       150,
    "font.size":        11,
    "axes.titlesize":   13,
    "axes.titleweight": "bold",
    "axes.labelsize":   11,
    "xtick.labelsize":  9,
    "ytick.labelsize":  10,
    "legend.fontsize":  10,
})

# colorblind-friendly pair: blue for O1, orange for NAIVE
COLORS = {"O1": "#4C72B0", "NAIVE": "#DD8452"}
LABELS = {"O1": "O(1) Optimised", "NAIVE": "Naïve"}
FILE =  "../build/benchmark_results.csv"

# ── load data ───────────────────────────────────────────────────────────────
csv_path = sys.argv[1] if len(sys.argv) > 1 else FILE

if not os.path.exists(csv_path):
    print(f"[ERROR] File not found: {csv_path}")
    print("  Run the C++ benchmark first to produce the CSV, then re-run this script.")
    sys.exit(1)

df = pd.read_csv(csv_path)
required_cols = {"name", "type", "capacity", "operations",
                 "key_range", "time_ms", "ns_per_op"}
if not required_cols.issubset(df.columns):
    missing = required_cols - set(df.columns)
    print(f"[ERROR] Missing columns in CSV: {missing}")
    sys.exit(1)

# normalise type strings
df["type"] = df["type"].str.strip().str.upper()

types     = sorted(df["type"].unique())          # ["NAIVE", "O1"] or subset
workloads = df["name"].unique()                  # unique benchmark names

# ── helper: grouped bar chart ───────────────────────────────────────────────
def grouped_bar(metric: str, ylabel: str, title: str, out_file: str):
    """
    One group of bars per workload name.
    Within each group, one bar per algorithm type.
    X-axis labels show 'name (capacity=N)'.
    """

    # build x-labels and per-type values
    x_labels = []
    type_vals = {t: [] for t in types}

    for name in workloads:
        sub = df[df["name"] == name]
        cap = sub["capacity"].iloc[0]
        x_labels.append(f"{name}\n(cap={cap:,})")
        for t in types:
            row = sub[sub["type"] == t]
            type_vals[t].append(row[metric].iloc[0] if not row.empty else 0)

    n_groups = len(x_labels)
    n_types  = len(types)
    bar_w    = 0.35
    offsets  = np.arange(n_types) * bar_w - (n_types - 1) * bar_w / 2
    x        = np.arange(n_groups)

    fig, ax = plt.subplots(figsize=(max(10, n_groups * 2.5), 6))

    for i, t in enumerate(types):
        vals = type_vals[t]
        bars = ax.bar(
            x + offsets[i], vals, bar_w,
            label=LABELS.get(t, t),
            color=COLORS.get(t, f"C{i}"),
            alpha=0.88,
            edgecolor="white",
            linewidth=0.6,
        )
        # value labels on top of each bar
        for bar, v in zip(bars, vals):
            if v > 0:
                ax.text(
                    bar.get_x() + bar.get_width() / 2,
                    bar.get_height() * 1.012,
                    f"{v:,.0f}",
                    ha="center", va="bottom",
                    fontsize=8, color="#333333",
                )

    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.set_xticks(x)
    ax.set_xticklabels(x_labels, ha="center")
    ax.legend(loc="upper left", frameon=True)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.yaxis.set_major_formatter(
        mticker.FuncFormatter(lambda v, _: f"{v:,.0f}")
    )

    # note about missing NAIVE rows (skipped for large capacity)
    skipped = [
        name for name in workloads
        if df[(df["name"] == name) & (df["type"] == "NAIVE")].empty
    ]
    if skipped:
        note = "* NAIVE not run for: " + ", ".join(skipped)
        fig.text(0.5, -0.04, note, ha="center", fontsize=8, color="#888888",
                 style="italic")

    plt.tight_layout()
    plt.savefig(out_file, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"[saved] {out_file}")


# ── helper: line chart (capacity on x, metric on y) ─────────────────────────
def line_chart(metric: str, ylabel: str, title: str, out_file: str):
    """
    X-axis = capacity (numeric, log scale).
    One line per algorithm type.
    Points labelled with the workload name.
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    for t in types:
        sub = df[df["type"] == t].sort_values("capacity")
        if sub.empty:
            continue
        ax.plot(
            sub["capacity"], sub[metric],
            marker="o", linewidth=2, markersize=7,
            label=LABELS.get(t, t),
            color=COLORS.get(t, None),
        )
        # annotate each point with the short workload name
        for _, row in sub.iterrows():
            short = row["name"].split(",")[0]          # first segment only
            ax.annotate(
                short,
                xy=(row["capacity"], row[metric]),
                xytext=(4, 4), textcoords="offset points",
                fontsize=7, color="#444444",
            )

    ax.set_xscale("log")
    ax.set_title(title)
    ax.set_xlabel("Capacity  (log scale)")
    ax.set_ylabel(ylabel)
    ax.legend(loc="upper left", frameon=True)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.yaxis.set_major_formatter(
        mticker.FuncFormatter(lambda v, _: f"{v:,.0f}")
    )

    plt.tight_layout()
    plt.savefig(out_file, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"[saved] {out_file}")


# ── generate charts ─────────────────────────────────────────────────────────
print(f"\nLoaded {len(df)} rows from '{csv_path}'")
print(f"  Algorithm types : {types}")
print(f"  Workloads       : {list(workloads)}\n")

grouped_bar(
    metric   = "time_ms",
    ylabel   = "Wall-clock time  (ms)",
    title    = "LRU Cache Benchmark — Total Time (ms) per Workload",
    out_file = "benchmark_time_ms.png",
)

grouped_bar(
    metric   = "ns_per_op",
    ylabel   = "Latency per operation  (ns)",
    title    = "LRU Cache Benchmark — Latency per Operation (ns/op)",
    out_file = "benchmark_ns_per_op.png",
)

line_chart(
    metric   = "time_ms",
    ylabel   = "Wall-clock time  (ms)",
    title    = "Time (ms) vs Cache Capacity — O(1) vs Naïve",
    out_file = "benchmark_time_vs_capacity.png",
)

line_chart(
    metric   = "ns_per_op",
    ylabel   = "ns / operation",
    title    = "ns/op vs Cache Capacity — O(1) vs Naïve",
    out_file = "benchmark_nsop_vs_capacity.png",
)

print("\nDone.  Four PNG files written to the current directory.")
