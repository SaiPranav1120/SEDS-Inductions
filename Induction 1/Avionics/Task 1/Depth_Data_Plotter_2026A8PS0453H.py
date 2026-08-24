# Code for Avionics Task-1, written by Sai Pranav Perugupalli, 2026A8PS0453H

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

df = pd.read_csv("Depth Data.csv")                                                                     # Reading the .csv file
df['Depth_Clean'] = pd.to_numeric(df['Depth (m)'], errors='coerce')                                    # Clearing the String errors
df.loc[(df['Depth_Clean'] < -500) | (df['Depth_Clean'] >= 0), 'Depth_Clean'] = np.nan                  # Replacing the exteme values with NaN
df['Depth_Clean'] = df['Depth_Clean'].interpolate()                                                    # Linear Interpolation to fill the NaN blocks with usable data
df['Depth_Filtered'] = df['Depth_Clean'].rolling(window=5, min_periods=1).median()                     #Using rolling median function to smoothen out the noise
#print(df[['Depth_Clean', 'Depth_Filtered']].describe())

fig, ax = plt.subplots()                                                                               # Shit down from here is to configure and tweak the animated graph
ax.set_title("Sea Floor Depth Profile")
ax.set_xlabel("Time (seconds)"); ax.set_ylabel("Depth (meters)")
ax.set_xlim(0, 300); ax.set_ylim(-450, -50)
line_filtered, = ax.plot([], [], color='red', linewidth=2, label='Filtered Depth')
ax.grid(True, linestyle=':', alpha=0.6)
ax.legend()

def update(frame):
    x = df['Point'].iloc[:frame]
    y = df['Depth_Filtered'].iloc[:frame]
    line_filtered.set_data(x, y)
    return line_filtered

ani = animation.FuncAnimation(fig, update, frames=300, interval=100)
plt.show()