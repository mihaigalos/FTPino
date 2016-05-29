/*****************************************************************************
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
*
******************************************************************************/
#pragma once
#include "Config.h"
#ifdef CLIENT_ACTIVE
#include "application.h"

typedef enum TE_FTPClient_WriteMode{
    TE_FTPClient_WriteMode_Unknown,
    TE_FTPClient_WriteMode_Append,      // appends to the end of the file
    TE_FTPClient_WriteMode_Overwrite    // creates a new file and writes to it. If file already exists, it gets deleted and recreated with the same name
};

class FTPClient{
    String server;
    String username, password;
    
    TCPClient client;   // client for mitigating the connection, credentials, etc
    TCPClient dclient;  // client for handling the datastream
    
    char outBuf[128];
    uint8_t outCount;
public:
    FTPClient(String &_server, String &_username, String &_password){
        server = _server; username = _username; password = _password;
    }    
    
    char readServerResponse();
    String send(String &stringToWrite, String &remoteFile, TE_FTPClient_WriteMode writeMode=TE_FTPClient_WriteMode_Append);
    void onFail();
    
    
};
#endif // CLIENT_ACTIVE