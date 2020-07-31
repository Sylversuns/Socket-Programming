To request the file "shortfile.txt" from Port 12344 on localhost, with 12343 for the data port
python3 ftclient.py localhost 12344 -g shortfile.txt 12343

To request the directory to be listed out with the same specifications
python3 ftclient.py localhost 12344 -l 12343
