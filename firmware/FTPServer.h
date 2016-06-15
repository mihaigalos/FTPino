/**************************************************************************************************************************
* 
* Name:        FTPino FTPServer
* Author:      Mihai Galos
* Timestamp:   16 May 2016
* 
* Tested with: FTPRush     2.1.8   : Active Mode(OK) / Passive Mode (Ok, no STOR)
*              FileZilla   3.17.0.1: Passive Mode (OK, no STOR, i.e.: no sending of files to FTPino) / Active Mode: Not Supported
* 
* Notes:
*      
*      - Please make sure to set Maximum Simultaneous connections to 1 in your client. This will then only use a single 
*        data socket for both storing and retrieving files to and from the server.
*      - For best results, please use FTPRush in active mode. 
*      - A STOR command in passive mode does not store the last couple of kB of a file. This is probably because the 
*          client side closes and flushes the output buffer before the FTPino server has had a chance to read it all. Use the active 
*          mode for tranferring files to and from FRPino.
*      - Dataspeed:
*          Particle Photon : 500kB/s Down, 300kB/s Up
* 
* 
* This file is part of FTPino.
* 
* FTPino is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* FTPino is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with FTPino.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************************************************************/

#pragma once
#include "Config.h"
#ifdef SERVER_ACTIVE
#include "application.h"
#include <vector>
#include "FileHandler.h"
using namespace std;

typedef enum {
    TEftpTransferMode_Unknown, TEftpTransferMode_Active, TEftpTransferMode_Passive
}TEftpTransferMode;

typedef enum{
	TEftpState_Unknown,         
	TEftpState_Init,			TEftpState_Welcome,			TEftpState_User,			TEftpState_Pass,		TEftpState_AuthOk,
	TEftpState_AuthFail,		TEftpState_CurrentDir,		TEftpState_List,			TEftpState_System,		TEftpState_Features,
	TEftpState_Type,			TEftpState_ChangeDir,		TEftpState_Passive,		 	TEftpState_RetrieveFile,TEftpState_Store,
	TEftpState_Append,			TEftpState_DeleteFile,		TEftpState_DeleteDir,       TEftpState_RestartAt,   TEftpState_Port,        
	TEftpState_ParentDir,       TEftpState_MakeDir,         TEftpState_RenameFrom,      TEftpState_RenameTo,    TEftpState_Client,
	TEftpState_Quit
}TEftpState;



typedef struct{
	String username, password; 
}TSCredentials;



class FTPServer{
	TSCredentials credentials, receivedCredentials;
	TEftpState state;
	
	uint16_t port;
	TCPServer* server=NULL, *dserver=NULL;
	TCPClient client, dclient;   // client control/data socket for mitigating the connection, credentials, etc / sending and receiving data
    IPAddress remoteIp;
    
    bool isFTPinoClient;
    int16_t timeoutSec; uint32_t aliveTimer;

	const int passiveDataPortHi = 4, passiveDataPortLo = 0;
	int activeDataPortHi, activeDataPortLo = 0;
	static uint32_t totalConnections;
	
	TEftpTransferMode transferMode;
	IFileHandler* fh;
	TEftpState parseCommand(String response, String &info);
	inline void waitForClientDataConnection();
	
	String  dataRead    ();   // low-level read
	void	dataWrite   (String data);
	void    readFile    (String file);
	void    writeFile   (String file, IFileHandler* fh, bool isAppend = false);
	
public:
	
	FTPServer(String & user, String &pass, uint16_t _port = 21, int16_t _timeoutSec=-1, const int chipSelectSdCard = A2 ); 
	~FTPServer();
	String run();
	
};
#endif // SERVER_ACTIVE