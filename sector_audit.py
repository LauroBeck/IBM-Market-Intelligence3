import pandas as pd
import os

# Load the fresh audit data
if not os.path.exists('stargate_audit.csv'):
    print("Error: stargate_audit.csv not found. Run ./quant_engine first.")
    exit(1)

df = pd.read_csv('stargate_audit.csv')

# Define Sectors
sectors = {
    'IBM': 'TECH (VALUE)',
    'NVDA': 'TECH (GROWTH)',
    'CVX': 'ENERGY (VALUE)',
    'JPM': 'FINANCE (VALUE)',
    '^IXIC': 'INDEX (TECH)'
}
df['Sector'] = df['Ticker'].map(sectors)

# Calculate Relative Strength
print("\n--- STARGATE V3: SECTOR ROTATION AUDIT | MAR 23 EOD ---")
print("| Ticker | Sector          | RSI  | ATR  | Status  |")
print("|--------|-----------------|------|------|---------|")

for index, row in df.iterrows():
    ticker = str(row['Ticker'])
    if ticker in sectors:
        rsi = float(row['RSI'])
        atr = float(row['ATR'])
        sector = sectors[ticker]
        
        # Strength Logic
        strength = "STRONG" if rsi > 55 else ("WEAK" if rsi < 45 else "NEUTRAL")
        
        print(f"| {ticker:6} | {sector:15} | {rsi:4.1f} | {atr:4.2f} | {strength:7} |")

print("-------------------------------------------------------")
