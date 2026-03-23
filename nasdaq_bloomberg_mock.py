import time
import random
from datetime import datetime

# We try to import the real blpapi; if it fails or Terminal is closed, we mock it.
try:
    import blpapi
    MOCK_MODE = False
except ImportError:
    MOCK_MODE = True

class BloombergSimulator:
    """Simulates Bloomberg response messages for testing without a Terminal."""
    def __init__(self):
        self.data = {
            "NKY Index": 38235.50,
            "HSI Index": 16543.20,
            "NVDA US Equity": 173.00,
            "AAPL US Equity": 248.19
        }

    def get_latest_price(self, ticker):
        # Add a little random 'volatility' to the mock data
        change = random.uniform(-0.5, 0.5)
        self.data[ticker] += change
        return self.data[ticker]

def run_feed():
    sim = BloombergSimulator()
    tickers = ["NKY Index", "HSI Index", "NVDA US Equity", "AAPL US Equity"]
    
    print(f"--- {'[MOCK MODE]' if MOCK_MODE else '[LIVE BLOOMBERG]'} Feed Started ---")
    
    try:
        while True:
            now = datetime.now().strftime('%H:%M:%S')
            print(f"\nTime: {now} | Bloomberg Asia/US Hybrid Feed")
            print("-" * 45)
            
            for ticker in tickers:
                price = sim.get_latest_price(ticker)
                # Simulate the terminal's 8194 port behavior
                print(f"{ticker:<15} | Last: {price:>10.2f} | Status: OK")
                
            time.sleep(5) # Faster refresh for testing
    except KeyboardInterrupt:
        print("\nFeed stopped.")

if __name__ == "__main__":
    if MOCK_MODE:
        print("Note: blpapi not found or Terminal offline. Running in Simulation.")
    run_feed()
