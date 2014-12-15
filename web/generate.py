import csv
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

with open('SJ.csv', 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    objnum = []
    touchtime = []
    ctouchtime = []
    retouchtime = []
    epsilons = []
    resultsnum = []
    sampleeps = -1
    objnummax = -1
    for row in spamreader:
        if (sampleeps == -1 and row[0] != 'Algorithm'):
            sampleeps = row[1]
            objnummax = row[2]
        if (row[0] == 'TOUCH' and (row[1] == sampleeps)):
            objnum.insert(0,int(row[2]))
            touchtime.insert(0,float(row[22]))
        if (row[0] == 'cTOUCH' and (row[1] == sampleeps)):
            ctouchtime.insert(0,float(row[22]))
        if (row[0] == 'reTOUCH' and (row[1] == sampleeps)):
            retouchtime.insert(0,float(row[22]))
        if (row[0] == 'TOUCH' and (row[2] == objnummax)):
            epsilons.insert(0,int(row[1]))
            resultsnum.insert(0,int(row[14]))

frc_out= 'timings.png'
mae_out = 'resultnum.png'

t, = plt.plot(objnum,touchtime,label='TOUCH'); 
ct, = plt.plot(objnum,ctouchtime,label='cTOUCH'); 
rt, = plt.plot(objnum,retouchtime,label='reTOUCH'); 
plt.xlabel('Spatial object number, x');
plt.ylabel('Time [s], y');
plt.legend([t, ct, rt], ['TOUCH', 'cTOUCH', 'reTOUCH'])
plt.savefig(frc_out)
plt.clf()
w, = plt.plot(epsilons,resultsnum); 
plt.xlabel('Epsilon');
plt.ylabel('Number of intersections');
plt.savefig(mae_out)
