import pandas as pd
import matplotlib.pyplot as plt
import os

# 1. Path Configuration
# Updated to match the repository structure: lucca-aguilar/unigou-remote/.../runs/run5.csv
path = r"C:\Users\sophi\Documents\GitHub\unigou-remote\runs\run5.csv"

def plot_ram():
    if not os.path.exists(path):
        print(f"Error: File {path} not found.")
        return

    # 2. Load and Clean Data
    df = pd.read_csv(path)
    df.columns = df.columns.str.strip() # Remove accidental spaces

    # Filter only RAM records
    df_ram = df[df['type'].str.strip() == 'RAM'].copy()

    if df_ram.empty:
        print("No RAM data found in CSV. Check the 'type' column.")
        return

    # 3. Processing
    df_ram['timestamp_ms'] = df_ram['timestamp_us'] / 1000.0
    df_ram['ram_kb'] = df_ram['state_or_value'].astype(float) / 1024.0

    # 4. Chart Creation
    plt.figure(figsize=(12, 6))
    
    # Main line with markers at sampling points
    plt.plot(df_ram['timestamp_ms'], df_ram['ram_kb'], 
             color='#e74c3c', linestyle='-', marker='o', 
             markersize=4, label='Free Heap (KB)')

    # Fill below the line to highlight volume
    plt.fill_between(df_ram['timestamp_ms'], df_ram['ram_kb'], 
                     color='#e74c3c', alpha=0.1)

    # 5. Styling
    plt.title('Heap Memory Monitoring - Arduino Uno R4', fontsize=14, pad=20)
    plt.xlabel('Elapsed Time (ms)', fontsize=12)
    plt.ylabel('Free Memory (KB)', fontsize=12)
    
    plt.grid(True, which='both', linestyle='--', alpha=0.5)
    plt.legend()
    
    # Adjust Y-axis to start slightly below the minimum value for context
    y_min = df_ram['ram_kb'].min() * 0.95
    y_max = df_ram['ram_kb'].max() * 1.05
    plt.ylim(y_min, y_max)

    plt.tight_layout()
    print("RAM chart generated successfully!")
    plt.show()

if __name__ == "__main__":
    plot_ram()