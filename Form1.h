#pragma once

#include "antares.h"

extern "C" {

	//#include <libusb/libusb.h>
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


	struct husb_device_handle;
	int find_usb_paths(char *dev_paths,  int *pids, int max_paths,  int max_length_paths, char *driver_names);
	struct husb_device_handle* open_winusb_device(HANDLE hdev);
	struct husb_device_handle* open_tfbulk_device(HANDLE hdev);
	char *windows_error_str(uint32_t retval);
	void husb_free(struct husb_device_handle *fd);

}


#using <mscorlib.dll>



extern FILE old_stdout;
extern FILE old_stderr;
extern FILE* hf;


#include "copydialog.h"
#include "deleteconfirmation.h"
#include "overwriteconfirmation.h"
#include "LowSpaceAlert.h"
#include "ProgInfo.h"
#include "Settings.h"
#include "SettingsDialog.h"

//ref class CopyDialog ;





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
	delegate void TopfieldSummaryCallback(void);
	delegate void CheckConnectionCallback(void);
	delegate void TransferEndedCallback(void);
	delegate void ComputerBackgroundCallback(void);
	delegate void TopfieldBackgroundCallback(void);




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



	enum OverwriteAction overwrite_action_type;

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
		// Form1 constructor
		Form1(void)
		{

			this->topfield_background_enumerator = nullptr;
			this->computer_background_enumerator = nullptr;
			//this->close_request=false;

			this->topfield_background_event = gcnew AutoResetEvent(false);
			this->computer_background_event = gcnew AutoResetEvent(false);


			this->locker = gcnew Object();
			this->last_topfield_freek = -1;
			this->last_topfield_freek_time = 0;
			this->pid=0;

			this->current_copydialog = nullptr;


			this->proginfo_cache = gcnew ProgramInformationCache();
			this->connection_needs_checking = true;

			icons = gcnew Antares::Icons();

			this->listView1SortColumn = -1;
			this->listView2SortColumn=-1;
			this->turbo_mode = gcnew System::Boolean;

			this->finished_constructing = 0;
			this->last_layout_x=-1;
			this->last_layout_y=-1;
			InitializeComponent();
			this->clist = this->listView2;
			this->tlist = this->listView1;

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
			this->topfieldChannelHeader = this->listView1->Columns->Add("Channel",120,HorizontalAlignment::Left);
			this->topfieldDescriptionHeader = this->listView1->Columns->Add("Description",120,HorizontalAlignment::Left);

			this->topfieldHeaders = gcnew array<ColumnHeader^>
			{topfieldNameHeader, topfieldSizeHeader, topfieldTypeHeader, topfieldDateHeader, topfieldChannelHeader, topfieldDescriptionHeader};

			this->computerNameHeader = this->listView2->Columns->Add("Name",140,HorizontalAlignment::Left);
			this->computerSizeHeader = this->listView2->Columns->Add("Size",70,HorizontalAlignment::Right);
			this->computerTypeHeader = this->listView2->Columns->Add("Type",60,HorizontalAlignment::Left);
			this->computerDateHeader = this->listView2->Columns->Add("Date",120,HorizontalAlignment::Left);
			this->computerChannelHeader = this->listView2->Columns->Add("Channel",120,HorizontalAlignment::Left);
			this->computerDescriptionHeader = this->listView2->Columns->Add("Description",120,HorizontalAlignment::Left);

			this->computerHeaders = gcnew array<ColumnHeader^>
			{computerNameHeader, computerSizeHeader, computerTypeHeader, computerDateHeader, computerChannelHeader, computerDescriptionHeader};


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


			int hist_len = this->settings->maximum_history_length;
			for (int j=0; j<hist_len; j++)
			{
				String^ key = "ComputerHistory"+j.ToString();
				String^ str = this->settings->getSettingOrNull(key);
				if (str==nullptr) break;
				this->textBox1->Items->Add(str);
			}
			this->add_path_to_history(this->textBox1, this->computerCurrentDirectory);
			for (int j=0; j<hist_len; j++)
			{
				String^ key = "TopfieldHistory"+j.ToString();
				String^ str = this->settings->getSettingOrNull(key);
				if (str==nullptr) break;
				this->textBox2->Items->Add(str);
			}
			this->textBox2->Select(0,0);






			this->listView2->ListViewItemSorter = gcnew ListViewItemComparer(this->listView2SortColumn,this->listView2->Sorting);
			this->listView1->ListViewItemSorter = gcnew ListViewItemComparer(this->listView1SortColumn,this->listView1->Sorting);




			this->TopfieldClipboard = gcnew array<String^> {};
			this->TopfieldClipboardDirectory = "";

			this->finished_constructing = 1;






			this->CheckConnection();
			this->last_layout_x = -1;this->last_layout_y=-1;
			this->Arrange();

			// Set double-buffering and image list on the ListViews
			this->setListViewStyle(listView1);
			this->setListViewStyle(listView2);


			this->Focus();
			this->listView2->Focus();


			this->cbthread = gcnew Thread(gcnew ThreadStart(this,&Form1::computerBackgroundWork));
			this->tbthread = gcnew Thread(gcnew ThreadStart(this,&Form1::topfieldBackgroundWork));

			cbthread->Name = "cbthread";
			tbthread->Name = "tbthread";

			this->cbthread->Start();
			this->tbthread->Start();


			this->loadTopfieldDir();
			this->loadComputerDir();
			//this->ResizeRedraw = true;




		}




		void absorb_late_packets(int count, int timeout)
		{
			int r;
			struct tf_packet reply;

			//return;
			if (this->fd==NULL) return;
			//printf("\nLate Packets:\n");
			for (int i=0; i<count; i++)

			{
				r = get_tf_packet2(this->fd, &reply,timeout, 1);
				//printf("r=%d   reply.cmd = %d \n",r,get_u32(&reply.cmd));

			}
		}





		void CheckConnection(void)
		{

			if (this->InvokeRequired)
			{
				CheckConnectionCallback^ d = gcnew CheckConnectionCallback(this, &Form1::CheckConnection);
				this->Invoke(d);
			}
			else
			{
				const int max_paths=10;
				const int paths_max_length=256;
				char dev_paths[max_paths][paths_max_length];
				char driver_names[max_paths][paths_max_length];
				char *device_path;
				int pids[max_paths];
				//int this_pid=-1;
				//HANDLE usb_handle;
				HANDLE hdev;
				husb_device_handle *fdtemp;
				fdtemp=NULL;
				String^ error_str = "PVR: Device not connected";
				if (this->fd != NULL) 
				{
					//MessageBox::Show(" husb_free,  "+( (int) this->fd).ToString());
					printf("husb_free, %ld \n",(long int) this->fd);
					husb_free((husb_device_handle*) (void*) this->fd);
					this->fd=NULL;
				};

				int ndev = find_usb_paths(&dev_paths[0][0],  pids, max_paths,  paths_max_length, &driver_names[0][0]);

				for (int j=0; j<ndev; j++)
				{
					device_path = &dev_paths[j][0];
					String^ driver_name = gcnew String( &driver_names[j][0]);
					driver_name = driver_name->ToLowerInvariant();
					Console::WriteLine("Driver: " + driver_name);
					if (driver_name->Equals("winusb"))
					{
						hdev = CreateFileA(device_path, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,  NULL);

					}
					else if (driver_name->Equals("tfbulk"))
					{

						hdev = CreateFileA(device_path, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,  NULL);
					}
					else
					{
						error_str = "PVR: Error -- wrong driver installed.";
						continue;
					}

					if (hdev==INVALID_HANDLE_VALUE)
					{
						DWORD last_error = GetLastError();
						printf("%s\n",windows_error_str(last_error));
						if (last_error==ERROR_ACCESS_DENIED) 
							error_str="PVR: Error -- already in use.";

						continue;
					}
					else
						printf("  CreateFile seemed to return successfully.\n");


					if (driver_name->Equals("winusb"))
						fdtemp  = open_winusb_device(hdev);
					else
						fdtemp  = open_tfbulk_device(hdev);

					//bResult = WinUsb_Initialize(deviceHandle, &usbHandle);

					if (fdtemp!=NULL)

					{
						this->pid=pids[j];
						break;
					}



				}

				//this->fd=NULL;
				printf("this->fd = %ld   fdtemp=%ld  \n",(long int) this->fd, (long int) fdtemp);
				this->fd = (libusb_device_handle *) (void*) fdtemp;

				if (this->fd !=NULL)
				{
					this->loadTopfieldDir();
				}
				else
				{
					this->label2->Text = error_str;
					this->listView1->Items->Clear();
					this->listView1->Tag="";
					this->topfield_background_enumerator=nullptr;
				}
			}
		}


		void CheckConnection_old(void)
		{
			if (this->InvokeRequired)
			{
				CheckConnectionCallback^ d = gcnew CheckConnectionCallback(this, &Form1::CheckConnection);
				this->Invoke(d);
			}
			else
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
				bool threadsafe = true;//! this->InvokeRequired;
				int pid = 0;

				//winusb_experiment();
				//int find_usb_paths(char **dev_paths,  int max_paths,  int max_length_paths);
				for (i=0; i<cnt; i++)
				{
					dev=devs[i];
					r = libusb_get_device_descriptor(dev, &desc);

					printf("%04x:%04x\n",desc.idVendor,desc.idProduct);
					if (desc.idVendor==0x11db && ( desc.idProduct == 0x1000 || desc.idProduct == 0x1100) )
						//if (desc.idVendor==0x11db &&  desc.idProduct ,0x1100 )

					{
						device=dev;
						pid=desc.idProduct;
					}
					else {continue;};

					if (this->fd != NULL) break;

					r=libusb_open(device,&dh);
					if (r) {
						printf("New open call failed.\n");
						device=NULL;
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
				this->pid=pid;
				if (threadsafe) this->loadTopfieldDir();

				return;
			}
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

		int wait_for_space_on_pc(Antares::CopyDialog^ copydialog, String^ path)
		{
			array<long long int>^ fs = this->computerFreeSpace(path);
			if (fs[0]>1024*1024) return -2;

			//Antares::TaskbarState::setError(this);
			copydialog->set_error( " NO SPACE LEFT ON PC. Retrying . . . ");
			if (*this->turbo_mode ) this->set_turbo_mode(0);

			copydialog->reset_rate();
			copydialog->update_dialog_threadsafe();

			while(1)
			{
				if (copydialog->cancelled) {return -1;};
				fs = this->computerFreeSpace(path);
				if (fs[0]<1024*1024)
				{
					Thread::Sleep(2000);
				}
				else
				{
					if ( copydialog->turbo_request != *this->turbo_mode)
						this->set_turbo_mode(copydialog->turbo_request);
					copydialog->clear_error();
					//Antares::TaskbarState::setNormal(this);
					return 0;

				}


			}



		}


		int wait_for_connection(Antares::CopyDialog^ copydialog)
		{

			//Antares::TaskbarState::setError(this);
			if (copydialog->usb_error)
				copydialog->set_error( " ERROR CONNECTING TO THE PVR. Retrying . . . ");
			if (copydialog->freespace_check_needed)
			{
				copydialog->current_file = "Checking free space on PVR...";

				copydialog->update_dialog_threadsafe();
				goto check_freespace;
			}
			copydialog->reset_rate();
			copydialog->update_dialog_threadsafe();
			int r,ret=0;

wait_again:
			while(1)
			{
				this->absorb_late_packets(4,100);
				Thread::Sleep(500);
				r=this->tf_init();
				if (copydialog->cancelled) {ret=-1;break;};
				if (r==0) {ret=0;break;};

				//System::Threading::Thread::Sleep(1000);
				this->CheckConnection();
				if (copydialog->cancelled) {ret=-1;break;};

			}

check_freespace:

			if (ret==0 && copydialog->copydirection == CopyDirection::PC_TO_PVR)
			{


				TopfieldFreeSpace freespace = this->getTopfieldFreeSpace();
				if (freespace.freek<0) goto wait_again;

				printf("freek = %d\n",freespace.freek);
				if (freespace.freek < this->topfield_minimum_free_megs*1024.0) {
					printf("copydialog->cancelled %d \n",copydialog->cancelled);
					copydialog->set_error( " NO SPACE LEFT ON PVR. Retrying . . . ");
					copydialog->reset_rate();
					copydialog->update_dialog_threadsafe();
					this->set_turbo_mode(0);

					for (int j=0; j<5; j++)
					{
						Thread::Sleep(1000);
						if (copydialog->cancelled) {ret=-1;break;};
					}

					goto wait_again;
				}

			}

			this->set_turbo_mode(copydialog->turbo_request);

			if (ret==0)
				copydialog->clear_error();

			//Antares::TaskbarState::setNormal(this);
			return ret;
		}

		int set_turbo_mode(int turbo_on)
		{

			//return 0;
			//turbo_on=0;
			*this->turbo_mode = (turbo_on!=0);
			int r;
			printf("\nSetting turbo mode: %d\n",turbo_on);  
			struct tf_packet reply;

			if (this->fd == NULL) return -1;

			//this->absorb_late_packets(2,100);
			r = send_cmd_turbo(fd, turbo_on);
			if(r < 0)
			{
				this->absorb_late_packets(2,100);
				this->connection_error_occurred();
				return -EPROTO;
			}

			r = get_tf_packet(this->fd, &reply);
			if(r < 0)
			{
				this->absorb_late_packets(2,100);
				this->connection_error_occurred();
				return -EPROTO;
			}

			switch (get_u32(&reply.cmd))
			{
			case SUCCESS:
				trace(1,
					fprintf(stderr, "Turbo mode: %s\n",
					turbo_on ? "ON" : "OFF"));
				this->absorb_late_packets(2,100);

				return 0;
				break;

			case FAIL:
				fprintf(stderr, "ERROR: Device reports %s in set_turbo_mode\n",
					decode_error(&reply));
				this->connection_error_occurred();
				break;

			default:
				fprintf(stderr, "ERROR: Unhandled packet (in set_turbo_mode) cmd=%d\n",&reply.cmd);
				this->connection_error_occurred();
				this->absorb_late_packets(2,100);
			}
			this->absorb_late_packets(2,100);
			this->connection_error_occurred();
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


			try 
			{
				list = System::IO::Directory::GetFileSystemEntries(dir);
			}
			catch(...)
			{
				array<ComputerItem^>^ items = gcnew array<ComputerItem^>(0);
				return items;
			}

			array<ComputerItem^>^ items = gcnew array<ComputerItem^>(list->Length);
			for (j=0; j<list->Length; j++)
			{

				items[j] = gcnew ComputerItem(list[j], dir);
			}
			return items;
		}

		array<long long int>^ computerFreeSpace(String^ path)
		{
			// Return a two-element array {free space, total size};
			array<long long int>^ ret={-1,-1};
			try
			{
				String^ str = Path::GetPathRoot(path);
				if (str->Length ==0) return ret;
				DriveInfo^ drive = gcnew DriveInfo(str);

				ret[0]=drive->AvailableFreeSpace;
				ret[1]=drive->TotalSize;
				return  ret;
			}
			catch(...)
			{
				return ret;
			}

		}		



		void computerBackgroundWork(void)
		{

			try {
				while(1)
				{
repeat:
					this->computer_background_event->WaitOne();


					System::Collections::IEnumerator ^en = this->computer_background_enumerator;
					if (en==nullptr) continue;



					while(en->MoveNext())
					{
						ComputerItem^ item = safe_cast<ComputerItem^>(en->Current);
						FileType^ info = this->icons->GetCachedIconIndex(item->full_filename, false, item->isdir);

						int ic = info->icon_index;
						if (ic>=0 && (ic != item->icon_index || (!item->isdir && info->file_type != item->file_type)))
						{
							int was=item->icon_index;
							item->icon_index=ic;
							if (!item->isdir) item->file_type = info->file_type;
							ComputerBackgroundCallback ^d = gcnew ComputerBackgroundCallback(item, &FileItem::update_icon);
							this->Invoke(d);
							//Console::WriteLine("Invoke for icon " + ic.ToString()+", was " + was.ToString()+". ext="+Path::GetExtension(item->full_filename));
							Application::DoEvents();  // why is this needed?

						}
						if (en!= this->computer_background_enumerator)
						{
							goto repeat;

						}
					}

					if (! (this->settings["PC_Column4Visible"]=="1" || this->settings["PC_Column5Visible"]=="1")) continue;
					en->Reset();

					tRECHeaderInfo ri;

					while (en->MoveNext())
					{ 
						ComputerItem^ item = safe_cast<ComputerItem^>(en->Current);
						//Console::WriteLine(item->full_filename);

						if (!item->isdir && this->proginfo_cache->query(item)==nullptr)
						{
							this->loadInfo(item,&ri);
							this->proginfo_cache->add(item);
							if (! (item->channel == "")  || ! (item->description == "") )
							{
								ComputerBackgroundCallback ^d = gcnew ComputerBackgroundCallback(item, &FileItem::update_program_information);
								this->Invoke(d);
								//printf("Invoking something...\n");

							}
						}
						if (en != this->computer_background_enumerator) {goto repeat;};
						//ComputerBackgroundCallback ^d = gcnew ComputerBackgroundCallback(this, &Form1::computerBackgroundWork);


						//this->BeginInvoke(d);
						//Application::DoEvents();


					}


				}

			}
			catch (...)
			{
			}

		}


		void  topfieldBackgroundWork(void)
		{

			try{

				while(1)
				{
repeat:

					this->topfield_background_event->WaitOne();
					printf("topfieldBackgroundWork iter.\n");
					System::Collections::IEnumerator ^en = this->topfield_background_enumerator;
					if (en==nullptr) continue;


					while(en->MoveNext())
					{
						TopfieldItem^ item = safe_cast<TopfieldItem^>(en->Current);
						FileType^ info = this->icons->GetCachedIconIndex("T:"+safeString(item->directory)+"\\"+item->safe_filename, true, item->isdir);
						int ic = info->icon_index;
						if (ic>=0 && (ic != item->icon_index || (!item->isdir && info->file_type != item->file_type )) )
						{
							item->icon_index=ic;
							if (!item->isdir) item->file_type=info->file_type;
							TopfieldBackgroundCallback ^d = gcnew TopfieldBackgroundCallback(item, &FileItem::update_icon);
							this->Invoke(d);
							printf("Invoke, ic=%d ext=",ic); Console::WriteLine(Path::GetExtension(safeString(item->directory)+"\\"+item->safe_filename));
							Application::DoEvents();  // why is this needed?

						}
						if (en!= this->topfield_background_enumerator)
						{
							goto repeat;

						}
					}

					if (! (this->settings["PVR_Column4Visible"]=="1" || this->settings["PVR_Column5Visible"]=="1")) continue;

					en->Reset();
					tRECHeaderInfo ri;
					while (en->MoveNext())
					{ 
						TopfieldItem^ item = safe_cast<TopfieldItem^>(en->Current);
						//Console::WriteLine(item->full_filename);

						if (!item->isdir && this->proginfo_cache->query(item)==nullptr)
						{
							ri.readsize=0;
							this->loadInfo(item,&ri);
							Console::WriteLine(item->full_filename);
							printf(" item->size = %d,  readsize=%d\n",(int)item->size, (int)ri.readsize);
							//if (!(item->size > 2048 && ri.readsize<2048))
							this->proginfo_cache->add(item);
							if (! (item->channel == "")  || ! (item->description == "") )
							{
								TopfieldBackgroundCallback ^d = gcnew TopfieldBackgroundCallback(item, &FileItem::update_program_information);
								this->Invoke(d);
								Application::DoEvents();  // why is this needed?
								//printf("Invoking something... (TF)\n");

							}

						}
						//Console::WriteLine(en->GetHashCode()); Console::WriteLine(this->topfield_background_enumerator->GetHashCode());

						if (en!= this->topfield_background_enumerator)
						{
							goto repeat;

						}
						//TopfieldBackgroundCallback ^d = gcnew TopfieldBackgroundCallback(this, &Form1::topfieldBackgroundWork);


						//if (this->transfer_in_progress) return;

						//this->BeginInvoke(d);
						//Application::DoEvents();
					}

				}


			}
			catch (...)    //Todo: find out why there is an exception here when you close window while bkgnd work pending.
			{

				printf("The topfieldBackgroundWork thread stopped!!!!!!!!!!!\n");
			}


		}


		void updateListViewItems(ListView^ listview,  array<FileItem^>^ newitems)
		{
			int j;
			FileItem^ item;
			FileItem^ item2;
			ListView::ListViewItemCollection^ olditems = listview->Items;

			int oldcount = olditems->Count;
			int newcount = newitems->Length;
			Dictionary< String^, FileItem^>^ to_remove = gcnew Dictionary<String^, FileItem^>(oldcount, StringComparer::Ordinal);
			Dictionary< String^, FileItem^>^ to_add    = gcnew Dictionary<String^, FileItem^>(newcount, StringComparer::Ordinal);

			for (j=0; j<oldcount; j++)
			{
				item = static_cast<FileItem^>(olditems[j]);
				try {
					to_remove->Add(item->full_filename, item);
				}
				catch(...)
				{
				}
			}

			for (j=0; j<newcount; j++)
			{
				item = newitems[j];
				if (to_remove->ContainsKey(item->full_filename))        // This new item matches an old item
				{

					item2 = to_remove[item->full_filename];
					try
					{
						to_remove->Remove(item->full_filename);
					}
					catch(...)
					{

					}



					// update item2 with details from item.

					item2->update(item);


				}
				else                // This new item has no matching old item
				{
					try
					{
						//to_add->Add(item->full_filename, item);
						listview->Items->Add(item);
					}
					catch(...)
					{
					}


				}

			}


			for each (FileItem^ it in to_remove->Values)
			{
				listview->Items->Remove(it);
			}




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




			bool reloaded;
			try{

				reloaded = (dir == this->clist->Tag);
			}
			catch (...)
			{
			}




			ListViewItem^ rename_item=nullptr;
			ListViewItem^ select_item=nullptr;

			if (dir->Equals(""))  // List drives
			{
				this->listView2->Items->Clear();
				DWORD drives = GetLogicalDrives();
				for (j=0; j<26; j++)
				{
					if ( (drives&1)==1)
					{
						item = gcnew ComputerItem(j);
						FileType^ info = this->icons->GetCachedIconIndex(item->full_filename);
						int ic = info->icon_index;
						if (ic>=0)
							item->ImageIndex=item->icon_index=ic;
						else
							item->ImageIndex=item->icon_index=this->icons->folder_index;
						this->listView2->Items->Add(item);
					}

					drives>>=1;
				}
				this->label1->Text = "My Computer";
				settings->changeSetting("ComputerDir","");
				this->clist->Tag = "";
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
					FileType^ info = this->icons->GetCachedIconIndexFast(item->full_filename,false, item->isdir);
					int ic = info->icon_index;
					if (ic >= 0)
					{
						item->ImageIndex = item->icon_index = ic;
						if (!item->isdir)
						{
							item->file_type = info->file_type;
							item->SubItems[2]->Text = info->file_type;
						}

					}
					else
					{
						if (item->isdir)
							item->ImageIndex = item->icon_index= this->icons->folder_index;
						else
							item->ImageIndex = item->icon_index= this->icons->file_index;
					}

					CachedProgramInformation^ pi = this->proginfo_cache->query(item);
					if (pi!=nullptr) pi->apply_to_item(item);

					if (item->filename == start_rename)
					{
						rename_item=item;
					}

					if (item->filename == name_to_select)
					{
						select_item = item;
					}




				}


				if (reloaded)
				{
					this->updateListViewItems(clist,safe_cast<array<FileItem^>^>(items));
				}
				else

				{
					this->listView2->BeginUpdate();
					this->listView2->Items->Clear();

					this->listView2->Items->AddRange(items);

					this->listView2->EndUpdate();
				}

				this->textBox1->Select(0,0);
				this->clist->Tag = dir;


				settings->changeSetting("ComputerDir",dir);
				// Add a drive summary to label1:
				array<long long int>^ freespaceArray = this->computerFreeSpace(dir);
				if (freespaceArray[0] > -1)
				{
					String ^str = " Local Disk "+str+"  --  " + HumanReadableSize(freespaceArray[0]) + " Free / " + HumanReadableSize(freespaceArray[1])+ " Total";

					label1->Text = str;

				}
				else
				{
					label1->Text = "";
				}
				ListView::ListViewItemCollection^ q = this->listView2->Items; 


				if (rename_item) 
				{
					this->clist->SelectedItems->Clear();
					rename_item->BeginEdit();
				}
				else 
				{
					if (nullptr!=select_item )
					{
						select_item->Selected=true;
						select_item->Focused=true;
						select_item->EnsureVisible();
					}
					else
					{

						if (!reloaded && q->Count>0) {q[0]->Selected=true;q[0]->Focused=true;};
					}
				}


				this->computer_background_enumerator = q->GetEnumerator();
				this->computer_background_event->Set();
				/*
				if (this->settings["PC_Column4Visible"]=="1" || this->settings["PC_Column5Visible"]=="1")
				{
				//ComputerBackgroundCallback ^d = gcnew ComputerBackgroundCallback(this, &Form1::computerBackgroundWork);
				//this->BeginInvoke(d);
				this->computer_background_event->Set();

				}
				*/




			}
			this->listView2->Tag = dir;
			if (!rename_item) this->Arrange2();

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
			this->add_path_to_history(this->textBox1, this->computerCurrentDirectory);
		}

		void topfieldUpDir(void)
		{
			if (this->fd == NULL) return;

			array<String^>^ parts = TopfieldFileParts(this->topfieldCurrentDirectory);

			this->setTopfieldDir(parts[0]);

			this->loadTopfieldDir("",parts[1]);
			this->add_path_to_history(this->textBox2, this->topfieldCurrentDirectory);
		}



		// Load the specified topfield directory into an array of TopfieldItems
		array<TopfieldItem^>^ loadTopfieldDirArrayOrNull(String^ path)               
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
				return nullptr;//items; 
			}
			absorb_late_packets(1,50);
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
					fprintf(stderr, "ERROR: Device reports %s in loadTopfieldDirArray, path %s\n",
						decode_error(&reply),path);
					this->connection_error_occurred();
					return nullptr;//items;

					break;
				default:
					fprintf(stderr, "ERROR: Unhandled packet\n");
					this->absorb_late_packets(4,100);
					this->connection_error_occurred();
					return nullptr;//items;
				}

			}
			return items;
		}

		array<TopfieldItem^>^ loadTopfieldDirArray(String^ path)       
		{
			array<TopfieldItem^>^ items = this->loadTopfieldDirArrayOrNull(path);
			if (items==nullptr)
				items = gcnew array<TopfieldItem^>(0);

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
			absorb_late_packets(2,50);
			int r;
			tf_packet reply;
			TopfieldFreeSpace v;

			v.valid = false;
			r = send_cmd_hdd_size(fd);
			if(r < 0)
			{
				this->connection_error_occurred();
				return v;
			}

			r = get_tf_packet(fd, &reply);
			if(r < 0)
			{
				this->connection_error_occurred();
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
				fprintf(stderr, "ERROR: Device reports %s in getTopfieldFreeSpace\n",
					decode_error(&reply));
				this->connection_error_occurred();
				break;

			default:
				fprintf(stderr, "ERROR: Unhandled packet in load_topfield_dir/hdd_size\n");
				this->connection_error_occurred();
				this->absorb_late_packets(4,100);
			}
			this->last_topfield_freek = v.freek;
			this->last_topfield_freek_time = time(NULL);
			this->last_topfield_freespace = v;
			this->bytes_sent_since_last_freek = 0;
			return v;
		}

		void updateTopfieldSummary(void)
		{
			if (this->InvokeRequired)
			{
				TopfieldSummaryCallback^ d = gcnew TopfieldSummaryCallback(this, &Form1::updateTopfieldSummary);

				this->Invoke(d);
			}
			else
			{

				//TopfieldFreeSpace v = getTopfieldFreeSpace();

				TopfieldFreeSpace v = this->last_topfield_freespace;
				if (v.valid)
				{
					String^ str=" Topfield device";

					if (this->pid==0x1100) str="Topfield second device";
					this->label2->Text = str+"  --  "+HumanReadableSize(1024* ((__u64) v.freek))+" Free / " + HumanReadableSize(1024*( (__u64) v.totalk)) + " Total";
				}
			}

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


			String^ dir = this->topfieldCurrentDirectory;

			bool reloaded = false;

			try{
				reloaded = (dir == this->tlist->Tag);
			}
			catch(...)
			{
			}

			if (this->fd==NULL)
			{
				//toolStripStatusLabel1->Text="Topfield not connected.";
				return -EPROTO; 
			}



			///// Actually load the directory
			Monitor::Enter(this->locker);
			this->getTopfieldFreeSpace();
			items = this->loadTopfieldDirArrayOrNull(dir);
			Monitor::Exit(this->locker);
			/////
			this->updateTopfieldSummary();

			if (items==nullptr)   //TODO: think about better error handling
			{
				this->tlist->Tag="";
				return -1;
			}

			for(i = 0; i < items->Length ; i++)
			{


				item = items[i];

				if (item->isdir)
					item->ImageIndex = item->icon_index=this->icons->folder_index;
				else
				{
					FileType^ info = this->icons->GetCachedIconIndexFast("T:"+safeString(item->directory)+"\\"+item->safe_filename, true,false);


					item->ImageIndex = item->icon_index = info->icon_index;	
					item->file_type = info->file_type;
					item->SubItems[2]->Text = info->file_type;
				}

				if (String::Equals(start_rename,item->filename) && !String::Equals(start_rename,"") )
				{
					rename_item=item; do_rename=true;
				}

				if (String::Compare(item->filename, name_to_select)==0 && !String::Equals(name_to_select,"") )
				{
					select_item = item; do_select=true;
				}



				CachedProgramInformation^ pi = this->proginfo_cache->query(item);
				if (pi!=nullptr) pi->apply_to_item(item);

				if (dir == this->TopfieldClipboardDirectory)
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
			settings->changeSetting("TopfieldDir",dir);//TODO: don't do this when error in directory load
			this->tlist->Tag=dir;

			if (reloaded)
			{
				this->updateListViewItems(this->tlist, items);
			}
			else
			{
				this->listView1->BeginUpdate();
				this->listView1->Items->Clear();
				this->listView1->Items->AddRange(items);
				this->listView1->EndUpdate();
			}
			this->textBox2->Select(0,0);
			ListView::ListViewItemCollection^ q = this->listView1->Items; 
			if (do_rename) 
			{
				this->tlist->SelectedItems->Clear();
				rename_item->BeginEdit();


				//TopfieldBackgroundCallback ^d = gcnew TopfieldBackgroundCallback(rename_item, &TopfieldItem::BeginEdit);
				//this->BeginInvoke(d);

			}
			else 
			{
				if (do_select)
				{
					select_item->Selected=true;
					select_item->Focused=true;
					select_item->EnsureVisible();
				}
				else
				{

					if (!reloaded && q->Count>0) {q[0]->Selected=true;q[0]->Focused=true;};
				}
			}


			this->topfield_background_enumerator = q->GetEnumerator();
			this->topfield_background_event->Set();
			/*
			if (this->settings["PVR_Column4Visible"]=="1" || this->settings["PVR_Column5Visible"]=="1")
			{
			this->topfield_background_enumerator = q->GetEnumerator();
			//TopfieldBackgroundCallback ^d = gcnew TopfieldBackgroundCallback(this, &Form1::topfieldBackgroundWork);
			//this->BeginInvoke(d);

			this->topfield_background_event->Set();

			}

			*/

			if (!do_rename) this->Arrange2();
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



		/// Form1 member variables
	public: libusb_device_handle *fd;
			AutoResetEvent^ topfield_background_event;
			AutoResetEvent^ computer_background_event;
			Thread^ tbthread;
			Thread^ cbthread;
			Object^ locker; 
			int dircount;
			System::Windows::Forms::ColumnHeader^ topfieldNameHeader;
			System::Windows::Forms::ColumnHeader^ topfieldSizeHeader;
			System::Windows::Forms::ColumnHeader^ topfieldTypeHeader;
			System::Windows::Forms::ColumnHeader^ topfieldDateHeader;
			System::Windows::Forms::ColumnHeader^ topfieldChannelHeader;
			System::Windows::Forms::ColumnHeader^ topfieldDescriptionHeader;

			array<System::Windows::Forms::ColumnHeader^>^ topfieldHeaders;

			System::Windows::Forms::ColumnHeader^ computerNameHeader;
			System::Windows::Forms::ColumnHeader^ computerSizeHeader;
			System::Windows::Forms::ColumnHeader^ computerTypeHeader;
			System::Windows::Forms::ColumnHeader^ computerDateHeader;
			System::Windows::Forms::ColumnHeader^ computerChannelHeader;
			System::Windows::Forms::ColumnHeader^ computerDescriptionHeader;

			array<System::Windows::Forms::ColumnHeader^>^ computerHeaders;


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

			int last_topfield_freek;
			time_t last_topfield_freek_time;
			long long bytes_sent_since_last_freek;
			TopfieldFreeSpace last_topfield_freespace;

			static double topfield_minimum_free_megs = 150.0; // Antares won't let the free space get lower than this.
			static double worst_case_fill_rate = 4.5;  // Worst case MB/sec rate that we can imagine the topfield HD being filled up

			int pid;

			CopyDialog^ current_copydialog;

			// used for updating the program details columns in the background;
			System::Collections::IEnumerator ^topfield_background_enumerator;
			System::Collections::IEnumerator ^computer_background_enumerator;

			ProgramInformationCache^ proginfo_cache;
			ListView^ clist;
			ListView^ tlist;

			bool connection_needs_checking;
			//bool close_request;



	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::Panel^  panel1;










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









	private: 







	private: System::Windows::Forms::ToolStripButton^  toolStripButton12;

	private: System::Windows::Forms::NotifyIcon^  notifyIcon1;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton13;


	private: System::Windows::Forms::Panel^  panel7;
	private: System::Windows::Forms::RadioButton^  radioButton2;
	private: System::Windows::Forms::RadioButton^  radioButton1;
	private: System::Windows::Forms::Panel^  panel8;
	private: System::Windows::Forms::Panel^  panel3;

	private: 
	private: System::Windows::Forms::CheckBox^  checkBox1;
	public: 
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::ToolStrip^  toolStrip2;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton5;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton6;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton7;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton8;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton9;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton10;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
	private: System::Windows::Forms::ToolStripButton^  toolStripButton11;

	public: System::Windows::Forms::ListView^  listView1;
	private: System::Windows::Forms::ToolTip^  toolTip1;
	public: System::Windows::Forms::ComboBox^  textBox2;
	private: 
	public: System::Windows::Forms::ComboBox^  textBox1;
	private: System::Windows::Forms::CheckBox^  checkBox2;
	public: 

	public: 


	public: 
	private: 












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
			this->textBox2 = (gcnew System::Windows::Forms::ComboBox());
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
			this->checkBox2 = (gcnew System::Windows::Forms::CheckBox());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->panel7 = (gcnew System::Windows::Forms::Panel());
			this->radioButton2 = (gcnew System::Windows::Forms::RadioButton());
			this->radioButton1 = (gcnew System::Windows::Forms::RadioButton());
			this->panel8 = (gcnew System::Windows::Forms::Panel());
			this->panel4 = (gcnew System::Windows::Forms::Panel());
			this->textBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripButton1 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton2 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton3 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton4 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton13 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton12 = (gcnew System::Windows::Forms::ToolStripButton());
			this->listView2 = (gcnew System::Windows::Forms::ListView());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->basicIconsSmall = (gcnew System::Windows::Forms::ImageList(this->components));
			this->notifyIcon1 = (gcnew System::Windows::Forms::NotifyIcon(this->components));
			this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->statusStrip1->SuspendLayout();
			this->panel1->SuspendLayout();
			this->panel3->SuspendLayout();
			this->toolStrip2->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel7->SuspendLayout();
			this->panel4->SuspendLayout();
			this->toolStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip1
			// 
			this->statusStrip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->toolStripStatusLabel1});
			this->statusStrip1->Location = System::Drawing::Point(0, 580);
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
			this->panel1->Size = System::Drawing::Size(880, 580);
			this->panel1->TabIndex = 10;
			// 
			// panel3
			// 
			this->panel3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel3->Controls->Add(this->textBox2);
			this->panel3->Controls->Add(this->checkBox1);
			this->panel3->Controls->Add(this->label2);
			this->panel3->Controls->Add(this->toolStrip2);
			this->panel3->Controls->Add(this->listView1);
			this->panel3->Dock = System::Windows::Forms::DockStyle::Left;
			this->panel3->Location = System::Drawing::Point(0, 0);
			this->panel3->Margin = System::Windows::Forms::Padding(0, 3, 0, 3);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(495, 580);
			this->panel3->TabIndex = 8;
			// 
			// textBox2
			// 
			this->textBox2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->textBox2->FlatStyle = System::Windows::Forms::FlatStyle::System;
			this->textBox2->Font = (gcnew System::Drawing::Font(L"Lucida Console", 10));
			this->textBox2->ForeColor = System::Drawing::Color::Navy;
			this->textBox2->FormattingEnabled = true;
			this->textBox2->Location = System::Drawing::Point(9, 68);
			this->textBox2->Name = L"textBox2";
			this->textBox2->Size = System::Drawing::Size(486, 21);
			this->textBox2->TabIndex = 12;
			this->textBox2->Text = L"\\ProgramFiles";
			this->textBox2->SelectionChangeCommitted += gcnew System::EventHandler(this, &Form1::textBox2_SelectionChangeCommitted);
			// 
			// checkBox1
			// 
			this->checkBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->checkBox1->AutoSize = true;
			this->checkBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(240)));
			this->checkBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F));
			this->checkBox1->Location = System::Drawing::Point(412, 13);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 7;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = false;
			this->checkBox1->Visible = false;
			this->checkBox1->CheckedChanged += gcnew System::EventHandler(this, &Form1::checkBox1_CheckedChanged);
			// 
			// label2
			// 
			this->label2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->label2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(240)));
			this->label2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(9, 38);
			this->label2->Margin = System::Windows::Forms::Padding(5);
			this->label2->Name = L"label2";
			this->label2->Padding = System::Windows::Forms::Padding(5, 0, 0, 0);
			this->label2->Size = System::Drawing::Size(486, 24);
			this->label2->TabIndex = 5;
			this->label2->Text = L"PVR: Device not connected";
			this->label2->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStrip2
			// 
			this->toolStrip2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->toolStrip2->AutoSize = false;
			this->toolStrip2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->toolStrip2->Dock = System::Windows::Forms::DockStyle::None;
			this->toolStrip2->GripMargin = System::Windows::Forms::Padding(1);
			this->toolStrip2->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->toolStrip2->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(9) {this->toolStripButton5, 
				this->toolStripButton6, this->toolStripButton7, this->toolStripButton8, this->toolStripSeparator1, this->toolStripButton9, this->toolStripButton10, 
				this->toolStripSeparator2, this->toolStripButton11});
			this->toolStrip2->LayoutStyle = System::Windows::Forms::ToolStripLayoutStyle::HorizontalStackWithOverflow;
			this->toolStrip2->Location = System::Drawing::Point(9, 0);
			this->toolStrip2->Name = L"toolStrip2";
			this->toolStrip2->Padding = System::Windows::Forms::Padding(0, 0, 4, 0);
			this->toolStrip2->Size = System::Drawing::Size(486, 38);
			this->toolStrip2->Stretch = true;
			this->toolStrip2->TabIndex = 4;
			this->toolStrip2->Text = L"toolStrip2";
			// 
			// toolStripButton5
			// 
			this->toolStripButton5->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton6->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton7->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton8->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton9->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton10->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton11->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->listView1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->listView1->FullRowSelect = true;
			this->listView1->GridLines = true;
			this->listView1->HideSelection = false;
			this->listView1->LabelEdit = true;
			this->listView1->Location = System::Drawing::Point(9, 93);
			this->listView1->Margin = System::Windows::Forms::Padding(0);
			this->listView1->Name = L"listView1";
			this->listView1->Size = System::Drawing::Size(486, 487);
			this->listView1->TabIndex = 0;
			this->listView1->Tag = L"dummy";
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
			this->panel2->Controls->Add(this->checkBox2);
			this->panel2->Controls->Add(this->button2);
			this->panel2->Controls->Add(this->button1);
			this->panel2->Controls->Add(this->panel7);
			this->panel2->Controls->Add(this->panel8);
			this->panel2->Dock = System::Windows::Forms::DockStyle::Right;
			this->panel2->Location = System::Drawing::Point(495, 0);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(47, 580);
			this->panel2->TabIndex = 7;
			// 
			// checkBox2
			// 
			this->checkBox2->AutoSize = true;
			this->checkBox2->CheckAlign = System::Drawing::ContentAlignment::TopCenter;
			this->checkBox2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), 
				static_cast<System::Int32>(static_cast<System::Byte>(150)));
			this->checkBox2->Location = System::Drawing::Point(3, 133);
			this->checkBox2->Name = L"checkBox2";
			this->checkBox2->Size = System::Drawing::Size(38, 31);
			this->checkBox2->TabIndex = 7;
			this->checkBox2->Text = L"Move";
			this->checkBox2->TextAlign = System::Drawing::ContentAlignment::BottomCenter;
			this->checkBox2->UseVisualStyleBackColor = true;
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
			this->button2->Location = System::Drawing::Point(4, 324);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(38, 37);
			this->button2->TabIndex = 2;
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button1
			// 
			this->button1->AccessibleName = L"";
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button1->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->button1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"button1.Image")));
			this->button1->Location = System::Drawing::Point(4, 268);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(38, 37);
			this->button1->TabIndex = 1;
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// panel7
			// 
			this->panel7->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->panel7->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(245)), static_cast<System::Int32>(static_cast<System::Byte>(245)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel7->Controls->Add(this->radioButton2);
			this->panel7->Controls->Add(this->radioButton1);
			this->panel7->Location = System::Drawing::Point(5, 383);
			this->panel7->Name = L"panel7";
			this->panel7->Size = System::Drawing::Size(36, 64);
			this->panel7->TabIndex = 5;
			this->panel7->Visible = false;
			// 
			// radioButton2
			// 
			this->radioButton2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->radioButton2->AutoSize = true;
			this->radioButton2->BackColor = System::Drawing::Color::Transparent;
			this->radioButton2->CheckAlign = System::Drawing::ContentAlignment::TopCenter;
			this->radioButton2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), 
				static_cast<System::Int32>(static_cast<System::Byte>(200)));
			this->radioButton2->Location = System::Drawing::Point(-1, 32);
			this->radioButton2->Name = L"radioButton2";
			this->radioButton2->Size = System::Drawing::Size(38, 30);
			this->radioButton2->TabIndex = 6;
			this->radioButton2->Text = L"Move";
			this->radioButton2->UseMnemonic = false;
			this->radioButton2->UseVisualStyleBackColor = false;
			// 
			// radioButton1
			// 
			this->radioButton1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->radioButton1->AutoSize = true;
			this->radioButton1->BackColor = System::Drawing::Color::Transparent;
			this->radioButton1->CheckAlign = System::Drawing::ContentAlignment::TopCenter;
			this->radioButton1->Checked = true;
			this->radioButton1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), 
				static_cast<System::Int32>(static_cast<System::Byte>(200)));
			this->radioButton1->Location = System::Drawing::Point(1, 2);
			this->radioButton1->Margin = System::Windows::Forms::Padding(3, 7, 3, 7);
			this->radioButton1->Name = L"radioButton1";
			this->radioButton1->Size = System::Drawing::Size(35, 30);
			this->radioButton1->TabIndex = 5;
			this->radioButton1->TabStop = true;
			this->radioButton1->Text = L"Copy";
			this->radioButton1->UseMnemonic = false;
			this->radioButton1->UseVisualStyleBackColor = false;
			// 
			// panel8
			// 
			this->panel8->BackColor = System::Drawing::Color::White;
			this->panel8->Location = System::Drawing::Point(4, 382);
			this->panel8->Name = L"panel8";
			this->panel8->Size = System::Drawing::Size(38, 66);
			this->panel8->TabIndex = 6;
			this->panel8->Visible = false;
			// 
			// panel4
			// 
			this->panel4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel4->Controls->Add(this->textBox1);
			this->panel4->Controls->Add(this->label1);
			this->panel4->Controls->Add(this->toolStrip1);
			this->panel4->Controls->Add(this->listView2);
			this->panel4->Dock = System::Windows::Forms::DockStyle::Right;
			this->panel4->Location = System::Drawing::Point(542, 0);
			this->panel4->Name = L"panel4";
			this->panel4->Size = System::Drawing::Size(338, 580);
			this->panel4->TabIndex = 6;
			// 
			// textBox1
			// 
			this->textBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->textBox1->FlatStyle = System::Windows::Forms::FlatStyle::System;
			this->textBox1->Font = (gcnew System::Drawing::Font(L"Lucida Console", 10));
			this->textBox1->ForeColor = System::Drawing::Color::Navy;
			this->textBox1->FormattingEnabled = true;
			this->textBox1->Location = System::Drawing::Point(0, 68);
			this->textBox1->MaxDropDownItems = 12;
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(326, 21);
			this->textBox1->TabIndex = 4;
			this->textBox1->SelectionChangeCommitted += gcnew System::EventHandler(this, &Form1::textBox1_SelectedIndexChanged);
			// 
			// label1
			// 
			this->label1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->label1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(240)));
			this->label1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(0, 38);
			this->label1->Margin = System::Windows::Forms::Padding(5);
			this->label1->Name = L"label1";
			this->label1->Padding = System::Windows::Forms::Padding(5, 0, 0, 0);
			this->label1->Size = System::Drawing::Size(326, 24);
			this->label1->TabIndex = 4;
			this->label1->Text = L"label1";
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStrip1
			// 
			this->toolStrip1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->toolStrip1->AutoSize = false;
			this->toolStrip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->toolStrip1->Dock = System::Windows::Forms::DockStyle::None;
			this->toolStrip1->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(6) {this->toolStripButton1, 
				this->toolStripButton2, this->toolStripButton3, this->toolStripButton4, this->toolStripButton13, this->toolStripButton12});
			this->toolStrip1->Location = System::Drawing::Point(6, 0);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->Padding = System::Windows::Forms::Padding(0, 0, 4, 0);
			this->toolStrip1->Size = System::Drawing::Size(320, 38);
			this->toolStrip1->TabIndex = 3;
			this->toolStrip1->Text = L"toolStrip1";
			// 
			// toolStripButton1
			// 
			this->toolStripButton1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->toolStripButton4->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			// toolStripButton13
			// 
			this->toolStripButton13->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->toolStripButton13->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
			this->toolStripButton13->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton13.Image")));
			this->toolStripButton13->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton13->Margin = System::Windows::Forms::Padding(2, 1, 2, 2);
			this->toolStripButton13->Name = L"toolStripButton13";
			this->toolStripButton13->Size = System::Drawing::Size(53, 35);
			this->toolStripButton13->Text = L"Settings";
			this->toolStripButton13->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageAboveText;
			this->toolStripButton13->ToolTipText = L"Change Antares\' settings";
			this->toolStripButton13->Click += gcnew System::EventHandler(this, &Form1::toolStripButton13_Click);
			// 
			// toolStripButton12
			// 
			this->toolStripButton12->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
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
			this->listView2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->listView2->FullRowSelect = true;
			this->listView2->GridLines = true;
			this->listView2->HideSelection = false;
			this->listView2->LabelEdit = true;
			this->listView2->Location = System::Drawing::Point(0, 93);
			this->listView2->Name = L"listView2";
			this->listView2->Size = System::Drawing::Size(326, 487);
			this->listView2->TabIndex = 2;
			this->listView2->Tag = L"dummy";
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
			// notifyIcon1
			// 
			this->notifyIcon1->Text = L"notifyIcon1";
			this->notifyIcon1->Visible = true;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->AutoSize = true;
			this->ClientSize = System::Drawing::Size(880, 602);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->statusStrip1);
			this->ForeColor = System::Drawing::SystemColors::ControlText;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"Form1";
			this->Text = L"Antares  0.8.1";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->ResizeBegin += gcnew System::EventHandler(this, &Form1::Form1_ResizeBegin);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::Form1_Paint);
			this->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::Form1_Layout);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Form1::Form1_FormClosing);
			this->Resize += gcnew System::EventHandler(this, &Form1::Form1_Resize);
			this->ResizeEnd += gcnew System::EventHandler(this, &Form1::Form1_ResizeEnd);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->panel1->ResumeLayout(false);
			this->panel3->ResumeLayout(false);
			this->panel3->PerformLayout();
			this->toolStrip2->ResumeLayout(false);
			this->toolStrip2->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel2->PerformLayout();
			this->panel7->ResumeLayout(false);
			this->panel7->PerformLayout();
			this->panel4->ResumeLayout(false);
			this->toolStrip1->ResumeLayout(false);
			this->toolStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
		System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
		}
		System::Void splitContainer1_Panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		}
		System::Void toolStripButton1_Click(System::Object^  sender, System::EventArgs^  e) {

		}
		System::Void folderBrowserDialog1_HelpRequest(System::Object^  sender, System::EventArgs^  e) {
		}
		System::Void toolStripButton5_Click(System::Object^  sender, System::EventArgs^  e) {
			if (this->transfer_in_progress) return;
			this->topfieldUpDir();
		}
		System::Void toolStripStatusLabel1_Click(System::Object^  sender, System::EventArgs^  e) {
		}


		System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {



			//printf("Timer tick.\n");
			if (!this->transfer_in_progress)
			{
				if (this->fd==NULL)
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
		System::Void statusStrip1_ItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
		}



	protected: virtual void OnLayout(System::Windows::Forms::LayoutEventArgs^ levent) override
			   {
				   //Console::WriteLine("Layout--");

				   array<Control^>^ arr = {this,panel1,panel2,panel3, panel4,listView2};

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
				   this->textBox1->Select(0,0);
				   this->textBox2->Select(0,0);
				   this->Refresh();
				   Arrange_Buttons();

				   //panel4->Refresh();
			   }
	private:

		System::Void Arrange2a(array<ColumnHeader^>^ cols, String^ type, int client_width)
		{

			double widths0[] = {140, 60, 60, 120,60,140};
			double mwidths[] = {0,  70, 70, 130, 60, 0};

			bool something_visible = false;
			int ncols = cols->Length;
			for (int j=0; j<ncols; j++)
			{
				String^ setting_name = type+"_Column"+j.ToString()+"Visible";
				if (this->settings[setting_name] != "1") 
				{
					widths0[j]=0; mwidths[j]=0;
				}
				else
					something_visible=true;
			}

			if (!something_visible) {widths0[0]=140;};


			double tot0 = widths0[0]+widths0[1]+widths0[2]+widths0[3]+widths0[4]+widths0[5];
			double tot0_ = widths0[1]+widths0[2]+widths0[3]+widths0[4];
			double tot0m = mwidths[1]+mwidths[2]+mwidths[3]+mwidths[4];
			//double tot1 = listView1->ClientSize.Width;
			//double tot2 = listView2->ClientSize.Width;
			//
			//double tot1_ = listView1->Width;
			//tot1 = tot1_<tot1 ? tot1_ : tot1;

			double tot1 = client_width;

			if (tot0_ / tot0 * tot1  > tot0m)
			{

				cols[1]->Width = (int) mwidths[1];
				cols[2]->Width = (int) mwidths[2];
				cols[3]->Width = (int) mwidths[3];
				cols[4]->Width = (int) mwidths[4];

				double tmp = widths0[0]+widths0[5];

				cols[0]->Width = (int)( (tot1 - tot0m-5.0)*widths0[0]/tmp );

				cols[5]->Width = (int)( (tot1 - tot0m-5.0)*widths0[5]/tmp );

			}
			else
			{
				cols[0]->Width =  (int) (widths0[0]/tot0 * tot1);
				cols[1]->Width =  (int) (widths0[1]/tot0 * tot1);
				cols[2]->Width =  (int) (widths0[2]/tot0 * tot1);
				cols[3]->Width =  (int) (widths0[3]/tot0 * tot1);
				cols[4]->Width =  (int) (widths0[4]/tot0 * tot1);
				cols[5]->Width =  (int) (widths0[5]/tot0 * tot1);

			}



		}

		System::Void Arrange2(void)
		{
			if (this->finished_constructing ==1)
			{

				double cw1,cw2;

				cw1 = listView1->Width;
				cw2 = listView1->ClientSize.Width;
				cw1 = cw1 < cw2 ? cw1 : cw2;

				this->Arrange2a( this->topfieldHeaders, "PVR", (int) cw1);


				cw1 = listView2->Width;
				cw2 = listView2->ClientSize.Width;
				cw1 = cw1 < cw2 ? cw1 : cw2;

				this->Arrange2a( this->computerHeaders, "PC", (int) cw1);

				return;


				//static const double widths0[] = {140, 60, 50, 120};
				//static const double mwidths[] = {0,  60, 50, 120};

				static const double widths0[] = {140, 60, 0, 120};
				static const double mwidths[] = {0,  70, 0, 130};

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
					this->topfieldSizeHeader->Width =  (int) (widths0[1]/tot0 * tot1);
					this->topfieldTypeHeader->Width =  (int) (widths0[2]/tot0 * tot1);
					this->topfieldDateHeader->Width =  (int) (widths0[3]/tot0 * tot1);
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
					this->computerSizeHeader->Width =  (int) (widths0[1]/tot0 * tot2);
					this->computerTypeHeader->Width =  (int) (widths0[2]/tot0 * tot2);
					this->computerDateHeader->Width =  (int) (widths0[3]/tot0 * tot2);
				}

			}
			this->textBox2->Update();
			this->textBox2->Select(0,0);
			this->textBox1->Select(0,0);



		}

		System::Void Arrange_Buttons(void)
		{

			int d1=12;
			int d2=20;
			int d3=98;

			int ph = this->panel2->Height;
			int bh = this->button1->Height;
			int bp1 = ph/3 - d1 - bh;
			//int bp2 = 2*ph/3 + d2;
			int bp2;
			int pp;

			//if (bp1>268) {bp1=268;}

			bp1=240; if (bp1+d1+d2+bh+bh>ph) bp1 = ph-d1-d2-bh-bh;   //268

			bp2 = bp1 + d1+d2+bh;

			pp = bp1 - d3;


			Point p1 = this->button1->Location;
			p1.Y = bp1;


			Point p2 = this->button2->Location;
			p2.Y = bp2;



			Point p3 = this->panel7->Location;

			Point p4 = this->panel8->Location;

			pp = (ph-this->panel8->Size.Height + bp2)/2;

			//pp=93;//38+24+24;

			pp=bp1-this->panel8->Height - 24 - 8;
			pp=2;
			pp=this->panel2->Height - this->panel8->Height-2;

			p3.Y=pp;
			p4.Y=pp-1;


			Point p5 = this->checkBox2->Location;
			p5.Y = bp1-this->panel8->Height - 20;
			this->checkBox2->Location=p5;



			this->button1->Location=p1;
			this->button2->Location=p2;
			this->panel7->Location=p3;
			this->panel8->Location=p4;
		}

		System::Void Arrange1(void)
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
		System::Void Arrange(void)
		{
			Arrange1();
			Arrange2();
		}

		System::Void deselectComboBoxes(void)
		{
			this->textBox1->Select(0,0);
			this->textBox2->Select(0,0);

		}

		System::Void Form1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
			Console::WriteLine("Layout");

			//this->Arrange();

			ListViewSelectionDelegate^ d = gcnew ListViewSelectionDelegate(this, &Form1::deselectComboBoxes);
			this->BeginInvoke(d);

		}

		System::Void Form1_Resize(System::Object^  sender, System::EventArgs^  e) {

			//Console::WriteLine("Resize");
			if (this->WindowState != FormWindowState::Minimized)
				this->Arrange2();
			//this->Arrange();

		}

		System::Void listView1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
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
			this->textBox2->Select(0,0);
		}

		System::Void listView2_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {



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

		System::Void listView2_ItemActivate(System::Object^  sender, System::EventArgs^  e) {
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
				this->add_path_to_history(this->textBox1, this->computerCurrentDirectory);

			}
			else
			{
				this->ViewInfo(listview);
			}





		};
		System::Void toolStripButton1_Click_1(System::Object^  sender, System::EventArgs^  e) {
			//if (this->transfer_in_progress) return;
			this->computerUpDir();
		}

		void refreshComputer(void)
		{
			this->loadComputerDir();
		}

		System::Void toolStripButton2_Click(System::Object^  sender, System::EventArgs^  e) {


			this->refreshComputer();
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

		System::Void EnableComponents(bool x)
		{
			return;
			for each (Control^ c in this->Controls)
			{

				if (!x && c==this->current_copydialog) continue;
				c->Enabled = x;

			}
		}


		System::Void CentreCopyDialog(CopyDialog^ copydialog, int offset)
		{

			copydialog->Location = System::Drawing::Point( (this->Width - copydialog->Width)/2, offset+(this->Height - copydialog->Height)/2);
			copydialog->BringToFront();
		}

		System::Void ShowCopyDialog(CopyDialog^ copydialog)
		{
			this->current_copydialog = copydialog;
			copydialog->TopLevel = false;
			//copydialog->Dock= DockStyle::Bottom;
			this->EnableComponents(false);
			this->Controls->Add(copydialog);copydialog->Show();
			this->CentreCopyDialog(copydialog,-100);

		}


		System::Void TransferBegan(void)
		{
			this->panel7->Enabled=false;
			this->textBox2->Enabled=false;
			this->checkBox2->Enabled=false;

		}


		System::Void TransferEnded(void)
		{


			if (this->InvokeRequired)
			{
				Monitor::Exit(this->locker);
				printf("0. Transfer ended.\n");
				TransferEndedCallback^ d = gcnew TransferEndedCallback(this, &Form1::TransferEnded);
				this->BeginInvoke(d);

			}
			else
			{
				this->EnableComponents(true);

				this->panel7->Enabled=true;
				this->textBox2->Enabled=true;
				this->checkBox2->Enabled=true;

				Antares::TaskbarState::setNoProgress(this);


				this->Update();

				printf("1. Enable components\n");

				CopyDialog^ copydialog = this->current_copydialog;
				if (copydialog==nullptr) return;
				printf("2. Enable components\n");



				this->transfer_in_progress=false;

				if (this->checkBox1->Checked != copydialog->turbo_request)
					this->checkBox1->Checked = copydialog->turbo_request;

				if (copydialog->file_error->Length > 0)
				{
					MessageBox::Show(this,copydialog->file_error,"Error",MessageBoxButtons::OK);						
				}

				if (copydialog->copydirection == CopyDirection::PVR_TO_PC || copydialog->copymode == CopyMode::MOVE)
					this->loadComputerDir();

				if (copydialog->copydirection == CopyDirection::PC_TO_PVR || copydialog->copymode == CopyMode::MOVE)
					this->loadTopfieldDir();


				if (!copydialog->is_closed)
					copydialog->close_request_threadsafe();

				this->current_copydialog = nullptr;
			}

		}




		/////////////////////////////////////////////////////////////////////////////////


		System::Void transfer_to_PC(Object^ input){
			// Worker thread for doing the transfer from PVR -> PC
			CopyDialog^ copydialog = safe_cast<CopyDialog^>(input);
			//////////////
			Monitor::Enter(this->locker);
			//////////////
			while(copydialog->loaded==false)
			{
				Thread::Sleep(100);
			}
			copydialog->copydirection = CopyDirection::PVR_TO_PC;
			copydialog->update_dialog_threadsafe();
			int numitems = copydialog->numfiles;

			int this_overwrite_action;
			long long topfield_file_offset=0;
			long long probable_minimum_received_offset=-1;
			array<String^>^          dest_filename      = copydialog->dest_filename;
			array<bool>^             dest_exists        = copydialog->dest_exists;
			array<long long int>^    dest_size          = copydialog->dest_size;
			array<int>^              overwrite_action   = copydialog->overwrite_action;
			array<long long int>^    current_offsets    = copydialog->current_offsets;
			array<long long int>^    src_sizes          = copydialog->filesizes;
			array<FileItem^>^        src_items          = copydialog->src_items;
			array<bool>^             source_deleted     = gcnew array<bool>(numitems); for (int i=0; i<numitems; i++) source_deleted[i]=false;

			copydialog->maximum_successful_index=-1;
			for (int i=0; i<numitems; i++)
			{
				copydialog->current_index=i;

				TopfieldItem^ item = safe_cast<TopfieldItem^>(src_items[i]);

				if (item->isdir)
				{
					if (File::Exists(dest_filename[i]) && !Directory::Exists(dest_filename[i]))
					{

						//MessageBox::Show(this,"The folder "+dest_filename[ind]+" could not be created because a file of that name already exists. Aborting transfer.","Error",MessageBoxButtons::OK);
						copydialog->file_error = "The folder "+dest_filename[i]+" could not be created because a file of that name already exists. Aborting transfer.";
						goto end_copy_to_pc;                         

					}
					if (!Directory::Exists(dest_filename[i]))
					{
						try {
							Directory::CreateDirectory(dest_filename[i]);
						}
						catch (...)
						{

							//MessageBox::Show(this,"The folder "+dest_filename[i]+" could not be created. Aborting transfer.","Error",MessageBoxButtons::OK);
							copydialog->file_error = "The folder "+dest_filename[i]+" could not be created. Aborting transfer.","Error";

							goto end_copy_to_pc;

						}
					}

					if (copydialog->copymode == CopyMode::MOVE && i==numitems-1)
					{


						array<TopfieldItem^>^ dirarray = this->loadTopfieldDirArrayOrNull(item->full_filename);
						if (dirarray != nullptr && dirarray->Length==0)
						{

							int dr = this->deleteTopfieldPath(item->full_filename);

							if (dr>=0) source_deleted[i]=true;
						}
					}
					continue;
				}


				long long bytecount=0;
				String^ full_dest_filename = dest_filename[i];
				String^ full_source_filename = item->full_filename; 



				/// This section of code adapted from commands.c [wuppy]
				int result = -EPROTO;

				enum
				{
					START,
					DATA,
					ABORT
				} state;
				struct utimbuf mod_utime_buf = { 0, 0 };
				FileStream^ dest_file;

				int this_overwrite_action;
				long long topfield_file_offset = 0;
				long long probable_minimum_received_offset=-1;
				long long bytes_received;
				long long total_bytes_received=0;

				bool has_restarted=false;

				if(0)
				{
restart_copy_to_pc:
					copydialog->usb_error=false;
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
				bool io_error=false;

				if (this_overwrite_action==SKIP)
				{
					topfield_file_offset = src_sizes[i];
					goto check_delete;	
				}

				tf_packet reply;
				int r;

				array<Byte>^ existing_bytes = gcnew array<Byte>(    2* (int) resume_granularity);
				long long existing_bytes_start; long long existing_bytes_count=0;

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
					//MessageBox::Show(this,"Antares cannot save the file to the location you chose. Please select another location and try again.","Write permission denied",MessageBoxButtons::OK);
					copydialog->file_error = "Antares cannot save the file to the location you chose. Please select another location and try again.";

					goto end_copy_to_pc;				
				}

				try{
					dest_file->SetLength(src_sizes[i]);
				}
				catch (...)
				{
				}

				dest_file->Seek(topfield_file_offset,SeekOrigin::Begin);



				char* srcPath = (char*)(void*)Marshal::StringToHGlobalAnsi(full_source_filename);

				//bool was_cancelled=false;
				bool turbo_changed=false;


				//printf("topfield_file_offset = %ld\n",topfield_file_offset);
				copydialog->file_began();
				if (topfield_file_offset==0) 
					r = send_cmd_hdd_file_send(this->fd, GET, srcPath);   
				else
					r = send_cmd_hdd_file_send_with_offset(this->fd, GET, srcPath,topfield_file_offset);

				Marshal::FreeHGlobal((System::IntPtr)(void*)srcPath);

				if(r < 0)
				{
					copydialog->usb_error=true;
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
				copydialog->current_offsets[i]=topfield_file_offset;
				copydialog->current_bytes_received=bytes_received;
				copydialog->total_bytes_received=total_bytes_received;
				//copydialog->current_filesize = item->size;
				copydialog->update_dialog_threadsafe();


				int update=0;
				while(1)
				{

					update = update+1;


					r = get_tf_packet1(fd, &reply,  0);




					if (r<=0)
					{
						copydialog->usb_error=true;
						this->connection_error_occurred();
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
							this->connection_error_occurred();
							send_cancel(fd);
							copydialog->usb_error=true;
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
								printf("ERROR: Short packet %d instead of %d\n", r,
									get_u16(&reply.length));

								copydialog->usb_error=true;
								goto out;
							}

							//send_success(this->fd);

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
									printf("Warning: resume failed. Starting again.\n");     //TODO: Handle this case better
									topfield_file_offset = 0;
									this_overwrite_action=OVERWRITE;
									dest_file->Close();
									send_cancel(this->fd);
									absorb_late_packets(4,400);
									goto restart_copy_to_pc;
								}

							}
							try{
								dest_file->Write(buffer, 0, dataLen);        //TODO:  Catch full-disk errors  (Sytem::IO:IOException)
							}
							catch(...)
							{
								io_error=true;
								goto out;
							}
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
							if (update%4==0)
							{
								//this->Update();
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
								//was_cancelled=true;
								state = ABORT;
								goto out;

							}

							if (copydialog->turbo_request != *this->turbo_mode)
							{
								turbo_changed=true;
								copydialog->update_dialog_threadsafe();

								send_cancel(fd);
								state=ABORT;
								goto out;
							}



						}
						else
						{
							printf(
								"ERROR: Unexpected DATA_HDD_FILE_DATA packet in state %d\n",
								state);
							send_cancel(fd);
							copydialog->usb_error=true;
							this->connection_error_occurred();
							state = ABORT;
						}
						break;

					case DATA_HDD_FILE_END:
						send_success(fd);
						//item->Selected = false;
						//printf("DATA_HDD_FILE_END\n");
						result = 0;
						goto out;
						break;

					case FAIL:
						printf("ERROR: Device reports %s in transfer_to_PC\n",
							decode_error(&reply));
						this->connection_error_occurred();
						send_cancel(fd);
						state = ABORT;
						copydialog->usb_error=true;
						break;

					case SUCCESS:
						//printf("SUCCESS\n");
						goto out;
						break;

					default:
						printf("ERROR: Unhandled packet (cmd 0x%x)\n",
							get_u32(&reply.cmd));
						this->connection_error_occurred();
						send_cancel(fd);
						copydialog->usb_error=true;
						goto out;
					}




				}

out:

				try  
				{
					dest_file->SetLength(copydialog->current_offsets[i]);
				} catch(...){};

				try
				{
					dest_file->Close();
					File::SetCreationTime(full_dest_filename, item->datetime);
					File::SetLastWriteTime(full_dest_filename, item->datetime);
				}
				catch(...)
				{

				}

				if (copydialog->cancelled==true) break;

				if (copydialog->usb_error)
				{
					this->connection_error_occurred();

					Monitor::Exit(this->locker);
					int wfc = this->wait_for_connection(copydialog);
					Monitor::Enter(this->locker);



					if (wfc < 0) 
						goto end_copy_to_pc;
					else
						goto restart_copy_to_pc;
				}

				if (io_error)
				{
					send_cancel(fd);
					this->absorb_late_packets(2,100);
					switch(this->wait_for_space_on_pc(copydialog,dest_filename[i]))
					{
					case -1:
						goto end_copy_to_pc;
						break;
					case 0:
						goto restart_copy_to_pc;
						break;
					case -2:
						copydialog->file_error = "An error occurred writing the file to your computer";
						goto end_copy_to_pc;
					}
				}

				if (turbo_changed || copydialog->turbo_request != *this->turbo_mode )
				{
					this->absorb_late_packets(4,100);
					this->set_turbo_mode(copydialog->turbo_request);
					copydialog->reset_rate();
					goto restart_copy_to_pc;
				}

				if (!copydialog->cancelled) {copydialog->maximum_successful_index=i;};

check_delete:

				if (copydialog->copymode==CopyMode::MOVE  && topfield_file_offset == src_sizes[i])
				{
					int dr;
					if (overwrite_action[i]!=SKIP || copydialog->action1_skipdelete)
					{
						dr = this->deleteTopfieldPath(item->full_filename);
						if (dr>=0) source_deleted[i]=true;
					}


					// Look for directories which might now be empty, and delete them:

					for (int j=i-1; j>=0; j--)
					{
						TopfieldItem ^titem = safe_cast<TopfieldItem^>(src_items[j]);
						String^ pth_with_slash = titem->full_filename;
						if (!pth_with_slash->EndsWith("\\")) pth_with_slash = pth_with_slash + "\\";
						if (titem->isdir && !source_deleted[j])
						{
							bool probably_empty=true;


							for (int k=j+1; k<numitems; k++)  // assumes that sub-directories and sub-files are always later in array
							{
								TopfieldItem^ titem_k = safe_cast<TopfieldItem^>(src_items[k]);
								if (!source_deleted[k] && titem_k->full_filename->StartsWith(pth_with_slash))
								{
									probably_empty=false;
									break;
								}
							}


							if (probably_empty)
							{
								// Make real sure it's empty now.

								array<TopfieldItem^>^ dirarray = this->loadTopfieldDirArrayOrNull(titem->full_filename);
								if (dirarray != nullptr && dirarray->Length==0)
								{
									// Now finally delete it
									dr = this->deleteTopfieldPath(titem->full_filename);

									if (dr>=0) source_deleted[j]=true;
								}
							}

						}

					}

				}



			}  // end loop over items to be copied

end_copy_to_pc:

			copydialog->close_request_threadsafe();
			this->absorb_late_packets(2,200);
			this->set_turbo_mode(0);
			this->TransferEnded();

		}


		///////////////////////////////////////////////////////////////////////////

		System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {    


			// Copy files from Topfield to computer

			if (this->transfer_in_progress) return;
			const int max_folders = 1000;

			CopyMode copymode = this->getCopyMode();


			// Enumerate selected source items (PVR)

			ListView^ listview = this->listView1;

			ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;


			if (selected->Count==0) return;


			/////////////////////////////
			Monitor::Enter(this->locker);
			//////////////////////////////

			CopyDialog^ copydialog = gcnew CopyDialog();
			copydialog->settings = this->settings;
			copydialog->cancelled=false;
			copydialog->parent_win = this;
			copydialog->parent_form = this;
			//copydialog->showCopyDialog();

			if (copymode==CopyMode::COPY)
				copydialog->window_title="Copying File(s) ... [PVR --> PC]";
			else
				copydialog->window_title="Moving File(s) ... [PVR --> PC]";

			copydialog->Text = copydialog->window_title;

			copydialog->tiny_size();
			copydialog->label3->Text="Finding files...";
			this->ShowCopyDialog(copydialog);

			copydialog->Update();



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
			for (folder_ind=0; folder_ind<numfolders; folder_ind++)
			{
				these_items = items_by_folder[folder_ind];
				for each (TopfieldItem^ item in these_items)
				{
					src_items[ind]=item;
					ind++;
				}
			}

			TopfieldItem^ item;

			int num_files =0;
			long long totalsize = 0;

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


			for (ind=0; ind<numitems; ind++)
			{
				item=src_items[ind];
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


				dest_exists[ind]=File::Exists(dest_filename[ind]);
				dest_size[ind]=0;
				if (dest_exists[ind])
				{          // TODO: error handling
					FileInfo^ fi = gcnew FileInfo(dest_filename[ind]);

					dest_date[ind]=fi->CreationTime;
					dest_size[ind]=fi->Length;
					int cat=2;
					if (dest_size[ind] == item->size)
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


			if (numitems==0) goto aborted;
			bool action1_skipdelete;

			int num_skip=0;
			if (num_exist>0)
			{
				//printf("num_exist=%d  num_cat={%d,%d,%d}\n",num_exist,num_cat[0],num_cat[1],num_cat[2]);
				OverwriteConfirmation^ oc = gcnew OverwriteConfirmation(files_cat[0],files_cat[1],files_cat[2]);
				oc->copymode=copymode;
				if (num_exist==1) oc->title_label->Text="A file with this name already exists                                                                     ";
				else oc->title_label->Text="Files with these names already exist                                                                                  ";					
				//oc->files1->Text = files_cat[0];
				if (num_cat[0]==0)
				{
					oc->panel1->Visible = false;oc->files1->Visible=false;
				}
				if (num_cat[0]==0 || copymode!=CopyMode::MOVE)
				{
					oc->checkBox1->Visible=false;
				}
				else
				{
					oc->checkBox1->Visible=true;
					if (num_cat[0]==1)
						oc->checkBox1->Text = "Delete the PVR copy";
					else
						oc->checkBox1->Text = "Delete the PVR copies";
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
				if (num_cat[2]>1) oc->label3->Text = "These existing files are larger!"; else oc->label3->Text = "This existing file is larger!";

				if (::DialogResult::Cancel == oc->ShowDialog() ) goto aborted;

				int action1 = ( oc->overwrite1->Checked * OVERWRITE ) + oc->skip1->Checked * SKIP;
				int action2 = ( oc->overwrite2->Checked * OVERWRITE ) + oc->skip2->Checked * SKIP + oc->resume2->Checked*RESUME;
				int action3 = ( oc->overwrite3->Checked * OVERWRITE ) + oc->skip3->Checked * SKIP;

				action1_skipdelete = oc->checkBox1->Checked;

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
			if (num_skip==numitems && copymode == CopyMode::COPY) goto aborted;

			for (int i=0; i<numitems; i++)
			{
				if (src_sizes[i]<0 || src_sizes[i]>1000000000000LL)  // A riculous source size indicates a corrupt file. Skip.
					overwrite_action[i]=SKIP;
			}


			long long space_required=0;
			for (int i=0; i<numitems; i++)
			{
				if (overwrite_action[i] != SKIP)     //TODO: modify if we every have an "auto-rename" option.
					space_required += (src_sizes[i] - dest_size[i]);
			}
			//array<long long int>^ freespaceArray = this->computerFreeSpace(this->computerCurrentDirectory);


			array<long long int>^ freespaceArray = this->computerFreeSpace(this->computerCurrentDirectory);
			if (space_required > freespaceArray[0])
			{

				LowSpaceAlert^ alert = gcnew LowSpaceAlert();

				alert->required_label->Text = "Required: " + HumanReadableSize(space_required);
				alert->available_label->Text = "Available: " + HumanReadableSize(freespaceArray[0]);
				if (::DialogResult::Cancel ==  alert->ShowDialog())
				{
					goto aborted;
				}
			}

			if (this->settings["TurboMode"] == "on")//(this->checkBox1->Checked)
				this->set_turbo_mode( 1); //TODO: error handling for turbo mode selection
			else
				this->set_turbo_mode( 0);


			copydialog->SuspendLayout();
			//this->SuspendDrawing(this);
			//this->SuspendDrawing(copydialog);


			copydialog->total_start_time = time(NULL);
			copydialog->current_start_time = 0 ;
			copydialog->filesizes = src_sizes;
			copydialog->current_offsets = current_offsets;
			copydialog->numfiles = num_files;
			copydialog->current_index = 0;

			//copydialog->current_file="Waiting for PVR...";
			copydialog->turbo_mode = this->turbo_mode;
			//copydialog->update_dialog_threadsafe();


			copydialog->current_offsets = current_offsets;
			copydialog->dest_exists = dest_exists;
			copydialog->dest_size = dest_size;
			copydialog->dest_filename = dest_filename;
			copydialog->src_items = src_items;
			//copydialog->topfield_items_by_folder = topfield_items_by_folder;
			copydialog->overwrite_action = overwrite_action;
			copydialog->numfiles=numitems;
			copydialog->current_index=0;
			copydialog->parent_checkbox = this->checkBox1;
			copydialog->copymode=copymode;
			copydialog->action1_skipdelete = action1_skipdelete;
			copydialog->turbo_request = (this->settings["TurboMode"]=="on");


			if (numitems>1)
				copydialog->normal_size();
			else
				copydialog->small_size();
			//this->CentreCopyDialog(copydialog);

			//this->ResumeDrawing(this);
			//this->ResumeDrawing(copydialog);

			copydialog->ResumeLayout();

			//long long bytecount;
			time_t startTime = time(NULL);

			this->transfer_in_progress=true;
			this->TransferBegan();

			Thread^ thread = gcnew Thread(gcnew ParameterizedThreadStart(this,&Form1::transfer_to_PC));
			thread->Name = "Transfer_to_PC";
			copydialog->thread=thread;
			thread->Start(copydialog);
			Monitor::Exit(this->locker);
			//copydialog->showDialog_thread();

			//this->ShowCopyDialog(copydialog);


			return;

aborted:   // If the transfer was cancelled before it began


			Monitor::Exit(this->locker);

			this->TransferEnded();




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
		System::Void transfer_to_PVR(Object^ input){
			// Worker thread for doing the transfer from PC -> PVR
			CopyDialog^ copydialog = safe_cast<CopyDialog^>(input);
			///////
			Monitor::Enter(this->locker);
			//////
			while(copydialog->loaded==false)
			{
				Thread::Sleep(100);
			}
			copydialog->copydirection=CopyDirection::PC_TO_PVR;
			copydialog->update_dialog_threadsafe();
			int numitems = copydialog->numfiles;

			int this_overwrite_action;
			long long topfield_file_offset=0;
			long long probable_minimum_received_offset=-1;
			array<String^>^          dest_filename      = copydialog->dest_filename;
			array<bool>^             dest_exists        = copydialog->dest_exists;
			array<long long int>^    dest_size          = copydialog->dest_size;
			array<int>^              overwrite_action   = copydialog->overwrite_action;
			array<long long int>^    current_offsets    = copydialog->current_offsets;
			array<long long int>^    src_sizes          = copydialog->filesizes;
			array<FileItem^>^        src_items          = copydialog->src_items;
			array<bool>^             source_deleted     = gcnew array<bool>(numitems); for (int i=0; i<numitems; i++) source_deleted[i]=false;
			array<array<TopfieldItem^>^>^ topfield_items_by_folder = copydialog->topfield_items_by_folder;
			TopfieldItem^ titem;
			copydialog->maximum_successful_index=-1;
			for (int i=0; i<numitems; i++)
			{
				copydialog->current_index = i;

				ComputerItem^ item = safe_cast<ComputerItem^>(src_items[i]);
				String^ full_dest_filename = dest_filename[i];
				String^ full_src_filename = item->full_filename;


				if (  (time(NULL) - this->last_topfield_freek_time) > 60) this->updateTopfieldSummary();

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
						//copydialog->close_request_threadsafe();
						//MessageBox::Show(this,"The folder "+dest_filename[i]+" could not be created. Aborting transfer.","Error",MessageBoxButtons::OK);							
						copydialog->file_error="The folder "+dest_filename[i]+" could not be created. Aborting transfer.";
						goto finish_transfer;
					}


					if (copydialog->copymode==CopyMode::MOVE)
					{
						// Try deleting this directory at the source, and any directories which might now be empty

						for (int j=i; j>=0; j--)  // NB: loop in reverse order to sweep up recursive directories with no files
						{
							ComputerItem^ item2 = safe_cast<ComputerItem^>(src_items[j]);
							if (item2->isdir && !source_deleted[j])
							{
								if (item->full_filename->StartsWith(item2->full_filename))
								{
									try
									{
										Directory::Delete(item2->full_filename);
										source_deleted[j]=true;
									}
									catch(...)
									{

									}
								}
							}
						}
					}




					continue;
				}


				FileStream^ src_file;
				bool has_restarted = false;

				copydialog->usb_error=false;
				copydialog->file_error="";

				topfield_file_offset=0;
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

				if (this_overwrite_action==SKIP) {
					printf("Skipping.\n");

					if (copydialog->copymode == CopyMode::MOVE && copydialog->action1_skipdelete)
					{
						if (dest_size[i]==src_sizes[i])
						{
							try 
							{
								File::Delete(item->full_filename);
								source_deleted[i]=true;
							}
							catch(...)
							{

							}

						}

					}

					continue;
				}  

				if (this_overwrite_action==RESUME && dest_size[i]>=src_sizes[i]) {
					printf("Not resuming.\n");
					if (dest_size[i]==src_sizes[i]) topfield_file_offset = dest_size[i];
					goto check_delete;
				} // TODO: Handle this case better

				copydialog->freespace_check_needed = false;

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
				if(fileSize == 0)   
				{
					//printf("ERROR: Source file is empty - not transfering.\n");
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

				printf("topfield_file_offset=%lld   = %f MB\n",topfield_file_offset,((double)topfield_file_offset)/1024.0/1024.0);
				copydialog->file_began();
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

					update = (update + 1) % 2;
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
								this->bytes_sent_since_last_freek += w;
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
									copydialog->update_dialog_threadsafe();
									turbo_changed=true;
									state=END;
								}

								/* Detect EOF and transition to END */
								if((w < 0) || (topfield_file_offset >= fileSize))
								{
									printf("\nEOF conditition. w=%d  bytes_sent=%f  fileSize=%f topfield_file_offset=%f\n",w,(double) bytes_sent,(double) fileSize,(double)topfield_file_offset);
									state = END;
								}

								if(w > 0 || true)
								{
									trace(3,
										fprintf(stderr, "%s: DATA_HDD_FILE_DATA\n",
										__FUNCTION__));
									r = send_tf_packet(this->fd, &packet);
									if(r < w)
									{
										printf("ERROR: Incomplete send.\n");
										copydialog->usb_error=true;
										state=END;break;  // This line an experiment, 26/1/11
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

								double dt = (double) time(NULL)-this->last_topfield_freek_time;
								if (dt>30)
								{
									double worst_free_mb = (double) this->last_topfield_freek / 1024.0;
									worst_free_mb -=  (this->bytes_sent_since_last_freek/1024.0/1024.0 + 2.0 * dt);
									if (worst_free_mb < this->topfield_minimum_free_megs)
									{
										copydialog->freespace_check_needed = true;
										state=END;
									}
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
								printf("ERROR: Incomplete send.\n");
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
						fprintf(stderr, "ERROR: Device reports %s in transfer_to_PVR\n",
							decode_error(&reply));
						copydialog->usb_error=true;
						state=END; copydialog->usb_error=true;break;  // This line an experiment, 26/1/11.
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
				{
					this->connection_error_occurred();

					Monitor::Exit(this->locker);
					int wfc = this->wait_for_connection(copydialog);
					Monitor::Enter(this->locker);

					if (wfc < 0) 
					{
						//copydialog->close_request_threadsafe();
						printf("Cancelling after error.\n");
						goto finish_transfer;
					}
					else
						goto restart_copy_to_pvr;
				}

				if (copydialog->freespace_check_needed)
				{
					this->getTopfieldFreeSpace();
					this->updateTopfieldSummary();
					if (this->last_topfield_freek < 1024.0 * this->topfield_minimum_free_megs )
					{
						Monitor::Exit(this->locker);
						int wfc = this->wait_for_connection(copydialog);
						Monitor::Enter(this->locker);

						if (wfc<0)
						{
							goto finish_transfer;
						}
					}
					goto restart_copy_to_pvr;
				}

				if (turbo_changed)
				{
					copydialog->update_dialog_threadsafe();
					this->absorb_late_packets(2,100);
					this->set_turbo_mode(copydialog->turbo_request);
					copydialog->reset_rate();
					goto restart_copy_to_pvr;
				}


				if (copydialog->turbo_request != *this->turbo_mode)
				{
					copydialog->update_dialog_threadsafe();
					this->set_turbo_mode( copydialog->turbo_request ? 1:0);
					copydialog->reset_rate();

				}

				if (!copydialog->cancelled) {copydialog->maximum_successful_index=i;};
				if (copydialog->cancelled)
				{
					//copydialog->close_request_threadsafe();
					break;
				}

check_delete:
				Console::WriteLine(item->full_filename);
				printf("  topfield_file_offset =%ld   src_sizes=%ld  \n",topfield_file_offset, src_sizes[i]);

				if (copydialog->copymode==CopyMode::MOVE  && topfield_file_offset == src_sizes[i])
				{
					try{
						if (overwrite_action[i]!=SKIP || copydialog->action1_skipdelete)
						{
							Console::WriteLine(item->full_filename);
							File::Delete(item->full_filename);
							source_deleted[i]=true;
						}
					}
					catch(...)
					{
						Console::WriteLine("Didn't delete.");

					}



					// Try deleting any directories that this file might have been in which might now be empty

					for (int j=i-1; j>=0; j--)  // NB: loop in reverse order to sweep up recursive directories with no files
					{
						ComputerItem^ item2 = safe_cast<ComputerItem^>(src_items[j]);
						if (item2->isdir && !source_deleted[j])
						{
							if (item->full_filename->StartsWith(item2->full_filename))
							{
								try
								{
									Directory::Delete(item2->full_filename);
									source_deleted[j]=true;
								}
								catch(...)
								{

								}
							}
						}
					}

				}




			}  // (end loop over files to transfer)

finish_transfer:

			copydialog->close_request_threadsafe();
			this->absorb_late_packets(2,200);
			this->set_turbo_mode(0);
			this->TransferEnded();
			//printf("!!!!!!! Transfer thread ended normally.\n");
		}

		////////////////////////////////////////////////////////////////////////////////////
		System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

			// Copy files from Computer to Topfield


			if (this->transfer_in_progress) return;
			if (this->fd==NULL) return;

			const int max_folders = 1000;

			CopyMode copymode = this->getCopyMode();

			//time_t startTime = time(NULL);

			int src = -1;
			//int r;
			//int update = 0;

			// Enumerate selected source items on computer

			ListView^ listview = this->listView2;

			ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

			if (selected->Count==0) return;


			/////////////////////////////
			Monitor::Enter(this->locker);
			////////////////////////////////

			CopyDialog^ copydialog = gcnew CopyDialog();
			copydialog->settings = this->settings;
			copydialog->cancelled=false;
			copydialog->parent_win = this;
			copydialog->parent_form = this;
			//copydialog->showCopyDialog();

			if (copymode==CopyMode::COPY)
				copydialog->window_title="Copying File(s) ... [PC --> PVR]";
			else
				copydialog->window_title="Moving File(s) ... [PC --> PVR]";

			copydialog->Text = copydialog->window_title;

			copydialog->tiny_size();
			copydialog->label3->Text="Finding files...";
			this->ShowCopyDialog(copydialog);

			copydialog->Update();


			array<ComputerItem^>^ items = gcnew array<ComputerItem^>(selected->Count);
			selected->CopyTo(items,0);
			for (int i=0; i<items->Length; i++) if (items[i]->isdrive) {goto abort;};  // Can't copy whole drives at a time

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

					bool seen_before=false;
					for (int j=0; j<ind; j++)
					{
						for each (TopfieldItem^ titem in topfield_items_by_folder[j])
						{
							if (titem->full_filename->StartsWith(tmp)) {seen_before=true;break;};
						}
						if (seen_before) break;
					}
					if (seen_before)
						topfield_items_by_folder[ind]=this->loadTopfieldDirArray(tmp);
					else
						topfield_items_by_folder[ind]=gcnew array<TopfieldItem^>(0);
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
				{
					dest_exists[ind]=false;
					dest_size[ind]=0;
				}
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
			if (numitems==0) goto abort;


			int num_skip=0;
			bool action1_skipdelete=true;
			if (num_exist>0)
			{
				//printf("num_exist=%d  num_cat={%d,%d,%d}\n",num_exist,num_cat[0],num_cat[1],num_cat[2]);
				OverwriteConfirmation^ oc = gcnew OverwriteConfirmation(files_cat[0],files_cat[1], files_cat[2]);
				oc->copymode=copymode;
				if (num_exist==1) oc->title_label->Text="A file with this name already exists                                                   ";
				else oc->title_label->Text = "Files with these names already exist                                                              ";

				if (num_cat[0]==0)
				{
					oc->panel1->Visible = false;oc->files1->Visible=false;

				}
				if (num_cat[0]==0 || copymode!=CopyMode::MOVE)
				{
					oc->checkBox1->Visible=false;
				}
				else
				{
					oc->checkBox1->Visible=true;
					if (num_cat[0]==1)
						oc->Text = "Delete the PC copy";
					else
						oc->Text = "Delete the PC copies";
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
				if (num_cat[2]>1) oc->label3->Text = "These exising files are larger!!"; else oc->label3->Text = "This existing file is larger!!";

				if (::DialogResult::Cancel == oc->ShowDialog() ) goto abort;

				int action1 = ( oc->overwrite1->Checked * OVERWRITE ) + oc->skip1->Checked * SKIP;
				int action2 = ( oc->overwrite2->Checked * OVERWRITE ) + oc->skip2->Checked * SKIP + oc->resume2->Checked*RESUME;
				int action3 = ( oc->overwrite3->Checked * OVERWRITE ) + oc->skip3->Checked * SKIP;

				action1_skipdelete = oc->checkBox1->Checked;

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
			if (num_skip==numitems && copymode == CopyMode::COPY) goto abort;


			long long space_required=0;
			for (int i=0; i<numitems; i++)
			{
				if (overwrite_action[i] != SKIP)     //TODO: modify if we every have an "auto-rename" option.
					space_required += (src_sizes[i] - dest_size[i]);
			}



			TopfieldFreeSpace tfs = this->getTopfieldFreeSpace();

			long long int freespace = (long long int) tfs.freek * 1024LL;
			long long int margin = 1024*1024*3; if (freespace>margin) freespace-=margin;  // You can never seem to use the last couple of MB on topfield
			if (tfs.valid)
			{
				if (space_required > freespace)
				{

					LowSpaceAlert^ alert = gcnew LowSpaceAlert();
					alert->required_label->Text = "Required: " + HumanReadableSize(space_required);
					alert->available_label->Text = "Available: " + HumanReadableSize(freespace);
					if (freespace < this->topfield_minimum_free_megs*1024*1024)
					{
						alert->label4->Visible = false;
						alert->button1->Visible = false;
					}
					if (::DialogResult::Cancel ==  alert->ShowDialog())
					{
						goto abort;
					}
				}
			}


			if (this->settings["TurboMode"]=="on")//(this->checkBox1->Checked)
				this->set_turbo_mode(1); //TODO: error handling for turbo mode selection
			else
				this->set_turbo_mode(0);



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


			copydialog->turbo_mode = this->turbo_mode;
			copydialog->parent_checkbox = this->checkBox1;
			copydialog->copymode=copymode;
			copydialog->action1_skipdelete = action1_skipdelete;
			copydialog->turbo_request = (this->settings["TurboMode"]=="on");



			if (numitems>1)
				copydialog->normal_size();
			else
				copydialog->small_size();

			//copydialog->TopLevel = false;this->panel1->Controls->Add(copydialog);copydialog->Show();copydialog->Visible=true;
			//copydialog->Dock = DockStyle::Bottom;


			this->transfer_in_progress=true;
			this->TransferBegan();
			Thread^ thread = gcnew Thread(gcnew ParameterizedThreadStart(this,&Form1::transfer_to_PVR));
			thread->Name = "transfer_to_PVR";
			copydialog->thread = thread;
			thread->Start(copydialog);

			Monitor::Exit(this->locker);

			//this->ShowCopyDialog(copydialog);


			return;
abort:  // If the transfer was cancelled before it began

			Monitor::Exit(this->locker);
			this->TransferEnded();

		}

		System::Void listView1_ItemActivate(System::Object^  sender, System::EventArgs^  e) {

			if (this->transfer_in_progress) return;

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
				this->add_path_to_history(this->textBox2, this->topfieldCurrentDirectory);

			}
			else
			{
				this->ViewInfo(listview);
			}



		}

		void refreshTopfield(void)
		{
			if (this->transfer_in_progress) return;
			this->loadTopfieldDir();
		}

		System::Void toolStripButton6_Click(System::Object^  sender, System::EventArgs^  e) {
			this->refreshTopfield();
		}

		System::Void listView_ColumnClick(System::Object^  sender, System::Windows::Forms::ColumnClickEventArgs^  e) {

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

		int deleteTopfieldPath(String^ path)
		{


			char* ascii_path = (char*)(void*)Marshal::StringToHGlobalAnsi(path);

			int r = do_hdd_del(this->fd, ascii_path);
			Marshal::FreeHGlobal((System::IntPtr)(void*)ascii_path);

			return r;

		}


		System::Void toolStripButton7_Click(System::Object^  sender, System::EventArgs^  e) {

			// Delete files on the Topfield

			if (this->transfer_in_progress) return;
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
			//long long bytecount;
			time_t startTime = time(NULL);


			Monitor::Enter(this->locker);
			while ( myEnum->MoveNext() )
			{
				item = safe_cast<TopfieldItem^>(myEnum->Current);

				this->deleteTopfieldPath(item->full_filename);
			}
			this->loadTopfieldDir();
			Monitor::Exit(this->locker);
			this->absorb_late_packets(2,200);

		}
		System::Void listView_AfterLabelEdit(System::Object^  sender, System::Windows::Forms::LabelEditEventArgs^  e) {
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
					if (this->transfer_in_progress) {e->CancelEdit=true;item->Text = item->filename;return;};
					char* old_path = (char*)(void*)Marshal::StringToHGlobalAnsi(old_full_filename);
					String^ new_filename = safeString(e->Label);
					String^ new_full_filename = item->directory + "\\" + new_filename;
					char* new_path = (char*)(void*)Marshal::StringToHGlobalAnsi(new_full_filename);
					Monitor::Enter(this->locker);
					int r = do_hdd_rename(this->fd, old_path,new_path);
					Monitor::Exit(this->locker);

					Marshal::FreeHGlobal((System::IntPtr)(void*)old_path);
					Marshal::FreeHGlobal((System::IntPtr)(void*)new_path);

					if (r==0) // success
					{
						e->CancelEdit=true;
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

							e->CancelEdit=true;



						}


					}


				}



			}

		}

		System::Void listView_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {


			if (this->transfer_in_progress) return;

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
					this->refreshTopfield();
				else
					this->refreshComputer();
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

			int r=-1;
			char* path = (char*)(void*)Marshal::StringToHGlobalAnsi(dir);
			Monitor::Enter(this->locker);
			try{
				r = do_hdd_mkdir(this->fd,path);}
			catch(...){};
			Monitor::Exit(this->locker);
			Marshal::FreeHGlobal((System::IntPtr)(void*)path);
			return r;
		}

		System::Void toolStripButton8_Click(System::Object^  sender, System::EventArgs^  e) {

			// Clicked "New Folder", Topfield.
			if (this->transfer_in_progress) return;
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

				Monitor::Enter(this->locker);
				r=this->newTopfieldFolder(dir);
				Monitor::Exit(this->locker);
				if (r!=0) this->toolStripStatusLabel1->Text="Error creating new folder.";
				success=true;
				break;
			}
			if (success)
				this->loadTopfieldDir(foldername);
		}

		System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			if (this->checkBox1->Checked)
				settings->changeSetting("TurboMode","on");
			else
				settings->changeSetting("TurboMode","off");
			if (this->current_copydialog != nullptr)
				this->current_copydialog->checkBox1->Checked = this->checkBox1->Checked;
		}

		System::Void listView2_SelectionChanged_Finally(void)
		{
			this->listView2_selection_was_changed=false;
			ListView^ listview = this->listView2;
			String^ txt = "";
			if(listview->SelectedItems->Count ==0 )
			{
				//this->button1->Enabled = false;
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



		System::Void listView1_SelectionChanged_Finally(void)
		{
			this->listView1_selection_was_changed=false;

			ListView^ listview = this->listView1;
			String^ txt = "";
			if(listview->SelectedItems->Count ==0 )
			{
				//this->button2->Enabled = false;
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

		System::Void listView2_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {


			if (this->listView2_selection_was_changed==false)
			{
				ListViewSelectionDelegate^ d = gcnew ListViewSelectionDelegate(this, &Form1::listView2_SelectionChanged_Finally);
				this->listView2_selection_was_changed=true;
				this->BeginInvoke(d);//, gcnew array<Object^> { });
			}

		}

		System::Void listView1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
			if (this->listView1_selection_was_changed==false)
			{
				ListViewSelectionDelegate^ d = gcnew ListViewSelectionDelegate(this, &Form1::listView1_SelectionChanged_Finally);
				this->listView1_selection_was_changed=true;
				this->BeginInvoke(d);//, gcnew array<Object^> { });
			}

		}


		System::Void toolStripButton9_Click(System::Object^  sender, System::EventArgs^  e) {
			// "Cut" button pressed on the topfield side.
			// Change colour of cut items, and record the filenames on the clipboard.   
			//if (this->transfer_in_progress) return;
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
		System::Void toolStripButton10_Click(System::Object^  sender, System::EventArgs^  e) {
			//Someone pressed the "Paste" button (Topfield)
			if (this->transfer_in_progress) return;
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
				Monitor::Enter(this->locker);
				int r = do_hdd_rename(this->fd, src_path,dest_path);
				Monitor::Exit(this->locker);




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

		bool loadInfo(FileItem^ item,   tRECHeaderInfo *ri)
		{

			ComputerItem ^citem;
			TopfieldItem ^titem;
			bool ret;
			citem = dynamic_cast<ComputerItem^>(item);
			if (citem!=nullptr)
			{
				ret = this->computerLoadInfo(citem, ri);
			}
			else 
			{
				titem = dynamic_cast<TopfieldItem^>(item);
				if (titem!=nullptr)
				{
					ret = this->topfieldLoadInfo(titem,ri);
				}
				else
					return false;
			}
			if (!ret) return false;

			item->channel = gcnew String(ri->SISvcName);
			//String^ title = gcnew String(ri->EventEventName);
			item->description = gcnew String(ri->EventEventDescription);
			String^ ext = gcnew String(ri->ExtEventText);
			if (item->description->Length >0 && ext->Length >0 )
			{
				item->description = item->description + "  --- ";
			}
			item->description = item->description + ext;

			if (!this->InvokeRequired)
			{
				item->SubItems[4]->Text = item->channel;
				item->SubItems[5]->Text = item->description;
			}

			return ret;
		}



		bool computerLoadInfo(ComputerItem^ item, tRECHeaderInfo *ri)
		{

			const int readsize = 2048;
			char charbuf[readsize]; 
			array<Byte>^ buffer = gcnew array<Byte>(readsize);
			FileStream^ file;
			try{
				file = File::Open(item->full_filename,System::IO::FileMode::Open, System::IO::FileAccess::Read,System::IO::FileShare::Read);
			}
			catch(...)
			{

				return false;
			}

			int size;
			try {

				size = file->Read(buffer, 0, readsize);
				file->Close();
			}
			catch(...)
			{
				return false;
			}

			if (size==readsize)
			{

				Marshal::Copy(buffer,0,System::IntPtr( &charbuf[0]),size);
				HDD_DecodeRECHeader (charbuf, ri);
				ri->readsize=size;
				return true;
			}
			else
				return false;


		}

		bool topfieldLoadInfo(TopfieldItem^ item,  tRECHeaderInfo *ri)
		{
			const int readsize = 2048;
			char charbuf[readsize];
			//if (this->transfer_in_progress) return false;
			//this->transfer_in_progress = true;
			array<Byte>^ buff;
			Monitor::Enter(this->locker);
			try{
				buff = this->read_topfield_file_snippet(item->full_filename, 0);
			} catch(...){};
			Monitor::Exit(this->locker);
			//this->transfer_in_progress=false;
			int size = buff->Length;
			if (size>=readsize)
			{
				Marshal::Copy(buff,0,System::IntPtr( &charbuf[0]),readsize);
				HDD_DecodeRECHeader (charbuf, ri);
				ri->readsize = size;
				return true;
			}
			else return false;

		}


		System::Void ViewInfo(ListView^ listview)
		{
			int type;
			if (listview==this->listView1) type=0; else type=1;
			ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
			System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
			FileItem^ item;
			tRECHeaderInfo ri;

			while ( myEnum->MoveNext() )
			{


				item = safe_cast<FileItem^>(myEnum->Current);
				if (item->isdir) continue;

				bool ret =  this->loadInfo(item, &ri);

				if (ret)
				{


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

					//item->description = gcnew String(ri.EventEventDescription);
					//item->channel = gcnew String(ri.SISvcName);

					ProgInfo^ pi = gcnew ProgInfo(&ri,"Program Information, "+item->full_filename);

					pi->ShowDialog(this);
					break;
				}


			}
		}


		System::Void Info_Click(System::Object^  sender, System::EventArgs^  e) {
			// Someone clicked "info" on either PVR or PC side

			//ListView^ listview;

			if (sender == this->toolStripButton11)
			{
				if (this->transfer_in_progress) return;
				this->ViewInfo(listView1);
			}
			else
				this->ViewInfo(listView2);
		}


		array<Byte>^ read_topfield_file_snippet(String^ filename, long long offset) {    
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
				this->connection_error_occurred();
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
						this->connection_error_occurred();
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
							this->connection_error_occurred();

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
						this->connection_error_occurred();
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
					fprintf(stderr, "ERROR: Device reports %s in read_topfield_file_snippet\n",
						decode_error(&reply));
					send_cancel(fd);
					this->connection_error_occurred();
					state = ABORT;

					break;

				case SUCCESS:
					printf("SUCCESS\n");

					break;

				default:
					fprintf(stderr, "ERROR: Unhandled packet (cmd 0x%x)\n",
						get_u32(&reply.cmd));
					this->connection_error_occurred();
				}




				if (state==ABORT) break;
			}

			this->absorb_late_packets(2,200);
			return out_array;
		}





		System::Void Form1_ResizeEnd(System::Object^  sender, System::EventArgs^  e) {
			Console::WriteLine("ResizeEnd");
			//this->ResumeLayout();
		}
		System::Void Form1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
			// Console::WriteLine("Paint");
		}
		System::Void Form1_ResizeBegin(System::Object^  sender, System::EventArgs^  e) {
			//Console::WriteLine(this->topfieldSizeHeader->);
			// this->SuspendLayout();
		}
		/////////////////////////////
		System::Void toolStripButton3_Click(System::Object^  sender, System::EventArgs^  e) {
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
		System::Void toolStripButton4_Click(System::Object^  sender, System::EventArgs^  e) {
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

		System::Void Form1_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
			if (this->transfer_in_progress)
			{
				CopyDialog^ copydialog = this->current_copydialog;
				copydialog->cancelled = true;

				if (copydialog->thread != nullptr)
					copydialog->thread->Join();
				//this->TransferEnded();


			}
		}
		System::Void toolStripButton13_Click(System::Object^  sender, System::EventArgs^  e) {
			SettingsDialog^ sd = gcnew SettingsDialog(this->settings);
			sd->ShowDialog();
			this->loadComputerDir();
			this->loadTopfieldDir();
			this->Arrange2();
		}



		void add_path_to_history(ComboBox^ cb, String ^path)
		{
			cb->Items->Remove(path);
			cb->Items->Insert(0,path);
			cb->Text=path;
			cb->Select(0,0);


			String ^ str = "ComputerHistory";
			if (cb == this->textBox2) str = "TopfieldHistory";

			// update settings.
			int hist_len = this->settings->maximum_history_length;
			int num = cb->Items->Count;  
			String^ key;
			for (int j=0; j<hist_len; j++)
			{
				key=str+j.ToString();

				if (j<num)
					this->settings->changeSetting(key, safe_cast<String^>(cb->Items[j]));
				else
					this->settings->clearSetting(key);					
			}


		}



		// An item in the computer path history was selected
		System::Void textBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
			ComboBox^ cb = this->textBox1;
			this->setComputerDir( safe_cast<String^>(cb->SelectedItem));
			this->loadComputerDir();
			this->add_path_to_history(cb,this->computerCurrentDirectory);  
			this->clist->Focus();
		}

		// An item in the topfield path history was selected
		System::Void textBox2_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
			ComboBox^ cb = this->textBox2;
			this->setTopfieldDir(safe_cast<String^>(cb->SelectedItem));
			this->loadTopfieldDir();

			this->add_path_to_history(this->textBox2, this->topfieldCurrentDirectory);
			this->tlist->Focus();
		}

		void centreRB(RadioButton^ rb)
		{
			Point p = rb->Location;
			p.X = ( this->panel7->ClientSize.Width - rb->Width)/2;
			rb->Location=p;

		}

		/*
		private: System::Void radioButton1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		return;
		System::Drawing::Font^ boldfont = gcnew System::Drawing::Font(this->radioButton1->Font,FontStyle::Bold);
		System::Drawing::Font^ plainfont = gcnew System::Drawing::Font(this->radioButton1->Font,FontStyle::Regular);

		if (this->radioButton1->Checked)
		{
		this->radioButton1->Font = boldfont;
		this->radioButton2->Font = plainfont;

		}
		else
		{
		this->radioButton2->Font = boldfont;
		this->radioButton1->Font = plainfont;
		}

		this->centreRB(this->radioButton1);
		this->centreRB(this->radioButton2);

		}
		*/


		CopyMode getCopyMode(void)
		{
			if (this->checkBox2->Checked) 
				return CopyMode::MOVE;
			else
				return CopyMode::COPY;
		}

		void connection_error_occurred(void)
		{
			this->connection_needs_checking=true;
		}



	};    // class form1
};    // namespace antares

