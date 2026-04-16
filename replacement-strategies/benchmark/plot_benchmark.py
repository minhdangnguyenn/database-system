import math
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

plt.style.use("seaborn-v0_8-whitegrid")

STRATEGY_ORDER = ["NAIVE", "LRU", "FIFO"]
COLORS = {
    "NAIVE": "#DD8452",
    "LRU": "#4C72B0",
    "FIFO": "#55A868",
}


REQUIRED_COLS = {
    "name",
    "type",
    "capacity",
    "operations",
    "key_range",
    "time_ms",
    "ns_per_op",
}



def find_default_csv() -> str:
    candidates = [
        "benchmark_results.csv",
        "../build/benchmark_results.csv",
        "./build/benchmark_results.csv",
    ]
    for path in candidates:
        if os.path.exists(path):
            return path
    return candidates[0]



def load_dataframe() -> pd.DataFrame:
    csv_path = sys.argv[1] if len(sys.argv) > 1 else find_default_csv()

    if not os.path.exists(csv_path):
        print(f"[ERROR] File not found: {csv_path}")
        print("Run the C++ benchmark first to generate benchmark_results.csv")
        sys.exit(1)

    df = pd.read_csv(csv_path)
    missing = REQUIRED_COLS - set(df.columns)
    if missing:
        print(f"[ERROR] Missing columns in CSV: {sorted(missing)}")
        sys.exit(1)

    if "run" not in df.columns:
        df["run"] = 0

    df["type"] = df["type"].astype(str).str.strip().str.upper()

    allowed = set(STRATEGY_ORDER)
    before = len(df)
    df = df[df["type"].isin(allowed)].copy()
    dropped = before - len(df)
    if dropped > 0:
        print(f"[INFO] Dropped {dropped} rows with unsupported strategy type")

    if df.empty:
        print("[ERROR] No rows for NAIVE/LRU/FIFO were found in the CSV")
        sys.exit(1)

    print(f"Loaded {len(df)} rows from {csv_path}")
    return df



def aggregate(df: pd.DataFrame) -> pd.DataFrame:
    agg = (
        df.groupby(["name", "type", "capacity"], as_index=False)
        .agg(
            mean_time_ms=("time_ms", "mean"),
            std_time_ms=("time_ms", "std"),
            mean_ns_per_op=("ns_per_op", "mean"),
            std_ns_per_op=("ns_per_op", "std"),
            runs=("run", "nunique"),
        )
        .sort_values(["name", "capacity", "type"])
    )

    for col in ["std_time_ms", "std_ns_per_op"]:
        agg[col] = agg[col].fillna(0.0)

    return agg



def ordered_workloads(agg: pd.DataFrame):
    return agg["name"].drop_duplicates().tolist()



def strategies_present(agg: pd.DataFrame):
    present = set(agg["type"].unique())
    return [s for s in STRATEGY_ORDER if s in present]



def plot_metric_vs_capacity(
    agg: pd.DataFrame,
    metric_mean: str,
    metric_std: str,
    y_label: str,
    title: str,
    out_file: str,
) -> None:
    workloads = ordered_workloads(agg)
    strategies = strategies_present(agg)

    cols = 2 if len(workloads) > 1 else 1
    rows = math.ceil(len(workloads) / cols)

    fig, axes = plt.subplots(rows, cols, figsize=(7 * cols, 4 * rows))
    axes = np.array(axes).reshape(-1)

    for idx, workload in enumerate(workloads):
        ax = axes[idx]
        sub = agg[agg["name"] == workload]

        for strategy in strategies:
            d = sub[sub["type"] == strategy].sort_values("capacity")
            if d.empty:
                continue

            ax.errorbar(
                d["capacity"],
                d[metric_mean],
                yerr=d[metric_std],
                marker="o",
                linewidth=2,
                capsize=3,
                label=strategy,
                color=COLORS[strategy],
            )

        ax.set_xscale("log")
        ax.set_title(workload)
        ax.set_xlabel("Capacity (log)")
        ax.set_ylabel(y_label)

    for idx in range(len(workloads), len(axes)):
        axes[idx].axis("off")

    handles, labels = axes[0].get_legend_handles_labels()
    if handles:
        fig.legend(handles, labels, loc="upper center", ncol=len(labels), frameon=True)

    fig.suptitle(title, y=0.995, fontsize=13, fontweight="bold")
    fig.tight_layout(rect=[0, 0, 1, 0.95])
    fig.savefig(out_file, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"[saved] {out_file}")



def max_common_capacity(agg: pd.DataFrame):
    strategies = strategies_present(agg)
    if not strategies:
        return None

    cap_sets = []
    for strategy in strategies:
        caps = set(agg[agg["type"] == strategy]["capacity"].unique())
        cap_sets.append(caps)

    common = sorted(set.intersection(*cap_sets)) if cap_sets else []
    if not common:
        return None
    return common[-1]



def plot_grouped_at_capacity(
    agg: pd.DataFrame,
    metric_mean: str,
    metric_std: str,
    y_label: str,
    title_prefix: str,
    out_file: str,
) -> None:
    selected_capacity = max_common_capacity(agg)
    if selected_capacity is None:
        print(f"[WARN] Skip {out_file}: no common capacity across strategies")
        return

    sub = agg[agg["capacity"] == selected_capacity]
    workloads = ordered_workloads(sub)
    strategies = strategies_present(sub)

    x = np.arange(len(workloads))
    width = 0.24

    fig, ax = plt.subplots(figsize=(max(10, len(workloads) * 2.3), 6))

    for i, strategy in enumerate(strategies):
        vals = []
        errs = []
        for workload in workloads:
            row = sub[(sub["name"] == workload) & (sub["type"] == strategy)]
            if row.empty:
                vals.append(0.0)
                errs.append(0.0)
            else:
                vals.append(float(row[metric_mean].iloc[0]))
                errs.append(float(row[metric_std].iloc[0]))

        offset = (i - (len(strategies) - 1) / 2.0) * width
        ax.bar(
            x + offset,
            vals,
            width,
            yerr=errs,
            capsize=3,
            label=strategy,
            color=COLORS[strategy],
            edgecolor="white",
            linewidth=0.6,
            alpha=0.9,
        )

    ax.set_title(f"{title_prefix} (capacity={selected_capacity:,})")
    ax.set_ylabel(y_label)
    ax.set_xticks(x)
    ax.set_xticklabels(workloads)
    ax.legend(loc="upper left", frameon=True)

    fig.tight_layout()
    fig.savefig(out_file, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"[saved] {out_file}")



def main() -> None:
    df = load_dataframe()
    agg = aggregate(df)

    plot_metric_vs_capacity(
        agg=agg,
        metric_mean="mean_time_ms",
        metric_std="std_time_ms",
        y_label="Time (ms)",
        title="Benchmark Time vs Capacity",
        out_file="benchmark_time_vs_capacity.png",
    )

    plot_metric_vs_capacity(
        agg=agg,
        metric_mean="mean_ns_per_op",
        metric_std="std_ns_per_op",
        y_label="ns per op",
        title="Benchmark Latency vs Capacity",
        out_file="benchmark_nsop_vs_capacity.png",
    )

    plot_grouped_at_capacity(
        agg=agg,
        metric_mean="mean_time_ms",
        metric_std="std_time_ms",
        y_label="Time (ms)",
        title_prefix="Mean Time by Workload",
        out_file="benchmark_time_ms.png",
    )

    plot_grouped_at_capacity(
        agg=agg,
        metric_mean="mean_ns_per_op",
        metric_std="std_ns_per_op",
        y_label="ns per op",
        title_prefix="Mean Latency by Workload",
        out_file="benchmark_ns_per_op.png",
    )

    print("Done.")


if __name__ == "__main__":
    main()
