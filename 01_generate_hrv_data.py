# File: F:\HRV\01_generate_hrv_data.py
import numpy as np
import pandas as pd
import os

# Create HRV directory if it doesn't exist
os.makedirs('F:/HRV', exist_ok=True)

# Simulate RR-interval data (time between heartbeats in milliseconds)
def generate_sample_hrv_data(n_samples=300):
    """Generate synthetic HRV data for demonstration"""
    np.random.seed(42)
    
    data = []
    for i in range(n_samples):
        # Base RR intervals (600-1000 ms is normal)
        base_rr = np.random.normal(800, 50)
        
        # Add physiological variability
        rr_intervals = []
        for j in range(300):  # 5 minutes of data at 1Hz
            # Respiratory sinus arrhythmia (high frequency component)
            hf = 20 * np.sin(2 * np.pi * 0.25 * j)  # 0.25 Hz respiratory component
            
            # Mayer waves (low frequency component)
            lf = 15 * np.sin(2 * np.pi * 0.1 * j)   # 0.1 Hz Mayer waves
            
            # Noise
            noise = np.random.normal(0, 5)
            
            rr = base_rr + hf + lf + noise
            rr_intervals.append(rr)
        
        # Label: 0=Normal, 1=High Stress, 2=Fatigue
        if i < n_samples//3:
            label = 0  # Normal
        elif i < 2*n_samples//3:
            label = 1  # High Stress (reduced variability)
            rr_intervals = [x * 0.8 + np.random.normal(0, 2) for x in rr_intervals]  # Reduce variability
        else:
            label = 2  # Fatigue (increased LF component)
            rr_intervals = [x + 30 * np.sin(2 * np.pi * 0.05 * j) for j, x in enumerate(rr_intervals)]
        
        data.append({'rr_intervals': rr_intervals, 'label': label})
    
    return pd.DataFrame(data)

# Generate sample data
print("Generating HRV sample data...")
hrv_data = generate_sample_hrv_data(300)
print(f"Generated {len(hrv_data)} samples")
print(f"Data structure: {hrv_data.columns}")
print(f"First sample label: {hrv_data.iloc[0]['label']}")
print(f"First sample RR intervals length: {len(hrv_data.iloc[0]['rr_intervals'])}")

# Save this data to CSV for later use
hrv_data.to_csv('F:/HRV/hrv_sample_data.csv', index=False)
print("Sample data saved to: F:/HRV/hrv_sample_data.csv")

# Also save a sample of the data for inspection
sample_df = hrv_data.head(10).copy()
sample_df.to_csv('F:/HRV/hrv_sample_inspection.csv', index=False)
print("Sample inspection data saved to: F:/HRV/hrv_sample_inspection.csv")