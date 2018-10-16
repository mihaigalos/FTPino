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
#include "Buzz.h"
#include "Config.h"

#if - 1 != BUZZER_PIN
int Buzzer::outPin;
int Buzzer::melody[3] = {2551, 2551, 2551};
int Buzzer::noteDurations[3] = {8, 8, 8};
#endif
