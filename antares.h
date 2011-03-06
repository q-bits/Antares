
#pragma once

extern "C" {

#include <libusb/libusb.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include "usb_io.h"
#include "connect.h"
#include "commands.h"

#include <time.h>


}


#define EPROTO 1

namespace Antares {


	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Collections::Generic;

	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;
	using namespace System::Configuration;
	using namespace System::Runtime::InteropServices; // for class Marshal

	[DllImport("shell32.dll")]
	DWORD_PTR SHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO* psfi, UINT cbSizeFileInfo, UINT uFlags);


	System::String^ HumanReadableSize(__u64 size);
	System::String^ DateString(time_t time);
	System::String^ DateString(System::DateTime time);
	System::String^ safeString( char* filename );
	System::String^ safeString( String^ filename );
	System::DateTime Time_T2DateTime(time_t t);
	String^ combineTopfieldPath(String^ path1,  String^ path2);





	public value class TopfieldFreeSpace
	{
	public:
		int freek;
		int totalk;
		bool valid;
	};

	public ref class Icons {
	public:
		DWORD_PTR imagelist;
		int folder_index;
		int file_index;
		Dictionary<String^, int> ^dic;
		Icons(void)
		{
			imagelist = GetFileIconList("test.txt");
			folder_index = GetApproximateFolderIconIndex("c:\\test\\");
			file_index = GetApproximateFileIconIndex("test.tkd3");
			dic = gcnew Dictionary<String^,int>();

		}



		int GetCachedIconIndex(String^ path)
		{
			return GetCachedIconIndex(path, false);
		}
		int GetCachedIconIndex(String ^path, bool istopfield)
		{
			if (dic->ContainsKey(path))
				return dic[path];
			else
			{
				int ic;
				if (istopfield)
					ic=GetApproximateFileIconIndex(path);
				else
					ic=GetFileIconIndex( path);

				if (ic>=0) 
				{
					dic->Add(path, ic);
					//Console::WriteLine("In dictionary, "+path+" has index "+ic.ToString());
				}

				return ic;
			}
		}



		static int GetFileIconIndex(String ^ path)
		{
			SHFILEINFO shinfo;
			wchar_t* str = (wchar_t*)(void*)Marshal::StringToHGlobalUni(path);
			DWORD_PTR ind;
			ind = Antares::SHGetFileInfo( str, 0, &shinfo, sizeof(shinfo), 0*SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
			return shinfo.iIcon;

		}



		static int GetApproximateFileIconIndex(String ^ path)
		{
			SHFILEINFO shinfo;
			wchar_t* str = (wchar_t*)(void*)Marshal::StringToHGlobalUni(path);
			DWORD_PTR ind;
			ind = Antares::SHGetFileInfo( str, FILE_ATTRIBUTE_NORMAL, &shinfo, sizeof(shinfo), 0*SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON );
			return shinfo.iIcon;

		}
			static int GetApproximateFolderIconIndex(String ^ path)
		{
			SHFILEINFO shinfo;
			wchar_t* str = (wchar_t*)(void*)Marshal::StringToHGlobalUni(path);
			DWORD_PTR ind;
			ind = Antares::SHGetFileInfo( str, FILE_ATTRIBUTE_DIRECTORY, &shinfo, sizeof(shinfo), 0*SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON );
			return shinfo.iIcon;

		}


		static DWORD_PTR GetFileIconList(String ^ path)
		{
			SHFILEINFO shinfo;
			wchar_t* str = (wchar_t*)(void*)Marshal::StringToHGlobalUni(path);
			DWORD_PTR im_list;
			im_list = Antares::SHGetFileInfo( str, 0, &shinfo, sizeof(shinfo), 0*SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES  | 0*SHGFI_ICON);
			return im_list;

		}

	};


	public ref class FileItem : public System::Windows::Forms::ListViewItem{
	public:
		FileItem(void) : System::Windows::Forms::ListViewItem("",0)
		{
			return;
		}

		System::String^ filename;
		System::String^ directory;
		System::String^ safe_filename;
		System::String^ datestring;
		System::String^ full_filename;
		System::String^ recursion_offset;

		System::String^ channel;
		System::String^ description;

		System::DateTime datetime;
		char type;
		long long int size;
		bool isdir;
		bool isdrive;



	};



	public ref class ComputerItem : public FileItem {


	public:
		bool isdrive;

		ComputerItem(int letter) : FileItem()    // Represents drive letter with letter-th letter of the alphabet
		{
			char dstr[2];
			dstr[0]='A'+letter;
			dstr[1]=0;
			String^ namestring =  ( gcnew String(dstr) ) +":\\";
			this->Text = namestring;
			this->isdir=true;
			this->datestring = "";
			this->size=0;
			this->type='f';
			this->filename=namestring;
            this->full_filename = namestring;
			this->recursion_offset="";
			this->isdrive = true;
			this->channel="";
			this->description="";

		}

		ComputerItem(String^ path, String^ dir) :  FileItem()
		{

			FileInfo^ f = gcnew FileInfo(path);   //Todo: handle exceptions
			FileAttributes attr = f->Attributes::get();
			this->filename = Path::GetFileName(path);
            this->isdrive = false;
			String^ typestring;
			String^ sizestring="";
			this->recursion_offset="";
			this->channel="";
			this->description="";
			this->directory = dir;
			if ( (attr & FileAttributes::Directory) == FileAttributes::Directory)
			{
				this->isdir=true;
				typestring = "Folder";
				this->size=0;
				//this->ImageIndex = 0;
			}
			else
			{
				this->isdir = false;
				typestring="File";
				this->size = f->Length::get();
				sizestring = HumanReadableSize(this->size);
				//if (this->filename->EndsWith(".rec"))
				//	this->ImageIndex = 2;
				//else
				//	this->ImageIndex = 1;



			}
			DateTime date = f->LastWriteTime;
			this->datetime = date;
			this->datestring = DateString(date);
			this->Text = this->filename;
			this->full_filename = path; 
			this->SubItems->Add( sizestring );
			this->SubItems->Add(typestring);
			this->SubItems->Add(this->datestring);
			this->SubItems->Add(this->channel);
			this->SubItems->Add(this->description);
			this->safe_filename = Antares::safeString(filename);

		}


	};




	public ref class TopfieldItem : public FileItem {

	public:


		TopfieldItem(typefile *entry, String^ containing_directory) : FileItem()
		{

			time_t timestamp;
			String ^namestring = gcnew String( (char *) entry->name);

			//toolStripStatusLabel1->Text= " i= "+i.ToString();
			String ^typestring = gcnew String("");
			switch (entry->filetype)
			{
			case 1:
				this->type = 'd';
				typestring = "Folder";
				this->isdir = true;
				//this->ImageIndex = 0;
				break;

			case 2:
				this->type = 'f';
				typestring = "File";
				//if (namestring->EndsWith(".rec"))
				//	this->ImageIndex=2;
				//else
				//	this->ImageIndex = 1;
				this->isdir=false;
				break;

			default:

				this->type= '?';
				typestring="??";
				this->isdir=false;
			}

			/* This makes the assumption that the timezone of the Toppy and the system
			* that puppy runs on are the same. Given the limitations on the length of
			* USB cables, this condition is likely to be satisfied. */
			timestamp = tfdt_to_time(&entry->stamp);
			//printf("%c %20llu %24.24s %s\n", type, get_u64(&entry->size),	ctime(&timestamp), entry->name);

			String ^safe_namestring = safeString((char *) entry->name );
			this->filename = namestring;
			this->size = get_u64(&entry->size);
			this->safe_filename = safe_namestring;
			this->recursion_offset = "";
			//char sizestr[100]; StrFormatByteSizeA( get_u64(&entries[i].size), sizestr, 99);
			String ^sizestring =  HumanReadableSize((__u64) get_u64(&entry->size));
			if (this->type=='d') sizestring="";
			//Console::WriteLine(safe_namestring);
			//String ^datestring = gcnew String( (char*) ctime(&timestamp));
			//CTime t = new CTime(timestamp);
			//DateTime^ dt =  gcnew DateTime(1970,1,1);
			//dt->AddSeconds((double) timestamp);


			this->channel="";
			this->description="";

			//newtime = localtime(&timestamp);
			this->datetime = Time_T2DateTime(timestamp);
			this->datestring = DateString(this->datetime);

			this->directory = containing_directory;
			this->full_filename = combineTopfieldPath(containing_directory, filename);
			//DateTime^ dt = DateTime::FromFileTimeUtc(timestamp * 10000000LL + 116444736000000000LL);



			//printf("Seconds = %d \n",timestamp);
			//String ^datestring = gcnew String( dt->ToString());

			//item = (gcnew ListViewItem(namestring  ,0));   //entries[i].name 
			this->Text = namestring;

			this->SubItems->Add( sizestring );
			this->SubItems->Add(typestring);
			this->SubItems->Add(this->datestring);
			this->SubItems->Add(this->channel);
			this->SubItems->Add(this->description);


		}



	};



	public ref class ListViewItemComparer : System::Collections::IComparer {
	public:
		int col;
		int order;
		ListViewItemComparer() {
			col=0;
			order=1;
		}
		ListViewItemComparer(int column, SortOrder sortorder) 
		{
			col=column;
			order=1; if (sortorder==SortOrder::Descending) order=-1;
		}
		virtual int Compare(System::Object^ x, System::Object^ y)
		{
			return this->order * CompareAscending(x,y);
		}
		int CompareAscending(System::Object^x, System::Object^y)
		{
			//int returnVal = 0;

			//returnVal = String::Compare(((ListViewItem)x).SubItems[col].Text,
			//((ListViewItem)y).SubItems[col].Text);
			FileItem^ fx = safe_cast<FileItem^> (x);
			FileItem^ fy = safe_cast<FileItem^> (y);

			// Whatever column it is, a folder is always "less" than a file
			if (fx->isdir && !fy->isdir) return -1;
			if (!fx->isdir && fy->isdir) return 1;

			// Sort by name
			if (this->col == 0) return String::Compare(fx->filename, fy->filename, true);

			// Sort by size
			if (this->col == 1)
			{
				if (fx->size < fy->size) return -1;
				if (fx->size > fy->size) return 1;
				return 0;
			}

			// Sort by type
			if (this->col == 2) return 0; //(currently only recognise two types)

			// Sort by date
			return String::Compare(fx->datestring, fy->datestring);


		}
	};


}