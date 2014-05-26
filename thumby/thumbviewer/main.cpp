//
//  main.cpp
//  Test
//
//  Created by Andreas Dewald on 18.04.14.
//  Copyright (c) 2014 Andreas Dewald. All rights reserved.
//

/*#include <iostream>

int main(int argc, const char * argv[])
{

    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
*/






/*
 thumbcache_viewer_cmd will extract thumbnail images from thumbcache database files.
 Copyright (C) 2011-2013 Eric Kutcher
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#define STRICT
//#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
#include <limits.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <wchar.h>

#include <iostream>
#include <fstream>
using namespace std;

// Magic identifiers for various image formats.
#define FILE_TYPE_BMP   "BM"
#define FILE_TYPE_JPEG  "\xFF\xD8\xFF\xE0"
#define FILE_TYPE_PNG   "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"

// Database version.
#define WINDOWS_VISTA   0x14
#define WINDOWS_7               0x15
#define WINDOWS_8               0x1A
#define WINDOWS_8v2             0x1C
#define WINDOWS_8v3             0x1E
#define WINDOWS_8_1             0x1F

int MAX_PATH = 1000;

// Thumbcache header information.
struct database_header
{
    char magic_identifier[ 4 ];
    unsigned int version;
    unsigned int type;      // Windows Vista & 7: 00 = 32, 01 = 96, 02 = 256, 03 = 1024, 04 = sr // Windows 8: 00 = 16, 01 = 32, 02 = 48, 03 = 96, 04 = 256, 05 = 1024, 06 = sr, 07 = wide, 08 = exif
};                                              // Windows 8.1: 00 = 16, 01 = 32, 02 = 48, 03 = 96, 04 = 256, 05 = 1024, 06 = 1600, 07 = sr, 08 = wide, 09 = exif, 0A = wide_alternate

// Found in WINDOWS_VISTA/7/8 databases.
struct database_header_entry_info
{
    unsigned int first_cache_entry;
    unsigned int available_cache_entry;
    unsigned int number_of_cache_entries;
};

// Found in WINDOWS_8v2 databases.
struct database_header_entry_info_v2
{
    unsigned int unknown;
    unsigned int first_cache_entry;
    unsigned int available_cache_entry;
    unsigned int number_of_cache_entries;
};

// Found in WINDOWS_8v3/8_1 databases.
struct database_header_entry_info_v3
{
    unsigned int unknown;
    unsigned int first_cache_entry;
    unsigned int available_cache_entry;
};

// Window 7 Thumbcache entry.
struct database_cache_entry_7
{
    char magic_identifier[ 4 ];
    unsigned int cache_entry_size;
    long long entry_hash;
    unsigned int filename_length;
    unsigned int padding_size;
    unsigned int data_size;
    unsigned int unknown;
    long long data_checksum;
    long long header_checksum;
};

// Window 8 Thumbcache entry.
struct database_cache_entry_8
{
    char magic_identifier[ 4 ];
    unsigned int cache_entry_size;
    long long entry_hash;
    unsigned int filename_length;
    unsigned int padding_size;
    unsigned int data_size;
    unsigned int width;
    unsigned int height;
    unsigned int unknown;
    long long data_checksum;
    long long header_checksum;
};

// Windows Vista Thumbcache entry.
struct database_cache_entry_vista
{
    char magic_identifier[ 4 ];
    unsigned int cache_entry_size;
    long long entry_hash;
    wchar_t extension[ 4 ];
    unsigned int filename_length;
    unsigned int padding_size;
    unsigned int data_size;
    unsigned int unknown;
    long long data_checksum;
    long long header_checksum;
};

int min(int i1, int i2){
	if (i1 < i2) return i1;
	return i2;
}

bool scan_memory( FILE *  hFile, unsigned int &offset )
{
    // Allocate a 32 kilobyte chunk of memory to scan. This value is arbitrary.
    char *buf = ( char * )malloc( sizeof( char ) * 32768 );
    char *scan = NULL;
    char read = 0;
    
    while ( true )
    {
        // Begin reading through the database.
        //ReadFile( hFile, buf, sizeof( char ) * 32768, &read, NULL );
        read=fread(buf, sizeof(char), 32768, hFile);
        
        if ( read <= 4 )
        {
            free( buf );
            return false;
        }
        
        // Binary string search. Look for the magic identifier.
        scan = buf;
        while ( read-- > 4 && memcmp( scan++, "CMMM", 4 ) != 0 )
        {
            ++offset;
        }
        
        // If it's not found, then we'll keep scanning.
        /*if ( read < 4 )
         {
         // Adjust the offset back 4 characters (in case we truncated the magic identifier when reading).
         SetFilePointer( hFile, offset, NULL, FILE_BEGIN );
         // Keep scanning.
         continue;
         }*/
        
        break;
    }
    
    free( buf );
    return true;
}

int main( int argc, char *argv[] )
{
    bool output_html = false;
    bool output_csv = false;
    bool skip_blank = false;
    bool extract_thumbnails = true;
    MAX_PATH = 1000;
    
    // Ask user for input filename.
	if (argc < 3)
		return -1;
    char* name = argv[1];//"../win7/databases/vol2-C..Users.Frodo.AppData.Local.Microsoft.Windows.Explorer.thumbcache_96.db";
    char* output_path = argv[2];//"/media/chris/Data/Uni/Thumbcaches/output/";
	argc=1;
    if ( argc == 1 )
    {
        /*printf( "Please enter the name of the database: " );
        fgets( name, MAX_PATH, stdin );
        
        // Remove the newline character if it was appended.
        int input_length = strlen( name );
        if ( name[ input_length - 1 ] == L'\n' )
        {
            name[ input_length - 1 ] = L'\0';
        }
         */
               
        /*printf( "Select a report to output:\n 1\tHTML\n 2\tComma-separated values (CSV)\n 3\tHTML and CSV\n 0\tNo report\nSelect: " );
        wint_t choice = getwchar();     // Newline character will remain in buffer.
         */
        wint_t choice = L'2';
        
        if ( choice == L'1' )
        {
            output_html = true;
        }
        else if ( choice == L'2' )
        {
            output_csv = true;
        }
        else if ( choice == L'3' )
        {
            output_html = output_csv = true;
        }
        
        
        if ( output_html == true || output_csv == true )
        {
            /*printf( "Do you want to skip reporting 0 byte files? (Y/N) " );
            while ( getwchar() != L'\n' );  // Clear the input buffer.
            choice = getwchar();            // Newline character will remain in buffer.
            if ( choice == L'y' || choice == L'Y' )
            {
                skip_blank = true;
            }*/
            skip_blank = false;
            
            /*printf( "Do you want to extract the thumbnail images? (Y/N) " );
            while ( getwchar() != L'\n' );  // Clear the input buffer.
            choice = getwchar();                            // Newline character will remain in buffer.
            if ( choice == L'n' || choice == L'N' )
            {
                extract_thumbnails = false;
            }
             */
            extract_thumbnails = true;
        }
        
        //while ( getwchar() != L'\n' );          // Clear the input buffer.
        
        /*printf( "Please enter a path to output the database files (Press Enter for the current directory): " );
        fgets( output_path, MAX_PATH, stdin );
        
        // Remove the newline character if it was appended.
        input_length = strlen( output_path );
        if ( output_path[ input_length - 1 ] == '\n' )
        {
            output_path[ input_length - 1 ] = '\0';
        }*/
    }
    else
    {
        // We're going to designate the last argument as the database path.
        int arg_len = strlen( argv[ argc - 1 ] );
        memcpy( name, argv[ argc - 1 ], ( arg_len > MAX_PATH ? MAX_PATH : arg_len ) );
        
        // Go through each argument and set the appropriate switch.
        for ( int i = 1; i <= ( argc - 1 ); i++ )
        {
            if ( strlen( argv[ i ] ) > 1 && ( argv[ i ][ 0 ] == '-' || argv[ i ][ 0 ] == '/' ) )
            {
                switch ( argv[ i ][ 1 ] )
                {
                    case 'o':
                    case 'O':
                    {
                        // Make sure our output switch is not the second to last argument.
                        if ( i < ( argc - 2 ) )
                        {
                            arg_len = strlen( argv[ ++i ] );
                            memcpy( output_path, argv[ i ], ( arg_len > MAX_PATH ? MAX_PATH : arg_len ) );
                        }
                    }
                        break;
                        
                    case 'w':
                    case 'W':
                    {
                        output_html = true;
                    }
                        break;
                        
                    case 'c':
                    case 'C':
                    {
                        output_csv = true;
                    }
                        break;
                        
                    case 'z':
                    case 'Z':
                    {
                        skip_blank = true;
                    }
                        break;
                        
                    case 'n':
                    case 'N':
                    {
                        extract_thumbnails = false;
                    }
                        break;
                        
                    case '?':
                    case 'h':
                    case 'H':
                    {
                        printf( "\nthumbcache_viewer_cmd [-o directory][-w][-c][-z][-n] thumbcache_*.db\n" \
                               " -o\tSet the output directory for thumbnails and reports.\n" \
                               " -w\tGenerate an HTML report.\n" \
                               " -c\tGenerate a comma-separated values (CSV) report.\n" \
                               " -z\tIgnore 0 byte files when generating a report.\n" \
                               " -n\tDo not extract thumbnails.\n");
                        return 0;
                    }
                        break;
                        
                    case 'a':
                    case 'A':
                    {
                        printf( "\nThumbcache Viewer is made free under the GPLv3 license.\nCopyright (c) 2011-2013 Eric Kutcher\n" );
                        return 0;
                    }
                        break;
                }
            }
        }
    }
    
    printf( "Attempting to open the database file.\n" );
    
    // Attempt to open our database file.
    FILE *  hFile = fopen( name, "rt" );
    if ( hFile != NULL )
    {
        char read = 0;
        char written = 0;
        
        unsigned int file_offset = 0;
        
        int path_length = 0;
        char *path = NULL;
        int name_length = 0;
        char *name = NULL;
        
        //FILE *  hFile_html = INVALID_FILE * _VALUE;
        //FILE *  hFile_csv = INVALID_FILE * _VALUE;
        FILE *  hFile_html =  0;
        FILE *  hFile_csv = 0;
        
        database_header dh = { 0 };
        //ReadFile( hFile, &dh, sizeof( database_header ), &read, NULL );
        read=fread( (void*) &dh, sizeof( database_header ), 1, hFile);
        
        // Make sure it's a thumbcache database and the stucture was filled correctly.
        if ( memcmp( dh.magic_identifier, "CMMM", 4 ) != 0 )//|| read != sizeof( database_header ) )
        {
            fclose ( hFile );
            printf( "The file is not a thumbcache database.\n" );
            return 0;
        }
        
        printf( "---------------------------------------------\n" );
        printf( "Extracting file header (%s bytes).\n", ( dh.version != WINDOWS_8v2 ? "24" : "28" ) );
        printf( "---------------------------------------------\n" );
        
        // Magic identifer.
        char stmp[ 5 ] = { 0 };
        memcpy( stmp, dh.magic_identifier, sizeof( char ) * 4 );
        printf( "Signature (magic identifier): %s\n", stmp );
        
        // Version of database.
        if ( dh.version == WINDOWS_VISTA )
        {
            printf( "Version: Windows Vista\n" );
        }
        else if ( dh.version == WINDOWS_7 )
        {
            printf( "Version: Windows 7\n" );
        }
        else if ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 )
        {
            printf( "Version: Windows 8\n" );
        }
        else if ( dh.version == WINDOWS_8_1 )
        {
            printf( "Version: Windows 8.1\n" );
        }
        else
        {
            fclose ( hFile );
            printf( "Database is not supported.\n" );
            return 0;
        }
        
        // Type of thumbcache database.
        if ( dh.version == WINDOWS_VISTA || dh.version == WINDOWS_7 )   // Windows Vista & 7: 00 = 32, 01 = 96, 02 = 256, 03 = 1024, 04 = sr
        {
            if ( dh.type == 0x00 )
            {
                printf( "Cache type: thumbcache_32.db, 32x32\n" );
            }
            else if ( dh.type == 0x01 )
            {
                printf( "Cache type: thumbcache_96.db, 96x96\n" );
            }
            else if ( dh.type == 0x02 )
            {
                printf( "Cache type: thumbcache_256.db, 256x256\n" );
            }
            else if ( dh.type == 0x03 )
            {
                printf( "Cache type: thumbcache_1024.db, 1024x1024\n" );
            }
            else if ( dh.type == 0x04 )
            {
                printf( "Cache type: thumbcache_sr.db\n" );
            }
            else
            {
                printf( "Cache type: Unknown\n" );
            }
        }
        else if ( dh.version == WINDOWS_8_1 )
        {
            if ( dh.type == 0x00 )
            {
                printf( "Cache type: thumbcache_16.db, 16x16\n" );
            }
            else if ( dh.type == 0x01 )
            {
                printf( "Cache type: thumbcache_32.db, 32x32\n" );
            }
            else if ( dh.type == 0x02 )
            {
                printf( "Cache type: thumbcache_48.db, 48x48\n" );
            }
            else if ( dh.type == 0x03 )
            {
                printf( "Cache type: thumbcache_96.db, 96x96\n" );
            }
            else if ( dh.type == 0x04 )
            {
                printf( "Cache type: thumbcache_256.db, 256x256\n" );
            }
            else if ( dh.type == 0x05 )
            {
                printf( "Cache type: thumbcache_1024.db, 1024x1024\n" );
            }
            else if ( dh.type == 0x06 )
            {
                printf( "Cache type: thumbcache_1600.db, 1600x1600\n" );
            }
            else if ( dh.type == 0x07 )
            {
                printf( "Cache type: thumbcache_sr.db\n" );
            }
            else if ( dh.type == 0x08 )
            {
                printf( "Cache type: thumbcache_wide.db\n" );
            }
            else if ( dh.type == 0x09 )
            {
                printf( "Cache type: thumbcache_exif.db\n" );
            }
            else if ( dh.type == 0x0A )
            {
                printf( "Cache type: thumbcache_wide_alternate.db\n" );
            }
            else
            {
                printf( "Cache type: Unknown\n" );
            }
        }
        else // Windows 8: 00 = 16, 01 = 32, 02 = 48, 03 = 96, 04 = 256, 05 = 1024, 06 = sr, 07 = wide, 08 = exif
        {
            if ( dh.type == 0x00 )
            {
                printf( "Cache type: thumbcache_16.db, 16x16\n" );
            }
            else if ( dh.type == 0x01 )
            {
                printf( "Cache type: thumbcache_32.db, 32x32\n" );
            }
            else if ( dh.type == 0x02 )
            {
                printf( "Cache type: thumbcache_48.db, 48x48\n" );
            }
            else if ( dh.type == 0x03 )
            {
                printf( "Cache type: thumbcache_96.db, 96x96\n" );
            }
            else if ( dh.type == 0x04 )
            {
                printf( "Cache type: thumbcache_256.db, 256x256\n" );
            }
            else if ( dh.type == 0x05 )
            {
                printf( "Cache type: thumbcache_1024.db, 1024x1024\n" );
            }
            else if ( dh.type == 0x06 )
            {
                printf( "Cache type: thumbcache_sr.db\n" );
            }
            else if ( dh.type == 0x07 )
            {
                printf( "Cache type: thumbcache_wide.db\n" );
            }
            else if ( dh.type == 0x08 )
            {
                printf( "Cache type: thumbcache_exif.db\n" );
            }
            else
            {
                printf( "Cache type: Unknown\n" );
            }
        }
        
        unsigned int first_cache_entry = 0;
        unsigned int available_cache_entry = 0;
        unsigned int number_of_cache_entries = 0;
        
        // WINDOWS_8v2 has an additional 4 bytes before the entry information.
        if ( dh.version == WINDOWS_8v2 )
        {
            database_header_entry_info_v2 dhei = { 0 };
            //ReadFile( hFile, &dhei, sizeof( database_header_entry_info_v2 ), &read, NULL );
            read=fread( (void*) &dhei, sizeof( database_header_entry_info_v2 ), 1, hFile);
            first_cache_entry = dhei.first_cache_entry;
            available_cache_entry = dhei.available_cache_entry;
            number_of_cache_entries = dhei.number_of_cache_entries;
        }
        else if ( dh.version == WINDOWS_8v3 || dh.version == WINDOWS_8_1 )
        {
            database_header_entry_info_v3 dhei = { 0 };
            //ReadFile( hFile, &dhei, sizeof( database_header_entry_info_v3 ), &read, NULL );
            read=fread( (void*) &dhei, sizeof( database_header_entry_info_v3 ), 1, hFile);
            first_cache_entry = dhei.first_cache_entry;
            available_cache_entry = dhei.available_cache_entry;
        }
        else
        {
            database_header_entry_info dhei = { 0 };
            //ReadFile( hFile, &dhei, sizeof( database_header_entry_info ), &read, NULL );
            read=fread( (void*) &dhei, sizeof( database_header_entry_info ), 1, hFile);
            first_cache_entry = dhei.first_cache_entry;
            available_cache_entry = dhei.available_cache_entry;
            number_of_cache_entries = dhei.number_of_cache_entries;
        }
        
        // Offset to the first cache entry.
        printf( "Offset to first cache entry: %lu bytes\n", first_cache_entry );
        
        // Offset to the available cache entry.
        printf( "Offset to available cache entry: %lu bytes\n", available_cache_entry );
        
        // Number of cache entries.
        if ( dh.version != WINDOWS_8v3 && dh.version != WINDOWS_8_1 )
        {
            printf( "Number of cache entries: %lu\n", number_of_cache_entries );
        }
        else
        {
            printf( "Number of cache entries: Unknown\n" );
        }
        
        printf( "---------------------------------------------\n" );
        
        // Set the file pointer to the first possible cache entry. (Should be at an offset equal to the size of the header)
        unsigned int current_position = ( dh.version != WINDOWS_8v2 ? 24 : 28 );
        
        // Create and set the directory that we'll be outputting files to.
        /*if ( GetFileAttributes( output_path ) == INVALID_FILE_ATTRIBUTES )
         {
         CreateDirectory( output_path, NULL );
         }*/
        
        //SetCurrentDirectory( output_path );                             // Set the path (relative or full)
        //GetCurrentDirectory( MAX_PATH, output_path );   // Get the full path
        
        // Convert our wide character strings to UTF-8 if we're going to output a report.
        /*if ( output_html == true || output_csv == true )
         {
         utf8_path_length = WideCharToMultiByte( CP_UTF8, 0, output_path, -1, NULL, 0, NULL, NULL );
         utf8_path = ( char * )malloc( sizeof( char ) * utf8_path_length );
         WideCharToMultiByte( CP_UTF8, 0, output_path, -1, utf8_path, utf8_path_length, NULL, NULL );
         
         utf8_name_length = WideCharToMultiByte( CP_UTF8, 0, name, -1, NULL, 0, NULL, NULL );
         utf8_name = ( char * )malloc( sizeof( char ) * utf8_name_length );
         WideCharToMultiByte( CP_UTF8, 0, name, -1, utf8_name, utf8_name_length, NULL, NULL );
         }*/
        
        // Create the HTML report file.
        if ( output_html == true )
        {
            //hFile_html = CreateFileA( "Report.html" , "w" );// , GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
            FILE *  hFile_html = fopen( "Report.html", "w" );
            if ( hFile_html == 0 )
            {
                printf( "HTML report could not be created.\n" );
                output_html = false;
            }
            else
            {
                // Add UTF-8 marker (BOM) if we're at the beginning of the file.
                /*if ( SetFilePointer( hFile_html, 0, NULL, FILE_END ) == 0 )
                 {
                 WriteFile( hFile_html, "\xEF\xBB\xBF", 3, &written, NULL );
                 }*/
                
                char entries[ 11 ] = { 0 };
                if ( dh.version != WINDOWS_8v3 && dh.version != WINDOWS_8_1 )
                {
                    sprintf( entries, "%lu", number_of_cache_entries );
                }
                else
                {
                    sprintf( entries, "Unknown" );
                }
                
                char *buf = ( char * )malloc( sizeof( char ) * ( name_length + path_length + 567 ) );
                int write_size = sprintf( buf, //name_length + path_length + 567,
                                         "<html><head><title>HTML Report</title></head><body>Filename: %s<br />" \
                                         "Version: %s<br />" \
                                         "Type: %s<br />" \
                                         "Offset to first cache entry (bytes): %lu<br />" \
                                         "Offset to available cache entry (bytes): %lu<br />" \
                                         "Number of cache entries: %s<br />" \
                                         "Output path: %s\\<br /><br />" \
                                         "<table border=1 cellspacing=0><tr><td>Index</td><td>Offset (bytes)</td><td>Cache Size (bytes)</td><td>Data Size (bytes)</td>%s<td>Entry Hash</td><td>Data Checksum</td><td>Header Checksum</td><td>Indentifier String</td><td>Image</td></tr>",
                                         name, ( dh.version == WINDOWS_VISTA ? "Windows Vista" : ( dh.version == WINDOWS_7 ? "Windows 7" : ( dh.version == WINDOWS_8_1 ? "Windows 8.1" : "Windows 8" ) ) ),
                                         ( dh.version == WINDOWS_VISTA || dh.version == WINDOWS_7 ) ? \
                                         ( dh.type == 0x00 ? "thumbcache_32.db" : ( dh.type == 0x01 ? "thumbcache_96.db" : ( dh.type == 0x02 ? "thumbcache_256.db" : ( dh.type == 0x03 ? "thumbcache_1024.db" : ( dh.type == 0x04 ? "thumbcache_sr.db" : "Unknown" ) ) ) ) ) : \
                                         ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 ) ? \
                                         ( dh.type == 0x00 ? "thumbcache_16.db" : ( dh.type == 0x01 ? "thumbcache_32.db" : ( dh.type == 0x02 ? "thumbcache_48.db" : ( dh.type == 0x03 ? "thumbcache_96.db" : ( dh.type == 0x04 ? "thumbcache_256.db" : ( dh.type == 0x05 ? "thumbcache_1024.db" : ( dh.type == 0x06 ? "thumbcache_sr.db" : ( dh.type == 0x07 ? "thumbcache_wide.db" : ( dh.type == 0x08 ? "thumbcache_exif.db" : "Unknown" ) ) ) ) ) ) ) ) ) : \
                                         ( dh.type == 0x00 ? "thumbcache_16.db" : ( dh.type == 0x01 ? "thumbcache_32.db" : ( dh.type == 0x02 ? "thumbcache_48.db" : ( dh.type == 0x03 ? "thumbcache_96.db" : ( dh.type == 0x04 ? "thumbcache_256.db" : ( dh.type == 0x05 ? "thumbcache_1024.db" : ( dh.type == 0x06 ? "thumbcache_1600.db" : ( dh.type == 0x07 ? "thumbcache_sr.db" : ( dh.type == 0x08 ? "thumbcache_wide.db" : ( dh.type == 0x09 ? "thumbcache_exif.db" : ( dh.type == 0x0A ? "thumbcache_wide_alternate.db" : "Unknown" ) ) ) ) ) ) ) ) ) ) ),
                                         first_cache_entry, available_cache_entry, entries, path, ( ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 || dh.version == WINDOWS_8_1 ) ? "<td>Dimensions</td>" : "" ) );
                //WriteFile( hFile_html, buf, write_size, &written, NULL );
                written=fwrite( buf, write_size, 1, hFile_html );
                
                free( buf );
            }
        }
        
        // Create the CSV report file.
        if ( output_csv == true )
        {
			std::string name1 = output_path; 
			name1 += "Report.csv";
            hFile_csv = fopen( name1.c_str() , "w" );// , GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
            if ( hFile_csv == 0)// INVALID_FILE * _VALUE )
            {
                printf( "CVS report could not be created.\n" );
                output_csv = false;
            }
            else
            {
                // Add UTF-8 marker (BOM) if we're at the beginning of the file.
                /*if ( SetFilePointer( hFile_csv, 0, NULL, FILE_END ) == 0 )
                 {
                 WriteFile( hFile_csv, "\xEF\xBB\xBF", 3, &written, NULL );
                 }*/
                
                char entries[ 11 ] = { 0 };
                if ( dh.version != WINDOWS_8v3 && dh.version != WINDOWS_8_1 )
                {
                    sprintf( entries, "%lu", number_of_cache_entries );
                }
                else
                {
                    sprintf( entries, "Unknown" );
                }
                
                char *buf = ( char * )malloc( sizeof( char ) * ( name_length + path_length + 357 ) );
                int write_size = sprintf( buf, //name_length + path_length + 357,
                                         "Filename,\"%s\"\r\n" \
                                         "Version,%s\r\n" \
                                         "Type,%s\r\n" \
                                         "Offset to first cache entry (bytes),%lu\r\n" \
                                         "Offset to available cache entry (bytes),%lu\r\n" \
                                         "Number of cache entries,%s\r\n" \
                                         "Output path,\"%s\\\"\r\n\r\n" \
                                         "Index,Offset (bytes),Cache Size (bytes),Data Size (bytes),%sEntry Hash,Data Checksum,Header Checksum,Indentifier String\r\n",
                                         name, ( dh.version == WINDOWS_VISTA ? "Windows Vista" : ( dh.version == WINDOWS_7 ? "Windows 7" : ( dh.version == WINDOWS_8_1 ? "Windows 8.1" : "Windows 8" ) ) ),
                                         ( dh.version == WINDOWS_VISTA || dh.version == WINDOWS_7 ) ? \
                                         ( dh.type == 0x00 ? "thumbcache_32.db" : ( dh.type == 0x01 ? "thumbcache_96.db" : ( dh.type == 0x02 ? "thumbcache_256.db" : ( dh.type == 0x03 ? "thumbcache_1024.db" : ( dh.type == 0x04 ? "thumbcache_sr.db" : "Unknown" ) ) ) ) ) : \
                                         ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 ) ? \
                                         ( dh.type == 0x00 ? "thumbcache_16.db" : ( dh.type == 0x01 ? "thumbcache_32.db" : ( dh.type == 0x02 ? "thumbcache_48.db" : ( dh.type == 0x03 ? "thumbcache_96.db" : ( dh.type == 0x04 ? "thumbcache_256.db" : ( dh.type == 0x05 ? "thumbcache_1024.db" : ( dh.type == 0x06 ? "thumbcache_sr.db" : ( dh.type == 0x07 ? "thumbcache_wide.db" : ( dh.type == 0x08 ? "thumbcache_exif.db" : "Unknown" ) ) ) ) ) ) ) ) ) : \
                                         ( dh.type == 0x00 ? "thumbcache_16.db" : ( dh.type == 0x01 ? "thumbcache_32.db" : ( dh.type == 0x02 ? "thumbcache_48.db" : ( dh.type == 0x03 ? "thumbcache_96.db" : ( dh.type == 0x04 ? "thumbcache_256.db" : ( dh.type == 0x05 ? "thumbcache_1024.db" : ( dh.type == 0x06 ? "thumbcache_1600.db" : ( dh.type == 0x07 ? "thumbcache_sr.db" : ( dh.type == 0x08 ? "thumbcache_wide.db" : ( dh.type == 0x09 ? "thumbcache_exif.db" : ( dh.type == 0x0A ? "thumbcache_wide_alternate.db" : "Unknown" ) ) ) ) ) ) ) ) ) ) ),
                                         first_cache_entry, available_cache_entry, entries, path, ( ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 || dh.version == WINDOWS_8_1 ) ? "Dimensions," : "" ) );
                //WriteFile( hFile_csv, buf, write_size, &written, NULL );
                written=fwrite( buf, write_size, 1, hFile_csv );
                
                free( buf );
            }
        }
        
        // Free our UTF-8 strings.
        free( name );
        free( path );
        
        // Go through our database and attempt to extract each cache entry.
        for ( unsigned int i = 0; true; ++i )
        {
            printf( "\n---------------------------------------------\n" );
            printf( "Extracting cache entry %lu at %lu bytes.\n", i + 1, current_position );
            printf( "---------------------------------------------\n" );
            
            file_offset = current_position; // Save for our report files.
            
            // Set the file pointer to the end of the last cache entry.
            //current_position = SetFilePointer( hFile, current_position, NULL, FILE_BEGIN );
            int current_position2 = fseek ( hFile , current_position , SEEK_SET );
            if ( current_position2 != 0) //INVALID_SET_FILE_POINTER )
            {
                printf( "End of file reached. There are no more entries.\n" );
                break;
            }
            
            void *database_cache_entry = NULL;
            
            // Determine the type of database we're working with and store its content in the correct structure.
            if ( dh.version == WINDOWS_7 )
            {
                database_cache_entry = ( database_cache_entry_7 * )malloc( sizeof( database_cache_entry_7 ) );
                //ReadFile( hFile, database_cache_entry, sizeof( database_cache_entry_7 ), &read, NULL );
                int mysize = sizeof( database_cache_entry_7 );
                read=fread( (void*) database_cache_entry, 1, sizeof( database_cache_entry_7 ), hFile);
                
                // Make sure it's a thumbcache database and the stucture was filled correctly.
                if ( read != sizeof( database_cache_entry_7 ) )
                {
                    free( database_cache_entry );
                    printf( "End of file reached. There are no more entries.\n" );
                    break;
                }
                else if ( memcmp( ( ( database_cache_entry_7 * )database_cache_entry )->magic_identifier, "CMMM", 4 ) != 0 )
                {
                    free( database_cache_entry );
                    
                    printf( "Invalid cache entry located at %lu bytes.\n", current_position );
                    printf( "Attempting to scan for next entry.\n" );
                    
                    // Walk back to the end of the last cache entry.
                    //current_position = SetFilePointer( hFile, current_position, NULL, FILE_BEGIN );
                    current_position2 = fseek ( hFile , current_position , SEEK_SET );
                    
                    // If we found the beginning of the entry, attempt to read it again.
                    if ( scan_memory( hFile, current_position ) == true )
                    {
                        printf( "A valid entry has been found.\n" );
                        printf( "---------------------------------------------\n" );
                        --i;
                        continue;
                    }
                    
                    printf( "Scan failed to find any valid entries.\n" );
                    printf( "---------------------------------------------\n" );
                    break;
                }
            }
            else if ( dh.version == WINDOWS_VISTA )
            {
                database_cache_entry = ( database_cache_entry_vista * )malloc( sizeof( database_cache_entry_vista ) );
                //ReadFile( hFile, database_cache_entry, sizeof( database_cache_entry_vista ), &read, NULL );
                read=fread( (void*) database_cache_entry, sizeof( database_cache_entry_vista ), 1, hFile);
                
                // Make sure it's a thumbcache database and the stucture was filled correctly.
                if ( read != sizeof( database_cache_entry_vista ) )
                {
                    free( database_cache_entry );
                    printf( "End of file reached. There are no more entries.\n" );
                    break;
                }
                else if ( memcmp( ( ( database_cache_entry_vista * )database_cache_entry )->magic_identifier, "CMMM", 4 ) != 0 )
                {
                    free( database_cache_entry );
                    
                    printf( "Invalid cache entry located at %lu bytes.\n", current_position );
                    printf( "Attempting to scan for next entry.\n" );
                    
                    // Walk back to the end of the last cache entry.
                    //current_position = SetFilePointer( hFile, current_position, NULL, FILE_BEGIN );
                    current_position2 = fseek ( hFile , current_position , SEEK_SET );
                    
                    // If we found the beginning of the entry, attempt to read it again.
                    if ( scan_memory( hFile, current_position ) == true )
                    {
                        printf( "A valid entry has been found.\n" );
                        printf( "---------------------------------------------\n" );
                        --i;
                        continue;
                    }
                    
                    printf( "Scan failed to find any valid entries.\n" );
                    printf( "---------------------------------------------\n" );
                    break;
                }
            }
            else if ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 || dh.version == WINDOWS_8_1 )
            {
                database_cache_entry = ( database_cache_entry_8 * )malloc( sizeof( database_cache_entry_8 ) );
                //ReadFile( hFile, database_cache_entry, sizeof( database_cache_entry_8 ), &read, NULL );
                read=fread( (void*) database_cache_entry, sizeof( database_cache_entry_8 ), 1, hFile);
                
                // Make sure it's a thumbcache database and the stucture was filled correctly.
                if ( read != sizeof( database_cache_entry_8 ) )
                {
                    free( database_cache_entry );
                    printf( "End of file reached. There are no more entries.\n" );
                    break;
                }
                else if ( memcmp( ( ( database_cache_entry_8 * )database_cache_entry )->magic_identifier, "CMMM", 4 ) != 0 )
                {
                    free( database_cache_entry );
                    
                    printf( "Invalid cache entry located at %lu bytes.\n", current_position );
                    printf( "Attempting to scan for next entry.\n" );
                    
                    // Walk back to the end of the last cache entry.
                    //current_position = SetFilePointer( hFile, current_position, NULL, FILE_BEGIN );
                    current_position2 = fseek ( hFile , current_position , SEEK_SET );
                    
                    // If we found the beginning of the entry, attempt to read it again.
                    if ( scan_memory( hFile, current_position ) == true )
                    {
                        printf( "A valid entry has been found.\n" );
                        printf( "---------------------------------------------\n" );
                        --i;
                        continue;
                    }
                    
                    printf( "Scan failed to find any valid entries.\n" );
                    printf( "---------------------------------------------\n" );
                    break;
                }
            }
            
            // I think this signifies the end of a valid database and everything beyond this is data that's been overwritten.
            if ( ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->entry_hash : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->entry_hash : ( ( database_cache_entry_8 * )database_cache_entry )->entry_hash ) ) == 0 )
            {
                printf( "Empty cache entry located at %lu bytes.\n", current_position );
                printf( "Adjusting offset for next entry.\n" );
                printf( "---------------------------------------------\n" );
                
                // Skip the header of this entry. If the next position is invalid (which it probably will be), we'll end up scanning.
                current_position += read;
                --i;
                // Free each database entry that we've skipped over.
                free( database_cache_entry );
                
                continue;
            }
            
            // Cache size includes the 4 byte signature and itself ( 4 bytes ).
            unsigned int cache_entry_size = ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->cache_entry_size : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->cache_entry_size : ( ( database_cache_entry_8 * )database_cache_entry )->cache_entry_size ) );
            
            current_position += cache_entry_size;
            
            // The magic identifier for the current entry.
            char *magic_identifier = ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->magic_identifier : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->magic_identifier : ( ( database_cache_entry_8 * )database_cache_entry )->magic_identifier ) );
            memcpy( stmp, magic_identifier, sizeof( char ) * 4 );
            printf( "Signature (magic identifier): %s\n", stmp );
            
            printf( "Cache size: %lu bytes\n", cache_entry_size );
            
            // The entry hash may be the same as the filename.
            char s_entry_hash[ 19 ] = { 0 };
            sprintf( s_entry_hash, "0x%016llx", ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->entry_hash : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->entry_hash : ( ( database_cache_entry_8 * )database_cache_entry )->entry_hash ) ) ); // This will probably be the same as the file name.
            printf( "Entry hash: %s\n", s_entry_hash );
            
            // Windows Vista
            if ( dh.version == WINDOWS_VISTA )
            {
                // UTF-16 file extension.
                wprintf( L"File extension: %.4s\n", ( ( database_cache_entry_vista * )database_cache_entry )->extension );
            }
            
            // The length of our filename.
            unsigned int filename_length = ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->filename_length : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->filename_length : ( ( database_cache_entry_8 * )database_cache_entry )->filename_length ) );
            printf( "Identifier string size: %lu bytes\n", filename_length );
            
            // Padding size.
            unsigned int padding_size = ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->padding_size : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->padding_size : ( ( database_cache_entry_8 * )database_cache_entry )->padding_size ) );
            printf( "Padding size: %lu bytes\n", padding_size );
            
            // The size of our data.
            unsigned int data_size = ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->data_size : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->data_size : ( ( database_cache_entry_8 * )database_cache_entry )->data_size ) );
            printf( "Data size: %lu bytes\n", data_size );
            
            // Windows 8 contains the width and height of the image.
            if ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 || dh.version == WINDOWS_8_1 )
            {
                printf( "Dimensions: %lux%lu\n", ( ( database_cache_entry_8 * )database_cache_entry )->width, ( ( database_cache_entry_8 * )database_cache_entry )->height );
            }
            
            // Unknown value.
            unsigned int unknown = ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->unknown : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->unknown : ( ( database_cache_entry_8 * )database_cache_entry )->unknown ) );
            printf( "Unknown value: 0x%04x\n", unknown );
            
            // CRC-64 data checksum.
            char s_data_checksum[ 19 ] = { 0 };
            sprintf( s_data_checksum, "0x%016llx", ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->data_checksum : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->data_checksum : ( ( database_cache_entry_8 * )database_cache_entry )->data_checksum ) ) );
            printf( "Data checksum (CRC-64): %s\n", s_data_checksum );
            
            // CRC-64 header checksum.
            char s_header_checksum[ 19 ] = { 0 };
            sprintf( s_header_checksum, "0x%016llx", ( ( dh.version == WINDOWS_7 ) ? ( ( database_cache_entry_7 * )database_cache_entry )->header_checksum : ( ( dh.version == WINDOWS_VISTA ) ? ( ( database_cache_entry_vista * )database_cache_entry )->header_checksum : ( ( database_cache_entry_8 * )database_cache_entry )->header_checksum ) ) );
            printf( "Header checksum (CRC-64): %s\n", s_header_checksum );
            
            // Since the database can store CLSIDs that extend beyond MAX_PATH, we'll have to set a larger truncation length. A length of 32767 would probably never be seen.
            unsigned int filename_truncate_length = min( filename_length, ( sizeof( wchar_t ) * SHRT_MAX ) );
            
            // UTF-16 filename. Allocate the filename length plus 6 for the unicode extension and null character.
            wchar_t *filename = ( wchar_t * )malloc( filename_truncate_length + ( sizeof( wchar_t ) * 6 ) );
            memset( filename, 0, filename_truncate_length + ( sizeof( wchar_t ) * 6 ) );
            //ReadFile( hFile, filename, filename_truncate_length, &read, NULL );
            read=fread( (void*) filename, filename_truncate_length, 1, hFile);
            if ( read == 0 )
            {
                free( filename );
                free( database_cache_entry );
                printf( "End of file reached. There are no more valid entries.\n" );
                break;
            }
            
            unsigned int file_position = 0;
            
            // Adjust our file pointer if we truncated the filename. This really shouldn't happen unless someone tampered with the database, or it became corrupt.
            if ( filename_length > filename_truncate_length )
            {
                // Offset the file pointer and see if we've moved beyond the EOF.
                //file_position = SetFilePointer( hFile, filename_length - filename_truncate_length, 0, FILE_CURRENT );
                int file_position2 = fseek ( hFile , filename_length - filename_truncate_length , SEEK_CUR );
                if ( file_position2 != 0)//INVALID_SET_FILE_POINTER )
                {
                    free( filename );
                    free( database_cache_entry );
                    printf( "End of file reached. There are no more valid entries.\n" );
                    break;
                }
            }
            
            // This will set our file pointer to the beginning of the data entry.
            //file_position = SetFilePointer( hFile, padding_size, 0, FILE_CURRENT );
            int file_position2 = fseek ( hFile , padding_size , SEEK_CUR );
            if ( file_position2 != 0)//INVALID_SET_FILE_POINTER )
            {
                free( filename );
                free( database_cache_entry );
                printf( "End of file reached. There are no more valid entries.\n" );
                break;
            }
            
            // Retrieve the data content.
            char *buf = NULL;
            char *myextension=".jpg";
            
            if ( data_size != 0 )
            {
                buf = ( char * )malloc( sizeof( char ) * data_size );
                //ReadFile( hFile, buf, data_size, &read, NULL );
                read=fread( (void*) buf, data_size, 1, hFile);
                if ( read == 0 )
                {
                    free( buf );
                    free( filename );
                    free( database_cache_entry );
                    printf( "End of file reached. There are no more valid entries.\n" );
                    break;
                }
                
                // Detect the file extension and copy it into the filename string.
                if ( memcmp( buf, FILE_TYPE_BMP, 2 ) == 0 )                     // First 3 bytes
                {
                    memcpy( filename + ( filename_truncate_length / sizeof( wchar_t ) ), ".bmp", 4 ); myextension = ".bmp";
                }
                else if ( memcmp( buf, FILE_TYPE_JPEG, 4 ) == 0 )       // First 4 bytes
                {
                    memcpy( filename + ( filename_truncate_length / sizeof( wchar_t ) ), ".jpg", 4 );myextension = ".jpg";
                }
                else if ( memcmp( buf, FILE_TYPE_PNG, 8 ) == 0 )        // First 8 bytes
                {
                    memcpy( filename + ( filename_truncate_length / sizeof( wchar_t ) ), ".png", 4 );myextension = ".png";
                }
                else if ( dh.version == WINDOWS_VISTA && ( ( database_cache_entry_vista * )database_cache_entry )->extension[ 0 ] != NULL )     // If it's a Windows Vista thumbcache file and we can't detect the extension, then use the one given.
                {
                    memcpy( filename + ( filename_truncate_length / sizeof( wchar_t ) ),  ".", 1 );
                    memcpy( filename + ( filename_truncate_length / sizeof( wchar_t ) ) + 1, ( ( database_cache_entry_vista * )database_cache_entry )->extension, 4 );
                }
            }
            else
            {
                // Windows Vista thumbcache files should include the extension.
                if ( dh.version == WINDOWS_VISTA && ( ( database_cache_entry_vista * )database_cache_entry )->extension[ 0 ] != NULL )
                {
                    memcpy( filename + ( filename_truncate_length / sizeof( wchar_t ) ), ".", 1 );
                    memcpy( filename + ( filename_truncate_length / sizeof( wchar_t ) ) + 1, ( ( database_cache_entry_vista * )database_cache_entry )->extension, 4 );
                }
            }
            
            
            wprintf( L"Identifier string: %s\n", filename );
            
            //char *filename = NULL;
            //int filename_length = 0;
            
            // Write the entry to a new table row in the HTML report file.
            if ( output_html == true && ( skip_blank == false || ( skip_blank == true && data_size > 0 ) ) )
            {
                char buf[ 196 ];
                int write_size = 0;
                if ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 || dh.version == WINDOWS_8_1 )   // Windows 8 includes dimensions (width x height)
                {
                    write_size = sprintf( buf,  "<tr><td>%lu</td><td>%lu</td><td>%lu</td><td>%lu</td><td>%lux%lu</td><td>%s</td><td>%s</td><td>%s</td><td>", i + 1, file_offset, cache_entry_size, data_size, ( ( database_cache_entry_8 * )database_cache_entry )->width, ( ( database_cache_entry_8 * )database_cache_entry )->height, s_entry_hash, s_data_checksum, s_header_checksum );
                }
                else
                {
                    write_size = sprintf( buf,  "<tr><td>%lu</td><td>%lu</td><td>%lu</td><td>%lu</td><td>%s</td><td>%s</td><td>%s</td><td>", i + 1, file_offset, cache_entry_size, data_size, s_entry_hash, s_data_checksum, s_header_checksum );
                }
                //WriteFile( hFile_html, buf, write_size, &written, NULL );
                //ReadFile( hFile, database_cache_entry, sizeof( database_cache_entry_7 ), &read, NULL );
                //read=fread( (void*) database_cache_entry, sizeof( database_cache_entry_7 ), 1, hFile);
                written=fwrite( buf, write_size, 1, hFile_html );
                
                /*filename_length = WideCharToMultiByte( CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL );
                 filename = ( char * )malloc( sizeof( char ) * filename_length );      // Includes NULL character.
                 WideCharToMultiByte( CP_UTF8, 0, filename, -1, utf8_filename, utf8_filename_length, NULL, NULL );*/
                //WriteFile( hFile_html, filename, filename_length - 1, &written, NULL );
                written=fwrite( filename, filename_length-1, 1, hFile_html );
                
                // If there's an image we want to extract, then insert it into the last column.
                if ( data_size != 0 && extract_thumbnails == true )
                {
                    char *out_buf = ( char * )malloc( sizeof( char ) * ( filename_length + 33 ) );
                    write_size = sprintf( out_buf,  "</td><td><img src=\"%s\" /></td></tr>", filename );
                    
                    //WriteFile( hFile_html, out_buf, write_size, &written, NULL );
                    written=fwrite( out_buf, write_size, 1, hFile_html );
                    
                    free( out_buf );
                }
                else    // Otherwise, the column will remain empty.
                {
                    //WriteFile( hFile_html, "</td><td></td></tr>", 19, &written, NULL );
                    written=fwrite( "</td><td></td></tr>", 19, 1, hFile_html );
                }
                
                // Save the filename if we're going to output a cvs file. Cuts down on the number of conversions.
                if ( output_csv == false )
                {
                    free( filename );
                }
            }
            
            // Write the entry to a new line in the CSV report file.
            if ( output_csv == true && ( skip_blank == false || ( skip_blank == true && data_size > 0 ) ) )
            {
                char buf[ 125 ];
                int write_size = 0;
                if ( dh.version == WINDOWS_8 || dh.version == WINDOWS_8v2 || dh.version == WINDOWS_8v3 || dh.version == WINDOWS_8_1 )   // Windows 8 includes dimensions (width x height)
                {
                    write_size = sprintf( buf, "%lu,%lu,%lu,%lu,%lux%lu,%s,%s,%s,\"", i + 1, file_offset, cache_entry_size, data_size, ( ( database_cache_entry_8 * )database_cache_entry )->width, ( ( database_cache_entry_8 * )database_cache_entry )->height, s_entry_hash, s_data_checksum, s_header_checksum );
                }
                else
                {
                    write_size = sprintf( buf, "%lu,%lu,%lu,%lu,%s,%s,%s,\"", i + 1, file_offset, cache_entry_size, data_size, s_entry_hash, s_data_checksum, s_header_checksum );
                }
                //WriteFile( hFile_csv, buf, write_size, &written, NULL );
                written=fwrite( buf, write_size, 1, hFile_csv );
                
                /*if ( filename == NULL )
                 {
                 filename_length = WideCharToMultiByte( CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL );
                 filename = ( char * )malloc( sizeof( char ) * filename_length );      // Includes NULL character.
                 WideCharToMultiByte( CP_UTF8, 0, filename, -1, filename, utf8_filename_length, NULL, NULL );
                 }*/
                
                char *out_buf = ( char * )malloc( sizeof( char ) * ( filename_length + 3 ) );
                write_size = sprintf( out_buf, "%s\"\r\n", filename );
                //WriteFile( hFile_csv, out_buf, write_size, &written, NULL );
                written=fwrite( out_buf, write_size, 1, hFile_csv );
                
                free( out_buf );
                free( filename );
            }
            
            // Output the data with the given (UTF-16) filename.
            printf( "---------------------------------------------\n" );
            if ( data_size != 0 && extract_thumbnails == true )
            {
                printf( "Writing data to file.\n" );
                // Attempt to save the buffer to a file.
                char *filename2 = ( char * )malloc(  filename_truncate_length + 60 );
                memset( filename2, 0, filename_truncate_length+60);
				memcpy(filename2, output_path, 30);
                
                wcstombs(filename2+30, filename, wcslen(filename));
                
                filename2 = s_entry_hash;
				std::string name = output_path;
				name += s_entry_hash;
                
                FILE *  hFile_save = fopen( name.c_str(), "w" ); //CreateFile( filename , "w" );// , GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
                if ( hFile_save != 0)// INVALID_FILE * _VALUE )
                {
                    //WriteFile( hFile_save, buf, data_size, &written, NULL );
                    written=fwrite( buf, data_size, 1, hFile_save );
                    fclose ( hFile_save );
                    printf( "Writing complete.\n" );
                }
                else
                {
                    printf( "Writing failed.\n" );
                }
            }
            else if ( extract_thumbnails == false )
            {
                printf( "Writing skipped.\n" );
            }
            else
            {
                printf( "No data to write.\n" );
            }
            printf( "---------------------------------------------\n" );
            
            // Delete our data buffer.
            free( buf );
            
            // Delete our filename.
            //free( filename );
            
            // Delete our database cache entry.
            free( database_cache_entry );
        }
        
        // Close our HTML report.
        if ( output_html == true )
        {
            //WriteFile( hFile_html, "</table><br /></body></html>", 28, &written, NULL );
            written=fwrite( "</table><br /></body></html>", 28, 1, hFile_html );
            fclose ( hFile_html );
        }
        
        // Close our CSV report.
        if ( output_csv == true )
        {
            //WriteFile( hFile_csv, "\r\n", 2, &written, NULL );
            written=fwrite( "\r\n", 2, 1, hFile_csv );
            fclose ( hFile_csv );
        }
        
        // Close the input file.
        fclose ( hFile );
    }
    else
    {
        // See if they typed an incorrect filename.
        /*if ( GetLastError() == ERROR_FILE_NOT_FOUND )
         {
         printf( "The database file does not exist.\n" );
         }
         else    // For all other errors, it probably failed to open.
         {
         printf( "The database file failed to open.\n" );
         }*/
        ;
    }
    
    return 0;
}