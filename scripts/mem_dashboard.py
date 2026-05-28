import os
import re
import pandas as pd
import plotly.express as px
import streamlit as st

# Configuração da página
st.set_page_config(page_title="Monitor de Memória FreeRTOS", layout="wide")
st.title("💾 Monitor de Memória Dedicado (Heap & Stack)")

# Caminho do arquivo de log/csv gerado pelo Arduino
LOG_FILE_PATH = "C:\\Users\\sophi\\Documents\\GitHub\\unigou-remote\\runs\\data.csv"

@st.cache_data(ttl=1)  # Atualiza o cache rapidamente se o arquivo mudar
def parse_memory_data(filepath):
    if not os.path.exists(filepath):
        return pd.DataFrame(), pd.DataFrame()
    
    heap_records = []
    stack_records = []
    
    # Expressões regulares para extrair os números de dentro das strings do logger
    heap_regex = re.compile(r"Free:\s*(\d+)\s*B\s*\|\s*Min ever:\s*(\d+)\s*B")
    stack_regex = re.compile(r"Task\s+([a-zA-Z0-9_\-]+)\s*\|\s*Stack HWM:\s*(\d+)\s*words")
    
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            parts = line.strip().split(",")
            # Verifica se a linha tem o formato esperado e veio da "Memory task"
            if len(parts) >= 3 and parts[1].strip() == "Memory task":
                timestamp = int(parts[0])
                content = parts[2].strip()
                
                # 1. Tenta casar com o padrão de Heap
                heap_match = heap_regex.search(content)
                if heap_match:
                    free_heap = int(heap_match.group(1))
                    min_heap = int(heap_match.group(2))
                    heap_records.append({
                        "timestamp_ticks": timestamp,
                        "Free Heap (Bytes)": free_heap,
                        "Mínimo Heap Histórico (Bytes)": min_heap
                    })
                    continue
                
                # 2. Tenta casar com o padrão de Stack High Water Mark
                stack_match = stack_regex.search(content)
                if stack_match:
                    task_name = stack_match.group(1)
                    hwm_words = int(stack_match.group(2))
                    stack_records.append({
                        "timestamp_ticks": timestamp,
                        "Task": task_name,
                        "Stack HWM (Words)": hwm_words
                    })
                    
    df_heap = pd.DataFrame(heap_records)
    df_stack = pd.DataFrame(stack_records)
    
    return df_heap, df_stack

# Carrega e processa o arquivo
df_heap, df_stack = parse_memory_data(LOG_FILE_PATH)

if df_heap.empty and df_stack.empty:
    st.error(f"Nenhum dado da 'Memory task' encontrado no arquivo: {LOG_FILE_PATH}")
else:
    # Criação de duas colunas no layout do Streamlit
    col1, col2 = st.columns(2)
    
    with col1:
        st.header("📊 Otimização do Heap (RAM Global)")
        if not df_heap.empty:
            # Transforma o dataframe para o formato longo ideal para o Plotly plotar duas linhas
            df_heap_melted = df_heap.melt(
                id_vars=["timestamp_ticks"], 
                value_vars=["Free Heap (Bytes)", "Mínimo Heap Histórico (Bytes)"],
                var_name="Métrica", value_name="Bytes"
            )
            
            fig_heap = px.line(
                df_heap_melted,
                x="timestamp_ticks",
                y="Bytes",
                color="Métrica",
                title="Evolução do Heap Alocado dinamicamente",
                labels={"timestamp_ticks": "Tempo (Ticks)"},
                markers=True
            )
            st.plotly_chart(fig_heap, use_container_width=True)
            
            # Mostra os valores atuais em formato de métrica visual
            ultimo_heap = df_heap.iloc[-1]
            st.metric("Free Heap Atual", f"{ultimo_heap['Free Heap (Bytes)']} B")
            st.metric("Pior Caso Histórico (Min Ever)", f"{ultimo_heap['Mínimo Heap Histórico (Bytes)']} B")
        else:
            st.info("Sem dados de Heap para exibir.")

    with col2:
        st.header("🛡️ Margem de Stack por Task (High Water Mark)")
        st.markdown(
            "> **Nota:** O FreeRTOS reporta o *High Water Mark* como o **mínimo de espaço que sobrou** na pilha desde que a Task iniciou (em Words). "
            "Se este valor chegar perto de **0**, a Task sofrerá um Stack Overflow!"
        )
        
        if not df_stack.empty:
            # Gráfico de linhas mostrando a evolução do HWM de cada tarefa individualmente
            fig_stack = px.line(
                df_stack,
                x="timestamp_ticks",
                y="Stack HWM (Words)",
                color="Task",
                title="Margem Livre da Pilha (Quanto maior, mais seguro)",
                labels={"timestamp_ticks": "Tempo (Ticks)"},
                markers=True
            )
            st.plotly_chart(fig_stack, use_container_width=True)
            
        