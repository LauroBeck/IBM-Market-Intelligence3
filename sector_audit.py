import pandas as pd
import os

if not os.path.exists('stargate_audit.csv'):
    print("Error: Run ./quant_engine first.")
    exit(1)

df = pd.read_csv('stargate_audit.csv')

sectors = {
    '^GSPC': 'INDEX (S&P 500)',
    '^DJI':  'INDEX (DOW 30)',
    '^RUT':  'INDEX (RUSSELL 2k)',
    '^IXIC': 'INDEX (NASDAQ)',
    'IBM':   'TECH (VALUE)',
    'NVDA':  'TECH (GROWTH)',
    'CVX':   'ENERGY (VALUE)',
    'JPM':   'FINANCE (VALUE)'
}
df['Sector'] = df['Ticker'].map(sectors)

print("\n--- STARGATE V3: MACRO DEPLOYMENT AUDIT | MAR 23 ---")
print("| Ticker | Sector          | RSI  | ATR    | Status  |")
print("|--------|-----------------|------|--------|---------|")

for _, row in df.iterrows():
    t = str(row['Ticker'])
    if t in sectors:
        rsi, atr = float(row['RSI']), float(row['ATR'])
        stat = "STRONG" if rsi > 55 else ("WEAK" if rsi < 45 else "NEUTRAL")
        print(f"| {t:6} | {sectors[t]:15} | {rsi:4.1f} | {atr:6.2f} | {stat:7} |")
print("-------------------------------------------------------")
