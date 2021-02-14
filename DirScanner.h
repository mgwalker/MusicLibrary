#ifndef __DIRSCANNER_H
#define __DIRSCANNER_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <cmath>
using namespace std;

#define uint  unsigned int
#define uchar unsigned char

// The code was originally written to the Win32 API,
// so the function calls are native to the OS when
// compiling on Windows.  Just #include and #define
// and everything works great!
#ifdef WIN32
	#include <windows.h>
	#define findData   WIN32_FIND_DATA
	#define fileHandle HANDLE
	#define SLASH	'\\'
	#define SLASHS	"\\"
	#define STAR	'*'
// Not quite so on Linux, though.  The Win32 API
// is very partially reimplemented for Linux here
// so everything works.  It's probably not as clean
// as it should be, but it gets the job done!
#else
	#include <sys/types.h>
	#include <dirent.h>
	#include <sys/stat.h>

	#define SLASH	'/'
	#define SLASHS	"/"
	#define STAR	0
	#define fileHandle DIR*
	#define INVALID_HANDLE_VALUE NULL
	#define FILE_ATTRIBUTE_DIRECTORY 1

	struct findData
	{
		uchar dwFileAttributes;
		char* cFileName;
	};

	fileHandle FindFirstFile(const char* dir, findData* x)
	{
		DIR* ret = opendir(dir);

		if(ret)
		{
			struct dirent *d = readdir(ret);
			if(d)
			{
				x->cFileName = d->d_name;
				x->dwFileAttributes = 0;

				string fn = dir;
				fn.append(x->cFileName);

				struct stat sb;
				stat(fn.c_str(), &sb);
				if((sb.st_mode & S_IFMT) == S_IFDIR)
					x->dwFileAttributes = 1;
			}
			else
				return NULL;
		}
		return ret;
	}

	bool FindNextFile(fileHandle f, findData* x, string dir)
	{
		struct dirent *d = readdir(f);
		if(d)
		{
			x->cFileName = d->d_name;
			x->dwFileAttributes = 0;

			string fn = dir;
			fn.append(x->cFileName);

			struct stat sb;
			stat(fn.c_str(), &sb);
			if((sb.st_mode & S_IFMT) == S_IFDIR)
				x->dwFileAttributes = 1;
		}
		else	
			return false;

		return true;
	}
#endif

ostream& operator << (ostream& out, string s);
bool getTime(char* header, int fsize, int &min, int &sec);

class Track
{
	public:
		Track(){}
		Track(string _title){ this->title = _title; }
		string getTitle(){ return this->title; }
		string getFile(){ return this->filename; }
		int getMin(){ return this->min; }
		int getSec(){ return this->sec; }
		void setTitle(string _title){ this->title = _title; }
		void setFile(string _filename){ this->filename = _filename; }
		void setTime(int _min, int _sec){ this->min = _min; this->sec = _sec; }
	private:
		int min;
		int sec;
		string title;
		string filename;
};

class Album
{
	public:
		Album(){ num = 0; }
		Album(string _name){ num = 0; this->name = _name; }
		string getName(){ return this->name; }
		void setName(string _name){ this->name = _name; }
		Track* addTrack(string _ttitle);
		int num;
		Track* tracks;
	private:
		string name;
};

class Artist
{
	public:
		Artist(){ num = 0; }
		Artist(string _name){ num = 0; this->name = _name; }
		string getName(){ return this->name; }
		void setName(string _name) { this->name = _name; }
		Album* addAlbum(string _aname);
		void sortAlbums();
		int num;
		Album* albums;
	private:
		string name;
};

class DirScanner
{
	public:
		DirScanner();
		void go(string startDir);
		void exportList(string outfile);
		void exportStats(void);
		static char* strtoupper(char* str);
		static int strlen(char * str);
		static char* trim(char* str, int len);
		void printTime(unsigned int sec);

		void noRecurse()                { this->recurse = false; }
		void HTML()                     { this->html = true; }
		void useTemplate()              { this->usetmplt = true; }
		void templateHTML(string _tmplt){ this->tmplt = _tmplt; }
		void noArtists()                { this->showArt = false; }
		void noAlbums()                 { this->showAlb = false; }
		void noSongs()                  { this->showSongs = false; }
		void yesFile()                  { this->showFile = true; }
		void yesTime()                  { this->showTime = true; }
		void yesXML()                   { this->xml = true; }
		void justStats()                { this->justGatherStats = true; }
		bool justGatherStats;

	private:
		void sortArtists();
		Artist* AddArtist(string _aname);
		bool recurse;
		bool html;
		bool usetmplt;
		bool showArt;
		bool showAlb;
		bool showSongs;
		bool showFile;
		bool showTime;
		bool xml;
		string tmplt;
		Artist* artists;
		int num;
		void parseTemplate(char* header, char* footer,
			               char* bartist, char* aartist,
			               char* balbum, char* aalbum,
						   char* bsong, char* asong,
						   char* bfile, char* afile);
};

#endif
