import time
from datetime import datetime
import web
import math
from OSC import OSCClient, OSCMessage

activeSkipIts = [3,4] #set skipits selected here, streams coorespond to index

urls = (
  '/data', 'Index'
)

app = web.application(urls, globals())

#OSC for relay
nutsAndBoltsClient = OSCClient()
nutsAndBoltsClient.connect( ("localhost", 7210) )

#Relays data to osc
def oscSend(skipitID, tag, load):
    tag = "/" + str(skipitID) + tag
    msg = OSCMessage()
    msg.setAddress(tag)
    msg.append(load)
    nutsAndBoltsClient.send(msg)


#define skipit tied to active stream  


class Index(object):
    prevMag = 0

    def GET(self):
        #Parse Input
        form = web.input(skipit="skipit",magnitude="magnitude")

        skipit = int(form.skipit)
        magnitude = float(form.magnitude)
        print "skipit = " +str(skipit)
        print "Magnitude = " + str(magnitude);
        print str(datetime.now())

        selectedStream = -1;

        if activeSkipIts[0] == skipit:
            selectedStream = 0
        elif activeSkipIts[1] == skipit:
            selectedStream = 1
        else:
            print str(skipit) + " inactive - put to sleep." 
            return "sleep"

        oscSend(selectedStream, "/magnitude", magnitude)
        return "assigned"
       

if __name__ == "__main__":
    app.run()