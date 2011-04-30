
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
#include "FBLib_rec.h"
#include <time.h>


}

#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif

#define EPROTO 1

namespace Antares {


	public enum class CopyDirection {PVR_TO_PC, PC_TO_PVR, UNDEFINED};
	public enum class CopyMode {COPY, MOVE, UNDEFINED};


	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Collections::Generic;

	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;
	using namespace System::Configuration;
	using namespace System::Runtime::InteropServices;

	[DllImport("shell32.dll")]
	DWORD_PTR SHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO* psfi, UINT cbSizeFileInfo, UINT uFlags);


	enum OverwriteAction
	{
		OVERWRITE,
		SKIP,
		RESUME
	};

	System::String^ HumanReadableSize(__u64 size);
	System::String^ DateString(time_t time);
	System::String^ DateString(System::DateTime time);
	System::String^ safeString( char* filename );
	System::String^ safeString( String^ filename );
	System::DateTime Time_T2DateTime(time_t t);
	String^ combineTopfieldPath(String^ path1,  String^ path2);


	///////////////////////////////////////////////////////
	//// Definitions relating to the TaskBarState class
	//
	// TODO: Clean up, put somewhere else


	enum TBPFLAG
	{
		TBPF_NOPROGRESS = 0,
		TBPF_INDETERMINATE = 0x1,
		TBPF_NORMAL = 0x2,
		TBPF_ERROR = 0x4,
		TBPF_PAUSED = 0x8
	};
	enum TBATFLAG
	{
		TBATF_USEMDITHUMBNAIL = 0x1,
		TBATF_USEMDILIVEPREVIEW = 0x2
	};

	enum THBMASK
	{
		THB_BITMAP = 0x1,
		THB_ICON = 0x2,
		THB_TOOLTIP = 0x4,
		THB_FLAGS = 0x8
	};

	enum THBFLAGS
	{
		THBF_ENABLED = 0,
		THBF_DISABLED = 0x1,
		THBF_DISMISSONCLICK = 0x2,
		THBF_NOBACKGROUND = 0x4,
		THBF_HIDDEN = 0x8
	};

	typedef unsigned int uint;

	[ComImport]
	[Guid("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")]
	[InterfaceType(ComInterfaceType::InterfaceIsIUnknown)]
	interface class ITaskbarList3
	{
		// ITaskbarList
		[PreserveSig]
		HRESULT HrInit();
		[PreserveSig]
		void AddTab(HWND hwnd);

		[PreserveSig]
		void DeleteTab(IntPtr hwnd);
		[PreserveSig]
		void ActivateTab(IntPtr hwnd);
		[PreserveSig]
		void SetActiveAlt(IntPtr hwnd);

		// ITaskbarList2
		[PreserveSig]
		void MarkFullscreenWindow(
			IntPtr hwnd,
			[MarshalAs(UnmanagedType::Bool)] bool fFullscreen);

		// ITaskbarList3
		HRESULT SetProgressValue(HWND hwnd, UInt64 ullCompleted, UInt64 ullTotal);
		HRESULT SetProgressState(HWND hwnd, TBPFLAG tbpFlags);


		void RegisterTab(IntPtr hwndTab, IntPtr hwndMDI);
		void UnregisterTab(IntPtr hwndTab);
		void SetTabOrder(IntPtr hwndTab, IntPtr hwndInsertBefore);
		void SetTabActive(IntPtr hwndTab, IntPtr hwndMDI, TBATFLAG tbatFlags);
		void ThumbBarAddButtons(IntPtr hwnd,		unsigned int cButtons,		[MarshalAs(UnmanagedType::LPArray)] void* pButtons);
		void ThumbBarUpdateButtons(
			IntPtr hwnd,
			unsigned int cButtons,
			[MarshalAs(UnmanagedType::LPArray)] void* pButtons);
		void ThumbBarSetImageList(IntPtr hwnd, IntPtr himl);
		void SetOverlayIcon(
			IntPtr hwnd,
			IntPtr hIcon,
			[MarshalAs(UnmanagedType::LPWStr)] String^ pszDescription);
		void SetThumbnailTooltip(
			IntPtr hwnd,
			[MarshalAs(UnmanagedType::LPWStr)] String^ pszTip);
		void SetThumbnailClip(
			IntPtr hwnd,
			RECT prcClip);

	};

	[Guid("56FDF344-FD6D-11d0-958A-006097C9A090")]
	[ClassInterface(ClassInterfaceType::None)]
	[ComImport]
	public ref class CTaskbarList { };


	public ref class TaskbarState {

	private:

		static int error_count=0;
		static int max_error_count = 10;
		static ITaskbarList3^ taskbarList;



	public:


		static void init()
		{
			if (taskbarList == nullptr)
			{
				try
				{

					taskbarList = (ITaskbarList3^) gcnew CTaskbarList();
					HRESULT hr=taskbarList->HrInit();
					//printf("HrInit() = %d\n",(int) hr);

				}
				catch (...) {error_count = max_error_count+1;}
			}
		}

		static void setState( System::Windows::Forms::Form^ form, TBPFLAG flags)
		{
			if (error_count<max_error_count)
			{

				init();
				try{
					HRESULT hr = taskbarList->SetProgressState((HWND) form->Handle.ToPointer(), flags);
					//printf("SetProgressState,  flags=%d  hr=%d   hwd=%d \n",(int) flags,(int) hr, (int) form->Handle.ToPointer());
				}
				catch(...){error_count++;}
			}
		}

		static void setNoProgress(System::Windows::Forms::Form^ form)
		{
			setState(form, TBPF_NOPROGRESS);
		}

		static void setNormal(System::Windows::Forms::Form^ form)
		{
			setState(form, TBPF_NORMAL);
		}
			static void setPaused(System::Windows::Forms::Form^ form)
		{
			setState(form, TBPF_PAUSED);
		}

		static void setError(System::Windows::Forms::Form^ form)
		{
			setState(form, TBPF_ERROR);
		}
		static void setIndeterminate(System::Windows::Forms::Form^ form)
		{
			setState(form, TBPF_INDETERMINATE);
		}

		static void addTab( System::Windows::Forms::Form^ form)
		{
			taskbarList->AddTab((HWND) form->Handle.ToPointer());
		}

		static void setValue( System::Windows::Forms::Form^ form, ULONGLONG value,  ULONGLONG maximum)
		{
			if (error_count<max_error_count)
			{
				init();

				try{
					HRESULT hr = taskbarList->SetProgressValue((HWND) form->Handle.ToPointer(), value, maximum);
					//printf("SetProgressValue,  val=%d  max=%d hr=%d   hwd=%d \n",(int) value, (int) maximum,(int) hr, (int) form->Handle.ToPointer());
				}
				catch(...){error_count++;}

			}
		}

	};


	////////////////////////////////////////////////




	public value class TopfieldFreeSpace
	{
	public:
		int freek;
		int totalk;
		bool valid;
	};



	public ref class FileType
	{
	public:
		int icon_index;
		String^ file_type;
		FileType(int icon_index_in, String^ file_type_in)
		{
			this->icon_index = icon_index_in;
			this->file_type = file_type_in;

		}
	};


	///////////  Icons class


	public ref class Icons {
	public:
		DWORD_PTR imagelist;
		int folder_index;
		int file_index;
		int play_index;
        FileType^ folder_info, ^file_info, ^play_info;

		Dictionary<String^, FileType^> ^dic;
		Dictionary<String^, FileType^> ^extension_dic;
		Icons(void)
		{
			imagelist = GetFileIconList("test.txt");
			folder_index = GetApproximateFolderIconIndex("c:\\test\\");
			file_index = GetApproximateFileIconIndex("test.tkd3")->icon_index;
			play_index = GetFileIconIndex(Application::ExecutablePath)->icon_index;
			folder_info = gcnew FileType(folder_index, "Folder");
			file_info = gcnew FileType(file_index, "");
			play_info = gcnew FileType(play_index,"REC File");
			//Console::WriteLine(Directory::GetCurrentDirectory());
			//Console::WriteLine(Application::ExecutablePath);
			//Console::WriteLine(System::Diagnostics::Process::GetCurrentProcess()->ProcessName+".exe");
			//Console::WriteLine("play_index="+play_index.ToString());Console::WriteLine(file_index); Console::WriteLine(folder_index);
			
			//Console::WriteLine(System::Diagnostics::Process::GetCurrentProcess()->ProcessName);
			dic = gcnew Dictionary<String^,FileType^>();
			extension_dic = gcnew Dictionary<String^, FileType^>();

		}



		FileType^ GetCachedIconIndex(String^ path)
		{
			return GetCachedIconIndex(path, false, false);
		}


		FileType^ GetCachedIconIndexFast(String ^path, bool istopfield, bool isfolder)
		{

			if (dic->ContainsKey(path))
				return dic[path];
			else if (!isfolder)
			{
				String^ ext = "";
				try {ext = Path::GetExtension(path);} catch(...){};

				if (ext->Length  && extension_dic->ContainsKey(ext))
				{
                     return extension_dic[ext];
				}

				if (path->EndsWith(".rec")) 
				{
					return this->play_info;
				}
			}

			if (isfolder) return 
				this->folder_info; 
			else 
			{
				FileType^ info = gcnew FileType(file_index, "");
				String^ type = "File";
				try {
				type =  Path::GetExtension(path) + " " + type;
				}
				catch(...){};

				info->file_type = type;
				return info;
			}


		}

		FileType^ GetCachedIconIndex(String ^path, bool istopfield, bool isfolder)
		{
			//istopfield=true;
			if (dic->ContainsKey(path))
				return dic[path];
			else
			{
				//if (isfolder) return folder_index; else return file_index;

				//if (fast_mode) return -1;

				FileType^ info;
				if (istopfield)
				{
					if (isfolder)
					{
						info=gcnew FileType(GetApproximateFolderIconIndex(path), this->folder_info->file_type);
					}
					else
						info=GetApproximateFileIconIndex(path);
				}
				else
					info=GetFileIconIndex( path);
			

				if (path->EndsWith(".rec",StringComparison::InvariantCultureIgnoreCase)) 
				{
					if(info->icon_index==file_index)  info=play_info;

					info->file_type = "REC File";
				}


				if (info->icon_index>=0) 
				{
					dic->Add(path, info);

					String^ ext = "";
					try {ext = Path::GetExtension(path);} catch(...){};


					if (!isfolder && ext->Length  && !extension_dic->ContainsKey(ext))
					{
						extension_dic->Add(ext, info);
						//Console::WriteLine("added ext,ic = " + ext + ", "+ic.ToString()+" to dic.");
					}


				}

				return info;
			}
		}



		static FileType^ GetFileIconIndex(String ^ path)
		{
			SHFILEINFO shinfo;
			FileType^ info;
			wchar_t* str = (wchar_t*)(void*)Marshal::StringToHGlobalUni(path);
			DWORD_PTR ind;
			ind = Antares::SHGetFileInfo( str, 0, &shinfo, sizeof(shinfo), 0*SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME  );

			//if (ind==file_index && path->EndsWith(".rec",StringComparison::InvariantCultureIgnoreCase)) ind=play_index;

			info = gcnew FileType(shinfo.iIcon, gcnew String(shinfo.szTypeName) );
			return info; //shinfo.iIcon;

		}



		static FileType^ GetApproximateFileIconIndex(String ^ path)
		{
			SHFILEINFO shinfo;
			FileType^ info;
			wchar_t* str = (wchar_t*)(void*)Marshal::StringToHGlobalUni(path);
			DWORD_PTR ind;
			ind = Antares::SHGetFileInfo( str, FILE_ATTRIBUTE_NORMAL, &shinfo, sizeof(shinfo), 0*SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON | SHGFI_TYPENAME );

			//if (ind==file_index && path->EndsWith(".rec",StringComparison::InvariantCultureIgnoreCase)) ind=play_index;

			info = gcnew FileType(shinfo.iIcon, gcnew String(shinfo.szTypeName));
			return info;//shinfo.iIcon;

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

		void update(FileItem^ item)
		{
			this->filename=item->filename;
			this->directory=item->directory;
			this->safe_filename=item->safe_filename;
			this->datetime=item->datetime;
			this->datestring=item->datestring;
			this->size=item->size;
			//this->file_type=item->file_type;


			for (int j=0; j<5; j++)
			{
				this->SubItems[j]->Text = item->SubItems[j]->Text;
			}
		}
		
		void update_program_information(void)
		{
			this->SubItems[4]->Text = this->channel;
			this->SubItems[5]->Text = this->description;
		}
		void update_icon(void)
		{
			this->ImageIndex = this->icon_index;
			this->SubItems[2]->Text=this->file_type;
		}


		System::String^ filename;
		System::String^ directory;
		System::String^ safe_filename;
		System::String^ datestring;
		System::String^ full_filename;
		System::String^ recursion_offset;

		System::String^ file_type;

		System::String^ channel;
		System::String^ description;

		System::DateTime datetime;
		char type;
		long long int size;
		bool isdir;
		bool isdrive;
		int icon_index;



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
			this->Name = namestring;
			this->recursion_offset="";
			this->isdrive = true;
			this->channel="";
			this->description="";
			this->file_type="";

		}

		ComputerItem(String^ path, String^ dir) :  FileItem()
		{

			FileInfo^ f = gcnew FileInfo(path);   //Todo: handle exceptions
			FileAttributes attr = f->Attributes::get();
			this->filename = Path::GetFileName(path);
			this->isdrive = false;
			String^ sizestring="";
			this->recursion_offset="";
			this->channel="";
			this->description="";
			this->directory = dir;
			if ( (attr & FileAttributes::Directory) == FileAttributes::Directory)
			{
				this->isdir=true;
				this->file_type = "Folder";
				this->size=0;
				//this->ImageIndex = 0;
			}
			else
			{
				this->isdir = false;
				this->file_type="File";
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
			this->Name=path;
			this->SubItems->Add( sizestring );
			this->SubItems->Add(this->file_type);
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
			//String ^typestring = gcnew String("");
			switch (entry->filetype)
			{
			case 1:
				this->type = 'd';
				this->file_type = "Folder";
				this->isdir = true;
				//this->ImageIndex = 0;
				break;

			case 2:
				this->type = 'f';
				this->file_type = "File";
				//if (namestring->EndsWith(".rec"))
				//	this->ImageIndex=2;
				//else
				//	this->ImageIndex = 1;
				this->isdir=false;
				break;

			default:

				this->type= '?';
				this->file_type="??";
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
			this->Name=this->full_filename;
			//DateTime^ dt = DateTime::FromFileTimeUtc(timestamp * 10000000LL + 116444736000000000LL);



			//printf("Seconds = %d \n",timestamp);
			//String ^datestring = gcnew String( dt->ToString());

			//item = (gcnew ListViewItem(namestring  ,0));   //entries[i].name 
			this->Text = namestring;

			this->SubItems->Add( sizestring );
			this->SubItems->Add(this->file_type);
			this->SubItems->Add(this->datestring);
			this->SubItems->Add(this->channel);
			this->SubItems->Add(this->description);


		}



	};


	public ref class CachedProgramInformation
	{
	public:
		String^ channel;
		String^ description;
		CachedProgramInformation(String^ ch, String ^ desc)
		{
			this->channel = ch;
			this->description=desc;
		}

		void apply_to_item(FileItem^ item)
		{
			item->channel = this->channel;
			item->description = this->description;
			item->SubItems[4]->Text = item->channel;
			item->SubItems[5]->Text = item->description;
		}
	};

	public ref class ProgramInformationCache
	{
	public:
		Dictionary<String^,  CachedProgramInformation^> ^dic;

		String^ dic_key(FileItem^ item)
		{
			return item->full_filename + item->size.ToString();
		}

		ProgramInformationCache(void)
		{
			dic = gcnew Dictionary<String^, CachedProgramInformation^>;
		}
		CachedProgramInformation^ query(FileItem^ item)
		{
			String^ key = dic_key(item);
			if (dic->ContainsKey(key))
				return dic[key];

			return nullptr;
		}

		void add(FileItem^ item)
		{
			String^ key = dic_key(item);
			CachedProgramInformation^ pi = gcnew CachedProgramInformation(item->channel, item->description);
			dic[key]=pi;

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
			if (this->col == 2) return String::Compare(fx->file_type, fy->file_type);

			// Sort by date
			if (this->col == 3) return String::Compare(fx->datestring, fy->datestring);

			if (this->col == 4) return String::Compare(fx->channel, fy->channel);

			if (this->col == 5) return String::Compare(fx->description, fy->description);

			return 1;


		}
	};


}