import os
import re
import pandas as pd
import plotly.graph_objects as go
import streamlit as st

st.set_page_config(page_title="FreeRTOS Timeline Analyser", layout="wide")
st.title("FreeRTOS State Analyser")
st.markdown("Optimized visualization of state transitions and context switching.")

LOG_FILE_PATH = "C:\\Users\\sophi\\Documents\\GitHub\\unigou-remote\\runs\\data.csv"

@st.cache_data(ttl=1)
def load_and_process_tracer(filepath):
    if not os.path.exists(filepath):
        return pd.DataFrame()
        
    records = []
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            parts = line.strip().split(",")
            
            if len(parts) == 6:
                try:
                    timestamp = int(parts[2].strip())  
                    task_name = parts[3].strip()       
                    state = parts[4].strip().upper()   
                    
                    if task_name == "task_name" or state not in ["RUNNING", "READY", "BLOCKED", "SUSPENDED", "DELETED"]:
                        continue
                        
                    records.append({
                        "Timestamp": timestamp,
                        "Task": task_name,
                        "State": state
                    })
                except ValueError:
                    continue
                    
    return pd.DataFrame(records)

df_tracer = load_and_process_tracer(LOG_FILE_PATH)

if df_tracer.empty:
    st.error(f"No valid data found in: {LOG_FILE_PATH}")
else:
    st.sidebar.header("Visualization Filters")
    
    all_tasks = df_tracer["Task"].unique().tolist()
    selected_tasks = st.sidebar.multiselect(
        "Select tasks to display:", 
        options=all_tasks, 
        default=all_tasks
    )
    
    min_time = int(df_tracer["Timestamp"].min())
    max_time = int(df_tracer["Timestamp"].max())
    
    time_window = st.sidebar.slider(
        "Time Window (Microseconds / Ticks)",
        min_value=min_time,
        max_value=max_time,
        value=(min_time, min_time + 5000 if (min_time + 5000) < max_time else max_time)
    )
    
    df_filtered = df_tracer[
        (df_tracer["Task"].isin(selected_tasks)) & 
        (df_tracer["Timestamp"] >= time_window[0]) & 
        (df_tracer["Timestamp"] <= time_window[1])
    ].copy()

    state_numeric = {"SUSPENDED": 0, "BLOCKED": 1, "READY": 2, "RUNNING": 3}
    df_filtered["State_Num"] = df_filtered["State"].map(state_numeric).fillna(0)

    st.subheader("State Transition Diagram (Logic Analyser Style)")
    
    color_map = {
        "RUNNING": "#2ca02c",    
        "READY": "#ff7f0e",      
        "BLOCKED": "#d62728",    
        "SUSPENDED": "#7f7f7f"   
    }

    fig = go.Figure()

    for idx, task in enumerate(selected_tasks):
        df_task = df_filtered[df_filtered["Task"] == task].sort_values("Timestamp")
        
        if not df_task.empty:
            offset = idx * 4  
            
            initial_state = df_task.iloc[0]["State_Num"] + offset
            final_state = df_task.iloc[-1]["State_Num"] + offset
            
            x_line = [time_window[0]] + df_task["Timestamp"].tolist() + [time_window[1]]
            y_line = [initial_state] + (df_task["State_Num"] + offset).tolist() + [final_state]
            
            fig.add_trace(go.Scatter(
                x=x_line,
                y=y_line,
                mode="lines",
                line=dict(shape="hv", width=3, color="#555555"),
                hoverinfo="skip",
                showlegend=False
            ))
            
            for state, color in color_map.items():
                df_sub = df_task[df_task["State"] == state]
                if not df_sub.empty:
                    fig.add_trace(go.Scatter(
                        x=df_sub["Timestamp"],
                        y=df_sub["State_Num"] + offset,
                        mode="markers",
                        marker=dict(size=10, color=color, symbol="square"),
                        name=f"{task} ({state})",
                        text=[f"Task: {task}<br>State: {state}<br>Time: {t} us" for t in df_sub["Timestamp"]],
                        hoverinfo="text"
                    ))

    y_ticks_val = []
    y_ticks_text = []
    for idx, task in enumerate(selected_tasks):
        offset = idx * 4
        for state, num in state_numeric.items():
            y_ticks_val.append(num + offset)
            y_ticks_text.append(f"{task} [{state}]")

    fig.update_layout(
        xaxis_title="Time (Microseconds / Ticks)",
        yaxis=dict(
            tickmode="array",
            tickvals=y_ticks_val,
            ticktext=y_ticks_text,
            gridcolor="#E5E5E5"
        ),
        height=len(selected_tasks) * 180 + 100,
        showlegend=False,
        plot_bgcolor="white"
    )
    
    st.plotly_chart(fig, use_container_width=True)

    st.subheader("Filtered Sequential History")
    st.dataframe(
        df_filtered[["Timestamp", "Task", "State"]].sort_values("Timestamp", ascending=False), 
        use_container_width=True
    )