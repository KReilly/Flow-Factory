from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import os

#Create custom HTTPRequestHandler class
class skipit(BaseHTTPRequestHandler):
  
  #handle GET command
  def do_GET(self):
    args = self.path.split('?')[1]
    assignments = args.split('&')
    skipit = assignments[0].split('=')[1]
    mag = assignments[1].split('=')[1]
    
    print "skipit = " + str(skipit)
    print "mag = " + str(mag)
    
    #send code 200 response
    #self.send_response(200)

    #send file content to client
    self.wfile.write("assigned")
    return


def run():
  print('http server is starting...')

  #ip and port of servr
  #by default http server port is 80
  server_address = ('192.168.1.2', 8080)
  httpd = HTTPServer(server_address, skipit)
  print('http server is running...')
  httpd.serve_forever()
  
if __name__ == '__main__':
  run()