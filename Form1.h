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
			this->fd  = connect_device2(&reason);
			if (this->fd==NULL) this->label2->Text="Device not connected.";

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
			Console::WriteLine("I wonder what's wrong?");
			try{
				int sort_PC=0;  int sort_PVR=0;
				this->config =  ConfigurationManager::OpenExeConfiguration( ConfigurationUserLevel::None);
				this->settings=config->AppSettings->Settings;
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

			this->finished_constructing = 1;

			this->loadTopfieldDir();
			this->loadComputerDir();

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

			this->listView2->Items->Clear();


			String^ dir = this->computerCurrentDirectory;

			if (dir->Equals(""))  // List drives
			{
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
				for (j=0; j<list->Length; j++)
				{

					item = gcnew ComputerItem(list[j]);
					item->directory = dir;
					this->listView2->Items->Add(item);
				}
                this->changeSetting("ComputerDir",dir);
				// Add a drive summary to label1:
				String^ str = Path::GetPathRoot(dir);
				if (str->Length > 0)
				{
					DriveInfo^ drive = gcnew DriveInfo(str);
					str = "[Local Disk "+str+"] " + HumanReadableSize(drive->AvailableFreeSpace) + " Free / " + HumanReadableSize(drive->TotalSize)+ " Total";
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

			this->important_thread_waiting=true;
			this->TopfieldMutex->WaitOne();
			this->important_thread_waiting=false;

			this->listView1->Items->Clear();
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

			this->label2->Text = "[Topfield device] "+HumanReadableSize(1024* ((__u64) freek))+" free / " + HumanReadableSize(1024*( (__u64) totalk)) + " total";






			toolStripStatusLabel1->Text="Beginning dir "+this->dircount.ToString();

			char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(this->topfieldCurrentDirectory);
			//printf(str2);
			send_cmd_hdd_dir(this->fd,str2);
			Marshal::FreeHGlobal((System::IntPtr)(void*)str2);
            TopfieldItem^ rename_item;bool do_rename=false;
			j=0;
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



					for(i = 0; (i < count); i++)
					{


						item = gcnew TopfieldItem(&entries[i]);
						item->directory = this->topfieldCurrentDirectory;
						if (String::Compare(item->filename,"..")!=0 ) 
						{
							this->listView1->Items->Add(item);
							if (String::Compare(start_rename,item->filename)==0)
							{
								rename_item=item; do_rename=true;
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















	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::Panel^  panel3;
	private: System::Windows::Forms::CheckBox^  checkBox1;
	private: System::Windows::Forms::TextBox^  textBox2;
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
	private: System::Windows::Forms::TextBox^  textBox1;
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
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->toolStrip2 = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripButton5 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton6 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton7 = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton8 = (gcnew System::Windows::Forms::ToolStripButton());
			this->listView1 = (gcnew System::Windows::Forms::ListView());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->panel4 = (gcnew System::Windows::Forms::Panel());
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
			this->toolStrip2->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel4->SuspendLayout();
			this->toolStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->toolStripStatusLabel1});
			this->statusStrip1->Location = System::Drawing::Point(0, 638);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(868, 22);
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
			this->panel1->Size = System::Drawing::Size(868, 638);
			this->panel1->TabIndex = 10;
			// 
			// panel3
			// 
			this->panel3->Controls->Add(this->checkBox1);
			this->panel3->Controls->Add(this->textBox2);
			this->panel3->Controls->Add(this->label2);
			this->panel3->Controls->Add(this->toolStrip2);
			this->panel3->Controls->Add(this->listView1);
			this->panel3->Dock = System::Windows::Forms::DockStyle::Fill;
			this->panel3->Location = System::Drawing::Point(0, 0);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(496, 638);
			this->panel3->TabIndex = 8;
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->Location = System::Drawing::Point(12, 81);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 7;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = true;
			// 
			// textBox2
			// 
			this->textBox2->Dock = System::Windows::Forms::DockStyle::Top;
			this->textBox2->Location = System::Drawing::Point(0, 51);
			this->textBox2->Name = L"textBox2";
			this->textBox2->Size = System::Drawing::Size(496, 20);
			this->textBox2->TabIndex = 6;
			// 
			// label2
			// 
			this->label2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label2->Dock = System::Windows::Forms::DockStyle::Top;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(0, 25);
			this->label2->Margin = System::Windows::Forms::Padding(5);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(496, 26);
			this->label2->TabIndex = 5;
			this->label2->Text = L"label2";
			this->label2->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStrip2
			// 
			this->toolStrip2->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->toolStripButton5, 
				this->toolStripButton6, this->toolStripButton7, this->toolStripButton8});
			this->toolStrip2->Location = System::Drawing::Point(0, 0);
			this->toolStrip2->Name = L"toolStrip2";
			this->toolStrip2->Size = System::Drawing::Size(496, 25);
			this->toolStrip2->TabIndex = 4;
			this->toolStrip2->Text = L"toolStrip2";
			// 
			// toolStripButton5
			// 
			this->toolStripButton5->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton5->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton5.Image")));
			this->toolStripButton5->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton5->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton5->Name = L"toolStripButton5";
			this->toolStripButton5->Size = System::Drawing::Size(23, 22);
			this->toolStripButton5->Text = L"High Folder";
			this->toolStripButton5->Click += gcnew System::EventHandler(this, &Form1::toolStripButton5_Click);
			// 
			// toolStripButton6
			// 
			this->toolStripButton6->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton6->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton6.Image")));
			this->toolStripButton6->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton6->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton6->Name = L"toolStripButton6";
			this->toolStripButton6->Size = System::Drawing::Size(23, 22);
			this->toolStripButton6->Text = L"Refresh";
			this->toolStripButton6->Click += gcnew System::EventHandler(this, &Form1::toolStripButton6_Click);
			// 
			// toolStripButton7
			// 
			this->toolStripButton7->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton7->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton7.Image")));
			this->toolStripButton7->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton7->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton7->Name = L"toolStripButton7";
			this->toolStripButton7->Size = System::Drawing::Size(23, 22);
			this->toolStripButton7->Text = L"Delete";
			this->toolStripButton7->Click += gcnew System::EventHandler(this, &Form1::toolStripButton7_Click);
			// 
			// toolStripButton8
			// 
			this->toolStripButton8->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton8->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton8.Image")));
			this->toolStripButton8->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton8->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton8->Name = L"toolStripButton8";
			this->toolStripButton8->Size = System::Drawing::Size(23, 22);
			this->toolStripButton8->Text = L"New Folder";
			this->toolStripButton8->Click += gcnew System::EventHandler(this, &Form1::toolStripButton8_Click);
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
			this->listView1->Location = System::Drawing::Point(12, 104);
			this->listView1->Name = L"listView1";
			this->listView1->Size = System::Drawing::Size(478, 520);
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
			this->panel2->BackColor = System::Drawing::SystemColors::Control;
			this->panel2->Controls->Add(this->button2);
			this->panel2->Controls->Add(this->button1);
			this->panel2->Dock = System::Windows::Forms::DockStyle::Right;
			this->panel2->Location = System::Drawing::Point(496, 0);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(54, 638);
			this->panel2->TabIndex = 7;
			// 
			// button2
			// 
			this->button2->Anchor = System::Windows::Forms::AnchorStyles::Left;
			this->button2->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(192)), 
				static_cast<System::Int32>(static_cast<System::Byte>(0)));
			this->button2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"button2.Image")));
			this->button2->Location = System::Drawing::Point(9, 348);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(39, 41);
			this->button2->TabIndex = 2;
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button1
			// 
			this->button1->Anchor = System::Windows::Forms::AnchorStyles::Left;
			this->button1->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(192)), 
				static_cast<System::Int32>(static_cast<System::Byte>(0)));
			this->button1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"button1.Image")));
			this->button1->Location = System::Drawing::Point(9, 301);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(39, 41);
			this->button1->TabIndex = 1;
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// panel4
			// 
			this->panel4->AutoSize = true;
			this->panel4->BackColor = System::Drawing::SystemColors::Control;
			this->panel4->Controls->Add(this->textBox1);
			this->panel4->Controls->Add(this->label1);
			this->panel4->Controls->Add(this->toolStrip1);
			this->panel4->Controls->Add(this->listView2);
			this->panel4->Dock = System::Windows::Forms::DockStyle::Right;
			this->panel4->Location = System::Drawing::Point(550, 0);
			this->panel4->Name = L"panel4";
			this->panel4->Size = System::Drawing::Size(318, 638);
			this->panel4->TabIndex = 6;
			// 
			// textBox1
			// 
			this->textBox1->Dock = System::Windows::Forms::DockStyle::Top;
			this->textBox1->Location = System::Drawing::Point(0, 51);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(318, 20);
			this->textBox1->TabIndex = 5;
			// 
			// label1
			// 
			this->label1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->label1->Dock = System::Windows::Forms::DockStyle::Top;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(0, 25);
			this->label1->Margin = System::Windows::Forms::Padding(5);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(318, 26);
			this->label1->TabIndex = 4;
			this->label1->Text = L"label1";
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStrip1
			// 
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->toolStripButton1, 
				this->toolStripButton2, this->toolStripButton3, this->toolStripButton4});
			this->toolStrip1->Location = System::Drawing::Point(0, 0);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->Size = System::Drawing::Size(318, 25);
			this->toolStrip1->TabIndex = 3;
			this->toolStrip1->Text = L"toolStrip1";
			// 
			// toolStripButton1
			// 
			this->toolStripButton1->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton1.Image")));
			this->toolStripButton1->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton1->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton1->Name = L"toolStripButton1";
			this->toolStripButton1->Size = System::Drawing::Size(23, 22);
			this->toolStripButton1->Text = L"High Folder";
			this->toolStripButton1->Click += gcnew System::EventHandler(this, &Form1::toolStripButton1_Click_1);
			// 
			// toolStripButton2
			// 
			this->toolStripButton2->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton2.Image")));
			this->toolStripButton2->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton2->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton2->Name = L"toolStripButton2";
			this->toolStripButton2->Size = System::Drawing::Size(23, 22);
			this->toolStripButton2->Text = L"Refresh";
			this->toolStripButton2->Click += gcnew System::EventHandler(this, &Form1::toolStripButton2_Click);
			// 
			// toolStripButton3
			// 
			this->toolStripButton3->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton3->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton3.Image")));
			this->toolStripButton3->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton3->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton3->Name = L"toolStripButton3";
			this->toolStripButton3->Size = System::Drawing::Size(23, 22);
			this->toolStripButton3->Text = L"Delete";
			// 
			// toolStripButton4
			// 
			this->toolStripButton4->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton4->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton4.Image")));
			this->toolStripButton4->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton4->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton4->Name = L"toolStripButton4";
			this->toolStripButton4->Size = System::Drawing::Size(23, 22);
			this->toolStripButton4->Text = L"New Folder";
			// 
			// listView2
			// 
			this->listView2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->listView2->FullRowSelect = true;
			this->listView2->GridLines = true;
			this->listView2->HideSelection = false;
			this->listView2->Location = System::Drawing::Point(6, 102);
			this->listView2->Name = L"listView2";
			this->listView2->Size = System::Drawing::Size(300, 522);
			this->listView2->TabIndex = 2;
			this->listView2->UseCompatibleStateImageBehavior = false;
			this->listView2->View = System::Windows::Forms::View::Details;
			this->listView2->ItemActivate += gcnew System::EventHandler(this, &Form1::listView2_ItemActivate);
			this->listView2->AfterLabelEdit += gcnew System::Windows::Forms::LabelEditEventHandler(this, &Form1::listView_AfterLabelEdit);
			this->listView2->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::listView2_Layout);
			this->listView2->ColumnClick += gcnew System::Windows::Forms::ColumnClickEventHandler(this, &Form1::listView_ColumnClick);
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
			this->ClientSize = System::Drawing::Size(868, 660);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->statusStrip1);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"Form1";
			this->Text = L"Antares  0.3";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &Form1::Form1_Layout);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->panel3->ResumeLayout(false);
			this->panel3->PerformLayout();
			this->toolStrip2->ResumeLayout(false);
			this->toolStrip2->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel4->ResumeLayout(false);
			this->panel4->PerformLayout();
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
				 libusb_device *device;
				 int conf,r,bus,address;
				 if (this->fd==NULL)
				 {
					 toolStripStatusLabel1->Text="NULL fd";
				 }
				 else
				 {
					 device=libusb_get_device(this->fd);
					 if (device==NULL)
					 {
						 toolStripStatusLabel1->Text="NULL device";
					 }
					 else
					 {
						 r = libusb_get_configuration(this->fd, &conf);
						 if (r!=0)
						 {
							 toolStripStatusLabel1->Text="Error "+r.ToString();
						 }
						 else
						 {
							 this->dircount++;
							 bus = libusb_get_bus_number(device);
							 address = libusb_get_device_address(device);
							 toolStripStatusLabel1->Text="OK "+this->dircount.ToString()+ "  " + "Bus "+bus.ToString() + " address " + address.ToString();
						 }

					 }
				 }

			 }
	private: System::Void statusStrip1_ItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
			 }

	private: System::Void Form1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
				 int avg = (this->panel3->Width + this->panel4->Width)/2;
				 //this->listView1->Width=avg;
				 this->panel4->Width=avg;
				 //printf("Layout\n");
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
};
};

