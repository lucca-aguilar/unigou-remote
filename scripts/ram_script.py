import io
import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv('C:\\Users\\sophi\\Documents\\GitHub\\unigou-remote\\runs\\data.csv')

# 2. Filtrar apenas os registros de memória RAM
df_ram = df[df["type"] == "RAM"].copy()

# 3. Converter os dados para tipos numéricos para evitar erros no gráfico
# Dividimos o timestamp por 1.000.000 se quisermos o tempo em segundos (s)
df_ram["tempo_s"] = df_ram["timestamp_us"].astype(float) / 1_000_000
df_ram["ram_disponivel"] = df_ram["state_or_value"].astype(float)

# 4. Criar o gráfico
plt.figure(figsize=(10, 5))
plt.plot(
    df_ram["tempo_s"],
    df_ram["ram_disponivel"],
    marker="o",
    linestyle="-",
    color="#2b5c8f",
    linewidth=2,
)

# Configurações de estilo e títulos
plt.title("Evolução da Memória RAM Disponível ao Longo do Tempo", fontsize=14)
plt.xlabel("Tempo (Segundos)", fontsize=12)
plt.ylabel("RAM Disponível (Bytes / Unidades)", fontsize=12)
plt.grid(True, linestyle="--", alpha=0.6)

# Rotacionar os eixos se os números ficarem muito grandes
plt.xticks(rotation=45)
plt.tight_layout()

# 5. Exibir ou salvar o gráfico
# plt.savefig('grafico_ram.png', dpi=300) # Caso queira salvar como imagem
plt.show()