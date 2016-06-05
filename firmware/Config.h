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

#define DEBUG 2         // 1: Stream debug stream to Cloud, 2: to Serial USB, 9: all.
#define FILESYSTEM 2    // 1: eeprom, 2: external sd card. 
#define BUZZER_PIN  D0  // if you have a piezo-electric buzzer attached to your setup, give in its pin. Otherwise set to -1 to disable.

#define ERR_BUFFER_FULL (-16)

#define SERVER_ACTIVE
#define CLIENT_ACTIVE