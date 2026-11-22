import pandas as pd
import matplotlib.pyplot as plt
import sys

dataFrames = []
for i in range(1, len(sys.argv)):
	fData = pd.read_csv(sys.argv[i])
	fData = fData[(fData["functionIdx"] == 3) & (fData["parallel"] == 0)]
	fData = fData.groupby(["blockSize"]).median()
	dataFrames.append(fData)
    
plt.style.use("dark_background")
for df in dataFrames:
	print("---")
	print(df["time"])
	plt.plot(df["time"])

plt.style.use('dark_background')
plt.xticks((8,16,32,48,64,96,128,192,256,320,384,448,512,576,640,768,896,1024))
plt.show()

