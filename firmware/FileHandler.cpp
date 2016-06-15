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
#include "SdFat/SdFat.h"
#include "FileHandler.h"

#include <map>
#include <vector>



//--------------------------------------- Factory for different supported file types --------------------------------------------------------------
IFileHandler* FileHandlerFactory::newFileHandler(TEFileSystem filesystem, const int chipSelect){
    IFileHandler* newFileHandler = NULL;  
    switch (filesystem){
        case TEFileSource_Eeprom:   newFileHandler = new Eeprom_FileHandler();  break;
        case TEFileSource_SdCard:   newFileHandler = new SDCard_FileHandler(chipSelect);  break;
        
        TEFileSource_Unknown:
        default:
        break;
    }
    return newFileHandler;
    
}


//--------------------------------------- EEPROM file types --------------------------------------------------------------
Eeprom_FileHandler::Eeprom_FileHandler(){
    
    FilePropeties fp;
    
    fp.owner = "Mihai"; fp.group = "Mihai"; fp.permissions = "-rwx------+"; 
    fp.lastModified = Time.now(); // example : 1462903935
    fp.type = TEFileType_File;
    fp.contents = "Mary had a little lamb.";
    
    filemap["data.txt"] = fp;
}

String Eeprom_FileHandler::getDirList(){
    String ret;
    for(std::map<String, FilePropeties>::iterator i = filemap.begin(); i != filemap.end(); ++i ){
        String filename = i->first; FilePropeties fp = i->second;
        ret+= fp.permissions+" 1 "+fp.owner+" "+fp.group+" "+String(fp.contents.length())+" "+Time.format(fp.lastModified, " %b %d %H:%M ")+" "+filename+"\r\n"; // example : Aug 23 14:55
    }
    return ret;
}

int Eeprom_FileHandler::readFile(String file, uint8_t* out , uint32_t maxOutSize){
    //return (filemap.end() != filemap.find(file))    ?   filemap[file].contents     :   "<Unknown file> : "+file;
    return 0;
}

int Eeprom_FileHandler::deleteTarget(String target, bool isDir){
    int result = -1; std::map<String,FilePropeties>::iterator it;
    if(filemap.end() != (it = filemap.find(target))) filemap.erase(it), result = 0;
    return result;
}

int Eeprom_FileHandler::writeFile(String file, char* contents, uint16_t length, bool isAppend){
    int result = -1; /* std::map<String,FilePropeties>::iterator it;
    if(filemap.end() != (it = filemap.find(file))){
        filemap[file].contents = contents; filemap[file].lastModified = Time.now() ;
        result = 0;
    }*/
}

//--------------------------------------- StringUtils for manipulating Strings --------------------------------------------------------------

class StringUtils{
    public:
    static std::vector<String> split(String input, String token){ // splits a string on given tokens
        std::vector<String> result;    
        for(int startPos =0, nextPos=0; (startPos<input.length());){
           // while ( (input.substring(startPos,nextPos) == token) && startPos<input.length()) startPos++;
            nextPos = input.indexOf(token, startPos+1); if (-1 == nextPos) break;
            result.push_back(input.substring(startPos, nextPos)); startPos =nextPos + token.length();
        }
        return result;
    }
    
    static String getFilename               (String fileInfo){ return fileInfo.substring(fileInfo.lastIndexOf(" ")+1,   fileInfo.length());             }
    static String getFilesize               (String fileInfo){ return fileInfo.substring(fileInfo.indexOf(" ",18)+1,    fileInfo.lastIndexOf(" "));      }
    static String getFilelastModification   (String fileInfo){ 
        const char months[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
        String month    = String(months[String(String(fileInfo[5])+String(fileInfo[6])).toInt()-1]);
        String day      = String (fileInfo[8])+String(fileInfo[9]);
        String timestamp=fileInfo.substring(fileInfo.indexOf(" ")+1,fileInfo.indexOf(" ")+6);
        return month + " "+ day +" "+timestamp;
    }
    
};


//--------------------------------------- SDCard file types --------------------------------------------------------------

void SDCard_FileHandler::dateTime(uint16_t* date, uint16_t* time) {
    *date = FAT_DATE(Time.year(),   Time.month(),     Time.day());      // return date using FAT_DATE macro to format fields
    *time = FAT_TIME(Time.hour(),   Time.minute(),    Time.second());  // return time using FAT_TIME macro to format fields
}

SDCard_FileHandler::SDCard_FileHandler(const int chipSelect){
    SdFile::dateTimeCallback(dateTime);
    cwd ="/";
    if (!SD.begin(chipSelect, SPI_HALF_SPEED)) SD.initErrorHalt(), Particle.publish("FTPino", "error initializing SD card."),delay(1000), System.reset(); // Adieu, World!..
}

String SDCard_FileHandler::getDirList(){
    String ret;
    String defaultFilePermissions = "-rwx------+", defaultFolderPermissions = "drwx------+", defaultUser = "User", defaultGroup ="Group";
    
    PrintHelper ph; Print* p = &ph;
    SD.ls(p, LS_DATE | LS_SIZE); // get files as listing, result will be in the ph pointer. Contents is accessible with ph->getBuffer()
    
    if(0 < ph.getBuffer().length() ){
        auto allFileInfos = StringUtils::split(ph.getBuffer(), "\r\n");
        
        for(auto fileInfo: allFileInfos){
            auto fileName = StringUtils::getFilename(fileInfo); bool isFolder = false;
            if(-1 != fileName.indexOf("/")) ret += defaultFolderPermissions, isFolder = true; // is  a folder
            else  ret += defaultFilePermissions;
            ret+=" "+defaultUser+" "+defaultGroup+" "+(!isFolder ? (StringUtils::getFilesize(fileInfo)):("0")) +" "+StringUtils::getFilelastModification(fileInfo)+" "+fileName+"\r\n";
        }
    }
    return ret;
}

int SDCard_FileHandler::readFile(String file, uint8_t* buffer, uint32_t maxOutSize){
    
    int bytesRead = -1; 
    bool ok = false;
    
    if(!myFile.isOpen()) ok = myFile.open(file,O_READ);
    else ok = true;
	if (ok){
        bytesRead = myFile.read(buffer, maxOutSize);
	}

    return bytesRead;
    
}

int SDCard_FileHandler::flush(){
    myFile.close();
}

int SDCard_FileHandler::deleteTarget(String target, bool isDir){
    int result = -1; 
    if(isDir){ if (SD.rmdir(cwd+target))    result = 0;}
    else     { if (SD.remove(target))       result = 0;}
    return result;
}

int SDCard_FileHandler::writeFile(String file, char* contents, uint16_t length, bool isAppend){
    auto result = -1; bool ok = false;
    
    if(!myFile.isOpen()) {
        if(!isAppend)   ok = myFile.open(file, O_WRITE | O_CREAT);
        else            ok = myFile.open(file, O_WRITE | O_CREAT | O_APPEND);
    }
    else ok = true;
	if (ok){
	    auto iterator = 0;
	    result = myFile.write(contents, length) ;
	}
	return result;
}

uint32_t SDCard_FileHandler::fileSize(String file){
    SdFile myFile; myFile.open(file);
    auto size = myFile.fileSize();
    return size;
}

int SDCard_FileHandler::changeDir(String destination){
    auto result = -1;
    char buf[255]; destination.toCharArray(buf, sizeof(buf));
    if(destination == cwd) result = 0; // nothing to do, fall-through
    else if(SD.chdir(buf)) cwd+=destination+"/", result = 0;
    return result;
    
}

int SDCard_FileHandler::changeToParentDir(){
    auto result = -1; 
    if(!cwd.equals("/")){ // not root
        auto foldersPath = StringUtils::split(cwd,"/"); auto depth = foldersPath.size()-1; cwd ="";
        
        for(int i= 0;i<depth; i++) cwd += foldersPath[i]+"/";
        if(0 == cwd.length()) cwd = "/";
        char buf[255]; cwd.toCharArray(buf, sizeof(buf));
        if(SD.chdir(buf)) result = 0;
    }
    return result;
}

int SDCard_FileHandler::makeDir(String dirName){
    auto result = -1;
    if(SD.mkdir(dirName)) result = 0;
    return result;
}

int SDCard_FileHandler::renameFrom(String oldFile){
    renameFromName = oldFile;
    return 0;
}

int SDCard_FileHandler::renameTo(String newFile){
    auto result = -1;
    if(SD.rename(renameFromName, newFile)) result = 0;
    return result;
}