import matplotlib.pyplot as plt
import numpy as np

l = np.array([])

with open("places.log","r") as f:
	for row in f:
		x,y,z = row.strip(')').split(',')
		l = np.append(l,[float(x[1:]),float(y),float(z[:-2])])
l = l.reshape(l.shape[0]/3,3)

plt.scatter(l[:,0],l[:,1],s=1)
plt.axis([-1000,1000,-1000,1000])
plt.savefig("pic.png",dpi=300);
plt.clf();
