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


}

#using <mscorlib.dll>



extern FILE old_stdout;
extern FILE old_stderr;
extern FILE* hf;


#include "copydialog.h"
#include "deleteconfirmation.h"

//ref class CopyDialog ;

#include "antares.h"

System::String^ HumanReadableSize(__u64 size);
System::String^ DateString(time_t time);
System::String^ safeString( char* filename );
time_t DateTimeToTime_T(System::DateTime datetime);

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


	[DllImport("user32.dll", EntryPoint = "SendMessage", CharSet = CharSet::Auto)]
	  LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


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

	protected:
		  virtual void WndProc( Message% m ) override
      {

         // Listen for operating system messages.
         switch ( m.Msg )
          {
            case WM_ACTIVATEAPP:

               break;
			   case WM_DEVICECHANGE:
 printf("WM_DEVICECHANGE received. m.WParam =%d   m.LParam=%d \n",m.WParam,m.LParam);
             this->CheckConnection();
         }
		 
         Form::WndProc( m );
      }



	public:
		Form1(void)
		{
			//ListViewItem^ item;
			TopfieldItem^ item;


			int reason;
			this->listView1SortColumn = -1;
			this->listView2SortColumn=-1;
			this->turbo_mode = gcnew System::Boolean;

			this->finished_constructing = 0;
			InitializeComponent();

			TopfieldMutex = gcnew System::Threading::Mutex(false);
			this->important_thread_waiting=false;


			this->setTopfieldDir("\\DataFiles\\");
			this->setComputerDir("C:\\");

			this->SetStyle(ControlStyles::AllPaintingInWmPaint | ControlStyles::UserPaint | ControlStyles::DoubleBuffer,true);



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
			this->listView1->SmallImageList = this->basicIconsSmall;
			this->listView2->SmallImageList = this->basicIconsSmall;


			// Load configuration. 
			
			try{
				int sort_PC=0;  int sort_PVR=0;
				this->config =  ConfigurationManager::OpenExeConfiguration( ConfigurationUserLevel::None);
				this->settings=config->AppSettings->Settings;
				if (nullptr!=settings["TurboMode"])
					if (String::Compare("on",settings["TurboMode"]->Value)==0) this->checkBox1->Checked = true; else this->checkBox1->Checked = false;

				if (nullptr!=settings["TopfieldDir"])
					this->setTopfieldDir(settings["TopfieldDir"]->Value);
				if (nullptr!=settings["ComputerDir"])
					this->setComputerDir(settings["ComputerDir"]->Value);
				if (nullptr!=settings["PVR_SortColumn"])
				{
					this->listView1SortColumn = System::Convert::ToInt32(settings["PVR_SortColumn"]->Value);
					sort_PVR++;
				}
				if (nullptr!=settings["PC_SortColumn"])
				{
					this->listView2SortColumn = System::Convert::ToInt32(settings["PC_SortColumn"]->Value);
					Console::WriteLine("PC_SortColumn was "+this->listView2SortColumn.ToString());
					sort_PC++;
				}
				if (nullptr!=settings["PVR_SortOrder"])
				{
					if (String::Compare(settings["PVR_SortOrder"]->Value, "Ascending") == 0)  
					{
						this->listView1->Sorting = SortOrder::Ascending;
						sort_PVR++;
					}

                    if (String::Compare(settings["PVR_SortOrder"]->Value, "Descending") == 0)  
					{
						this->listView1->Sorting = SortOrder::Descending;
						sort_PVR++;
					}
				}
				if (nullptr!=settings["PC_SortOrder"])
				{
					if (String::Compare(settings["PC_SortOrder"]->Value, "Ascending") == 0)  
					{
						this->listView2->Sorting = SortOrder::Ascending;
						Console::WriteLine("PC_SortOrder was Ascending");
						sort_PC++;
					}

                    if (String::Compare(settings["PC_SortOrder"]->Value, "Descending") == 0)  
					{
						this->listView2->Sorting = SortOrder::Descending;
						Console::WriteLine("PC_SortOrder was Descending");
						sort_PC++;
					}
				}
				if (sort_PC>1)
				{
					this->listView2->ListViewItemSorter = gcnew ListViewItemComparer(this->listView2SortColumn,this->listView2->Sorting);
				}
					if (sort_PVR>1)
				{
					this->listView1->ListViewItemSorter = gcnew ListViewItemComparer(this->listView1SortColumn,this->listView1->Sorting);
				}
				
			}
			catch(...)
			{

			}


			this->TopfieldClipboard = gcnew array<String^> {};
			this->TopfieldClipboardDirectory = "";

			this->finished_constructing = 1;

			this->loadTopfieldDir();
			this->loadComputerDir();

		
			// Enable double-buffering on the ListViews
			LRESULT styles = Antares::SendMessage((HWND) this->listView2->Handle.ToPointer(), (int) LVM_GETEXTENDEDLISTVIEWSTYLE, 0,0);
            styles |= LVS_EX_DOUBLEBUFFER ;
			Antares::SendMessage((HWND)this->listView2->Handle.ToPointer(), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (int) styles);
            Antares::SendMessage((HWND)this->listView1->Handle.ToPointer(), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (int) styles);


		 this->CheckConnection();

		}




		void changeSetting(String^ key, String^ val)
		{
			if (this->config != nullptr && this->settings != nullptr)
			{
				if (this->settings[key] == nullptr)
				{
					this->settings->Add(key,val);
					this->config->Save(ConfigurationSaveMode::Modified);
				     Console::WriteLine("Added the setting "+key+" to " +val);
				}
				else
				{
					if (String::Compare(val, this->settings[key]->Value)==0) return;
				
				this->settings[key]->Value = val;
				this->config->Save(ConfigurationSaveMode::Modified);
				Console::WriteLine("Changed the setting "+key+" to " +val);
				}
			}
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
			struct libusb_bus * bus;
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
                printf("Topfield is now disconnected.\n");
				this->label2->Text = "PVR: Device not connected";
				this->listView1->Items->Clear();
				return;
			}


			if (this->fd !=NULL && device!=NULL)  // Topfield is apparently still connected
			{
			    libusb_free_device_list(devs, 1);
				printf("Topfield is still connected.\n");
				return;
			}


			if (this->fd == NULL && device==NULL)  // Topfield is apparently still disconnected
			{
				libusb_free_device_list(devs, 1);
				printf("Topfield is still disconnected.\n");
				this->label2->Text = "PVR: Device not connected";
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
			this->loadTopfieldDir();
			return;
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


		void loadComputerDir(void)
		{

			int j;
			ComputerItem^ item;
			array<ComputerItem^>^ items = {};
			


			String^ dir = this->computerCurrentDirectory;

			if (dir->Equals(""))  // List drives
			{
					this->listView2->Items->Clear();
				DWORD drives = GetLogicalDrives();
				for (j=0; j<26; j++)
				{
					if ( (drives&1)==1)
					{
						item = gcnew ComputerItem(j);
						this->listView2->Items->Add(item);
					}

					drives>>=1;
				}
				this->label1->Text = "My Computer";
				this->changeSetting("ComputerDir","");
			}
			else   //List contents of actual directory
			{
				array<String^>^ list;
				try{
					list = System::IO::Directory::GetFileSystemEntries(dir);
				}
				catch(System::IO::IOException ^E)
				{
					this->setComputerDir("");
					this->loadComputerDir();
					return;
				}
				catch(System::UnauthorizedAccessException ^E)
				{
					toolStripStatusLabel1->Text="Access denied: "+dir;
					this->computerUpDir();
					return;

				}
				Array::Resize(items,list->Length);
				for (j=0; j<list->Length; j++)
				{

					item = gcnew ComputerItem(list[j]);
					item->directory = dir;
					//this->listView2->Items->Add(item);
					items[j]=item;
				}
				this->listView2->BeginUpdate();
				this->listView2->Items->Clear();
				this->listView2->Items->AddRange(items);
				this->listView2->EndUpdate();
                this->changeSetting("ComputerDir",dir);
				// Add a drive summary to label1:
				String^ str = Path::GetPathRoot(dir);
				if (str->Length > 0)
				{
					DriveInfo^ drive = gcnew DriveInfo(str);
					str = " Local Disk "+str+"  --  " + HumanReadableSize(drive->AvailableFreeSpace) + " Free / " + HumanReadableSize(drive->TotalSize)+ " Total";
					label1->Text = str;

				}
				else
				{
					label1->Text = "";
				}

			}

		};


		void computerUpDir(void)
		{
			String^ dir = this->computerCurrentDirectory;
			String^ dir2;
			dir2="";
			try{
				dir2 = Path::GetDirectoryName(dir);
			}
			catch (System::ArgumentException^ E)
			{
				dir2="";
			}
			//if (dir2==NULL) dir2="";


			try
			{
				dir2=dir2+"";
			}
			catch(System::NullReferenceException^ E)
			{
				dir2="";
			}

			this->setComputerDir(dir2);
			this->loadComputerDir();
		}
		void topfieldUpDir(void)
		{
			// Todo: make this a bit more robust. What happens when directory name has a special character?
			String^ dir = this->topfieldCurrentDirectory;
			String^ dir2;
			try{
				dir2 = Path::GetDirectoryName(dir);
			}
			catch (System::ArgumentException^ E)
			{
				dir2="";
			}
			//if (dir2==NULL) dir2="";


			try
			{
				dir2=dir2+"";
			}
			catch(System::NullReferenceException^ E)
			{
				dir2="";
			}

			this->setTopfieldDir(dir2);
			this->loadTopfieldDir();
		}

	
		// Load and display files in the current topfield directory.
		// If a file is named start_rename, then start the name editing process after the directory is loaded.
		// (useful when we have just created a new folder).
		int loadTopfieldDir(String^ start_rename)               // Uses the TopfieldMutex. Need to release before return.
		{
			tf_packet reply;

			__u16 count;
			int i,j,r;

			char type;
			time_t timestamp;
			//struct tm *newtime;
			struct typefile *entries;


			TopfieldItem^ item;
            array<TopfieldItem^>^ items = {};

			this->important_thread_waiting=true;
			this->TopfieldMutex->WaitOne();
			this->important_thread_waiting=false;

			
			if (this->fd==NULL)
			{
				toolStripStatusLabel1->Text="Topfield not connected.";
				return -EPROTO; 
			}




			bool hdd_size_successful = false;
			unsigned int totalk, freek;
			r = send_cmd_hdd_size(fd);
			if(r < 0)
			{
				return -EPROTO;
			}

			r = get_tf_packet(fd, &reply);
			if(r < 0)
			{
				return -EPROTO;
			}

			switch (get_u32(&reply.cmd))
			{
			case DATA_HDD_SIZE:
				{
					totalk = get_u32(&reply.data);
					freek = get_u32(&reply.data[4]);

					//printf("Total %10u kB %7u MB %4u GB\n", totalk, totalk / 1024,
					//	totalk / (1024 * 1024));
					//printf("Free  %10u kB %7u MB %4u GB\n", freek, freek / 1024,
					//	freek / (1024 * 1024));
					//return 0;
					hdd_size_successful=true;
					break;
				}

			case FAIL:
				fprintf(stderr, "ERROR: Device reports %s\n",
					decode_error(&reply));
				break;

			default:
				fprintf(stderr, "ERROR: Unhandled packet in load_topfield_dir/hdd_size\n");
			}

			this->label2->Text = " Topfield device  --  "+HumanReadableSize(1024* ((__u64) freek))+" free / " + HumanReadableSize(1024*( (__u64) totalk)) + " total";






			toolStripStatusLabel1->Text="Beginning dir "+this->dircount.ToString();

			char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(this->topfieldCurrentDirectory);
			//printf(str2);
			send_cmd_hdd_dir(this->fd,str2);
			Marshal::FreeHGlobal((System::IntPtr)(void*)str2);
            TopfieldItem^ rename_item;bool do_rename=false;
			j=0;
			int numitems=0;
			while(0 < get_tf_packet(this->fd, &reply))
			{


				j=j+1; 
				toolStripStatusLabel1->Text=j.ToString() + "/" + this->dircount.ToString();
				switch (get_u32(&reply.cmd))
				{
				case DATA_HDD_DIR:
					//decode_dir(&reply);

					count =
						(get_u16(&reply.length) - PACKET_HEAD_SIZE) / sizeof(struct typefile);
					entries = (struct typefile *) reply.data;
					Array::Resize(items, items->Length + count);


					for(i = 0; (i < count); i++)
					{


						item = gcnew TopfieldItem(&entries[i]);
						item->directory = this->topfieldCurrentDirectory;
						if (String::Compare(item->filename,"..")!=0 ) 
						{
							//this->listView1->Items->Add(item);
							
                            items[numitems] = item;
							numitems++;
							if (String::Compare(start_rename,item->filename)==0)
							{
								rename_item=item; do_rename=true;
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
                         

					}

					send_success(this->fd);
					toolStripStatusLabel1->Text="Sent success, j="+j.ToString();

					break;

				case DATA_HDD_DIR_END:
					//toolStripStatusLabel1->Text="Finished dir "+this->dircount.ToString();
					this->TopfieldMutex->ReleaseMutex();

					this->changeSetting("TopfieldDir",this->topfieldCurrentDirectory);
					Array::Resize(items,numitems);
                    this->listView1->BeginUpdate();
					this->listView1->Items->Clear();
                    this->listView1->Items->AddRange(items);
					this->listView1->EndUpdate();
					if (do_rename) rename_item->BeginEdit();
					return 0;
					break;

				case FAIL:
					fprintf(stderr, "ERROR: Device reports %s\n",
						decode_error(&reply));
					//toolStripStatusLabel1->Text="FAIL in dir " + this->dircount.ToString();
					this->TopfieldMutex->ReleaseMutex();
					return -EPROTO;
					break;

				default:
					fprintf(stderr, "ERROR: Unhandled packet\n");
					//toolStripStatusLabel1->Text="UNHANDLED in dir " + this->dircount.ToString();
					this->TopfieldMutex->ReleaseMutex();
					return -EPROTO;
				}

			}
			//toolStripStatusLabel1->Text="Unexpected finished dir "+this->dircount.ToString();
			this->TopfieldMutex->ReleaseMutex();
			return -EPROTO;
		}

		int loadTopfieldDir(void)
		{
            String^ start_rename = "";
			return this->loadTopfieldDir(start_rename);
		}


	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
#ifdef _DEBUG
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

			System::Threading::Mutex^ TopfieldMutex;
			bool important_thread_waiting;

			bool^ turbo_mode;

			//System::Windows::Forms::ImageList^ basicIconsSmall;

			int listView1SortColumn;
			int listView2SortColumn;
		
            // Configuration related:
			System::Configuration::Configuration^ config;
			System::Configuration::KeyValueConfigurationCollection^ settings;


			array<String^>^ TopfieldClipboard;  
			String^ TopfieldClipboardDirectory;

			static Color cut_background_colour = Color::FromArgb(255,250,105);
			static Color normal_background_colour = Color::FromArgb(255,255,255);










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
private: System::Windows::Forms::TextBox^  textBox2;
private: System::Windows::Forms::Panel^  panel6;
private: System::Windows::Forms::TextBox^  textBox1;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;




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
			this->toolStripStatusLabel1->Size = System::Drawing::Size(118, 17);
			this->toolStripStatusLabel1->Text = L"toolStripStatusLabel1";
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
			this->panel3->Controls->Add(this->panel5);
			this->panel3->Controls->Add(this->checkBox1);
			this->panel3->Controls->Add(this->label2);
			this->panel3->Controls->Add(this->toolStrip2);
			this->panel3->Controls->Add(this->listView1);
			this->panel3->Dock = System::Windows::Forms::DockStyle::Fill;
			this->panel3->Location = System::Drawing::Point(0, 0);
			this->panel3->Margin = System::Windows::Forms::Padding(0, 3, 0, 3);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(503, 638);
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
			this->panel5->Size = System::Drawing::Size(503, 32);
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
			this->textBox2->Size = System::Drawing::Size(491, 19);
			this->textBox2->TabIndex = 7;
			this->textBox2->Text = L"\\ProgramFiles";
			// 
			// checkBox1
			// 
			this->checkBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->checkBox1->AutoSize = true;
			this->checkBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->checkBox1->Location = System::Drawing::Point(418, 11);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 7;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = false;
			this->checkBox1->CheckedChanged += gcnew System::EventHandler(this, &Form1::checkBox1_CheckedChanged);
			// 
			// label2
			// 
			this->label2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(230)), static_cast<System::Int32>(static_cast<System::Byte>(230)), 
				static_cast<System::Int32>(static_cast<System::Byte>(230)));
			this->label2->Dock = System::Windows::Forms::DockStyle::Top;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(0, 38);
			this->label2->Margin = System::Windows::Forms::Padding(5);
			this->label2->Name = L"label2";
			this->label2->Padding = System::Windows::Forms::Padding(5, 0, 0, 0);
			this->label2->Size = System::Drawing::Size(503, 24);
			this->label2->TabIndex = 5;
			this->label2->Text = L"label2";
			this->label2->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStrip2
			// 
			this->toolStrip2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->toolStrip2->GripMargin = System::Windows::Forms::Padding(1);
			this->toolStrip2->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(8) {this->toolStripButton5, 
				this->toolStripButton6, this->toolStripButton7, this->toolStripButton8, this->toolStripSeparator1, this->toolStripButton9, this->toolStripButton10, 
				this->toolStripSeparator2});
			this->toolStrip2->LayoutStyle = System::Windows::Forms::ToolStripLayoutStyle::HorizontalStackWithOverflow;
			this->toolStrip2->Location = System::Drawing::Point(0, 0);
			this->toolStrip2->Name = L"toolStrip2";
			this->toolStrip2->Padding = System::Windows::Forms::Padding(0, 0, 4, 0);
			this->toolStrip2->Size = System::Drawing::Size(503, 38);
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
			this->toolStripButton10->CheckOnClick = true;
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
			// listView1
			// 
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
			this->listView1->Size = System::Drawing::Size(491, 545);
			this->listView1->TabIndex = 0;
			this->listView1->UseCompatibleStateImageBehavior = false;
			this->listView1->View = System::Windows::Forms::View::Details;
			this->listView1->ItemActivate += gcnew System::EventHandler(this, &Form1::listView1_ItemActivate);
			this->listView1->AfterLabelEdit += gcnew System::Windows::Forms::LabelEditEventHandler(this, &Form1::listView_AfterLabelEdit);
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
			this->panel2->Location = System::Drawing::Point(503, 0);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(39, 638);
			this->panel2->TabIndex = 7;
			// 
			// button2
			// 
			this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button2->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->button2->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->button2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"button2.Image")));
			this->button2->Location = System::Drawing::Point(0, 346);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(39, 26);
			this->button2->TabIndex = 2;
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button1->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->button1->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->button1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"button1.Image")));
			this->button1->Location = System::Drawing::Point(0, 278);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(39, 27);
			this->button1->TabIndex = 1;
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// panel4
			// 
			this->panel4->AutoSize = true;
			this->panel4->BackColor = System::Drawing::SystemColors::Control;
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
			this->textBox1->Location = System::Drawing::Point(3, 7);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(323, 20);
			this->textBox1->TabIndex = 6;
			this->textBox1->Text = L"c:\\Topfield\\mp3";
			// 
			// label1
			// 
			this->label1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(230)), static_cast<System::Int32>(static_cast<System::Byte>(230)), 
				static_cast<System::Int32>(static_cast<System::Byte>(230)));
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
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->toolStripButton1, 
				this->toolStripButton2, this->toolStripButton3, this->toolStripButton4});
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
			// 
			// listView2
			// 
			this->listView2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->listView2->FullRowSelect = true;
			this->listView2->GridLines = true;
			this->listView2->HideSelection = false;
			this->listView2->Location = System::Drawing::Point(3, 93);
			this->listView2->Name = L"listView2";
			this->listView2->Size = System::Drawing::Size(323, 545);
			this->listView2->TabIndex = 2;
			this->listView2->UseCompatibleStateImageBehavior = false;
			this->listView2->View = System::Windows::Forms::View::Details;
			this->listView2->ItemActivate += gcnew System::EventHandler(this, &Form1::listView2_ItemActivate);
			this->listView2->AfterLabelEdit += gcnew System::Windows::Forms::LabelEditEventHandler(this, &Form1::listView_AfterLabelEdit);
			this->listView2->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listView2_SelectedIndexChanged_1);
			this->listView2->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::listView2_Layout);
			this->listView2->ColumnClick += gcnew System::Windows::Forms::ColumnClickEventHandler(this, &Form1::listView_ColumnClick);
			this->listView2->ItemSelectionChanged += gcnew System::Windows::Forms::ListViewItemSelectionChangedEventHandler(this, &Form1::listView2_ItemSelectionChanged);
			this->listView2->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::listView_KeyDown);
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 2000;
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
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"Form1";
			this->Text = L"Antares  0.4";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::Form1_Layout);
			this->Resize += gcnew System::EventHandler(this, &Form1::Form1_Resize);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
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
	private: System::Void listView1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void listView2_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
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


                this->CheckConnection();

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

	private: System::Void Form1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
				// int avg = (this->panel3->Width + this->panel4->Width)/2;
				  int avg = (this->Width - this->panel2->Width)/2;
				 //this->listView1->Width=avg;
				 this->panel4->Width=avg;
				 printf("Layout\n");
			 }

			 private: System::Void Form1_Resize(System::Object^  sender, System::EventArgs^  e) {
						  //int avg = (this->panel3->Width + this->panel4->Width)/2;
                         int avg = (this->Width - this->panel2->Width)/2;
						  //this->listView1->Width=avg;
						  this->panel4->Width=avg;
						  printf("Resize!\n");
					  }

	private: System::Void listView1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
				 //double tot = this->listView1->Width;
				 double tot = listView1->ClientSize.Width;
				 static const double widths0[] = {140, 60, 50, 120};
				 double tot0 = widths0[0]+widths0[1]+widths0[2]+widths0[3];
				 if (this->finished_constructing==1)
				 {
					 topfieldNameHeader->Width = (int) (widths0[0]/tot0 * tot);
					 topfieldSizeHeader->Width = (int) (widths0[1]/tot0 * tot)-1;
					 topfieldTypeHeader->Width = (int) (widths0[2]/tot0 * tot)-1;
					 topfieldDateHeader->Width = (int) (widths0[3]/tot0 * tot)-1;
				 }
				 printf("ListView1 layout\n");
			 }

	private: System::Void listView2_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {



				 //double tot = this->listView2->Width;
				 double tot = listView2->ClientSize.Width;
				 static const double widths0[] = {140, 60, 50, 120};
				 double tot0 = widths0[0]+widths0[1]+widths0[2]+widths0[3];
				 if (this->finished_constructing==1)
				 {
					 computerNameHeader->Width = (int) (widths0[0]/tot0 * tot);
					 computerSizeHeader->Width = (int) (widths0[1]/tot0 * tot)-1;
					 computerTypeHeader->Width = (int) (widths0[2]/tot0 * tot)-1;
					 computerDateHeader->Width = (int) (widths0[3]/tot0 * tot)-1;
				 }
                 printf("ListView2 layout\n");
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





			 };
	private: System::Void toolStripButton1_Click_1(System::Object^  sender, System::EventArgs^  e) {
				 this->computerUpDir();
			 }
	private: System::Void toolStripButton2_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->loadComputerDir();

			 }

	private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {    
				 // Copy files from Topfield to computer


				 // Enumerate selected source items (PVR)

				 ListView^ listview = this->listView1;

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 TopfieldItem^ item;

				 int numfiles =0;
				 long long totalsize = 0;

				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<TopfieldItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) {continue;}   // Don't support whole directories yet
					 numfiles++;
					 totalsize += item->size;
				 }

				 if (this->checkBox1->Checked)
					 this->set_turbo_mode( 1); //TODO: error handling for turbo mode selection
				 else
					 this->set_turbo_mode( 0);

				 CopyDialog^ copydialog = gcnew CopyDialog();
				 copydialog->cancelled=false;
				 copydialog->showCopyDialog();
				 copydialog->total_filesize = totalsize;
				 copydialog->total_start_time = time(NULL);
				 copydialog->current_filesize = 0; 
				 copydialog->current_offset=0; 
				 copydialog->window_title="Copying File(s) ... [PVR --> PC]";
				 copydialog->current_file="Waiting for PVR...";
				 copydialog->turbo_mode = this->turbo_mode;
				 copydialog->update_dialog_threadsafe();

				 //
				 //copydialog->label3->Text = "Waiting for PVR...";
				 this->important_thread_waiting=true;
				 this->TopfieldMutex->WaitOne();
				 this->important_thread_waiting=false;
				 //copydialog->label3->Text = "";


				 myEnum = selected->GetEnumerator();
				 long long bytes_received;
				 long long total_bytes_received=0;
				 long long bytecount;
				 time_t startTime = time(NULL);
				 tf_packet reply;
				 int r;

	

				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<TopfieldItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) {continue;}   // Don't support whole directories yet

					 bytecount=0;
					 String^ full_dest_filename = this->computerCurrentDirectory + "\\" + item->safe_filename;
					 String^ full_source_filename = item->directory + "\\" + item->filename;



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
					 try{
					 //  TODO: Further exception handling for file open?
					 dest_file = File::Open(full_dest_filename,System::IO::FileMode::Create, System::IO::FileAccess::Write,System::IO::FileShare::Read);
					 }
					 catch(System::UnauthorizedAccessException ^E)
					 {
                         copydialog->close_request_threadsafe();
						 MessageBox::Show(this,"Antares cannot save the file to the location you chose. Please select another location and try again.","Write permission denied",MessageBoxButtons::OK);
                         goto end_copy_to_pc;				
					 }
					 //dst = _open(dstPath, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY);
					 //if(dst < 0)
					 //{
					 //	 fprintf(stderr, "ERROR: Can not open destination file: %s\n",
					 //		 strerror(errno));
					 //	 return errno;
					 //}

					 char* srcPath = (char*)(void*)Marshal::StringToHGlobalAnsi(full_source_filename);

					 r = send_cmd_hdd_file_send(this->fd, GET, srcPath);
					 Marshal::FreeHGlobal((System::IntPtr)(void*)srcPath);

					 if(r < 0)
					 {
						 goto out;
					 }

					 state = START;

					 bytes_received=0;
					 copydialog->current_start_time = time(NULL);
					 copydialog->current_file = full_dest_filename;
					 copydialog->current_offset=0;
					 copydialog->current_filesize = item->size;
					 copydialog->update_dialog_threadsafe();



					 while(0 < (r = get_tf_packet(fd, &reply)))
					 {

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
								 state = ABORT;
							 }
							 break;

						 case DATA_HDD_FILE_DATA:
							 if(state == DATA)
							 {
								 __u64 offset = get_u64(reply.data);
								 __u16 dataLen =
									 get_u16(&reply.length) - (PACKET_HEAD_SIZE + 8);
								 int w;

								 // if( !quiet)
								 // {
								 progressStats(bytecount, offset + dataLen, startTime);
								 // }

								 if(r < get_u16(&reply.length))
								 {
									 fprintf(stderr,
										 "ERROR: Short packet %d instead of %d\n", r,
										 get_u16(&reply.length));
									 /* TODO: Fetch the rest of the packet */
								 }

								 //w = _write(dst, &reply.data[8], dataLen);  //TODO!!

								 array <unsigned char>^ buffer = gcnew array<unsigned char>(dataLen);
								 //System::Byte buffer[] = gcnew System::Byte[dataLen];
								 Marshal::Copy( IntPtr( (void*)  &reply.data[8] ) , buffer, 0,(int) dataLen);
								 dest_file->Write(buffer, 0, dataLen);

								 bytes_received += dataLen;
								 total_bytes_received +=dataLen;
								 copydialog->total_offset = total_bytes_received;
								 copydialog->current_offset = bytes_received;
								 //copydialog->teststring = offset.ToString();
								 copydialog->update_dialog_threadsafe();
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
									 state = ABORT;

								 }
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
							 result = 0;
							 goto out;
							 break;

						 case FAIL:
							 fprintf(stderr, "ERROR: Device reports %s\n",
								 decode_error(&reply));
							 send_cancel(fd);
							 state = ABORT;
							 break;

						 case SUCCESS:
							 printf("SUCCESS\n");
							 goto out;
							 break;

						 default:
							 fprintf(stderr, "ERROR: Unhandled packet (cmd 0x%x)\n",
								 get_u32(&reply.cmd));
						 }




					 }

					 //_utime64(dstPath,(struct __utimbuf64 *) &mod_utime_buf);     
					 finalStats(bytecount, startTime);

out:
					 //_close(dst);
					 dest_file->Close();
					 File::SetCreationTime(full_dest_filename, item->datetime);
					 File::SetLastWriteTime(full_dest_filename, item->datetime);
					 if (state==ABORT) break;
					 //return result;

					 /// End of section adapted from commands.c [wuppy]a


					  if (copydialog->turbo_request != *this->turbo_mode)
					 {
						 this->set_turbo_mode( copydialog->turbo_request ? 1:0);
					 }



				 }
end_copy_to_pc:
				 this->loadComputerDir();
				 this->absorb_late_packets(2,200);
				 this->set_turbo_mode(0);
				 if (!copydialog->is_closed)
					 copydialog->close_request_threadsafe();

			 }
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

				 // Copy files from Computer to Topfield

				 int result = -EPROTO;
				 //time_t startTime = time(NULL);
				 enum
				 {
					 START,
					 DATA,
					 END,
					 FINISHED
				 } state;
				 int src = -1;
				 //int r;
				 int update = 0;

				 //trace(4, fprintf(stderr, "%s\n", __FUNCTION__));




				 // Enumerate selected source items (Computer)

				 ListView^ listview = this->listView2;

				 ListView::SelectedListViewItemCollection^ selected = listview->SelectedItems;

				 System::Collections::IEnumerator^ myEnum = selected->GetEnumerator();
				 ComputerItem^ item;

				 int numfiles =0;
				 long long totalsize = 0;

				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<ComputerItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) {continue;}   // Don't support whole directories yet
					 numfiles++;
					 totalsize += item->size;
				 }


				 CopyDialog^ copydialog = gcnew CopyDialog();
				 copydialog->cancelled=false;
				 copydialog->showCopyDialog();
				 copydialog->total_filesize = totalsize;
				 copydialog->total_start_time = time(NULL);
				 copydialog->current_filesize = 0; 
				 copydialog->current_offset=0; 
				 copydialog->window_title="Copying File(s) ... [PC --> PVR]";
				 copydialog->current_file="Waiting for PVR...";
				  copydialog->turbo_mode = this->turbo_mode;
				 copydialog->update_dialog_threadsafe();
				

				 //
				 //copydialog->label3->Text = "Waiting for PVR...";
				 this->important_thread_waiting=true;
				 this->TopfieldMutex->WaitOne();
				 this->important_thread_waiting=false;
				 //copydialog->label3->Text = "";


				 myEnum = selected->GetEnumerator();
				 long long bytes_sent;
				 long long total_bytes_sent=0;
				 long long byteCount;
				 long long fileSize;
				 time_t startTime = time(NULL);
				 struct tf_packet packet;
				 struct tf_packet reply;
				 int r;
				 array<Byte>^ inp_buffer = gcnew array<unsigned char>(sizeof(packet.data));

				 if (this->checkBox1->Checked)
					 this->set_turbo_mode(1); //TODO: error handling for turbo mode selection
				 else
					 this->set_turbo_mode( 0);

				 while ( myEnum->MoveNext() )
				 {
					 item = safe_cast<ComputerItem^>(myEnum->Current);
					 Console::WriteLine(item->Text);
					 if (item->isdir) {continue;}   // Don't support whole directories yet

					 byteCount=0;
					 String^ full_src_filename = this->computerCurrentDirectory + "\\" + item->filename;
					 String^ full_dest_filename = this->topfieldCurrentDirectory + "\\" + item->filename;


					 // TODO:  Exception handling for file open
					 FileStream^ src_file = File::Open(full_src_filename,System::IO::FileMode::Open, System::IO::FileAccess::Read,System::IO::FileShare::Read);

					 //int bb;
					 //while( (bb = src_file->ReadByte()) !=-1 ) printf("%c",(unsigned char) bb);copydialog->close_request_threadsafe();return;

					 //Console::WriteLine(full_src_filename);return;

					 //src = _open(srcPath, _O_RDONLY | _O_BINARY);
					 //if(src < 0)
					 //{
					 //	 fprintf(stderr, "ERROR: Can not open source file: %s\n",
					 //		 strerror(errno));
					 //	 return;
					 //}

					 FileInfo^ src_file_info = gcnew FileInfo(full_src_filename);


					 //if(0 != _fstat64(src, &srcStat))
					 //{
					 //	 fprintf(stderr, "ERROR: Can not examine source file: %s\n",
					 //		 strerror(errno));
					 //	 result = errno;
					 //	 goto out;
					 //}

					 fileSize = src_file_info->Length; //srcStat.st_size;
					 if(fileSize == 0)
					 {
						 fprintf(stderr, "ERROR: Source file is empty - not transfering.\n");
						 result = -1;
						 continue;
					 }
					 char* dstPath = (char *) (void*) Marshal::StringToHGlobalAnsi(full_dest_filename);
					 r = send_cmd_hdd_file_send(this->fd, PUT, dstPath);
					 Marshal::FreeHGlobal((System::IntPtr) (void*)dstPath);
					 if(r < 0)
					 {
						 //TODO: better handling
						 goto out;
					 }
					 bytes_sent=0;

					 copydialog->current_start_time = time(NULL);
					 copydialog->current_file = full_src_filename;
					 copydialog->current_offset=0;
					 copydialog->current_filesize = fileSize;
					 copydialog->update_dialog_threadsafe();

					 state = START;
					 while(0 < get_tf_packet(this->fd, &reply))
					 {
						 update = (update + 1) % 16;
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
									 time_to_tfdt64(DateTimeToTime_T(src_file_info->LastWriteTime.ToUniversalTime()) , &tf->stamp); 
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
										 goto out;
									 }
									 state = DATA;
									 break;
								 }

							 case DATA:
								 {
									 int payloadSize = sizeof(packet.data) - 9;
									 //int w = _read(src, &packet.data[8], payloadSize);
									 int w = src_file->Read(inp_buffer, 0, payloadSize);
									 Marshal::Copy(inp_buffer,0,System::IntPtr( &packet.data[8]),w);
									 //printf("w=%d  payloadSize=%d\n",w,payloadSize);
									 //for ( int j = 0; j<w; j++) printf("%c",inp_buffer[j]); 
									 /* Detect a Topfield protcol bug and prevent the sending of packets
									 that are a multiple of 512 bytes. */
									 if((w > 4)
										 &&
										 (((((PACKET_HEAD_SIZE + 8 + w) +
										 1) & ~1) % 0x200) == 0))
									 {
										 //_lseeki64(src, -4, SEEK_CUR);
										 printf("\n -- SEEK CORRECTION ---\n");
										 src_file->Seek(-4, System::IO::SeekOrigin::Current);
										 w -= 4;
										 payloadSize -= 4;
									 }

									 put_u16(&packet.length, PACKET_HEAD_SIZE + 8 + w);
									 put_u32(&packet.cmd, DATA_HDD_FILE_DATA);
									 put_u64(packet.data, byteCount);
									 byteCount += w;
									 bytes_sent+=w;
									 total_bytes_sent+=w;

									 copydialog->total_offset = total_bytes_sent;
									 copydialog->current_offset = bytes_sent;
									 copydialog->update_dialog_threadsafe();


									 if (copydialog->cancelled == true)
									 {
										 printf("CANCELLING\n");
										 //send_cancel(fd);
										 state = END;
									 }
									 /* Detect EOF and transition to END */
									 if((w < 0) || (byteCount >= fileSize))
									 {
										 printf("\nEOF conditition. w=%d  byteCount=%f  fileSize=%f\n",w,(double) byteCount,(double) fileSize);
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
											 goto out;
										 }
									 }

									 if(!update && !quiet)
									 {
										 progressStats(fileSize, byteCount, startTime);
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
									 goto out;
								 }
								 state = FINISHED;
								 break;

							 case FINISHED:
								 result = 0;
								 goto out;
								 break;
							 }
							 break;

						 case FAIL:
							 fprintf(stderr, "ERROR: Device reports %s\n",
								 decode_error(&reply));
							 goto out;
							 break;

						 default:
							 fprintf(stderr, "ERROR: Unhandled packet (copy PVR -> PC)\n");
							 break;
						 }
					 }
					 finalStats(byteCount, startTime);

out:
					 //_close(src);
					 //return result;
					 src_file->Close();

					 if (copydialog->cancelled==true) break;
					 //end section derived from commands.c

					 if (copydialog->turbo_request != *this->turbo_mode)
					 {
						 this->set_turbo_mode( copydialog->turbo_request ? 1:0);
					 }

				 }

				 this->loadTopfieldDir();
				 this->absorb_late_packets(2,200);
				 this->set_turbo_mode(0);
				 copydialog->close_request_threadsafe();

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
						 this->changeSetting(type+"_SortOrder","Descending");
					 }
					 else
					 {
						 listview->Sorting = SortOrder::Ascending;
						 this->changeSetting(type+"_SortOrder","Ascending");
					 }
					 printf("%d\n",listview->Sorting);

				 }
				 else
				 {
					 listview->Sorting = SortOrder::Ascending;
					 this->changeSetting(type+"_SortOrder","Ascending");
				 }
				 *sortcolumn = e->Column;
				 this->changeSetting(type+"_SortColumn", e->Column.ToString());

				 listview->ListViewItemSorter = gcnew ListViewItemComparer(e->Column,listview->Sorting);

				 listview->Sort();
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

				 
			
				 this->important_thread_waiting=true;
				 this->TopfieldMutex->WaitOne();
				 this->important_thread_waiting=false;
			


				 myEnum = selected->GetEnumerator();
				 long long bytes_received;
				 long long total_bytes_received=0;
				 long long bytecount;
				 time_t startTime = time(NULL);
				 tf_packet reply;
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
						 String^ new_full_filename = item->directory + "\\" + e->Label;
                         char* new_path = (char*)(void*)Marshal::StringToHGlobalAnsi(new_full_filename);
 					     int r = do_hdd_rename(this->fd, old_path,new_path);

					     Marshal::FreeHGlobal((System::IntPtr)(void*)old_path);
						 Marshal::FreeHGlobal((System::IntPtr)(void*)new_path);

						 if (r==0) // success
						 {

						 }
						 else
						 {
                             this->loadTopfieldDir();
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
			 
			 if (e->KeyCode == Keys::Delete)
			 {
				 Console::WriteLine("Delete pressed!");
                 if (selected->Count >0)
				 {
					 if (listview == this->listView1)
					 {
						 toolStripButton7_Click(nullptr,nullptr);
					 }
				 }
			 }

			 // Console::WriteLine(keystr);
			 //Console::WriteLine(e->KeyData);

			 //Console::WriteLine(e->KeyValue);

		 }

private: System::Void toolStripButton8_Click(System::Object^  sender, System::EventArgs^  e) {
             
             // Clicked "New Folder", Topfield.
			 if (this->fd==NULL)
			 {
				 this->toolStripStatusLabel1->Text="Topfield not connected.";
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

				 char* path = (char*)(void*)Marshal::StringToHGlobalAnsi(dir);


				 r = do_hdd_mkdir(this->fd,path);
				 Marshal::FreeHGlobal((System::IntPtr)(void*)path);
				 if (r!=0) this->toolStripStatusLabel1->Text="Error creating new folder.";
				 success=true;
				 break;
			 }
			 if (success)
			 this->loadTopfieldDir(foldername);
		 }

private: System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 if (this->checkBox1->Checked)
			      this->changeSetting("TurboMode","on");
			 else
				 this->changeSetting("TurboMode","off");
		 }
private: System::Void listView2_ItemSelectionChanged(System::Object^  sender, System::Windows::Forms::ListViewItemSelectionChangedEventArgs^  e) {
			 printf("ListView2 Item Selection Changed.\n");
		 }
private: System::Void listView2_SelectedIndexChanged_1(System::Object^  sender, System::EventArgs^  e) {
			 printf("ListView2 Selected Index Changed\n");
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
};
};

