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
plt.title("Time vs Size (rnd)")
plt.grid()
plt.xlabel("Elements")
plt.ylabel("Time (ns)")
plt.show()


for name, frame in frames.items():
    if "Almost" not in name: 
        plt.plot(frame["X"], frame["Y"], label=name)
plt.legend()
plt.title("Time vs Size (rnd)")
plt.grid()
plt.yscale('log')
plt.xlabel("Elements")
plt.ylabel("Time (ns) (Log)")
plt.show()


from scipy.interpolate import interp1d
from scipy.optimize import curve_fit
import numpy as np

def nlogn(x, a, b, c):
    x = np.where(x < 1, 1, x)
    return x*c*np.log(a*x)+b
def nn(x, a, b, c):
    return a*x**2 + b*x + c
def is_logarithmic(x):
    LOG = ["Quick", "Std"]
    return any(log in x for log in LOG)

params_nlogn = dict()
params_nn = dict()

for name, frame in frames.items():
    if "Almost" not in name:
        iterp = interp1d(frame["X"], frame["Y"])
        XX = np.linspace(frame["X"].min(), frame["X"].max(), 100)
        interpolated = iterp(XX)
        if is_logarithmic(name):
            params_nlogn[name], _ = curve_fit(nlogn, XX, interpolated ,maxfev = 50_000)
        else:
            params_nn[name], _ = curve_fit(nn, XX, interpolated)

print(params_nn)
print(params_nlogn)

XX = np.linspace(25000, 500_000, 5000)
for name, frame in frames.items():
    if "Almost" not in name:
        c=np.random.rand(3,)
        plt.plot(frame["X"], frame["Y"], c=c)
        if is_logarithmic(name):
            plt.plot(XX, nlogn(XX, *params_nlogn[name]), label=name, c=c, linestyle="--")
        else:
            plt.plot(XX, nn(XX, *params_nn[name]), label=name, c=c, linestyle="--")
plt.legend()
plt.yscale('log')
plt.title("Time vs Size (rnd), Predictions")
plt.grid()
plt.autoscale(True, tight=True)
plt.xlabel("Elements")
plt.ylabel("Time (ns) (Log)")
plt.show()


