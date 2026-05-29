import os
import re
import pandas as pd
import plotly.express as px
import streamlit as st

st.set_page_config(page_title="FreeRTOS Memory Monitor", layout="wide")
st.title("Dedicated Memory Monitor (Heap & Stack)")

LOG_FILE_PATH = "C:\\Users\\sophi\\Documents\\GitHub\\unigou-remote\\runs\\data.csv"

@st.cache_data(ttl=1)
def parse_memory_data(filepath):
    if not os.path.exists(filepath):
        return pd.DataFrame(), pd.DataFrame()
    
    heap_records = []
    stack_records = []
    
    heap_regex = re.compile(r"Free:\s*(\d+)\s*B\s*\|\s*Min ever:\s*(\d+)\s*B")
    stack_regex = re.compile(r"Task\s+([a-zA-Z0-9_\-]+)\s*\|\s*Stack HWM:\s*(\d+)\s*words")
    
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            parts = line.strip().split(",")
            if len(parts) >= 3 and parts[1].strip() == "Memory task":
                timestamp = int(parts[0])
                content = parts[2].strip()
                
                heap_match = heap_regex.search(content)
                if heap_match:
                    free_heap = int(heap_match.group(1))
                    min_heap = int(heap_match.group(2))
                    heap_records.append({
                        "timestamp_ticks": timestamp,
                        "Free Heap (Bytes)": free_heap,
                        "Historical Minimum Heap (Bytes)": min_heap
                    })
                    continue
                
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

df_heap, df_stack = parse_memory_data(LOG_FILE_PATH)

if df_heap.empty and df_stack.empty:
    st.error(f"No 'Memory task' data found in the file: {LOG_FILE_PATH}")
else:
    col1, col2 = st.columns(2)
    
    with col1:
        st.header("Heap Optimization (Global RAM)")
        if not df_heap.empty:
            df_heap_melted = df_heap.melt(
                id_vars=["timestamp_ticks"], 
                value_vars=["Free Heap (Bytes)", "Historical Minimum Heap (Bytes)"],
                var_name="Metric", value_name="Bytes"
            )
            
            fig_heap = px.line(
                df_heap_melted,
                x="timestamp_ticks",
                y="Bytes",
                color="Metric",
                title="Dynamically Allocated Heap Evolution",
                labels={"timestamp_ticks": "Time (Ticks)"},
                markers=True
            )
            st.plotly_chart(fig_heap, use_container_width=True)
            
            ultimo_heap = df_heap.iloc[-1]
            st.metric("Current Free Heap", f"{ultimo_heap['Free Heap (Bytes)']} B")
            st.metric("Historical Worst Case (Min Ever)", f"{ultimo_heap['Historical Minimum Heap (Bytes)']} B")
        else:
            st.info("No Heap data to display.")

    with col2:
        st.header("Stack Margin per Task (High Water Mark)")
        st.markdown(
            "> **Note:** FreeRTOS reports the *High Water Mark* as the **minimum remaining space** in the stack since the Task started (in Words). "
            "If this value gets close to **0**, the Task will trigger a Stack Overflow!"
        )
        
        if not df_stack.empty:
            fig_stack = px.line(
                df_stack,
                x="timestamp_ticks",
                y="Stack HWM (Words)",
                color="Task",
                title="Free Stack Margin (The higher, the safer)",
                labels={"timestamp_ticks": "Time (Ticks)"},
                markers=True
            )
            st.plotly_chart(fig_stack, use_container_width=True)