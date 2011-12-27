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
#include "tf_fwio.h"
#include "connect.h"
#include "commands.h"
#include "windows.h"
#include "commctrl.h"
#include <time.h>
#include "FBLib_rec.h"
#include "commandline.h"


	//TODO: put these prototypes somewhere better
	struct husb_device_handle;
	int find_usb_paths(char *dev_paths,  int *pids, int max_paths,  int max_length_paths, char *driver_names, int specified_pid);
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
#include "FirmwareInstaller.h"

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
	using namespace System::Text::RegularExpressions;


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
			if (parent==nullptr) return;
			Antares::SendMessage((HWND) parent->Handle.ToPointer(), WM_SETREDRAW, false, 0); 
		} 

		static void ResumeDrawing( Control^ parent ) 
		{ 
			if (parent==nullptr) return;
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

			this->tooltip_timer = gcnew System::Timers::Timer(200);
			this->tooltip_timer->AutoReset = false;
			this->tooltip_timer->SynchronizingObject = this;
			this->tooltip_timer->Elapsed += gcnew System::Timers::ElapsedEventHandler(this,&Form1::tooltip_timer_elapsed);
			this->tooltip_string = "";
			this->tooltip_location = System::Drawing::Point(0,0);
			

			this->commandline = gcnew CommandLine(Environment::CommandLine);

			if (this->commandline->verbose) 
			{
				printf("Verbose mode.\n");
				set_verbose(3,2);
			}

			this->watched_directory = "";
			this->recording_in_progress_last_checked=DateTime::Now;


			this->data_files_cached = nullptr;

			this->exit_on_completion=false;
			this->no_prompt = false;
			this->idle_count = 0;
			Application::Idle += gcnew EventHandler(this, &Form1::Application_Idle);


			this->topfield_background_enumerator = nullptr;
			this->computer_background_enumerator = nullptr;
			//this->close_request=false;


			this->stopwatch = gcnew System::Diagnostics::Stopwatch();
			this->stopwatch->Start();
			this->listview_click_time=0;


			this->topfield_background_event = gcnew AutoResetEvent(false);
			this->computer_background_event = gcnew AutoResetEvent(false);


			this->locker = gcnew Object();
			this->last_topfield_freek = -1;
			this->last_topfield_freek_time = 0;
			this->computer_new_folder_time = 0;
			this->pid=0;this->ndev=0;

			this->current_copydialog = nullptr;

			this->mi_pc_copy=nullptr;
			this->mi_pvr_copy=nullptr;


			this->proginfo_cache = gcnew ProgramInformationCache();
			this->connection_needs_checking = true;

			icons = gcnew Antares::Icons();

			this->listView1SortColumn = -1;
			this->listView2SortColumn=-1;
			this->turbo_mode = gcnew System::Boolean;
			this->turbo_mode2 = gcnew System::Boolean;

			this->finished_constructing = 0;
			this->last_layout_x=-1;
			this->last_layout_y=-1;
			// Load configuration. 
			this->settings = gcnew Settings();



			trace(1,printf("Hiding form.\n"));

			this->Hide();
			///////////////////////////
			trace(1,printf("InitializeComponent()\n"));
			InitializeComponent();
			///////////////////////////


			//this->SuspendLayout();
			//this->SuspendDrawing(this);
			this->SuspendLayout();
			System::Drawing::Size sz = this->Size;
			sz.Width = Convert::ToInt32(this->settings["Width"]);
			sz.Height = Convert::ToInt32(this->settings["Height"]);
			System::Drawing::Point loc = this->Location;

			try
			{
				loc.X = Convert::ToInt32(this->settings["X"]);
				loc.Y = Convert::ToInt32(this->settings["Y"]);
			}
			catch (...){}


			if (this->location_is_sane(sz.Width, sz.Height, loc.X, loc.Y))
			{
				trace(1,printf("Setting location.\n"));
				this->Size =sz;
				this->Location = loc;
			}




			//this->ResumeLayout(false);

			//printf("-----------  %d  %d ----------\n",Convert::ToInt32(this->settings["Width"]), Convert::ToInt32(this->settings["Height"]));

			this->clist = this->listView2;
			this->tlist = this->listView1;

			this->setTopfieldDir("\\DataFiles\\");
			this->setComputerDir("C:\\");

			listView2_selection_was_changed=false; 
			listView1_selection_was_changed=false;

			//this->SetStyle(ControlStyles::AllPaintingInWmPaint | ControlStyles::UserPaint | ControlStyles::DoubleBuffer,true);

			this->transfer_in_progress=false;
			this->firmware_transfer_in_progress=false;

			this->dircount=0;
			this->fd  = NULL;//connect_device2(&reason);
			//if (this->fd==NULL) this->label2->Text="PVR: Device not connected";

			trace(1,printf("Constructing columns.\n"));
			this->headerNames = gcnew array<String^>{"Name", "Size", "Type", "Date","Channel","Description"};


			this->mi_pc_choose_columns_array = gcnew array<ToolStripMenuItem^>(this->headerNames->Length);
			this->mi_pvr_choose_columns_array = gcnew array<ToolStripMenuItem^>(this->headerNames->Length);

			this->topfieldNameHeader = this->listView1->Columns->Add(headerNames[0],140,HorizontalAlignment::Left);
			this->topfieldSizeHeader = this->listView1->Columns->Add(headerNames[1],70,HorizontalAlignment::Right);
			this->topfieldTypeHeader = this->listView1->Columns->Add(headerNames[2],60,HorizontalAlignment::Left);
			this->topfieldDateHeader = this->listView1->Columns->Add(headerNames[3],120,HorizontalAlignment::Left);
			this->topfieldChannelHeader = this->listView1->Columns->Add(headerNames[4],120,HorizontalAlignment::Left);
			this->topfieldDescriptionHeader = this->listView1->Columns->Add(headerNames[5],120,HorizontalAlignment::Left);

			this->topfieldHeaders = gcnew array<ColumnHeader^>
			{topfieldNameHeader, topfieldSizeHeader, topfieldTypeHeader, topfieldDateHeader, topfieldChannelHeader, topfieldDescriptionHeader};

			this->computerNameHeader = this->listView2->Columns->Add(headerNames[0],140,HorizontalAlignment::Left);
			this->computerSizeHeader = this->listView2->Columns->Add(headerNames[1],70,HorizontalAlignment::Right);
			this->computerTypeHeader = this->listView2->Columns->Add(headerNames[2],60,HorizontalAlignment::Left);
			this->computerDateHeader = this->listView2->Columns->Add(headerNames[3],120,HorizontalAlignment::Left);
			this->computerChannelHeader = this->listView2->Columns->Add(headerNames[4],120,HorizontalAlignment::Left);
			this->computerDescriptionHeader = this->listView2->Columns->Add(headerNames[5],120,HorizontalAlignment::Left);

			this->computerHeaders = gcnew array<ColumnHeader^>
			{computerNameHeader, computerSizeHeader, computerTypeHeader, computerDateHeader, computerChannelHeader, computerDescriptionHeader};



			trace(1,printf("Applying language settings.\n"));
			this->apply_language_setting();

			trace(1,printf("Applying columns visible.\n"));
			this->apply_columns_visible();




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

			trace(1,printf("Applying history.\n"));

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






			trace(1,printf("Checking connection.\n"));
			this->CheckConnection();
			this->last_layout_x = -1;this->last_layout_y=-1;
			this->Arrange();

			// Set double-buffering and image list on the ListViews
			this->setListViewStyle(listView1);
			this->setListViewStyle(listView2);



			if (this->settings["Maximized"]=="1") 
			{
				this->WindowState = FormWindowState::Maximized;
			}


			if (this->commandline->showgui) this->Focus();


			this->cbthread = gcnew Thread(gcnew ThreadStart(this,&Form1::computerBackgroundWork));
			this->tbthread = gcnew Thread(gcnew ThreadStart(this,&Form1::topfieldBackgroundWork));

			cbthread->Name = "cbthread";
			tbthread->Name = "tbthread";

			cbthread->IsBackground = true;
			tbthread->IsBackground = true;


			trace(1,printf("Starting background threads.\n"));
			this->cbthread->Start();
			this->tbthread->Start();

			//this->ResumeDrawing(this);


			trace(1,printf("Resuming layout.\n"));
			this->ResumeLayout(false);

			//Console::WriteLine("Constructed Form.");


			trace(1,printf("Showing gui again.\n"));
			if (this->commandline->showgui)
			{
				this->listView2->Focus();
				this->Show();
				this->Opacity=1.0;
			}
			else
			{
				this->Visible=false;
				this->ShowInTaskbar=false;
				this->WindowState = FormWindowState::Minimized;
				this->Hide();
			}


			//this->fileSystemWatcher1->BeginInit();


			trace(1,printf("topfield_background_event->Set.\n"));
			this->topfield_background_event->Set();

			this->fileSystemWatcher1->Path=".";


			//this->loadTopfieldDir();
			this->loadComputerDir();
			//this->ResizeRedraw = true;

			//this->fileSystemWatcher1->EndInit();
			//this->fileSystemWatcher1->EnableRaisingEvents = true;





			trace(1,printf("Finished constructing form1.\n"));


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

			if (this->InvokeRequired && !this->firmware_transfer_in_progress)
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
				String^ error_str = lang::st_not_connected;//"PVR: Device not connected";
				if (this->fd != NULL) 
				{
					//MessageBox::Show(" husb_free,  "+( (int) this->fd).ToString());
					//printf("husb_free, %ld \n",(long int) this->fd);
					husb_free((husb_device_handle*) (void*) this->fd);
					this->fd=NULL;
				};

				int ndev = find_usb_paths(&dev_paths[0][0],  pids, max_paths,  paths_max_length, &driver_names[0][0], this->commandline->pid);

				this->ndev = ndev;
				for (int j=0; j<ndev; j++)
				{
					device_path = &dev_paths[j][0];
					String^ driver_name = gcnew String( &driver_names[j][0]);
					driver_name = driver_name->ToLowerInvariant();
					if (verbose) printf("%d: Driver: %s\n",j, driver_name);
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
						error_str = lang::st_wrong_driver;//"PVR: Error -- wrong driver installed.";
						continue;
					}

					if (hdev==INVALID_HANDLE_VALUE)
					{
						DWORD last_error = GetLastError();
						printf("%s\n",windows_error_str(last_error));
						if (last_error==ERROR_ACCESS_DENIED) 
							error_str=lang::st_in_use;//"PVR: Error -- already in use.";

						continue;
					}
					//	else
					//
					//		printf("  CreateFile seemed to return successfully.\n");


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
				if (verbose) Console::WriteLine(error_str);

				//this->fd=NULL;
				//printf("this->fd = %ld   fdtemp=%ld  \n",(long int) this->fd, (long int) fdtemp);
				this->fd = (libusb_device_handle *) (void*) fdtemp;

				if (!this->firmware_transfer_in_progress)
				{
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


				this->textBox2->Enabled = (this->fd != NULL);

			}
		}
		/*

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
		fprintf(stdout, "connect: Select configuration failed\n");

		continue;
		}

		// Claim interface 0x00
		if (libusb_claim_interface(dh, 0x00))
		{
		fprintf(stdout, "connect: Claim interface failed\n");

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
		*/

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
			copydialog->set_error( " "+lang::c_no_space_pc);
			if (*this->turbo_mode ) this->set_turbo_mode(0);

			copydialog->reset_rate();
			//copydialog->update_dialog_threadsafe();

			while(1)
			{
				copydialog->update_dialog_threadsafe();
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
			{
				copydialog->set_error( " "+lang::c_error);
				Console::WriteLine(lang::c_error);
			}
			if (copydialog->freespace_check_needed)
			{
				copydialog->current_file = lang::c_checking;

				copydialog->update_dialog_threadsafe();
				goto check_freespace;
			}
			copydialog->reset_rate();
			copydialog->update_dialog_threadsafe();
			int r,ret=0;

wait_again:
			while(1)
			{
				copydialog->update_dialog_threadsafe();
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
			bool printed=false;
			if (ret==0 && copydialog->copydirection == CopyDirection::PC_TO_PVR)
			{


				TopfieldFreeSpace freespace = this->getTopfieldFreeSpace();
				if (freespace.freek<0) goto wait_again;

				//printf("freek = %d\n",freespace.freek);
				if (freespace.freek < this->topfield_minimum_free_megs*1024.0) {
					//printf("copydialog->cancelled %d \n",copydialog->cancelled);
					copydialog->set_error( " "+lang::c_no_space_pvr);
					if (!printed) {printed=true; Console::WriteLine(lang::c_no_space_pvr);};
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



			if (turbo_on)
			{
				if (this->recording_in_progress() )
				{
					printf("Not enabling turbo mode. (Recording in progress).\n");
					turbo_on = 0;
				}

			}


			*this->turbo_mode2 = (turbo_on!=0);


			int r;
			printf("Setting turbo mode: %d\n",turbo_on);  
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
					fprintf(stdout, "Turbo mode: %s\n",
					turbo_on ? "ON" : "OFF"));
				this->absorb_late_packets(2,100);

				return 0;
				break;

			case FAIL:
				fprintf(stdout, "ERROR: Device reports %s in set_turbo_mode\n",
					decode_error(&reply));
				this->connection_error_occurred();
				break;

			default:
				fprintf(stdout, "ERROR: Unhandled packet (in set_turbo_mode) cmd=%d\n",&reply.cmd);
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


		array<ComputerItem^>^ loadComputerDirArrayOrNull(String^ dir)
		{
			array<String^>^ list;
			int j;


			trace(1,printf("GetFileSystemEntries\n"));
			try 
			{
				list = System::IO::Directory::GetFileSystemEntries(dir);
			}
			catch(...)
			{
				return nullptr;
			}

			array<ComputerItem^>^ items = gcnew array<ComputerItem^>(list->Length);
			int ind = 0;
			for (j=0; j<list->Length; j++)
			{
				ComputerItem^ item; 
				try{
					item = gcnew ComputerItem(list[j], dir);
				}
				catch(...)
				{
					continue;
				}

				items[ind] = item;
				trace(1,printf("Adding computer item: %s.\n",item->filename));
				ind++;
			}
			Array::Resize(items, ind);
			return items;
		}

		array<ComputerItem^>^ loadComputerDirArray(String^ dir)
		{
			array<ComputerItem^>^ items = this->loadComputerDirArrayOrNull(dir);
			if (items==nullptr)
				items = gcnew array<ComputerItem^>(0);
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
					//printf("topfieldBackgroundWork iter.\n");
					System::Collections::IEnumerator ^en = this->topfield_background_enumerator;
					if (en==nullptr) continue;
					if (this->Visible==false) continue;

					// First, update the icon of all items

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
							//printf("Invoke, ic=%d ext=",ic); Console::WriteLine(Path::GetExtension(safeString(item->directory)+"\\"+item->safe_filename));
							Application::DoEvents();  // why is this needed?

						}
						if (en!= this->topfield_background_enumerator)
						{
							goto repeat;

						}
					}
					bool proginfo_columns_visible = (this->settings["PVR_Column4Visible"]=="1" || this->settings["PVR_Column5Visible"]=="1");



					DateTime now = DateTime::Now;
					en->Reset();
					tRECHeaderInfo ri;
					while (en->MoveNext())
					{ 
						TopfieldItem^ item = safe_cast<TopfieldItem^>(en->Current);
						//Console::WriteLine(item->full_filename);

						if (!item->isdir && this->proginfo_cache->query(item)==nullptr)
						{

							if (!proginfo_columns_visible)
							{

								// Even if Channel or Description are not visible, load info for programs which started recording
								// within the last day, but only if located directly in the \DataFiles\ folder.
								// This will allow current-recording detection to be done faster.

								if (!item->filename->EndsWith(".rec",StringComparison::InvariantCultureIgnoreCase) ) continue;
								if (!item->full_filename->StartsWith("\\DataFiles\\")) continue;
								if ( Math::Abs ( (now-item->datetime).TotalDays ) > 1.0 ) continue;
								bool in_data_files=false;
								try{ in_data_files = item->full_filename->IndexOf("\\",11)  == -1;} catch(...){};
								if (!in_data_files) continue;


							}


							ri.readsize=0;
							if (this->transfer_in_progress) break;
							this->loadInfo(item,&ri);
							//Console::WriteLine(item->full_filename);
							//printf(" item->size = %d,  readsize=%d\n",(int)item->size, (int)ri.readsize);
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

				//printf("The topfieldBackgroundWork thread stopped!!!!!!!!!!!\n");
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
		int loadComputerDir(String^ start_rename, String^ name_to_select)
		{

			int j;
			ComputerItem^ item;
			array<ComputerItem^>^ items = {};


			String^ dir = this->computerCurrentDirectory;
			trace(1,printf("Load computer dir: %s.\n",dir));



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
				trace(1,printf("GetLogicalDrives\n"));
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
						trace(1,printf("Drive item %s.\n", item->filename));
					}

					drives>>=1;
				}
				this->label1->Text = lang::st_my_computer;//"My Computer";
				settings->changeSetting("ComputerDir","");
				this->clist->Tag = "";
			}
			else   //List contents of actual directory
			{

				trace(1,printf("LoadComputerDirArrayOrNull.\n"));
				items = this->loadComputerDirArrayOrNull(dir);

				if (items==nullptr) return -1;
				/*
				catch(System::IO::IOException ^)
				{
				this->setComputerDir("");
				this->loadComputerDir();
				Console::WriteLine("Unhandled exception in loadComputerDir");
				return -1;
				}
				catch(System::UnauthorizedAccessException ^)
				{
				toolStripStatusLabel1->Text="Access denied: "+dir;
				this->computerUpDir();
				return -1;

				}
				*/

				for (j=0; j<items->Length; j++)
				{
					item = items[j];
					trace(1,printf("GetCachedIconIndexFast: %s.\n",item->filename));
					FileType^ info = this->icons->GetCachedIconIndexFast(item->full_filename,false, item->isdir);
					int ic = info->icon_index;
					if (ic >= 0)
					{
						item->ImageIndex = item->icon_index = ic;
						if (!item->isdir)
						{
							item->file_type = info->file_type;
							//item->populate_subitems();
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
					item->populate_subitems();

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
					if (verbose) printf("reloaded  in loadComputerDir.\n");
					this->updateListViewItems(clist,safe_cast<array<FileItem^>^>(items));
				}
				else

				{
					if (verbose) printf("not reloaded  in loadComputerDir.\n");
					this->listView2->BeginUpdate();
					this->listView2->Items->Clear();

					this->listView2->Items->AddRange(items);

					this->listView2->EndUpdate();
				}

				this->textBox1->Select(0,0);
				this->clist->Tag = dir;


				settings->changeSetting("ComputerDir",dir);
				// Add a drive summary to label1:
				trace(1,printf("Calculate free space.\n"));
				array<long long int>^ freespaceArray = this->computerFreeSpace(dir);
				if (freespaceArray[0] > -1)
				{
					String ^str = " "+lang::st_local+" "+str+"  --  " + HumanReadableSize(freespaceArray[0]) + " "+lang::st_free+" / " + HumanReadableSize(freespaceArray[1])+ " "+lang::st_total;

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
					this->computer_new_folder_time = time(NULL);
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


				trace(1,printf("Set computer background enumerator.\n"));
				this->computer_background_enumerator = q->GetEnumerator();
				this->computer_background_event->Set();

				trace(1,printf("set_filesystemwatcher()\n"));
				this->set_filesystemwatcher(dir);
				trace(1,printf("returned: set_filesystemwatcher()\n"));


				this->computer_needs_refreshing=false;



			}   // (if "My Computer")
			trace(1,printf("Setting listview tag\n"));
			this->listView2->Tag = dir;
			if (!rename_item) this->Arrange2();

			return 0;

		};

		void loadComputerDir(String^ start_rename)
		{
			this->loadComputerDir(start_rename,"");
		}

		int loadComputerDir(void)
		{
			return this->loadComputerDir("","");
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
			if (verbose) printf("loadTopfieldDirArrayOrNull(%s)\n ",path);
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
						if (verbose)
						{
							printf("%lld : %s\n",item->size, item->full_filename);

						}
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
					if (path=="\\DataFiles\\" || path=="\\DataFiles")
					{
						this->data_files_cached = items;
						this->data_files_read_time = DateTime::Now;

					}
					return items;
					break;

				case FAIL:
					//fprintf(stdout, "ERROR: Device reports %s in loadTopfieldDirArray, path %s\n",decode_error(&reply),path);
					this->connection_error_occurred();
					return nullptr;//items;

					break;
				default:
					fprintf(stdout, "ERROR: Unhandled packet\n");
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
				fprintf(stdout, "ERROR: Device reports %s in getTopfieldFreeSpace\n",
					decode_error(&reply));
				this->connection_error_occurred();
				break;

			default:
				fprintf(stdout, "ERROR: Unhandled packet in load_topfield_dir/hdd_size\n");
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
					String^ str= " "+lang::st_toppy;//" Topfield device";

					if (this->pid==0x1100) str=lang::st_toppy2;//"Topfield second device";
					this->label2->Text = str+"  --  "+HumanReadableSize(1024* ((__u64) v.freek))+" "+lang::st_free+" / " + HumanReadableSize(1024*( (__u64) v.totalk)) + " "+lang::st_total;
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
					//item->populate_subitems();
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
				item->populate_subitems();

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
			array<String^>^ headerNames;
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


			ToolStripMenuItem ^mi_pc_proginfo, ^mi_pc_copy, ^mi_pc_move, ^mi_pc_delete, ^mi_pc_show_in_explorer, ^mi_pc_install_firmware, ^mi_pc_choose_columns;
			ToolStripMenuItem ^mi_pc_rename, ^mi_pc_select_all;
			ToolStripMenuItem ^mi_pvr_proginfo, ^mi_pvr_copy, ^mi_pvr_move, ^mi_pvr_delete, ^mi_pvr_rename, ^mi_pvr_select_all;
			array<ToolStripMenuItem^>^ mi_pc_choose_columns_array;
			array<ToolStripMenuItem^>^ mi_pvr_choose_columns_array;

			array<TopfieldItem^>^ data_files_cached;   // Stores last known copy of \DataFiles\ directory.
			DateTime data_files_read_time;  // Time that the \DataFiles\ directory was last loaded
			DateTime recording_in_progress_last_checked; // Time that recording_in_progress() was last called


			String^ watched_directory;

			CommandLine^ commandline;
			int idle_count;
			bool exit_on_completion;
			bool no_prompt;


			array<System::Windows::Forms::ColumnHeader^>^ computerHeaders;


			int finished_constructing;
			System::String^ topfieldCurrentDirectory;
			System::String^ computerCurrentDirectory;


			bool computer_needs_refreshing;

			System::Timers::Timer ^tooltip_timer;
			String ^ tooltip_string;
			Point tooltip_location;
			ListView ^tooltip_listview;


			// "turbo_mode" is equal to the value most recently passed to set_turbo_mode( ).
			// It is usually equal to the actual current turbo mode of the device, except if 
			// set_turbo_mode chose not to enable turbo mode because of a current recording.
			bool^ turbo_mode; 


			// turbo_mode2 is like turbo_mode, except it reflects whether or not set_turbo_mode
			// actually set the turbo mode, after it detected whether there was a current recording
			bool^ turbo_mode2;



			int listView1SortColumn;
			int listView2SortColumn;

			int lasthash;  // (relates to displaying of the program description tooltip)

			array<String^>^ TopfieldClipboard;  
			String^ TopfieldClipboardDirectory;

			static Color cut_background_colour = Color::FromArgb(255,250,105);
			static Color normal_background_colour = Color::FromArgb(255,255,255);


			int last_layout_x, last_layout_y;

			System::Diagnostics::Stopwatch ^stopwatch;
			double listview_click_time;

			Settings^ settings;
			Antares::Icons^ icons;

			bool listView2_selection_was_changed, listView1_selection_was_changed;
			static const long long resume_granularity = 8192;
			static const double check_recording_interval = 120.0; 
			static const long long min_resume_size = 1024*1024;
			bool transfer_in_progress;
			bool firmware_transfer_in_progress;

			int last_topfield_freek;
			time_t last_topfield_freek_time;
			time_t computer_new_folder_time;
			long long bytes_sent_since_last_freek;
			TopfieldFreeSpace last_topfield_freespace;

			static double topfield_minimum_free_megs = 150.0; // Antares won't let the free space get lower than this.
			static double worst_case_fill_rate = 4.5;  // Worst case MB/sec rate that we can imagine the topfield HD being filled up

			int pid;
			int ndev; // number of devices connected

			CopyDialog^ current_copydialog;
			FirmwareInstaller^ current_firmware_installer;

			// used for updating the program details columns in the background;
			System::Collections::IEnumerator ^topfield_background_enumerator;
			System::Collections::IEnumerator ^computer_background_enumerator;

			ProgramInformationCache^ proginfo_cache;
			ListView^ clist;
			ListView^ tlist;

			bool connection_needs_checking;
			//bool close_request;



	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	public: System::Windows::Forms::Panel^  panel1;
	private: 











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
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip1;


	private: System::IO::FileSystemWatcher^  fileSystemWatcher1;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip2;
	private: System::Windows::Forms::CheckBox^  checkBox1;




















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
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->panel3 = (gcnew System::Windows::Forms::Panel());
			this->textBox2 = (gcnew System::Windows::Forms::ComboBox());
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
			this->contextMenuStrip1 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
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
			this->contextMenuStrip2 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->basicIconsSmall = (gcnew System::Windows::Forms::ImageList(this->components));
			this->notifyIcon1 = (gcnew System::Windows::Forms::NotifyIcon(this->components));
			this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->fileSystemWatcher1 = (gcnew System::IO::FileSystemWatcher());
			this->statusStrip1->SuspendLayout();
			this->panel1->SuspendLayout();
			this->panel3->SuspendLayout();
			this->toolStrip2->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel7->SuspendLayout();
			this->panel4->SuspendLayout();
			this->toolStrip1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fileSystemWatcher1))->BeginInit();
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
			this->panel1->Controls->Add(this->checkBox1);
			this->panel1->Controls->Add(this->panel3);
			this->panel1->Controls->Add(this->panel2);
			this->panel1->Controls->Add(this->panel4);
			this->panel1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->panel1->Location = System::Drawing::Point(0, 0);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(880, 580);
			this->panel1->TabIndex = 10;
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->BackColor = System::Drawing::Color::Transparent;
			this->checkBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F));
			this->checkBox1->Location = System::Drawing::Point(472, 13);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 14;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = false;
			// 
			// panel3
			// 
			this->panel3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel3->Controls->Add(this->textBox2);
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
			this->listView1->ContextMenuStrip = this->contextMenuStrip1;
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
			this->listView1->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::listView_MouseClick);
			this->listView1->AfterLabelEdit += gcnew System::Windows::Forms::LabelEditEventHandler(this, &Form1::listView_AfterLabelEdit);
			this->listView1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listView1_SelectedIndexChanged);
			this->listView1->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::listView1_Layout);
			this->listView1->ColumnClick += gcnew System::Windows::Forms::ColumnClickEventHandler(this, &Form1::listView_ColumnClick);
			this->listView1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::listView_MouseMove);
			this->listView1->ColumnWidthChanging += gcnew System::Windows::Forms::ColumnWidthChangingEventHandler(this, &Form1::listView_ColumnWidthChanging);
			this->listView1->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::listView_KeyDown);
			this->listView1->MouseLeave += gcnew System::EventHandler(this, &Form1::listView_MouseLeave);
			// 
			// contextMenuStrip1
			// 
			this->contextMenuStrip1->Name = L"contextMenuStrip1";
			this->contextMenuStrip1->Size = System::Drawing::Size(61, 4);
			this->contextMenuStrip1->ItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &Form1::contextMenuStrip1_ItemClicked);
			this->contextMenuStrip1->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &Form1::contextMenuStrip1_Opening);
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
			this->panel2->Margin = System::Windows::Forms::Padding(0);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(47, 580);
			this->panel2->TabIndex = 7;
			// 
			// checkBox2
			// 
			this->checkBox2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->checkBox2->AutoSize = true;
			this->checkBox2->CheckAlign = System::Drawing::ContentAlignment::TopCenter;
			this->checkBox2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), 
				static_cast<System::Int32>(static_cast<System::Byte>(150)));
			this->checkBox2->Location = System::Drawing::Point(5, 133);
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
			this->listView2->ContextMenuStrip = this->contextMenuStrip2;
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
			this->listView2->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::listView_MouseClick);
			this->listView2->AfterLabelEdit += gcnew System::Windows::Forms::LabelEditEventHandler(this, &Form1::listView_AfterLabelEdit);
			this->listView2->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listView2_SelectedIndexChanged);
			this->listView2->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::listView2_Layout);
			this->listView2->ColumnClick += gcnew System::Windows::Forms::ColumnClickEventHandler(this, &Form1::listView_ColumnClick);
			this->listView2->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::listView_MouseMove);
			this->listView2->ColumnWidthChanging += gcnew System::Windows::Forms::ColumnWidthChangingEventHandler(this, &Form1::listView_ColumnWidthChanging);
			this->listView2->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::listView_KeyDown);
			this->listView2->MouseLeave += gcnew System::EventHandler(this, &Form1::listView_MouseLeave);
			// 
			// contextMenuStrip2
			// 
			this->contextMenuStrip2->Name = L"contextMenuStrip2";
			this->contextMenuStrip2->Size = System::Drawing::Size(61, 4);
			this->contextMenuStrip2->ItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &Form1::contextMenuStrip2_ItemClicked);
			this->contextMenuStrip2->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &Form1::contextMenuStrip2_Opening);
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
			this->basicIconsSmall->Images->SetKeyName(3, L"show_file.ico");
			this->basicIconsSmall->Images->SetKeyName(4, L"left-arrow_small.ico");
			this->basicIconsSmall->Images->SetKeyName(5, L"left-arrow_orange_small.ico");
			this->basicIconsSmall->Images->SetKeyName(6, L"cog.ico");
			this->basicIconsSmall->Images->SetKeyName(7, L"right-arrow_orange_small.ico");
			this->basicIconsSmall->Images->SetKeyName(8, L"right-arrow_small.ico");
			this->basicIconsSmall->Images->SetKeyName(9, L"rename.ico");
			// 
			// notifyIcon1
			// 
			this->notifyIcon1->Text = L"notifyIcon1";
			this->notifyIcon1->Visible = true;
			// 
			// fileSystemWatcher1
			// 
			this->fileSystemWatcher1->EnableRaisingEvents = true;
			this->fileSystemWatcher1->NotifyFilter = System::IO::NotifyFilters::FileName;
			this->fileSystemWatcher1->SynchronizingObject = this;
			this->fileSystemWatcher1->Renamed += gcnew System::IO::RenamedEventHandler(this, &Form1::fileSystemWatcher1_Renamed);
			this->fileSystemWatcher1->Deleted += gcnew System::IO::FileSystemEventHandler(this, &Form1::fileSystemWatcher1_Changed);
			this->fileSystemWatcher1->Created += gcnew System::IO::FileSystemEventHandler(this, &Form1::fileSystemWatcher1_Changed);
			this->fileSystemWatcher1->Changed += gcnew System::IO::FileSystemEventHandler(this, &Form1::fileSystemWatcher1_Changed);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(880, 602);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->statusStrip1);
			this->ForeColor = System::Drawing::SystemColors::ControlText;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"Form1";
			this->Opacity = 0;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Antares  0.9.2-test-6";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->ResizeBegin += gcnew System::EventHandler(this, &Form1::Form1_ResizeBegin);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::Form1_Paint);
			this->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::Form1_Layout);
			this->Move += gcnew System::EventHandler(this, &Form1::Form1_Move);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Form1::Form1_FormClosing);
			this->Resize += gcnew System::EventHandler(this, &Form1::Form1_Resize);
			this->ResizeEnd += gcnew System::EventHandler(this, &Form1::Form1_ResizeEnd);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->panel3->ResumeLayout(false);
			this->toolStrip2->ResumeLayout(false);
			this->toolStrip2->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel2->PerformLayout();
			this->panel7->ResumeLayout(false);
			this->panel7->PerformLayout();
			this->panel4->ResumeLayout(false);
			this->toolStrip1->ResumeLayout(false);
			this->toolStrip1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fileSystemWatcher1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		double is_topfield_path(String^ path)
		{
			if (path->StartsWith("TF:",StringComparison::InvariantCultureIgnoreCase))
				return 1.0;

			int ind = path->IndexOf(':');
			if (ind != 1) 
			{
				if (path->StartsWith("."))
					return -0.5;
				else
					return 0.0;
			}

			String ^ drv = path->Substring(0,2);
			if (Directory::Exists(drv)) return -1.0; else return 0.0;

		}

		System::Void cmdline_error(String^ err)
		{
			Console::WriteLine(err);
			if (this->commandline->exit_on_completion)
				Application::Exit();

		}

		String^ normalize_pvr_commandline_path(String^ path)
		{
			int ind = path->IndexOf(':');
			if (ind==1 || ind==2)
			{
				path = path->Substring(ind+1,path->Length - ind - 1);
			}
			if (!path->StartsWith("\\") )
				path = "\\" + path;
			while(path->EndsWith("\\"))
			{
				path=path->Substring(0, path->Length-1);
			}

			return path;
		}
		String^ normalize_pc_commandline_path(String^ path)
		{
			while(path->EndsWith("\\"))
			{
				path=path->Substring(0, path->Length-1);
			}
			try {
				path = Path::GetFullPath(path);
			}
			catch(...)
			{
				return nullptr;
			}
			return path;
		}

		Regex^ WildcardToRegex(String ^pattern)   //http://www.codeproject.com/KB/recipes/wildcardtoregex.aspx
		{
			return gcnew Regex("^" + Regex::Escape(pattern)->
				Replace("\\*", ".*")->
				Replace("\\?", ".") + "$",
				RegexOptions::IgnoreCase) ;
		}

		int select_pattern(ListView ^listview, String ^ pattern, bool all_dirs, bool not_files, array<String^>^ exclude_patterns)
		{
			Regex^ re = this->WildcardToRegex(pattern);
			int num=0;
			int ne = exclude_patterns->Length;
			array<Regex^>^ exc_re = gcnew array<Regex^>(ne);
			for (int i=0; i<ne; i++) exc_re[i] = this->WildcardToRegex(exclude_patterns[i]);

			for each (ListViewItem^ item in listview->Items )
			{
				String^ filename;
				bool isdir;
				if (listview == this->listView1)
				{
					TopfieldItem^ titem = safe_cast<TopfieldItem^>(item);
					filename = titem->filename;
					isdir = titem->isdir;
				}
				else
				{
					ComputerItem^ citem = safe_cast<ComputerItem^>(item);
					filename = citem->filename;
					isdir = citem->isdir;
				}

				bool is_wild = pattern->Contains("?") || pattern->Contains("*");

				bool exc = false;
				for (int i=0; i<ne; i++)
					if (exc_re[i]->IsMatch(filename)) {exc=true;break;};

				bool match = re->IsMatch(filename);
				if (is_wild && isdir && !not_files) match = false;

				if (   (match   || (all_dirs && isdir) ) && !(!isdir && not_files)  && !exc)
				{
					num ++; 
					item->Selected=true;
				}
				else
					item->Selected=false;


			}
			return num;
		}

		System::Void run_command(CommandLine^ cmdline)
		{


			String^ cmd = cmdline->the_command;
			if (cmdline->e) {this->cmdline_error("");return;};

			if (cmdline->turbo_specified)
			{

				this->checkBox1->Checked = (cmdline->turbo_mode==1);
				this->settings->changeSetting("TurboMode",  cmdline->turbo_mode==1 ? "on" : "off" );
			}

			if (cmd=="cp" || cmd=="mv")
			{


				if (this->fd == NULL)
				{
					this->cmdline_error("ERROR: Could not connect to PVR.");
					return;
				}


				String^ path1 = cmdline->cmd_param1;
				String^ path2 = cmdline->cmd_param2;

				double d1 = this->is_topfield_path(path1);
				double d2 = this->is_topfield_path(path2);
				//Console::WriteLine("path1 = "+path1+" d1="+d1.ToString());
				//Console::WriteLine("path2 = "+path2+" d2="+d2.ToString());
				TransferOptions ^transferoptions = gcnew TransferOptions();

				transferoptions->overwrite_all = cmdline->overwrite_all;


				if (d1==d2)
				{
					this->cmdline_error("ERROR: the parameters to the "+cmd+" command could not be understood.\n(Which one is the PVR, and which one is the PC?)");
					return;
				}

				if (d1< d2)   // From PC to PVR
				{
					String^ pvr_path = this->normalize_pvr_commandline_path(path2);
					String ^src_folder, ^src_pattern;
					bool src_ends_with_slash = false;
					bool is_wild;
					try{
						String^ pc_path = Path::Combine(Environment::CurrentDirectory,path1);

						while(pc_path->EndsWith("\\"))
						{
							pc_path = pc_path->Substring(0,pc_path->Length-1);
							src_ends_with_slash = true;
						}
						int ind = pc_path->LastIndexOf("\\");

						if (ind>2)
							src_folder = pc_path->Substring(0,ind);
						else
							src_folder = pc_path->Substring(0,ind+1);
						src_folder = Path::GetFullPath(src_folder);

						src_pattern = pc_path->Substring(ind+1,pc_path->Length-ind-1);
						is_wild = (src_pattern->Contains("*") || src_pattern->Contains("?"));

					}
					catch(...)
					{
						this->cmdline_error("ERROR: The following path could not accessed on the PC: "+path1);
						return;
					}

					this->setComputerDir(src_folder);
					this->loadComputerDir();

					this->setTopfieldDir(pvr_path);
					array<TopfieldItem^> ^arr = this->loadTopfieldDirArrayOrNull(pvr_path);
					if (arr==nullptr)
					{
						this->cmdline_error("ERROR: The destination path could not be found: "+path2);
						return;
					}
					this->loadTopfieldDir();

					int num = this->select_pattern(this->listView2, src_pattern, cmdline->recurse && is_wild && !src_ends_with_slash, src_ends_with_slash, cmdline->exclude_patterns );



					if (num==0) 
					{
						String^ x = "";
						if (path1->Contains("/")) x="\nNote: you must use a backslash (not slash) to separate directories.\n";
						this->cmdline_error("ERROR: File not found! ("+path1+")"+x);
					}


					transferoptions->exclude_patterns = cmdline->exclude_patterns;
					if (is_wild && !src_ends_with_slash)
						transferoptions->pattern = src_pattern; 

					transferoptions->copymode = cmd=="cp" ? CopyMode::COPY : CopyMode::MOVE;

					transferoptions->never_delete_directories = is_wild && (src_pattern!="*") && !src_ends_with_slash || cmdline->exclude_patterns->Length>0;

					transferoptions->skip_directories_with_no_files = is_wild && (src_pattern!="*") && !src_ends_with_slash;



					// remember, set turbo mode

					this->transfer_selection_to_PVR(transferoptions);



				}
				else         // From PVR to PC
				{


					bool src_ends_with_slash = path1->EndsWith("\\");

					String^ pvr_path = this->normalize_pvr_commandline_path(path1);

					String^ pc_path;

					try{
						pc_path = Path::Combine(Environment::CurrentDirectory,path2);
						pc_path = Path::GetFullPath(pc_path);
						while(pc_path->EndsWith("\\"))
							pc_path = pc_path->Substring(0,pc_path->Length-1);


					}
					catch(...)
					{
						this->cmdline_error("ERROR: The following path could not accessed on the PC: "+path2);
						return;
					}
					if (pc_path->Length == 2 && pc_path->Substring(1,1)==":")
						pc_path = pc_path + "\\";

					while(pvr_path->EndsWith("\\"))
						pvr_path = pvr_path->Substring(0,pvr_path->Length-1);

					if (!pvr_path->StartsWith("\\"))
						pvr_path = pvr_path + "\\";

					int ind = pvr_path->LastIndexOf("\\");
					String ^src_folder, ^src_pattern;				

					src_folder = pvr_path->Substring(0,ind);
					src_pattern = pvr_path->Substring(ind+1,pvr_path->Length-ind-1);

					bool is_wild = src_pattern->Contains("*") || src_pattern->Contains("?");

					if (src_pattern->Length==0)
					{
						this->cmdline_error("ERROR: The source path is invalid ("+path1+")");
						return;
					}

					this->setComputerDir(pc_path);
					int r = this->loadComputerDir();
					if (r!=0)
					{
						this->cmdline_error("ERROR: The destination path "+pc_path+" is invalid or cannot be accessed.");
						return;

					}


					this->setTopfieldDir(src_folder);


					array<TopfieldItem^> ^arr = this->loadTopfieldDirArrayOrNull(src_folder);
					if (arr==nullptr)
					{
						String^ x = "";
						if (path1->Contains("/")) x="\nNote: you must use a backslash (not slash) to separate directories.\n";

						this->cmdline_error("ERROR: The source location could not be found: "
							+
							path1
							+"\n(Note: directory names are case sensitive.)"+x);
						return;
					}


					this->loadTopfieldDir();

					int num = this->select_pattern(this->listView1, src_pattern, cmdline->recurse && is_wild && !src_ends_with_slash, src_ends_with_slash, cmdline->exclude_patterns  );
					if (num==0) 
					{
						String^ x = "";
						if (path1->Contains("/")) x="\nNote: you must use a backslash (not slash) to separate directories.\n";
						this->cmdline_error("ERROR: File not found! ("+path1+")"+x);
						return;
					}

					transferoptions->exclude_patterns = cmdline->exclude_patterns;
					if (is_wild && !src_ends_with_slash)
						transferoptions->pattern = src_pattern; 

					transferoptions->copymode = cmd=="cp" ? CopyMode::COPY : CopyMode::MOVE;

					transferoptions->never_delete_directories = is_wild && (src_pattern!="*") && !src_ends_with_slash || cmdline->exclude_patterns->Length>0;


					transferoptions->skip_directories_with_no_files = is_wild && (src_pattern!="*") && !src_ends_with_slash;

					this->transfer_selection_to_PC(transferoptions);

				}





			}



		}


		System::Void Application_Idle(System::Object^  sender, System::EventArgs^  e)
		{

			//Console::WriteLine("Idle");
			this->idle_count++;
			if (this->idle_count > 1 ) return;

			Application::Idle -= gcnew EventHandler(this, &Form1::Application_Idle);

			//this->fileSystemWatcher1->EnableRaisingEvents=true;
			//this->fileSystemWatcher1->EndInit();

			if (this->commandline->the_command->Length==0 && !this->commandline->dont_free_console)
			{

				try{
#ifndef _DEBUG

					CloseConsole();
#endif
				}catch(...){}

			}


			if (this->commandline->the_command->Length>0)
			{
				this->exit_on_completion = this->commandline->exit_on_completion;
				this->no_prompt = this->commandline->no_prompt;
				this->settings->backup_settings();
				this->run_command(this->commandline);
			}



		}


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

			if (this->computer_needs_refreshing)
			{
				if (  (time(NULL) - this->computer_new_folder_time)>60)
				{
					this->computer_needs_refreshing=false;
					this->loadComputerDir();
				}
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
		System::Void Arrange2a(array<ColumnHeader^>^ cols, String^ type, int client_width, ListView^ listview)
		{
			//array<int> column_inds = type=="PC" ? FileItem::computer_column_inds : FileItem::topfield_column_inds;



			int nc = FileItem::num_computer_columns;
			array<int> ^widths = gcnew array<int>(nc);
			array<bool> ^visible = gcnew array<bool>(nc);
			if (listview == this->tlist) nc=FileItem::num_topfield_columns;

			for (int j=0; j<nc; j++)
			{
				String ^str = type+"_Column"+j.ToString()+"Width";
				widths[j] = Convert::ToInt32(this->settings[str]);

				str = type+"_Column"+j.ToString()+"Visible";
				visible[j] = this->settings[str]=="1";
			}
			bool scaleit = this->settings["RescaleColumns"]=="1";

			if (scaleit)
			{
				int sumw = 0;
				for (int j=0; j<nc; j++)
				{
					if (visible[j]) sumw += widths[j];

				}
				double factor=1.0;
				int scale = Convert::ToInt32(this->settings[type+"_ColumnScale"]);

				if (sumw>scale)
				{
					if (scale<=client_width && client_width<= sumw) factor=1.0;

					if (client_width > sumw) factor = (double) client_width / sumw;

					if (client_width < scale) factor = (double) client_width / scale;

				}
				else
				{
					if (client_width >= scale) 
						factor = (double) client_width / scale;
					else 
						if (client_width>sumw) factor = 1.0;
						else 
							factor = (double) client_width / sumw;
				}

				if (factor != 1.0)
				{
					double err=0;
					for (int j=0; j<nc; j++)
					{
						double w = factor *widths[j] - err;
						int rw =   (int) ( w + .5) ; 

						err = rw - w;
						widths[j]=rw;


					}
				}




			}

			listview->BeginUpdate();
			for (int j=0; j<nc; j++)
			{
				if (widths[j]<0) widths[j]=0;
				cols[j]->Width = widths[j];
			}
			listview->EndUpdate();



		}

		System::Void Arrange2a_old(array<ColumnHeader^>^ cols, String^ type, int client_width, ListView^ listview)
		{

			double widths0[] = {140, 60, 60, 120,60,140};
			double mwidths[] = {0,  70, 70, 130, 60, 0};


			//for (int j=0; j<cols->Length; j++) cols[j]->AutoResize(ColumnHeaderAutoResizeStyle::ColumnContent);
			//return;


			ListView::ColumnHeaderCollection ^chc = listview->Columns;

			bool something_visible = false;
			int ncols = cols->Length;
			for (int j=0; j<ncols; j++)
			{
				String^ setting_name = type+"_Column"+j.ToString()+"Visible";
				if (this->settings[setting_name] != "1") 
				{
					//widths0[j]=0; mwidths[j]=0;
					//if (chc->Contains(cols[j]))
					//	chc->Remove(cols[j]);



				}
				else{
					something_visible=true;
					//if (!chc->Contains(cols[j]))
					//	chc->Add(cols[j]);
				}
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
			trace(1,printf("Arrange2()\n"));
			if (this->finished_constructing ==1)
			{

				double cw1,cw2;

				cw1 = listView1->Width;
				cw2 = listView1->ClientSize.Width;
				cw1 = cw1 < cw2 ? cw1 : cw2;

				this->Arrange2a( this->topfieldHeaders, "PVR", (int) cw1, listView1);


				cw1 = listView2->Width;
				cw2 = listView2->ClientSize.Width;
				cw1 = cw1 < cw2 ? cw1 : cw2;

				this->Arrange2a( this->computerHeaders, "PC", (int) cw1, listView2);

				trace(1,printf("Arrange2()  (return--) \n"));

				return;

			}
			this->textBox2->Update();
			this->textBox2->Select(0,0);
			this->textBox1->Select(0,0);


			trace(1,printf("Arrange2()  (return) \n"));

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

			int cw = this->checkBox2->Width;
			int pw = this->panel2->Width;
			//if (cw > pw)
			p5.X = (pw-cw-1)/2;


			this->checkBox2->Location=p5;



			cw = this->checkBox1->Width;
			Point p6 =  this->checkBox1->Location;
			int x1,x2,x3;
			x1 = this->panel3->Width - cw + 8;
			x2 = this->toolStripButton11->Bounds.Right  + 12;
			x3 = this->panel4->Location.X - cw  + 18;
			int x = x1;
			if (x2>x) x=x2;
			if (x3<x) x=x3;
			p6.X = x;

			this->checkBox1->Location=p6;
			this->checkBox1->BringToFront();


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
			//Console::WriteLine("Layout");

			//this->Arrange();

			ListViewSelectionDelegate^ d = gcnew ListViewSelectionDelegate(this, &Form1::deselectComboBoxes);
			this->BeginInvoke(d);

		}

		bool location_is_sane(int width, int height, int X, int Y)
		{
			if (width<500 || height < 250) return false;
			if (Y<-32) return false;

			Screen ^screen = System::Windows::Forms::Screen::PrimaryScreen;
			int s_width = screen->Bounds.Width;
			int s_height = screen->Bounds.Height;


			if (width > s_width + 100) return false;
			if (height > s_height + 100) return false;

			if (Y>s_height-32) return false;

			double off_width = 0;
			if (X<0) off_width -= X;
			if (X+width>s_width) off_width+=(X+width-s_width);

			if (off_width / width > .3) return false;

			double off_height = 0;
			if (Y<0) off_height = -Y;
			if (Y+height > s_height) off_height -= (Y+height - s_height);
			if (off_height / height > .3) return false;

			return true;
		}

		System::Void save_location(void)
		{

			if (this->finished_constructing==0) return;

			//Console::WriteLine(this->Size);
			//Console::WriteLine(this->Location);


			//Console::WriteLine(System::Windows::Forms::Screen::PrimaryScreen);
			bool is_maximized = (this->WindowState == FormWindowState::Maximized);
			bool is_minimized = (this->WindowState == FormWindowState::Minimized);


			this->settings->changeSetting("Maximized",  ((int) is_maximized).ToString());
			if (is_minimized || is_maximized) return;



			int width = this->Size.Width;
			int height = this->Size.Height;
			int X = this->Location.X;
			int Y = this->Location.Y;


			if (this->location_is_sane(width, height, X, Y)) 
			{

				this->settings->changeSetting("Width", width.ToString());
				this->settings->changeSetting("Height", height.ToString());
				this->settings->changeSetting("X",X.ToString());
				this->settings->changeSetting("Y",Y.ToString());

			}


		}

		System::Void Form1_Resize(System::Object^  sender, System::EventArgs^  e) {

			if (this->WindowState != FormWindowState::Minimized)
				this->Arrange2();
			//this->Arrange();			
			//Console::WriteLine("Resize");
			this->save_location();

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
		System::Void firmware_transfer_ended(void)
		{



			if (this->InvokeRequired)
			{
				//Monitor::Exit(this->locker);
				//printf("0. Transfer ended.\n");
				TransferEndedCallback^ d = gcnew TransferEndedCallback(this, &Form1::firmware_transfer_ended);
				this->BeginInvoke(d);

			}
			else
			{
				this->EnableComponents(true);

				this->panel7->Enabled=true;
				this->textBox2->Enabled=true;
				this->checkBox2->Enabled=true;

				this->Update();

				FirmwareInstaller^ firmware_dialog = this->current_firmware_installer;

				if (firmware_dialog==nullptr) return;

				if (firmware_dialog->error_str->Length > 0)
				{
					MessageBox::Show(this,firmware_dialog->error_str,"Error",MessageBoxButtons::OK);						
				}


				System::Threading::Thread ^thr = firmware_dialog->thread;
				if (thr != nullptr) thr->Join();

				this->transfer_in_progress=false;
				this->firmware_transfer_in_progress=false;


				this->CheckConnection();

				this->loadTopfieldDir();

				firmware_dialog->Close();
				this->current_firmware_installer = nullptr;

			}




		}

		System::Void firmware_installer_cancelled(System::Object^  sender, System::EventArgs^  e) {

			Console::WriteLine("Firmware installation cancelled.");


			FirmwareInstaller^ firmware_dialog = this->current_firmware_installer;

			if (firmware_dialog) {
				firmware_dialog->button1->Enabled=false;
				firmware_dialog->cancelled=true;
				firmware_dialog->button1->Text="Please wait...";
			}

			/*
			this->EnableComponents(true);
			this->transfer_in_progress=false;
			this->firmware_transfer_in_progress=false;

			if (firmware_dialog) firmware_dialog->Close();
			this->current_firmware_installer=nullptr;
			*/

		}

		System::Void transfer_firmware_to_PVR(Object^ input)
		{
			struct tf_packet reply;
			int r;
			time_t reboot_time=0;
			tf_fw_data_t fw_data;
			char buffer[MAX_DATA_SIZE];
			array<Byte>^ inp_buffer = gcnew array<unsigned char>(MAX_DATA_SIZE);
			FirmwareInstaller^ firmware_dialog = safe_cast<FirmwareInstaller^>(input);

			Monitor::Enter(this->locker);
			System::IO::FileStream ^src_file = nullptr;

			unsigned long long offset;
			while(1) // Loop until we successfully initiate upload. Todo: exit on cancel.
			{

				if (firmware_dialog->cancelled) goto out;
				r = tf_fw_upload(this->fd, &fw_data);
				if (r<0) 
				{
					Thread::Sleep(100);
					this->CheckConnection();
					if (this->fd==NULL)
					{
						if (  (time(NULL)-reboot_time) < 6)
							firmware_dialog->update_status_text("Rebooting...",false);
						else
							firmware_dialog->update_status_text("The PVR is off or disconnected.\r\n\r\nEnsure the PVR is off (standby) and plugged into the PC. Then turn the PVR on.",false);  
					}
					else
					{

						if (this->ndev>1)
						{
							firmware_dialog->error_str="There is more than one PVR connected! \r\n The firmware installation cannot proceed.";
							goto out;
						}

						firmware_dialog->update_status_text("The PVR is on and connected.\r\n\r\nTurn the PVR off (standby) and then on again.\r\n\r\nOr, click Reboot PVR.",true);

						if (firmware_dialog->reboot_requested)
						{

							firmware_dialog->reboot_requested=false;
							this->absorb_late_packets(5,50);
							int r = do_cmd_reset(this->fd);
							if (r==0) reboot_time=time(NULL);


						}

					}

					continue;

				}
				else
					break;

			}
			FileInfo^ file_info;
			long long file_size;
			try{
				src_file = File::Open(firmware_dialog->path, System::IO::FileMode::Open, System::IO::FileAccess::Read,System::IO::FileShare::Read);
				file_info = gcnew FileInfo(firmware_dialog->path);
				file_size = file_info->Length;
			} catch(...) {
				firmware_dialog->error_str="Unable to open file: "+firmware_dialog->path;
				goto out;
			};


			firmware_dialog->show_reboot=false;


			int perc = 0;
			String^ error_message="";

			while(r==0)
			{

				size_t len;

				if (offset != fw_data.offset)
				{
					printf("Warning:  offset=%d fs_data.offset=%d\n",(int) offset, (int) fw_data.offset);  
					bool seek_success=false;
					try{
						src_file->Seek(fw_data.offset,SeekOrigin::Begin); 
						seek_success=true;
					}catch(...){}
					if (!seek_success) 
					{
						firmware_dialog->error_str="Error reading from file (seeking). offset = " + offset.ToString() +", fw_data.offset="+fw_data.offset.ToString();
						goto out;
					}

				}
				if (fw_data.len > MAX_DATA_SIZE)
				{
					printf("Error: fw_data.len=%d, MAX_DATA_SIZE=%d\n",fw_data.len, MAX_DATA_SIZE);
					error_message="Error during transfer: fw_data.len > MAX_DATA_SIZE";
					r=-9;
					break;
				}

				int w=0;

				try{
					w = src_file->Read(inp_buffer, 0, fw_data.len);
				}catch(...) {
					firmware_dialog->error_str="Error reading from file: "+firmware_dialog->path;
					goto out;
				}
				if (w!=fw_data.len)
				{
					firmware_dialog->error_str="Error reading from file: "+firmware_dialog->path;
					goto out;

				}

				Marshal::Copy(inp_buffer,0,System::IntPtr( &buffer[0]),w);

				offset += w;
				int next_perc = (int)  ( 100*offset/file_size );
				if (next_perc/5 > perc/5)
				{
					firmware_dialog->update_status_text("Tansferring firmware:   " + next_perc.ToString() + "%");
					perc=next_perc;
				}

				/*fprintf(stderr, "Before tf_fw_upload_next() offset=%ld\n", offset);*/
				r = tf_fw_upload_next(this->fd, buffer, w, &fw_data);
				/*fprintf(stderr, "After tf_fw_upload_next() ret=%d, offset=%ld, len=%d, fw_data.offset=%lu, fw_data.len=%d\n", ret, offset, len, fw_data.offset, fw_data.len);*/


			}


			bool success;
			if (r == 1) {
				firmware_dialog->cancel_text="Close";
				//printf("Firmware upgrade completed successully -- you must reboot\n");
				firmware_dialog->update_status_text("The firmware was transferred successfully!"
					+ "\r\nThe PVR will install the firmware. " 
					+ "\r\n\r\nAntares will attempt to automatically reboot the PVR."
					);
				success=true;
			}
			else if (r < 0) {
				success=false;
				//printf("Failed to upgrade firmware -- you must reboot\n");
				firmware_dialog->cancel_text = "Close";
				if (!String::IsNullOrEmpty(error_message))
					firmware_dialog->update_status_text(error_message+"\r\nAntares will attempt to automatically reboot the PVR.");
				else
					firmware_dialog->update_status_text("The firmware upgrade failed. Are you sure the file is the correct version for your PVR?"
					+"\r\nAntares will attempt to automatically reboot the PVR.");
				Thread::Sleep(2000);


			}



			Thread::Sleep(500);
			bool reboot_success=false;
			int reboot_successes=0;
			for (int j=0; j<5; j++)
			{
				r=tf_fw_reboot(this->fd);
				printf("tf_fw_reboot returned %d. fd=%ld\n",r,(long int) this->fd);
				if (r==0) {reboot_success=true;reboot_successes++;}

				Thread::Sleep(200);

				if (reboot_successes>1) break;
				/*
				if (!r) this->CheckConnection()
				if (this->fd==null)
				{
				firmware_dialog->cancel_text="Close";
				if (reboot_success)
				firmware_dialog->update_status_text("The PVR is rebooting. You can close this window.");
				else
				firmware_dialog->update_status_text("When the firmware installation is complete, .");
				break;
				}
				if (!r) break;
				reboot_success=true;
				*/

			}




			while(1)
			{
				Thread::Sleep(100);
				if (firmware_dialog->cancelled) break;
			}



out:
			try{ src_file->Close();}catch(...){};
			Monitor::Exit(this->locker);
			this->firmware_transfer_ended();
		}


		System::Void firmware_click_install(System::Object^  sender, System::EventArgs^  e) {
			FirmwareInstaller^ firmware_dialog = this->current_firmware_installer;
			if (!firmware_dialog) return;

			//firmware_dialog->button3->Enabled=false;

			Thread^ thread = gcnew Thread(gcnew ParameterizedThreadStart(this,&Form1::transfer_firmware_to_PVR));
			thread->Name = "transfer_firmware_to_PVR";
			firmware_dialog->thread = thread;
			thread->Start(firmware_dialog);
		}

		System::Void install_firmware(String^ path)

		{
			if (this->transfer_in_progress || this->firmware_transfer_in_progress) return;



			if(::DialogResult::Yes != MessageBox::Show(this,"Do you want to install this firmware to your PVR?\n\n    "+path,"Really install firmware?",MessageBoxButtons::YesNo))
				return;

			//this->TransferBegan();
			this->transfer_in_progress=true;
			this->firmware_transfer_in_progress=true;


			FirmwareInstaller^ firmware_dialog = gcnew FirmwareInstaller();
			this->current_firmware_installer = firmware_dialog;

			firmware_dialog->TopLevel = false;

			this->Controls->Add(firmware_dialog);
			firmware_dialog->Show();
			this->EnableComponents(false);
			firmware_dialog->Location = System::Drawing::Point( (this->Width - firmware_dialog->Width)/2, -50+(this->Height - firmware_dialog->Height)/2);
			firmware_dialog->BringToFront();
			firmware_dialog->path = path;
			firmware_dialog->textBox1->Text = path;
			firmware_dialog->parent_form = this;
			//firmware_dialog->textBox2->Text = "The file you selected contains Topfield firmware. To install it to your device, click 'Install'.";
			firmware_dialog->button1->Click += gcnew System::EventHandler(this, &Form1::firmware_installer_cancelled);
			//firmware_dialog->button3->Click += gcnew System::EventHandler(this, &Form1::firmware_click_install);

			firmware_dialog->cancel_text="Cancel";
			this->firmware_click_install(nullptr, nullptr);






		}



		System::Void listView2_ItemActivate(System::Object^  sender, System::EventArgs^  e) {
			ListView^ listview = (ListView^) sender;
			//Console::WriteLine("Activated (2)");
			//ComputerItem^ item = (ComputerItem^) sender;
			//Console::WriteLine(item->Text);

			ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
			int cnt = selected->Count;

			if (cnt != 1) return;

			ComputerItem^ item = safe_cast<ComputerItem^> (selected[0]);

			if(item->isdir)
			{
				String ^old = this->computerCurrentDirectory;
				String^ dir = Path::Combine(this->computerCurrentDirectory,item->Text);
				this->setComputerDir(dir);
				int r = this->loadComputerDir();
				if (r<0)
				{
					this->setComputerDir(old);
					this->loadComputerDir();
					this->toolStripStatusLabel1->Text = lang::st_denied+" "+dir; // Access denied:
					return;
				}


				this->add_path_to_history(this->textBox1, this->computerCurrentDirectory);
				return;

			}


			if (item->full_filename->EndsWith(".tfd",StringComparison::CurrentCultureIgnoreCase))
			{

				this->install_firmware(item->full_filename);
				return;
			}


			this->ViewInfo(listview);






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
			if (this->commandline->showgui)
				copydialog->BringToFront();
			this->panel1->Dock=DockStyle::Fill;
		}

		System::Void ShowCopyDialog(CopyDialog^ copydialog)
		{
			this->current_copydialog = copydialog;
			copydialog->TopLevel = false;
			copydialog->parent_panel1 = this->panel1;
			//copydialog->Dock= DockStyle::Bottom;//copydialog->FormBorderStyle = Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->EnableComponents(false);
			this->Controls->Add(copydialog);


			//if (this->commandline->showgui)
			copydialog->Show();


			this->CentreCopyDialog(copydialog,-100);



			//this->panel1->Dock = DockStyle::Top;
			if (this->commandline->showgui)
				copydialog->BringToFront();
			//this->panel1->BringToFront();

		}


		System::Void TransferBegan(void)
		{
			this->panel7->Enabled=false;
			this->textBox2->Enabled=false;
			this->checkBox2->Enabled=false;

			try{
				if (this->settings["prevent_sleep_mode"]=="1")
					Antares::disable_sleep_mode();
			} catch(...){};

		}


		System::Void TransferEnded(void)
		{


			if (this->InvokeRequired)
			{
				Monitor::Exit(this->locker);
				//printf("0. Transfer ended.\n");
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

				//printf("1. Enable components\n");

				CopyDialog^ copydialog = this->current_copydialog;
				if (copydialog==nullptr) return;
				//printf("2. Enable components\n");



				this->transfer_in_progress=false;

				if (this->checkBox1->Checked != copydialog->turbo_request)
					this->checkBox1->Checked = copydialog->turbo_request;

				if (copydialog->file_error->Length > 0)
				{
					if (this->Visible)
						MessageBox::Show(this,copydialog->file_error,lang::st_error,MessageBoxButtons::OK);
					else
						Console::WriteLine(copydialog->file_error);
				}

				//if (copydialog->copydirection == CopyDirection::PVR_TO_PC || copydialog->copymode == CopyMode::MOVE)
				this->loadComputerDir();

				//if (copydialog->copydirection == CopyDirection::PC_TO_PVR || copydialog->copymode == CopyMode::MOVE)
				this->loadTopfieldDir();


				if (!copydialog->is_closed)
					copydialog->close_request_threadsafe();

				Antares::enable_sleep_mode();
				if (copydialog->completed && copydialog->file_error->Length==0)
				{
					int oc=-1;
					try {oc=OnCompletionAction::options[copydialog->on_completion];}catch(...){};
					if (oc>-1)
					{
						bool r;
						switch(oc)
						{
						case OnCompletionAction::DO_NOTHING:
							break;
						case OnCompletionAction::SLEEP:
							r=Application::SetSuspendState(PowerState::Suspend, false, false);     /// ..., force, disableWakeEvent)
							printf("SetSuspendState returned %d\n",(int) r);
							break;
						case OnCompletionAction::HIBERNATE:
							Application::SetSuspendState(PowerState::Hibernate, false, false);
							break;
						case OnCompletionAction::SHUTDOWN:
							System::Diagnostics::Process::Start("shutdown.exe", "-s -t 00");
							break;

						}
					}

				}


				this->current_copydialog = nullptr;


				if (this->exit_on_completion)
				{
					this->settings->restore_settings();
					Application::Exit();
				}
				this->no_prompt=false;


			}


		}




		/////////////////////////////////////////////////////////////////////////////////


		System::Void transfer_to_PC(Object^ input){
			// Worker thread for doing the transfer from PVR -> PC
			CopyDialog^ copydialog = safe_cast<CopyDialog^>(input);
			//////////////
			Monitor::Enter(this->locker);
			//////////////
			if (this->commandline->showgui)
			{
				while(copydialog->loaded==false)
				{
					Thread::Sleep(100);
				}
			}

			if (this->commandline->verbose) printf("transfer_to_PC\n");


			copydialog->copydirection = CopyDirection::PVR_TO_PC;
			copydialog->update_dialog_threadsafe();
			TransferOptions ^transferoptions = copydialog->transferoptions;
			int numitems = copydialog->numfiles;

			//int this_overwrite_action;
			long long topfield_file_offset=0;
			long long probable_minimum_received_offset=-1;
			array<String^>^          dest_filename      = copydialog->dest_filename;
			array<bool>^             dest_exists        = copydialog->dest_exists;
			array<long long int>^    dest_size          = copydialog->dest_size;
			array<int>^              overwrite_action   = copydialog->overwrite_action;
			array<long long int>^    current_offsets    = copydialog->current_offsets;
			array<long long int>^    src_sizes          = copydialog->filesizes;
			array<FileItem^>^        src_items          = copydialog->src_items;
			array<bool>^             filtered_dir_has_no_files =  copydialog->filtered_dir_has_no_files;
			array<bool>^             source_deleted     = gcnew array<bool>(numitems); for (int i=0; i<numitems; i++) source_deleted[i]=false;

			array<int>^              num_fails          = gcnew array<int>(numitems); for (int i=0; i<numitems; i++) num_fails[i]=0;
			array<String^>^          failed_filenames   = gcnew array<String^>(0);

			copydialog->maximum_successful_index=-1;
			for (int i=0; i<numitems; i++)
			{
				copydialog->current_index=i;

				TopfieldItem^ item = safe_cast<TopfieldItem^>(src_items[i]);

				if (item->isdir)
				{
					if (transferoptions->skip_directories_with_no_files && filtered_dir_has_no_files[i]) continue;
					if (File::Exists(dest_filename[i]) && !Directory::Exists(dest_filename[i]))
					{

						//copydialog->file_error = "The folder "+dest_filename[i]+" could not be created because a file of that name already exists. Aborting transfer.";
						copydialog->file_error = String::Format(lang::c_folder_file_clash, dest_filename[i]);
						goto end_copy_to_pc;                         

					}
					if (!Directory::Exists(dest_filename[i]))
					{
						try {
							Directory::CreateDirectory(dest_filename[i]);
						}
						catch (...)
						{


							//copydialog->file_error = "The folder "+dest_filename[i]+" could not be created. Aborting transfer.";
							copydialog->file_error = String::Format(lang::c_folder_error, dest_filename[i]);

							goto end_copy_to_pc;

						}
					}

					if (copydialog->copymode == CopyMode::MOVE && i==numitems-1 && !transferoptions->never_delete_directories)
					{


						array<TopfieldItem^>^ dirarray = this->loadTopfieldDirArrayOrNull(item->full_filename);
						if (dirarray != nullptr && dirarray->Length==0)
						{

							int dr = this->deleteTopfieldPath(item->full_filename);

							if (dr>=0) source_deleted[i]=true;
						}
					}
					copydialog->success(i);
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
				bool recording_in_progress_needs_checking = false;

				if(0)
				{
restart_copy_to_pc:
					copydialog->usb_error=false;
					has_restarted=true;
					recording_in_progress_needs_checking = false;
					topfield_file_offset=0;
					copydialog->reset_rate();

					if (num_fails[i] > 3)
					{
						int nff = failed_filenames->Length;

						Array::Resize(failed_filenames,nff+1);
						failed_filenames[nff] = full_source_filename;
						continue;
					}

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
					copydialog->success(i);
					//Console::WriteLine(item->full_filename + "   [Skipping]");
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
					copydialog->file_error = lang::c_bad_location;//"Antares cannot save the file to the location you chose. Please select another location and try again.";

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
				int file_ind = copydialog->file_indices[i];
				int max_file_ind = copydialog->file_indices[numitems-1];
				if (topfield_file_offset==0)
					printf("  %2d / %2d : %s\n",file_ind,max_file_ind,item->full_filename);
				else
					printf("  %2d / %2d : %s       [Resuming]\n",file_ind,max_file_ind,item->full_filename);

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
				if (verbose) printf("Start of transfer: topfield_file_offset=%lld \n",(long long) topfield_file_offset);


				int update=0;
				while(1)
				{

					update = update+1;


					r = get_tf_packet1(fd, &reply,  0);




					if (r<=0)
					{
						if (verbose) printf("r=%d in transfer_to_PC / get_tf_packet \n",r);
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
							if (verbose) printf("DATA_HDD_FILE_START, bytecount=%lld \n",(long long) bytecount);

							send_success(fd);
							state = DATA;
						}
						else
						{
							fprintf(stdout,
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

							if (verbose) printf("HDD_FILE_DATA, offset=%lld  dataLen=%lld\n",(long long) offset, (long long) dataLen);

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
								if (verbose) printf("An I/O error occurred in transfer_to_PC / Write\n");
								io_error=true;
								goto out;
							}
							if (topfield_file_offset != offset)
							{
								printf("Warning: offset mismatch! %lld %lld \n",(long long) topfield_file_offset, (long long) offset);
								// TODO: Handle this type of error
							}

							topfield_file_offset+=dataLen;
							probable_minimum_received_offset=topfield_file_offset;
							copydialog->new_packet(dataLen);

							bytes_received += dataLen;
							total_bytes_received +=dataLen;
							if (topfield_file_offset > item->size) printf("Warning: topfield_file_offset>item->size     %lld >  %lld  \n",(long long) topfield_file_offset, (long long) item->size);else
								//copydialog->total_offset = total_bytes_received;
								copydialog->current_offsets[i] = topfield_file_offset;//bytes_received;


							copydialog->current_bytes_received = bytes_received;
							copydialog->total_bytes_received = total_bytes_received;
							if (update%4==0)
							{
								if (verbose) printf("update_dialog_threadsafe\n");
								copydialog->update_dialog_threadsafe();
							}

							if (copydialog->cancelled == true)
							{
								if (verbose) printf("CANCELLING because of copy dialog.\n");
								send_cancel(fd);
								state = ABORT;
								goto out;

							}

							if (copydialog->turbo_request != *this->turbo_mode)
							{
								if (verbose) printf("Need to change turbo mode.\n");
								turbo_changed=true;
								copydialog->update_dialog_threadsafe();

								send_cancel(fd);
								state=ABORT;
								goto out;
							}

							if (update % 16 == 0)
							{
								// If turbo mode currently disabled due to recording, occasionally re-check current recording
								if (*this->turbo_mode 
									&& (*this->turbo_mode != *this->turbo_mode2) 
									&& Math::Abs( (DateTime::Now - this->recording_in_progress_last_checked).TotalSeconds) > this->check_recording_interval
									&& !copydialog->cancelled
									&& copydialog->current_offsets[i] > 0
									&& copydialog->current_offsets[i] < copydialog->filesizes[i]
								)
								{

									send_cancel(fd);
									state=ABORT;
									absorb_late_packets(4,100);
									recording_in_progress_needs_checking=true;
									goto out;

								}
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
						if (verbose) printf("DATA_HDD_FILE_END\n");
						result = 0;
						copydialog->success(i);
						goto out;
						break;

					case FAIL:
						printf("ERROR: Device reports %s in transfer_to_PC\n",
							decode_error(&reply));
						this->connection_error_occurred();
						send_cancel(fd);
						state = ABORT;
						copydialog->usb_error=true;
						if ( get_u32(reply.data)==3) num_fails[i]++;
						break;

					case SUCCESS:
						if (verbose) printf("SUCCESS.    state=%d\n",state);
						if (state==DATA)
						{
							goto out;
						}
						break;

					default:
						printf("ERROR: Unhandled packet (cmd 0x%x) in transfer_to_PC\n",
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
					if (verbose) printf("Setting length of destination file to %lld\n",copydialog->current_offsets[i]);
					dest_file->SetLength(copydialog->current_offsets[i]);
				} catch(...){
					if (verbose) printf("Setting length didn't work.\n");
				};

				try
				{
					dest_file->Close();
					File::SetCreationTime(full_dest_filename, item->datetime);
					File::SetLastWriteTime(full_dest_filename, item->datetime);
				}
				catch(...)
				{
					if (verbose) printf("Closing, and setting times, didn't work.\n");
				}

				if (copydialog->cancelled==true) break;

				if (copydialog->usb_error)
				{
					this->connection_error_occurred();

					Monitor::Exit(this->locker);
					int wfc = this->wait_for_connection(copydialog);
					if (verbose) printf("transfer_to_PC / wait_for_connection returned %d\n",wfc);
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
						copydialog->file_error = lang::c_error_writing;//"An error occurred writing the file to your computer.";
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

				// If turbo mode currently disabled due to recording, occasionally re-check current recording
				if (*this->turbo_mode 
					&& (*this->turbo_mode != *this->turbo_mode2) 
					&& Math::Abs( (DateTime::Now - this->recording_in_progress_last_checked).TotalSeconds) > this->check_recording_interval/4.0
					&& !copydialog->cancelled
					)
				{
					this->set_turbo_mode(*this->turbo_mode);
					copydialog->reset_rate();
					if (recording_in_progress_needs_checking)
						goto restart_copy_to_pc;
				}


				if (!copydialog->cancelled) {

					if (copydialog->current_offsets[i] == copydialog->filesizes[i])
						copydialog->maximum_successful_index=i;
					else
						goto restart_copy_to_pc;   // This path probably shouldn't ever happen

				};

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

					if (!transferoptions->never_delete_directories)
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

			int L = failed_filenames->Length;
			if (L > 0)
			{
				if (copydialog->file_error->Length > 0) copydialog->file_error  += "\n";
				//copydialog->file_error+="The following file"; if (L>1) copydialog->file_error+="s";
				//copydialog->file_error+=" could not be accessed on the PVR:\n";
				if (L>1)

					copydialog->file_error += lang::c_no_access_pvr_plural;
				else
					copydialog->file_error += lang::c_no_access_pvr;
				copydialog->file_error += "\n";




				for (int i=0; i<L; i++)
				{				
					copydialog->file_error+=failed_filenames[i]+"\n";
					if (i>4)
					{
						copydialog->file_error+=" ... ("+ (L-i-1).ToString() +" more)\n";
						break;
					}
				}
			}

			if (verbose)
			{
				Console::WriteLine(copydialog->file_error);
			}


		}


		///////////////////////////////////////////////////////////////////////////

		System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {    

			this->transfer_selection_to_PC(gcnew TransferOptions());
		}
		void transfer_selection_to_PC(TransferOptions^ transferoptions)
		{
			// Copy files from Topfield to computer

			if (this->transfer_in_progress) return;
			const int max_folders = 1000;

			CopyMode copymode = transferoptions->copymode;

			if (copymode == CopyMode::UNDEFINED)
				copymode = this->getCopyMode();


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
			copydialog->commandline=this->commandline;
			copydialog->transferoptions = transferoptions;
			copydialog->turbo_request = (this->settings["TurboMode"]=="on");
			//copydialog->showCopyDialog();


			if (copymode==CopyMode::COPY)
				copydialog->window_title1=lang::c_title1_copy;//"Copying File";
			else
				copydialog->window_title1=lang::c_title1_move;//"Moving File";

			copydialog->window_title2=lang::c_title2_to_pc;//"[PVR --> PC]";
			copydialog->Text = copydialog->window_title1 + " ... "+copydialog->window_title2; 
			//copydialog->Text = copydialog->window_title;

			copydialog->tiny_size();
			copydialog->label3->Text=lang::c_finding;//"Finding files...";
			Console::WriteLine(lang::c_finding);
			this->ShowCopyDialog(copydialog);

			copydialog->Update();



			array<TopfieldItem^>^ items = gcnew array<TopfieldItem^>(selected->Count);
			selected->CopyTo(items,0);

			array<array<TopfieldItem^>^>^ items_by_folder = gcnew array<array<TopfieldItem^>^>(max_folders);

			Regex ^pattern_regex;
			array<Regex^>^ exclude_regexes = gcnew array<Regex^>(transferoptions->exclude_patterns->Length);
			for (int ii = 0; ii<exclude_regexes->Length; ii++) exclude_regexes[ii]=this->WildcardToRegex(transferoptions->exclude_patterns[ii]);
			bool use_pattern = false;
			if (transferoptions->pattern->Length > 0)
			{
				use_pattern=true;
				pattern_regex = this->WildcardToRegex(transferoptions->pattern);
			}

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


						/////// Filter items if necessary
						int i2=0;
						for (int i1=0; i1<items->Length; i1++)
						{
							TopfieldItem^ it = items[i1];

							if (!it->isdir)
							{
								if (use_pattern && !pattern_regex->IsMatch(it->filename)) {continue;}

								bool exc = false;
								for each (Regex^ re in exclude_regexes)
									if (re->IsMatch(it->filename)) {exc = true;break;};
								if (exc) continue;

							}
							if (i1 != i2) items[i2]=items[i1]; i2++;

						}
						if (i2 != items->Length)
							Array::Resize(items,i2);
						//// (end filter code)


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
			array<DateTime>^         src_date = gcnew array<DateTime>(numitems);
			array<long long int>^    dest_size = gcnew array<long long int>(numitems);
			array<long long int>^    src_sizes = gcnew array<long long int>(numitems);
			array<String^>^          dest_filename= gcnew array<String^>(numitems);
			array<int>^              overwrite_category=gcnew array<int>(numitems);
			array<int>^              overwrite_action = gcnew array<int>(numitems);
			array<long long int>^    current_offsets = gcnew array<long long int>(numitems);
			array<int>^              file_indices = gcnew array<int>(numitems);
			array<bool>^             filtered_dir_has_no_files = gcnew array<bool>(numitems);


			array<int>^ num_cat = {0,0,0}; //numbers of existing files (divided by category)
			int num_exist=0;
			int num_dir_exist=0;
			int num_dir_missing=0;
			array<String^>^ files_cat = {"","",""};

			for (int i=0; i<numitems; i++) overwrite_action[i]=OVERWRITE;


			for (int i=0; i<numitems; i++)
			{
				TopfieldItem^ item = src_items[i];

				if (!item->isdir) continue;
				String^ x = item->full_filename;
				if (!x->EndsWith("\\")) x = x + "\\";

				bool file_found = false;
				for (int j=0; j<numitems; j++)
				{
					if (j==i || src_items[j]->isdir) continue;
					String^ y = src_items[j]->full_filename;
					if (y->StartsWith(x))
					{
						file_found=true;
						break;
					}
				}
				filtered_dir_has_no_files[i] = !file_found;
			}

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

					if (transferoptions->skip_directories_with_no_files && filtered_dir_has_no_files[ind]) 
					{
						num_dir_exist++;   // A bit hacky adding it to "existing" count
						continue;
					}

					if( Directory::Exists(dest_filename[ind]))
						num_dir_exist++;
					else
						num_dir_missing++;

					continue;
				}


				src_sizes[ind]=item->size;
				src_date[ind]=item->datetime;


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
				/*
				//printf("num_exist=%d  num_cat={%d,%d,%d}\n",num_exist,num_cat[0],num_cat[1],num_cat[2]);
				OverwriteConfirmation^ oc = gcnew OverwriteConfirmation(files_cat[0],files_cat[1],files_cat[2]);
				oc->copymode=copymode;
				//"A file with this name already exists";
				if (num_exist==1) oc->title_label->Text=lang::o_exist+"                                                                               ";
				else oc->title_label->Text=lang::o_exist_plural+"                                                                               ";;
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
						oc->checkBox1->Text = lang::o_delete_pvr;//"Delete the PVR copy";
					else
						oc->checkBox1->Text = lang::o_delete_pvr_plural;//"Delete the PVR copies";
				}
				if (num_cat[0]>1) oc->label1->Text = lang::o_correct_plural; else oc->label1->Text = lang::o_correct;//"File has correct size"; 

				//oc->files2->Text = files_cat[1];
				if (num_cat[1]==0)
				{
					oc->panel2->Visible = false;oc->files2->Visible=false;
				}
				if (num_cat[1]>1) oc->label2->Text =  lang::o_undersized_plural; else oc->label2->Text = lang::o_undersized ;///"Undersized file";

				//oc->files3->Text = files_cat[2];
				if (num_cat[2]==0)
				{
					oc->panel3->Visible = false;oc->files3->Visible=false;
				}
				if (num_cat[2]>1) oc->label3->Text = lang::o_oversized_plural; else oc->label3->Text =lang::o_oversized;//"This existing file is larger!";

				if (!this->no_prompt)
					if (::DialogResult::Cancel == oc->ShowDialog() ) goto aborted;

				int action1 = ( oc->overwrite1->Checked * OVERWRITE ) + oc->skip1->Checked * SKIP;
				int action2 = ( oc->overwrite2->Checked * OVERWRITE ) + oc->skip2->Checked * SKIP + oc->resume2->Checked*RESUME;
				int action3 = ( oc->overwrite3->Checked * OVERWRITE ) + oc->skip3->Checked * SKIP;

				if (transferoptions->overwrite_all) {action1=OVERWRITE; action2=OVERWRITE; action2=OVERWRITE;} 

				*/

				OverwriteConfirmation^ oc = gcnew OverwriteConfirmation( dest_filename, dest_size, dest_exists,
					src_sizes,  dest_date,  src_date, CopyDirection::PC_TO_PVR,  copymode, overwrite_category, overwrite_action);

				if (!this->no_prompt)
					if (::DialogResult::Cancel == oc->ShowDialog() ) goto aborted;

				array<OverwriteAction>^ actions_per_category = oc->overwrite_actions_per_category();
				action1_skipdelete = oc->checkBox1->Checked;


				for (int i=0; i<numitems; i++)
				{
					item=src_items[i];
					overwrite_action[i]=OVERWRITE;
					if (dest_exists[i] && !transferoptions->overwrite_all)
					{
						overwrite_action[i] = actions_per_category[overwrite_category[i]];
					}
					if (overwrite_action[i]==RESUME && dest_size[i] < this->min_resume_size) overwrite_action[i]=OVERWRITE; // (don't bother resuming tiny files).

					if (overwrite_action[i]==OVERWRITE) current_offsets[i]=0; else
						if (overwrite_action[i]==SKIP) {current_offsets[i]=item->size;num_skip++;} else
							if (overwrite_action[i]==RESUME) current_offsets[i]=dest_size[i];
				}
			}
			if ( (num_skip+num_dir_exist)==numitems && copymode == CopyMode::COPY) 
			{
				String ^s0 = ""; if (num_skip + num_dir_exist >0 ) s0 = (num_skip+num_dir_exist).ToString() + " item";
				if (num_skip+num_dir_exist > 1) s0 = " all "+s0+"s";else s0 = " the "+s0; 
				String ^s1 = ""; if (num_skip>0) s1=num_skip.ToString() + " file"; if (num_skip>1) s1=s1+"s";
				String ^s2 = ""; if (num_dir_exist>0) s2=num_dir_exist.ToString() + " folder"; if (num_dir_exist>1) s2=s2+"s";
				if (s2->Length > 0 && s1->Length > 0) s2 = " and "+s2;
				if (num_skip>0 || num_dir_exist>0) Console::WriteLine("Skipping"+s0+" ("+s1+s2+"). Nothing to do.");
				goto aborted;
			}


			int num_overwrite=0;
			int num_resume=0;
			for (int i=0; i<numitems; i++)
			{
				if (overwrite_action[i]==OVERWRITE && !src_items[i]->isdir) num_overwrite++;			
				if (overwrite_action[i]==RESUME) num_resume++;
			}
			String ^ p;
			String ^c0=", ";
			String ^ c="";
			p = num_files==1 ? "" : "s"; if (num_files>=0) printf("*\n* Found %d matching file%s on the PVR.  TO DO: ",num_files,p);
			p = num_skip==1 ? "" : "s"; if (num_skip>0) {printf(" Skip %d file%s",num_skip,p);c=c0;};
			String ^q="";
			p = num_resume==1 ? "" : "s"; if (num_resume>0) {printf("%s Resume %d file%s", c,num_resume,p);q=" whole";c=c0;};
			p = num_overwrite==1 ? "" : "s"; if (num_overwrite>0) {printf("%s Transfer %d%s file%s",c,num_overwrite,q,p);c=c0;}
			p = num_dir_missing==1 ? "" : "s"; if (num_dir_missing>0) printf("%s Create %d folder%s",c,num_dir_missing, p);
			printf(".\n*\n");

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

				alert->required_label->Text = lang::f_required+" " + HumanReadableSize(space_required);
				alert->available_label->Text = lang::f_available+" " + HumanReadableSize(freespaceArray[0]);
				if (!this->no_prompt)
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
			copydialog->numfiles = num_files;
			copydialog->current_index = 0;

			copydialog->turbo_mode = this->turbo_mode;
			copydialog->turbo_mode2 = this->turbo_mode2;


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
			copydialog->file_indices=file_indices;
			copydialog->filtered_dir_has_no_files=filtered_dir_has_no_files;

			for (int i=0, ind=0; i<numitems; i++) {if ( !(src_items[i]->isdir || overwrite_action[i]==SKIP) ) ind++; file_indices[i]=ind;};




			//String ^window_title_bit2 = "";
			if (file_indices[numitems-1]>1)
			{
				copydialog->normal_size();
				//window_title_bit2="s";
			}
			else
				copydialog->small_size();

			//copydialog->window_title=window_title_bit + " File"+window_title_bit2+" ... [PVR --> PC]"; 
			//copydialog->Text = copydialog->window_title;


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
			if (this->commandline->showgui)
				while(copydialog->loaded==false)
				{
					Thread::Sleep(100);
				}

				copydialog->copydirection=CopyDirection::PC_TO_PVR;
				copydialog->update_dialog_threadsafe();
				TransferOptions ^transferoptions = copydialog->transferoptions;
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
				array<bool>^             filtered_dir_has_no_files=copydialog->filtered_dir_has_no_files;
				array<bool>^             source_deleted     = gcnew array<bool>(numitems); for (int i=0; i<numitems; i++) source_deleted[i]=false;
				array<array<TopfieldItem^>^>^ topfield_items_by_folder = copydialog->topfield_items_by_folder;
				array<String^>^          failed_filenames   = gcnew array<String^>(0);
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
						if (transferoptions->skip_directories_with_no_files && filtered_dir_has_no_files[i]) continue;
						titem = this->topfieldFileExists(topfield_items_by_folder,dest_filename[i]);
						int r=0;
						if (titem==nullptr)
						{
							r = this->newTopfieldFolder(dest_filename[i]);
						}
						// Abort if required destination folder is already a file name
						if (titem!=nullptr && !titem->isdir)
						{
							//copydialog->file_error="The folder "+dest_filename[i]+" could not be created because there exists a file of the same name.";
							copydialog->file_error = String::Format(lang::c_folder_file_clash, dest_filename[i]);
							goto finish_transfer;
						}

						// If there was some other error creating the folder...
						// ... perhaps it was really successful. Try loading the folder contents to find out.

						if (r<0)
						{
							this->absorb_late_packets(2,100);
							array<TopfieldItem^> ^check = this->loadTopfieldDirArrayOrNull(dest_filename[i]);
							if (verbose) printf("newTopfieldFolder returned %d. Double checking %s\n",r,dest_filename[i]);

							if (check==nullptr)
							{
								if (verbose) printf("Double-check failed.\n");

								//copydialog->file_error="The folder "+dest_filename[i]+" could not be created. Aborting transfer.";
								copydialog->file_error=String::Format(lang::c_folder_error , dest_filename[i]);
								goto finish_transfer;
							}
						}


						if (copydialog->copymode==CopyMode::MOVE && !transferoptions->never_delete_directories)
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



						copydialog->success(i);
						continue;
					}


					FileStream^ src_file;
					bool has_restarted = false;
					bool recording_in_progress_needs_checking = false;

					copydialog->usb_error=false;
					copydialog->file_error="";

					topfield_file_offset=0;
					if (0)
					{
restart_copy_to_pvr:     
						copydialog->usb_error=false;
						has_restarted=true;
						recording_in_progress_needs_checking = false;
						topfield_file_offset=0;
						copydialog->reset_rate();
						TopfieldItem^ reloaded = this->reloadTopfieldItem(full_dest_filename);
						if (reloaded==nullptr)
						{
							//printf("reloaded == nullptr\n");
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
									//printf("reloaded->size = %lld  probable_minimum_received_offset=%lld\n",reloaded->size,probable_minimum_received_offset);
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
						//printf("%s   [Skipping]\n",item->full_filename);

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
						copydialog->success(i);

						continue;
					}  

					if (this_overwrite_action==RESUME && dest_size[i]>=src_sizes[i]) {
						//printf("Not resuming %s\n",item->full_filename);
						if (dest_size[i]==src_sizes[i]) topfield_file_offset = dest_size[i];
						goto check_delete;
					} // TODO: Handle this case better

					copydialog->freespace_check_needed = false;

					try {

						src_file = File::Open(full_src_filename,System::IO::FileMode::Open, System::IO::FileAccess::Read,System::IO::FileShare::Read);
					}
					catch(...)
					{


						int nff = failed_filenames->Length;

						Array::Resize(failed_filenames, nff+1);
						failed_filenames[nff] = full_src_filename;

						//copydialog->file_error="The file "+full_src_filename+" could not be opened for reading. Aborting transfer.";
						//printf("%s\n",copydialog->file_error);
						continue;

						//goto finish_transfer;
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
							//printf("dest_size[i]=%lld    existing_bytes_count PVR=%d PC=%d   overlap=%d\n",dest_size[i],existing_bytes_count_PVR, existing_bytes_count_PC, overlap);  
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
							//printf("Overlap success.\n");
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

					//printf("topfield_file_offset=%lld   = %f MB\n",topfield_file_offset,((double)topfield_file_offset)/1024.0/1024.0);
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


					int file_ind = copydialog->file_indices[i];
					int max_file_ind = copydialog->file_indices[numitems-1];
					if (topfield_file_offset==0)
						printf("  %2d / %2d : %s\n",file_ind,max_file_ind,item->full_filename);
					else
						printf("  %2d / %2d : %s       [Resuming]\n",file_ind,max_file_ind,item->full_filename);


					int update=0;
					while(1)
					{
						r = get_tf_packet(this->fd, &reply);

						if (r<=0)
						{
							if (verbose) printf("In main loop of transfer_to_PVR, get_tf_packet returned %d\n",r);
							copydialog->usb_error=true;
							goto out;
						}

						update = update + 1;
						switch (get_u32(&reply.cmd))
						{
						case SUCCESS:
							if (verbose) printf("SUCCESS\n");
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
									if (verbose) printf(" DATA_HDD_FILE_START  \n");
									r = send_tf_packet(this->fd, &packet);
									if(r < 0)
									{
										if (verbose) printf( "ERROR: Incomplete send in transfer_to_PVR.\n");
										copydialog->usb_error=true;
										goto out;
									}
									state = DATA;
									break;
								}

							case DATA:
								{
									if (verbose) printf("DATA\n");
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
											//printf("\n -- SEEK CORRECTION ---\n");
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
										//printf("CANCELLING\n");
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

									if (update % 8 == 0)
									{
										// If turbo mode currently disabled due to recording, occasionally re-check current recording
										if (*this->turbo_mode 
											&& (*this->turbo_mode != *this->turbo_mode2) 
											&& Math::Abs( (DateTime::Now - this->recording_in_progress_last_checked).TotalSeconds) > this->check_recording_interval
											&& !copydialog->cancelled
											&& copydialog->current_offsets[i] > 0
											&& copydialog->current_offsets[i] < copydialog->filesizes[i]
										)
										{
											state=END;
											recording_in_progress_needs_checking=true;

										}
									}



									/* Detect EOF and transition to END */
									if((w < 0) || (topfield_file_offset >= fileSize))
									{
										//printf("\nEOF conditition. w=%d  bytes_sent=%f  fileSize=%f topfield_file_offset=%f\n",w,(double) bytes_sent,(double) fileSize,(double)topfield_file_offset);
										state = END;
									}

									if(w > 0 || true)
									{
										trace(3,
											fprintf(stdout, "%s: DATA_HDD_FILE_DATA\n",
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


									if ( (update%2)==0)
									{
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
											//printf("\n -- SEEK CORRECTION ---\n");
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
								//trace(3,
								//	fprintf(stdout, "%s: DATA_HDD_FILE_END\n",
								//	__FUNCTION__));
								r = send_tf_packet(fd, &packet);
								if(r < 0)
								{
									printf("ERROR: Incomplete send in transfer_to_PVR.\n");
									copydialog->usb_error=true;
									goto out;
								}
								state = FINISHED;
								copydialog->success(i);
								//if (!was_cancelled) item->Selected = false;
								break;

							case FINISHED:
								result = 0;
								goto out;
								break;
							}    //(end switch state)
							break;

						case FAIL:
							fprintf(stdout, "ERROR: Device reports %s in transfer_to_PVR\n",
								decode_error(&reply));
							copydialog->usb_error=true;
							state=END; copydialog->usb_error=true;break;  // This line an experiment, 26/1/11.
							goto out;
							break;

						default:
							fprintf(stdout, "ERROR: Unhandled packet (copy PVR -> PC)\n");
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

					if (copydialog->cancelled==true)  {
						//printf("Cancelled.\n");
						goto finish_transfer;}

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

					// If turbo mode currently disabled due to recording, occasionally re-check current recording
					if (*this->turbo_mode 
						&& (*this->turbo_mode != *this->turbo_mode2) 
						&& Math::Abs( (DateTime::Now - this->recording_in_progress_last_checked).TotalSeconds) > this->check_recording_interval / 4.0
						&& !copydialog->cancelled
						)
					{
						if (recording_in_progress_needs_checking)
							this->absorb_late_packets(2,100);

						this->set_turbo_mode(*this->turbo_mode);
						copydialog->reset_rate();

						if (recording_in_progress_needs_checking)
							goto restart_copy_to_pvr;

					}


					if (!copydialog->cancelled) {
						if (topfield_file_offset == copydialog->filesizes[i])
							copydialog->maximum_successful_index=i;
						else
							goto restart_copy_to_pvr;   // This path probably shouldn't ever happen
					};
					if (copydialog->cancelled)
					{
						break;
					}

check_delete:
					//Console::WriteLine(item->full_filename);
					//printf("  topfield_file_offset =%ld   src_sizes=%ld  \n",topfield_file_offset, src_sizes[i]);

					if (copydialog->copymode==CopyMode::MOVE  && topfield_file_offset == src_sizes[i])
					{
						try{
							if (overwrite_action[i]!=SKIP || copydialog->action1_skipdelete)
							{
								//Console::WriteLine(item->full_filename);
								File::Delete(item->full_filename);
								source_deleted[i]=true;
							}
						}
						catch(...)
						{
							printf("Didn't delete %s at source.",item->full_filename);

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

				int L = failed_filenames->Length;
				if (L > 0)
				{
					if (copydialog->file_error->Length > 0) copydialog->file_error  += "\n";
					//copydialog->file_error+="The following file"; if (L>1) copydialog->file_error+="s";
					//copydialog->file_error+=" could not be accessed on the PC:\n";

					if (L>1)
						copydialog->file_error += lang::c_no_access_pc_plural;
					else
						copydialog->file_error += lang::c_no_access_pc;

					copydialog->file_error += "\n";

					for (int i=0; i<L; i++)
					{				
						copydialog->file_error+=failed_filenames[i]+"\n";
						if (i>4)
						{
							copydialog->file_error+=" ... ("+ (L-i-1).ToString() +" more)\n";
							break;
						}
					}
				}
				if (verbose) printf("%s\n", copydialog->file_error);


		}


		System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			this->transfer_selection_to_PVR(gcnew TransferOptions());
		}
		////////////////////////////////////////////////////////////////////////////////////
		void transfer_selection_to_PVR(TransferOptions ^transferoptions)
		{

			// Copy files from Computer to Topfield


			if (this->transfer_in_progress) return;
			if (this->fd==NULL) 
			{
				/*
				#ifdef _DEBUG
				ListView^ listview = this->listView2;

				ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;
				CopyDialog^ copydialog = gcnew CopyDialog();
				copydialog->settings = this->settings;
				copydialog->cancelled=false;
				copydialog->parent_win = this;
				copydialog->parent_form = this;
				//copydialog->showCopyDialog();


				this->ShowCopyDialog(copydialog);
				if (selected->Count>1) copydialog->normal_size(); else copydialog->small_size();
				#endif
				*/
				return;
			}

			const int max_folders = 1000;
			CopyMode copymode = transferoptions->copymode;

			if (copymode==CopyMode::UNDEFINED)
				copymode = this->getCopyMode();

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
			copydialog->commandline = this->commandline;
			copydialog->transferoptions = transferoptions;
			copydialog->turbo_request = (this->settings["TurboMode"]=="on");
			//copydialog->showCopyDialog();

			//String ^window_title_bit;
			if (copymode==CopyMode::COPY)
				copydialog->window_title1=lang::c_title1_copy;//"Copying File";
			else
				copydialog->window_title1=lang::c_title1_move;//"Moving File";

			copydialog->window_title2=lang::c_title2_to_pvr;//"[PC --> PVR]";
			copydialog->Text=copydialog->window_title1 + " ... "+copydialog->window_title2; 
			//copydialog->Text = copydialog->window_title;

			copydialog->tiny_size();
			copydialog->label3->Text=lang::c_finding;//"Finding files...";
			Console::WriteLine(lang::c_finding);
			this->ShowCopyDialog(copydialog);

			copydialog->Update();



			array<ComputerItem^>^ items = gcnew array<ComputerItem^>(selected->Count);
			selected->CopyTo(items,0);
			for (int i=0; i<items->Length; i++) if (items[i]->isdrive) {goto abort;};  // Can't copy whole drives at a time

			array<array<ComputerItem^>^>^ items_by_folder = gcnew array<array<ComputerItem^>^>(max_folders);

			Regex ^pattern_regex;
			array<Regex^>^ exclude_regexes = gcnew array<Regex^>(transferoptions->exclude_patterns->Length);
			for (int ii = 0; ii<exclude_regexes->Length; ii++) exclude_regexes[ii]=this->WildcardToRegex(transferoptions->exclude_patterns[ii]);
			bool use_pattern = false;
			if (transferoptions->pattern->Length > 0)
			{
				use_pattern=true;
				pattern_regex = this->WildcardToRegex(transferoptions->pattern);
			}

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

						/////// Filter items if necessary
						int i2=0;
						for (int i1=0; i1<items->Length; i1++)
						{
							ComputerItem^ it = items[i1];

							if (!it->isdir)
							{
								if (use_pattern && !pattern_regex->IsMatch(it->filename)) {continue;}

								bool exc = false;
								for each (Regex^ re in exclude_regexes)
									if (re->IsMatch(it->filename)) {exc = true;break;};
								if (exc) continue;

							}
							if (i1 != i2) items[i2]=items[i1]; i2++;

						}
						if (i2 != items->Length)
							Array::Resize(items,i2);
						//// (end filter code)


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
			array<DateTime>^         src_date = gcnew array<DateTime>(numitems);
			array<long long int>^    dest_size = gcnew array<long long int>(numitems);
			array<long long int>^    src_sizes = gcnew array<long long int>(numitems);
			array<String^>^          dest_filename= gcnew array<String^>(numitems);
			array<int>^              overwrite_category=gcnew array<int>(numitems);
			array<int>^              overwrite_action = gcnew array<int>(numitems);
			array<long long int>^    current_offsets = gcnew array<long long int>(numitems);
			array<int>^              file_indices = gcnew array<int>(numitems);
			array<bool>^             filtered_dir_has_no_files = gcnew array<bool>(numitems);

			for (int i=0; i<numitems; i++) overwrite_action[i]=OVERWRITE;

			for (int i=0; i<numitems; i++)
			{
				ComputerItem^ item = src_items[i];

				if (!item->isdir) continue;
				String^ x = item->full_filename;
				if (!x->EndsWith("\\")) x = x + "\\";

				bool file_found = false;
				for (int j=0; j<numitems; j++)
				{
					if (j==i || src_items[j]->isdir) continue;
					String^ y = src_items[j]->full_filename;
					if (y->StartsWith(x))
					{
						file_found=true;
						break;
					}
				}
				filtered_dir_has_no_files[i] = !file_found;
			}

			TopfieldItem^ titem;				 
			array<int>^ num_cat={0,0,0}; //numbers of existing files (divided by category of destination file: 0=correct size,  1=undersized, 2=oversized).
			int num_exist=0;
			int num_dir_exist=0;
			int num_dir_missing=0;
			array<String^>^ files_cat = {"","",""};
			for (ind=0; ind<numitems; ind++)
			{
				item = src_items[ind];
				if (item->recursion_offset == "")
					dest_filename[ind] = Antares::combineTopfieldPath(this->topfieldCurrentDirectory, item->safe_filename);
				else
				{

					//dest_filename[ind] = Path::Combine(this->topfieldCurrentDirectory, Antares::safeString(item->recursion_offset));
					//dest_filename[ind] = Path::Combine(dest_filename[ind], item->safe_filename);
					dest_filename[ind] = Antares::combineTopfieldPath(this->topfieldCurrentDirectory, Antares::safeString(item->recursion_offset));
					dest_filename[ind] = Antares::combineTopfieldPath(dest_filename[ind], item->safe_filename);
				}
				if (item->isdir) {

					if (transferoptions->skip_directories_with_no_files && filtered_dir_has_no_files[ind]) 
					{
						num_dir_exist++;   // A bit hacky adding it to "existing" count
						continue;
					}


					if (nullptr != this->topfieldFileExists(topfield_items_by_folder, dest_filename[ind]))
						num_dir_exist++;
					else
						num_dir_missing++;
					continue;
				}   
				else
					num_files++;

				src_sizes[ind]=item->size;
				src_date[ind]=item->datetime;

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
					dest_date[ind] = titem->datetime;
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
				/*
				//printf("num_exist=%d  num_cat={%d,%d,%d}\n",num_exist,num_cat[0],num_cat[1],num_cat[2]);
				OverwriteConfirmation^ oc = gcnew OverwriteConfirmation(files_cat[0],files_cat[1], files_cat[2]);
				oc->copymode=copymode;
				if (num_exist==1) oc->title_label->Text=lang::o_exist+"                                                             ";
				else oc->title_label->Text = lang::o_exist_plural+"                                                                        ";

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
						oc->Text = lang::o_delete_pc;//"Delete the PC copy";
					else
						oc->Text =  lang::o_delete_pc_plural;//"Delete the PC copies";
				}
				if (num_cat[0]>1) 
					oc->label1->Text = lang::o_correct_plural;//"Files have correct size"; 
				else oc->label1->Text = lang::o_correct;//"File has correct size"; 

				if (num_cat[1]==0)
				{
					oc->panel2->Visible = false;oc->files2->Visible=false;
				}
				if (num_cat[1]>1) oc->label2->Text = lang::o_undersized_plural;//"Undersized files";
				else oc->label2->Text = lang::o_undersized;//"Undersized file";

				if (num_cat[2]==0)
				{
					oc->panel3->Visible = false;oc->files3->Visible=false;
				}
				if (num_cat[2]>1) oc->label3->Text = lang::o_oversized_plural;//"These exising files are larger!!"; 
				else oc->label3->Text = lang::o_oversized;//"This existing file is larger!!";

				*/


				OverwriteConfirmation^ oc = gcnew OverwriteConfirmation( dest_filename, dest_size, dest_exists,
					src_sizes,  dest_date,  src_date, CopyDirection::PC_TO_PVR,  copymode, overwrite_category, overwrite_action);

				if (!this->no_prompt)
					if (::DialogResult::Cancel == oc->ShowDialog() ) goto abort;

				array<OverwriteAction>^ actions_per_category = oc->overwrite_actions_per_category();
				action1_skipdelete = oc->checkBox1->Checked;

				for (int i=0; i<numitems; i++)
				{
					item=src_items[i];
					overwrite_action[i]=OVERWRITE;
					if (dest_exists[i] && !transferoptions->overwrite_all)
					{
						overwrite_action[i] = actions_per_category[overwrite_category[i]];
					}
					if (overwrite_action[i]==RESUME && dest_size[i] < this->min_resume_size) overwrite_action[i]=OVERWRITE; // (don't bother resuming tiny files).

					if (overwrite_action[i]==OVERWRITE) current_offsets[i]=0; else
						if (overwrite_action[i]==SKIP) {current_offsets[i]=item->size;num_skip++;} else
							if (overwrite_action[i]==RESUME) current_offsets[i]=dest_size[i];
				}

				/*
				int action1 = ( oc->overwrite1->Checked * OVERWRITE ) + oc->skip1->Checked * SKIP;
				int action2 = ( oc->overwrite2->Checked * OVERWRITE ) + oc->skip2->Checked * SKIP + oc->resume2->Checked*RESUME;
				int action3 = ( oc->overwrite3->Checked * OVERWRITE ) + oc->skip3->Checked * SKIP;

				if (transferoptions->overwrite_all) {action1=OVERWRITE; action2=OVERWRITE; action2=OVERWRITE;} 

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
					if (overwrite_action[i]==RESUME && dest_size[i] < this->min_resume_size) overwrite_action[i]=OVERWRITE; // (don't bother resuming tiny files).

					if (overwrite_action[i]==OVERWRITE) current_offsets[i]=0; else
						if (overwrite_action[i]==SKIP) {current_offsets[i]=item->size;num_skip++;} else
							if (overwrite_action[i]==RESUME) current_offsets[i]=dest_size[i];
				}
				*/
			}
			
			if ( (num_dir_exist+num_skip)==numitems && copymode == CopyMode::COPY) 
			{
				String ^s0 = ""; if (num_skip + num_dir_exist >0 ) s0 = (num_skip+num_dir_exist).ToString() + " item";
				if (num_skip+num_dir_exist > 1) s0 = " all "+s0+"s";else s0 = " the "+s0; 
				String ^s1 = ""; if (num_skip>0) s1=num_skip.ToString() + " file"; if (num_skip>1) s1=s1+"s";
				String ^s2 = ""; if (num_dir_exist>0) s2=num_dir_exist.ToString() + " folder"; if (num_dir_exist>1) s2=s2+"s";
				if (s2->Length > 0 && s1->Length > 0) s2 = " and "+s2;
				if (num_skip>0 || num_dir_exist>0) Console::WriteLine("Skipping"+s0+" ("+s1+s2+"). Nothing to do.");
				goto abort;
			}

			int num_overwrite=0;
			int num_resume=0;
			for (int i=0; i<numitems; i++)
			{
				if (overwrite_action[i]==OVERWRITE && !src_items[i]->isdir) num_overwrite++;			
				if (overwrite_action[i]==RESUME) num_resume++;
			}
			String ^ p;
			String ^c0=", ";
			String ^c="";
			p = num_files==1 ? "" : "s"; if (num_files>=0) printf("*\n* Found %d matching file%s on the PC.  TO DO: ",num_files,p);
			p = num_skip==1 ? "" : "s"; if (num_skip>0) {printf(" Skip %d file%s",num_skip,p);c=c0;};
			String ^q="";
			p = num_resume==1 ? "" : "s"; if (num_resume>0) {printf("%s Resume %d file%s", c,num_resume,p);q=" whole";};
			p = num_overwrite==1 ? "" : "s"; if (num_overwrite>0) {printf("%s Transfer %d%s file%s",c,num_overwrite,q,p);c=c0;};
			p = num_dir_missing==1 ? "" : "s"; if (num_dir_missing>0) printf("%s Create %d folder%s",c,num_dir_missing, p);
			printf(".\n*\n");


			long long space_required=0;
			for (int i=0; i<numitems; i++)
			{
				if (overwrite_action[i] != SKIP)     //TODO: modify if we every have an "auto-rename" option.
					space_required += (src_sizes[i] - dest_size[i]);
			}


			long long int freespace;
			long long int margin;

			{
				TopfieldFreeSpace tfs = this->getTopfieldFreeSpace();

				freespace = (long long int) tfs.freek * 1024LL;
				margin = 1024*1024*3; if (freespace>margin) freespace-=margin;  // You can never seem to use the last couple of MB on topfield
				if (tfs.valid)
				{
					if (space_required > freespace)
					{

						LowSpaceAlert^ alert = gcnew LowSpaceAlert();
						alert->required_label->Text = lang::f_required+" " + HumanReadableSize(space_required);
						alert->available_label->Text =lang::f_available+" " + HumanReadableSize(freespace);
						if (freespace < this->topfield_minimum_free_megs*1024*1024)
						{
							alert->label4->Visible = false;
							alert->button1->Visible = false;
						}
						if (!this->no_prompt)
							if (::DialogResult::Cancel ==  alert->ShowDialog())
							{
								goto abort;
							}
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
			copydialog->file_indices=file_indices;
			copydialog->filtered_dir_has_no_files=filtered_dir_has_no_files;


			for (int i=0, ind=0; i<numitems; i++) {if ( !(src_items[i]->isdir || overwrite_action[i]==SKIP) ) ind++; file_indices[i]=ind;};


			copydialog->turbo_mode = this->turbo_mode;
			copydialog->turbo_mode2 = this->turbo_mode2;
			copydialog->parent_checkbox = this->checkBox1;
			copydialog->copymode=copymode;
			copydialog->action1_skipdelete = action1_skipdelete;
			copydialog->turbo_request = (this->settings["TurboMode"]=="on");



			//String ^window_title_bit2 = "";
			if (file_indices[numitems-1]>1)
			{
				copydialog->normal_size();
				//window_title_bit2="s";
			}
			else
				copydialog->small_size();

			//copydialog->window_title=window_title_bit + " File"+window_title_bit2+" ... [PC --> PVR]"; 
			//copydialog->Text = copydialog->window_title;



			this->transfer_in_progress=true;
			this->TransferBegan();
			Thread^ thread = gcnew Thread(gcnew ParameterizedThreadStart(this,&Form1::transfer_to_PVR));
			thread->Name = "transfer_to_PVR";
			copydialog->thread = thread;





			thread->Start(copydialog);

			Monitor::Exit(this->locker);




			return;
abort:  // If the transfer was cancelled before it began

			Monitor::Exit(this->locker);
			this->TransferEnded();

		}

		System::Void listView1_ItemActivate(System::Object^  sender, System::EventArgs^  e) {

			if (this->transfer_in_progress) return;

			ListView^ listview = (ListView^) sender;
			//Console::WriteLine("Activated (1)");
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
			array<int> ^column_inds;


			String^ type;
			if (listview==this->listView2) {
				sortcolumn=&this->listView2SortColumn;
				type = "PC";
				column_inds = FileItem::computer_column_inds;
			}
			else
			{
				sortcolumn = &this->listView1SortColumn; 
				type = "PVR";
				column_inds = FileItem::topfield_column_inds;
			}

			int col;
			col = column_inds[e->Column]; 
			if (col == *sortcolumn)
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

			printf("--- col = %d   sortcolumn=%d \n",col,*sortcolumn);
			*sortcolumn = col;
			settings->changeSetting(type+"_SortColumn", col.ToString());

			listview->ListViewItemSorter = gcnew ListViewItemComparer(col,listview->Sorting);

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

			this->delete_PVR_selection();
		}
		System::Void delete_PVR_selection(void)
		{
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
				conf_str = item->clean_filename(item->filename);
				if (item->isdir) conf_str = conf_str + "\\          "+lang::d_folder;//[Folder -- Contents will be deleted!!!]";
				confirmation->listBox1->Items->Add(conf_str);

			}
			if (numfiles+numdirs==0) return;
			/*
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
			*/
			if (numfiles+numdirs > 1) conf_str = lang::d_delete_plural; else conf_str = lang::d_delete;

			confirmation->label1->Text = conf_str;
			Console::WriteLine(confirmation->Size);

			confirmation->Height = min( confirmation->Height +(numfiles+numdirs-1)*confirmation->listBox1->ItemHeight,700);
			Console::WriteLine(confirmation->Size);
			if (!this->no_prompt)
			{
				result = confirmation->ShowDialog();
				if (result!=Windows::Forms::DialogResult::Yes) return;
			}

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
						String ^msg = "";
						try {
							if (item->isdir)
							{
								this->computer_new_folder_time = 0;
								Directory::Move(old_full_filename, new_full_filename);
							}
							else
							{
								File::Move(old_full_filename, new_full_filename); 
							}
						}

						catch(Exception ^ex)
						{
							success=false;

							msg ="\r\n"+ ex->Message;

						}

						if (!success)
						{
							e->CancelEdit = true;

							MessageBox::Show(this,lang::m_rename_error + msg,lang::st_error,MessageBoxButtons::OK);
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




			ListView^ listview = safe_cast<ListView^>(sender);

			if (this->transfer_in_progress && listview == this->listView1) return;

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

			if (!e->Alt && !e->Shift && e->Control && e->KeyCode==Keys::E)
				if (listview == this->listView2)
					this->show_in_explorer();

			if (e->KeyCode == Keys::F5)          // F5 (Refresh)
			{
				if (listview == this->listView1)
					this->refreshTopfield();
				else
					this->refreshComputer();
			}

			if (e->KeyCode == Keys::Delete)
			{
				//Console::WriteLine("Delete pressed!");
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
			if (verbose) printf("newTopfieldFolder(%s)\n",path);
			Monitor::Enter(this->locker);
			try{
				r = do_hdd_mkdir(this->fd,path);}
			catch(Exception^ e)
			{
				if (verbose) printf("Exception in newTopfieldFolder, calling do_hdd_mkdir. \n%s\n",e->Message); 
			};
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

				foldername = lang::m_new_folder_name + " " + i.ToString("D2");





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
				if (r!=0) this->toolStripStatusLabel1->Text=lang::m_folder_error;//"Error creating new folder.";
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
					//txt = "  Selected " + numfiles.ToString() +" files on PC  ( "+Antares::HumanReadableSize(totalsize)+" )";

					if (numdirs>0)
					{
						if (numdirs==1)
							txt = String::Format(lang::st_pc_selected_with_folder, numfiles, Antares::HumanReadableSize(totalsize)  );
						else
							txt = String::Format(lang::st_pc_selected_with_folders, numfiles, Antares::HumanReadableSize(totalsize) , numdirs);
						//txt = txt + "     and   "+numdirs.ToString();
						//if (numdirs>1) txt=txt+" folders"; else txt=txt+" folder";
						//txt=txt+" (size unknown) ";

					}

					else
						txt = String::Format(lang::st_pc_selected, numfiles, Antares::HumanReadableSize(totalsize)  );

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

					if (numdirs>0)
					{
						if (numdirs==1)
							txt = String::Format(lang::st_pvr_selected_with_folder, numfiles, Antares::HumanReadableSize(totalsize)  );
						else
							txt = String::Format(lang::st_pvr_selected_with_folders, numfiles, Antares::HumanReadableSize(totalsize) , numdirs);
					}
					else
						txt = String::Format(lang::st_pvr_selected, numfiles, Antares::HumanReadableSize(totalsize)  );

					/*
					txt = "  Selected " + numfiles.ToString() +" files on PVR ( "+Antares::HumanReadableSize(totalsize)+" )";

					if (numdirs>0)
					{
					txt = txt + "     and   "+numdirs.ToString();
					if (numdirs>1) txt=txt+" folders"; else txt=txt+" folder";
					txt=txt+" (size unknown) ";
					}
					*/

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
					lang::m_paste_error,    //"Cannot paste to this location, since it is inside a folder being moved.", 
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
			item->title = gcnew String(ri->EventEventName);
			item->description = gcnew String(ri->EventEventDescription);

			item->svcid = ri->SIServiceID;
			item->reclen = ri->HeaderDuration;
			item->proglen = ri->EventDurationMin + 60*ri->EventDurationHour;
			item->prog_start_time = Time_T2DateTime(tfdt_to_time( & ri->EventStartTime));


			String^ ext = gcnew String(ri->ExtEventText);
			if (item->description->Length >0 && ext->Length >0 )
			{
				item->description = item->description + "  --- ";
			}
			item->description = item->description + ext;

			if (!this->InvokeRequired)
			{
				item->populate_subitems();
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
				file = File::Open(item->full_filename,System::IO::FileMode::Open, System::IO::FileAccess::Read,System::IO::FileShare::ReadWrite);
			}
			catch(...)
			{
				//printf("Error opening file for reading rec header info.\n");

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
			if (verbose) printf("topfieldLoadInfo, %s.\n",item->full_filename);
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

					ProgInfo^ pi = gcnew ProgInfo(&ri,lang::p_wintitle+", "+item->clean_filename(item->full_filename)  );

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

			if (verbose) printf("read_topfield_file_snippet, %s : %lld\n",filename, offset);

			return this->read_topfield_file(filename, offset, 32768);

		}

		array<Byte>^ read_topfield_file(String^ filename, long long offset, long long max_bytes) {

			// Read a section of a file on the Topfield, starting at specified offset, and stopping
			// after max_bytes number of bytes (rounded up to the nearest packet) have been read.
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



			long long tot_num_bytes=0;

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
						fprintf(stdout,
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
							fprintf(stdout,
								"ERROR: Short packet %d instead of %d\n", r,
								get_u16(&reply.length));
							this->connection_error_occurred();

						}


						Array::Resize(out_array,(int) ( tot_num_bytes+dataLen)  );
						Marshal::Copy( IntPtr( (void*)  &reply.data[8] ) , out_array, (int) tot_num_bytes  ,(int) dataLen);

						tot_num_bytes += dataLen;
						if (tot_num_bytes > max_bytes)

						{
							send_cancel(fd);
							state = ABORT;
						}
						else
							send_success(fd);


					}
					else
					{
						fprintf(stdout,
							"ERROR: Unexpected DATA_HDD_FILE_DATA packet in state %d\n",
							state);
						this->connection_error_occurred();
						send_cancel(fd);
						state = ABORT;

					}
					break;

				case DATA_HDD_FILE_END:
					send_success(fd);

					//printf("DATA_HDD_FILE_END\n");

					state=ABORT;

					break;

				case FAIL:
					if (verbose) printf( "ERROR: Device reports %s in read_topfield_file_snippet\n",
						decode_error(&reply));
					send_cancel(fd);
					this->connection_error_occurred();
					state = ABORT;

					break;

				case SUCCESS:
					//printf("SUCCESS\n");

					break;

				default:
					fprintf(stdout, "ERROR: Unhandled packet (cmd 0x%x)\n",
						get_u32(&reply.cmd));
					this->connection_error_occurred();
				}




				if (state==ABORT) break;
			}

			this->absorb_late_packets(2,200);
			return out_array;
		}





		System::Void Form1_ResizeEnd(System::Object^  sender, System::EventArgs^  e) {

			this->save_location();


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
			this->delete_PC_selection();
		}
		void delete_PC_selection (void)
		{
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
				if (item->isdir) conf_str = conf_str + "\\          "+lang::d_folder;
				confirmation->listBox1->Items->Add(conf_str);

			}
			if (numfiles+numdirs==0) return;

			/*
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
			*/
			if (numfiles+numdirs > 1) conf_str = lang::d_delete_plural; else conf_str = lang::d_delete;

			confirmation->label1->Text = conf_str;
			Console::WriteLine(confirmation->Size);

			confirmation->Height = min( confirmation->Height +(numfiles+numdirs-1)*confirmation->listBox1->ItemHeight,700);
			Console::WriteLine(confirmation->Size);
			if (!this->no_prompt)
			{
				result = confirmation->ShowDialog();
				if (result!=Windows::Forms::DialogResult::Yes) return;
			}

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
				MessageBox::Show(this,lang::d_error,lang::st_error,MessageBoxButtons::OK);
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

				foldername = lang::m_new_folder_name;
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

			this->computer_new_folder_time = time(NULL);


			this->loadComputerDir(foldername);
			if (!success)
				MessageBox::Show(this,lang::m_folder_error,lang::st_error,MessageBoxButtons::OK);

		}

		System::Void Form1_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
			if (this->transfer_in_progress)
			{
				CopyDialog^ copydialog = this->current_copydialog;
				if ( (!this->firmware_transfer_in_progress) &&  (copydialog != nullptr))
				{
					copydialog->cancelled = true;
					System::Threading::Thread^ thr = copydialog->thread;

					if (thr != nullptr)
						thr->Join();
					//this->TransferEnded();
				}

				FirmwareInstaller^ firmware_dialog = this->current_firmware_installer;

				if (this->firmware_transfer_in_progress &&  (firmware_dialog != nullptr)) 
				{
					firmware_dialog->cancelled = true;
					System::Threading::Thread^ thr = firmware_dialog->thread ;
					if (thr!= nullptr)
					{
						thr->Join();
					}
				}

			}
			//Application::Exit();
		}

		void apply_columns_visible(void)
		{
			int ind;
			this->clist->Columns->Clear();
			ind=0;
			for (int j=0; j<FileItem::num_computer_columns; j++)
			{
				String ^str = "PC_Column"+j.ToString()+"Visible";
				bool vis = true;
				try {
					vis = this->settings[str]=="1";
				} catch(...){};
				FileItem::computer_column_visible[j]=vis;
				if (vis)
				{
					this->clist->Columns->Add(this->computerHeaders[j]);
					FileItem::computer_column_inds[ind]=j;
					ind++;
				}
			}

			this->tlist->Columns->Clear();
			ind=0;
			for (int j=0; j<FileItem::num_topfield_columns; j++)
			{
				String ^str = "PVR_Column"+j.ToString()+"Visible";
				bool vis = true;
				try {
					vis = this->settings[str]=="1";
				} catch(...){};
				FileItem::topfield_column_visible[j]=vis;
				if(vis)
				{
					this->tlist->Columns->Add(this->topfieldHeaders[j]);
					FileItem::topfield_column_inds[ind]=j;
					ind++;
				}

			}

			this->Arrange2();



		}


		System::Void toolStripButton13_Click(System::Object^  sender, System::EventArgs^  e) {
			if (this->transfer_in_progress) return;

			SettingsDialog^ sd = gcnew SettingsDialog(this->settings);
			sd->ShowDialog();
			this->apply_columns_visible();
			this->apply_language_setting();

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
			if (this->commandline->showgui) this->clist->Focus();
		}

		// An item in the topfield path history was selected
		System::Void textBox2_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
			ComboBox^ cb = this->textBox2;
			this->setTopfieldDir(safe_cast<String^>(cb->SelectedItem));
			this->loadTopfieldDir();

			this->add_path_to_history(this->textBox2, this->topfieldCurrentDirectory);
			if (this->commandline->showgui) this->tlist->Focus();
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



	private: System::Void Form1_Move(System::Object^  sender, System::EventArgs^  e) {
				 //this->save_location();
			 }

	private: System::Void listView_ColumnWidthChanging(System::Object^  sender, System::Windows::Forms::ColumnWidthChangingEventArgs^  e) {


				 ListView ^ listview = safe_cast<ListView^>(sender);
				 String ^type;
				 array<int> ^ind_array;
				 array<ColumnHeader^>^ cols;
				 int nc;
				 if (listview == this->clist)
				 {
					 type="PC";
					 ind_array=FileItem::computer_column_inds;
					 cols = this->computerHeaders;
					 nc = FileItem::num_computer_columns;
				 }
				 else
				 {
					 type="PVR";
					 ind_array=FileItem::topfield_column_inds;
					 cols = this->topfieldHeaders;
					 nc = FileItem::num_topfield_columns;
				 }


				 int col = -1;
				 try{ col=ind_array[e->ColumnIndex];}catch(...){};
				 if (col==-1) return;

				 for (int j=0; j<nc; j++)
				 {
					 // printf("Resizing col %d. Newwidth=%d\n",col,e->NewWidth);
					 String^ str = type+"_Column"+j.ToString()+"Width";
					 String^ str2 = type+"_Column"+j.ToString()+"Visible";
					 if (this->settings[str2]=="0") continue;
					 int w = (j==col) ? e->NewWidth : cols[j]->Width; 
					 this->settings->changeSetting(str,w.ToString());

				 }
				 String ^str3 = type+"_ColumnScale";
				 int cw1 = listview->ClientSize.Width;
				 int cw2 = listview->Width;
				 cw1 = cw1<cw2 ? cw1 : cw2;
				 this->settings->changeSetting(str3,cw1.ToString());



			 }

			 void watcher_event(String^ name, String^ fullpath)
			 {
				 trace(1,printf("Watcher event:  name=%s  fullpath=%s\n",name,fullpath));
				 this->computer_needs_refreshing=true;

			 }

	private: System::Void fileSystemWatcher1_Changed(System::Object^  sender, System::IO::FileSystemEventArgs^  e) {
				 this->watcher_event(e->Name, e->FullPath);
			 }

	private: System::Void fileSystemWatcher1_Renamed(System::Object^  sender, System::IO::RenamedEventArgs^  e) {
				 this->watcher_event(e->Name, e->FullPath);

			 }

			 System::Void init_contextMenuStrip1 (bool force_init)
			 {
				 if (this->mi_pvr_copy != nullptr && !force_init) return;


				 System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));

				 this->mi_pvr_copy = gcnew ToolStripMenuItem(lang::cm_copy_pc,this->basicIconsSmall->Images["right-arrow_small.ico"]);
				 this->mi_pvr_proginfo = gcnew ToolStripMenuItem(lang::cm_info,(cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton11.Image"))));

				 this->mi_pvr_move = gcnew ToolStripMenuItem(lang::cm_move_pc,this->basicIconsSmall->Images["right-arrow_orange_small.ico"]);
				 this->mi_pvr_delete = gcnew ToolStripMenuItem(lang::tb_delete,(cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton3.Image"))));

				 this->mi_pvr_rename = gcnew ToolStripMenuItem(lang::cm_rename,this->basicIconsSmall->Images["rename.ico"]);
				 this->mi_pvr_select_all = gcnew ToolStripMenuItem(lang::cm_select_all);

				 System::Windows::Forms::ContextMenuStrip ^cm = this->contextMenuStrip1;


				 this->mi_pvr_rename->ShortcutKeyDisplayString="F2";
				 this->mi_pvr_delete->ShortcutKeyDisplayString="Del";
				 this->mi_pvr_select_all->ShortcutKeyDisplayString="Ctrl+A";


				 ToolStripItemCollection ^ic = cm->Items;

				 ic->Clear();
				 ic->Add(mi_pvr_copy);
				 ic->Add(mi_pvr_move);
				 ic->Add(mi_pvr_proginfo);
				 ic->Add(mi_pvr_delete);
				 ic->Add(mi_pvr_rename);
				 ic->Add(mi_pvr_select_all);

				 int ind=0;
				 for each (String^ str in headerNames) 
				 {

					 ToolStripMenuItem ^mi = gcnew ToolStripMenuItem(str);
					 this->mi_pvr_choose_columns_array[ind]=mi;
					 ic->Add(mi);
					 ind++;
				 }

				 //printf("init_contextMenuStrip1\n");



			 }

			 System::Void init_contextMenuStrip2 (bool force_init)
			 {
				 if (this->mi_pc_copy != nullptr && !force_init) return;



				 System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));

				 this->mi_pc_copy = gcnew ToolStripMenuItem(lang::cm_copy_pvr,this->basicIconsSmall->Images["left-arrow_small.ico"]);
				 this->mi_pc_proginfo = gcnew ToolStripMenuItem(lang::cm_info,(cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton11.Image"))));

				 this->mi_pc_move = gcnew ToolStripMenuItem(lang::cm_move_pvr,this->basicIconsSmall->Images["left-arrow_orange_small.ico"]);
				 this->mi_pc_delete = gcnew ToolStripMenuItem(lang::tb_delete,(cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton3.Image"))));
				 this->mi_pc_show_in_explorer = gcnew ToolStripMenuItem(lang::cm_explorer, this->basicIconsSmall->Images["show_file.ico"]);
				 this->mi_pc_install_firmware = gcnew ToolStripMenuItem(lang::cm_firmware,this->basicIconsSmall->Images["cog.ico"]);
				 this->mi_pc_rename = gcnew ToolStripMenuItem(lang::cm_rename,this->basicIconsSmall->Images["rename.ico"]);
				 this->mi_pc_select_all = gcnew ToolStripMenuItem(lang::cm_select_all);

				 this->mi_pc_rename->ShortcutKeyDisplayString="F2";
				 this->mi_pc_delete->ShortcutKeyDisplayString="Del";
				 this->mi_pc_select_all->ShortcutKeyDisplayString="Ctrl+A";

				 this->mi_pc_show_in_explorer->ShortcutKeyDisplayString="Ctrl+E";


				 System::Windows::Forms::ContextMenuStrip ^cm = this->contextMenuStrip2;

				 ToolStripItemCollection ^ic = cm->Items;

				 ic->Clear();
				 ic->Add(mi_pc_copy);
				 ic->Add(mi_pc_move);
				 ic->Add(mi_pc_proginfo);
				 ic->Add(mi_pc_delete);
				 ic->Add(mi_pc_show_in_explorer);
				 ic->Add(mi_pc_install_firmware);
				 ic->Add(mi_pc_rename);
				 ic->Add(mi_pc_select_all);


				 // this->mi_pc_choose_columns = gcnew ToolStripMenuItem("Choose columns:");



				 int ind=0;
				 for each (String^ str in headerNames) 
				 {


					 ToolStripMenuItem ^mi = gcnew ToolStripMenuItem(str);
					 this->mi_pc_choose_columns_array[ind]=mi;

					 // this->mi_pc_choose_columns->DropDownItems->Add(mi);
					 ic->Add(mi);

					 ind++;
				 }
				 //			 ic->Add(this->mi_pc_choose_columns);

				 //printf("init_contextMenuStrip2\n");


			 }


	private: System::Void contextMenuStrip1_Opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e) {


				 this->init_contextMenuStrip1(false);
				 double dt =   this->stopwatch->Elapsed.TotalSeconds - this->listview_click_time;
				 printf("dt=%f\n",dt);


				 ListView::SelectedListViewItemCollection^ selected = this->listView1->SelectedItems;
				 int numdir=0, numfile=0, numtfd=0, numrec=0, numselected=0;
				 for each (ListViewItem^ item in selected)
				 {
					 numselected++;
					 TopfieldItem^ citem = safe_cast<TopfieldItem^>(item);
					 printf("%s\n",citem->filename);
					 if (citem->isdir) numdir++; else numfile++;
					 if (!citem->isdir && citem->filename->EndsWith(".tfd",StringComparison::CurrentCultureIgnoreCase) )
						 numtfd++;
					 if (!citem->isdir && citem->filename->EndsWith(".rec",StringComparison::CurrentCultureIgnoreCase) )
						 numrec++;
				 }

				 bool choose_columns = (dt>.1) || (numdir + numfile + numtfd == 0);

				 this->mi_pvr_copy->Available=!choose_columns;
				 this->mi_pvr_delete->Available=!choose_columns;
				 this->mi_pvr_move->Available=!choose_columns;
				 this->mi_pvr_select_all->Available=!choose_columns;
				 this->mi_pvr_rename->Available=!choose_columns && numselected==1;
				 this->mi_pvr_proginfo->Available=numrec>0 && !choose_columns;

				 for (int ind =0; ind<this->headerNames->Length; ind++)
				 {
					 this->mi_pvr_choose_columns_array[ind]->Available=choose_columns;
					 if (choose_columns)
					 {
						 this->mi_pvr_choose_columns_array[ind]->Checked = this->settings["PVR_Column"+ind.ToString()+"Visible"]=="1";

					 }


				 }



				 e->Cancel=false;

			 }
	private: System::Void contextMenuStrip2_Opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e) {
				 this->init_contextMenuStrip2(false);

				 double dt =   this->stopwatch->Elapsed.TotalSeconds - this->listview_click_time;
				 printf("dt=%f\n",dt);


				 ListView::SelectedListViewItemCollection^ selected = this->listView2->SelectedItems;
				 int numdir=0, numfile=0, numtfd=0, numrec=0, numselected=0;
				 bool isdrive=false;
				 for each (ListViewItem^ item in selected)
				 {
					 numselected++;
					 ComputerItem^ citem = safe_cast<ComputerItem^>(item);
					 printf("%s\n",citem->filename);
					 if (citem->isdir) numdir++; else numfile++;
					 if (!citem->isdir && citem->filename->EndsWith(".tfd",StringComparison::CurrentCultureIgnoreCase) )
						 numtfd++;
					 if (!citem->isdir && citem->filename->EndsWith(".rec",StringComparison::CurrentCultureIgnoreCase) )
						 numrec++;
					 if (citem->isdrive) isdrive=true;
				 }

				 bool choose_columns = (dt>.1) || (numdir + numfile + numtfd == 0);

				 this->mi_pc_copy->Available=!choose_columns && !isdrive;
				 this->mi_pc_delete->Available=!choose_columns && !isdrive;
				 this->mi_pc_move->Available=!choose_columns && !isdrive;
				 this->mi_pc_select_all->Available=!choose_columns && !isdrive;
				 this->mi_pc_rename->Available=!choose_columns && !isdrive && numselected==1;
				 this->mi_pc_show_in_explorer->Available=!choose_columns;
				 this->mi_pc_proginfo->Available=numrec>0 && !choose_columns && !isdrive;

				 this->mi_pc_install_firmware->Available=false;
				 for (int ind =0; ind<this->headerNames->Length; ind++)
				 {
					 this->mi_pc_choose_columns_array[ind]->Available=choose_columns;
					 if (choose_columns)
					 {
						 this->mi_pc_choose_columns_array[ind]->Checked = this->settings["PC_Column"+ind.ToString()+"Visible"]=="1";

					 }


				 }

				 if (!choose_columns)
				 {


					 this->mi_pc_install_firmware->Available = (numfile == 1 && numtfd==1 && numdir == 0);
				 }



				 e->Cancel=false;

			 }
	private: System::Void contextMenuStrip1_ItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
				 ToolStripMenuItem^ mi = safe_cast<ToolStripMenuItem^>(e->ClickedItem);

				 for (int i=0; i<this->mi_pvr_choose_columns_array->Length; i++)
				 {
					 if (mi == this->mi_pvr_choose_columns_array[i])
					 {
						 printf(" Column %d !!! \n",i);


						 String^ str = "PVR_Column"+i.ToString()+"Visible";
						 this->settings->changeSetting(str, this->settings[str]=="1" ? "0" : "1");
						 this->apply_columns_visible();
						 this->refreshTopfield();
						 return;
					 }
				 }
				 if (mi == this->mi_pvr_copy)
				 {
					 this->transfer_selection_to_PC(gcnew TransferOptions(CopyMode::COPY));
				 }
				 else if (mi == this->mi_pvr_delete)
				 {
					 this->delete_PVR_selection();

				 } 
				 else if (mi == this->mi_pvr_move)
				 {
					 this->transfer_selection_to_PC(gcnew TransferOptions(CopyMode::MOVE));
				 }
				 else if (mi == this->mi_pvr_proginfo)
				 {
					 if (this->transfer_in_progress) return;
					 this->ViewInfo(this->listView1);
				 }
				 else if (mi == this->mi_pvr_rename)
				 {

					 ListView::SelectedListViewItemCollection^ selected = this->listView1->SelectedItems;
					 try{
						 selected[0]->BeginEdit();
					 } catch(...) {};

				 }
				 else if (mi == this->mi_pvr_select_all)
				 {

					 ListView::ListViewItemCollection ^c =  this->listView1->Items;
					 for each (ListViewItem^ it in c) it->Selected=true;
				 }




			 }

	private: System::Void listView_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
				 this->listview_click_time = this->stopwatch->Elapsed.TotalSeconds;
			 }
	private: System::Void contextMenuStrip2_ItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {

				 ToolStripMenuItem^ mi = safe_cast<ToolStripMenuItem^>(e->ClickedItem);
				 //printf("%s \n",mi->ToString());

				 for (int i=0; i<this->mi_pc_choose_columns_array->Length; i++)
				 {
					 if (mi == this->mi_pc_choose_columns_array[i])
					 {
						 //printf(" Column %d !!! \n",i);


						 String^ str = "PC_Column"+i.ToString()+"Visible";
						 this->settings->changeSetting(str, this->settings[str]=="1" ? "0" : "1");
						 this->apply_columns_visible();
						 this->refreshComputer();
						 return;
					 }
				 }

				 if (mi == this->mi_pc_show_in_explorer)
				 {
					 this->show_in_explorer();
				 }
				 else if (mi == this->mi_pc_copy)
				 {
					 this->transfer_selection_to_PVR(gcnew TransferOptions(CopyMode::COPY));
				 }
				 else if (mi == this->mi_pc_delete)
				 {
					 this->delete_PC_selection();

				 } 
				 else if (mi == this->mi_pc_install_firmware)
				 {

					 ComputerItem ^ item;

					 ListView::SelectedListViewItemCollection^ selected = this->listView2->SelectedItems;

					 for (int i=0 ;  i<selected->Count; i++)
					 {
						 item = safe_cast<ComputerItem^>(selected[i]);
						 if ( (!item->isdir) && item->full_filename->EndsWith(".tfd",StringComparison::CurrentCultureIgnoreCase))
						 {
							 this->install_firmware(item->full_filename);
							 return;
						 }
					 }

				 }
				 else if (mi == this->mi_pc_move)
				 {
					 this->transfer_selection_to_PVR(gcnew TransferOptions(CopyMode::MOVE));
				 }
				 else if (mi == this->mi_pc_proginfo)
				 {
					 this->ViewInfo(this->listView2);

				 }
				 else if (mi == this->mi_pc_rename)
				 {

					 ListView::SelectedListViewItemCollection^ selected = this->listView2->SelectedItems;
					 try{
						 selected[0]->BeginEdit();
					 } catch(...) {};

				 }
				 else if (mi == this->mi_pc_select_all)
				 {

					 ListView::ListViewItemCollection ^c =  this->listView2->Items;
					 for each (ListViewItem^ it in c) it->Selected=true;
				 }


			 }

			 void show_in_explorer(void)
			 {
				 ComputerItem ^ item;

				 ListView::SelectedListViewItemCollection^ selected = this->listView2->SelectedItems;

				 if (selected->Count==0) return;
				 item = safe_cast<ComputerItem^>(selected[0]);


				 System::Diagnostics::Process::Start("explorer.exe", "/select,"+item->full_filename);

			 }


			 void apply_language(void)
			 {

				 ///////////////////////////
				 /// Toolbar buttons

				 // Up
				 this->toolStripButton1->Text = lang::tb_up;
				 this->toolStripButton5->Text = lang::tb_up;

				 this->toolStripButton1->ToolTipText = lang::tt_up;
				 this->toolStripButton5->ToolTipText = lang::tt_up;


				 // Refresh
				 this->toolStripButton2->Text = lang::tb_refresh;
				 this->toolStripButton6->Text = lang::tb_refresh;

				 this->toolStripButton2->ToolTipText = lang::tt_refresh;
				 this->toolStripButton6->ToolTipText = lang::tt_refresh;

				 // Delete
				 this->toolStripButton3->Text = lang::tb_delete;
				 this->toolStripButton7->Text = lang::tb_delete;

				 this->toolStripButton3->ToolTipText = lang::tt_delete;
				 this->toolStripButton7->ToolTipText = lang::tt_delete;


				 // New Folder
				 this->toolStripButton4->Text = lang::tb_new;
				 this->toolStripButton8->Text = lang::tb_new;

				 this->toolStripButton4->ToolTipText = lang::tt_new;
				 this->toolStripButton8->ToolTipText = lang::tt_new;


				 // View Info
				 this->toolStripButton11->Text = lang::tb_info;
				 this->toolStripButton12->Text = lang::tb_info;

				 this->toolStripButton11->ToolTipText = lang::tt_info;
				 this->toolStripButton12->ToolTipText = lang::tt_info;

				 //cut
				 this->toolStripButton9->Text = lang::tb_cut;
				 this->toolStripButton9->ToolTipText = lang::tt_cut;

				 //paste
				 this->toolStripButton10->Text = lang::tb_paste;
				 this->toolStripButton10->ToolTipText = lang::tt_paste;

				 //settings
				 this->toolStripButton13->Text = lang::tb_settings;
				 this->toolStripButton13->ToolTipText = lang::tt_settings;



				 //////////////////

				 // turbo mode check
				 this->checkBox1->Text = lang::tb_turbo_mode;

				 // move check

				 this->checkBox2->Text = lang::cb_move;    


				 ///////////////////// Headers

				 this->computerNameHeader->Text = lang::h_name;
				 this->topfieldNameHeader->Text = lang::h_name;
				 this->headerNames[0]=lang::h_name;


				 this->computerSizeHeader->Text = lang::h_size;
				 this->topfieldSizeHeader->Text = lang::h_size;
				 this->headerNames[1]=lang::h_size;

				 this->computerTypeHeader->Text = lang::h_type;
				 this->topfieldTypeHeader->Text = lang::h_type;
				 this->headerNames[2]=lang::h_type;

				 this->computerDateHeader->Text = lang::h_date;
				 this->topfieldDateHeader->Text = lang::h_date;
				 this->headerNames[3]=lang::h_date;

				 this->computerChannelHeader->Text = lang::h_channel;
				 this->topfieldChannelHeader->Text = lang::h_channel;
				 this->headerNames[4]=lang::h_channel;

				 this->computerDescriptionHeader->Text = lang::h_description;
				 this->topfieldDescriptionHeader->Text = lang::h_description;
				 this->headerNames[5]=lang::h_description;

				 // On-completion actions
				 OnCompletionAction::option_strings[1] = lang::c_sleep;
				 OnCompletionAction::option_strings[2] = lang::c_hibernate;
				 OnCompletionAction::option_strings[3] = lang::c_shutdown;

				 // Right-click context menu
				 this->init_contextMenuStrip1(true);
				 this->init_contextMenuStrip2(true);


				 this->Arrange_Buttons(); // mainly to centre Move checkbox.

			 }

			 void apply_language_setting(void)
			 {
				 String^ language = this->settings["language"];
				 if (language=="en-au")
					 lang::set_en_au();
				 else if (language=="en-gb")
					 lang::set_en_gb();
				 else if (language=="de")
					 lang::set_de();
				 else if (language=="fi")
					 lang::set_fi();
				 else     //auto
				 {
					 String^ culture = System::Globalization::CultureInfo::CurrentCulture->Name->ToLower();
					 String^ ui_culture = System::Globalization::CultureInfo::CurrentUICulture->Name->ToLower();

					 if (ui_culture->StartsWith("de")  || culture->StartsWith("de")  )
						 lang::set_de();
					 else if (ui_culture->StartsWith("fi") || culture->StartsWith("fi") )
						 lang::set_fi();
					 else if (culture=="en-gb")
						 lang::set_en_gb();
					 //else if (culture->StartsWith("fi"))
					 //	 lang::set_fi();
					 //else if (culture->StartsWith("de"))
					 //	 lang::set_de();
					 else lang::set_en_au();


				 }

				 this->apply_language();
			 }


			 /*
			 private: System::Void listView2_ItemMouseHover(System::Object^  sender, System::Windows::Forms::ListViewItemMouseHoverEventArgs^  e) {
			 Console::WriteLine("ItemMouseHover, " + e->Item->Text);
			 }
			 */

			 static String^ wordwrap(String^ str, int w)
			 {
				 String^ s = "";

				 try{
					 if (str!=nullptr)
					 {
						 while(str->Length>0)
						 {
							 int q;
							 String ^chunk;
							 if (str->Length<w) 
							 {
								 q = str->Length;
								 chunk = str;
								 str="";
							 }
							 else
							 {
								 q=w;
								 //chunk=str->Substring(0,q);
								 //str = str->Substring(q);
								 for (int j=w-1; j>0; j--)
								 {
									 if (str[j]==' ')
									 {
										 q=j;
										 chunk=str->Substring(0,q);
										 str=str->Substring(q+1);
										 break;
									 }
								 }
								 if (q==w)
								 {
									 chunk=str->Substring(0,q);
									 str=str->Substring(q+1);
								 }
							 }
							 if (s->Length>0) s = s+"\r\n";
							 s=s+chunk;


						 }

					 }
				 }
				 catch(...){}

				 return s;
			 }

	private: System::Void listView_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {


				 static int cnt = 0;
				 static int lastx, lasty;// lasthash;
				 int hash=0;


				 toolTip1->AutoPopDelay = 5000;
				 toolTip1->InitialDelay = 1000;
				 toolTip1->ReshowDelay = 500;

				 ListView ^list = safe_cast<ListView^> (sender);



				 FileItem ^item = safe_cast<FileItem^> (list->GetItemAt(e->X, e->Y));

				 bool over_copydialog = false;
				 Antares::CopyDialog ^copydialog = this->current_copydialog;

				 if (copydialog != nullptr)
				 {

					 Point abs = Point(e->X, e->Y);
					 abs.Offset(panel1->Location);
					 if (list==tlist)
					 {
						 abs.Offset(panel3->Location);
						 abs.Offset(tlist->Location);
					 }
					 else
					 {
						 abs.Offset(panel4->Location);
						 abs.Offset(clist->Location);
					 }


					 //printf("%d %d : %d %d : (%d,%d)-(%d,%d)\n",e->X, e->Y, abs.X, abs.Y, 
					 //copydialog->Bounds.Left,  copydialog->Bounds.Top,  copydialog->Bounds.Right,  copydialog->Bounds.Bottom);


					 if (copydialog->Bounds.Contains(abs.X, abs.Y) )
						 over_copydialog = true;
				 }


				 if (!(lastx==e->X && lasty==e->Y))
				 {
					 bool clear=true;
					 if(item != nullptr && !over_copydialog  )
					 {




						 ListViewItem::ListViewSubItem ^subitem = item->GetSubItemAt(e->X, e->Y);


						 hash = subitem->GetHashCode();
						 //Console::WriteLine(hash.ToString());

						 if (subitem->Tag == "desc")
						 {

							 if (hash != lasthash)

							 {

								 System::Drawing::Point p = subitem->Bounds.Location;
								 p.Y = p.Y + item->Bounds.Height+8;
								 //p.X = e->X+6;
								 p.X = subitem->Bounds.Left;


								 int ww=60;
								 String ^str = item->description->Trim();
								 DateTime prog_end_time = item->prog_start_time.AddMinutes(item->proglen);

								 // String^ str2 = item->channel +"\r\n"+item->prog_start_time.ToString("f") ;


								 String^ str2b = (item->proglen % 60).ToString() +lang::u_minutes;
								 int prog_hr = item->proglen/60;
								 if (prog_hr>0) str2b = prog_hr.ToString() + lang::u_hours+" " + str2b;

								 str2b = lang::p_proglen +"   "+str2b;

								 String^ str3 = (item->reclen % 60).ToString() +lang::u_minutes;
								 int recorded_hr = item->reclen/60;
								 if (recorded_hr>0) str3 = recorded_hr.ToString() + lang::u_hours+" " + str3;
								 str3 =lang::p_reclen+"   "+str3;

								 String^ tit_chan = item->title;
								 //try {
								 //	 tit_chan = tit_chan->PadRight(ww + 3 - item->channel->Length);
								 // } catch(...){}
								 tit_chan = tit_chan + "  ("+item->channel+")";

								 if (str->Length>0)
								 {
									 str = wordwrap(str,ww);

									 //str = item->title + "\r\n\r\n" +str + "\r\n\r\n"+item->channel+"\r\n"+str2b+"\r\n"+str3;

									 str = tit_chan + "\r\n\r\n" +str + "\r\n\r\n"+str2b+"\r\n"+str3;


									 this->tooltip_string = str;
									 this->tooltip_location = p;
									 this->tooltip_listview = list;
									 this->tooltip_timer->Interval=100;
									 this->tooltip_timer->Start();
									 this->toolTip1->SetToolTip(list, "");


									 //this->toolTip1->Show(str, list, p,20000);
									
									 //Console::WriteLine("Set: "+wordwrap(item->description,60));
									 clear=false;
									 lasthash=hash;
								 }


							 }   // if hash != lasthash

							 else
								 clear=false;
						 }   // if subitem->Tag == "desc"

						 else
						 {
							 if (lasthash == hash) clear=false;
						 }

						 //this->toolTip1->SetToolTip(list,  "This\r\nis\r\na\r\ntest"+item->description);

					 }     // if item != nullptr
					 if(clear)
					 {
						 this->tooltip_string="";
						 this->toolTip1->SetToolTip(list, "");
						 
						 lasthash=0;
						 //Console::WriteLine("Clear"+cnt.ToString() );cnt++;
					 }
				 }   // if not (lastx==X && lasty==Y)

				 // Console::WriteLine("MouseMove" + cnt.ToString());cnt++;

				 lastx = e->X; lasty=e->Y;




			 }

			 static void set_filesystemwatcher_callback(Object^ obj)
			 {

				 Form1 ^frm = safe_cast<Form1^>(obj);
				 String ^dir = frm->watched_directory;

				 Monitor::Enter(frm->fileSystemWatcher1);

				 trace(1,printf("Setting filesystemwatcher: %s\n",dir));

				 try{

					 frm->fileSystemWatcher1->Path = dir;
					 frm->fileSystemWatcher1->NotifyFilter = NotifyFilters::LastWrite
						 | NotifyFilters::FileName | NotifyFilters::DirectoryName | NotifyFilters::Size;


					 frm->fileSystemWatcher1->EnableRaisingEvents=true;

				 }catch(...){
					 trace(0,printf("Exception caught setting filesystemwatcher.\n"));
				 };

				 Monitor::Exit(frm->fileSystemWatcher1);

			 }

			 void set_filesystemwatcher(String ^ dir)
			 {


				 this->watched_directory = dir;
				 ThreadPool::QueueUserWorkItem( gcnew WaitCallback( Form1::set_filesystemwatcher_callback ), this );

				 //Form1::set_filesystemwatcher_callback(this);


			 }

			 // Try to determine if a recording is currently in progress.
			 // The answer might be a little out of date, since caching is used.
			 bool recording_in_progress(void)
			 {

				 this->recording_in_progress_last_checked = DateTime::Now;

				 if (this->settings["prevent_turbo_while_recording"]=="0") return false;

				 //printf("Is recording in progress?\n");


				 bool needs_refreshing = (this->data_files_cached == nullptr);

				 if (this->data_files_cached != nullptr)
				 {
					 if ( (DateTime::Now-this->data_files_read_time).TotalSeconds > 5)
						 needs_refreshing=true;

				 }

				 if (needs_refreshing)
				 {
					 Monitor::Enter(this->locker);
					 this->loadTopfieldDirArrayOrNull("\\DataFiles");
					 Monitor::Exit(this->locker);
				 }



				 if (this->data_files_cached == nullptr) return false;  // If in doubt, return false.


				 for each (TopfieldItem^ item in this->data_files_cached)
				 {
					 if (item->isdir) continue;
					 if (! item->filename->EndsWith(".rec",StringComparison::CurrentCultureIgnoreCase)) continue;

					 if (  Math::Abs (  (DateTime::Now - item->datetime).TotalHours ) > 24.0 )  continue;
					 //printf(" ### %s : %g\n", item->filename,(DateTime::Now - item->datetime).TotalHours );
					 if (item->description->Length > 0 || item->channel->Length	> 0 ) continue;

					 if (item->size == 0 ) return true;

					 Monitor::Enter(this->locker);
					 array <unsigned char>^ arr = this->read_topfield_file_snippet(item->full_filename, 0);
					 Monitor::Exit(this->locker);

					 if (arr == nullptr) return true;
					 if (arr->Length==0) return true;
				 }


				 return false;
			 }



	private: System::Void listView_MouseLeave(System::Object^  sender, System::EventArgs^  e) {

				 this->tooltip_string="";
				 ListView^ list = safe_cast<ListView^>(sender);
				 // Hide program description tooltip
				 this->toolTip1->SetToolTip(list, "");

				 lasthash=0;

			 }

			 void tooltip_timer_elapsed(System::Object ^sender, System::Timers::ElapsedEventArgs ^e)
			 {
				 static int cnt=0;
				 if (this->tooltip_listview == nullptr || this->tooltip_string == nullptr) return;
				 //printf("Elapsed.  %d\n",cnt++);
				 if (this->tooltip_string->Length==0)
				 {
					 this->toolTip1->SetToolTip(this->tooltip_listview, "");
					 return;

				 }
				 this->toolTip1->Show(this->tooltip_string, this->tooltip_listview, this->tooltip_location,20000);
			 }


	};    // class form1
};    // namespace antares

