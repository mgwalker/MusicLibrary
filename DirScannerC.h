//#include <iostream>
//#include <windows.h>
//#include <string.h>
//#include <fstream>
#include "DirScanner.h"
using namespace std;

Track* Album::addTrack(string _ttitle)
{
	for(int i = 0; i < this->num; i++)
		if(tracks[i].getTitle().compare(_ttitle) == 0)
			return &tracks[i];
			//doInsert = false;

	Track* temp = tracks;
	tracks = new Track[++(this->num)];
	for(int i = 0; i < (this->num - 1); i++)
		tracks[i] = temp[i];
	tracks[(this->num) - 1].setTitle(_ttitle);
	return &tracks[(this->num) - 1];
}

Album* Artist::addAlbum(string _aname)
{
	for(int i = 0; i < this->num; i++)
		if(albums[i].getName().compare(_aname) == 0)
			return &albums[i];
			//doInsert = false;

	Album* temp = albums;
	albums = new Album[++(this->num)];
	for(int i = 0; i < (this->num - 1); i++)
		albums[i] = temp[i];
	albums[(this->num) - 1].setName(_aname);
	return &albums[(this->num) - 1];
}

void Artist::sortAlbums()
{
	Album temp;
	for(int j = 0; j < this->num; j++)
	{
		for(int i = 0; i < (this->num) - 1; i++)
		{
			if(albums[i].getName().compare(albums[i + 1].getName()) > 0)
			{
				temp = albums[i];
				albums[i] = albums[i + 1];
				albums[i + 1] = temp;
			}
		}
	}
}

DirScanner::DirScanner()
{
	this->num = 0;
	this->recurse = true;
	this->html = false;
	this->usetmplt = false;
	this->showArt = true;
	this->showAlb = true;
	this->showSongs = true;
	this->showFile = false;
	this->showTime = false;
	this->xml = false;
	this->justGatherStats = false;
}

// Primary directory scanner
void DirScanner::go(string startDir)
{
	string file;		// File being examined
	ifstream infile;	// Input file stream
	findData x;			// WIN32 API data structure
	fileHandle hnd;		// WIN32 file handle
	bool go = true;		// True as long as more files are present in the directory

	// Find the first file in the starting directory.  The returned file
	// handle is used to find subsequent files
	hnd = FindFirstFile(startDir.c_str(), &x);

	// While the file handle is valid and there are files in the directory...
	while(hnd != INVALID_HANDLE_VALUE && go)
	{
		// If the file is a directory and is not "." or ".."...
		if(x.cFileName[0] != '.' && (x.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// Check to see if recursion has been enabled (default)
			if(this->recurse)
			{
				// Figure out the new starting directory.  For Windows, strip off
				// the trailing slash.  For Linux, leave it.  Weird, isn't it?
				#ifdef WIN32
					string newStart = startDir.substr(0, startDir.length() - 1);
				#else
					string newStart = startDir;
				#endif
				newStart.append(x.cFileName);
				char app[3];
				app[0] = SLASH;
				app[1] = STAR;
				app[2] = 0;
				newStart.append(app);

				// Start scanning this subdirectory
				this->go(newStart);
			}
		}
		// If this file is not a directory and is not "." or ".."...
		else if(x.cFileName[0] != '.')
		{
			// Figure out the filename's extension (last three characters)
			int slen = strlen(x.cFileName);
			char ext[4];
			ext[0] = x.cFileName[slen-3];
			ext[1] = x.cFileName[slen-2];
			ext[2] = x.cFileName[slen-1];
			ext[3] = 0;

			// Capitalize the extension so that all variants of MP3 (such as
			// mp3, mP3) can be detected
			strtoupper(ext);

			// Check to see if the file's extension is "MP3"
			if(ext[0] == 'M' && ext[1] == 'P' && ext[2] == '3')
			{
				// Figure out the full path and filename for this file
				#ifdef WIN32
					file = startDir.substr(0, startDir.length()-1);
				#else
					file = startDir;
				#endif
				file.append(x.cFileName);
				char in[4];
				in[0] = 0;
				in[1] = 0;
				in[2] = 0;
				in[3] = 0;

				Artist* partist;
				Album* palbum;
				Track* ptrack;
				string unk = "Unknown";

				// Open the file as a binary and read in three bytes
				infile.open(file.c_str(), ios_base::binary);
				infile.read(in, 3);

				// If the first three bytes are "ID3", then this is an
				// ID3v2 tag.  Parse it as such.
				if(in[0] == 'I' && in[1] == 'D' && in[2] == '3')
				{
					// Skip the next three bytes
					infile.seekg(3, ios::cur);

					// Read the next four bytes.  These four bytes tell the
					// size of the ID3 header (in bytes)
					char sze[4];
					infile.read(sze, 4);

					// Calculate the header size
					int tsize = ((sze[0] & 0x7F) << 21) + ((sze[1] & 0x7F) << 14) + ((sze[2] & 0x7F) << 7) + (sze[3] & 0x7F);
					char* tag = new char[tsize];	// Header

					// Artist, album, and title names
					char *artst, *album, *title;

					// Make the first character null for all.
					artst = new char[1];
					artst[0] = 0;
					album = new char[1];
					album[0] = 0;
					title = new char[1];
					title[0] = 0;
                    
					// Read the ID3v2 tag into the tag variable (faster than seeking back and forth
					// through the file)
                    infile.read(tag, tsize);

					// Get the filesize
					char header[4];
					infile.seekg(tsize, ios_base::beg);
					infile.read(header, 4);
					bool sync = ((((((unsigned char)header[0]) << 3) + (((unsigned char)header[1]) >> 5)) | 0xF800) == 0xFFFF) ? true : false;
					while(!sync && !infile.eof())
					{
						infile.seekg(-3, ios_base::cur);
						infile.read(header, 4);
						sync = ((((((unsigned char)header[0]) << 3) + (((unsigned char)header[1]) >> 5)) | 0xF800) == 0xFFFF) ? true : false;
						int bitrate = (unsigned char)header[2] >> 4;
						unsigned short int layer = (((unsigned char)header[1]) >> 1) & 0x03;
						if(bitrate == 15 || layer != 1)
							sync = false;
					}
					int min = 0;
					int sec = 0;
					infile.seekg(0, ios_base::end);
					bool gotTime = getTime(header, infile.tellg(), min, sec);

					int fsize = 0;	// Size of each tag

					// Loop through the whole tag
					for(int i = 0; i < tsize; i++)
					{
						// If "TP1" is found, this is the artist tag.  Extract the artist name.
						if(tag[i] == 'T' && tag[i + 1] == 'P' && tag[i + 2] == 'E' && tag[i + 3] == '1')
						{
							// Calculate the size of the artist tag
							fsize = (tag[i + 4] * 16777216) + (tag[i + 5] * 65536) + (tag[i + 6] * 256) + tag[i + 7] - 1;
							artst = new char[fsize + 1];
							artst[fsize] = 0;

							// Extract the artist name
							for(int j = 0; j < fsize; j++)
								artst[j] =  tag[i + 11 + j];
						}
					}
					// If the first character of the artist name is printable, print the
					// whole artist name to the screen.  Otherwise, print "Misc" as the
					// artist name.
					if(artst[0] > 32)
						partist = this->AddArtist(trim(artst, fsize));
						//cout << this->trim(artst, fsize) << " >> ";
					else
						partist = this->AddArtist(unk);
						//cout << "Misc >> ";

					fsize = 0;	// Reset to zero
					// Loop through the header again
					for(int i = 0; i < tsize; i++)
					{
						// If "TALB" is found, this is the album name.  Extract.
						if(tag[i] == 'T' && tag[i + 1] == 'A' && tag[i + 2] == 'L' && tag[i + 3] == 'B')
						{
							// Calculate the size of the album tag.
							fsize = (tag[i + 4] * 16777216) + (tag[i + 5] * 65536) + (tag[i + 6] * 256) + tag[i + 7] - 1;
							album = new char[fsize + 1];
							album[fsize] = 0;

							// Extract the album name.
							for(int j = 0; j < fsize; j++)
								album[j] = tag[i + 11 + j];
						}
					}
					// If the first character of the album name is printable, print the
					// whole album name to the screen.  Otherwise, print "Unknown" as the
					// album name.
					if(album[0] > 32)
						palbum = partist->addAlbum(trim(album, fsize));
						//cout << this->trim(album, fsize) << " >> ";
					else
						palbum = partist->addAlbum(unk);
						//cout << "Unknown >> ";

					fsize = 0;	// Reset to zero.
					// Loop through the header again
					for(int i = 0; i < tsize; i++)
					{
						// If "TIT2" is found, this is the track title.  Extract.
						if(tag[i] == 'T' && tag[i + 1] == 'I' && tag[i + 2] == 'T' && tag[i + 3] == '2')
						{
							// Calculate the size of the track title tag.
							fsize = (tag[i + 4] * 16777216) + (tag[i + 5] * 65536) + (tag[i + 6] * 256) + tag[i + 7] - 1;
							title = new char[fsize + 1];
							title[fsize] = 0;

							// Extract the track title
							for(int j = 0; j < fsize; j++)
								title[j] = tag[i + 11 + j];
						}
					}
					// If the first character of the track title is printable, print the
					// whole track title to the screen.  Otherwise, print "Unknown" as the
					// track title.
					if(title[0] > 32)
						ptrack = palbum->addTrack(trim(title, fsize));
						//cout << this->trim(title, fsize) << endl;
					else
						ptrack = palbum->addTrack(unk);
						//cout << "Unknown" << endl;
					palbum->tracks[palbum->num - 1].setFile(file);
					if(sync && gotTime)
						palbum->tracks[palbum->num - 1].setTime(min, sec);
					/*
					{
						cout << min << ":";
						if(sec < 10)
							cout << '0';
						cout << sec;
					}
					*/

				}
				else	// Not an ID3v2 tag
				{
					in[0] = 0;
					in[1] = 0;
					in[2] = 0;

					// Seek to the end of the file, then back 128 bytes
					int pos = infile.tellg();
					infile.seekg(-128, ios::end);

					// Read three characters
					infile.read(in, 3);

					// If the three characters are "TAG", then this is an
					// ID3v1 tag, so parse it as such.
					if(in[0] == 'T' && in[1] == 'A' && in[2] == 'G')
					{
						// Track, album, and artist names
						char title[30];
						char album[30];
						char artst[30];
						
						// Read the track title from the file
						infile.read(title, 30);
						// Read the artist name from the file
						infile.read(artst, 30);
						// Read the album name from the file
						infile.read(album, 30);

						// Get the filesize
						char header[4];
						infile.seekg(0, ios_base::beg);
						infile.read(header, 4);
						bool sync = ((((((unsigned char)header[0]) << 3) + (((unsigned char)header[1]) >> 5)) | 0xF800) == 0xFFFF) ? true : false;
						while(!sync && !infile.eof())
						{
							infile.seekg(-3, ios_base::cur);
							infile.read(header, 4);
							sync = ((((((unsigned char)header[0]) << 3) + (((unsigned char)header[1]) >> 5)) | 0xF800) == 0xFFFF) ? true : false;
							int bitrate = (unsigned char)header[2] >> 4;
							unsigned short int layer = (((unsigned char)header[1]) >> 1) & 0x03;
							if(bitrate == 15 || layer != 1)
								sync = false;
						}
						int min = 0;
						int sec = 0;
						infile.seekg(0, ios_base::end);
						bool gotTime = getTime(header, infile.tellg(), min, sec);

						// If the artist name is found, print it
						// out.  Otherwise, print "Misc"
						if(this->strlen(this->trim(artst, 30)) > 0)
							partist = this->AddArtist(trim(artst, 30));
							//cout << this->trim(artst, 30) << " >> ";
						else
							partist = this->AddArtist(unk);
							//cout << "Misc >> ";

						// If the album name is found, print it
						// out.  Otherwise, print "Unknown"
						if(strlen(trim(album, 30)) > 0)
							palbum = partist->addAlbum(trim(album, 30));
							//cout << trim(album, 30) << " >> ";
						else
							palbum = partist->addAlbum(unk);
							//cout << "Unknown >> ";

						// If the track title is found, print it
						// out.  Otherwise, print "Unknown"
						if(this->strlen(this->trim(title, 30)) > 0)
							ptrack = palbum->addTrack(trim(title, 30));
							//cout << this->trim(title, 30) << endl;
						else
							ptrack = palbum->addTrack(unk);
							//cout << "Unknown" << endl;
						ptrack->setFile(file);
						if(sync && gotTime)
							ptrack->setTime(min, sec);

					}
					else
					{
						// No ID3 tag at all
						// Get the filesize
						char header[4];
						infile.seekg(0, ios_base::beg);
						infile.read(header, 4);
						bool sync = ((((((unsigned char)header[0]) << 3) + (((unsigned char)header[1]) >> 5)) | 0xF800) == 0xFFFF) ? true : false;
						while(!sync && !infile.eof())
						{
							infile.seekg(-3, ios_base::cur);
							infile.read(header, 4);
							sync = ((((((unsigned char)header[0]) << 3) + (((unsigned char)header[1]) >> 5)) | 0xF800) == 0xFFFF) ? true : false;
							int bitrate = (unsigned char)header[2] >> 4;
							unsigned short int layer = (((unsigned char)header[1]) >> 1) & 0x03;
							if(bitrate == 15 || layer != 1)
								sync = false;
						}
						int min = 0;
						int sec = 0;
						infile.seekg(0, ios_base::end);
						bool gotTime = getTime(header, infile.tellg(), min, sec);

                        partist = this->AddArtist(unk);
						palbum = partist->addAlbum(unk);
						ptrack = palbum->addTrack(x.cFileName);
						ptrack->setFile(file);
						if(sync && gotTime)
							ptrack->setTime(min, sec);
					}
				}
				// Close the file
				infile.close();
			}
		}
		// Get the next file.  If there are no more files, go will be false.
		// Unfortunately, this has to be handled different for Windows and Linux
		// because the Windows version will tell whether or not the file is
		// a directory in a single call, but the Linux variant requires a second
		// call and the complete file path, so the directory has to be passed, too
		#ifdef WIN32
			go = FindNextFile(hnd, &x);
		#else
			go = FindNextFile(hnd, &x, startDir);
		#endif
	}
	//cout << "=== LEAVING DIRECTORY ===" << endl;
}

Artist* DirScanner::AddArtist(string _aname)
{
	for(int i = 0; i < (this->num); i++)
		if(artists[i].getName().compare(_aname) == 0)
			return &artists[i];

	Artist* temp;
	temp = new Artist[this->num];
	temp = artists;
	artists = new Artist[++(this->num)];
	for(int i = 0; i < (this->num - 1); i++)
		artists[i] = temp[i];
	artists[(this->num) - 1].setName(_aname);
	return &artists[(this->num) - 1];
}

void DirScanner::printTime(unsigned int sec)
{
	if(sec >= 60)
	{
		uint min = sec / 60;
		sec -= (min * 60);
		
		if(min >= 60)
		{
			uint hour = min / 60;
			min -= (hour * 60);

			if(hour >= 24)
			{
				uint day = hour / 24;
				hour -= (day * 24);
				cout << day << " days, " << hour << " hours, " << min << " minutes, and " << sec << " seconds";
			}
			else
				cout << hour << " hours, " << min << " minutes, and " << sec << " seconds";
		}
		else
			cout << min << " minutes and " << sec << " seconds";
	}
	else
		cout << sec << " seconds";
}

void DirScanner::exportStats()
{
	uint alb = 0, son = 0, sec = 0;
	for(uint i = 0; i < this->num; i++)
	{
		alb += this->artists[i].num;
		for(uint j = 0; j < this->artists[i].num; j++)
		{
			son += this->artists[i].albums[j].num;
			for(uint k = 0; k < this->artists[i].albums[j].num; k++)
				sec += this->artists[i].albums[j].tracks[k].getSec() + (60 * this->artists[i].albums[j].tracks[k].getMin());
		}
	}

	cout << "\nStatistics\n====================" << endl;
	cout << "Artists: " << this->num << endl;
	cout << "Albums:  " << alb << endl;
	cout << "Songs:   " << son << endl;
	cout << "====================" << endl;
	cout << "Total play length:   ";
	this->printTime(sec);
	cout << endl;
	cout << "Average play length: ";
	this->printTime(sec/son);
	cout << endl << endl;
}

void DirScanner::exportList(string outfile)
{
	ofstream out;
	out.open(outfile.c_str(), ios_base::out);
	this->sortArtists();

	if(this->html)
	{
		if(this->usetmplt)
		{
			char header[] = "list";
			char footer[] = "";
			char bartist[] = "artist";
			char aartist[] = "";
			char balbum[] = "album";
			char aalbum[] = "";
			char bsong[] = "song";
			char asong[] = "";
			char bfile[] = "file";
			char afile[] = "";

			this->parseTemplate(header, footer, bartist, aartist, balbum,
				                aalbum, bsong, asong, bfile, afile);

			ifstream tmplt;
			tmplt.open(this->tmplt.c_str(), ios_base::binary);

			for(int i = 0; i < this->num; i++)
			{
				if(this->showArt)
					out << bartist << artists[i].getName() << aartist;
				artists[i].sortAlbums();
				for(int j = 0; j < artists[i].num; j++)
				{
					if(this->showAlb)
						out << balbum << artists[i].albums[j].getName() << aalbum;
					for(int k = 0; k < artists[i].albums[j].num; k++)
					{
						if(this->showSongs)
							out << bsong << artists[i].albums[j].tracks[k].getTitle() << asong;
						if(this->showFile)
							out << bfile << artists[i].albums[j].tracks[k].getFile() << afile;
					}
				}
			}

			out << footer;

			tmplt.close();
		}
		else
		{
			out << "<HTML>\n<HEAD>\n  <TITLE>Music Collection</TITLE>\n  <STYLE TYPE=\"text/css\">\n";
			out << "  <!--\n    body { font-family: arial; background-color: black; }\n\n";
			out << "    td.artistName { background-color: black; color: white; font-weight: bold; ";
			out << "font-size: 24pt; padding: 5px; }\n";
			out << "    td.albumName  { background-color: #404040; color: white; font-weight: bold; ";
			out << "font-size: 16pt; padding-left: 30px; }\n";
			out << "    td.songList   { background-color: #E0E0E0; color: black; padding-left: 60px; }\n";
			out << "    td.fileList   { background-color: #E0E0E0; color: black; padding-left: 80px; ";
			out << "font-size: 8pt; padding-bottom: 15px; }\n";
			out << "    td.lengthList { background-color: #E0E0E0; color: black; padding-left: 80px; ";
			out << "font-size: 8pt; padding-bottom: 10px; }\n";
			out << "  -->\n  </STYLE>\n</HEAD>\n<BODY>\n<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=0 ";
			out << "ALIGN=\"CENTER\">\n";

			for(int i = 0; i < this->num; i++)
			{
				if(this->showArt)
					out << "  <TR>\n    <TD CLASS=\"artistName\">" << artists[i].getName()
						<< "</TD>\n  </TR>\n";
				artists[i].sortAlbums();
				for(int j = 0; j < artists[i].num; j++)
				{
					if(this->showAlb)
						out << "  <TR>\n    <TD CLASS=\"albumName\">" << artists[i].albums[j].getName()
						    << "</TD>\n  </TR>\n";
					for(int k = 0; k < artists[i].albums[j].num; k++)
					{
						if(this->showSongs)
							out << "  <TR>\n    <TD CLASS=\"songList\">"
								<< artists[i].albums[j].tracks[k].getTitle()
								<< "</TD>\n  </TR>\n";
						if(this->showFile)
						{
							out << "  <TR>\n    <TD CLASS=\"fileList\">"
								<< artists[i].albums[j].tracks[k].getFile();
                            if(this->showTime)
							{
								int min = artists[i].albums[j].tracks[k].getMin();
								int sec = artists[i].albums[j].tracks[k].getSec();
								if(min > 0 || sec > 0)
								{
									out << " - " << min << ":";
									if(sec < 10)
										out << '0';
									out << sec;
								}								
							}
								 
							out	<< "</TD>\n  </TR>\n";
						}
						else if(this->showTime)
						{
							int min = artists[i].albums[j].tracks[k].getMin();
							int sec = artists[i].albums[j].tracks[k].getSec();
							if(min > 0 || sec > 0)
							{
								out << "  <TR>\n    <TD CLASS=\"lengthList\">" << min << ":";
								if(sec < 10)
									out << '0';
								out << sec << "</TD>\n  </TR>\n";
							}
						}
					}						
				}
			}

			out << "</TABLE>\n</BODY>\n<!-- Generated by the MP3 Library List Generator -->\n</HTML>";
		}
	}
	else
	{
		if(this->xml)
			out << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<mp3liblg>\n";
		for(int i = 0; i < this->num; i++)
		{
			if(this->showArt)
			{
				if(this->xml)
				{
					string str = artists[i].getName();
					out << "  <artist name=\"";
					//<< artists[i].getName() << "\">\n";
					for(unsigned int j = 0; j < str.length(); j++)
					{
						if(str.c_str()[j] == '&')
							out << "&amp;";
						else if(str.c_str()[j] == '"')
							out << "&quot;";
						else
							out << str.c_str()[j];
					}
					out << "\">\n";
				}
				else
					out << artists[i].getName() << endl;
			}
			artists[i].sortAlbums();
			for(int j = 0; j < artists[i].num; j++)
			{
				if(this->showAlb)
				{
					if(this->xml)
					{
						string str = artists[i].albums[j].getName();
						out << "    <album name=\"";
						//<< artists[i].albums[j].getName() << "\">\n";
						for(unsigned int k = 0; k < str.length(); k++)
						{
							if(str.c_str()[k] == '&')
								out << "&amp;";
							else if(str.c_str()[k] == '"')
								out << "&quot;";
							else
								out << str.c_str()[k];
						}
						out << "\">\n";
					}
					else
						out << "  > " << artists[i].albums[j].getName() << endl;
				}
				if(this->showSongs)
				{
					for(int k = 0; k < artists[i].albums[j].num; k++)
					{
						string str = artists[i].albums[j].tracks[k].getTitle();
						if(this->xml && (this->showFile || this->showTime))
						{
							out << "      <song name=\"";
							//<< artists[i].albums[j].tracks[k].getTitle() << "\"";
							for(unsigned int l = 0; l < str.length(); l++)
							{
								if(str.c_str()[l] == '&')
									out << "&amp;";
								else if(str.c_str()[l] == '"')
									out << "&quot;";
								else
								out << str.c_str()[l];
							}
							out << "\"";
						}
						else if(this->xml)
						{
							out << "      <song>";
							//<< artists[i].albums[j].tracks[k].getTitle() << "</song>";
							for(unsigned int l = 0; l < str.length(); l++)
							{
								if(str.c_str()[l] == '&')
									out << "&amp;";
								else if(str.c_str()[l] == '"')
									out << "&quot;";
								else
								out << str.c_str()[l];
							}
							out << "</song>";
						}
						else
							out << "    | " << artists[i].albums[j].tracks[k].getTitle();
						if(this->showFile)
						{
							if(this->xml)
							{
								out << " file=\"";
								str = artists[i].albums[j].tracks[k].getFile();
								for(unsigned int l = 0; l < str.length(); l++)
								{
									if(str.c_str()[l] == '&')
										out << "&amp;";
									else if(str.c_str()[l] == '"')
										out << "&quot;";
									else
										out << str.c_str()[l];
								}
								out << "\"";
							}
							else
								out << " (" << artists[i].albums[j].tracks[k].getFile() << ")";
						}
						if(this->showTime)
						{
							int min = artists[i].albums[j].tracks[k].getMin();
							int sec = artists[i].albums[j].tracks[k].getSec();
							if(min > 0 || sec > 0)
							{
								if(this->xml)
									out << " time=\"" << min << ":";
								else
									out << " (" << min << ":";
								if(sec < 10)
									out << '0';
								if(this->xml)
									out << sec << "\"";
								else
									out << sec << ')';
							}
						}
						if(this->xml && (this->showFile || this->showTime))
							out << " />";
						out << endl;
					}
				}
				if(this->showAlb && this->xml)
					out << "    </album>\n";
			}
			if(this->showArt && this->xml)
				out << "  </artist>\n";
		}
		if(this->xml)
			out << "</mp3liblg>\n";
	}
	out.close();
}

void DirScanner::parseTemplate(char* header, char* footer, char* bartist, char* aartist,
							   char* balbum, char* aalbum, char* bsong, char* asong,
							   char* bfile, char* afile)
{
	ifstream tmplt(this->tmplt.c_str(), ios_base::binary);
	int slen = this->strlen(header) + 3;
	int len = 0;
	char* seek = new char[slen];
	char tmp = 0;
	char* body;

	seek[slen - 1] = 0;
	seek[0] = '<';
	seek[slen - 2] = '>';
	for(int i = 0; i < this->strlen(header); i++)
		seek[i + 1] = header[i];

	bool loop = true;
	
	while(loop && !tmplt.eof())
	{
		loop = false;
		tmplt.read(&tmp, 1);
		for(int i = 0; i < strlen(seek); i++)
		{
            if((tmp | 0x20) != (seek[i] | 0x20))
			{
				loop = true;
				break;
			}
			tmplt.read(&tmp, 1);
		}
	}
	len = (int)tmplt.tellg() - strlen(seek) - 1;

	seek = new char[++slen];
	seek[slen - 1] = 0;
	seek[0] = '<';
	seek[1] = '/';
	seek[slen - 2] = '>';
	for(int i = 0; i < this->strlen(header); i++)
		seek[i + 2] = header[i];

	header = new char[len + 1];
	header[len] = 0;
	tmplt.seekg(0, ios_base::beg);
	tmplt.read(header, len);
	for(int i = 0; i < len; i++)
		if(header[i] == 13)
			for(int j = i; j < len; j++)
				header[j] = header[j + 1];

	loop = true;
	while(loop && !tmplt.eof())
	{
		loop = false;
		tmplt.read(&tmp, 1);
		for(int i = 0; i < strlen(seek); i++)
		{
            if((tmp | 0x20) != (seek[i] | 0x20))
			{
				loop = true;
				break;
			}
			tmplt.read(&tmp, 1);
		}
	}
	int blen = (int)tmplt.tellg() - len - (2*this->strlen(seek)) - 1;
	body = new char[blen + 1];
	body[blen] = 0;
	tmplt.seekg(-(blen + this->strlen(seek) + 1), ios_base::cur);
	tmplt.read(body, blen);
	for(int i = 0; i < blen; i++)
		if(body[i] == 13)
			for(int j = i; j < blen; j++)
				body[j] = body[j + 1];

	tmplt.seekg(this->strlen(seek), ios_base::cur);
	len = (int)tmplt.tellg();
	tmplt.seekg(0, ios_base::end);
	len = (int)tmplt.tellg() - len;

	footer = new char[len + 1];
	footer[len] = 0;
	tmplt.seekg(-len, ios_base::end);
	tmplt.read(footer, len);
	for(int i = 0; i < len; i++)
		if(footer[i] == 13)
			for(int j = i; j < len; j++)
				footer[j] = footer[j + 1];

	// Look for "Before Album" tag section
	slen = this->strlen(balbum) + 2;
	seek = new char[slen + 1];
	seek[0] = '<';
	seek[slen - 1] = '>';
	seek[slen] = 0;
	for(int i = 0; i < this->strlen(balbum); i++)
		seek[i + 1] = balbum[i];
    
	loop = true;
	int start = 0;
	int end = 0;
	for(int i = 0; loop && (i < this->strlen(body)); i++)
	{
		loop = false;
		for(int j = 0; j < this->strlen(seek); j++)
		{
			if((seek[j] | 0x20) != (body[i] | 0x20))
			{
				loop = true;
				break;
			}
			else
			{
				start = ++i;
			}
		}
	}
	if(loop)
		this->showAlb = false;
	else
	{
		loop = true;
		end = 0;
		seek[0] = '_';
		seek[slen - 1] = '_';
		for(int i = 0; loop && (i < this->strlen(body)); i++)
		{
			loop = false;
			for(int j = 0; j < this->strlen(seek); j++)
			{
				if((seek[j] | 0x20) != (body[i] | 0x20))
				{
					loop = true;
					break;
				}
				else
				{
					end = ++i;
				}
			}
		}
		if(loop)
			this->showAlb = false;
		else
		{
			end -= this->strlen(seek);

			seek = new char[++slen + 1];
            seek[slen] = 0;
			seek[0] = '<';
			seek[1] = '/';
			seek[slen - 1] = '>';
			for(int i = 0; i < this->strlen(balbum); i++)
				seek[i + 2] = balbum[i];

			balbum = new char[end - start + 1];
			balbum[end - start] = 0;
            for(int i = start; i < end; i++)
				balbum[i - start] = body[i];

			start = end + this->strlen(seek) - 1;
			loop = true;
			end = 0;
			for(int i = 0; loop && (i < this->strlen(body)); i++)
			{
				loop = false;
				for(int j = 0; j < this->strlen(seek); j++)
				{
					if((seek[j] | 0x20) != (body[i] | 0x20))
					{
						loop = true;
						break;
					}
					else
					{
						end = ++i;
					}
				}
			}
			if(loop)
				this->showAlb = false;
			else
			{
				end -= this->strlen(seek);
				aalbum = new char[end - start + 1];
				aalbum[end - start] = 0;
				for(int i = start; i < end; i++)
					aalbum[i - start] = body[i];
				cout << aalbum;
			}
		}
	}

    tmplt.close();
	exit(0);
}

void DirScanner::sortArtists()
{
	/**/
	Artist temp;
	for(int j = 0; j < this->num; j++)
	{
		for(int i = 0; i < (this->num) - 1; i++)
		{
			if(artists[i].getName().compare(artists[i + 1].getName()) > 0)
			{
				temp = artists[i];
				artists[i] = artists[i + 1];
				artists[i + 1] = temp;
			}
		}
	}
	//*/
}

// Find the length of a string.  This is done by
// searching for the first null.
int DirScanner::strlen(char * str)
{
	char x = str[0];
	int ret = 0;

	while(x != 0)
		x = str[++ret];

	return ret;
}

// Convert a string to all uppercase
char* DirScanner::strtoupper(char* str)
{
	char x = str[0];
	int index = 0;

	while(x != 0)
	{
		if(x < 123 && x > 96)
			str[index] -= 32;
		x = str[++index];
	}

	return str;
}

// Trim the whitespace from the front and
// back of a string
char* DirScanner::trim(char* str, int len)
{
	char x = str[len - 1];
	int index = len - 1;

    while(x < 33 && index > 0)
	{
		str[index] = 0;
		x = str[--index];
	}

	return str;
}

bool getTime(char* header, int fsize, int &min, int &sec)
{
	unsigned short int version = (((unsigned char)header[1]) >> 3) & 0x03;
	unsigned short int layer = (((unsigned char)header[1]) >> 1) & 0x03;
	int bitrate = (unsigned char)header[2] >> 4;
	int samplerate = ((unsigned char)header[2] >> 2) & 0x03;

	layer = (layer == 1) ? 3 : 1;

	if(version == 3)
	{
		// MPEG version 1

		switch(samplerate)
		{
			case 0:
				samplerate = 44100;
				break;
			case 1:
				samplerate = 48000;
				break;
			case 2:
				samplerate = 32000;
				break;
			default:
				samplerate = -1;

		}
		switch(bitrate)
		{
			case 1:
				bitrate = 32;
				break;
			case 2:
				bitrate = 40;
				break;
			case 3:
				bitrate = 48;
				break;
			case 4:
				bitrate = 56;
				break;
			case 5:
				bitrate = 64;
				break;
			case 6:
				bitrate = 80;
				break;
			case 7:
				bitrate = 96;
				break;
			case 8:
				bitrate = 112;
				break;
			case 9:
				bitrate = 128;
				break;
			case 10:
				bitrate = 160;
				break;
			case 11:
				bitrate = 192;
				break;
			case 12:
				bitrate = 224;
				break;
			case 13:
				bitrate = 256;
				break;
			case 14:
				bitrate = 320;
				break;
			default:
				bitrate = -1;
				break;
		}
		bitrate *= 1000;
	}
	else if(version == 2)
	{
		switch(samplerate)
		{
			case 0:
				samplerate = 22050;
				break;
			case 1:
				samplerate = 24000;
				break;
			case 2:
				samplerate = 16000;
				break;
			default:
				samplerate = -1;

		}
		switch(bitrate)
		{
			case 1:
				bitrate = 8;
				break;
			case 2:
				bitrate = 16;
				break;
			case 3:
				bitrate = 24;
				break;
			case 4:
				bitrate = 32;
				break;
			case 5:
				bitrate = 40;
				break;
			case 6:
				bitrate = 48;
				break;
			case 7:
				bitrate = 56;
				break;
			case 8:
				bitrate = 64;
				break;
			case 9:
				bitrate = 80;
				break;
			case 10:
				bitrate = 96;
				break;
			case 11:
				bitrate = 112;
				break;
			case 12:
				bitrate = 128;
				break;
			case 13:
				bitrate = 144;
				break;
			case 14:
				bitrate = 160;
				break;
			case 15:
				bitrate = -1;
				break;
		}
		bitrate *= 1000;
	}
	else
	{
		samplerate = -1;
		bitrate = -1;
	}

	//cout << x.cFileName << "> ";

	if(layer == 3)
	{
		if(samplerate > 0 && bitrate > 0)
		{
			int framesize = ((144 * bitrate) / samplerate);
			float time = ((float)(fsize)/framesize) * 0.026;
			min = (int)(time / 60);
			sec = ((int)ceil(time) % 60);
			return true;
		}
	}
	return false;
}


ostream& operator << (ostream& out, string s)
{
	out << s.c_str();
	return out;
}
