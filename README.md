# FTPino
A tiny FTP Server and Client for your Photon / Arduino projects.
![alt tag](Doc/FTPino_FTPServer.jpg?raw=true "FTPino")

#Description
FTPino is a lightweight FTPServer and/or client for logging, storing or fetching your data.
It is built on the Particle's Photon hardware and comes in two flavors:

- SDCard Filesystem : based on SDFatLib's library, the filesystem is on a SD Card and I/O operations on files are performed on the card. 
- EEProm Filesystem : experimental, not yet fully functional. Support will be added for storing small files in the EEPROM, so the I/O operations on files will be made there.

#Hardware

# Code requirements
FTPino was tested only on Particle's Photon only. The linker output is presented below (server-only version, for client version flash usage is 1kB higher).

Output of arm-none-eabi-size:

text	data	bss		dec		hex
37740	208		1180	39128	98

In a nutshell:
Flash used	37948 / 110592	34.3 %
RAM used	1388 / 20480	6.8 %