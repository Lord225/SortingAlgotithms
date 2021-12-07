import pandas as pd
import glob
import os
import matplotlib.pyplot as plt

path = "Benchmarks\\"
csv_files = glob.glob(os.path.join(path, "*.csv"))

frames = dict()

for file in csv_files:
    name, _ = os.path.splitext(os.path.basename(file))

    frames[name] = pd.read_csv(file).sort_values("X")

for name, frame in frames.items():
    if "Almost" not in name: 
        plt.plot(frame["X"], frame["Y"], label=name)
plt.legend()
plt.grid()
plt.show()