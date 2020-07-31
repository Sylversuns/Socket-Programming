# Programmer Name: Daniel Lindsay
# This program is based on information from GeeksForGeeks, W3Schools, and Python.org
# Program Name: FT Client
# Program Description: A client that connects to a server to request and receive 
#		       either a directory or a file in the directory
# Course Name: CS372
# Last Modified: 11/25/2018

# Import socket functionality and sys, for command line arguments
# Also import os, to look at files in current directory
from socket import *
import sys
import os

print (sys.argv[0:])

# Give names to the arguments
webAddress = sys.argv[1]
controlPort = int(sys.argv[2])
command = sys.argv[3]

# Give names to the arguments. The dash in the if statement condition 
# is different from the one on the next line, so I had to adjust it.
if (command == "-g"):
    filename = sys.argv[4]
    originalFilename = filename
    f = open(filename, "w")
    dataPort = sys.argv[5]
else:
    dataPort = sys.argv[4]

# Set up control and data sockets.
controlSocket = socket(AF_INET,SOCK_STREAM)
controlSocket.connect((webAddress, controlPort))
dataSocket = socket(AF_INET,SOCK_STREAM)
dataSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
dataSocket.bind(('',int(dataPort)))
dataSocket.listen(1)
print ("The data port is listening.")

# Send the parameters. If it's a "-g" request, send 3. If not, 2. (No filename)
if (command == "-g"):
    controlSocket.send((command + ' ' + dataPort + ' ' + filename).encode('utf-8'))
else:
    controlSocket.send((command + ' ' + dataPort).encode('utf-8'))

# Receive connections and messages.
#print ((controlSocket.recv(10000)).decode('utf-8'))
dataSocket, addr = dataSocket.accept()
#print ((dataSocket.recv(10000)).decode('utf-8'))

while (True):
    data = dataSocket.recv(10000)
    if (data):
        while(os.path.exists(filename)):
            filename = filename + "x"
        print (originalFilename + " already exists. This will be written to " + filename)
        f = open(filename, "a+")
        print (str(data.decode('utf-8')))
        f.write(str(data.decode('utf-8')))
        print ("Transfer complete!")
        f.close()
    else:
        break

controlSocket.close()
