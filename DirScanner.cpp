#include <iostream>
#include <windows.h>
#include <string.h>
#include <fstream>
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
}

// Primary directory scanner
void DirScanner::go(string startDir)
{
	string file;		// File being examined
	ifstream infile;	// Input file stream
	WIN32_FIND_DATA x;	// WIN32 API data structure
	HANDLE hnd;			// WIN32 file handle
	BOOL go = true;		// True as long as more files are present in the directory

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
				// Figure out the new starting directory
				string newStart = startDir.substr(0, startDir.length() - 1);
				newStart.append(x.cFileName);
				char app[3];
				app[0] = '\\';
				app[1] = '*';
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
				file = startDir.substr(0, startDir.length()-1);
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
					int tsize = (2097152 * (sze[0] & 0x7F)) + (16384 * (sze[1] & 0x7F)) + (128 * (sze[2] & 0x7F)) + (sze[3] & 0x7F);
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
						palbum->tracks[palbum->num - 1].setFile(file);
					}
				}
				// Close the file
				infile.close();
			}
		}
		// Get the next file.  If there are no more files, go will be false
		go = FindNextFile(hnd, &x);
	}
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

void DirScanner::exportList(string outfile)
{
	ofstream out;
	out.open(outfile.c_str(), ios_base::out);
	this->sortArtists();

	if(this->html)
	{
		if(this->usetmplt)
		{
			ifstream tmplt;
			char tmp, *header, *footer;
			char *bartist, *balbum, *bsong, *aartist, *aalbum, *asong;
			char *bfile, *afile;
			int hlen, flen, arlen, allen, slen, filen, x;
			bool loop = true;
			tmplt.open(this->tmplt.c_str(), ios_base::binary);

			if(!tmplt)
			{
				cout << "\n  Error opening template file.\n";
				exit(0);
			}

			tmplt.read(&tmp, 1);
			hlen = 0;
			while(loop && !tmplt.eof())
			{
				if(tmp == '<')
				{
					tmplt.read(&tmp, 1);
					if((tmp | 0x20) == 'l')
					{
						tmplt.read(&tmp, 1);
						if((tmp | 0x20) == 'i')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 's')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 't')
								{
									tmplt.read(&tmp, 1);
									if(tmp == '>')
										loop = false;
									else
										hlen+=5;
								}
								else
									hlen+=4;
							}
							else
								hlen+=3;
						}
						else
							hlen+=2;
					}
					else
						hlen++;
				}
				else
				{
					tmplt.read(&tmp, 1);
					hlen++;
				}
			}
			if(loop)
				exit(0);
			header = new char[hlen + 1];
			header[hlen] = 0;
			tmplt.seekg(0, ios_base::beg);
			tmplt.read(header, hlen);
			for(int i = 0; i < hlen; i++)
				if(header[i] == 13)
					for(int j = i; j < hlen; j++)
						header[j] = header[j + 1];
			out << header;

			loop = true;
			tmplt.seekg(0, ios_base::beg);

			tmplt.read(&tmp, 1);
			while(loop && !tmplt.eof())
			{
				if(tmp == '<')
				{
					tmplt.read(&tmp, 1);
					if(tmp == '/')
					{
						tmplt.read(&tmp, 1);
						if((tmp | 0x20) == 'l')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 'i')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 's')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 't')
									{
										tmplt.read(&tmp, 1);
										if(tmp == '>')
											loop = false;
									}
								}
							}
						}
					}
				}
				else
					tmplt.read(&tmp, 1);
			}
            if(loop)
				exit(0);

			flen = tmplt.tellg();
			x = flen;
			tmplt.seekg(0, ios_base::end);
			flen = ((int)tmplt.tellg()) - flen;
			tmplt.seekg(x, ios_base::beg);
			footer = new char[flen + 1];
			tmp = 0;
			memcpy(footer, &tmp, flen + 1);
			for(int i = 0; i <= flen; i++)
				footer[i] = 0;
			tmplt.read(footer, flen);
			for(int i = 0; i < flen; i++)
				if(footer[i] == 13)
					for(int j = i; j < flen; j++)
						footer[j] = footer[j + 1];

///////////////////////////////////////////////////////////////
			tmplt.clear();
			tmplt.seekg(hlen, ios_base::beg);
			tmplt.read(&tmp, 1);
			loop = true;
			while(loop && !tmplt.eof())
			{
				if(tmp == '<')
				{
					tmplt.read(&tmp, 1);
					if((tmp | 0x20) == 'a')
					{
						tmplt.read(&tmp, 1);
						if((tmp | 0x20) == 'r')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 't')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'i')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 's')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 't')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == '>')
												loop = false;
										}
									}
								}
							}
						}
					}
				}
				else
					tmplt.read(&tmp, 1);
			}
			if(loop)
				this->showArt = false;
			else
			{
				arlen = (int)tmplt.tellg();
				tmp = 0;
				tmplt.read(&tmp, 1);
				loop = true;
				while(loop && !tmplt.eof())
				{
					if(tmp == '_')
					{
						tmplt.read(&tmp, 1);
						if((tmp | 0x20) == 'a')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 'r')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 't')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'i')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 's')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == 't')
											{
												tmplt.read(&tmp, 1);
												if(tmp == '_')
													loop = false;
											}
										}
									}
								}
							}
						}
					}
					else
						tmplt.read(&tmp, 1);
				}
				if(loop)
					this->showArt = false;
				else
				{
					x = (int)tmplt.tellg() - 8;
					tmplt.seekg(arlen, ios_base::beg);
					arlen = x - arlen;
					bartist = new char[arlen + 1];
					for(int i = 0; i <= arlen; i++)
						bartist[i] = 0;
					tmplt.read(bartist, arlen);
					for(int i = 0; i < arlen; i++)
						if(bartist[i] == '\r')
							for(int j = i; j < arlen; j++)
								bartist[j] = bartist[j + 1];
					arlen = (int)tmplt.tellg() + 8;
					tmplt.seekg(8, ios_base::cur);
					tmp = 0;
					tmplt.read(&tmp, 1);
					loop = true;
					while(loop && !tmplt.eof())
					{
						if(tmp == '<')
						{
							tmplt.read(&tmp, 1);
							if(tmp == '/')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'a')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'r')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 't')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == 'i')
											{
												tmplt.read(&tmp, 1);
												if((tmp | 0x20) == 's')
												{
													tmplt.read(&tmp, 1);
													if((tmp | 0x20) == 't')
													{
														tmplt.read(&tmp, 1);
														if((tmp | 0x20) == '>')
															loop = false;
													}
												}
											}
										}
									}
								}
							}
						}
						else
							tmplt.read(&tmp, 1);
					}
					if(loop)
						this->showArt = false;
					else
					{
						x = (int)tmplt.tellg() - 10;
						tmplt.seekg(arlen, ios_base::beg);
						arlen = x - arlen - 1;
						aartist = new char[arlen + 1];
						for(int i = 0; i <= arlen; i++)
							aartist[i] = 0;
						tmplt.read(aartist, arlen);
						for(int i = 0; i < arlen; i++)
							if(aartist[i] == 13)
								for(int j = i; j < arlen; j++)
									aartist[j] = aartist[j + 1];
					}
				}
			}
/////////////////////////////////////////////
/////////////////////////////////////////////
			tmplt.clear();
			if(this->showAlb)
			{
				tmplt.seekg(hlen, ios_base::beg);
				tmplt.read(&tmp, 1);
				loop = true;
				while(loop && !tmplt.eof())
				{
					if(tmp == '<')
					{
						tmplt.read(&tmp, 1);
						if((tmp | 0x20) == 'a')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 'l')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'b')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'u')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 'm')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == '>')
												loop = false;
										}
									}
								}
							}
						}
					}
					else
						tmplt.read(&tmp, 1);
				}
				if(loop)
					this->showAlb = false;
				else
				{
					allen = (int)tmplt.tellg();
					tmp = 0;
					tmplt.read(&tmp, 1);
					loop = true;
					while(loop && !tmplt.eof())
					{
						if(tmp == '_')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 'a')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'l')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'b')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 'u')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == 'm')
											{
												tmplt.read(&tmp, 1);
												if(tmp == '_')
													loop = false;
											}
										}
									}
								}
							}
						}
						else
							tmplt.read(&tmp, 1);
					}
					if(loop)
						this->showAlb = false;
					else
					{
						x = (int)tmplt.tellg() - 7;
						tmplt.seekg(allen, ios_base::beg);
						allen = x - allen;
						balbum = new char[allen + 1];
						for(int i = 0; i <= allen; i++)
							balbum[i] = 0;
						tmplt.read(balbum, allen);
						for(int i = 0; i < allen; i++)
							if(balbum[i] == 13)
								for(int j = i; j < allen; j++)
									balbum[j] = balbum[j + 1];
						allen = (int)tmplt.tellg() + 7;
						tmplt.seekg(7, ios_base::cur);
						tmp = 0;
						tmplt.read(&tmp, 1);
						loop = true;
						while(loop && !tmplt.eof())
						{
							if(tmp == '<')
							{
								tmplt.read(&tmp, 1);
								if(tmp == '/')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'a')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 'l')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == 'b')
											{
												tmplt.read(&tmp, 1);
												if((tmp | 0x20) == 'u')
												{
													tmplt.read(&tmp, 1);
													if((tmp | 0x20) == 'm')
													{
														tmplt.read(&tmp, 1);
														if((tmp | 0x20) == '>')
															loop = false;
													}
												}
											}
										}
									}
								}
							}
							else
								tmplt.read(&tmp, 1);
						}
						if(loop)
							this->showAlb = false;
						else
						{
							x = (int)tmplt.tellg() - 10;
							tmplt.seekg(allen, ios_base::beg);
							allen = x - allen;
							aalbum = new char[allen + 1];
							for(int i = 0; i <= allen; i++)
								aalbum[i] = 0;
							tmplt.read(aalbum, allen);
							for(int i = 0; i < allen; i++)
								if(aalbum[i] == 13)
									for(int j = i; j < allen; j++)
										aalbum[j] = aalbum[j + 1];
						}
					}
				}
			}
/////////////////////////////////////////////
/////////////////////////////////////////////
			tmplt.clear();
			if(this->showSongs)
			{
				tmplt.seekg(hlen, ios_base::beg);
				tmplt.read(&tmp, 1);
				loop = true;
				while(loop && !tmplt.eof())
				{
					if(tmp == '<')
					{
						tmplt.read(&tmp, 1);
						if((tmp | 0x20) == 's')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 'o')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'n')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'g')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == '>')
											loop = false;
									}
								}
							}
						}
					}
					else
						tmplt.read(&tmp, 1);
				}
				if(loop)
					this->showSongs = false;
				else
				{
					slen = (int)tmplt.tellg() + 1;
					tmp = 0;
					tmplt.read(&tmp, 1);
					loop = true;
					while(loop && !tmplt.eof())
					{
						if(tmp == '_')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 's')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'o')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'n')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 'g')
										{
											tmplt.read(&tmp, 1);
											if(tmp == '_')
												loop = false;
										}
									}
								}
							}
						}
						else
							tmplt.read(&tmp, 1);
					}
					if(loop)
						this->showSongs = false;
					else
					{
						x = (int)tmplt.tellg() - 6;
						tmplt.seekg(slen, ios_base::beg);
						slen = x - slen;
						bsong = new char[slen + 1];
						for(int i = 0; i <= slen; i++)
							bsong[i] = 0;
						tmplt.read(bsong, slen);
						for(int i = 0; i < slen; i++)
							if(bsong[i] == 13)
								for(int j = i; j < slen; j++)
									bsong[j] = bsong[j + 1];
						slen = (int)tmplt.tellg() + 6;
						tmplt.seekg(6, ios_base::cur);
						tmp = 0;
						tmplt.read(&tmp, 1);
						loop = true;
						while(loop && !tmplt.eof())
						{
							if(tmp == '<')
							{
								tmplt.read(&tmp, 1);
								if(tmp == '/')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 's')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 'o')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == 'n')
											{
												tmplt.read(&tmp, 1);
												if((tmp | 0x20) == 'g')
												{
													tmplt.read(&tmp, 1);
													if((tmp | 0x20) == '>')
														loop = false;
												}
											}
										}
									}
								}
							}
							else
								tmplt.read(&tmp, 1);
						}
						if(loop)
							this->showSongs = false;
						else
						{
							x = (int)tmplt.tellg() - 8;
							tmplt.seekg(slen, ios_base::beg);
							slen = x - slen;
							asong = new char[slen + 1];
							for(int i = 0; i <= slen; i++)
								asong[i] = 0;
							tmplt.read(asong, slen);
							for(int i = 0; i < slen; i++)
								if(asong[i] == 13)
									for(int j = i; j < slen; j++)
										asong[j] = asong[j + 1];
						}
					}
				}
			}
/////////////////////////////////////////////
/////////////////////////////////////////////
			tmplt.clear();
			if(this->showFile)
			{
				tmplt.seekg(hlen, ios_base::beg);
				tmplt.read(&tmp, 1);
				loop = true;
				while(loop && !tmplt.eof())
				{
					if(tmp == '<')
					{
						tmplt.read(&tmp, 1);
						if((tmp | 0x20) == 'f')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 'i')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'l')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'e')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == '>')
											loop = false;
									}
								}
							}
						}
					}
					else
						tmplt.read(&tmp, 1);
				}
				if(loop)
					this->showFile = false;
				else
				{
					filen = (int)tmplt.tellg() + 1;
					tmp = 0;
					tmplt.read(&tmp, 1);
					loop = true;
					while(loop && !tmplt.eof())
					{
						if(tmp == '_')
						{
							tmplt.read(&tmp, 1);
							if((tmp | 0x20) == 'f')
							{
								tmplt.read(&tmp, 1);
								if((tmp | 0x20) == 'i')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'l')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 'e')
										{
											tmplt.read(&tmp, 1);
											if(tmp == '_')
												loop = false;
										}
									}
								}
							}
						}
						else
							tmplt.read(&tmp, 1);
					}
					if(loop)
						this->showFile = false;
					else
					{
						x = (int)tmplt.tellg() - 6;
						tmplt.seekg(filen, ios_base::beg);
						filen = x - filen;
						bfile = new char[filen + 1];
						for(int i = 0; i <= filen; i++)
							bfile[i] = 0;
						tmplt.read(bfile, filen);
						for(int i = 0; i < filen; i++)
							if(bfile[i] == 13)
								for(int j = i; j < filen; j++)
									bfile[j] = bfile[j + 1];
						filen = (int)tmplt.tellg() + 6;
						tmplt.seekg(6, ios_base::cur);
						tmp = 0;
						tmplt.read(&tmp, 1);
						loop = true;
						while(loop && !tmplt.eof())
						{
							if(tmp == '<')
							{
								tmplt.read(&tmp, 1);
								if(tmp == '/')
								{
									tmplt.read(&tmp, 1);
									if((tmp | 0x20) == 'f')
									{
										tmplt.read(&tmp, 1);
										if((tmp | 0x20) == 'i')
										{
											tmplt.read(&tmp, 1);
											if((tmp | 0x20) == 'l')
											{
												tmplt.read(&tmp, 1);
												if((tmp | 0x20) == 'e')
												{
													tmplt.read(&tmp, 1);
													if((tmp | 0x20) == '>')
														loop = false;
												}
											}
										}
									}
								}
							}
							else
								tmplt.read(&tmp, 1);
						}
						if(loop)
							this->showFile = false;
						else
						{
							x = (int)tmplt.tellg() - 8;
							tmplt.seekg(filen, ios_base::beg);
							filen = x - filen;
							afile = new char[filen + 1];
							for(int i = 0; i <= filen; i++)
								afile[i] = 0;
							tmplt.read(afile, filen);
							for(int i = 0; i < filen; i++)
								if(afile[i] == 13)
									for(int j = i; j < filen; j++)
										afile[j] = afile[j + 1];
						}
					}
				}
			}
/////////////////////////////////////////////
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
							out << "  <TR>\n    <TD CLASS=\"fileList\">"
								<< artists[i].albums[j].tracks[k].getFile()
								<< "</TD>\n  </TR>\n";
					}						
				}
			}

			out << "</TABLE>\n</BODY>\n<!-- Generated by the MP3 Library List Generator -->\n</HTML>";
		}
	}
	else
	{
		for(int i = 0; i < this->num; i++)
		{
			if(this->showArt)
				out << artists[i].getName() << endl;
			artists[i].sortAlbums();
			for(int j = 0; j < artists[i].num; j++)
			{
				if(this->showAlb)
					out << "  > " << artists[i].albums[j].getName() << endl;
				if(this->showSongs)
				{
					for(int k = 0; k < artists[i].albums[j].num; k++)
					{
						out << "    | " << artists[i].albums[j].tracks[k].getTitle();
						if(this->showFile)
							out << " (" << artists[i].albums[j].tracks[k].getFile() << ")";
						out << endl;
					}
				}
			}
		}
	}
	out.close();
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

ostream& operator << (ostream& out, string s)
{
	out << s.c_str();
	return out;
}