import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import os

# Caminho do CSV
CSV_PATH = r"C:\Users\sophi\Documents\GitHub\unigou-remote\runs\run5.csv"

# Cores dos estados
STATE_COLORS = {
    'RUNNING': '#2ecc71',
    'BLOCKED': '#f1c40f',
    'READY': '#95a5a6',
    'SUSPENDED': '#9b59b6'
}


def generate_gantt():
    if not os.path.exists(CSV_PATH):
        print("Arquivo não encontrado.")
        return

    # -----------------------------
    # Leitura e limpeza
    # -----------------------------
    df = pd.read_csv(CSV_PATH)

    df.columns = df.columns.str.strip()

    for col in ['task_name', 'state_or_value', 'type']:
        df[col] = df[col].astype(str).str.strip()

    df['timestamp_us'] = pd.to_numeric(df['timestamp_us'])

    # -----------------------------
    # Ignora telemetria RAM
    # -----------------------------
    df = df[df['type'] == 'STATE'].copy()

    # Ordena cronologicamente
    df = df.sort_values('timestamp_us')

    # Converter para ms
    df['timestamp_ms'] = df['timestamp_us'] / 1000.0

    tasks = df['task_name'].unique()

    fig, ax = plt.subplots(figsize=(18, 8))

    # -----------------------------
    # Monta intervalos
    # -----------------------------
    for i, task in enumerate(tasks):

        task_data = (
            df[df['task_name'] == task]
            .sort_values('timestamp_ms')
            .reset_index(drop=True)
        )

        for idx in range(len(task_data) - 1):

            start = task_data.loc[idx, 'timestamp_ms']
            end = task_data.loc[idx + 1, 'timestamp_ms']
            duration = end - start

            state = task_data.loc[idx, 'state_or_value']

            if duration <= 0:
                continue

            ax.broken_barh(
                [(start, duration)],
                (i - 0.35, 0.7),
                facecolors=STATE_COLORS.get(state, '#34495e'),
                edgecolor='black',
                linewidth=0.8,
                alpha=0.9
            )

    # -----------------------------
    # Estilo
    # -----------------------------
    ax.set_yticks(range(len(tasks)))
    ax.set_yticklabels(tasks, fontsize=11)

    ax.set_xlabel("Tempo (ms)")
    ax.set_ylabel("Tasks")
    ax.set_title("FreeRTOS Task Timeline - Arduino Uno R4")

    ax.grid(True, axis='x', linestyle='--', alpha=0.5)

    legend = [
        Patch(facecolor=color, edgecolor='black', label=state)
        for state, color in STATE_COLORS.items()
    ]

    ax.legend(handles=legend, title="Estados")

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    generate_gantt()