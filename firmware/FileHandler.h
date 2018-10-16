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
#include "SdFat/SdFat.h"
#include "application.h"
#include <map>
typedef enum TEFileSystem {
  TEFileSource_Unknown,
  TEFileSource_Eeprom,
  TEFileSource_SdCard
};

typedef enum TEFileType {
  TEFileType_Unknown,
  TEFileType_File,
  TEFileType_SymbolicLink,
  TEFileType_Folder
};

class IFileHandler;
class FilePropeties;

class FileHandlerFactory {
public:
  static IFileHandler *newFileHandler(TEFileSystem, const int chipSelect = A2);
};

//--------------------------- Specialization of the Printer to redirect certain
//SdFat streams to String and String to char* to output in streams
//--------------------------------

class PrintHelper : public Print {
public:
  unsigned char buffer[1024];
  size_t byteCount;
  PrintHelper() { memset(buffer, 0, sizeof(buffer)); }
  unsigned char *c_str(String s) {
    s.getBytes(buffer, sizeof(buffer));
    return &buffer[0];
  }
  size_t write(uint8_t character) {
    if (byteCount >= sizeof(buffer))
      byteCount = 0;
    buffer[byteCount++] = character;
    return 1;
  }
  String getBuffer() { return String(reinterpret_cast<char *>(&buffer[0])); }
};

//--------------------------- Abstract base for the file handler
//--------------------------------

class IFileHandler { // interface / base class

public:
  virtual String getDirList() = 0;
  virtual int deleteTarget(String target, bool isDir) = 0;
  virtual int changeDir(String destination) = 0;
  virtual int flush() = 0;
  virtual uint32_t fileSize(String file) = 0;
  virtual int readFile(String file, uint8_t *out, uint32_t maxOutSize) = 0;
  virtual int writeFile(String file, char *contents, uint16_t length,
                        bool isAppend = false) = 0;
  virtual int changeToParentDir() = 0;
  virtual int makeDir(String dirName) = 0;
  virtual int renameFrom(String oldFile) = 0;
  virtual int renameTo(String newFile) = 0;
};

class FilePropeties {
  String permissions;
  String owner, group;
  uint32_t lastModified; // UNIX timestamp
  TEFileType type;
  String contents;
  friend class Eeprom_FileHandler;
  friend class SDCard_FileHandler;
  friend class IFileHandler;
};

//--------------------------- Specializations of the file handler
//--------------------------------

class Eeprom_FileHandler : public IFileHandler {

  std::map<String, FilePropeties> filemap;

public:
  Eeprom_FileHandler();
  String getDirList();
  int deleteTarget(String target, bool isDir);
  int changeDir(String destination){};
  int flush() {}
  uint32_t fileSize(String file) {}
  int readFile(String file, uint8_t *out, uint32_t maxOutSize);
  int writeFile(String file, char *contents, uint16_t length,
                bool isAppend = false);
  int changeToParentDir() {}
  int makeDir(String dirName) {}
  int renameFrom(String oldFile) {}
  int renameTo(String newFile) {}
};

class SDCard_FileHandler : public IFileHandler {
  SdFat SD;
  SdFile myFile;
  String cwd;
  String renameFromName;
  static void dateTime(uint16_t *date, uint16_t *time);

public:
  SDCard_FileHandler(const int chipSelect);
  String getDirList();
  int deleteTarget(String target, bool isDir);
  int changeDir(String destination);
  int flush();
  uint32_t fileSize(String file);
  int readFile(String file, uint8_t *out, uint32_t maxOutSize);
  int writeFile(String file, char *contents, uint16_t length,
                bool isAppend = false);
  int changeToParentDir();
  int makeDir(String dirName);
  int renameFrom(String oldFile);
  int renameTo(String newFile);
};
