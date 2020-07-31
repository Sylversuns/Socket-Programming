# By Daniel Lindsay, based on instructor (Stephen Redfield)'s code from lecture, and pythonforbeginners.com

# Import socket API
from socket import *

# Import parameters from command line
import sys

# Set server port to the 2nd parameter from the command line
serverPort = int(sys.argv[1])

# Make server socket and handle (MrServer)
serverSocket = socket(AF_INET,SOCK_STREAM)
serverHandle = 'MrServer> '
sentence = ''
serverSocket.bind(('',serverPort))

# Keep server alive forever
while(1):

  serverSocket.listen(1)
  print 'The server is ready to receive'
  connectionSocket, addr = serverSocket.accept()

  # Keep conversation alive forever until someone says quit
  while(1):
    sentence = connectionSocket.recv(1024)
    # If client says quit, quit
    if '\quit' in sentence:
      connectionSocket.close()
      break
    else:
      # Print what the client said, and send the server's response
      print sentence
      text = raw_input(serverHandle)
      connectionSocket.send(serverHandle + text)
      # If server says quit, quit
      if '\quit' in text:
	      connectionSocket.close()
        break