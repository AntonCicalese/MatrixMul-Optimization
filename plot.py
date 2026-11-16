import pandas as pd
import matplotlib.pyplot as plt
import sys

dataFrames = []
for i in range(1, len(sys.argv)):
	fData = pd.read_csv(sys.argv[i])
	fData = fData[(fData["functionIdx"] == 3) & (fData["parallel"] == 0)]
	fData = fData.groupby(["blockSize"]).median()
	dataFrames.append(fData)

for df in dataFrames:
	print("---")
	print(df["time"])
	plt.plot(df["time"])

plt.xticks((8, 64, 128, 256, 384, 512, 640, 768, 896, 1024))
plt.show()

