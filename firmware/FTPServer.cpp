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
#include "Config.h"

#ifdef SERVER_ACTIVE
    
#include "Buzz.h"
#include "SdFat/SdFat.h"
#include "application.h"
#include "FTPServer.h"
#include "Buzz.h"

uint32_t FTPServer::totalConnections;


#ifdef DEBUG
ArduinoOutStream cout(Serial);
class OutHelper{
public:    
    static unsigned char buffer [1024];
    
    static unsigned char* c_str(String s){
        s.getBytes(buffer, sizeof(buffer));
        return &buffer[0];
    } 
};
unsigned char OutHelper::buffer [1024];

    void dbg(String param1, String param2){
        
        
        
        #if (DEBUG == 1) || (DEBUG == 9)
        Particle.publish(param1, param2); delay(1000);
        #elif (DEBUG == 2)|| (DEBUG == 9)
        cout<<"[ "<<OutHelper::c_str(Time.timeStr())<<" ]     "<<OutHelper::c_str(param1)<<" -> "<<OutHelper::c_str(param2)<<endl; delay(30); //throttle
        #endif
    }
# else 
inline void dbg(String param1, String param2) {}
#endif

FTPServer::FTPServer(String & user, String &pass,  uint16_t _port, int16_t _timeoutSec){
	#if DEBUG > 0
    Serial.begin(9600);
    #endif
	credentials.username = user; credentials.password = pass; port = _port;
	server  = new TCPServer(port);   server->begin();
	dserver = new TCPServer((passiveDataPortHi<<8) | (passiveDataPortLo & 255)); dserver->begin();
	
	pinMode(D7, OUTPUT);	digitalWrite(D7,LOW);
	totalConnections = 0;
	#if -1 != BUZZER_PIN
	Buzzer::init(D0);
	#endif
	fh = FileHandlerFactory::newFileHandler(static_cast<TEFileSystem>(FILESYSTEM));
	if(_timeoutSec > 0) timeoutSec = _timeoutSec;
	
}

FTPServer::~FTPServer(){
	#if DEBUG > 0
    Serial.end();
    #endif
}

TEftpState FTPServer::parseCommand(String response, String &info){
	TEftpState newState = TEftpState_Unknown; auto posEnd = response.indexOf('\r');
	if(response.startsWith("USER")){
	    receivedCredentials.username = response.substring(String("USER ").length(), posEnd); info = receivedCredentials.username;   newState = TEftpState_User;
	}else if(response.startsWith("PASS")){
		receivedCredentials.password = response.substring(String("PASS ").length(),  posEnd);
		if( (credentials.username == receivedCredentials.username) && (credentials.password == receivedCredentials.password))       newState = TEftpState_AuthOk;
		else                                                                                                                        newState = TEftpState_AuthFail;
	}else if(response.startsWith("PORT")){
	    auto lastComma= response.lastIndexOf(",");	    auto secondLastComma = response.lastIndexOf(",",lastComma-1);
	    auto hiPort = response.substring(secondLastComma+1, lastComma); auto loPort = response.substring(lastComma+1, posEnd);
	    activeDataPortHi= hiPort.toInt(); activeDataPortLo = loPort.toInt();
	    newState = TEftpState_Port;
	}   
	
	else if(response.startsWith("PWD"))		newState = TEftpState_CurrentDir;		else if(response.startsWith("QUIT"))		    newState = TEftpState_Quit;	
	else if(response.startsWith("FEAT"))	newState = TEftpState_Features;			else if(response.startsWith("SYST"))		    newState = TEftpState_System;
	else if(response.startsWith("PASV"))	newState = TEftpState_Passive;			else if(response.startsWith("LIST"))		    newState = TEftpState_List;
	else if(response.startsWith("TYPE"))	newState = TEftpState_Type;         	else if(response.startsWith("CDUP"))            newState = TEftpState_ParentDir;
	
	else if(response.startsWith("REST"))    newState = TEftpState_RestartAt,    info = response.substring(String("REST ").length(), posEnd);
	else if(response.startsWith("RETR"))	newState = TEftpState_RetrieveFile, info = response.substring(String("RETR ").length(), posEnd);
	else if(response.startsWith("DELE"))	newState = TEftpState_DeleteFile,   info = response.substring(String("DELE ").length(), posEnd);
	else if(response.startsWith("STOR"))	newState = TEftpState_Store,        info = response.substring(String("STOR ").length(), posEnd);
    else if(response.startsWith("MKD"))	    newState = TEftpState_MakeDir,      info = response.substring(String("MKD ") .length(), posEnd);

	else if(response.startsWith("APPE"))	newState = TEftpState_Append,       info = response.substring(String("APPE ").length(), posEnd);
	else if(response.startsWith("CWD"))		newState = TEftpState_ChangeDir,    info = response.substring(String("CWD ") .length(), posEnd);
	else if(response.startsWith("RNFR"))    newState = TEftpState_RenameFrom,   info = response.substring(String("RNFR ").length(), posEnd);
	else if(response.startsWith("RNTO"))    newState = TEftpState_RenameTo,     info = response.substring(String("RNTO ").length(), posEnd);
	
	else if(response.startsWith("RMD"))     newState = TEftpState_DeleteDir,    info = response.substring(String("RMD ") .length(), posEnd);
	
	else info = response.substring(0,posEnd);
	
    if((-1 != aliveTimer) &&(response.length()>0)) aliveTimer = millis() + timeoutSec*1000;
	return newState;
	
}


void FTPServer::waitForClientDataConnection(){
	do{		dclient = dserver->available(); Particle.process();} while(!dclient.connected());
}

String FTPServer::dataRead(){
	char inBuffer[1024]; memset(inBuffer, 0, sizeof(inBuffer)); uint8_t pos =0 ;
	while (client.available())	inBuffer[pos++]= client.read();
	return String(inBuffer);
}

void FTPServer::dataWrite(String data){
	server->println("150 Opening ASCII mode data connection for transfer."); 
	dserver->write(data);	 
	dclient.stop(); dclient.flush();
	server->println("226 Transfer complete."); 
}


void FTPServer::readFile(String file){
    auto bytesRead =0; auto totalBytes = 0;
    server->println("150 Opening ASCII mode data connection for transfer."); 
    uint8_t buf[256]; auto fileSize = fh->fileSize(file);
    do{
        bytesRead = fh->readFile(file, buf, sizeof(buf));
        auto writtenBytes = 0; auto lengthToWrite = sizeof(buf); 
        if(totalBytes + sizeof(buf) >=fileSize) lengthToWrite = fileSize-totalBytes;  // account for very last iteration, do not print buffer twice  
        do{
                 if (transferMode==TEftpTransferMode_Active)        writtenBytes = dclient.write(buf, lengthToWrite);    
            else if (transferMode==TEftpTransferMode_Passive)       writtenBytes = dserver->write(buf, lengthToWrite); /* use passive store at your own risk. It doesn't write the last couple of kB of a file.
                                                                                                                        This is probably because the client side closes and flushes the output buffer before the FTPino 
                                                                                                                        server has had a chance to read it all. Use the active mode for tranferring files to and from FRPino.*/
            
        }while (ERR_BUFFER_FULL == writtenBytes);
        totalBytes+= writtenBytes;
    }while( (bytesRead == sizeof(buf)) && bytesRead > 0); // continue while still reading chunks of sizeof(buffer)from card..
    dclient.stop(); fh->flush();
    server->println("226 Transfer complete. "+String(totalBytes)+" read."); 
}

void FTPServer::writeFile(String file, IFileHandler* fh){ 
    server->println("150 Opening BINARY mode data connection for file transfer.");
    
	uint32_t pos =0; auto totalBytes = 0; uint8_t readBuffer[256]; 
	if(!dclient.connected()) {server->println("425 No data connection"); return; }
	else{
         auto bytesRead =0;
         do{
            bytesRead = dclient.read(readBuffer, sizeof(readBuffer));
            if(bytesRead>0){
                fh->writeFile(file, reinterpret_cast<char*>(readBuffer), bytesRead);
                totalBytes+= bytesRead, bytesRead = 0; 
            }
        }while( dclient.connected() || dclient.available());
	} 
	dclient.stop();dclient.flush();
    fh->flush();
	server->println("226 Transfer complete. "+String(totalBytes)+" bytes written.");
}

String FTPServer::run(){

	
	if (client.connected()) {
		String clientIP = String(remoteIp = client.remoteIP());
		dbg("-------------------------------------------------------------","");
		dbg("FTPino", "Client connected: "+clientIP);
		#if -1 != BUZZER_PIN
		Buzzer::beepTwice();
		#endif
		digitalWrite(D7,HIGH);
		String clientResponse, parseInfo;			
		
		if(timeoutSec>0)    aliveTimer = millis() + timeoutSec*1000;
	    else                aliveTimer = -1; 
		
			while(client.connected()){
				
				switch(state){
					case TEftpState_Init:           server->println("220 Welcome to FTPino FTP Server.");							                break;
					case TEftpState_User:           server->println("331 Password required for user "+parseInfo+".");				                break;
					case TEftpState_AuthOk:         server->println("230 User successfuly logged in.");							                    break;
					case TEftpState_AuthFail:       client.stop();																	                break;
					
					case TEftpState_CurrentDir:     server->println("257 \"/\" is current directory.");							                    break;
					case TEftpState_System:         server->println("215 UNIX emulated by FTPino.");								                break;
					case TEftpState_Features:       server->println("211 Extensions supported SIZE MDTM XCRC.");					                break;
					case TEftpState_Type:           server->println("200 Type set to BINARY.");    	                                                break;
					
					case TEftpState_RestartAt:      server->println("350 Restarting at "+parseInfo+". !!! Not implemented");                        break;
					case TEftpState_Store:          writeFile(parseInfo, fh);                                                                       break;
					case TEftpState_DeleteDir:      if(fh->deleteTarget(parseInfo,true)<0) server->println("550 Can't delete Directory.");     
					                                else server->println("250 Directory "+parseInfo+" was deleted.");                               break;
					case TEftpState_DeleteFile:     if(fh->deleteTarget(parseInfo,false)<0) server->println("550 Can't delete File.");     
					                                else server->println("250 File "+parseInfo+" was deleted.");                                    break;
					case TEftpState_MakeDir:        if(fh->makeDir(parseInfo)<0) server->println("550 Can't create directory."); 
					                                else server->println("257 Directory created : "+parseInfo+".");                                 break;                                                             
					                                
					case TEftpState_RenameFrom:     fh->renameFrom(parseInfo); server->println("350 Directory exists, ready for destination name"); break;
					case TEftpState_RenameTo:       fh->renameTo(parseInfo);   server->println("250 Directory renamed successfully");               break;
					case TEftpState_List:           dataWrite(fh->getDirList());										                            break; // implementing LIST verb as described at : https://files.stairways.com/other/ftp-list-specs-info.txt
					case TEftpState_RetrieveFile:   readFile(parseInfo);                                                                            break;
					case TEftpState_Quit:	        server->println("221 Bye.");	client.stop(); dclient.stop();                                  break;
					case TEftpState_ParentDir:      fh->changeToParentDir(); server->println("250 Going one level up.");                            break;
					case TEftpState_Port: {
					    transferMode = TEftpTransferMode_Active;
					    dclient.connect(remoteIp, activeDataPortHi<<8 | activeDataPortLo);server->println("200 Port command successful."); 
					    break;
					}
					case TEftpState_Passive: {
						auto ip = WiFi.localIP(); char buffer [1024]; 
						sprintf(buffer,"(%d,%d,%d,%d,"+String(passiveDataPortHi)+ ","+String(passiveDataPortLo)+")",ip[0],ip[1],ip[2],ip[3]);
						server->println("227 Entering Passive Mode "+String(buffer));											
						waitForClientDataConnection();
						transferMode = TEftpTransferMode_Passive;
						break;
					}
					case TEftpState_ChangeDir: {
					    if(fh->changeDir(parseInfo)<0)  server->println("550 Can't change directory to "+parseInfo+"."); 
					    else                            server->println("250 \"/"+parseInfo+"\" is current directory.");
					    break;
					}
					
					case TEftpState_Append:{
						break;
					}
					
				}
				
			

				if( client.connected()){// if client not disconnected by the logic above
					do{
						delay(100); // allow the client to read response
						
						clientResponse = dataRead(); 
						auto newState = parseCommand(clientResponse, parseInfo);
						
						if(newState != state) state=newState;
						if((clientResponse.length() > 0) && (TEftpState_Unknown == state)) server->println("502 Command not implemented: "+parseInfo+".");
						if((-1 != aliveTimer) && (static_cast<int64_t>(millis()) -static_cast<int64_t>(aliveTimer)) > 0)  server->println("530 Timeout, disconnecting control socket."), state = TEftpState_Quit; // timeout
						
					}while(TEftpState_Unknown == state) ;  
				}
				
				
				
			}
			
			state = TEftpState_Init;
			dbg("FTPino", "Client disconnected: "+clientIP);
			fh->flush();
			#if -1 != BUZZER_PIN
			Buzzer::beepOnce();
			#endif
			totalConnections++;
	} else {
	
		client = server->available();
		
		digitalWrite(D7,LOW);
		state = TEftpState_Init;
	}



	return "";
	
}
#endif // SERVER_ACTIVE