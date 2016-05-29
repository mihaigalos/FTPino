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

// This #include statement was automatically added by the Particle IDE.
#include "SdFat/SdFat.h"

#include "Buzz.h"

// This #include statement was automatically added by the Particle IDE.
#include "FileHandler.h"

// This #include statement was automatically added by the Particle IDE.
#include "FTPServer.h"

#include "FTPClient.h"


#define TIMEZONE (+2)

String user         = "Mihai";
String pass         = "Pass";


FTPServer *ftpServer = NULL;

void setup()
{
    Time.zone(TIMEZONE);
    ftpServer = new FTPServer(user, pass, 21, 63);
    
    Particle.publish("FTPinoIP", String(WiFi.localIP()));
    
    pinMode(D7, OUTPUT);
    digitalWrite(D7,LOW);
}

void loop()
{
    
   auto status = ftpServer->run();
   if(status.length()>0) Particle.publish("FTPServer"), delay(1000);
}






