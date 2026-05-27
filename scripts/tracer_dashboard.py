import os
import pandas as pd
import plotly.graph_objects as go
import streamlit as st

st.set_page_config(page_title="FreeRTOS Timeline Analyser", layout="wide")
st.title("⏱️ Analisador de Estados - FreeRTOS")
st.markdown("Visualização otimizada das transições de estados e chaveamento de contexto.")

LOG_FILE_PATH = "C:\\Users\\sophi\\Documents\\GitHub\\unigou-remote\\runs\\data.csv"

@st.cache_data(ttl=1)
def load_and_process_tracer(filepath):
    if not os.path.exists(filepath):
        return pd.DataFrame()
        
    records = []
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            parts = line.strip().split(",")
            if len(parts) == 3:
                try:
                    timestamp = int(parts[0])
                    task_name = parts[1].strip()
                    state = parts[2].strip().upper()
                    
                    if task_name == "task_name":
                        continue
                        
                    records.append({
                        "Timestamp": timestamp,
                        "Task": task_name,
                        "Estado": state
                    })
                except ValueError:
                    continue
                    
    return pd.DataFrame(records)

df_tracer = load_and_process_tracer(LOG_FILE_PATH)

if df_tracer.empty:
    st.error(f"Nenhum dado válido encontrado em: {LOG_FILE_PATH}")
else:
    # -------------------------------------------------------------------------
    # Painel de Filtros na Barra Lateral (Sidebar)
    # -------------------------------------------------------------------------
    st.sidebar.header("🔍 Filtros de Visualização")
    
    # Filtro 1: Seleção de Tarefas
    todas_tasks = df_tracer["Task"].unique().tolist()
    tasks_selecionadas = st.sidebar.multiselect(
        "Selecione as tarefas para exibir:", 
        options=todas_tasks, 
        default=todas_tasks
    )
    
    # Filtro 2: Range de Tempo (Janela de observação)
    min_time = int(df_tracer["Timestamp"].min())
    max_time = int(df_tracer["Timestamp"].max())
    
    janela_tempo = st.sidebar.slider(
        "Janela de Tempo (Microssegundos / Ticks)",
        min_value=min_time,
        max_value=max_time,
        value=(min_time, min_time + 5000 if (min_time + 5000) < max_time else max_time)
    )
    
    # Aplica os filtros ao DataFrame
    df_filtrado = df_tracer[
        (df_tracer["Task"].isin(tasks_selecionadas)) & 
        (df_tracer["Timestamp"] >= janela_tempo[0]) & 
        (df_tracer["Timestamp"] <= janela_tempo[1])
    ].copy()

    # Mapeamento de Estados para Altura Numérica (Simula Analisador Lógico)
    state_numeric = {"SUSPENDED": 0, "BLOCKED": 1, "READY": 2, "RUNNING": 3}
    df_filtrado["Estado_Num"] = df_filtrado["Estado"].map(state_numeric).fillna(0)

    # -------------------------------------------------------------------------
    # Renderização Alternativa: Gráfico de Ondas de Estado (Step Plot)
    # -------------------------------------------------------------------------
    st.subheader("📈 Diagrama de Transição de Estados (Estilo Analisador Lógico)")
    
    color_map = {
        "RUNNING": "#2ca02c",    # Verde
        "READY": "#ff7f0e",      # Laranja
        "BLOCKED": "#d62728",    # Vermelho
        "SUSPENDED": "#7f7f7f"   # Cinza
    }

    fig = go.Figure()

    # Plota uma linha em formato de degrau (Step) para cada tarefa separadamente
    for idx, task in enumerate(tasks_selecionadas):
        df_task = df_filtrado[df_filtrado["Task"] == task].sort_values("Timestamp")
        
        if not df_task.empty:
            # Desloca a altura vertical de cada tarefa para que elas fiquem em canais paralelos
            offset = idx * 4  
            
            # -----------------------------------------------------------------
            # PROLONGAMENTO DOS ESTADOS (Início e Fim da Janela)
            # -----------------------------------------------------------------
            estado_inicial = df_task.iloc[0]["Estado_Num"] + offset
            estado_final = df_task.iloc[-1]["Estado_Num"] + offset
            
            # Injeta o tempo mínimo e máximo nas extremidades do eixo X
            x_line = [janela_tempo[0]] + df_task["Timestamp"].tolist() + [janela_tempo[1]]
            
            # Repete o primeiro e último estado nas extremidades do eixo Y
            y_line = [estado_inicial] + (df_task["Estado_Num"] + offset).tolist() + [estado_final]
            
            # Adiciona a linha principal contínua (shape='hv' cria os degraus perfeitos)
            fig.add_trace(go.Scatter(
                x=x_line,
                y=y_line,
                mode="lines",
                line=dict(shape="hv", width=3, color="#555555"),
                hoverinfo="skip",
                showlegend=False
            ))
            
            # Adiciona pontos coloridos APENAS nos momentos reais de transição (df_task original)
            for estado, cor in color_map.items():
                df_sub = df_task[df_task["Estado"] == estado]
                if not df_sub.empty:
                    fig.add_trace(go.Scatter(
                        x=df_sub["Timestamp"],
                        y=df_sub["Estado_Num"] + offset,
                        mode="markers",
                        marker=dict(size=10, color=cor, symbol="square"),
                        name=f"{task} ({estado})",
                        text=[f"Task: {task}<br>Estado: {estado}<br>Tempo: {t} us" for t in df_sub["Timestamp"]],
                        hoverinfo="text"
                    ))

    # Customização dos Eixos para ficar legível e limpo
    y_ticks_val = []
    y_ticks_text = []
    for idx, task in enumerate(tasks_selecionadas):
        offset = idx * 4
        for estado, num in state_numeric.items():
            y_ticks_val.append(num + offset)
            y_ticks_text.append(f"{task} [{estado}]")

    fig.update_layout(
        xaxis_title="Tempo (Microssegundos / Ticks)",
        yaxis=dict(
            tickmode="array",
            tickvals=y_ticks_val,
            ticktext=y_ticks_text,
            gridcolor="#E5E5E5"
        ),
        height=len(tasks_selecionadas) * 180 + 100,
        showlegend=False,
        plot_bgcolor="white"
    )
    
    st.plotly_chart(fig, use_container_width=True)

    # --- Tabela Auxiliar de Eventos Rápidos ---
    st.subheader("📋 Histórico Sequencial Filtrado")
    st.dataframe(
        df_filtrado[["Timestamp", "Task", "Estado"]].sort_values("Timestamp", ascending=False), 
        use_container_width=True
    )