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

#if -1 != BUZZER_PIN
#include "application.h"
class Buzzer{
    static int outPin;
    static int melody[3];
    static int noteDurations[3];
    
    
    
    public:
    
    static void init(int outPin){
        outPin = outPin;
    }
    
    static void beepTwice(){
        for (int thisNote = 0; thisNote < 2; thisNote++) {
            int noteDuration = 1000/noteDurations[thisNote];
            tone(outPin, melody[thisNote],noteDuration);
    
            int pauseBetweenNotes = noteDuration * 1.30;
            delay(pauseBetweenNotes);
            noTone(outPin);
        }
    }
    
    static void beepOnce(){
        int noteDuration = 1000/noteDurations[2];
        tone(outPin, melody[2],noteDuration);

        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(outPin);
    }
};
#endif