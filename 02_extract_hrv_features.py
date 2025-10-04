# File: F:\HRV\02_extract_hrv_features.py
import numpy as np
import pandas as pd
from scipy import signal
import matplotlib.pyplot as plt
import os

print("Starting HRV Feature Extraction...")

# Load the generated data
hrv_data = pd.read_csv('F:/HRV/hrv_sample_data.csv')

# Convert string representation of lists to actual lists
print("Converting data formats...")
hrv_data['rr_intervals'] = hrv_data['rr_intervals'].apply(lambda x: eval(x) if isinstance(x, str) else x)

def calculate_hrv_features(rr_intervals):
    """Calculate comprehensive HRV features from RR intervals"""
    
    rr_intervals = np.array(rr_intervals)
    features = {}
    
    # Basic statistics
    features['mean_rr'] = np.mean(rr_intervals)
    features['std_rr'] = np.std(rr_intervals)
    features['min_rr'] = np.min(rr_intervals)
    features['max_rr'] = np.max(rr_intervals)
    
    # Time-domain features
    diff_rr = np.diff(rr_intervals)
    
    # SDNN - Standard deviation of NN intervals
    features['sdnn'] = np.std(rr_intervals)
    
    # RMSSD - Root mean square of successive differences
    features['rmssd'] = np.sqrt(np.mean(diff_rr ** 2))
    
    # NN50 - Number of pairs of successive NN intervals differing by more than 50 ms
    features['nn50'] = np.sum(np.abs(diff_rr) > 50)
    
    # pNN50 - Proportion of NN50 divided by total number of NN intervals
    features['pnn50'] = features['nn50'] / len(diff_rr) * 100
    
    # Frequency-domain features using Welch's method
    try:
        fs = 1.0  # Sampling frequency (1 Hz for RR intervals)
        f, Pxx = signal.welch(rr_intervals, fs=fs, nperseg=min(256, len(rr_intervals)))
        
        # Define frequency bands
        vlf_band = (0.003, 0.04)    # Very Low Frequency
        lf_band = (0.04, 0.15)      # Low Frequency
        hf_band = (0.15, 0.4)       # High Frequency
        
        # Calculate power in each band
        vlf_power = np.trapz(Pxx[(f >= vlf_band[0]) & (f < vlf_band[1])], f[(f >= vlf_band[0]) & (f < vlf_band[1])])
        lf_power = np.trapz(Pxx[(f >= lf_band[0]) & (f < lf_band[1])], f[(f >= lf_band[0]) & (f < lf_band[1])])
        hf_power = np.trapz(Pxx[(f >= hf_band[0]) & (f < hf_band[1])], f[(f >= hf_band[0]) & (f < hf_band[1])])
        total_power = vlf_power + lf_power + hf_power
        
        features['lf_power'] = lf_power
        features['hf_power'] = hf_power
        features['lf_hf_ratio'] = lf_power / hf_power if hf_power > 0 else 0
        features['total_power'] = total_power
        features['lf_nu'] = lf_power / (lf_power + hf_power) * 100 if (lf_power + hf_power) > 0 else 0
        features['hf_nu'] = hf_power / (lf_power + hf_power) * 100 if (lf_power + hf_power) > 0 else 0
        
    except Exception as e:
        # If frequency analysis fails, set default values
        features['lf_power'] = 0
        features['hf_power'] = 0
        features['lf_hf_ratio'] = 0
        features['total_power'] = 0
        features['lf_nu'] = 0
        features['hf_nu'] = 0
    
    # Non-linear features - Poincaré plot
    features['sd1'] = np.std(diff_rr) / np.sqrt(2)  # Short-term variability
    features['sd2'] = np.sqrt(2 * np.std(rr_intervals)**2 - 0.5 * np.std(diff_rr)**2)  # Long-term variability
    features['sd_ratio'] = features['sd2'] / features['sd1'] if features['sd1'] > 0 else 0
    
    return features

# Extract features for all samples
print("Extracting HRV features from RR intervals...")
feature_list = []
for idx, row in hrv_data.iterrows():
    if idx % 50 == 0:
        print(f"Processing sample {idx}/{len(hrv_data)}")
    
    features = calculate_hrv_features(row['rr_intervals'])
    features['label'] = row['label']
    feature_list.append(features)

hrv_features = pd.DataFrame(feature_list)
print(f"Extracted {len(hrv_features.columns)-1} HRV features")
print("Feature names:", list(hrv_features.columns))

# Save features to CSV
hrv_features.to_csv('F:/HRV/hrv_features.csv', index=False)
print("HRV features saved to: F:/HRV/hrv_features.csv")

# Create basic visualizations
print("Creating basic visualizations...")
plt.style.use('seaborn-v0_8-whitegrid')
fig, axes = plt.subplots(2, 2, figsize=(12, 10))

# 1. SDNN distribution by label
for label in [0, 1, 2]:
    subset = hrv_features[hrv_features['label'] == label]
    axes[0,0].hist(subset['sdnn'], alpha=0.7, label=f'Class {label}', bins=20)
axes[0,0].set_xlabel('SDNN (ms)')
axes[0,0].set_ylabel('Frequency')
axes[0,0].set_title('SDNN Distribution by Class')
axes[0,0].legend()

# 2. RMSSD distribution by label
for label in [0, 1, 2]:
    subset = hrv_features[hrv_features['label'] == label]
    axes[0,1].hist(subset['rmssd'], alpha=0.7, label=f'Class {label}', bins=20)
axes[0,1].set_xlabel('RMSSD (ms)')
axes[0,1].set_ylabel('Frequency')
axes[0,1].set_title('RMSSD Distribution by Class')
axes[0,1].legend()

# 3. LF/HF Ratio by label
for label in [0, 1, 2]:
    subset = hrv_features[hrv_features['label'] == label]
    axes[1,0].hist(subset['lf_hf_ratio'], alpha=0.7, label=f'Class {label}', bins=20)
axes[1,0].set_xlabel('LF/HF Ratio')
axes[1,0].set_ylabel('Frequency')
axes[1,0].set_title('LF/HF Ratio Distribution by Class')
axes[1,0].legend()

# 4. Feature correlation with labels
correlations = []
feature_names = [col for col in hrv_features.columns if col not in ['label', 'lf_nu', 'hf_nu']]
for feature in feature_names:
    if feature != 'label':
        corr = np.corrcoef(hrv_features[feature], hrv_features['label'])[0,1]
        correlations.append(abs(corr))

axes[1,1].barh(feature_names[:10], correlations[:10])  # Show top 10
axes[1,1].set_xlabel('Absolute Correlation with Label')
axes[1,1].set_title('Top 10 Feature Importance')

plt.tight_layout()
plt.savefig('F:/HRV/hrv_basic_analysis.png', dpi=300, bbox_inches='tight')
print("Basic analysis plot saved to: F:/HRV/hrv_basic_analysis.png")

# Print summary statistics
print("\n" + "="*50)
print("HRV FEATURE EXTRACTION SUMMARY")
print("="*50)
print(f"Total samples processed: {len(hrv_features)}")
print(f"Class distribution:")
print(f"  - Normal (0): {sum(hrv_features['label']==0)} samples")
print(f"  - Stress (1): {sum(hrv_features['label']==1)} samples")
print(f"  - Fatigue (2): {sum(hrv_features['label']==2)} samples")

print(f"\nKey HRV Metrics (Mean ± Std):")
print(f"  - SDNN: {hrv_features['sdnn'].mean():.2f} ± {hrv_features['sdnn'].std():.2f} ms")
print(f"  - RMSSD: {hrv_features['rmssd'].mean():.2f} ± {hrv_features['rmssd'].std():.2f} ms")
print(f"  - LF/HF Ratio: {hrv_features['lf_hf_ratio'].mean():.2f} ± {hrv_features['lf_hf_ratio'].std():.2f}")

plt.show()