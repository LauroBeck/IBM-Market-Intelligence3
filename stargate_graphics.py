import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# 1. Load Data
FILE = 'stargate_audit.csv'
if not os.path.exists(FILE):
    print(f"ERROR: {FILE} not found. Run ./quant_engine first.")
    exit()

df = pd.read_csv(FILE)
df['Timestamp'] = pd.to_datetime(df['Timestamp'])

# 2. Setup Plot (Dark Architect Theme)
plt.style.use('dark_background')
fig, ax1 = plt.subplots(figsize=(12, 7))

# 3. Plotting Percent Change to Normalize Scales
for ticker in df['Ticker'].unique():
    data = df[df['Ticker'] == ticker].sort_values('Timestamp')
    # Calculate % change from the start of the audit log
    first_price = data['Price'].iloc[0]
    data['Pct_Change'] = (data['Price'] / first_price - 1) * 100
    
    # Highlight IBM as the Sovereign Leader
    linewidth = 3 if ticker == 'IBM' else 1.5
    alpha = 1.0 if ticker == 'IBM' else 0.6
    
    ax1.plot(data['Timestamp'], data['Pct_Change'], label=f"{ticker}", linewidth=linewidth, alpha=alpha)

# 4. Styling & Annotations
ax1.set_title("STARGATE CLUSTER | MAR 23 EOD ROTATION ANALYSIS", fontsize=14, color='#00FFCC', pad=20)
ax1.set_ylabel("Price Performance (%)", fontsize=12)
ax1.axhline(0, color='white', linewidth=0.5, linestyle='--')

# 5. Legend & Branding
ax1.legend(loc='upper left', frameon=False, fontsize=10)
plt.grid(axis='y', alpha=0.1)
plt.tight_layout()

# Save Asset
plt.savefig('stargate_eod_report.png')
print("ARCHITECT LOG: Visual asset 'stargate_eod_report.png' generated.")
