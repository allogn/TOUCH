#!/usr/bin/python

#data table MUST BE SORTED BY column 3 (num of objects that variates)

import sys
import csv
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

if (len(sys.argv)<2):
    filename = 'SJ.csv'
else:
    filename = sys.argv[1]

# y : arbitrary
allcols = [ (lambda row: float(row[11]),"Number of compared objects (%)", "objnum"), \
            (lambda row: float(row[20]), "Time for assignment step (s)", "assignment"), \
            (lambda row: float(row[22]), "Time for local join (s)", "localJoin"), \
            (lambda row: float(row[12]), "ComparedMax objects", "NLjoin"), \
            (lambda row: float(row[37]), "Time for probing + SGH building (s)", "probingSGH"), \
            (lambda row: float(row[16]), "Filtered objects of type A", "filteredA"), \
            (lambda row: float(row[28]), "Time for building SGH (s)", "building"), \
            (lambda row: float(row[17]), "Filtered objects of type B", "filteredB"), \
            (lambda row: float(row[13]), "Number of duplicates", "duplicates"), \
            (lambda row: float(row[20])+float(row[21])+float(row[23])+float(row[29])+float(row[26]), 'Total time (build+assign+join+sgh grid+deduplication) (s)', "total"), \
            (lambda row: (float(row[16])*(float(row[3])-float(row[17]))+float(row[17])*(float(row[2])-float(row[16])))*1./(float(row[2])*float(row[3])*1.), "Fa|B|+Fb|A|/|A||B|", "filteredComp"), \
            (lambda row: float(row[16])+float(row[17]), "Total filtered", "totalfiltered") ]

allalg = [ 'TOUCH','dTOUCH','cTOUCH','reTOUCH','rereTOUCH' ]

def getFileName(name):
    t = name.find('/', 0, len(name))
    tt = t
    while t != -1:
        tt = t
        t = name.find('/', t+1, len(name))
    if (tt != -1):
        name = name[tt+1:]
    return name

totalColNum = 100

alldata = []

with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    i = 0
    for row in spamreader:
        if i == 0:
            #skip
            i = i+1
        else:
            newrow = []
            newrow.append(row[0])
            newrow.append(float(row[1]))
            newrow.append(float(row[2]))
            newrow.append(float(row[3]))
            newrow.append(getFileName(row[4]))
            newrow.append(getFileName(row[5]))
            newrow.append(getFileName(row[6]))
            for i in xrange(7,totalColNum):
                newrow.append(float(row[i]))
            alldata.append(newrow)
        
alldata = np.array(alldata)


# x : arbitrary
xcol = (3,'Number of objects used from datasets')
epsilon = 16

epsilondata = alldata[np.array(alldata[:,1],dtype=float)==epsilon,:]

allobjnum = np.unique(np.array(alldata[:,3],dtype=float))

additional = "E = " + str(epsilon) + "; Local Join: " + alldata[0,6] + "; Fanout: " + \
              alldata[0,7] + "; Leaf size: " + alldata[0,8] + "; Grid Size: " + alldata[0,9] + \
              "\nDatasets: " + getFileName(alldata[0,4]) + ", " + getFileName(alldata[0,5])

for col in allcols:

    mae_out = col[2] + '.png'
    plots = []
    for alg in allalg:
        algdata = epsilondata[epsilondata[:,0]==alg]
        results = []
        for objnum in allobjnum:
            objdata = algdata[np.array(algdata[:,3],dtype=float)==objnum]
            results.append(np.average(np.array(map(col[0],objdata),dtype=float)))
            #print 'alg' , alg , ' , obj ' , objnum , ' , col ' , col[1] , ' data ' , np.array(map(col[0],objdata),dtype=float) , ' ave ' , np.average(np.array(map(col[0],objdata),dtype=float))
        plt.plot(allobjnum,results,label=alg)
    plt.suptitle(col[1])
    plt.title(additional, fontsize = '8')
    plt.xlabel(xcol[1]);
    plt.ylabel(col[1]);
    plt.legend(loc='upper center', fontsize='8')
    plt.savefig(mae_out, dpi=300)
    plt.clf()
