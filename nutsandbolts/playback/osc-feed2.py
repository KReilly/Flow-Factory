from OSC import OSCClient, OSCMessage
import time


# OSC
client = OSCClient()
client.connect( ("localhost", 7110) )

client2 = OSCClient()
client2.connect( ("localhost", 7111) )

while(True):
	o = open('real_world_skipit-adam-med.dat', 'r')
	for line in o:
		print line
		time.sleep(0.05)
		msg = OSCMessage()
		msg.setAddress("/skip_data")
		msg.append(line)
		client.send(msg)

		msg2 = OSCMessage()
		msg2.setAddress("/skip_data2")
		msg2.append(line)
		client2.send(msg2)