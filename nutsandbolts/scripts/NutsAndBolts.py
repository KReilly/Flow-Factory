    #!/usr/bin/env python3
from OSC import OSCClient, OSCMessage, OSCServer
from collections import deque
import sys
import time
import math
import random

#Debug Helpers
displayTimeStamp = time.clock()
lastReceivedTimeStamp = [time.clock(), time.clock()]
displayEvery = 1
displayErrorInterval = 2
catastrophyErrorInterval = 20;
lastJitterTime = time.clock() 
sendJitterInterval = 1



def displayStats():
    #print "Max=" + str(maxValue) 
    print "Rolling Avg " + str(rollingAvg)


#File Saving Info

saveToFile = True
#raw data stash
#fileNameRaw = "./dataStash-raw.dat"
#foutRaw = open(fileNameRaw, 'w') 
#error file
#fileNameError = "./error-file.csv"
#foutError = open(fileNameError, 'w')


#previous values
prevSkipit = [0,0]

###################
# OSC Output Code #
###################

#OSC Clients for Audio and Video
clientVideo = OSCClient()
clientVideo.connect( ("localhost", 7110) )
clientVideo2 = OSCClient()
clientVideo2.connect( ("localhost", 7112) )
clientAudio = OSCClient()
clientAudio.connect( ("localhost", 7111) )

def sendJitter(skipitID, tag, loadVideo, jitter):
    jitter += loadVideo
    if(jitter > .99):
        jitter = .99
    if(jitter < 0):
        jitter = 0
    tag1 = "/" + str(skipitID) + "/"+ tag
    msg = OSCMessage()
    msg.setAddress(tag1)
    msg.append(jitter)
    clientVideo.send(msg)


#Relays data to osc
def oscSend(skipitID, tag, loadVideo, scaledMerge, loadAudio): #destination == 1 => video; destination == 2 => audio
        #looks like /2/magnitude
    tag1 = "/" + str(skipitID) + "/"+ tag
    msg = OSCMessage()
    msg.setAddress(tag1)
    print "load video " + str(loadVideo)
    msg.append(loadVideo)
    clientVideo.send(msg)

    tag1Merge = "/merge" 
    msgMerge = OSCMessage()
    msgMerge.setAddress(tag1Merge)
    msgMerge.append(scaledMerge)
    clientVideo.send(msgMerge)

    tag2 = "/" + str(skipitID) + "/"+ tag
    msg2 = OSCMessage()
    msg2.setAddress(tag2)
    msg2.append(loadAudio)
    clientAudio.send(msg2)

    tag2Merge = "/merge" 
    msg2Merge = OSCMessage()
    msg2Merge.setAddress(tag2Merge)
    msg2Merge.append(scaledMerge)
    clientAudio.send(msg2Merge)



    tag3 = "/" + str(skipitID) + "/"+ tag
    msg3 = OSCMessage()
    msg3.setAddress(tag3)
    #print "load video" + str(loadVideo)
    msg3.append(loadVideo)
    clientVideo2.send(msg3)


maxValue = [0,0]
minValue = [100,100]
#prevValue = 0
#currentDiff = 0

rollingAvgSize = 10
rollingAvgQue = [deque([0 for x in range(rollingAvgSize)]), deque([0 for x in range(rollingAvgSize)])]
rollingAvg = [0,0];

######################
# Data Analysis Code #
######################

def inspectMaths(streamIDTemp, dataValue): #TODO: Send both averaged values and individual values
    #global rollingSum, rollingAvg, rollingAvgQue
    global rollingAvg, rollingAvgQue
    
    global maxValue, minValue
    global displayTimeStamp, lastReceivedTimeStamp, displayErrorInterval

    if( dataValue > maxValue):
        print "New Max=" + str(maxValue)
        maxValue = dataValue
    if( dataValue < minValue):
        minValue = dataValue

    rollingAvgQue[streamIDTemp].popleft()
    rollingAvgQue[streamIDTemp].append(dataValue);
    rollingAvg[streamIDTemp] = 0
    for x in list(rollingAvgQue[streamIDTemp]):
        rollingAvg[streamIDTemp] = rollingAvg[streamIDTemp] + x
    rollingAvg[streamIDTemp] = rollingAvg[streamIDTemp] / rollingAvgSize

    if(time.clock() - displayTimeStamp > displayEvery):
        displayTimeStamp = time.clock()
        #displayStats()

    #use this to display errors when last received interval exceeds our displaytime interval
    currentTime = time.clock()
    if(currentTime - lastReceivedTimeStamp[streamIDTemp] > displayErrorInterval):
        print "Error: Skipit = "+ str(streamIDTemp) + "; Interval T = " + str(currentTime) + "; Interval Down T = " + str(currentTime-lastReceivedTimeStamp[streamIDTemp])
        #foutError.write(str(streamIDTemp)+str(currentTime)+","+str(currentTime-lastReceivedTimeStamp[streamIDTemp])+"\n")
    lastReceivedTimeStamp[streamIDTemp] = currentTime


##################
# OSC Input Code #
##################
server = OSCServer( ("localhost", 7210) )
server.timeout = 0
run = True

def handle_timeout(self):
    self.timed_out = True

import types
server.handle_timeout = types.MethodType(handle_timeout, server)

def scale(value, OldMax, OldMin, NewMax, NewMin):
    oldValue = value;
    if(oldValue > OldMax):
        oldValue = OldMax
    if(oldValue < OldMin):
        oldValue = OldMin

    OldRange = (OldMax - OldMin)  
    NewRange = (NewMax - NewMin) 
    newValue = (((oldValue - OldMin) * NewRange) / OldRange) + NewMin

    return newValue;

def user_callback(path, tags, args, source):
    paths = path.split("/")
    if(len(paths) > 1):
        streamIDTemp = int(paths[1])
        dataIDTemp = paths[2]
        dataValTemp = args[0]
        #println "id = " + str(streamIDTemp) + "; data =  "  + str(dataValTemp)
        
        inspectMaths(streamIDTemp, dataValTemp)

        #Math to map skipit data range [9,60] -> [0,1]
        OldMax = 50#40
        OldMin = 15#9.6
        NewMax = .99
        NewMin = 0
        AvgWindowMax = .7
        AvgWindowMin = .3

        rollingAvgCompute = rollingAvg[streamIDTemp]

        print rollingAvgCompute
        ####
        ####
        #BJ Play Area!!!
        #rollingAvgCompute = 22.1

        ####
        ####

        if(rollingAvg[streamIDTemp] > OldMax):
            rollingAvgCompute = OldMax
        if(rollingAvg[streamIDTemp] < OldMin):
            rollingAvgCompute = OldMin

        scaledCurrentValue = scale(rollingAvgCompute, OldMax, OldMin, NewMax, NewMin)

        print scaledCurrentValue


        if(scaledCurrentValue == 1):
            scaledCurrentValue = .99

        prevSkipit[int(streamIDTemp)] = scaledCurrentValue

        if(prevSkipit[0] >= AvgWindowMin and prevSkipit[0] <= AvgWindowMax and prevSkipit[1] >= AvgWindowMin and prevSkipit[1] <= AvgWindowMax):
            scaledMerge = 1
        else:
            scaledMerge = (prevSkipit[0] + prevSkipit[1]) / 2
        
        #print "scaled rolling avg = " + str(scaledCurrentValue)
        #print "scaled merge = " + str(scaledMerge)

        #send video scaled [0,1] and audio scaled [~9,~60]
        oscSend(streamIDTemp, dataIDTemp, scaledCurrentValue, scaledMerge, rollingAvgCompute)

        #record the raw data stream
        #if (saveToFile): 
        #    foutRaw.write(str(time.time()) + "," + str(streamIDTemp) + "," + str(dataValTemp) + "\n")
    else:
        print "Error in OSC paths length"

#two possible streams of input data
server.addMsgHandler( "/0/magnitude", user_callback )
server.addMsgHandler( "/1/magnitude", user_callback )
server.addMsgHandler( "/2/magnitude", user_callback )
server.addMsgHandler( "/3/magnitude", user_callback )

def quit_callback(path, tags, args, source):
    global run
    run = False
    server.close()

def readBuffer():
    server.timed_out = False
    while not server.timed_out:
        server.handle_request()


while run:
    readBuffer()
    #check for catastrophies here!
    currentTime = time.clock()
    for n in range(0,1):
        if(currentTime - lastReceivedTimeStamp[n] > catastrophyErrorInterval):
            print "Catastrophic Error: Skipit = "+ str(n) + "; Interval T = " + str(currentTime) + "; Interval Down T = " + str(currentTime-lastReceivedTimeStamp[n])
            #foutError.write(str(n)+str(currentTime)+","+str(currentTime-lastReceivedTimeStamp[n])+", Super Error\n")
            lastReceivedTimeStamp[n] = currentTime
        #if(currentTime - lastReceivedTimeStamp[n] > displayErrorInterval and currentTime-lastJitterTime > sendJitterInterval):
        #    print "Error: triggering random jitter"
        #    sendJitter(str(n), "magnitude", rollingAvg[n], ((.5-random.random())/10))
        #    lastJitterTime = currentTime
