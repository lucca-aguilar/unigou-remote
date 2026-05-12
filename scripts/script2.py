import pandas as pd
import matplotlib.pyplot as plt
import os

# 1. Configuração do Caminho
path = r"C:\Users\sophi\Documents\PlatformIO\Projects\unigou-remote\runs\run5.csv"

def plot_ram():
    if not os.path.exists(path):
        print(f"Erro: Arquivo {path} não encontrado.")
        return

    # 2. Carregar e Limpar Dados
    df = pd.read_csv(path)
    df.columns = df.columns.str.strip() # Remove espaços acidentais

    # Filtra apenas os registros de RAM
    df_ram = df[df['type'].str.strip() == 'RAM'].copy()

    if df_ram.empty:
        print("Nenhum dado de RAM encontrado no CSV. Verifique a coluna 'type'.")
        return

    # 3. Processamento
    df_ram['timestamp_ms'] = df_ram['timestamp_us'] / 1000.0
    df_ram['ram_kb'] = df_ram['state_or_value'].astype(float) / 1024.0

    # 4. Criação do Gráfico
    plt.figure(figsize=(12, 6))
    
    # Linha principal com marcadores nos pontos de amostragem
    plt.plot(df_ram['timestamp_ms'], df_ram['ram_kb'], 
             color='#e74c3c', linestyle='-', marker='o', 
             markersize=4, label='Heap Livre (KB)')

    # Preenchimento abaixo da linha para destacar o volume
    plt.fill_between(df_ram['timestamp_ms'], df_ram['ram_kb'], 
                     color='#e74c3c', alpha=0.1)

    # 5. Estilização
    plt.title('Monitoramento de Memória Heap - Arduino Uno R4', fontsize=14, pad=20)
    plt.xlabel('Tempo Decorrido (ms)', fontsize=12)
    plt.ylabel('Memória Livre (KB)', fontsize=12)
    
    plt.grid(True, which='both', linestyle='--', alpha=0.5)
    plt.legend()
    
    # Ajusta o eixo Y para começar um pouco abaixo do valor mínimo para dar contexto
    y_min = df_ram['ram_kb'].min() * 0.95
    y_max = df_ram['ram_kb'].max() * 1.05
    plt.ylim(y_min, y_max)

    plt.tight_layout()
    print("Gráfico de RAM gerado com sucesso!")
    plt.show()

if __name__ == "__main__":
    plot_ram()