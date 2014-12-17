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
    filename2 = 'SJ_RE.csv'
else:
    filename = sys.argv[1]


def collapseX(data):
    res = np.array([],dtype=int)
    for r in data:
        if  res.shape[0] == 0 or res[-1] != r:
            res = np.append(res,[r])
    return res

def collapseY(xdata,data):
    res = np.array([])
    firstind = 0
    secondind = 0
    for r in xrange(1,len(xdata)):
        if (xdata[r] != xdata[r-1]) or r == len(xdata)-1:
            sumn = 0
            secondind = r
            for i in xrange(firstind,secondind+1):
                sumn = sumn + data[i]
            sumn = sumn*1./(secondind - firstind)
            firstind = r+1
            res = np.append(res,[sumn])
    return res

# x : arbitrary
xcol = (3,'Number of objects used from second dataset (first = 2000)')

# y : arbitrary
allcols = [ (11, "Number of compared objects (%)"), \
            (20, "Time for assignment step (s)"), \
            (22, "Time for local join (s)"), \
            (21, "Time for probing (s)"), \
            (12, "ComparedMax objects"), \
            (36, "Time for probing + SGH building (s)"), \
            (16, "Filtered objects of type A"), \
            (28, "Time for building SGH (s)"), \
            (17, "Filtered objects of type B"), \
            (13, "Number of duplicates") ]

with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    i = 0
    for row in spamreader:
        if i < 2:
            i = i + 1
        if i == 2:
            epsilon = row[1]
            d1 = row[4]
            # get pnly file name
            t = d1.find('/', 0, len(d1))
            tt = t
            while t != -1:
                tt = t
                t = d1.find('/', t+1, len(d1))
            if (tt != -1):
                d1 = d1[tt+1:]

            d2 = row[5]
            t = d2.find('/', 0, len(d2))
            tt = t
            while t != -1:
                tt = t
                t = d2.find('/', t+1, len(d2))
            if (tt != -1):
                d2 = d2[tt+1:]

            lj = row[6]
            fan = row[7]
            part = row[8]
            gs = row[9]
            i = i + 1
        if i > 2:
            break
additional = "E = " + epsilon + "; Local Join: " + lj + "; Fanout: " + fan + "; Leaf size: " + part + "; Grid Size: " + gs + "\nDatasets: " + d1 + ", " + d2


for col in allcols:
    with open(filename, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
        objnum = []
        retouchtime = []
        reretouchtime = []
        retouchtime2 = []
        reretouchtime2 = []
        for row in spamreader:
            if (row[0] == 'reTOUCH'):
                objnum.insert(0,int(row[xcol[0]]))
                retouchtime.insert(0,float(row[col[0]]))
            if (row[0] == 'rereTOUCH'):
                reretouchtime.insert(0,float(row[col[0]]))
    with open(filename2, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
        for row in spamreader:
            if (row[0] == 'reTOUCH'):
                retouchtime2.insert(0,float(row[col[0]]))
            if (row[0] == 'rereTOUCH'):
                reretouchtime2.insert(0,float(row[col[0]]))

    mae_out = 'pic' + str(col[0]) + '.png'
    rt, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime),label='reTOUCH'); 
    rrt, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime),'x',label='rereTOUCH'); 
    rt2, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime2),label='reTOUCH2'); 
    rrt2, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime2),'x',label='rereTOUCH2'); 

    plt.suptitle(col[1])
    plt.title(additional, fontsize = '8')
    plt.xlabel(xcol[1]);
    plt.ylabel(col[1]);
    plt.legend([rt, rrt, rt2, rrt2], ['AB', 'ABA', 'BA', 'BAB'], loc='upper center', fontsize='8')
    plt.savefig(mae_out, dpi=300)
    plt.clf()

#plot total filtered objects
with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    objnum = []
    retouchtime = []
    reretouchtime = []
    retouchtime2 = []
    reretouchtime2 = []
    for row in spamreader: 
        if (row[0] == 'reTOUCH'):
            objnum.insert(0,int(row[xcol[0]]))
            retouchtime.insert(0,int(row[17])+int(row[16]))
        if (row[0] == 'rereTOUCH'):
            reretouchtime.insert(0,int(row[17])+int(row[16]))
with open(filename2, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    for row in spamreader: 
        if (row[0] == 'reTOUCH'):
            retouchtime2.insert(0,int(row[17])+int(row[16]))
        if (row[0] == 'rereTOUCH'):
            reretouchtime2.insert(0,int(row[17])+int(row[16]))

mae_out = 'picTotalFiltered.png'
rt, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime),label='reTOUCH'); 
rrt, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime),'x',label='rereTOUCH'); 
rt2, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime2),label='reTOUCH2'); 
rrt2, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime2),'x',label='rereTOUCH2'); 

plt.suptitle('Total filtered objects')
plt.title(additional, fontsize = '8')
plt.xlabel(xcol[1]);
plt.ylabel('Total filtered objects');
plt.legend([rt, rrt, rt2, rrt2], ['AB', 'ABA', 'BA', 'BAB'], loc='upper center', fontsize='8')
plt.savefig(mae_out, dpi=300)
plt.clf()

#plot complex filtered objects
with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    objnum = []
    retouchtime = []
    reretouchtime = []
    retouchtime2 = []
    reretouchtime2 = []
    for row in spamreader: 
        if (row[0] == 'reTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            objnum.insert(0,int(row[xcol[0]]))
            retouchtime.insert(0,r)
        if (row[0] == 'rereTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            reretouchtime.insert(0,r)

with open(filename2, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    for row in spamreader: 
        if (row[0] == 'reTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            retouchtime2.insert(0,r)
        if (row[0] == 'rereTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            reretouchtime2.insert(0,r)

mae_out = 'picComplexFiltered.png'
rt, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime),label='reTOUCH'); 
rrt, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime),'x',label='rereTOUCH'); 
rt2, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime2),label='reTOUCH2'); 
rrt2, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime2),'x',label='rereTOUCH2'); 

plt.suptitle('Benefit for filtering')
plt.title(additional, fontsize = '8')
plt.xlabel(xcol[1]);
plt.ylabel('Fa|B|+Fb|A|/|A||B|');
plt.legend([rt, rrt, rt2, rrt2], ['AB', 'ABA', 'BA', 'BAB'], loc='upper center', fontsize='8')
plt.savefig(mae_out, dpi=300)
plt.clf()

#plot total filtered objects
with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    objnum = []
    retouchtime = []
    reretouchtime = []
    retouchtime2 = []
    reretouchtime2 = []
    for row in spamreader: 
        if (row[0] == 'reTOUCH'):
            objnum.insert(0,int(row[xcol[0]]))
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            retouchtime.insert(0,s)
        if (row[0] == 'rereTOUCH'):
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            reretouchtime.insert(0,s)

with open(filename2, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    for row in spamreader: 
        if (row[0] == 'reTOUCH'):
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            retouchtime2.insert(0,s)
        if (row[0] == 'rereTOUCH'):
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            reretouchtime2.insert(0,s)

mae_out = 'picTotalTime.png'
rt, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime),label='reTOUCH'); 
rrt, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime),'x',label='rereTOUCH'); 
rt2, = plt.plot(collapseX(objnum),collapseY(objnum,retouchtime2),label='reTOUCH2'); 
rrt2, = plt.plot(collapseX(objnum),collapseY(objnum,reretouchtime2),'x',label='rereTOUCH2'); 

plt.suptitle('Total time (s)')
plt.title(additional, fontsize = '8')
plt.xlabel(xcol[1]);
plt.ylabel('Total time (build+assign+join+sgh grid+deduplication) (s)');
plt.legend([rt, rrt, rt2, rrt2], ['AB', 'ABA', 'BA', 'BAB'], loc='upper center', fontsize='8')
plt.savefig(mae_out, dpi=300)
plt.clf()
