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
allcols = [ (lambda row: float(row[24]), 'Total time (s)', "total") ]

allalg = [ ['TOUCH:TD(Case1)','rs-'],['SGrid:TD(Case1)','kx-'],['PBSM:TD(Case1)','g.--'],['dTOUCH:BU(Case4)','bo:'],['cTOUCH:BU(Case4)','yp-'],\
            ['reTOUCH:BU(Case4)','c8:'],['rereTOUCH:BU(Case4)','ks-'] ]


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
xcol = (1,'Epsilon')

for objnum in xrange(5000,45000,5000):

    epsilondata = alldata[np.array(alldata[:,3],dtype=float)==objnum,:]
    allobjnum = np.unique(np.array(alldata[:,1],dtype=float))

    additional = "Local Join: " + alldata[0,6] + "; ObjNum: " + \
                  alldata[0,3] + "; Grid Size: " + alldata[0,9] + \
                  "\nDatasets: " + getFileName(alldata[0,4]) + ", " + getFileName(alldata[0,5])

    for col in allcols:

        mae_out = col[2] + str(objnum) + '.png'
        plots = []
        for alginf in allalg:
            alg = alginf[0]
            linest = alginf[1]
            algdata = epsilondata[epsilondata[:,0]==alg]
            results = []
            for epsilon in allobjnum:
                objdata = algdata[np.array(algdata[:,1],dtype=float)==epsilon]
                results.append(np.average(np.array(map(col[0],objdata),dtype=float)))
                #print 'alg' , alg , ' , obj ' , objnum , ' , col ' , col[1] , ' data ' , np.array(map(col[0],objdata),dtype=float) , ' ave ' , np.average(np.array(map(col[0],objdata),dtype=float))
            plt.semilogy(allobjnum,results,linest,label=alg)
        plt.suptitle(col[1])
        plt.title(additional, fontsize = '8')
        plt.xlabel(xcol[1]);
        plt.ylabel(col[1]);
        plt.legend(loc='upper center', fontsize='8')
        plt.savefig(mae_out, dpi=300)
        plt.clf()
