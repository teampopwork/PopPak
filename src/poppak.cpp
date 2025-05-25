#include <iostream>
#include <string>
#include <list>
#include <cstdarg>

#include "pakinterface.hpp"
#include "common.hpp" // get MkDir and some other helper functions.

namespace fs = std::filesystem;

using namespace PopWork;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

bool package = false;
bool unpackage = false;

//////////////////////////////////////////////////////////////////////////
// This is the default password for the PakInterface.
// The idea is to be able to change the password  on the command prompt.
std::string gEncryptPassword = "PopCapPopWorkFramework";
//////////////////////////////////////////////////////////////////////////

FILE *gDestFP = NULL;

// 0xBAC04AC0

class DeferredFile
{
  public:
	std::string mFileName;
	int mFileSize;
};

enum
{
	FILEFLAGS_END = 0x80
};

typedef std::list<DeferredFile> DeferredFileList;

DeferredFileList gDeferredFilesList;

//************************************
// Method:    enc_fwrite
// FullName:  enc_fwrite
// Access:    public
// Returns:   int
// Description: Encrypted Write to the Pak File.  All data (even the headers)
//				are written to the PAK file in an encrypted format.  The encryption
//				is a simple XOR operation where the key is a selected from the
//				gEncryptPassword.  The key rotates through the Password, so no
//				adjacent values are encrypted with the same key.
//
// Strength:	Because the file header for the PAK file is publicly known (0xBAC04AC000000000),
//				the first 8 characters of the password are easily obtainable.
//				This means two things: the password should be longer than 8 characters
//				and the password should not be guessable from the first 8 characters.
// Parameter: const void * theData
// Parameter: size_t theElemSize
// Parameter: size_t theCount
// Parameter: FILE * theFP
//************************************
int enc_fwrite(const void *theData, size_t theElemSize, size_t theCount, FILE *theFP)
{
	//////////////////////////////////////////////////////////////////////////
	// The Password Key is a modulus of the current file pointer position.
	// This is so things will line up when we decrypt.
	int aPos = ftell(theFP);
	int aPWSize = gEncryptPassword.length();

	int aSize = theElemSize * theCount;
	char *aBuf = new char[aSize];

	// Encrypt with a key from the Password using XOR, and then increment to the
	// next PW Key.
	for (int i = 0; i < aSize; i++)
		aBuf[i] = ((char *)theData)[i] ^ gEncryptPassword[(aPos++) % aPWSize]; // post increment

	int aCount = fwrite(aBuf, theElemSize, theCount, theFP);
	delete[] aBuf;
	return aCount;
}

//************************************
// Method:    AddFiles
// FullName:  AddFiles
// Access:    public
// Returns:   void
// Description: This method iterates through all of the files in the source
//				directory and does two things:
//					1. Writes the PakRecord Data to the Pak File
//					2. Stores the File information for later when we
//					   add the file to the end of the PAK file.
// Parameter: const std::string & theDir
// Parameter: const std::string & theRelDir
//************************************
void AddFiles(const std::string &theDir, const std::string &theRelDir)
{
	if (!fs::exists(theDir) || !fs::is_directory(theDir)) {
		std::cerr << "Directory does not exist: " << theDir << std::endl;
		return;
	}

	for (const auto& entry : fs::directory_iterator(theDir)) {
		const fs::path& path = entry.path();
		const fs::path relPath = theRelDir / path.filename();

		if (entry.is_regular_file()) {
			std::ifstream srcFile(path, std::ios::binary | std::ios::ate);
			if (!srcFile.is_open()) {
				std::cerr << "Unable to open source file '" << path << "'" << std::endl;
				exit(4);
			}

			std::streamsize fileSize = srcFile.tellg();
			srcFile.seekg(0, std::ios::beg);

			uchar aFlags = 0;
			enc_fwrite(&aFlags, 1, 1, gDestFP);

			std::string relStr = relPath.string();
			uchar aNameWidth = static_cast<uchar>(relStr.length());
			enc_fwrite(&aNameWidth, 1, 1, gDestFP);
			enc_fwrite(relStr.c_str(), 1, aNameWidth, gDestFP);

			int size32 = static_cast<int>(fileSize);
			enc_fwrite(&size32, sizeof(int), 1, gDestFP);


			auto ftime = std::chrono::duration_cast<std::chrono::seconds>(fs::last_write_time(path).time_since_epoch()).count();
			int64_t fileTime = static_cast<int64_t>(ftime);
			enc_fwrite(&fileTime, sizeof(fileTime), 1, gDestFP);

			DeferredFile aDeferredFile;
			aDeferredFile.mFileName = path.string();
			aDeferredFile.mFileSize = static_cast<size_t>(fileSize);
			gDeferredFilesList.push_back(aDeferredFile);
		}
		else if (entry.is_directory()) {
			AddFiles(path.string(), relPath.string());
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		std::cerr << "Usage: PopPak [/U] [/P] [/K \"The Password in Quotes\"] <FileName> <DirPath>" << std::endl;
		std::cerr << "  /U    Unpacks pak file to DirPath" << std::endl;
		std::cerr << "  /P    Creates pak file from files in DirPath" << std::endl;
		std::cerr << "  /K    Changes the Default Encryption Password" << std::endl;
		return 1;
	}

	int anArgPos = 1;

	while (argc >= anArgPos + 1)
	{
		if (argv[anArgPos][0] == '/')
		{
			if (stricmp(argv[anArgPos], "/U") == 0)
				unpackage = true;
			else if (stricmp(argv[anArgPos], "/P") == 0)
				package = true;
			else if (stricmp(argv[anArgPos], "/K") == 0)
			{
				anArgPos++;
				gEncryptPassword = argv[anArgPos];

				if (gEncryptPassword == "")
				{
					std::cerr << "Invalid Password:" << argv[anArgPos] << std::endl;
					return 106;
				}
			}
			else
			{
				std::cerr << "Invalid option" << std::endl;
				return 101;
			}

			anArgPos++;
		}
		else
			break;
	}

	////////////////////////////////////////////////////////////////////////////
	// PopCapPWE should be an internal tool.  In any event, the default Password
	// is actually really easy to guess.
	std::cout << StrFormat("Password: %s", gEncryptPassword.c_str()).c_str() << std::endl;
	if (gEncryptPassword == "PopCapPopWorkFramework")
		std::cerr << StrFormat("WARNING: %s is the Default Password and is NOT allowed for Distribution!",
							   gEncryptPassword.c_str())
						 .c_str()
				  << std::endl;

	if (argc < anArgPos + 2)
	{
		std::cerr << "Usage: PopPak [/U] [/P] [/K \"The Password in Quotes\"] <FileName> <DirPath>" << std::endl;
		std::cerr << "  /U    Unpacks pak file to DirPath" << std::endl;
		std::cerr << "  /P    Creates pak file from files in DirPath" << std::endl;
		std::cerr << "  /K    Changes the Default Encryption Password" << std::endl;
		return 102;
	}

	if (package && unpackage)
	{
		std::cerr << "/U and /P may not be specified together." << std::endl;
		std::cerr << "Usage: PopPak [/U] [/P] [/K \"The Password in Quotes\"] <FileName> <DirPath>" << std::endl;
		std::cerr << "  /U    Unpacks pak file to DirPath" << std::endl;
		std::cerr << "  /P    Creates pak file from files in DirPath" << std::endl;
		std::cerr << "  /K    Changes the Default Encryption Password" << std::endl;
		return 103;
	}

	std::string aPackName = argv[anArgPos++];
	std::string aDirName = argv[anArgPos++];

	if ((aDirName[aDirName.length() - 1] != '\\') && (aDirName[aDirName.length() - 1] != '/'))
		aDirName += "\\";

	if (package)
	{
		gDestFP = fopen(aPackName.c_str(), "wb");
		if (gDestFP == NULL)
		{
			std::cerr << "Unable to create '" << aPackName.c_str() << "'" << std::endl;
			return 2;
		}

		ulong anId = 0xBAC04AC0;
		enc_fwrite(&anId, sizeof(ulong), 1, gDestFP);

		ulong aVersion = 0;
		enc_fwrite(&aVersion, sizeof(ulong), 1, gDestFP);

		AddFiles(aDirName, "");

		uchar aFileFlags = FILEFLAGS_END;
		enc_fwrite(&aFileFlags, 1, 1, gDestFP);

		if (gDeferredFilesList.size() == 0)
			std::cout << "Warning: no files!" << std::endl;

		int aCount = 0;

		DeferredFileList::iterator anItr = gDeferredFilesList.begin();
		while (anItr != gDeferredFilesList.end())
		{
			std::cout << "Writing: " << ((aCount * 100) / gDeferredFilesList.size()) << "%\r";
			std::cout.flush();

			DeferredFile *aDeferredFile = &(*anItr);

			FILE *aSrcFP = fopen(aDeferredFile->mFileName.c_str(), "rb");
			if (aSrcFP == NULL)
			{
				std::cerr << "Unable to open source file '" << aDeferredFile->mFileName.c_str() << "'" << std::endl;
				exit(4);
			}

			int aTotalSize = 0;
			while (!feof(aSrcFP))
			{
				uchar aBuffer[4096];
				int aSize = (int)fread(aBuffer, 1, 4096, aSrcFP);

				enc_fwrite(aBuffer, 1, aSize, gDestFP);
				aTotalSize += aSize;
			}

			fclose(aSrcFP);

			if (aTotalSize != aDeferredFile->mFileSize)
			{
				std::cerr << "Error: File size changed!" << std::endl;
				return 3;
			}

			++anItr;
			aCount++;
		}

		fclose(gDestFP);
	}

	
	if (unpackage)
	{
		PakInterface anInterface;
		anInterface.mDecryptPassword = gEncryptPassword;
		if (!anInterface.AddPakFile(aPackName))
		{
			std::cerr << "Pak File: " << aPackName << " was unable to be loaded. Was this created with a earlier version of PopPack?"
					  << std::endl;
			return 104;
		}

		fs::path outDir = aDirName;
		if (fs::exists(outDir)) {
			std::cout << "Warning: Directory " << aDirName << " already exists. Data will be overwritten.\n";
		}

		MkDir(aDirName);

		int aNumFiles = (int)anInterface.mPakRecordMap.size();
		int aDoneFiles = 0;
		for (PakRecordMap::iterator anItr = anInterface.mPakRecordMap.begin(); anItr != anInterface.mPakRecordMap.end(); ++anItr)
		{
			PakRecord &aRecord = anItr->second;

			if (anItr->first == StringToUpper(GetFileName(aPackName)))
				continue;

			std::string aFileName = aDirName + aRecord.mFileName;
			MkDir(GetFileDir(aFileName));
			FILE *fp = fopen(aFileName.c_str(), "wb");
			if (!fp)
			{
				std::cerr << "Error: Unable to open " << aFileName << " for writing: " << errno << std::endl;
				return errno;
			}

			PFILE aFile;
			aFile.mFP = NULL;
			aFile.mPos = 0;
			aFile.mRecord = &aRecord;

			std::byte aByte;
			while (aFile.mPos < aRecord.mSize && anInterface.FRead(&aByte, 1, 1, &aFile) > 0)
			{
				if (fwrite(&aByte, 1, 1, fp) != 1)
				{
					int err_save = errno; // in case fclose destroys errno
					std::cerr << "Error: Writing data to " << aFileName << ": " << err_save << std::endl;
					fclose(fp);
					return err_save;
				}
			}
			if (aFile.mPos < aRecord.mSize)
			{
				std::cerr << "Error: Reading data from PAK file for " << aFileName << std::endl;
				fclose(fp);
				return 105;
			}

			fclose(fp);

			std::cout << aFileName;

			aDoneFiles++;
			std::cout << "Writing: " << ((aDoneFiles * 100) / aNumFiles) << "%\r";
			std::cout.flush();
		}

		std::cout << "Writing: 100%\r";
		std::cout.flush();
	}

	return 0;
}
