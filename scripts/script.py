import pandas as pd
import matplotlib.pyplot as plt

try:
    df = pd.read_csv("C:\\Users\\sophi\\Documents\\PlatformIO\\Projects\\unigou-remote\\runs\\run1.csv")
except FileNotFoundError:
    print("Error: data.csv not found.")
    exit()

df['timestamp_ms'] = df['timestamp_us'] / 1000.0

fig, ax = plt.subplots(figsize=(12, 6))
tasks = df['task_name'].unique()
colors = {'Blink': 'tab:blue', 'Sensor': 'tab:green'}

for i, task in enumerate(tasks):
    task_data = df[df['task_name'] == task].reset_index()

    for idx in range(len(task_data) - 1):
        if task_data.loc[idx, 'event_type'] == 'START' and \
           task_data.loc[idx+1, 'event_type'] == 'STOP':
            
            start_time = task_data.loc[idx, 'timestamp_ms']
            duration = task_data.loc[idx+1, 'timestamp_ms'] - start_time

            ax.broken_barh([(start_time, duration)], 
                           (i - 0.4, 0.8), 
                           facecolors=colors.get(task, 'tab:gray'),
                           edgecolor='black',
                           alpha=0.8)

ax.set_yticks(range(len(tasks)))
ax.set_yticklabels(tasks)
ax.set_xlabel('Time (ms)')
ax.set_title('Tracer FreeRTOS - Task Timeline (Arduino Uno R4)')
ax.grid(True, axis='x', linestyle='--', alpha=0.7)

ax.set_xlim(df['timestamp_ms'].min() - 10, df['timestamp_ms'].max() + 10)

plt.tight_layout()
plt.ion()
plt.show(block=True)