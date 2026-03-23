import yfinance as yf
import time
from datetime import datetime
import pandas as pd

# Nasdaq & Tech Giants
tickers = ["^IXIC", "NDAQ", "NVDA", "AAPL", "MSFT", "GOOGL"]

def fetch_nasdaq_data():
    now = datetime.now().strftime('%H:%M:%S')
    print(f"\n--- Nasdaq Analytics Feed: {now} ---")
    print(f"{'Ticker':<8} | {'Price':>10} | {'% Change':>10} | {'5-Min SMA':>10} | {'Volume':>12}")
    print("-" * 65)
    
    # Fetch 1-minute data for the last hour to calculate SMA
    data = yf.download(tickers, period="1d", interval="1m", group_by='ticker', progress=False)
    
    for ticker in tickers:
        try:
            ticker_df = data[ticker].dropna()
            last_quote = ticker_df.iloc[-1]
            day_open = ticker_df['Open'].iloc[0]
            
            price = last_quote['Close']
            volume = last_quote['Volume']
            
            # Calculate 5-minute Simple Moving Average
            sma_5 = ticker_df['Close'].rolling(window=5).mean().iloc[-1]
            pct_change = ((price - day_open) / day_open) * 100
            
            # Highlight if price is above SMA (Bullish sign for the session)
            trend = "↑" if price > sma_5 else "↓"
            
            print(f"{ticker:<8} | ${price:>9.2f} | {pct_change:>9.2f}% | {sma_5:>9.2f} {trend} | {int(volume):>12,}")
            
        except Exception:
            pass 
    print("-" * 65)

if __name__ == "__main__":
    try:
        while True:
            fetch_nasdaq_data()
            time.sleep(60) 
    except KeyboardInterrupt:
        print("\nMonitoring stopped.")
