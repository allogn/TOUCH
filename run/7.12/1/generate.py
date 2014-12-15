#!/usr/bin/python

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


# x : arbitrary
xcol = (2,'Number of objects used from each of dataset')

# y : arbitrary
allcols = [ (11, "Number of compared objects (%)"), \
            (20, "Time for assignment step (s)"), \
            (22, "Time for local join (s)"), \
            (21, "Time for probing (s)"), \
            (37, "Time for probing + SGH building (s)"), \
            (16, "Filtered objects of type A"), \
            (29, "Time for building SGH (s)"), \
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
    if (lj == "NL" and (col[0] == 37 or col[0] == 13 or col[0] == 29)):
        continue
    with open(filename, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
        objnum = []
        touchtime = []
        dtouchtime = []
        ctouchtime = []
        retouchtime = []
        reretouchtime = []
        for row in spamreader:
            if (row[0] == 'TOUCH'):
                objnum.insert(0,int(row[xcol[0]]))
                touchtime.insert(0,float(row[col[0]]))
            if (row[0] == 'cTOUCH'):
                ctouchtime.insert(0,float(row[col[0]]))
            if (row[0] == 'reTOUCH'):
                retouchtime.insert(0,float(row[col[0]]))
            if (row[0] == 'dTOUCH'):
                dtouchtime.insert(0,float(row[col[0]]))
            if (row[0] == 'rereTOUCH'):
                reretouchtime.insert(0,float(row[col[0]]))

    mae_out = 'pic' + str(col[0]) + '.png'
    t, = plt.plot(objnum,touchtime,label='TOUCH'); 
    dt, = plt.plot(objnum,dtouchtime,label='dTOUCH'); 
    ct, = plt.plot(objnum,ctouchtime,label='cTOUCH'); 
    rt, = plt.plot(objnum,retouchtime,label='reTOUCH'); 
    rrt, = plt.plot(objnum,reretouchtime,'x',label='rereTOUCH'); 

    plt.suptitle(col[1])
    plt.title(additional, fontsize = '8')
    plt.xlabel(xcol[1]);
    plt.ylabel(col[1]);
    plt.legend([t, dt, ct, rt, rrt], ['TOUCH', 'dTOUCH', 'cTOUCH', 'reTOUCH', 'rereTOUCH'], loc='upper center', fontsize='8')
    plt.savefig(mae_out, dpi=300)
    plt.clf()

#plot total filtered objects
with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    objnum = []
    touchtime = []
    dtouchtime = []
    ctouchtime = []
    retouchtime = []
    reretouchtime = []
    for row in spamreader: 
        if (row[0] == 'TOUCH'):
            objnum.insert(0,int(row[xcol[0]]))
            touchtime.insert(0,int(row[17])+int(row[16]))
        if (row[0] == 'cTOUCH'):
            ctouchtime.insert(0,int(row[17])+int(row[16]))
        if (row[0] == 'reTOUCH'):
            retouchtime.insert(0,int(row[17])+int(row[16]))
        if (row[0] == 'dTOUCH'):
            dtouchtime.insert(0,int(row[17])+int(row[16]))
        if (row[0] == 'rereTOUCH'):
            reretouchtime.insert(0,int(row[17])+int(row[16]))

mae_out = 'picTotalFiltered.png'
t, = plt.plot(objnum,touchtime,label='TOUCH'); 
dt, = plt.plot(objnum,dtouchtime,label='dTOUCH'); 
ct, = plt.plot(objnum,ctouchtime,label='cTOUCH'); 
rt, = plt.plot(objnum,retouchtime,label='reTOUCH'); 
rrt, = plt.plot(objnum,reretouchtime,'x',label='rereTOUCH'); 

plt.suptitle('Total filtered objects')
plt.title(additional, fontsize = '8')
plt.xlabel(xcol[1]);
plt.ylabel('Total filtered objects');
plt.legend([t, dt, ct, rt, rrt], ['TOUCH', 'dTOUCH', 'cTOUCH', 'reTOUCH', 'rereTOUCH'], loc='upper center', fontsize='8')
plt.savefig(mae_out, dpi=300)
plt.clf()

#plot complex filtered objects
with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    objnum = []
    touchtime = []
    dtouchtime = []
    ctouchtime = []
    retouchtime = []
    reretouchtime = []
    for row in spamreader: 
        if (row[0] == 'TOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            objnum.insert(0,int(row[xcol[0]]))
            touchtime.insert(0,r)
        if (row[0] == 'cTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            ctouchtime.insert(0,r)
        if (row[0] == 'reTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            retouchtime.insert(0,r)
        if (row[0] == 'dTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            dtouchtime.insert(0,r)
        if (row[0] == 'rereTOUCH'):
            fa = int(row[16])
            fb = int(row[17])
            a = int(row[2])
            b = int(row[3])
            r = (fa*(b-fb)+fb*(a-fa))*1./(a*b*1.)
            reretouchtime.insert(0,r)

mae_out = 'picComplexFiltered.png'
t, = plt.plot(objnum,touchtime,label='TOUCH'); 
dt, = plt.plot(objnum,dtouchtime,label='dTOUCH'); 
ct, = plt.plot(objnum,ctouchtime,label='cTOUCH'); 
rt, = plt.plot(objnum,retouchtime,label='reTOUCH'); 
rrt, = plt.plot(objnum,reretouchtime,'x',label='rereTOUCH'); 

plt.suptitle('Benefit for filtering')
plt.title(additional, fontsize = '8')
plt.xlabel(xcol[1]);
plt.ylabel('Fa|B|+Fb|A|/|A||B|');
plt.legend([t, dt, ct, rt, rrt], ['TOUCH', 'dTOUCH', 'cTOUCH', 'reTOUCH', 'rereTOUCH'], loc='upper center', fontsize='8')
plt.savefig(mae_out, dpi=300)
plt.clf()

#plot total filtered objects
with open(filename, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
    objnum = []
    touchtime = []
    dtouchtime = []
    ctouchtime = []
    retouchtime = []
    reretouchtime = []
    for row in spamreader: 
        if (row[0] == 'TOUCH'):
            objnum.insert(0,int(row[xcol[0]]))
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            touchtime.insert(0,s)
        if (row[0] == 'cTOUCH'):
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            ctouchtime.insert(0,s)
        if (row[0] == 'reTOUCH'):
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            retouchtime.insert(0,s)
        if (row[0] == 'dTOUCH'):
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            dtouchtime.insert(0,s)
        if (row[0] == 'rereTOUCH'):
            s = float(row[21])+float(row[20])+float(row[23])
            if row[6] == 'SGrid':
                s = s + float(row[29])+float(row[26])
            reretouchtime.insert(0,s)

mae_out = 'picTotalTime.png'
t, = plt.plot(objnum,touchtime,label='TOUCH'); 
dt, = plt.plot(objnum,dtouchtime,label='dTOUCH'); 
ct, = plt.plot(objnum,ctouchtime,label='cTOUCH'); 
rt, = plt.plot(objnum,retouchtime,label='reTOUCH'); 
rrt, = plt.plot(objnum,reretouchtime,'x',label='rereTOUCH'); 

plt.suptitle('Total time (s)')
plt.title(additional, fontsize = '8')
plt.xlabel(xcol[1]);
plt.ylabel('Total time (build+assign+join+sgh grid+deduplication) (s)');
plt.legend([t, dt, ct, rt, rrt], ['TOUCH', 'dTOUCH', 'cTOUCH', 'reTOUCH', 'rereTOUCH'], loc='upper center', fontsize='8')
plt.savefig(mae_out, dpi=300)
plt.clf()
