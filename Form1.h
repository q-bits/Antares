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
#include "windows.h"
#include "commctrl.h"
#include <time.h>
#include "FBLib_rec.h"

}

#using <mscorlib.dll>



extern FILE old_stdout;
extern FILE old_stderr;
extern FILE* hf;


#include "copydialog.h"
#include "deleteconfirmation.h"
#include "overwriteconfirmation.h"
#include "ProgInfo.h"
#include "Settings.h"

//ref class CopyDialog ;

#include "antares.h"



#define EPROTO 1

namespace Antares {



	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Text;
	using namespace System::IO;
	using namespace System::Threading;
	using namespace System::Runtime::InteropServices; // for class Marshal


	delegate void ListViewSelectionDelegate(void);


	//TODO: put these prototypes somewhere better
	System::String^ HumanReadableSize(__u64 size);
	System::String^ DateString(time_t time);
	System::String^ safeString( char* filename );
	System::String^ safeString( String^ filename_str );
	time_t DateTimeToTime_T(System::DateTime datetime);
	String^ ComputeTopfieldUpDir(String^ filename);
	array<String^>^ TopfieldFileParts(String^ filename);

	[DllImport("user32.dll", EntryPoint = "SendMessage", CharSet = CharSet::Auto)]
	LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);




	[DllImport("user32.dll")]
	BOOL WINAPI DestroyIcon(HICON hIcon);



	enum
	{
		OVERWRITE,
		SKIP,
		RESUME
	} overwrite_action_type;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>



	public ref class Form1 : public System::Windows::Forms::Form
	{

	public: 



		void setListViewStyle(ListView^ listview)
		{
			HWND window_pointer = (HWND) listview->Handle.ToPointer();
			Antares::SendMessage(window_pointer ,  LVM_SETIMAGELIST, LVSIL_SMALL, this->icons->imagelist);
			LRESULT styles = Antares::SendMessage(window_pointer, (int) LVM_GETEXTENDEDLISTVIEWSTYLE, 0,0);
			styles |= LVS_EX_DOUBLEBUFFER ;
			Antares::SendMessage(window_pointer, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (int) styles);
		}


		static void SuspendDrawing( Control^ parent ) 
		{ 
			Antares::SendMessage((HWND) parent->Handle.ToPointer(), WM_SETREDRAW, false, 0); 
		} 

		static void ResumeDrawing( Control^ parent ) 
		{ 
			Antares::SendMessage((HWND) parent->Handle.ToPointer(), WM_SETREDRAW, true, 0); 
			//parent->Refresh(); 
		} 

	protected:
		virtual void WndProc( Message% m ) override
		{

			// Listen for operating system messages.
			switch ( m.Msg )
			{
			case WM_ACTIVATEAPP:

				break;
			case WM_DEVICECHANGE:

				if (!this->transfer_in_progress)
				{
					printf("WM_DEVICECHANGE received. m.WParam =%d   m.LParam=%d \n",m.WParam,m.LParam);
					this->CheckConnection();
				}
			}

			Form::WndProc( m );
		}



	public:
		Form1(void)
		{

			icons = gcnew Antares::Icons();

			this->listView1SortColumn = -1;
			this->listView2SortColumn=-1;
			this->turbo_mode = gcnew System::Boolean;

			this->finished_constructing = 0;
			this->last_layout_x=-1;
			this->last_layout_y=-1;
			InitializeComponent();

			this->setTopfieldDir("\\DataFiles\\");
			this->setComputerDir("C:\\");

			listView2_selection_was_changed=false; 
			listView1_selection_was_changed=false;

			//this->SetStyle(ControlStyles::AllPaintingInWmPaint | ControlStyles::UserPaint | ControlStyles::DoubleBuffer,true);

			this->transfer_in_progress=false;

			this->dircount=0;
			this->fd  = NULL;//connect_device2(&reason);
			//if (this->fd==NULL) this->label2->Text="PVR: Device not connected";

			this->topfieldNameHeader = this->listView1->Columns->Add("Name",140,HorizontalAlignment::Left);
			this->topfieldSizeHeader = this->listView1->Columns->Add("Size",70,HorizontalAlignment::Right);
			this->topfieldTypeHeader = this->listView1->Columns->Add("Type",60,HorizontalAlignment::Left);
			this->topfieldDateHeader = this->listView1->Columns->Add("Date",120,HorizontalAlignment::Left);


			this->computerNameHeader = this->listView2->Columns->Add("Name",140,HorizontalAlignment::Left);
			this->computerSizeHeader = this->listView2->Columns->Add("Size",70,HorizontalAlignment::Right);
			this->computerTypeHeader = this->listView2->Columns->Add("Type",60,HorizontalAlignment::Left);
			this->computerDateHeader = this->listView2->Columns->Add("Date",120,HorizontalAlignment::Left);


			//this->basicIconsSmall = gcnew ImageList();
			//this->basicIconsSmall->Images->Add( Bitmap::FromFile( "folder.bmp" ) );
			//this->basicIconsSmall->Images->Add( Bitmap::FromFile( "document.bmp" ) );
			//this->basicIconsSmall->Images->Add( Bitmap::FromFile( "rec_file.bmp" ) );
			//this->listView1->SmallImageList = this->basicIconsSmall;
			//this->listView2->SmallImageList = this->basicIconsSmall;


			// Load configuration. 
			this->settings = gcnew Settings();

			if (String::Compare("on",settings["TurboMode"])==0) this->checkBox1->Checked = true; else this->checkBox1->Checked = false;

			this->setTopfieldDir(settings["TopfieldDir"]);
			this->setComputerDir(settings["ComputerDir"]);
			this->listView1SortColumn = System::Convert::ToInt32(settings["PVR_SortColumn"]);
			this->listView2SortColumn = System::Convert::ToInt32(settings["PC_SortColumn"]);
			if (String::Equals(settings["PVR_SortOrder"], "Ascending"))  
			{
				this->listView1->Sorting = SortOrder::Ascending;

			}
			else
			{
				this->listView1->Sorting = SortOrder::Descending;
			}

			if (String::Equals(settings["PC_SortOrder"], "Ascending"))  
			{
				this->listView2->Sorting = SortOrder::Ascending;
			}

			else
			{
				this->listView2->Sorting = SortOrder::Descending;
			}

			this->listView2->ListViewItemSorter = gcnew ListViewItemComparer(this->listView2SortColumn,this->listView2->Sorting);
			this->listView1->ListViewItemSorter = gcnew ListViewItemComparer(this->listView1SortColumn,this->listView1->Sorting);




			this->TopfieldClipboard = gcnew array<String^> {};
			this->TopfieldClipboardDirectory = "";

			this->finished_constructing = 1;

			this->loadTopfieldDir();
			this->loadComputerDir();
			//this->ResizeRedraw = true;





			this->CheckConnection();
			this->last_layout_x = -1;this->last_layout_y=-1;
			this->Arrange();

			// Set double-buffering and image list on the ListViews
			this->setListViewStyle(listView1);
			this->setListViewStyle(listView2);

		}




		void absorb_late_packets(int count, int timeout)
		{
			int r;
			struct tf_packet reply;
			if (this->fd==NULL) return;
			printf("\nLate Packets:\n");
			for (int i=0; i<count; i++)

			{
				r = get_tf_packet2(this->fd, &reply,timeout);
				printf("r=%d   reply.cmd = %d \n",r,get_u32(&reply.cmd));

			}
		}



		void CheckConnection(void)
		{
			libusb_device_handle* dh;
			int success;
			//struct libusb_bus * bus;
			struct libusb_device *dev, *device;
			int i;
			int r;
			int cnt;
			libusb_device **devs;
			struct libusb_device_descriptor desc;

			cnt = libusb_get_device_list(NULL, &devs);
			dh = NULL;
			device = NULL;
			success = 0;
			bool threadsafe = ! this->InvokeRequired;
			for (i=0; i<cnt; i++)
			{
				dev=devs[i];
				r = libusb_get_device_descriptor(dev, &desc);

				if (desc.idVendor==0x11db && desc.idProduct == 0x1000)
				{
					device=dev;
				}
				else {continue;};

				if (this->fd != NULL) break;

				r=libusb_open(device,&dh);
				if (r) {
					fprintf(stderr,"New open call failed");

					continue;
				}

				// Select configuration 0x01
				if (libusb_set_configuration(dh, 0x01))
				{
					fprintf(stderr, "connect: Select configuration failed\n");

					continue;
				}

				// Claim interface 0x00
				if (libusb_claim_interface(dh, 0x00))
				{
					fprintf(stderr, "connect: Claim interface failed\n");

					continue;
				}
				success=1; break;

			}

			if (this->fd != NULL && device==NULL)    // Topfield has apparently been disconnected. 
			{
				libusb_close(this->fd);
				this->fd=NULL;
				libusb_free_device_list(devs, 1);
				//printf("Topfield is now disconnected.\n");
				if (threadsafe)
				{
					this->label2->Text = "PVR: Device not connected";
					this->listView1->Items->Clear();
				}
				return;
			}


			if (this->fd !=NULL && device!=NULL)  // Topfield is apparently still connected
			{
				libusb_free_device_list(devs, 1);
				//printf("Topfield is still connected.\n");
				return;
			}


			if (this->fd == NULL && device==NULL)  // Topfield is apparently still disconnected
			{
				libusb_free_device_list(devs, 1);
				//printf("Topfield is still disconnected.\n");
				if (threadsafe) this->label2->Text = "PVR: Device not connected";
				return;
			}


			if (!success && dh)
			{
				libusb_close(dh);
				this->fd=NULL;
				libusb_free_device_list(devs, 1);
				printf("Topfield is connected, but could not be opened.\n");
				return;
			}

			this->fd=dh;
			printf("Topfield is now connected.\n");
			libusb_free_device_list(devs, 1);
			if (threadsafe) this->loadTopfieldDir();
			return;
		}


		int tf_init(void)
		{
			if (this->fd==NULL) return -1;

			this->absorb_late_packets(2,100);
			/* Send a cancel in case anything is outstanding */
			if (do_cancel(this->fd) == 0) {
				return 0;
			}

			/* Often the first cancel fails, so try again */
			if (do_cancel(this->fd) == 0) {
				return 0;
			}

			/* If that fails, ... */
			/* First send a fail */
			//tf_send_fail(tf, 2);

			/* Now a success */
			send_success(this->fd);

			/* Try a ready command */
			if (do_cmd_ready(this->fd) == 0) {
				return 0;
			}

			/* And finally, one more cancel */
			if (do_cancel(this->fd) == 0) {
				return 0;
			}

			/* Give up */
			return -1;
		}



		int wait_for_connection(Antares::CopyDialog^ copydialog)
		{

			copydialog->current_file = " * * * ERROR. Retrying... * * * ";
			copydialog->reset_rate();
			copydialog->update_dialog_threadsafe();
			int r,ret;

			while(1)
			{
				this->absorb_late_packets(4,100);
				r=this->tf_init();

				if (r==0) {ret=0;break;};
				if (copydialog->cancelled) {ret=-1;break;};
				//System::Threading::Thread::Sleep(1000);
				this->CheckConnection();
				if (copydialog->cancelled) {ret=-1;break;};

			}

			if (ret==0)
				this->set_turbo_mode(*this->turbo_mode);

			return ret;
		}

		int set_turbo_mode(int turbo_on)
		{

			//return 0;
			//turbo_on=0;
			int r;
			printf("\nSetting turbo mode: %d\n",turbo_on);  
			struct tf_packet reply;

			if (this->fd == NULL) return -1;

			//this->absorb_late_packets(2,100);
			r = send_cmd_turbo(fd, turbo_on);
			if(r < 0)
			{
				this->absorb_late_packets(2,100);
				return -EPROTO;
			}

			r = get_tf_packet(this->fd, &reply);
			if(r < 0)
			{
				this->absorb_late_packets(2,100);
				return -EPROTO;
			}

			switch (get_u32(&reply.cmd))
			{
			case SUCCESS:
				trace(1,
					fprintf(stderr, "Turbo mode: %s\n",
					turbo_on ? "ON" : "OFF"));
				this->absorb_late_packets(2,100);
				*this->turbo_mode = (turbo_on!=0);
				return 0;
				break;

			case FAIL:
				fprintf(stderr, "ERROR: Device reports %s\n",
					decode_error(&reply));
				break;

			default:
				fprintf(stderr, "ERROR: Unhandled packet (in set_turbo_mode) cmd=%d\n",&reply.cmd);
			}
			this->absorb_late_packets(2,100);
			return -EPROTO;
		}


		// Todo: add some sanity checking here
		void setComputerDir(String ^directory)
		{

			if (directory->Length > 3 && directory->EndsWith("\\"))
			{
				directory = directory->Substring(0,directory->Length - 1);

			}
			this->computerCurrentDirectory = directory;
			this->textBox1->Text = this->computerCurrentDirectory;


		};

		void setTopfieldDir(String ^directory)
		{
			if (directory->EndsWith("\\"))
			{
				directory = directory->Substring(0,directory->Length - 1);

			}
			this->topfieldCurrentDirectory=directory;
			this->textBox2->Text = this->topfieldCurrentDirectory;
		};


		array<ComputerItem^>^ loadComputerDirArray(String^ dir)
		{
			array<String^>^ list;
			int j;



			list = System::IO::Directory::GetFileSystemEntries(dir);

			array<ComputerItem^>^ items = gcnew array<ComputerItem^>(list->Length);
			for (j=0; j<list->Length; j++)
			{

				items[j] = gcnew ComputerItem(list[j], dir);
			}
			return items;
		}



		// Load and display files in the current computer directory.
		// If a file is named start_rename, then start the name editing process after the directory is loaded.
		// (useful when we have just created a new folder).
		void loadComputerDir(String^ start_rename, String^ name_to_select)
		{

			int j;
			ComputerItem^ item;
			array<ComputerItem^>^ items = {};
			Console::WriteLine("Load computer dir");


			String^ dir = this->computerCurrentDirectory;
			ComputerItem^ rename_item; bool do_rename=false;
			ComputerItem^ select_item; bool do_select=false;

			if (dir->Equals(""))  // List drives
			{
				this->listView2->Items->Clear();
				DWORD drives = GetLogicalDrives();
				for (j=0; j<26; j++)
				{
					if ( (drives&1)==1)
					{
						item = gcnew ComputerItem(j);
						int ic = this->icons->GetCachedIconIndex(item->full_filename);
						if (ic>=0)
							item->ImageIndex=ic;
						else
							item->ImageIndex=this->icons->folder_index;
						this->listView2->Items->Add(item);
					}

					drives>>=1;
				}
				this->label1->Text = "My Computer";
				settings->changeSetting("ComputerDir","");
			}
			else   //List contents of actual directory
			{

				try{
					items = this->loadComputerDirArray(dir);
				}
				catch(System::IO::IOException ^)
				{
					this->setComputerDir("");
					this->loadComputerDir();
					Console::WriteLine("Unhandled exception in loadComputerDir");
					return;
				}
				catch(System::UnauthorizedAccessException ^)
				{
					toolStripStatusLabel1->Text="Access denied: "+dir;
					this->computerUpDir();
					return;

				}

				for (j=0; j<items->Length; j++)
				{
					item = items[j];
					int ic = this->icons->GetCachedIconIndex(item->full_filename);
					if (ic >= 0)
					{
						item->ImageIndex = ic;
					}
					else
					{
						item->ImageIndex = this->icons->file_index;
					}

					if (String::Compare(item->filename, start_rename)==0)
					{
						rename_item=item; do_rename=true;
					}

					if (String::Compare(item->filename, name_to_select)==0)
					{
						select_item = item; do_select=true;
					}

				}
				this->listView2->BeginUpdate();
				this->listView2->Items->Clear();
				this->listView2->Items->AddRange(items);
				this->listView2->EndUpdate();
				settings->changeSetting("ComputerDir",dir);
				// Add a drive summary to label1:
				String^ str = Path::GetPathRoot(dir);
				if (str->Length > 0)
				{
					DriveInfo^ drive = gcnew DriveInfo(str);
					str = " Local Disk "+str+"  --  " + HumanReadableSize(drive->AvailableFreeSpace) + " Free / " + HumanReadableSize(drive->TotalSize)+ " Total";
					//str = " Local Disk "+str+"  --  " + HumanReadableSize(4e12) + " Free / " + HumanReadableSize(4e12)+ " Total";

					label1->Text = str;

				}
				else
				{
					label1->Text = "";
				}
				if (do_rename) rename_item->BeginEdit();
				else if (do_select)
				{
					select_item->Selected=true;
					select_item->Focused=true;
					select_item->EnsureVisible();
				}
				else
				{
					ListView::ListViewItemCollection^ q = this->listView2->Items; 
					if (q->Count>0) {q[0]->Selected=true;q[0]->Focused=true;};
				}

			}

		};

		void loadComputerDir(String^ start_rename)
		{
			this->loadComputerDir(start_rename,"");
		}

		void loadComputerDir(void)
		{
			this->loadComputerDir("","");
		}

		void computerUpDir(void)
		{
			String^ dir = this->computerCurrentDirectory;
			String^ dir2;
			String^ fn;
			dir2="";
			try{
				dir2 = Path::GetDirectoryName(dir);
				fn = Path::GetFileName(dir);
			}
			catch (System::ArgumentException^ )
			{
				dir2="";
				Console::WriteLine("Handled exception in computerUpDir");
			}
			//if (dir2==NULL) dir2="";


			try
			{
				dir2=dir2+"";
				fn=fn+"";
			}
			catch(System::NullReferenceException^ )
			{
				dir2="";
				Console::WriteLine("Handled exception in computerUpDir");
			}

			this->setComputerDir(dir2);
			this->loadComputerDir("",fn);
		}

		void topfieldUpDir(void)
		{
			if (this->fd == NULL) return;

			array<String^>^ parts = TopfieldFileParts(this->topfieldCurrentDirectory);

			this->setTopfieldDir(parts[0]);

			this->loadTopfieldDir("",parts[1]);
		}



		// Load the specified topfield directory into an array of TopfieldItems
		array<TopfieldItem^>^ loadTopfieldDirArray(String^ path)               
		{
			//Console::WriteLine("Loading directory: "+path);
			tf_packet reply;

			__u16 count;
			int i,j;

			struct typefile *entries;


			TopfieldItem^ item;
			array<TopfieldItem^>^ items = {};

			if (this->fd==NULL)
			{
				//toolStripStatusLabel1->Text="Topfield not connected.";
				return items; 
			}

			char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(path);

			send_cmd_hdd_dir(this->fd,str2);
			Marshal::FreeHGlobal((System::IntPtr)(void*)str2);

			j=0;
			int numitems=0;
			while(0 < get_tf_packet(this->fd, &reply))
			{
				j=j+1; 
				switch (get_u32(&reply.cmd))
				{
				case DATA_HDD_DIR:
					count =
						(get_u16(&reply.length) - PACKET_HEAD_SIZE) / sizeof(struct typefile);
					entries = (struct typefile *) reply.data;
					Array::Resize(items, items->Length + count);
					for(i = 0; (i < count); i++)
					{
						item = gcnew TopfieldItem(&entries[i],path);
						item->directory = path;
						if (String::Compare(item->filename,"..")!=0 ) 
						{
							items[numitems] = item;
							numitems++;
						}
					}

					send_success(this->fd);
					break;

				case DATA_HDD_DIR_END:
					Array::Resize(items,numitems);
					return items;
					break;

				case FAIL:
					fprintf(stderr, "ERROR: Device reports %s\n",
						decode_error(&reply));
					return items;
					break;
				default:
					fprintf(stderr, "ERROR: Unhandled packet\n");
					return items;
				}

			}
			return items;
		}


		// Reload the directory entry of a particular topfield item specified by filename (i.e., to retrieve an update on its size)
		TopfieldItem^ reloadTopfieldItem(String^ filename)
		{
			TopfieldItem^ newitem = nullptr;
			array<String^>^ fileparts = TopfieldFileParts(filename);
			String^ dir = fileparts[0];
			String^ file = fileparts[1];
			array<TopfieldItem^>^ items = this->loadTopfieldDirArray(dir);
			for each (TopfieldItem^ item in items)
			{
				if ( String::Compare(item->filename, file)==0)
				{
					newitem=item;
					break;
				}
			}
			return newitem;
		}


		// Query the Topfield for its free space and total size
		TopfieldFreeSpace getTopfieldFreeSpace(void)
		{

			//bool hdd_size_successful = false;
			//unsigned int totalk, freek;
			int r;
			tf_packet reply;
			TopfieldFreeSpace v;

			v.valid = false;
			r = send_cmd_hdd_size(fd);
			if(r < 0)
			{
				return v;
			}

			r = get_tf_packet(fd, &reply);
			if(r < 0)
			{
				return v;
			}

			switch (get_u32(&reply.cmd))
			{
			case DATA_HDD_SIZE:
				{
					v.totalk = get_u32(&reply.data);
					v.freek = get_u32(&reply.data[4]);

					v.valid=true;
					break;
				}

			case FAIL:
				fprintf(stderr, "ERROR: Device reports %s\n",
					decode_error(&reply));
				break;

			default:
				fprintf(stderr, "ERROR: Unhandled packet in load_topfield_dir/hdd_size\n");
			}
			return v;
		}

		// Load and display files in the current topfield directory.
		// If a file is named start_rename, then start the name editing process after the directory is loaded.
		// (useful when we have just created a new folder).
		int loadTopfieldDir(String^ start_rename, String^ name_to_select)               
		{

			int i;
			TopfieldItem^ item, ^rename_item;bool do_rename=false;
			TopfieldItem^ select_item; bool do_select=false;
			array<TopfieldItem^>^ items;



			if (this->fd==NULL)
			{
				//toolStripStatusLabel1->Text="Topfield not connected.";
				return -EPROTO; 
			}


			TopfieldFreeSpace v = getTopfieldFreeSpace();

			this->label2->Text = " Topfield device  --  "+HumanReadableSize(1024* ((__u64) v.freek))+" free / " + HumanReadableSize(1024*( (__u64) v.totalk)) + " total";

			///// Actually load the directory
			items = this->loadTopfieldDirArray(this->topfieldCurrentDirectory);     //TODO: handle errors in directory load
			/////

			for(i = 0; i < items->Length ; i++)
			{


				item = items[i];
				if (item->isdir)
					item->ImageIndex = this->icons->folder_index;
				else
					item->ImageIndex = this->icons->GetCachedIconIndex(item->filename, true);

				if (String::Equals(start_rename,item->filename) && !String::Equals(start_rename,"") )
				{
					rename_item=item; do_rename=true;
				}

				if (String::Compare(item->filename, name_to_select)==0 && !String::Equals(name_to_select,"") )
				{
					select_item = item; do_select=true;
				}

				if (String::Compare(this->topfieldCurrentDirectory, this->TopfieldClipboardDirectory)==0)
				{
					int numc = this->TopfieldClipboard->Length;
					if (numc>0)
					{
						bool iscut=false;
						for (int ii=0; ii<numc; ii++)
							if (String::Compare(item->filename, this->TopfieldClipboard[ii])==0)
							{iscut=true; break;};
						if (iscut) item->BackColor = this->cut_background_colour;
					}

				}
			}



			//if (items->Length > 0)  
			settings->changeSetting("TopfieldDir",this->topfieldCurrentDirectory);//TODO: don't do this when error in directory load

			this->listView1->BeginUpdate();
			this->listView1->Items->Clear();
			this->listView1->Items->AddRange(items);
			this->listView1->EndUpdate();
			if (do_rename) rename_item->BeginEdit();
			else if (do_select)
			{
				select_item->Selected=true;
				select_item->Focused=true;
				select_item->EnsureVisible();
			}
			else
			{
				ListView::ListViewItemCollection^ q = this->listView1->Items; 
				if (q->Count>0) {q[0]->Selected=true;q[0]->Focused=true;};
			}

			return 0;
		}

		int loadTopfieldDir(void)
		{
			String^ start_rename = "";
			String^ name_to_select = "";
			return this->loadTopfieldDir(start_rename, name_to_select);
		}
		int loadTopfieldDir(String^ start_rename)
		{
			String^ name_to_select = "";
			return this->loadTopfieldDir(start_rename, name_to_select);
		}


	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
#ifdef _DEBUG____
			*stdout = old_stdout;
			*stderr = old_stderr;
			fclose(hf);
			FreeConsole();
#endif

			if (components)
			{
				delete components;
			}
		}


		//void showCopyDialog(void)   //DELETE?
		//{
		//ThreadStart^ threadDelegate = gcnew ThreadStart( copydialog, &CopyDialog::showDialog_);
		//Thread^ newThread = gcnew Thread( threadDelegate );
		//newThread->Start();
		//
		//		}



	public: libusb_device_handle *fd;
	public: int dircount;
			System::Windows::Forms::ColumnHeader^ topfieldNameHeader;
			System::Windows::Forms::ColumnHeader^ topfieldSizeHeader;
			System::Windows::Forms::ColumnHeader^ topfieldTypeHeader;
			System::Windows::Forms::ColumnHeader^ topfieldDateHeader;
			System::Windows::Forms::ColumnHeader^ computerNameHeader;
			System::Windows::Forms::ColumnHeader^ computerSizeHeader;
			System::Windows::Forms::ColumnHeader^ computerTypeHeader;
			System::Windows::Forms::ColumnHeader^ computerDateHeader;
			int finished_constructing;
			System::String^ topfieldCurrentDirectory;
			System::String^ computerCurrentDirectory;


			// "turbo_mode" is what we believe the actual current turbo mode of the PVR is
			// (which is not necessarily equal to the requested turbo mode setting)
			bool^ turbo_mode; 


			int listView1SortColumn;
			int listView2SortColumn;


			array<String^>^ TopfieldClipboard;  
			String^ TopfieldClipboardDirectory;

			static Color cut_background_colour = Color::FromArgb(255,250,105);
			static Color normal_background_colour = Color::FromArgb(255,255,255);


			int last_layout_x, last_layout_y;


			Settings^ settings;
			Antares::Icons^ icons;

			bool listView2_selection_was_changed, listView1_selection_was_changed;
			static const long long resume_granularity = 8192;
			bool transfer_in_progress;




	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::Panel^  panel3;
	private: System::Windows::Forms::CheckBox^  checkBox1;

	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::ToolStrip^  toolStrip2;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton5;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton6;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton7;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton8;
	public: System::Windows::Forms::ListView^  listView1;
	private: 

	private: System::Windows::Forms::Panel^  panel2;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Panel^  panel4;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::ToolStrip^  toolStrip1;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton1;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton2;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton3;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton4;
	public: System::Windows::Forms::ListView^  listView2;
	private: 

	private: System::Windows::Forms::Timer^  timer1;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel1;
	private: System::Windows::Forms::ImageList^  basicIconsSmall;




	private: System::Windows::Forms::ToolStripButton^  toolStripButton9;

	private: System::Windows::Forms::ToolStripButton^  toolStripButton10;
	private: System::Windows::Forms::Panel^  panel5;
	public: System::Windows::Forms::TextBox^  textBox2;
	private: 

	private: System::Windows::Forms::Panel^  panel6;
	private: System::Windows::Forms::TextBox^  textBox1;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton11;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator3;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton12;





	private: System::ComponentModel::IContainer^  components;


	protected: 

	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusLabel1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->panel3 = (gcnew System::Windows::Forms::Panel());
			this->panel5 = (gcnew System::Windows::Forms::Panel());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->toolStrip2 = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripButton5 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton6 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton7 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton8 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->toolStripButton9 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton10 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->toolStripButton11 = (gcnew System::Windows::Forms::ToolStripButton());
			this->listView1 = (gcnew System::Windows::Forms::ListView());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->panel4 = (gcnew System::Windows::Forms::Panel());
			this->panel6 = (gcnew System::Windows::Forms::Panel());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripButton1 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton2 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton3 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton4 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator3 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->toolStripButton12 = (gcnew System::Windows::Forms::ToolStripButton());
			this->listView2 = (gcnew System::Windows::Forms::ListView());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->basicIconsSmall = (gcnew System::Windows::Forms::ImageList(this->components));
			this->statusStrip1->SuspendLayout();
			this->panel1->SuspendLayout();
			this->panel3->SuspendLayout();
			this->panel5->SuspendLayout();
			this->toolStrip2->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel4->SuspendLayout();
			this->panel6->SuspendLayout();
			this->toolStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip1
			// 
			this->statusStrip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->toolStripStatusLabel1});
			this->statusStrip1->Location = System::Drawing::Point(0, 638);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(880, 22);
			this->statusStrip1->TabIndex = 9;
			this->statusStrip1->Text = L"Test";
			// 
			// toolStripStatusLabel1
			// 
			this->toolStripStatusLabel1->Name = L"toolStripStatusLabel1";
			this->toolStripStatusLabel1->Size = System::Drawing::Size(0, 17);
			// 
			// panel1
			// 
			this->panel1->Controls->Add(this->panel3);
			this->panel1->Controls->Add(this->panel2);
			this->panel1->Controls->Add(this->panel4);
			this->panel1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->panel1->Location = System::Drawing::Point(0, 0);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(880, 638);
			this->panel1->TabIndex = 10;
			// 
			// panel3
			// 
			this->panel3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel3->Controls->Add(this->panel5);
			this->panel3->Controls->Add(this->checkBox1);
			this->panel3->Controls->Add(this->label2);
			this->panel3->Controls->Add(this->toolStrip2);
			this->panel3->Controls->Add(this->listView1);
			this->panel3->Dock = System::Windows::Forms::DockStyle::Left;
			this->panel3->Location = System::Drawing::Point(0, 0);
			this->panel3->Margin = System::Windows::Forms::Padding(0, 3, 0, 3);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(495, 638);
			this->panel3->TabIndex = 8;
			// 
			// panel5
			// 
			this->panel5->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel5->Controls->Add(this->textBox2);
			this->panel5->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel5->Location = System::Drawing::Point(0, 62);
			this->panel5->Name = L"panel5";
			this->panel5->Size = System::Drawing::Size(495, 32);
			this->panel5->TabIndex = 8;
			// 
			// textBox2
			// 
			this->textBox2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->textBox2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->textBox2->Font = (gcnew System::Drawing::Font(L"Lucida Console", 10));
			this->textBox2->ForeColor = System::Drawing::Color::Navy;
			this->textBox2->Location = System::Drawing::Point(9, 7);
			this->textBox2->Margin = System::Windows::Forms::Padding(90);
			this->textBox2->Multiline = true;
			this->textBox2->Name = L"textBox2";
			this->textBox2->Size = System::Drawing::Size(483, 19);
			this->textBox2->TabIndex = 7;
			this->textBox2->Text = L"\\ProgramFiles";
			// 
			// checkBox1
			// 
			this->checkBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->checkBox1->AutoSize = true;
			this->checkBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->checkBox1->Location = System::Drawing::Point(410, 11);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 7;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = false;
			this->checkBox1->CheckedChanged += gcnew System::EventHandler(this, &Form1::checkBox1_CheckedChanged);
			// 
			// label2
			// 
			this->label2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(240)));
			this->label2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label2->Dock = System::Windows::Forms::DockStyle::Top;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(0, 38);
			this->label2->Margin = System::Windows::Forms::Padding(5);
			this->label2->Name = L"label2";
			this->label2->Padding = System::Windows::Forms::Padding(5, 0, 0, 0);
			this->label2->Size = System::Drawing::Size(495, 24);
			this->label2->TabIndex = 5;
			this->label2->Text = L"label2";
			this->label2->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStrip2
			// 
			this->toolStrip2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->toolStrip2->GripMargin = System::Windows::Forms::Padding(1);
			this->toolStrip2->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(9) {this->toolStripButton5, 
				this->toolStripButton6, this->toolStripButton7, this->toolStripButton8, this->toolStripSeparator1, this->toolStripButton9, this->toolStripButton10, 
				this->toolStripSeparator2, this->toolStripButton11});
			this->toolStrip2->LayoutStyle = System::Windows::Forms::ToolStripLayoutStyle::HorizontalStackWithOverflow;
			this->toolStrip2->Location = System::Drawing::Point(0, 0);
			this->toolStrip2->Name = L"toolStrip2";
			this->toolStrip2->Padding = System::Windows::Forms::Padding(0, 0, 4, 0);
			this->toolStrip2->Size = System::Drawing::Size(495, 38);
			this->toolStrip2->TabIndex = 4;
			this->toolStrip2->Text = L"toolStrip2";
			// 
			// toolStripButton5
			// 
			this->toolStripButton5->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton5->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton5.Image")));
			this->toolStripButton5->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton5->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton5->Margin = System::Windows::Forms::Padding(8, 1, 4, 2);
			this->toolStripButton5->Name = L"toolStripButton5";
			this->toolStripButton5->Size = System::Drawing::Size(26, 35);
			this->toolStripButton5->Text = L"Up";
			this->toolStripButton5->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton5->ToolTipText = L"Up Folder";
			this->toolStripButton5->Click += gcnew System::EventHandler(this, &Form1::toolStripButton5_Click);
			// 
			// toolStripButton6
			// 
			this->toolStripButton6->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton6->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton6.Image")));
			this->toolStripButton6->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton6->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton6->Margin = System::Windows::Forms::Padding(2, 2, 4, 3);
			this->toolStripButton6->Name = L"toolStripButton6";
			this->toolStripButton6->Size = System::Drawing::Size(50, 33);
			this->toolStripButton6->Text = L"Refresh";
			this->toolStripButton6->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton6->Click += gcnew System::EventHandler(this, &Form1::toolStripButton6_Click);
			// 
			// toolStripButton7
			// 
			this->toolStripButton7->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton7->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton7.Image")));
			this->toolStripButton7->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton7->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton7->Margin = System::Windows::Forms::Padding(2, 1, 4, 2);
			this->toolStripButton7->Name = L"toolStripButton7";
			this->toolStripButton7->Size = System::Drawing::Size(44, 35);
			this->toolStripButton7->Text = L"Delete";
			this->toolStripButton7->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton7->Click += gcnew System::EventHandler(this, &Form1::toolStripButton7_Click);
			// 
			// toolStripButton8
			// 
			this->toolStripButton8->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton8->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton8.Image")));
			this->toolStripButton8->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton8->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton8->Margin = System::Windows::Forms::Padding(2, 1, 2, 2);
			this->toolStripButton8->Name = L"toolStripButton8";
			this->toolStripButton8->Size = System::Drawing::Size(34, 35);
			this->toolStripButton8->Text = L"New";
			this->toolStripButton8->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton8->ToolTipText = L"New Folder";
			this->toolStripButton8->Click += gcnew System::EventHandler(this, &Form1::toolStripButton8_Click);
			// 
			// toolStripSeparator1
			// 
			this->toolStripSeparator1->Name = L"toolStripSeparator1";
			this->toolStripSeparator1->Size = System::Drawing::Size(6, 38);
			// 
			// toolStripButton9
			// 
			this->toolStripButton9->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton9->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton9.Image")));
			this->toolStripButton9->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton9->Margin = System::Windows::Forms::Padding(2, 1, 2, 2);
			this->toolStripButton9->Name = L"toolStripButton9";
			this->toolStripButton9->Size = System::Drawing::Size(29, 35);
			this->toolStripButton9->Text = L"Cut";
			this->toolStripButton9->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton9->ToolTipText = L"Cut";
			this->toolStripButton9->Click += gcnew System::EventHandler(this, &Form1::toolStripButton9_Click);
			// 
			// toolStripButton10
			// 
			this->toolStripButton10->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton10->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton10.Image")));
			this->toolStripButton10->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton10->Margin = System::Windows::Forms::Padding(2, 1, 2, 2);
			this->toolStripButton10->Name = L"toolStripButton10";
			this->toolStripButton10->Size = System::Drawing::Size(38, 35);
			this->toolStripButton10->Text = L"Paste";
			this->toolStripButton10->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton10->ToolTipText = L"Paste";
			this->toolStripButton10->Click += gcnew System::EventHandler(this, &Form1::toolStripButton10_Click);
			// 
			// toolStripSeparator2
			// 
			this->toolStripSeparator2->Name = L"toolStripSeparator2";
			this->toolStripSeparator2->Size = System::Drawing::Size(6, 38);
			// 
			// toolStripButton11
			// 
			this->toolStripButton11->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton11->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton11.Image")));
			this->toolStripButton11->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton11->Margin = System::Windows::Forms::Padding(2, 1, 2, 2);
			this->toolStripButton11->Name = L"toolStripButton11";
			this->toolStripButton11->Size = System::Drawing::Size(35, 35);
			this->toolStripButton11->Text = L"Info.";
			this->toolStripButton11->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton11->ToolTipText = L"Show program info";
			this->toolStripButton11->Click += gcnew System::EventHandler(this, &Form1::Info_Click);
			// 
			// listView1
			// 
			this->listView1->AllowDrop = true;
			this->listView1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->listView1->FullRowSelect = true;
			this->listView1->GridLines = true;
			this->listView1->HideSelection = false;
			this->listView1->LabelEdit = true;
			this->listView1->Location = System::Drawing::Point(9, 93);
			this->listView1->Margin = System::Windows::Forms::Padding(0);
			this->listView1->Name = L"listView1";
			this->listView1->Size = System::Drawing::Size(483, 545);
			this->listView1->TabIndex = 0;
			this->listView1->UseCompatibleStateImageBehavior = false;
			this->listView1->View = System::Windows::Forms::View::Details;
			this->listView1->ItemActivate += gcnew System::EventHandler(this, &Form1::listView1_ItemActivate);
			this->listView1->AfterLabelEdit += gcnew System::Windows::Forms::LabelEditEventHandler(this, &Form1::listView_AfterLabelEdit);
			this->listView1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listView1_SelectedIndexChanged);
			this->listView1->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::listView1_Layout);
			this->listView1->ColumnClick += gcnew System::Windows::Forms::ColumnClickEventHandler(this, &Form1::listView_ColumnClick);
			this->listView1->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::listView_KeyDown);
			// 
			// panel2
			// 
			this->panel2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel2->Controls->Add(this->button2);
			this->panel2->Controls->Add(this->button1);
			this->panel2->Dock = System::Windows::Forms::DockStyle::Right;
			this->panel2->Location = System::Drawing::Point(495, 0);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(47, 638);
			this->panel2->TabIndex = 7;
			// 
			// button2
			// 
			this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button2->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->button2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"button2.Image")));
			this->button2->Location = System::Drawing::Point(6, 342);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(38, 37);
			this->button2->TabIndex = 2;
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button1->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->button1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"button1.Image")));
			this->button1->Location = System::Drawing::Point(6, 268);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(38, 37);
			this->button1->TabIndex = 1;
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// panel4
			// 
			this->panel4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel4->Controls->Add(this->panel6);
			this->panel4->Controls->Add(this->label1);
			this->panel4->Controls->Add(this->toolStrip1);
			this->panel4->Controls->Add(this->listView2);
			this->panel4->Dock = System::Windows::Forms::DockStyle::Right;
			this->panel4->Location = System::Drawing::Point(542, 0);
			this->panel4->Name = L"panel4";
			this->panel4->Size = System::Drawing::Size(338, 638);
			this->panel4->TabIndex = 6;
			// 
			// panel6
			// 
			this->panel6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel6->Controls->Add(this->textBox1);
			this->panel6->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel6->Location = System::Drawing::Point(0, 62);
			this->panel6->Name = L"panel6";
			this->panel6->Size = System::Drawing::Size(338, 32);
			this->panel6->TabIndex = 6;
			// 
			// textBox1
			// 
			this->textBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->textBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->textBox1->Font = (gcnew System::Drawing::Font(L"Lucida Console", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->textBox1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), 
				static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(128)));
			this->textBox1->Location = System::Drawing::Point(6, 7);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(320, 20);
			this->textBox1->TabIndex = 6;
			this->textBox1->Text = L"c:\\Topfield\\mp3";
			// 
			// label1
			// 
			this->label1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(240)));
			this->label1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label1->Dock = System::Windows::Forms::DockStyle::Top;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(0, 38);
			this->label1->Margin = System::Windows::Forms::Padding(5);
			this->label1->Name = L"label1";
			this->label1->Padding = System::Windows::Forms::Padding(5, 0, 0, 0);
			this->label1->Size = System::Drawing::Size(338, 24);
			this->label1->TabIndex = 4;
			this->label1->Text = L"label1";
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStrip1
			// 
			this->toolStrip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(6) {this->toolStripButton1, 
				this->toolStripButton2, this->toolStripButton3, this->toolStripButton4, this->toolStripSeparator3, this->toolStripButton12});
			this->toolStrip1->Location = System::Drawing::Point(0, 0);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->Padding = System::Windows::Forms::Padding(0, 0, 4, 0);
			this->toolStrip1->Size = System::Drawing::Size(338, 38);
			this->toolStrip1->TabIndex = 3;
			this->toolStrip1->Text = L"toolStrip1";
			// 
			// toolStripButton1
			// 
			this->toolStripButton1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton1.Image")));
			this->toolStripButton1->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton1->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton1->Margin = System::Windows::Forms::Padding(8, 1, 4, 2);
			this->toolStripButton1->Name = L"toolStripButton1";
			this->toolStripButton1->Size = System::Drawing::Size(26, 35);
			this->toolStripButton1->Text = L"Up";
			this->toolStripButton1->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton1->ToolTipText = L"Up Folder";
			this->toolStripButton1->Click += gcnew System::EventHandler(this, &Form1::toolStripButton1_Click_1);
			// 
			// toolStripButton2
			// 
			this->toolStripButton2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton2.Image")));
			this->toolStripButton2->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton2->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton2->Margin = System::Windows::Forms::Padding(2, 2, 4, 3);
			this->toolStripButton2->Name = L"toolStripButton2";
			this->toolStripButton2->Size = System::Drawing::Size(50, 33);
			this->toolStripButton2->Text = L"Refresh";
			this->toolStripButton2->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton2->Click += gcnew System::EventHandler(this, &Form1::toolStripButton2_Click);
			// 
			// toolStripButton3
			// 
			this->toolStripButton3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton3->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton3.Image")));
			this->toolStripButton3->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton3->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton3->Margin = System::Windows::Forms::Padding(2, 2, 4, 3);
			this->toolStripButton3->Name = L"toolStripButton3";
			this->toolStripButton3->Size = System::Drawing::Size(44, 33);
			this->toolStripButton3->Text = L"Delete";
			this->toolStripButton3->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton3->Click += gcnew System::EventHandler(this, &Form1::toolStripButton3_Click);
			// 
			// toolStripButton4
			// 
			this->toolStripButton4->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton4->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton4.Image")));
			this->toolStripButton4->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton4->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton4->Margin = System::Windows::Forms::Padding(2, 2, 4, 3);
			this->toolStripButton4->Name = L"toolStripButton4";
			this->toolStripButton4->Size = System::Drawing::Size(34, 33);
			this->toolStripButton4->Text = L"New";
			this->toolStripButton4->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton4->ToolTipText = L"New Folder";
			this->toolStripButton4->Click += gcnew System::EventHandler(this, &Form1::toolStripButton4_Click);
			// 
			// toolStripSeparator3
			// 
			this->toolStripSeparator3->Name = L"toolStripSeparator3";
			this->toolStripSeparator3->Size = System::Drawing::Size(6, 38);
			// 
			// toolStripButton12
			// 
			this->toolStripButton12->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->toolStripButton12->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton12.Image")));
			this->toolStripButton12->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton12->Margin = System::Windows::Forms::Padding(2, 1, 2, 2);
			this->toolStripButton12->Name = L"toolStripButton12";
			this->toolStripButton12->Size = System::Drawing::Size(35, 35);
			this->toolStripButton12->Text = L"Info.";
			this->toolStripButton12->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton12->ToolTipText = L"Show program info";
			this->toolStripButton12->Click += gcnew System::EventHandler(this, &Form1::Info_Click);
			// 
			// listView2
			// 
			this->listView2->AllowDrop = true;
			this->listView2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->listView2->FullRowSelect = true;
			this->listView2->GridLines = true;
			this->listView2->HideSelection = false;
			this->listView2->LabelEdit = true;
			this->listView2->Location = System::Drawing::Point(6, 93);
			this->listView2->Name = L"listView2";
			this->listView2->Size = System::Drawing::Size(320, 545);
			this->listView2->TabIndex = 2;
			this->listView2->UseCompatibleStateImageBehavior = false;
			this->listView2->View = System::Windows::Forms::View::Details;
			this->listView2->ItemActivate += gcnew System::EventHandler(this, &Form1::listView2_ItemActivate);
			this->listView2->AfterLabelEdit += gcnew System::Windows::Forms::LabelEditEventHandler(this, &Form1::listView_AfterLabelEdit);
			this->listView2->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listView2_SelectedIndexChanged);
			this->listView2->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::listView2_Layout);
			this->listView2->ColumnClick += gcnew System::Windows::Forms::ColumnClickEventHandler(this, &Form1::listView_ColumnClick);
			this->listView2->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::listView_KeyDown);
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 4000;
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
			// 
			// basicIconsSmall
			// 
			this->basicIconsSmall->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"basicIconsSmall.ImageStream")));
			this->basicIconsSmall->TransparentColor = System::Drawing::Color::Transparent;
			this->basicIconsSmall->Images->SetKeyName(0, L"folder.bmp");
			this->basicIconsSmall->Images->SetKeyName(1, L"document.bmp");
			this->basicIconsSmall->Images->SetKeyName(2, L"rec_file.bmp");
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->AutoSize = true;
			this->ClientSize = System::Drawing::Size(880, 660);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->statusStrip1);
			this->ForeColor = System::Drawing::SystemColors::ControlText;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"Form1";
			this->Text = L"Antares  0.7";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->ResizeBegin += gcnew System::EventHandler(this, &Form1::Form1_ResizeBegin);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::Form1_Paint);
			this->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::Form1_Layout);
			this->Resize += gcnew System::EventHandler(this, &Form1::Form1_Resize);
			this->ResizeEnd += gcnew System::EventHandler(this, &Form1::Form1_ResizeEnd);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->panel1->ResumeLayout(false);
			this->panel3->ResumeLayout(false);
			this->panel3->PerformLayout();
			this->panel5->ResumeLayout(false);
			this->panel5->PerformLayout();
			this->toolStrip2->ResumeLayout(false);
			this->toolStrip2->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel4->ResumeLayout(false);
			this->panel4->PerformLayout();
			this->panel6->ResumeLayout(false);
			this->panel6->PerformLayout();
			this->toolStrip1->ResumeLayout(false);
			this->toolStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void splitContainer1_Panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
			 }
	private: System::Void toolStripButton1_Click(System::Object^  sender, System::EventArgs^  e) {

			 }
	private: System::Void folderBrowserDialog1_HelpRequest(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void toolStripButton5_Click(System::Object^  sender, System::EventArgs^  e) {

				 this->topfieldUpDir();
			 }
	private: System::Void toolStripStatusLabel1_Click(System::Object^  sender, System::EventArgs^  e) {
			 }


	private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {


				 //printf("Timer tick.\n");
				 if (!this->transfer_in_progress)
				 {
					 this->CheckConnection();
				 }
				 // int conf,r,bus,address;
				 // if (this->fd==NULL)
				 // {
				 //	 toolStripStatusLabel1->Text="NULL fd";
				 // }
				 // else
				 // {
				 //	 device=libusb_get_device(this->fd);
				 //	 if (device==NULL)
				 //	 {
				 //		 toolStripStatusLabel1->Text="NULL device";
				 //	 }
				 //	 else
				 //	 {
				 //		 r = libusb_get_configuration(this->fd, &conf);
				 //		 if (r!=0)
				 //		 {
				 //			 toolStripStatusLabel1->Text="Error "+r.ToString();
				 //		 }
				 //		 else
				 //		 {
				 //			 this->dircount++;
				 //			 bus = libusb_get_bus_number(device);
				 //			 address = libusb_get_device_address(device);
				 //			 toolStripStatusLabel1->Text="OK "+this->dircount.ToString()+ "  " + "Bus "+bus.ToString() + " address " + address.ToString();
				 //		 }
				 //
				 //	 }
				 // }

			 }
	private: System::Void statusStrip1_ItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
			 }



	protected: virtual void OnLayout(System::Windows::Forms::LayoutEventArgs^ levent) override
			   {
				   //Console::WriteLine("Layout--");

				   array<Control^>^ arr = {this,panel1,panel2,panel3, panel4,panel5,listView2};

				   //				for (int j=0; j<arr->Length; j++) arr[j]->SuspendLayout();

				   for (int j=0; j<arr->Length; j++) this->SuspendDrawing(arr[j]);

				   this->Arrange1();
				   //printf("%d %d\n",panel4->Width, panel3->Width);
				   //for (int j=0; j<arr->Length; j++) arr[j]->ResumeLayout(true);
				   this->Arrange2();
				   //printf("%d %d\n",panel4->Width, panel3->Width);
				   //panel4->Width=panel3->Width;
				   Form::OnLayout(levent);
				   for (int j=0; j<arr->Length; j++) this->ResumeDrawing(arr[j]);
				   this->Refresh();
				   Arrange_Buttons();
				   //panel4->Refresh();
			   }

	private: System::Void Arrange2(void)
			 {
				 if (this->finished_constructing ==1)
				 {
					 static const double widths0[] = {140, 60, 50, 120};
					 static const double mwidths[] = {0,  60, 50, 120};
					 double tot0 = widths0[0]+widths0[1]+widths0[2]+widths0[3];
					 double tot0_ = widths0[1]+widths0[2]+widths0[3];
					 double tot0m = mwidths[1]+mwidths[2]+mwidths[3];
					 double tot1 = listView1->ClientSize.Width;
					 double tot2 = listView2->ClientSize.Width;

					 double tot1_ = listView1->Width;
					 tot1 = tot1_<tot1 ? tot1_ : tot1;

					 if (tot0_ / tot0 * tot1  > tot0m)
					 {

						 this->topfieldSizeHeader->Width = (int) mwidths[1];
						 this->topfieldTypeHeader->Width = (int) mwidths[2];
						 this->topfieldDateHeader->Width = (int) mwidths[3];
						 this->topfieldNameHeader->Width = (int) (tot1 - tot0m-5);
					 }
					 else
					 {
						 this->topfieldNameHeader->Width =  (int) (widths0[0]/tot0 * tot1);
						 this->topfieldSizeHeader->Width =  (int) (widths0[1]/tot0 * tot1)-1;
						 this->topfieldTypeHeader->Width =  (int) (widths0[2]/tot0 * tot1)-1;
						 this->topfieldDateHeader->Width =  (int) (widths0[3]/tot0 * tot1)-1;
					 }
					 if (tot0_ / tot0 * tot2  > tot0m)
					 {

						 this->computerSizeHeader->Width = (int) mwidths[1];
						 this->computerTypeHeader->Width = (int) mwidths[2];
						 this->computerDateHeader->Width = (int) mwidths[3];
						 this->computerNameHeader->Width = (int) (tot2 - tot0m-5);
					 }
					 else
					 {
						 this->computerNameHeader->Width =  (int) (widths0[0]/tot0 * tot2);
						 this->computerSizeHeader->Width =  (int) (widths0[1]/tot0 * tot2)-1;
						 this->computerTypeHeader->Width =  (int) (widths0[2]/tot0 * tot2)-1;
						 this->computerDateHeader->Width =  (int) (widths0[3]/tot0 * tot2)-1;
					 }

				 }

			 }

	private: System::Void Arrange_Buttons(void)
			 {

				 int ph = this->panel2->Height;
				 int bh = this->button1->Height;
				 int bp1 = 2*ph/3 - 18 - bh;
				 int bp2 = 2*ph/3 + 18;
				 if (bp1>268) {bp1=268; bp2=bp1+36+bh;}

				 Point p1 = this->button1->Location;
				 p1.Y = bp1;


				 Point p2 = this->button2->Location;
				 p2.Y = bp2;
				 this->button1->Location=p1;
				 this->button2->Location=p2;
			 }

	private: System::Void Arrange1(void)
			 {

				 int fw = this->ClientRectangle.Width;
				 int fh = this->ClientRectangle.Height;
				 if (fw==this->last_layout_x && fh==this->last_layout_y) return;
				 this->last_layout_x = fw;
				 this->last_layout_y = fh;
				 if (fw==0) return;
				 int avg = (fw - this->panel2->Width)/2;

				 this->panel4->Width=avg;
				 this->panel3->Width=avg;

				 //Arrange_Buttons();






			 }
	private: System::Void Arrange(void)
			 {
				 Arrange1();
				 Arrange2();
			 }

	private: System::Void Form1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
				 //Console::WriteLine("Layout");
				 //this->Arrange();
			 }

	private: System::Void Form1_Resize(System::Object^  sender, System::EventArgs^  e) {


				 //this->Arrange();

			 }

	private: System::Void listView1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
				 //double tot = this->listView1->Width;
				 //double tot = listView1->ClientSize.Width;
				 //static const double widths0[] = {140, 60, 50, 120};
				 //double tot0 = widths0[0]+widths0[1]+widths0[2]+widths0[3];
				 //if (this->finished_constructing==1)
				 //{
				 //	 topfieldNameHeader->Width = (int) (widths0[0]/tot0 * tot);
				 //	 topfieldSizeHeader->Width = (int) (widths0[1]/tot0 * tot)-1;
				 //	 topfieldTypeHeader->Width = (int) (widths0[2]/tot0 * tot)-1;
				 //	 topfieldDateHeader->Width = (int) (widths0[3]/tot0 * tot)-1;
				 // }
				 // printf("ListView1 layout\n");
			 }

	private: System::Void listView2_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {



				 //double tot = this->listView2->Width;
				 // double tot = listView2->ClientSize.Width;
				 //static const double widths0[] = {140, 60, 50, 120};
				 //double tot0 = widths0[0]+widths0[1]+widths0[2]+widths0[3];
				 //if (this->finished_constructing==1)
				 //{
				 //	 computerNameHeader->Width = (int) (widths0[0]/tot0 * tot);
				 //	 computerSizeHeader->Width = (int) (widths0[1]/tot0 * tot)-1;
				 //	 computerTypeHeader->Width = (int) (widths0[2]/tot0 * tot)-1;
				 //		 computerDateHeader->Width = (int) (widths0[3]/tot0 * tot)-1;
				 //	 }
				 //	 printf("ListView2 layout\n");
			 }

	private: System::Void listView2_ItemActivate(System::Object^  sender, System::EventArgs^  e) {
				 ListView^ listview = (ListView^) sender;
				 Console::WriteLine("Activated (2)");
				 //ComputerItem^ item = (ComputerItem^) sender;
				 //Console::WriteLine(item->Text);

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 ComputerItem^ item;
				 bool success=false;
				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<ComputerItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) {success=true;break;};

				 }

				 if(success)
				 {
					 String^ dir = Path::Combine(this->computerCurrentDirectory,item->Text);
					 this->setComputerDir(dir);
					 this->loadComputerDir();

				 }
				 else
				 {
					 this->ViewInfo(listview);
				 }





			 };
	private: System::Void toolStripButton1_Click_1(System::Object^  sender, System::EventArgs^  e) {
				 this->computerUpDir();
			 }
	private: System::Void toolStripButton2_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->loadComputerDir();

			 }

			 // Helper function: return size (in bytes) of file on computer, or -1 if there's a problem.
			 static long long int FileSize(String^ filename)
			 {
				 try
				 {
					 if (false == File::Exists(filename))
						 return -1;
					 else
					 {

						 FileInfo^ fi = gcnew FileInfo(filename); 
						 return fi->Length;
					 }
				 }
				 catch(...)
				 {
					 return -1;
				 }
			 }

	private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {    
				 // Copy files from Topfield to computer
				 const int max_folders = 1000;
				 // Enumerate selected source items (PVR)

				 ListView^ listview = this->listView1;

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 array<TopfieldItem^>^ items = gcnew array<TopfieldItem^>(selected->Count);
				 selected->CopyTo(items,0);

				 array<array<TopfieldItem^>^>^ items_by_folder = gcnew array<array<TopfieldItem^>^>(max_folders);


				 array<TopfieldItem^>^ these_items;

				 //Recurse into subdirectories, if applicable
				 int numfolders=1;
				 items_by_folder[0]=items;
				 int folder_ind;
				 int total_items_after_recursion = items->Length;
				 for( folder_ind=0;folder_ind<numfolders; folder_ind++)
				 {
					 these_items = items_by_folder[folder_ind];
					 for each (TopfieldItem^ item in these_items)
					 {
						 //Console::WriteLine(item->full_filename);
						 if (item->isdir)
						 {
							 items = this->loadTopfieldDirArray(item->full_filename);

							 for each ( TopfieldItem^ it in items)
							 {
								 if (item->recursion_offset == "")
									 it->recursion_offset = item->safe_filename;
								 else
									 it->recursion_offset = Antares::combineTopfieldPath(item->recursion_offset, item->safe_filename);
								 //Console::WriteLine("Item has full filename "+it->full_filename+", and recursion offset "+it->recursion_offset);
							 }

							 if (items->Length > 0 && numfolders<max_folders)
							 {
								 items_by_folder[numfolders]=items;
								 total_items_after_recursion += items->Length;
								 numfolders++;
							 }
						 }
					 }
				 }

				 int numitems = total_items_after_recursion;
				 // Copy the results into the flat "src_items" array
				 array<TopfieldItem^>^    src_items = gcnew array<TopfieldItem^>(numitems);
				 int ind=0;
				 //int numdirs=1;
				 for (folder_ind=0; folder_ind<numfolders; folder_ind++)
				 {
					 these_items = items_by_folder[folder_ind];
					 for each (TopfieldItem^ item in these_items)
					 {
						 src_items[ind]=item;
						 ind++;
						 //if (item->isdir) numdirs++;
					 }
				 }


				 //System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 TopfieldItem^ item;

				 int num_files =0;
				 long long totalsize = 0;
				 //long long totalsize_notskip=0;
				 //long long resume_granularity=8192;


				 array<bool>^             dest_exists = gcnew array<bool>(numitems);
				 array<DateTime>^         dest_date = gcnew array<DateTime>(numitems);
				 array<long long int>^    dest_size = gcnew array<long long int>(numitems);
				 array<long long int>^    src_sizes = gcnew array<long long int>(numitems);
				 array<String^>^          dest_filename= gcnew array<String^>(numitems);
				 array<int>^              overwrite_category=gcnew array<int>(numitems);
				 array<int>^              overwrite_action = gcnew array<int>(numitems);
				 array<long long int>^    current_offsets = gcnew array<long long int>(numitems);


				 array<int>^ num_cat = {0,0,0}; //numbers of existing files (divided by category)
				 int num_exist=0;
				 array<String^>^ files_cat = {"","",""};



				 //while ( myEnum->MoveNext() )
				 for (ind=0; ind<numitems; ind++)
				 {
					 //item = safe_cast<TopfieldItem^>(myEnum->Current);
					 item=src_items[ind];
					 //Console::WriteLine(item->Text);
					 //if (item->isdir) {continue;}   // Don't support whole directories yet
					 //src_items[numfiles]=item;


					 if (item->recursion_offset == "")
						 dest_filename[ind] = Path::Combine(this->computerCurrentDirectory, item->safe_filename);
					 else
					 {

						 dest_filename[ind] = Path::Combine(this->computerCurrentDirectory, Antares::safeString(item->recursion_offset));
						 dest_filename[ind] = Path::Combine(dest_filename[ind], item->safe_filename);
					 }
					 if (item->isdir)
					 {

						 continue;
					 }

					 src_sizes[ind]=item->size;


					 //dest_filename[ind]=Path::Combine(this->computerCurrentDirectory, item->safe_filename);

					 dest_exists[ind]=File::Exists(dest_filename[ind]);
					 if (dest_exists[ind])
					 {          // TODO: error handling
						 FileInfo^ fi = gcnew FileInfo(dest_filename[ind]);

						 dest_date[ind]=fi->CreationTime;//File::GetLastWriteTime(dest_filename[numfiles]);
						 dest_size[ind]=fi->Length;
						 int cat=2;
						 if (dest_size[ind] == item->size) // && dest_date[numfiles]==item->datetime)
							 cat=0;
						 else
						 {
							 if (dest_size[ind] < item->size) cat=1;
						 }

						 overwrite_category[ind]=cat;
						 num_cat[cat]++;if (num_cat[cat]>1) files_cat[cat] = files_cat[cat]+"\n";
						 files_cat[cat] = files_cat[cat]+dest_filename[ind]; 

						 num_exist++;
					 }

					 num_files++;
					 totalsize += item->size;
				 }


				 if (numitems==0) return;

				 int num_skip=0;
				 if (num_exist>0)
				 {
					 printf("num_exist=%d  num_cat={%d,%d,%d}\n",num_exist,num_cat[0],num_cat[1],num_cat[2]);
					 OverwriteConfirmation^ oc = gcnew OverwriteConfirmation(files_cat[0],files_cat[1],files_cat[2]);
					 if (num_exist==1) oc->title_label->Text="A file with this name already exists                                                                     ";
					 else oc->title_label->Text="Files with these names already exist                                                                                  ";					
					 //oc->files1->Text = files_cat[0];
					 if (num_cat[0]==0)
					 {
						 oc->panel1->Visible = false;oc->files1->Visible=false;
					 }
					 if (num_cat[0]>1) oc->label1->Text = "Files have correct size"; else oc->label1->Text = "File has correct size"; 

					 //oc->files2->Text = files_cat[1];
					 if (num_cat[1]==0)
					 {
						 oc->panel2->Visible = false;oc->files2->Visible=false;
					 }
					 if (num_cat[1]>1) oc->label2->Text = "Undersized files"; else oc->label2->Text = "Undersized file";

					 //oc->files3->Text = files_cat[2];
					 if (num_cat[2]==0)
					 {
						 oc->panel3->Visible = false;oc->files3->Visible=false;
					 }
					 if (num_cat[2]>1) oc->label3->Text = "These existing files are oversized:"; else oc->label3->Text = "This existing file is oversized:";

					 if (::DialogResult::Cancel == oc->ShowDialog() ) return;

					 int action1 = ( oc->overwrite1->Checked * OVERWRITE ) + oc->skip1->Checked * SKIP;
					 int action2 = ( oc->overwrite2->Checked * OVERWRITE ) + oc->skip2->Checked * SKIP + oc->resume2->Checked*RESUME;
					 int action3 = ( oc->overwrite3->Checked * OVERWRITE ) + oc->skip3->Checked * SKIP;

					 for (int i=0; i<numitems; i++)
					 {
						 item=src_items[i];
						 overwrite_action[i]=OVERWRITE;
						 if (dest_exists[i])
						 {
							 if(overwrite_category[i]==0)  overwrite_action[i]=action1; else
								 if(overwrite_category[i]==1)  overwrite_action[i]=action2; else
									 if(overwrite_category[i]==2)  overwrite_action[i]=action3;

						 }
						 if (overwrite_action[i]==RESUME && dest_size[i]<2*resume_granularity) overwrite_action[i]=OVERWRITE; // (don't bother resuming tiny files).
						 //  if (overwrite_action[i]==OVERWRITE) totalsize_notskip+=item->size;else
						 //	 if (overwrite_action[i]==RESUME) totalsize_notskip+=item->size-dest_size[i];

						 if (overwrite_action[i]==OVERWRITE) current_offsets[i]=0; else
							 if (overwrite_action[i]==SKIP) {current_offsets[i]=item->size;num_skip++;} else
								 if (overwrite_action[i]==RESUME) current_offsets[i]=dest_size[i];
					 }
				 }
				 if (num_skip==numitems) return;

				 if (this->checkBox1->Checked)
					 this->set_turbo_mode( 1); //TODO: error handling for turbo mode selection
				 else
					 this->set_turbo_mode( 0);



				 CopyDialog^ copydialog = gcnew CopyDialog();
				 copydialog->cancelled=false;
				 copydialog->showCopyDialog();
				 //copydialog->total_filesize = totalsize;
				 copydialog->total_start_time = time(NULL);
				 copydialog->current_start_time = 0 ;
				 //copydialog->current_filesize = 0; 
				 //copydialog->current_offset=0;
				 copydialog->filesizes = src_sizes;
				 copydialog->current_offsets = current_offsets;
				 copydialog->numfiles = num_files;
				 copydialog->current_index = 0;
				 copydialog->window_title="Copying File(s) ... [PVR --> PC]";
				 copydialog->current_file="Waiting for PVR...";
				 copydialog->turbo_mode = this->turbo_mode;
				 copydialog->update_dialog_threadsafe();


				 //myEnum = selected->GetEnumerator();
				 long long bytes_received;
				 long long total_bytes_received=0;
				 long long bytecount;
				 time_t startTime = time(NULL);
				 tf_packet reply;
				 int r;

				 array<Byte>^ existing_bytes = gcnew array<Byte>(    2* (int) resume_granularity);
				 long long existing_bytes_start; long long existing_bytes_count=0;

				 for (int i=0; i<numitems; i++)
				 {

					 item=src_items[i];

					 if (item->isdir)
					 {
						 if (File::Exists(dest_filename[i]) && !Directory::Exists(dest_filename[i]))
						 {
							 copydialog->close_request_threadsafe();
							 MessageBox::Show(this,"The folder "+dest_filename[ind]+" could not be created because a file of that name already exists. Aborting transfer.","Error",MessageBoxButtons::OK);

							 return;					
						 }
						 if (!Directory::Exists(dest_filename[i]))
						 {
							 try {
								 Directory::CreateDirectory(dest_filename[i]);
							 }
							 catch (...)
							 {
								 copydialog->close_request_threadsafe();
								 MessageBox::Show(this,"The folder "+dest_filename[i]+" could not be created. Aborting transfer.","Error",MessageBoxButtons::OK);

								 return;
							 }
						 }
						 continue;
					 }


					 //item = safe_cast<TopfieldItem^>(myEnum->Current);


					 //Console::WriteLine(item->Text);

					 bytecount=0;
					 String^ full_dest_filename = dest_filename[i]; //this->computerCurrentDirectory + "\\" + item->safe_filename;
					 String^ full_source_filename = item->full_filename; //item->directory + "\\" + item->filename;



					 /// This section of code adapted from commands.c [wuppy]
					 int result = -EPROTO;

					 enum
					 {
						 START,
						 DATA,
						 ABORT
					 } state;

					 //int dst = -1;

					 //int update = 0;
					 //__u64 byteCount = 0;
					 struct utimbuf mod_utime_buf = { 0, 0 };
					 FileStream^ dest_file;
					 //String^ line = Console::ReadLine();full_dest_filename += line;

					 int this_overwrite_action;
					 long long topfield_file_offset = 0;
					 long long probable_minimum_received_offset=-1;

					 bool has_restarted=false;

					 if(0)
					 {
restart_copy_to_pc:
						 has_restarted=true;
						 topfield_file_offset=0;
						 copydialog->reset_rate();

						 long long newsize = this->FileSize(full_dest_filename);
						 if (newsize>=0) dest_exists[i]=true;
						 if (newsize <= 1000000) 
						 {
							 printf (" newsize = %lld \n",newsize);
							 this_overwrite_action = OVERWRITE;
						 }
						 else
						 {
							 // If the user specified OVERWRITE initially, be careful about changing it to RESUME
							 // just because an error has occurred.
							 if (this_overwrite_action==OVERWRITE && dest_exists[i])   
							 {
								 if (newsize > 1000000 && newsize <= (probable_minimum_received_offset + 65537))
								 {
									 this_overwrite_action=RESUME;

								 }
							 }
							 dest_size[i] = newsize;
						 }

					 }
					 else
					 {
						 this_overwrite_action = OVERWRITE;
						 if (dest_exists[i]) this_overwrite_action=overwrite_action[i];

					 }

					 if (this_overwrite_action==SKIP) {item->Selected=false;continue;}  

					 try{
						 //  TODO: Further exception handling for file open?
						 if (this_overwrite_action==OVERWRITE)
							 dest_file = File::Open(full_dest_filename,System::IO::FileMode::Create, System::IO::FileAccess::ReadWrite,System::IO::FileShare::Read);
						 else if (this_overwrite_action==RESUME)
						 {
							 dest_file = File::Open(full_dest_filename,System::IO::FileMode::Open, System::IO::FileAccess::ReadWrite,System::IO::FileShare::Read);
							 existing_bytes_start = (dest_size[i]/resume_granularity)*resume_granularity - resume_granularity;
							 existing_bytes_count = dest_size[i]-existing_bytes_start;
							 dest_file->Seek(existing_bytes_start,SeekOrigin::Begin);
							 existing_bytes_count = dest_file->Read(existing_bytes, 0,(int) existing_bytes_count);
							 topfield_file_offset = existing_bytes_start; 

						 }

					 }
					 catch(System::UnauthorizedAccessException ^)
					 {
						 copydialog->close_request_threadsafe();
						 MessageBox::Show(this,"Antares cannot save the file to the location you chose. Please select another location and try again.","Write permission denied",MessageBoxButtons::OK);
						 goto end_copy_to_pc;				
					 }


					 dest_file->Seek(topfield_file_offset,SeekOrigin::Begin);



					 char* srcPath = (char*)(void*)Marshal::StringToHGlobalAnsi(full_source_filename);

					 bool was_cancelled=false;
					 bool usb_error=false;
					 bool turbo_changed=false;


					 //printf("topfield_file_offset = %ld\n",topfield_file_offset);
					 if (topfield_file_offset==0) 
						 r = send_cmd_hdd_file_send(this->fd, GET, srcPath);   
					 else
						 r = send_cmd_hdd_file_send_with_offset(this->fd, GET, srcPath,topfield_file_offset);

					 Marshal::FreeHGlobal((System::IntPtr)(void*)srcPath);

					 if(r < 0)
					 {
						 usb_error=true;
						 goto out;
					 }

					 state = START;

					 bytes_received=0;
					 if (copydialog->current_start_time==0)
					 {
						 copydialog->total_start_time=time(NULL);
						 copydialog->current_start_time=time(NULL);
					 }
					 else
						 copydialog->current_start_time = time(NULL);
					 copydialog->current_file = full_dest_filename;
					 copydialog->current_index = i;
					 copydialog->current_offsets[i]=topfield_file_offset;
					 copydialog->current_bytes_received=bytes_received;
					 copydialog->total_bytes_received=total_bytes_received;
					 //copydialog->current_filesize = item->size;
					 copydialog->update_dialog_threadsafe();


					 int update=0;
					 while(1)
					 {

						 r = get_tf_packet(fd, &reply);
						 update = (update+1)%8;
						 if (r<=0)
						 {
							 usb_error=true;
							 goto out;
						 }


						 switch (get_u32(&reply.cmd))
						 {
						 case DATA_HDD_FILE_START:
							 if(state == START)
							 {
								 struct typefile *tf = (struct typefile *) reply.data;

								 bytecount = get_u64(&tf->size);
								 mod_utime_buf.actime = mod_utime_buf.modtime =
									 tfdt_to_time(&tf->stamp);

								 send_success(fd);
								 state = DATA;
							 }
							 else
							 {
								 fprintf(stderr,
									 "ERROR: Unexpected DATA_HDD_FILE_START packet in state %d\n",
									 state);
								 send_cancel(fd);
								 usb_error=true;
								 state = ABORT;
							 }
							 break;

						 case DATA_HDD_FILE_DATA:
							 if(state == DATA)
							 {
								 __u64 offset = get_u64(reply.data);
								 __u16 dataLen =
									 get_u16(&reply.length) - (PACKET_HEAD_SIZE + 8);


								 // if( !quiet)
								 // {
								 //progressStats(bytecount, offset + dataLen, startTime);
								 // }

								 if(r < get_u16(&reply.length))
								 {
									 fprintf(stderr,
										 "ERROR: Short packet %d instead of %d\n", r,
										 get_u16(&reply.length));

									 usb_error=true;
									 goto out;
								 }



								 array <unsigned char>^ buffer = gcnew array<unsigned char>(dataLen);
								 //System::Byte buffer[] = gcnew System::Byte[dataLen];
								 Marshal::Copy( IntPtr( (void*)  &reply.data[8] ) , buffer, 0,(int) dataLen);
								 if (topfield_file_offset == existing_bytes_start && existing_bytes_count>0)
								 {
									 int overlap_size = (int) ( dataLen < existing_bytes_count ? dataLen : existing_bytes_count );
									 bool failed=false;
									 //printf("Overlap_size=%d\n",overlap_size);
									 for (int j=0; j<overlap_size; j++)
									 {

										 //printf("(%02x %02x) ",buffer[j],existing_bytes[j]);
										 if (buffer[j] != existing_bytes[j]) {failed=true; break; };
									 }
									 if (failed==true || overlap_size<10)
									 {
										 printf("Warning: resume failed. Starting again.\n");
										 topfield_file_offset = 0;
										 this_overwrite_action=OVERWRITE;
										 dest_file->Close();
										 send_cancel(this->fd);
										 absorb_late_packets(4,400);
										 goto restart_copy_to_pc;
									 }

								 }
								 dest_file->Write(buffer, 0, dataLen);        //TODO:  Catch full-disk errors  (Sytem::IO:IOException)
								 topfield_file_offset+=dataLen;
								 probable_minimum_received_offset=topfield_file_offset;
								 copydialog->new_packet(dataLen);
								 if (topfield_file_offset != offset)
								 {
									 //printf("Warning: offset mismatch! %lu %lu \n",topfield_file_offset,offset);
									 // TODO: Handle this type of error
								 }

								 bytes_received += dataLen;
								 total_bytes_received +=dataLen;
								 if (topfield_file_offset>item->size) printf("Warning: topfield_file_offset>item->size\n");else
									 //copydialog->total_offset = total_bytes_received;
									 copydialog->current_offsets[i] = topfield_file_offset;//bytes_received;


								 copydialog->current_bytes_received = bytes_received;
								 copydialog->total_bytes_received = total_bytes_received;
								 if (update==0)
								 {
									 this->Update();
									 copydialog->update_dialog_threadsafe();
								 }
								 //if(w < dataLen)
								 //{
								 //	 /* Can't write data - abort transfer */
								 //	 fprintf(stderr, "ERROR: Can not write data: %s\n",
								 //		 strerror(errno));
								 //	 send_cancel(fd);
								 //	 state = ABORT;
								 //}



								 if (copydialog->cancelled == true)
								 {
									 printf("CANCELLING\n");
									 send_cancel(fd);
									 was_cancelled=true;
									 state = ABORT;
									 goto out;

								 }

								 if (copydialog->turbo_request != *this->turbo_mode)
								 {
									 turbo_changed=true;

									 send_cancel(fd);
									 state=ABORT;
									 goto out;
								 }



							 }
							 else
							 {
								 fprintf(stderr,
									 "ERROR: Unexpected DATA_HDD_FILE_DATA packet in state %d\n",
									 state);
								 send_cancel(fd);
								 usb_error=true;
								 state = ABORT;
							 }
							 break;

						 case DATA_HDD_FILE_END:
							 send_success(fd);
							 item->Selected = false;
							 //printf("DATA_HDD_FILE_END\n");
							 result = 0;
							 goto out;
							 break;

						 case FAIL:
							 fprintf(stderr, "ERROR: Device reports %s\n",
								 decode_error(&reply));
							 send_cancel(fd);
							 state = ABORT;
							 usb_error=true;
							 break;

						 case SUCCESS:
							 //printf("SUCCESS\n");
							 goto out;
							 break;

						 default:
							 fprintf(stderr, "ERROR: Unhandled packet (cmd 0x%x)\n",
								 get_u32(&reply.cmd));
							 send_cancel(fd);
							 usb_error=true;
							 goto out;
						 }




					 }

out:
					 //_close(dst);
					 try
					 {
						 dest_file->Close();
						 File::SetCreationTime(full_dest_filename, item->datetime);
						 File::SetLastWriteTime(full_dest_filename, item->datetime);
					 }
					 catch(...)
					 {

					 }

					 if (was_cancelled) break;
					 if (copydialog->cancelled==true) break;

					 if (usb_error)
					 {
						 if (this->wait_for_connection(copydialog) < 0) 
						 {
							 copydialog->close_request_threadsafe();
							 return;
						 }
						 else
							 goto restart_copy_to_pc;
					 }




					 if (turbo_changed || copydialog->turbo_request != *this->turbo_mode )
					 {
						 this->absorb_late_packets(4,100);
						 this->set_turbo_mode(copydialog->turbo_request);
						 copydialog->reset_rate();
						 goto restart_copy_to_pc;
					 }


					 //if ()
					 //{
					 //	 this->set_turbo_mode( copydialog->turbo_request ? 1:0);
					 //	 copydialog->reset_rate();
					 //	
					 //}



				 }
end_copy_to_pc:
				 this->loadComputerDir();
				 this->absorb_late_packets(2,200);
				 this->set_turbo_mode(0);
				 if (!copydialog->is_closed)
					 copydialog->close_request_threadsafe();

			 }

			 TopfieldItem^ topfieldFileExists(array<array<TopfieldItem^>^>^ topfield_items_by_folder,  String^ dest_path)
			 {
				 int num = topfield_items_by_folder->Length;
				 int j;
				 for (j=0; j<num; j++)
				 {
					 array<TopfieldItem^>^ these_items = topfield_items_by_folder[j];
					 for each (TopfieldItem^ titem in these_items)
					 {
						 if (titem->full_filename == dest_path)
							 return titem;
					 }
				 }
				 return nullptr;
			 }

			 ///////////////////////////////////////////
	private: System::Void transfer_to_PVR(Object^ input){
				 // Worker thread for doing the transfer from PC -> PVR
				 printf("!!!!!!!!!!!!!!!!!!! copy thread started\n");
				 CopyDialog^ copydialog = safe_cast<CopyDialog^>(input);
				 while(copydialog->loaded==false)
				 {
					 Thread::Sleep(100);
				 }
				 copydialog->update_dialog_threadsafe();
				 int numitems = copydialog->numfiles;

				 int this_overwrite_action;
				 long long topfield_file_offset=0;
				 long long probable_minimum_received_offset=-1;
				 //String^                  full_src_filename  = copydialog->full_src_filename;
				 //String^                  full_dest_filename = copydialog->full_dest_filename;
				 array<String^>^          dest_filename      = copydialog->dest_filename;
				 array<bool>^             dest_exists        = copydialog->dest_exists;
				 array<long long int>^    dest_size          = copydialog->dest_size;
				 array<int>^              overwrite_action   = copydialog->overwrite_action;
				 array<long long int>^    current_offsets    = copydialog->current_offsets;
				 // int                      i                  = copydialog->current_index;
				 array<long long int>^    src_sizes          = copydialog->filesizes;
				 array<FileItem^>^        src_items          = copydialog->src_items;
				 array<array<TopfieldItem^>^>^ topfield_items_by_folder = copydialog->topfield_items_by_folder;
				 TopfieldItem^ titem;
				 copydialog->maximum_successful_index=-1;
				 for (int i=0; i<numitems; i++)
				 {
					 copydialog->current_index = i;

					 ComputerItem^ item = safe_cast<ComputerItem^>(src_items[i]);
					 String^ full_dest_filename = dest_filename[i];
					 String^ full_src_filename = item->full_filename;

					 if (item->isdir)
					 {
						 titem = this->topfieldFileExists(topfield_items_by_folder,dest_filename[i]);
						 int r=0;
						 if (titem==nullptr)
						 {
							 r = this->newTopfieldFolder(dest_filename[i]);
						 }
						 // Abort if required destination folder is already a file name, or any other error creating folder.
						 if (r<0 || (titem!=nullptr && !titem->isdir) )
						 {
							 copydialog->close_request_threadsafe();
							 //MessageBox::Show(this,"The folder "+dest_filename[i]+" could not be created. Aborting transfer.","Error",MessageBoxButtons::OK);							
							 copydialog->file_error="The folder "+dest_filename[i]+" could not be created. Aborting transfer.";
							 goto finish_transfer;
						 }
						 continue;
					 }


					 FileStream^ src_file;
					 bool has_restarted = false;

					 copydialog->usb_error=false;
					 copydialog->file_error="";

					 if (0)
					 {
restart_copy_to_pvr:     
						 copydialog->usb_error=false;
						 has_restarted=true;
						 topfield_file_offset=0;
						 copydialog->reset_rate();
						 TopfieldItem^ reloaded = this->reloadTopfieldItem(full_dest_filename);
						 if (reloaded==nullptr)
						 {
							 printf("reloaded == nullptr\n");
							 this_overwrite_action = OVERWRITE;

						 }
						 else
						 {

							 if (this_overwrite_action==OVERWRITE)
							 {
								 // If the user specified OVERWRITE initially, be careful about changing it to RESUME
								 // just because an error has occurred.
								 if  (dest_exists[i])   
								 {
									 printf("reloaded->size = %lld  probable_minimum_received_offset=%lld\n",reloaded->size,probable_minimum_received_offset);
									 if (reloaded->size > 1000000 && reloaded->size <= (probable_minimum_received_offset + 65537))
									 {
										 this_overwrite_action=RESUME;

									 }
								 }
								 else
								 {
									 this_overwrite_action=RESUME;
									 dest_exists[i]=true;
								 }
							 }
							 dest_size[i] = reloaded->size;
						 }

					 }

					 else
					 {
						 this_overwrite_action = OVERWRITE;
						 if (dest_exists[i]) this_overwrite_action=overwrite_action[i];
					 }

					 if (this_overwrite_action==SKIP) {printf("Skipping.\n");goto finish_transfer;} //{item->Selected=false;continue;}  

					 if (this_overwrite_action==RESUME && dest_size[i]>=src_sizes[i]) {printf("Not resuming.\n");goto finish_transfer;}//{item->Selected=false;continue;}


					 try {

						 src_file = File::Open(full_src_filename,System::IO::FileMode::Open, System::IO::FileAccess::Read,System::IO::FileShare::Read);
					 }
					 catch(...)
					 {
						 //TODO: better error handling?
						 //copydialog->close_request_threadsafe();
						 copydialog->file_error="The file "+full_src_filename+" could not be opened for reading. Aborting transfer.";
						 printf("%s\n",copydialog->file_error);
						 goto finish_transfer;
					 }

					 FileInfo^ src_file_info = gcnew FileInfo(full_src_filename);


					 long long fileSize = src_file_info->Length;
					 if(fileSize == 0)   //TODO: shouldn't we still create 0-byte files?
					 {
						 printf("ERROR: Source file is empty - not transfering.\n");
						 //result = -1;
						 return;
						 //continue;
					 }
					 char* dstPath = (char *) (void*) Marshal::StringToHGlobalAnsi(full_dest_filename);

					 // bool was_cancelled=false;
					 // bool usb_error=false;
					 bool turbo_changed=false;
					 struct tf_packet packet;
					 struct tf_packet reply;
					 array<Byte>^ inp_buffer = gcnew array<unsigned char>(sizeof(packet.data));


					 if (this_overwrite_action==RESUME)
					 {
						 bool overlap_failed;
						 long long existing_bytes_start = (dest_size[i]/resume_granularity)*resume_granularity - resume_granularity;
						 if (existing_bytes_start<=0)
							 overlap_failed=true;
						 else
						 {
							 int existing_bytes_count = (int)  (dest_size[i]-existing_bytes_start);
							 src_file->Seek(existing_bytes_start,SeekOrigin::Begin);
							 int existing_bytes_count_PC = src_file->Read(inp_buffer, 0, existing_bytes_count);
							 array<Byte>^ existing_bytes;
							 existing_bytes = this->read_topfield_file_snippet(full_dest_filename,existing_bytes_start);

							 if (existing_bytes->Length == 0)
							 {
								 copydialog->usb_error=true;
								 goto out;

							 }

							 int existing_bytes_count_PVR = existing_bytes->Length;

							 int overlap = existing_bytes_count_PC < existing_bytes_count_PVR ? existing_bytes_count_PC : existing_bytes_count_PVR;
							 overlap_failed=false;
							 printf("dest_size[i]=%lld    existing_bytes_count PVR=%d PC=%d   overlap=%d\n",dest_size[i],existing_bytes_count_PVR, existing_bytes_count_PC, overlap);  
							 if (overlap<resume_granularity)
							 {
								 overlap_failed=true;
								 printf("Resume failed: overlap too small.\n");

							 }
							 else
							 {
								 for (int j=0; j<overlap; j++)
								 {
									 if (existing_bytes[j]!=inp_buffer[j])
									 { 
										 printf("Overlap failed: bytes disagree. Position %d.\n",j);
										 overlap_failed=true;
										 break;
									 }
								 }

							 }
						 }

						 if (!overlap_failed)
						 {
							 printf("Overlap success.\n");
							 topfield_file_offset = existing_bytes_start; 
							 src_file->Seek(existing_bytes_start,SeekOrigin::Begin);
							 current_offsets[i]=topfield_file_offset;
						 }
						 if (overlap_failed)
						 {
							 topfield_file_offset=0;
							 current_offsets[i]=0;
							 this_overwrite_action=OVERWRITE;
							 src_file->Seek(0,SeekOrigin::Begin);
						 }
					 }

					 int r;

					 printf("topfield_file_offset=%ld   = %f MB\n",topfield_file_offset,((double)topfield_file_offset)/1024.0/1024.0);
					 if (topfield_file_offset==0)
						 r = send_cmd_hdd_file_send(this->fd, PUT, dstPath);
					 else
						 r = send_cmd_hdd_file_send_with_offset(this->fd, PUT, dstPath,topfield_file_offset);
					 Marshal::FreeHGlobal((System::IntPtr) (void*)dstPath);
					 if(r < 0)
					 {
						 copydialog->usb_error=true;
						 goto out;
					 }
					 long long bytes_sent=0;

					 if (copydialog->current_start_time==0)
					 {
						 copydialog->current_start_time = time(NULL);
						 copydialog->total_start_time = time(NULL);
					 }
					 else
						 copydialog->current_start_time=time(NULL);

					 copydialog->current_file = full_src_filename;
					 //copydialog->current_index=i;
					 copydialog->current_bytes_received = bytes_sent;
					 copydialog->update_dialog_threadsafe();

					 enum
					 {
						 START,
						 DATA,
						 END,
						 FINISHED
					 } state;
					 state = START;
					 int result = -EPROTO;
					 int nextw;
					 bool have_next_packet=false;


					 int update=0;
					 while(1)
					 {
						 r = get_tf_packet(this->fd, &reply);

						 if (r<=0)
						 {
							 copydialog->usb_error=true;
							 goto out;
						 }

						 update = (update + 1) % 4;
						 switch (get_u32(&reply.cmd))
						 {
						 case SUCCESS:
							 switch (state)
							 {
							 case START:
								 {
									 /* Send start */
									 struct typefile *tf = (struct typefile *) packet.data;

									 put_u16(&packet.length, PACKET_HEAD_SIZE + 114);
									 put_u32(&packet.cmd, DATA_HDD_FILE_START);

									 // TODO: how are timezones being accounted for?
									 time_to_tfdt64(Antares::DateTimeToTime_T(src_file_info->LastWriteTime.ToUniversalTime()) , &tf->stamp); 
									 //time_to_tfdt64(1275312247 , &tf->stamp);
									 tf->filetype = 2;
									 put_u64(&tf->size, src_file_info->Length);
									 strncpy((char *) tf->name, dstPath, 94);
									 tf->name[94] = '\0';
									 tf->unused = 0;
									 tf->attrib = 0;
									 trace(3,
										 fprintf(stderr, "%s: DATA_HDD_FILE_START\n",
										 __FUNCTION__));
									 r = send_tf_packet(this->fd, &packet);
									 if(r < 0)
									 {
										 fprintf(stderr, "ERROR: Incomplete send.\n");
										 copydialog->usb_error=true;
										 goto out;
									 }
									 state = DATA;
									 break;
								 }

							 case DATA:
								 {
									 int payloadSize = sizeof(packet.data) - 9;    payloadSize = payloadSize / 1024*1024; 

									 int w;

									 if (have_next_packet)
									 {

										 w=nextw;
										 have_next_packet=false;
									 }
									 else
									 {

										 w = src_file->Read(inp_buffer, 0, payloadSize);

										 Marshal::Copy(inp_buffer,0,System::IntPtr( &packet.data[8]),w);

										 /* Detect a Topfield protcol bug and prevent the sending of packets
										 that are a multiple of 512 bytes. */
										 if((w > 4)
											 &&
											 (((((PACKET_HEAD_SIZE + 8 + w) +
											 1) & ~1) % 0x200) == 0))
										 {
											 printf("\n -- SEEK CORRECTION ---\n");
											 src_file->Seek(-4, System::IO::SeekOrigin::Current);
											 w -= 4;
											 payloadSize -= 4;
										 }

										 put_u16(&packet.length, PACKET_HEAD_SIZE + 8 + w);
										 put_u32(&packet.cmd, DATA_HDD_FILE_DATA);
										 put_u64(packet.data, topfield_file_offset);
									 }
									 //byteCount += w;
									 bytes_sent+=w;
									 copydialog->total_bytes_received+=w;
									 probable_minimum_received_offset=topfield_file_offset;
									 topfield_file_offset+=w;


									 if (copydialog->cancelled == true)
									 {
										 printf("CANCELLING\n");
										 //send_cancel(fd);
										 //was_cancelled=true;
										 state = END;
									 }

									 if (copydialog->turbo_request != *this->turbo_mode)
									 {
										 turbo_changed=true;
										 state=END;
									 }

									 /* Detect EOF and transition to END */
									 if((w < 0) || (topfield_file_offset >= fileSize))
									 {
										 printf("\nEOF conditition. w=%d  bytes_sent=%f  fileSize=%f\n",w,(double) bytes_sent,(double) fileSize);
										 state = END;
									 }

									 if(w > 0)
									 {
										 trace(3,
											 fprintf(stderr, "%s: DATA_HDD_FILE_DATA\n",
											 __FUNCTION__));
										 r = send_tf_packet(this->fd, &packet);
										 if(r < w)
										 {
											 fprintf(stderr, "ERROR: Incomplete send.\n");
											 copydialog->usb_error=true;
											 goto out;
										 }
										 copydialog->new_packet(r);
									 }

									 if (update==0)
									 {
										 //this->Update();

										 copydialog->current_offsets[i] = topfield_file_offset;
										 copydialog->current_bytes_received = bytes_sent;
										 copydialog->update_dialog_threadsafe();

									 }


									 if (state != END)
									 {
										 // create_next_packet
										 nextw = src_file->Read(inp_buffer, 0, payloadSize);
										 Marshal::Copy(inp_buffer,0,System::IntPtr( &packet.data[8]),w);

										 /* Detect a Topfield protcol bug and prevent the sending of packets
										 that are a multiple of 512 bytes. */
										 if((nextw > 4)
											 &&
											 (((((PACKET_HEAD_SIZE + 8 + nextw) +
											 1) & ~1) % 0x200) == 0))
										 {
											 printf("\n -- SEEK CORRECTION ---\n");
											 src_file->Seek(-4, System::IO::SeekOrigin::Current);
											 nextw -= 4;
											 payloadSize -= 4;
										 }

										 put_u16(&packet.length, PACKET_HEAD_SIZE + 8 + nextw);
										 put_u32(&packet.cmd, DATA_HDD_FILE_DATA);
										 put_u64(packet.data, topfield_file_offset);
										 if (nextw>0) have_next_packet=true;
									 }




									 break;
								 }

							 case END:
								 /* Send end */
								 put_u16(&packet.length, PACKET_HEAD_SIZE);
								 put_u32(&packet.cmd, DATA_HDD_FILE_END);
								 trace(3,
									 fprintf(stderr, "%s: DATA_HDD_FILE_END\n",
									 __FUNCTION__));
								 r = send_tf_packet(fd, &packet);
								 if(r < 0)
								 {
									 fprintf(stderr, "ERROR: Incomplete send.\n");
									 copydialog->usb_error=true;
									 goto out;
								 }
								 state = FINISHED;
								 //if (!was_cancelled) item->Selected = false;
								 break;

							 case FINISHED:
								 result = 0;
								 goto out;
								 break;
							 }    //(end switch state)
							 break;

						 case FAIL:
							 fprintf(stderr, "ERROR: Device reports %s\n",
								 decode_error(&reply));
							 copydialog->usb_error=true;
							 goto out;
							 break;

						 default:
							 fprintf(stderr, "ERROR: Unhandled packet (copy PVR -> PC)\n");
							 copydialog->usb_error=true;
							 goto out;
							 break;
						 }   // (end switch reply.cmd) 
					 }  // (end while loop over packets sent)

out:

					 try {
						 src_file->Close();
					 }
					 catch(...)
					 {
					 }

					 if (copydialog->cancelled==true)  {printf("Cancelled.\n");goto finish_transfer;}

					 //if (was_cancelled) break;

					 if (copydialog->usb_error)
						 if (this->wait_for_connection(copydialog) < 0) 
						 {
							 //copydialog->close_request_threadsafe();
							 printf("Cancelling after error.\n");
							 goto finish_transfer;
						 }
						 else
							 goto restart_copy_to_pvr;

					 if (turbo_changed)
					 {
						 this->absorb_late_packets(2,100);
						 this->set_turbo_mode(copydialog->turbo_request);
						 copydialog->reset_rate();
						 goto restart_copy_to_pvr;
					 }


					 if (copydialog->turbo_request != *this->turbo_mode)
					 {
						 this->set_turbo_mode( copydialog->turbo_request ? 1:0);
						 copydialog->reset_rate();

					 }

					 if (!copydialog->cancelled) {copydialog->maximum_successful_index=i;};
					 if (copydialog->cancelled)
					 {
						 //copydialog->close_request_threadsafe();
						 break;
					 }
				 }  // (end loop over files to transfer)

finish_transfer:
				 copydialog->close_request_threadsafe();
				 printf("!!!!!!! Transfer thread ended normally.\n");
			 }

			 ////////////////////////////////////////////////////////////////////////////////////
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

				 // Copy files from Computer to Topfield

				 const int max_folders = 1000;

				 //time_t startTime = time(NULL);

				 int src = -1;
				 //int r;
				 //int update = 0;

				 // Enumerate selected source items on computer

				 ListView^ listview = this->listView2;

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 array<ComputerItem^>^ items = gcnew array<ComputerItem^>(selected->Count);
				 selected->CopyTo(items,0);
				 array<array<ComputerItem^>^>^ items_by_folder = gcnew array<array<ComputerItem^>^>(max_folders);

				 array<ComputerItem^>^ these_items;


				 //Recurse into subdirectories, if applicable
				 int numfolders=1;
				 items_by_folder[0]=items;
				 int folder_ind;
				 int total_items_after_recursion = items->Length;
				 for( folder_ind=0;folder_ind<numfolders; folder_ind++)
				 {
					 these_items = items_by_folder[folder_ind];
					 for each (ComputerItem^ item in these_items)
					 {
						 //Console::WriteLine(item->full_filename);
						 if (item->isdir)
						 {
							 items = this->loadComputerDirArray(item->full_filename);

							 for each ( ComputerItem^ it in items)
							 {
								 if (item->recursion_offset == "")
									 it->recursion_offset = item->safe_filename;
								 else
									 it->recursion_offset = Path::Combine(item->recursion_offset, item->safe_filename);
								 //Console::WriteLine("Item has full filename "+it->full_filename+", and recursion offset "+it->recursion_offset);
							 }

							 if (items->Length > 0 && numfolders<max_folders)
							 {
								 items_by_folder[numfolders]=items;
								 total_items_after_recursion += items->Length;
								 numfolders++;
							 }
						 }
					 }
				 }

				 int numitems = total_items_after_recursion;
				 // Copy the results into the flat "src_items" array
				 array<ComputerItem^>^    src_items = gcnew array<ComputerItem^>(numitems);
				 int ind=0;
				 int numdirs=1;
				 for (folder_ind=0; folder_ind<numfolders; folder_ind++)
				 {
					 these_items = items_by_folder[folder_ind];
					 for each (ComputerItem^ item in these_items)
					 {
						 src_items[ind]=item;
						 ind++;
						 if (item->isdir) numdirs++;
					 }
				 }

				 // Load the each topfield directory corresponding to a source directory, if it exists
				 array<array<TopfieldItem^>^>^ topfield_items_by_folder = gcnew array<array<TopfieldItem^>^>(numdirs);
				 ind=0;
				 topfield_items_by_folder[ind] = this->loadTopfieldDirArray(this->topfieldCurrentDirectory);
				 for each (ComputerItem^ item in src_items)
				 {
					 if (item->isdir)
					 {
						 ind++;
						 String ^tmp;
						 if (item->recursion_offset=="")
							 tmp = Antares::combineTopfieldPath(this->topfieldCurrentDirectory,item->filename);
						 else
						 {
							 tmp = Antares::combineTopfieldPath(this->topfieldCurrentDirectory,item->recursion_offset);
							 tmp = Antares::combineTopfieldPath(tmp,item->filename);
						 }
						 topfield_items_by_folder[ind]=this->loadTopfieldDirArray(tmp);
					 }
				 }

				 topfield_items_by_folder->Resize(topfield_items_by_folder, ind+1);



				 ComputerItem^ item;

				 int num_files =0;
				 long long totalsize=0;
				 //long long resume_granularity=8192;

				 array<bool>^             dest_exists = gcnew array<bool>(numitems);
				 array<DateTime>^         dest_date = gcnew array<DateTime>(numitems);
				 array<long long int>^    dest_size = gcnew array<long long int>(numitems);
				 array<long long int>^    src_sizes = gcnew array<long long int>(numitems);
				 array<String^>^          dest_filename= gcnew array<String^>(numitems);
				 array<int>^              overwrite_category=gcnew array<int>(numitems);
				 array<int>^              overwrite_action = gcnew array<int>(numitems);
				 array<long long int>^    current_offsets = gcnew array<long long int>(numitems);

				 TopfieldItem^ titem;				 
				 array<int>^ num_cat={0,0,0}; //numbers of existing files (divided by category of destination file: 0=correct size,  1=undersized, 2=oversized).
				 int num_exist=0;
				 array<String^>^ files_cat = {"","",""};
				 for (ind=0; ind<numitems; ind++)
				 {
					 item = src_items[ind];
					 if (item->recursion_offset == "")
						 dest_filename[ind] = Antares::combineTopfieldPath(this->topfieldCurrentDirectory, item->safe_filename);
					 else
					 {

						 dest_filename[ind] = Path::Combine(this->topfieldCurrentDirectory, Antares::safeString(item->recursion_offset));
						 dest_filename[ind] = Path::Combine(dest_filename[ind], item->safe_filename);
					 }
					 if (item->isdir) {continue;}   

					 src_sizes[ind]=item->size;

					 titem = this->topfieldFileExists(topfield_items_by_folder, dest_filename[ind]);
					 if (titem == nullptr)
						 dest_exists[ind]=false;
					 else
					 {
						 dest_exists[ind]=true;
						 dest_size[ind] = titem->size;
					 }

					 if (dest_exists[ind])
					 { 
						 int cat=2;
						 if (dest_size[ind] == item->size) 
							 cat=0;
						 else if (dest_size[ind] < item->size) cat=1;

						 overwrite_category[ind]=cat;
						 num_cat[cat]++;if (num_cat[cat]>1) files_cat[cat] = files_cat[cat]+"\n";
						 files_cat[cat] = files_cat[cat]+dest_filename[ind]; 
						 num_exist++;
					 }

					 current_offsets[ind]=0;
					 totalsize += item->size;
				 }
				 if (numitems==0) return;


				 int num_skip=0;
				 if (num_exist>0)
				 {
					 printf("num_exist=%d  num_cat={%d,%d,%d}\n",num_exist,num_cat[0],num_cat[1],num_cat[2]);
					 OverwriteConfirmation^ oc = gcnew OverwriteConfirmation(files_cat[0],files_cat[1], files_cat[2]);
					 if (num_exist==1) oc->title_label->Text="A file with this name already exists                                                   ";
					 else oc->title_label->Text = "Files with these names already exist                                                              ";

					 if (num_cat[0]==0)
					 {
						 oc->panel1->Visible = false;oc->files1->Visible=false;
					 }
					 if (num_cat[0]>1) oc->label1->Text = "Files have correct size"; else oc->label1->Text = "File has correct size"; 

					 if (num_cat[1]==0)
					 {
						 oc->panel2->Visible = false;oc->files2->Visible=false;
					 }
					 if (num_cat[1]>1) oc->label2->Text = "Undersized files"; else oc->label2->Text = "Undersized file";

					 if (num_cat[2]==0)
					 {
						 oc->panel3->Visible = false;oc->files3->Visible=false;
					 }
					 if (num_cat[2]>1) oc->label3->Text = "These existing files are oversized:"; else oc->label3->Text = "This existing file is oversized:";

					 if (::DialogResult::Cancel == oc->ShowDialog() ) return;

					 int action1 = ( oc->overwrite1->Checked * OVERWRITE ) + oc->skip1->Checked * SKIP;
					 int action2 = ( oc->overwrite2->Checked * OVERWRITE ) + oc->skip2->Checked * SKIP + oc->resume2->Checked*RESUME;
					 int action3 = ( oc->overwrite3->Checked * OVERWRITE ) + oc->skip3->Checked * SKIP;

					 for (int i=0; i<numitems; i++)
					 {
						 item=src_items[i];
						 overwrite_action[i]=OVERWRITE;
						 if (dest_exists[i])
						 {
							 if(overwrite_category[i]==0)  overwrite_action[i]=action1; else
								 if(overwrite_category[i]==1)  overwrite_action[i]=action2; else
									 if(overwrite_category[i]==2)  overwrite_action[i]=action3;

						 }
						 if (overwrite_action[i]==RESUME && dest_size[i]<2*resume_granularity) overwrite_action[i]=OVERWRITE; // (don't bother resuming tiny files).

						 if (overwrite_action[i]==OVERWRITE) current_offsets[i]=0; else
							 if (overwrite_action[i]==SKIP) {current_offsets[i]=item->size;num_skip++;} else
								 if (overwrite_action[i]==RESUME) current_offsets[i]=dest_size[i];
					 }
				 }
				 if (num_skip==numitems) return;


				 if (this->checkBox1->Checked)
					 this->set_turbo_mode(1); //TODO: error handling for turbo mode selection
				 else
					 this->set_turbo_mode(0);


				 CopyDialog^ copydialog = gcnew CopyDialog();
				 copydialog->cancelled=false;
				 //copydialog->showCopyDialog();
				 //copydialog->total_filesize = totalsize;
				 copydialog->total_start_time = time(NULL);
				 copydialog->current_start_time=0;
				 //copydialog->current_filesize = 0; 
				 //copydialog->current_offset=0;
				 copydialog->total_bytes_received=0;
				 copydialog->current_bytes_received=0;
				 copydialog->filesizes = src_sizes;
				 copydialog->current_offsets = current_offsets;
				 copydialog->dest_exists = dest_exists;
				 copydialog->dest_size = dest_size;
				 copydialog->dest_filename = dest_filename;
				 copydialog->src_items = src_items;
				 copydialog->topfield_items_by_folder = topfield_items_by_folder;
				 copydialog->overwrite_action = overwrite_action;
				 copydialog->numfiles=numitems;
				 copydialog->current_index=0;
				 copydialog->window_title="Copying File(s) ... [PC --> PVR]";
				 copydialog->current_file="Waiting for PVR...";
				 copydialog->turbo_mode = this->turbo_mode;

				 this->transfer_in_progress=true;
				 Thread^ thread = gcnew Thread(gcnew ParameterizedThreadStart(this,&Form1::transfer_to_PVR));
				 thread->Start(copydialog);
				 copydialog->showDialog_thread();
				 thread->Join();
				 this->transfer_in_progress=false;


				 //if (!copydialog->cancelled) item->Selected = false;


				 //if (copydialog->cancelled)
				 //{
				 //	 //copydialog->close_request_threadsafe();
				 //	 break;
				 // }

				 if (copydialog->file_error->Length > 0)
				 {
					 MessageBox::Show(this,copydialog->file_error,"Error",MessageBoxButtons::OK);						


				 }



				 // }   //(end loop over items to be copied)

				 this->loadTopfieldDir();
				 this->absorb_late_packets(2,200);
				 this->set_turbo_mode(0);
				 //copydialog->close_request_threadsafe();

			 }

	private: System::Void listView1_ItemActivate(System::Object^  sender, System::EventArgs^  e) {

				 ListView^ listview = (ListView^) sender;
				 Console::WriteLine("Activated (1)");
				 //ComputerItem^ item = (ComputerItem^) sender;
				 //Console::WriteLine(item->Text);

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 TopfieldItem^ item;
				 bool success=false;
				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<TopfieldItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) {success=true;break;};

				 }

				 if(success)
				 {
					 //String^ dir = Path::Combine(this->computerCurrentDirectory,item->Text);
					 String^ dir = this->topfieldCurrentDirectory+"\\"+item->Text;
					 this->setTopfieldDir(dir);
					 this->loadTopfieldDir();

				 }
				 else
				 {
					 this->ViewInfo(listview);
				 }



			 }
	private: System::Void toolStripButton6_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->loadTopfieldDir();
			 }

	private: System::Void listView_ColumnClick(System::Object^  sender, System::Windows::Forms::ColumnClickEventArgs^  e) {

				 //this.listView1.ListViewItemSorter = new ListViewItemComparer(e.Column);
				 // Call the sort method to manually sort.
				 //listView1.Sort();


				 cli::interior_ptr<int> sortcolumn;
				 ListView^ listview = safe_cast<ListView^>(sender);

				 String^ type;
				 if (listview==this->listView2) {
					 sortcolumn=&this->listView2SortColumn;
					 type = "PC";
				 }
				 else
				 {
					 sortcolumn = &this->listView1SortColumn; 
					 type = "PVR";
				 }
				 if (e->Column == *sortcolumn)
				 {
					 if (listview->Sorting == SortOrder::Ascending)
					 {
						 listview->Sorting = SortOrder::Descending;
						 settings->changeSetting(type+"_SortOrder","Descending");
					 }
					 else
					 {
						 listview->Sorting = SortOrder::Ascending;
						 settings->changeSetting(type+"_SortOrder","Ascending");
					 }
					 printf("%d\n",listview->Sorting);

				 }
				 else
				 {
					 listview->Sorting = SortOrder::Ascending;
					 settings->changeSetting(type+"_SortOrder","Ascending");
				 }
				 *sortcolumn = e->Column;
				 settings->changeSetting(type+"_SortColumn", e->Column.ToString());

				 listview->ListViewItemSorter = gcnew ListViewItemComparer(e->Column,listview->Sorting);

				 listview->Sort();
				 this->setListViewStyle(listview);
			 }
	private: System::Void toolStripButton7_Click(System::Object^  sender, System::EventArgs^  e) {

				 // Delete files on the Topfield

				 // Enumerate selected items (PVR)
				 System::Windows::Forms::DialogResult result;
				 DeleteConfirmation^ confirmation = gcnew Antares::DeleteConfirmation();

				 //Array::Resize( confirmation->textBox1->Lines,10);
				 //Windows::Forms::Label^ label;
				 //for (int i=0; i<10; i++)
				 //{
				 //	 System::Drawing::Size sz = confirmation->listBox1->GetPreferredSize(System::Drawing::Size(0,0));
				 //
				 //	confirmation->listBox1->Items->Add(i.ToString() + " is a very interesting number!!  " + sz.ToString());
				 //}


				 ListView^ listview = this->listView1;

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 TopfieldItem^ item;

				 int numfiles =0;
				 int numdirs=0;
				 long long totalsize = 0;

				 System::String^ conf_str;
				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<TopfieldItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) numdirs++; else numfiles++;
					 totalsize += item->size;
					 conf_str = item->filename;
					 if (item->isdir) conf_str = conf_str + "\\          [Folder -- Contents will be deleted!!!]";
					 confirmation->listBox1->Items->Add(conf_str);

				 }
				 if (numfiles+numdirs==0) return;
				 conf_str = "Delete the following";
				 //if (numfiles>1 || numdirs>1) conf_str = conf_str+"these"; else conf_str=conf_str+"this"; 
				 if (numfiles>0) conf_str+=" file";
				 if (numfiles>1) conf_str+="s";
				 if (numdirs>0)
				 {
					 if (numfiles>0) conf_str+=" and";
					 conf_str+=" folder";
					 if (numdirs>1) conf_str+="s";
				 }
				 conf_str+="?";

				 confirmation->label1->Text = conf_str;
				 Console::WriteLine(confirmation->Size);

				 confirmation->Height = min( confirmation->Height +(numfiles+numdirs-1)*confirmation->listBox1->ItemHeight,700);
				 Console::WriteLine(confirmation->Size);
				 result = confirmation->ShowDialog();

				 if (result!=Windows::Forms::DialogResult::Yes) return;


				 myEnum = selected->GetEnumerator();
				 long long total_bytes_received=0;
				 long long bytecount;
				 time_t startTime = time(NULL);
				 int r;


				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<TopfieldItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 //if (item->isdir) {continue;}   

					 bytecount=0;

					 String^ full_filename = item->directory + "\\" + item->filename;

					 char* path = (char*)(void*)Marshal::StringToHGlobalAnsi(full_filename);

					 r = do_hdd_del(this->fd, path);
					 Marshal::FreeHGlobal((System::IntPtr)(void*)path);

				 }
				 this->loadTopfieldDir();
				 this->absorb_late_packets(2,200);

			 }
	private: System::Void listView_AfterLabelEdit(System::Object^  sender, System::Windows::Forms::LabelEditEventArgs^  e) {
				 ListView^ listview = safe_cast<ListView^>(sender);
				 // User has finished editing a label
				 if (e->Label == nullptr)
					 Console::WriteLine(e->Item.ToString()+": No change was made");
				 else
				 {
					 Console::WriteLine(e->Item.ToString() + ": " + e->Label );

					 if (listview == this->listView1)
						 // Rename a file on the PVR
					 {

						 TopfieldItem^ item = safe_cast<TopfieldItem^>(listview->Items[e->Item]);
						 String^ old_full_filename = item->directory + "\\" + item->filename;
						 char* old_path = (char*)(void*)Marshal::StringToHGlobalAnsi(old_full_filename);
						 String^ new_filename = safeString(e->Label);
						 String^ new_full_filename = item->directory + "\\" + new_filename;
						 char* new_path = (char*)(void*)Marshal::StringToHGlobalAnsi(new_full_filename);
						 int r = do_hdd_rename(this->fd, old_path,new_path);

						 Marshal::FreeHGlobal((System::IntPtr)(void*)old_path);
						 Marshal::FreeHGlobal((System::IntPtr)(void*)new_path);

						 if (r==0) // success
						 {
							 this->loadTopfieldDir("",new_filename);
						 }
						 else
						 {
							 e->CancelEdit = true;
							 this->loadTopfieldDir("" );
						 }


					 }

					 else if (listview == this->listView2)
						 // Rename a file on the PC
					 {
						 ComputerItem^ item = safe_cast<ComputerItem^>(listview->Items[e->Item]);
						 String^ old_full_filename = item->directory + "\\" + item->filename;
						 String^ new_filename = safeString(e->Label);
						 String^ new_full_filename = item->directory + "\\" + new_filename;

						 if (String::Compare(old_full_filename, new_full_filename)!=0)
						 {

							 bool success=true;
							 try {
								 if (item->isdir)
								 {
									 Directory::Move(old_full_filename, new_full_filename);
								 }
								 else
								 {
									 File::Move(old_full_filename, new_full_filename); 
								 }
							 }
							 catch(...)
							 {
								 success=false;


							 }
							 if (!success)
							 {
								 e->CancelEdit = true;

								 MessageBox::Show(this,"An error occurred during rename.","Error.",MessageBoxButtons::OK);
								 //this->listView2->Items->Clear();
								 this->loadComputerDir();
							 }
							 else
							 {
								 this->loadComputerDir("",new_filename);
							 }


						 }


					 }



				 }

			 }

	private: System::Void listView_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
				 ListView^ listview = safe_cast<ListView^>(sender);
				 //Console::WriteLine(listview->DoubleBuffered);
				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
				 if (e->KeyCode == Keys::F2)    // Start editing filename if F2 is pressed
				 {
					 Console::WriteLine("F2 pressed!");


					 if (selected->Count == 1 && listview->LabelEdit == true)
					 {
						 ListViewItem^ item = selected[0];
						 item->BeginEdit();
					 }

					 return;
				 }
				 if (e->KeyCode == Keys::F5)          // F5 (Refresh)
				 {
					 if (listview == this->listView1)
						 this->loadTopfieldDir();
					 else
						 this->loadComputerDir();
				 }

				 if (e->KeyCode == Keys::Delete)
				 {
					 Console::WriteLine("Delete pressed!");
					 if (selected->Count >0)
					 {
						 if (listview == this->listView1)
						 {
							 toolStripButton7_Click(nullptr,nullptr);
						 }
						 else if (listview==this->listView2)
						 {
							 toolStripButton3_Click(nullptr,nullptr);
						 }
					 }
					 return;
				 }
				 if (!e->Alt && !e->Shift && e->Control && e->KeyCode==Keys::A)    // Select all
				 {

					 ListView::ListViewItemCollection^ items = listview->Items;
					 for (int i=0; i<items->Count; i++) items[i]->Selected=true;
					 return;
				 }

				 if (e->Alt && !e->Control && !e->Shift && e->KeyCode == Keys::Up)
				 {
					 if (listview == this->listView1)  this->topfieldUpDir();
					 else this->computerUpDir();
				 }

				 //Console::WriteLine(keystr);
				 //Console::WriteLine(e->KeyData);
				 // Console::WriteLine(e->KeyValue);
				 // Console::WriteLine(e->KeyCode);

			 }
			 int newTopfieldFolder(String^ dir)
			 {
				 int r;
				 char* path = (char*)(void*)Marshal::StringToHGlobalAnsi(dir);
				 r = do_hdd_mkdir(this->fd,path);
				 Marshal::FreeHGlobal((System::IntPtr)(void*)path);
				 return r;
			 }

	private: System::Void toolStripButton8_Click(System::Object^  sender, System::EventArgs^  e) {

				 // Clicked "New Folder", Topfield.
				 if (this->fd==NULL)
				 {
					 //this->toolStripStatusLabel1->Text="Topfield not connected.";
					 return; 
				 }

				 ListView^ listview = this->listView1;

				 ListView::ListViewItemCollection^ items = listview->Items;
				 TopfieldItem^ item;

				 String^ foldername;
				 String^ dir;
				 int r;
				 bool success = false;
				 for( int i=0; i<1000; i++)
				 {

					 foldername = "NewFolder" + i.ToString("D2");





					 System::Collections::IEnumerator^ myEnum = items->GetEnumerator();

					 bool clash=false;
					 while ( myEnum->MoveNext() )
					 {
						 item = safe_cast<TopfieldItem^>(myEnum->Current);


						 if (String::Compare(item->filename,foldername)==0)
						 {
							 clash=true;break;
						 }
					 }
					 if(clash==true) continue;



					 dir = this->topfieldCurrentDirectory + "\\"+ foldername; 

					 r=this->newTopfieldFolder(dir);
					 if (r!=0) this->toolStripStatusLabel1->Text="Error creating new folder.";
					 success=true;
					 break;
				 }
				 if (success)
					 this->loadTopfieldDir(foldername);
			 }

	private: System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (this->checkBox1->Checked)
					 settings->changeSetting("TurboMode","on");
				 else
					 settings->changeSetting("TurboMode","off");
			 }

	private: System::Void listView2_SelectionChanged_Finally(void)
			 {
				 this->listView2_selection_was_changed=false;
				 ListView^ listview = this->listView2;
				 String^ txt = "";
				 if(listview->SelectedItems->Count ==0 )
				 {
					 this->button1->Enabled = false;
				 }
				 else

				 {
					 this->button1->Enabled = true;


					 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
					 long long totalsize = 0;
					 int numfiles=0;
					 int numdirs=0;
					 for each (ComputerItem^ item in selected)
					 {
						 if (!item->isdir)
						 {
							 totalsize+=item->size;
							 numfiles++;
						 }
						 else numdirs++;
					 }



					 if (numfiles>1)
					 {
						 txt = "  Selected " + numfiles.ToString() +" files on PC  ( "+Antares::HumanReadableSize(totalsize)+" )";

						 if (numdirs>0)
						 {
							 txt = txt + "     and   "+numdirs.ToString();
							 if (numdirs>1) txt=txt+" folders"; else txt=txt+" folder";
							 txt=txt+" (size unknown) ";
						 }

					 }




				 }
				 this->toolStripStatusLabel1->Text = txt;

			 }



	private: System::Void listView1_SelectionChanged_Finally(void)
			 {
				 this->listView1_selection_was_changed=false;

				 ListView^ listview = this->listView1;
				 String^ txt = "";
				 if(listview->SelectedItems->Count ==0 )
				 {
					 this->button2->Enabled = false;
				 }
				 else

				 {
					 this->button2->Enabled = true;


					 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
					 long long totalsize = 0;
					 int numfiles=0;
					 int numdirs=0;
					 for each (TopfieldItem^ item in selected)
					 {
						 if (!item->isdir)
						 {
							 totalsize+=item->size;
							 numfiles++;
						 }
						 else numdirs++;
					 }



					 if (numfiles>1)
					 {
						 txt = "  Selected " + numfiles.ToString() +" files on PVR ( "+Antares::HumanReadableSize(totalsize)+" )";

						 if (numdirs>0)
						 {
							 txt = txt + "     and   "+numdirs.ToString();
							 if (numdirs>1) txt=txt+" folders"; else txt=txt+" folder";
							 txt=txt+" (size unknown) ";
						 }

					 }




				 }
				 this->toolStripStatusLabel1->Text = txt;

			 }

	private: System::Void listView2_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {


				 if (this->listView2_selection_was_changed==false)
				 {
					 ListViewSelectionDelegate^ d = gcnew ListViewSelectionDelegate(this, &Form1::listView2_SelectionChanged_Finally);
					 this->listView2_selection_was_changed=true;
					 this->BeginInvoke(d);//, gcnew array<Object^> { });
				 }

			 }

	private: System::Void listView1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (this->listView1_selection_was_changed==false)
				 {
					 ListViewSelectionDelegate^ d = gcnew ListViewSelectionDelegate(this, &Form1::listView1_SelectionChanged_Finally);
					 this->listView1_selection_was_changed=true;
					 this->BeginInvoke(d);//, gcnew array<Object^> { });
				 }

			 }


	private: System::Void toolStripButton9_Click(System::Object^  sender, System::EventArgs^  e) {
				 // "Cut" button pressed on the topfield side.
				 // Change colour of cut items, and record the filenames on the clipboard.   
				 ListView^ listview = this->listView1;
				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
				 int num = selected->Count;
				 if (num==0) return;
				 Array::Resize(this->TopfieldClipboard,num);

				 TopfieldItem^ item;

				 ListView::ListViewItemCollection^ items = listview->Items;
				 for (int i=0; i<items->Count; i++) 
				 {
					 items[i]->BackColor = this->normal_background_colour;
				 }

				 //System::String^ full_filename;
				 for (int i=0; i<num; i++)
				 {
					 item = safe_cast<TopfieldItem^>(selected[i]);
					 //full_filename = item->directory + "\\" + item->filename;
					 this->TopfieldClipboard[i]=item->filename;
					 //Console::WriteLine(full_filename);
					 item->BackColor = cut_background_colour;
					 //item->Selected = false; 
				 }
				 System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<TopfieldItem^>(myEnum->Current);
					 item->Selected=false;
				 }

				 this->TopfieldClipboardDirectory = item->directory;

			 }
	private: System::Void toolStripButton10_Click(System::Object^  sender, System::EventArgs^  e) {
				 //Someone pressed the "Paste" button (Topfield)
				 int numc = this->TopfieldClipboard->Length;
				 if (numc==0) return;
				 // Need to check and avoid the following:   1) pasting to same location as source  2) pasting inside a directory being moved


				 //actually I should probably just uncut if this happens.
				 if (String::Compare(this->topfieldCurrentDirectory, this->TopfieldClipboardDirectory)==0) 
				 {

					 Array::Resize(this->TopfieldClipboard,0);
					 ListView::ListViewItemCollection^ items = this->listView1->Items;
					 for (int i=0; i<items->Count; i++) 
					 {
						 items[i]->BackColor = this->normal_background_colour;
					 }
					 return;
				 }


				 bool bad_location = false;
				 String^ full_src_filename;
				 for (int i=0; i<numc; i++)
				 {
					 full_src_filename = this->TopfieldClipboardDirectory + "\\" + this->TopfieldClipboard[i]; 
					 if (this->topfieldCurrentDirectory->StartsWith(full_src_filename))
					 {bad_location=true; break;};
				 }
				 if (bad_location)
				 {
					 MessageBox::Show(
						 "Cannot paste to this location, since it is inside a folder being moved.", 
						 "", MessageBoxButtons::OK);
					 return;
				 }

				 array<bool>^ failed = gcnew array<bool>(numc);
				 String^ full_dest_filename;
				 int numfailed=0;
				 for (int i=0; i<numc; i++)
				 {

					 full_src_filename = this->TopfieldClipboardDirectory + "\\" + this->TopfieldClipboard[i]; 
					 full_dest_filename = this->topfieldCurrentDirectory + "\\" + this->TopfieldClipboard[i]; 

					 char* src_path = (char*)(void*)Marshal::StringToHGlobalAnsi(full_src_filename);
					 char* dest_path = (char*)(void*)Marshal::StringToHGlobalAnsi(full_dest_filename);
					 int r = do_hdd_rename(this->fd, src_path,dest_path);


					 if (r!=0) 

					 { failed[i]=true;numfailed++;}

					 else failed[i]=false;


					 Marshal::FreeHGlobal((System::IntPtr)(void*)src_path);
					 Marshal::FreeHGlobal((System::IntPtr)(void*)dest_path);

				 }

				 array<String^>^ newclip = gcnew array<String^>(numfailed);
				 int ind=0; for (int i=0; i<numc; i++) {if (failed[i]) {newclip[ind]=this->TopfieldClipboard[i];ind++;}};
				 this->TopfieldClipboard = newclip;

				 this->loadTopfieldDir();

			 }

			 //////////////////////////////

	private:  System::Void ViewInfo(ListView^ listview)
			  {
				  int type;
				  if (listview==this->listView1) type=0; else type=1;
				  ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
				  System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				  FileItem^ item;
				  tRECHeaderInfo ri;
				  const int readsize = 2048;

				  char charbuf[readsize]; 
				  while ( myEnum->MoveNext() )
				  {


					  item = safe_cast<FileItem^>(myEnum->Current);
					  if (item->isdir) continue;
					  String^ fname = item->directory + "\\" + item->filename;
					  if (type==1)
					  {
						  array<Byte>^ buffer = gcnew array<Byte>(readsize);
						  FileStream^ file = File::Open(fname,System::IO::FileMode::Open, System::IO::FileAccess::Read,System::IO::FileShare::Read);

						  int size = file->Read(buffer, 0, readsize);
						  file->Close();
						  if (size==readsize)
						  {

							  Marshal::Copy(buffer,0,System::IntPtr( &charbuf[0]),size);
							  HDD_DecodeRECHeader (charbuf, &ri);

							  // int j;
							  // printf("-----------------------\n");
							  // printf("HeaderSvcNumber = %d\n",ri.HeaderSvcNumber);
							  // printf("Event Duration = %02d:%02d\n",ri.EventDurationHour,ri.EventDurationMin);
							  // printf("Header duration = %d \n",ri.HeaderDuration);
							  // printf("SISvcName = %s\n",ri.SISvcName);
							  // printf("EventEventName = %s\n",ri.EventEventName);
							  // printf("EventEventDescription = %s\n",ri.EventEventDescription);
							  // printf("ExtEventText = %s\n",ri.ExtEventText);
							  // printf("-----------------------\n");

							  ProgInfo^ pi = gcnew ProgInfo(&ri,"Program Information, "+fname);

							  pi->ShowDialog(this);
							  break;
						  }
					  }
					  else  //type =0
					  {

						  array<Byte>^ buff = this->read_topfield_file_snippet(fname, 0);
						  int size = buff->Length;
						  if (size>=readsize)
						  {
							  Marshal::Copy(buff,0,System::IntPtr( &charbuf[0]),readsize);
							  HDD_DecodeRECHeader (charbuf, &ri);
							  ProgInfo^ pi = gcnew ProgInfo(&ri,"Program Information, "+fname);

							  pi->ShowDialog(this);
							  break;
						  }
					  }

				  }
			  }


	private: System::Void Info_Click(System::Object^  sender, System::EventArgs^  e) {
				 // Someone clicked "info" on either PVR or PC side
				 ListView^ listview;

				 if (sender == this->toolStripButton11)
					 listview = this->listView1;
				 else
					 listview = this->listView2;


				 this->ViewInfo(listview);
			 }


	private: array<Byte>^ read_topfield_file_snippet(String^ filename, long long offset) {    
				 // Read one packet's worth of a file on the Topfield, starting at specified offset.
				 // Return as an array of Bytes.


				 array<Byte>^ out_array = gcnew array<Byte>(0); 
				 struct tf_packet reply;
				 int r;
				 enum
				 {
					 START,
					 DATA,
					 ABORT
				 } state;

				 char* srcPath = (char*)(void*)Marshal::StringToHGlobalAnsi(filename);

				 if (offset==0) 
					 r = send_cmd_hdd_file_send(this->fd, GET, srcPath);   
				 else
					 r = send_cmd_hdd_file_send_with_offset(this->fd, GET, srcPath,offset);

				 Marshal::FreeHGlobal((System::IntPtr)(void*)srcPath);

				 if(r < 0)
				 {
					 return out_array;
				 }

				 state = START;




				 while(0 < (r = get_tf_packet(fd, &reply)))
				 {

					 switch (get_u32(&reply.cmd))
					 {
					 case DATA_HDD_FILE_START:
						 if(state == START)
						 {

							 send_success(fd);
							 state = DATA;
						 }
						 else
						 {
							 fprintf(stderr,
								 "ERROR: Unexpected DATA_HDD_FILE_START packet in state %d\n",
								 state);
							 send_cancel(fd);
							 state = ABORT;
						 }
						 break;

					 case DATA_HDD_FILE_DATA:
						 if(state == DATA)
						 {
							 __u64 offset = get_u64(reply.data);
							 __u16 dataLen =
								 get_u16(&reply.length) - (PACKET_HEAD_SIZE + 8);

							 // if( !quiet)
							 // {
							 //progressStats(bytecount, offset + dataLen, startTime);
							 // }

							 if(r < get_u16(&reply.length))
							 {
								 fprintf(stderr,
									 "ERROR: Short packet %d instead of %d\n", r,
									 get_u16(&reply.length));

							 }


							 Array::Resize(out_array,dataLen);
							 Marshal::Copy( IntPtr( (void*)  &reply.data[8] ) , out_array, 0,(int) dataLen);

							 send_cancel(fd);
							 state = ABORT;


						 }
						 else
						 {
							 fprintf(stderr,
								 "ERROR: Unexpected DATA_HDD_FILE_DATA packet in state %d\n",
								 state);
							 send_cancel(fd);
							 state = ABORT;

						 }
						 break;

					 case DATA_HDD_FILE_END:
						 send_success(fd);

						 printf("DATA_HDD_FILE_END\n");

						 state=ABORT;

						 break;

					 case FAIL:
						 fprintf(stderr, "ERROR: Device reports %s\n",
							 decode_error(&reply));
						 send_cancel(fd);
						 state = ABORT;

						 break;

					 case SUCCESS:
						 printf("SUCCESS\n");

						 break;

					 default:
						 fprintf(stderr, "ERROR: Unhandled packet (cmd 0x%x)\n",
							 get_u32(&reply.cmd));
					 }




					 if (state==ABORT) break;
				 }

				 this->absorb_late_packets(2,200);
				 return out_array;
			 }





	private: System::Void Form1_ResizeEnd(System::Object^  sender, System::EventArgs^  e) {
				 // Console::WriteLine("ResizeEnd");
				 //this->ResumeLayout();
			 }
	private: System::Void Form1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
				 // Console::WriteLine("Paint");
			 }
	private: System::Void Form1_ResizeBegin(System::Object^  sender, System::EventArgs^  e) {
				 //Console::WriteLine(this->topfieldSizeHeader->);
				 // this->SuspendLayout();
			 }
			 /////////////////////////////
	private: System::Void toolStripButton3_Click(System::Object^  sender, System::EventArgs^  e) {
				 //Delete files on the PC

				 // Enumerate selected items (PC)
				 System::Windows::Forms::DialogResult result;
				 DeleteConfirmation^ confirmation = gcnew Antares::DeleteConfirmation();

				 ListView^ listview = this->listView2;

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 ComputerItem^ item;

				 int numfiles =0;
				 int numdirs=0;
				 long long totalsize = 0;

				 System::String^ conf_str;
				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<ComputerItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) numdirs++; else numfiles++;
					 totalsize += item->size;
					 conf_str = item->filename;
					 if (item->isdir) conf_str = conf_str + "\\          [Folder -- Contents will be deleted!!!]";
					 confirmation->listBox1->Items->Add(conf_str);

				 }
				 if (numfiles+numdirs==0) return;
				 conf_str = "Delete the following";
				 //if (numfiles>1 || numdirs>1) conf_str = conf_str+"these"; else conf_str=conf_str+"this"; 
				 if (numfiles>0) conf_str+=" file";
				 if (numfiles>1) conf_str+="s";
				 if (numdirs>0)
				 {
					 if (numfiles>0) conf_str+=" and";
					 conf_str+=" folder";
					 if (numdirs>1) conf_str+="s";
				 }
				 conf_str+="?";

				 confirmation->label1->Text = conf_str;
				 Console::WriteLine(confirmation->Size);

				 confirmation->Height = min( confirmation->Height +(numfiles+numdirs-1)*confirmation->listBox1->ItemHeight,700);
				 Console::WriteLine(confirmation->Size);
				 result = confirmation->ShowDialog();

				 if (result!=Windows::Forms::DialogResult::Yes) return;


				 myEnum = selected->GetEnumerator();

				 bool error = false;

				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<ComputerItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);

					 String^ full_filename = item->directory + "\\" + item->filename;

					 try {
						 if (item->isdir)
						 {
							 Directory::Delete(full_filename,true);
						 }
						 else
						 {
							 File::Delete(full_filename);
						 }
					 }
					 catch(...)
					 {
						 error=true;
					 }

				 }

				 if (error)
				 {
					 MessageBox::Show(this,"An error occurred while deleting.","Error.",MessageBoxButtons::OK);


				 }
				 this->loadComputerDir();
			 }
			 /////////////////////////////
	private: System::Void toolStripButton4_Click(System::Object^  sender, System::EventArgs^  e) {
				 // New folder on the PC
				 if (String::Compare(this->computerCurrentDirectory, "")==0) 
					 return;
				 ListView^ listview = this->listView2;

				 ListView::ListViewItemCollection^ items = listview->Items;
				 //ComputerItem^ item;
				 //
				 String^ foldername;
				 String^ dir;
				 //int r;
				 bool success = false;
				 for( int i=0; i<100; i++)
				 {

					 foldername = "New Folder";
					 if (i>0) foldername = foldername + " ("+i.ToString()+")";
					 dir = this->computerCurrentDirectory + "\\" + foldername;

					 if (Directory::Exists(dir) || File::Exists(dir)) continue;

					 try{
						 Directory::CreateDirectory(dir);
						 success=true;
					 }
					 catch(...)
					 {
					 }

					 if (success) break;


				 }



				 this->loadComputerDir(foldername);
				 if (!success)
					 MessageBox::Show(this,"An error occurred while creating the new folder.","Error.",MessageBoxButtons::OK);

			 }

	};    // class form1
};    // namespace antares

