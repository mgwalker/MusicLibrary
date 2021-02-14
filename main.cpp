/******************************************************************************
 *	Music Library Documenter
 *
 *	Checks all MP3s in a given directory and its subdirectories.  If the MP3s
 *	have ID3v1 or ID3v2 tags, it attempts to extract the artist, album name,
 *	and track title.  The total list of tracks, artists, and albums is then
 *	printed to the screen.
 ******************************************************************************/
#include <iostream>
#include <string.h>
#include "DirScannerC.h"
using namespace std;

#ifdef WIN32
	#define SLASH	'\\'
	#define SLASHS	"\\"
	#define STARS	"*"
#else
	#define SLASH	'/'
	#define SLASHS	"/"
	#define STARS	""
#endif

void showHelp(char* prog)
{
	cout << "\n  MP3 Library List Generator\n\n  Usage:\n  " << prog;
	cout << " [-RHAST] [argument] <starting directory> <output file>\n\n";
	cout << "  The starting directory is where you want the generator to begin looking for\n";
	cout << "  your MP3 files.  It will also search all of the subdirectories.  When it\n";
	cout << "  finds an MP3 file, it attempts to read the artist name, album name, and song\n";
	cout << "  title.  When it has finished searching everything in the starting directory,\n";
	cout << "  it writes its output to the file specified by the output file parameter.\n\n";
	cout << "  OPTIONS\n\n";
	cout << "    -R     No recursion.  Use this option if you do not want the generator to";
	cout << "\n           check the starting directory's subdirectories.\n\n";
	cout << "    -H     Generate HTML output.  By default, the generator creates a plain ";
	cout << "\n           text listing of the music that it finds.  Use this option to create\n";
	cout << "           the output in HTML tables.  Combine this option with -T to create\n";
	cout << "           your own output format.\n\n";
	cout << "    -P     Do not display artist names in the output.\n\n";
	cout << "    -A     Do not display album names in the output.\n\n";
	cout << "    -S     Do not display song names in the output.\n\n";
	cout << "    -F     Display the path and filename for the song.\n\n";
	cout << "    -L     Display the song play length.\n\n";
	cout << "    -X     Output in XML.\n\n";
	cout << "    -T     Use a template file for output.  This option automatically enables";
	cout << "\n           HTML output (see -H).  Using a template file allows you to configure\n";
	cout << "           the output however you want it to be.  The template file must\n";
	cout << "           contain certain elements so the generator knows how to deal with it.\n\n";
    cout << "             <LIST>   This tells the generator where to begin putting the list.\n";
	cout << "                      The section is terminated with </LIST>.\n\n";
	cout << "             <ARTIST> This section describes how the artist name is to be\n";
	cout << "                      output.  The section is terminated with </ARTIST>.\n\n";
	cout << "             <ALBUM>  This section describes how the album name is to be\n";
	cout << "                      output.  The section is terminated with </ALBUM>.\n\n";
	cout << "             <SONG>   This section describes how the song title is to be\n";
	cout << "                      output.  The section is terminated with </SONG>.\n\n";
	cout << "             _ARTIST_ This is where the artist name goes.  This must appear\n";
	cout << "                      inside the <ARTIST> section.\n\n";
	cout << "             _ALBUM_  This is where the album name goes.  This must appear\n";
	cout << "                      inside the <ALBUM> section.\n\n";
	cout << "             _SONG_   This is where the song title goes.  This must appear\n";
	cout << "                      inside the <SONG> section.\n\n";
	cout << "             There should be no extra information inside the <LIST> section.\n";
	cout << "             Everything should insde the <LIST> section must be inside the\n";
	cout << "             <ARTIST>, <ALBUM>, or <SONG> sections.  See the included template\n";
	cout << "             for an example.\n\n";
	cout << "    -STATS Generate statistics and no list.  This option disables the list and";
	cout << "\n           instead shows some statistics about the MP3s that are found.\n\n";
	cout << "    -HELP  Show help.  This is help.\n";
	exit(0);
}

int main(int argc, char* argv[])
{
	bool run = false;
	string startDir = "";
	string output = "";
	DirScanner ds;
	for(int i = 0; i < (argc - 1); i++)
	{
		if(argv[i + 1][0] != '-')
		{
			startDir = argv[i + 1];
			if(argv[i + 1][DirScanner::strlen(argv[i + 1]) - 1] != SLASH)
				startDir.append(SLASHS);
			startDir.append(STARS);
			if(i < (argc - 2))
			{
				output = argv[i + 2];
				run = true;
			}
			else if(i < (argc - 1) && ds.justGatherStats)
				run = true;
			break;
		}
		else
		{
			string tmp = DirScanner::strtoupper(argv[i + 1]);
			if(tmp.compare("-R") == 0)
				ds.noRecurse();
			else if(tmp.compare("-H") == 0)
				ds.HTML();
			else if(tmp.compare("-P") == 0)
				ds.noArtists();
			else if(tmp.compare("-A") == 0)
				ds.noAlbums();
			else if(tmp.compare("-S") == 0)
				ds.noSongs();
			else if(tmp.compare("-F") == 0)
				ds.yesFile();
			else if(tmp.compare("-L") == 0)
				ds.yesTime();
			else if(tmp.compare("-X") == 0)
				ds.yesXML();
			else if(tmp.compare("-HELP") == 0 || tmp.compare("-H") == 0)
				showHelp(argv[0]);
			else if(tmp.compare("-STATS") == 0)
				ds.justStats();
			else if(tmp.compare("-T") == 0)
			{
				if(argv[i + 2][0] != '-')
				{
					ds.templateHTML(argv[i + 2]);
					ds.HTML();
					ds.useTemplate();
					i++;
				}
				else
				{
					run = false;
					break;
				}
			}
		}
	}

	if(run)
	{
		ds.go(startDir);
		if(ds.justGatherStats)
			ds.exportStats();
		else
			ds.exportList(output);
	}
	else
	{
		cout << "\n  Incorrect parameters.  Please try again.\n";
		showHelp(argv[0]);
	}
	return 0;
}
