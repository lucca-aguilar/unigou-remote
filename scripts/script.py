import pandas as pd
import matplotlib.pyplot as plt

# 1. Carregar os dados
try:
    df = pd.read_csv("C:\\Users\\sophi\\Documents\\PlatformIO\\Projects\\unigou-remote\\runs\\run1.csv")
except FileNotFoundError:
    print("Erro: Arquivo data.csv não encontrado.")
    exit()

# 2. Converter microsegundos para milissegundos para melhor visualização
df['timestamp_ms'] = df['timestamp_us'] / 1000.0

# 3. Preparar a plotagem
fig, ax = plt.subplots(figsize=(12, 6))
tasks = df['task_name'].unique()
colors = {'Blink': 'tab:blue', 'Sensor': 'tab:green'}

# 4. Processar os intervalos START/STOP
for i, task in enumerate(tasks):
    task_data = df[df['task_name'] == task].reset_index()
    
    # Encontrar pares de START e STOP
    for idx in range(len(task_data) - 1):
        if task_data.loc[idx, 'event_type'] == 'START' and \
           task_data.loc[idx+1, 'event_type'] == 'STOP':
            
            start_time = task_data.loc[idx, 'timestamp_ms']
            duration = task_data.loc[idx+1, 'timestamp_ms'] - start_time
            
            # Desenha a barra no gráfico de Gantt
            ax.broken_barh([(start_time, duration)], 
                           (i - 0.4, 0.8), 
                           facecolors=colors.get(task, 'tab:gray'),
                           edgecolor='black',
                           alpha=0.8)

# 5. Estilização do Gráfico
ax.set_yticks(range(len(tasks)))
ax.set_yticklabels(tasks)
ax.set_xlabel('Tempo (ms)')
ax.set_title('Tracer FreeRTOS - Timeline de Tarefas (Arduino Uno R4)')
ax.grid(True, axis='x', linestyle='--', alpha=0.7)

# Ajustar limites para começar do primeiro evento
ax.set_xlim(df['timestamp_ms'].min() - 10, df['timestamp_ms'].max() + 10)

plt.tight_layout()
plt.ion()
plt.show(block=True)