# FTPino
A tiny FTP Server and Client for your Photon / Arduino projects.
![alt tag](Doc/FTPino_FTPServer.jpg?raw=true "FTPino")

#Description
FTPino is a lightweight FTPServer and/or client for logging, storing or fetching your data.
It is built on the Particle's Photon hardware and comes in two flavors:

- SDCard Filesystem : based on SDFatLib's library, the filesystem is on a SD Card and I/O operations on files are performed on the card. 
- EEProm Filesystem : experimental, not yet fully functional. Support will be added for storing small files in the EEPROM, so the I/O operations on files will be made there.

#Hardware
- Particle Photon (https://store.particle.io/)
- Pololu SDCard (https://www.pololu.com/product/2597)
- SDHC card (<32 GB or ==32GB, as long as it's SDHC and not SDXC). Make sure you format the card with the SDCard Formatter (https://www.sdcard.org/downloads/formatter_4/)

## SDCard breakout
Certain important things need to be considered when searching for a SDCard breakout board:
- the pin logic on the SD is at 3.3V, not 5V.
- the VDD may draw a lot of current, up to 150mA when writing. You cannot supply the board from an output I/O pin directly, for most pins cannot handle neither delivering nor sinking that much current.

If you scout the web a bit for a decent SDCard breakout board, you'll find most are only pin-compatible with Arduino. Cool, what about the Photon?
Unfortunately, I couldn't find a one that directly fits the Photon, but the Pololu cut it pretty close.

The SPI pins directly overlap those of the Photon. Only problem was with the GND and VDD pins which couldn't be directly connected.
![alt tag](Doc/FTPino_SDHolder1.jpg?raw=true "FTPino_SDHolder1")

My proposed solution consists of soldering the GND of the next pin and cutting it so it doesn't go into the Photon's pin headers.
As for the VDD, an airwire was needed.
![alt tag](Doc/FTPino_SDHolder2.jpg?raw=true "FTPino_SDHolder2")

#Speed
```
PC <- Read  <- FTPino : 500 kB/s
PC -> Write -> FTPino : 300 kB/s
```

#FTP Client (PC)
If you choose to interact with FTPino from a PC, I would reccomend using FTPRush FTP client (http://www.wftpserver.com/download.htm)
Other clients I've used are FileZilla and FreeCommander, although with those, you can only read from the remote SDCard.

PC clients interact with FTPino in FTP Active mode.

In respect to FTPRush, you must configure your client to use a single socket for data. In v2.1.8 this can be found under Options -> Transfer -> Single connection mode (must be checked).

I've had a bittersweet experience trying to validate FTPino with FileZilla. Most operations work if one respects the prerequisite of setting a single data socket. All except the Store command.
This is responsible for sending data to FTPino for it to be written to the SD. 

FTP Active mode just doesn't work. In Passive mode, the data is sent, received and written to the SD as you would expect. Except the last coouple of kB.

My only explanation as to what is happening is that FileZilla writes to FTPino's buffer without checking if the buffer is full ? and closes the connection, before the client has had a chance to 
read the whole file.. ?

# Code requirements
FTPino was tested only on Particle's Photon only. The linker output is presented below (server-only version, for client version flash usage is 1kB higher).
```
Output of arm-none-eabi-size:

text	data	bss		dec		hex
37740	208		1180	39128	98

In a nutshell:
Flash used	37948 / 110592	34.3 %
RAM used	1388 / 20480	6.8 %
```

# License
This software is freely available under the GNU GPL v3.0 aegis, please consult the LICENSE file for further information.