import math
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

plt.style.use("seaborn-v0_8-white")


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
    out_prefix: str,  # <-- renamed from out_file to out_prefix
) -> None:
    workloads = ordered_workloads(agg)
    strategies = strategies_present(agg)

    for workload in workloads:
        fig, ax = plt.subplots(figsize=(8, 5))

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
        ax.set_title(f"{title} — {workload}", fontsize=12, fontweight="bold")
        ax.set_xlabel("Capacity (log scale)")
        ax.set_ylabel(y_label)
        ax.legend(loc="upper left", frameon=True)

        fig.tight_layout()

        # build a safe filename from the workload name
        safe_name = workload.lower().replace(" ", "_").replace("-", "_")
        out_file = f"{out_prefix}_{safe_name}.png"

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
    x_label: str,
    title_prefix: str,
    out_prefix: str,
) -> None:
    selected_capacity = max_common_capacity(agg)
    if selected_capacity is None:
        print(f"[WARN] Skip {out_prefix}: no common capacity across strategies")
        return

    sub = agg[agg["capacity"] == selected_capacity]
    workloads = ordered_workloads(sub)
    strategies = strategies_present(sub)

    for workload in workloads:
        wsub = sub[sub["name"] == workload]
        n_strat = len(strategies)
        height = 0.22
        y = np.arange(n_strat)

        fig, ax = plt.subplots(figsize=(8, 2.8))

        vals = []
        errs = []
        for i, strategy in enumerate(strategies):
            row = wsub[wsub["type"] == strategy]
            val = float(row[metric_mean].iloc[0]) if not row.empty else 0.0
            err = float(row[metric_std].iloc[0]) if not row.empty else 0.0
            vals.append(val)
            errs.append(err)

            ax.barh(
                y[i],
                val,
                height,
                xerr=err,
                capsize=3,
                label=strategy,
                color=COLORS[strategy],
                edgecolor="white",
                linewidth=0.6,
                alpha=0.9,
            )

        # ── set xlim once with headroom ───────────────────────────
        xmax = max(v + e for v, e in zip(vals, errs)) if vals else 1.0
        ax.set_xlim(0, xmax * 1.22)
        xlim_right = ax.get_xlim()[1]

        # ── pass 2: annotate with inside/outside flip ─────────────
        offset = xmax * 0.015
        for i, (strategy, val, err) in enumerate(zip(strategies, vals, errs)):
            if val == 0.0:
                continue

            # flip inside when the bar itself fills >70% of the axis
            bar_ratio = val / xlim_right
            would_clip = bar_ratio > 0.70

            if would_clip:
                ax.text(
                    val * 0.97,
                    y[i],
                    f"{val:.1f}",
                    va="center",
                    ha="right",
                    fontsize=8,
                    fontweight="bold",
                    color="white",
                )
            else:
                ax.text(
                    val + err + offset,
                    y[i],
                    f"{val:.1f}",
                    va="center",
                    ha="left",
                    fontsize=8,
                    fontweight="bold",
                    color=COLORS[strategy],
                )

        ax.tick_params(axis="y", pad=12)
        ax.set_xlabel(x_label)
        ax.set_yticks(y)
        ax.set_yticklabels(strategies, fontsize=9)
        ax.legend(loc="lower right", frameon=True)

        fig.tight_layout()

        safe_name = workload.lower().replace(" ", "_").replace("-", "_")
        out_file = f"{out_prefix}_{safe_name}.png"
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
        out_prefix="benchmark_time_vs_capacity",
    )

    plot_metric_vs_capacity(
        agg=agg,
        metric_mean="mean_ns_per_op",
        metric_std="std_ns_per_op",
        y_label="ns per op",
        title="Benchmark Latency vs Capacity",
        out_prefix="benchmark_nsop_vs_capacity",
    )

    plot_grouped_at_capacity(  # <-- x_label and out_prefix
        agg=agg,
        metric_mean="mean_time_ms",
        metric_std="std_time_ms",
        x_label="Time (ms)",
        title_prefix="Mean Time by Workload",
        out_prefix="benchmark_time_ms",
    )

    plot_grouped_at_capacity(
        agg=agg,
        metric_mean="mean_ns_per_op",
        metric_std="std_ns_per_op",
        x_label="ns per op",
        title_prefix="Mean Latency by Workload",
        out_prefix="benchmark_ns_per_op",
    )

    print("Done.")


if __name__ == "__main__":
    main()
