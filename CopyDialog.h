#pragma once

extern "C"
{
#include <stdio.h>
#include <time.h>
#include <math.h>

}
#include "antares.h"
#include "settings.h"
#include "commandline.h"




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
	using namespace System::Runtime::InteropServices;

	delegate void UpdateDialogCallback(void);
	delegate void CloseRequestCallback(void);




	/// <summary>
	/// Summary for CopyDialog
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class CopyDialog : public System::Windows::Forms::Form
	{
	public:
		CopyDialog(void)
		{
			InitializeComponent();
		


			this->cancelled=false;
			this->completed=false;
			this->is_closed=false;
			this->label3->Text = "";
			this->label4->Text = "0.0 MB";

			this->label6->Text = "--:--:--";
			this->label5->Text = "0.0 MB";
			this->label7->Text = "--:--:--";
			this->label8->Text = "";
			this->label10->Text = "";

			this->current_file="";
			this->last_current_delta=0;
			this->has_initialised=false;

			this->rate_stopwatch = gcnew System::Diagnostics::Stopwatch();
			this->rate_bytes = 0;
			this->rate_seconds = 0;
			this->parent_win = nullptr;
			this->parent_form = nullptr;
			this->settings=nullptr;
			this->parent_panel1=nullptr;

			this->on_completion=0;

			this->usb_error=false;
			this->file_error="";
			this->loaded = false;
			this->maximum_successful_index=-1;
			this->copydirection = CopyDirection::UNDEFINED;
			this->copymode = CopyMode::UNDEFINED;
			this->current_error = "";
			this->freespace_check_needed=false;
			this->close_requested=false;
			this->thread  = nullptr;
			this->parent_checkbox = nullptr;
			this->error_last_time=false;

			this->num_between_files=0;
			this->time_between_files=0;
			this->bytes_between_files=0;
			//this->comboBox1->SelectedIndex = 0;

			this->max_success_index=-1;

			this->comboBox1->Items->Clear();
			for each (String^ str in OnCompletionAction::option_strings)
				this->comboBox1->Items->Add(str);
			this->comboBox1->SelectedIndex=0;
			this->size_type=-1;
			this->is_resizing=false;
			this->resizing_was_docked=false;

			this->apply_language();


		}


		void apply_language(void)
		{
			this->button1->Text = lang::b_cancel;
			this->checkBox1->Text = lang::tb_turbo_mode;
			this->label9->Text = lang::c_completion;
			this->label9->Update();

			System::Drawing::Point point = this->label9->Location;
			point.X = this->Width - this->label9->Width-28;
			this->label9->Location = point;


		}

		void success(int i)
		{
			this->max_success_index=i;
			if (i==numfiles-1)
			{
				this->completed=true;
			}
		}

		void file_began(void)
		{

			this->file_began_time = (double) this->rate_stopwatch->GetTimestamp() / (double) this->rate_stopwatch->Frequency;
			this->no_packets_since_file_began=true;
		}

		double get_avg_time_between_files(void)
		{
			double a;
			if (this->num_between_files==0)  // No measurements, so return realistic defaults
			{

				if ( *this->turbo_mode)
				{
					if (this->copydirection == CopyDirection::PC_TO_PVR)
						a=.33;
					else
						a=.11;
				}
				else
				{
					if (this->copydirection == CopyDirection::PC_TO_PVR)
						a=.53;
					else
						a=.13;
				}
			}
			else
			{

				a= this->time_between_files / (double) this->num_between_files;

			}

			//printf("avg = %f\n",a);
			return a;
		}

		void new_packet(long long bytes)
		{

			long long ts = this->rate_stopwatch->GetTimestamp();
			if (this->no_packets_since_file_began)
			{ 
				double delta = (double) (ts -  this->last_packet_ticks) / (double) this->rate_stopwatch->Frequency ;

				if (delta < 2.0 && delta >=0.0)
				{

					this->num_between_files++;
					this->time_between_files+=delta;
					this->bytes_between_files+=bytes;
				}

			}


			if ( ! (this->no_packets_since_file_began || this->no_packets_since_last_reset))
			{



				double delta = (double) (ts-this->last_packet_ticks)  / (double) this->rate_stopwatch->Frequency;

				if (delta < 2.0 && delta >= 0.0)
				{
					this->rate_bytes+=bytes;
					this->rate_seconds += delta;
					//printf("# %lld:  %d  %f\n", this->current_offsets[this->current_index],(int) bytes, delta);
				}


			}
			this->last_packet_ticks = ts;

			this->no_packets_since_file_began=false;
			this->no_packets_since_last_reset=false;


			//printf("bytes = %lld  milliseconds=%lld  \n",this->rate_bytes, this->rate_milliseconds);
		}
		void reset_rate(void)
		{
			//this->rate_stopwatch->Reset();
			this->rate_bytes=0;
			this->rate_seconds=0;
			this->time_between_files=0;
			this->num_between_files=0;
			this->last_packet_ticks=0;
			this->no_packets_since_last_reset=true;
			this->bytes_between_files=0;
		}
		double get_rate(void)
		{
			double ret;
			if (this->rate_seconds == 0) // choose a reasonable default if we have not made a measurement
			{
				if ( *this->turbo_mode)
				{
					if (this->copydirection == CopyDirection::PC_TO_PVR)
						ret=2.30 * 1024.0*1024.0;
					else
						ret=3.33 * 1024.0*1024.0;
				}
				else
				{
					if (this->copydirection == CopyDirection::PC_TO_PVR)
						ret=1.02 * 1024.0*1024.0;
					else
						ret=1.33 * 1024.0*1024.0;
				}
			}
			else
			{
				ret=(double) this->rate_bytes / (double) this->rate_seconds ;
			}

			return ret;

		}


		void showDialog_thread(void)
		{
			if (this->TopLevel)
			{
				this->Visible=false;

				if(this->parent_win == nullptr)
					this->ShowDialog();
				else
					this->ShowDialog(this->parent_win);
			}
		}

		void showCopyDialog(void)
		{

			// Hmm... I think this code can be thrown out
			ThreadStart^ threadDelegate = gcnew ThreadStart( this, &CopyDialog::showDialog_thread);
			Thread^ newThread = gcnew Thread( threadDelegate );
			newThread->Start();

		}

		static String^ time_remaining_string(double rate, double bytes, double total_files_remaining, double avg_time_between_files)
		{
			String^ str;
			str="--:--:--";
			if (rate==0) return(str);
			double t = bytes/rate;

			t += avg_time_between_files*total_files_remaining;


			//printf("t=%f   total_files_remaining=%f   avg=%f \n",t,total_files_remaining, avg_time_between_files);

			if (t > 99*60*60 || t<0) return(str);
			int ti = (int) t;
			int sec = ti % 60;
			ti = ti/60;
			int min = ti % 60;
			ti=ti/60;
			str = ti.ToString("d2") + ":" + min.ToString("d2") + ":" + sec.ToString("d2");
			return str;

		}


		void close_request_threadsafe(void)
		{
			this->is_closed=true;
			if (this->InvokeRequired)
			{
				CloseRequestCallback^ d = gcnew CloseRequestCallback(this, &CopyDialog::close_request_threadsafe);
				try{
					this->BeginInvoke(d);
				}
				catch (...)     // (in case the window has been closed)
				{
				}
			}
			else
			{
				this->close_requested=true;
				//printf("Actually called close.\n");
				this->Close();
			}
		}

		void set_error(String^ str)
		{
			this->current_error = str;
		}

		void clear_error()
		{
			this->current_error="";
		}


		void update_dialog_threadsafe(void)
		{

			if (this->cancelled) return;

			if (this->TopLevel)
			{
				while (!this->IsHandleCreated) 				
				{
					Thread::Sleep(100);
				}
			}

			if (this->InvokeRequired)
			{
				UpdateDialogCallback^ d = 
					gcnew UpdateDialogCallback(this, &CopyDialog::update_dialog);
				if (this->TopLevel)
					this->BeginInvoke(d, gcnew array<Object^> { });
				else
					this->parent_form->BeginInvoke(d, gcnew array<Object^> { });
			}
			else 
				this->update_dialog();
		}


		void tiny_size(void)    //visual arrangement for "finding files" stage
		{
			this->SuspendLayout();


			String ^wt = this->copymode == CopyMode::MOVE ? lang::c_title1_move : lang::c_title1_copy;
			wt = wt + "     ";
			wt = wt + (this->copydirection == CopyDirection::PC_TO_PVR ? lang::c_title2_to_pvr : lang::c_title2_to_pc    );
			this->Text = wt;


			this->label3->Text=lang::c_finding;//"Finding files...";



			this->progressBar1->Visible=false;
			this->progressBar2->Visible=false;

			this->label2->Visible=false;
			this->label5->Visible=false;
			this->label7->Visible=false;

			this->label1->Visible=false;
			this->label4->Visible=false;
			this->label6->Visible=false;

			this->button1->Visible=false;
			this->checkBox1->Visible=false;

			this->label9->Visible=false;
			this->comboBox1->Visible=false;

			int H=100;
			this->Size = System::Drawing::Size(this->proper_width, H);
			this->MaximumSize = System::Drawing::Size(this->proper_width, H);
			this->MinimumSize = System::Drawing::Size(this->proper_width, H);
			this->ResumeLayout();
			this->size_type = 0;

		}


		void small_size(void)   // visual arrangement for transferring one file.
		{

			this->SuspendLayout();
			this->label2->Visible=false;
			this->label5->Visible=false;
			this->label7->Visible=false;

			this->label1->Visible=true;
			this->label4->Visible=true;
			this->label6->Visible=true;

			this->button1->Visible=true;
			this->checkBox1->Visible=true;

			this->progressBar2->Visible=false;

			this->progressBar1->Visible=true;

			this->label9->Visible=true;
			this->comboBox1->Visible=true;
			int H = 216 - this->bottom_trim - this->bottom_trim2; 


			
			this->MaximumSize = System::Drawing::Size(2000,H);
			this->MinimumSize = System::Drawing::Size(0,H);
			this->Size = System::Drawing::Size(this->proper_width, H);


			this->ResumeLayout();
			this->size_type=1;
		}

		void normal_size(void) // visual arrangement for transferring several files
		{

			this->SuspendLayout();
			this->label2->Visible=true;
			this->label5->Visible=true;
			this->label7->Visible=true;
			this->progressBar2->Visible=true;

			this->label1->Visible=true;
			this->label4->Visible=true;
			this->label6->Visible=true;

			this->button1->Visible=true;
			this->checkBox1->Visible=true;

			this->progressBar1->Visible=true;

			this->label9->Visible=true;
			this->comboBox1->Visible=true;

			int H = 282-this->bottom_trim-this->bottom_trim2;

			
			this->MaximumSize = System::Drawing::Size(2000,H);
			this->MinimumSize = System::Drawing::Size(0,H);
			this->Size = System::Drawing::Size(this->proper_width, H);


			this->ResumeLayout();
			this->size_type=2;

		}

		void proper_size(void)
		{

			//printf("(%d,%d) (%d,%d) -- ",this->Size.Width, this->Size.Height, this->ClientSize.Width, this->ClientSize.Height);
			int W1 = this->Width;
			switch(this->size_type)
			{
			case 0:
				this->tiny_size();
				break;
			case 1:
				this->small_size();
				break;
			case 2:
				this->normal_size();
				break;
			default:
				printf("Proper size!!??\n");

			}
			int W2 = this->Width;
			if (W2!=W1)
			{
				Point p = this->Location;
				p.X +=  (W1-W2)/2;
				this->Location = p;
			}
			//printf("(%d,%d) (%d,%d) \n",this->Size.Width, this->Size.Height, this->ClientSize.Width, this->ClientSize.Height);


		}


		void update_dialog(void)
		{
			//if ( ! this->Visible) return;

		

			if (this->current_error->Length >  0)
			{
				this->label3->Text = this->current_error;
				this->label3->ForeColor = Color::FromArgb(200,0,0);
				Antares::TaskbarState::setError(this->parent_form);
				error_last_time=true;

			}
			else
			{


				if (this->size_type==0)
					this->label3->Text=lang::c_finding;//"Finding files...";
				else
					this->label3->Text = current_file;

				this->label3->ForeColor = System::Drawing::Color::DarkBlue;

				if (error_last_time)
				{
					Antares::TaskbarState::setNormal(this->parent_form);
				}

				error_last_time=false;

			}


			try{
			if (Console::KeyAvailable)
			{
				System::ConsoleKeyInfo key = Console::ReadKey(true);
				String ^kstr = "";


				if (key.Key == ConsoleKey::Escape) kstr="ESC key";
				if (key.Modifiers == ConsoleModifiers::Control && key.Key==ConsoleKey::C) kstr="CTRL-C";

				if (kstr->Length > 0)
				{
					
					Console::WriteLine(kstr+" pressed. Cancelling...");
					this->button1->Enabled=false;
					this->cancelled=true;
				}
				
			}
			}
			catch(...){}  //Todo: nicer way of dealing with an absent console.



			int i = this->current_index;
			int ind = 0;
			int indmax  =0;
		
			try{ind=this->file_indices[i]; indmax=this->file_indices[this->numfiles-1];} catch(...){};

			String ^wt;
			if (this->size_type == 0 || ind==0)
			{

				wt = this->copymode == CopyMode::MOVE ? lang::c_title1_move : lang::c_title1_copy;

			}
			else
			{
				wt = this->copymode == CopyMode::MOVE ? lang::c_title1b_move : lang::c_title1b_copy;
				wt = String::Format(wt, ind, indmax);

			}
			wt = wt + "     ";
			wt = wt + (this->copydirection == CopyDirection::PC_TO_PVR ? lang::c_title2_to_pvr : lang::c_title2_to_pc    );

			this->Text = wt;
	



			double current_delta = (double) this->rate_stopwatch->GetTimestamp() / (double) this->rate_stopwatch->Frequency;
			double current_rate;//, total_rate;
			

			long long size = this->filesizes[i];
			long long offset = this->current_offsets[i]; 

			current_rate = this->get_rate();


			if (this->has_initialised==false) {

				if (this->settings != nullptr)
				{
					this->checkBox1->Checked = this->turbo_request = (this->settings["TurboMode"] == "on");

				}
				else
					this->checkBox1->Checked = *this->turbo_mode;


				this->has_initialised=true;

			};
			//printf(" turbo_mode = %d  turbo_mode2=%d  turbo_request = %d  len=%d\n",(int) *this->turbo_mode, (int) *this->turbo_mode2, (int) this->turbo_request,this->current_error->Length);
			if (*this->turbo_mode != this->turbo_request && this->current_error->Length == 0)
			{
				this->checkBox1->Text = lang::c_turbo_changing;//"Turbo mode [Changing...]";
			}
			else if (*this->turbo_mode != *this->turbo_mode2 && this->current_error->Length == 0)

			{
				this->checkBox1->Text = lang::tb_turbo_mode + "\n" + lang::c_turbo_disabled;//(Disabled due to current recording)";
 			}

			else
			{
				this->checkBox1->Text = lang::tb_turbo_mode;//"Turbo mode";
			}

			this->label4->Text =  (offset / 1024.0/1024.0).ToString("#,#,0.0")+" "+lang::u_mb+" / "+(size/1024.0/1024.0).ToString("#,#,0")+" "+lang::u_mb;
			
			long long total_offset=0;  long long total_size=0;
			long long files_remaining=0;
			for (int j=0; j < this->numfiles; j++)
			{
				total_offset += this->current_offsets[j];
				total_size += this->filesizes[j];
				if (j>this->current_index)
				{
					if (this->overwrite_action[j] != SKIP)
						files_remaining++;
				}
			}



			this->label5->Text =  (total_offset / 1024.0 / 1024.0).ToString("#,#,0.0")+" "+lang::u_mb+" / "
				+  (total_size/1024.0 / 1024.0).ToString("#,#,0")+" "+lang::u_mb+"  "+lang::st_total;
	
			


			if (size>0)
			{
				int val1 = (int)  (  (double) this->progressBar1->Maximum * (double) offset / (double) size  );
				if (val1<= this->progressBar1->Maximum && val1>=this->progressBar1->Minimum)
					this->progressBar1->Value= val1; 
				else printf("Warning: progressBar1 out of bounds!\n");
			}

			if (this->current_error->Length > 0 )
			{
				this->label8->Text = "-- MB/sec";
				this->label6->Text = "--:--:--";
				this->label7->Text = "--:--:--";
			}

			else
			{

				if ( floor(current_delta) > floor(this->last_current_delta) && current_delta>1.0)
				{

					String^ ratestring="--";

					double current_rate2 = (double) (this->bytes_between_files + this->rate_bytes) /  (this->time_between_files + this->rate_seconds);

					//printf("%lld,  %lld,  %f,  %f  : %f\n",bytes_between_files, rate_bytes, time_between_files, rate_seconds, current_rate2);
					if (current_rate2>0.0 && current_rate2<20000000.0)
						ratestring = (current_rate2/1024.0/1024.0).ToString("F2");


					this->label8->Text = ratestring + " MB/s";


					//this->label8->Text = (current_rate/1024.0/1024.0).ToString("F2")+" MB/sec";
					current_rate = fabs(current_rate);
					//printf("size=%lld offset=%lld  total_offset=%lld  total_size=%lld  files_remaining=%lld  \n",size,offset,total_offset,total_size,files_remaining);
					this->label6->Text = time_remaining_string(current_rate,(double) (size - offset ), 0, 0 );
					this->label7->Text = time_remaining_string(current_rate, (double) ( total_size - total_offset ),(double) files_remaining , this->get_avg_time_between_files());
				}

			}


			if (total_size>0)
			{
				int val2 = (int)  ( (double) this->progressBar2->Maximum * (double) total_offset / (double) total_size  );
				Antares::TaskbarState::setValue(this->parent_form, total_offset, total_size);
				if (val2<= this->progressBar2->Maximum && val2>=this->progressBar2->Minimum)
					this->progressBar2->Value = val2;
				else printf("Warning: progressBar2 out of bounds!\n");
			}


			int perc1 = ((int)(100.00 * (double) offset / (double) size));
			int perc2 = ((int)(100.00 * (double) total_offset / (double) total_size));
			perc1 = perc1<0 ? 0 : perc1;
			perc2 = perc2<0 ? 0 : perc2;
			perc1 = perc1>100 ? 100 : perc1;
			perc2 = perc2>100 ? 100 : perc2;

			this->label1->Text =  perc1.ToString() + "%";
			this->label2->Text =  perc2.ToString() + "%";
			this->last_current_delta = current_delta;

			this->arrange_centred_labels();

		}
	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~CopyDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	public:
		bool cancelled;
		bool is_closed;
		bool turbo_request;
		bool^ turbo_mode;
		bool^ turbo_mode2;

		int on_completion;

		TransferOptions^ transferoptions;

		CommandLine^ commandline;
		double rate_seconds;
		long long rate_bytes;
		System::Diagnostics::Stopwatch ^rate_stopwatch;
		double file_began_time;
		double last_packet_time;
		bool no_packets_since_file_began;
		bool no_packets_since_last_reset;
		double time_between_files;
		int num_between_files;
		long long last_packet_ticks;
		long long bytes_between_files;

		long long current_bytes_received;
		long long total_bytes_received;
		array<long long int>^ filesizes;

		int current_index;
		int numfiles;

		time_t current_start_time, total_start_time;
		String^ current_file;
		double last_current_delta;
		bool has_initialised;
		IWin32Window ^parent_win;

		array<bool>^          dest_exists;
		array<long long int>^ dest_size;
		array<int>^           overwrite_action;
		array<int>^           overwrite_category;
		array<long long int>^ current_offsets;
		array<String^>^       dest_filename;
		array<FileItem^>^     src_items;
		array<int>^           file_indices; 
		array<bool>^          filtered_dir_has_no_files;
		array<array<TopfieldItem^>^>^ topfield_items_by_folder;

		List<MyStuffInfo^>^ info_list;

		bool usb_error; 
		String^ file_error; 
		bool loaded;
		int maximum_successful_index;

		CopyDirection copydirection;
		CopyMode copymode;
		bool action1_skipdelete;

		String^ current_error;

		bool freespace_check_needed;
		bool close_requested;
		System::Windows::Forms::Form^ parent_form;
		System::Threading::Thread^ thread;
		System::Windows::Forms::CheckBox^ parent_checkbox;
		Settings^ settings;
		Panel^ parent_panel1;
		bool error_last_time;
		bool completed;

		int size_type;

		static const int proper_width = 648;
		static const int bottom_trim = 5 + 5+5;
		static const int bottom_trim2 = 5+5+5;
		bool is_resizing;
		bool resizing_was_docked;

	private:
		int max_success_index;



	private: System::Windows::Forms::ProgressBar^  progressBar1;
	protected: 
	private: System::Windows::Forms::ProgressBar^  progressBar2;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Button^  button1;
	public: System::Windows::Forms::Label^  label3;
	private: 

	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::Label^  label8;
	public: System::Windows::Forms::CheckBox^  checkBox1;

	public: 
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::Label^  label9;
private: System::Windows::Forms::Label^  label10;




	private: 


	protected: 


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->progressBar2 = (gcnew System::Windows::Forms::ProgressBar());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// progressBar1
			// 
			this->progressBar1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->progressBar1->Location = System::Drawing::Point(12, 58);
			this->progressBar1->MarqueeAnimationSpeed = 0;
			this->progressBar1->Maximum = 1000;
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(644, 30);
			this->progressBar1->Step = 0;
			this->progressBar1->TabIndex = 0;
			// 
			// progressBar2
			// 
			this->progressBar2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->progressBar2->Location = System::Drawing::Point(12, 124);
			this->progressBar2->MarqueeAnimationSpeed = 0;
			this->progressBar2->Maximum = 1000;
			this->progressBar2->Name = L"progressBar2";
			this->progressBar2->Size = System::Drawing::Size(644, 30);
			this->progressBar2->TabIndex = 1;
			// 
			// label1
			// 
			this->label1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->label1->AutoSize = true;
			this->label1->BackColor = System::Drawing::SystemColors::Control;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(319, 65);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(27, 16);
			this->label1->TabIndex = 2;
			this->label1->Text = L"0%";
			this->label1->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			// 
			// label2
			// 
			this->label2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->label2->AutoSize = true;
			this->label2->BackColor = System::Drawing::SystemColors::Control;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(319, 131);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(27, 16);
			this->label2->TabIndex = 3;
			this->label2->Text = L"0%";
			this->label2->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->Location = System::Drawing::Point(270, 198);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(124, 31);
			this->button1->TabIndex = 4;
			this->button1->Text = L"Cancel";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &CopyDialog::button1_Click);
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(245)), static_cast<System::Int32>(static_cast<System::Byte>(245)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->ForeColor = System::Drawing::Color::DarkBlue;
			this->label3->Location = System::Drawing::Point(9, 9);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(464, 16);
			this->label3->TabIndex = 5;
			this->label3->Text = L"C:\\blah\\blah\\something\\blah\\a very long path\\some more\\Some TV show.rec";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label4->Location = System::Drawing::Point(9, 39);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(45, 16);
			this->label4->TabIndex = 6;
			this->label4->Text = L"label4";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label5->Location = System::Drawing::Point(8, 105);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(45, 16);
			this->label5->TabIndex = 7;
			this->label5->Text = L"label5";
			// 
			// label6
			// 
			this->label6->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->label6->AutoSize = true;
			this->label6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label6->Location = System::Drawing::Point(596, 39);
			this->label6->Margin = System::Windows::Forms::Padding(5, 0, 15, 0);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(45, 16);
			this->label6->TabIndex = 8;
			this->label6->Text = L"label6";
			// 
			// label7
			// 
			this->label7->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->label7->AutoSize = true;
			this->label7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label7->Location = System::Drawing::Point(596, 105);
			this->label7->Margin = System::Windows::Forms::Padding(5, 0, 15, 0);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(45, 16);
			this->label7->TabIndex = 9;
			this->label7->Text = L"label7";
			// 
			// label8
			// 
			this->label8->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->label8->AutoSize = true;
			this->label8->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F));
			this->label8->Location = System::Drawing::Point(611, 9);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(45, 16);
			this->label8->TabIndex = 10;
			this->label8->Text = L"label8";
			// 
			// checkBox1
			// 
			this->checkBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->checkBox1->AutoSize = true;
			this->checkBox1->Location = System::Drawing::Point(11, 211);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 11;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = true;
			this->checkBox1->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &CopyDialog::checkBox1_MouseClick);
			this->checkBox1->CheckedChanged += gcnew System::EventHandler(this, &CopyDialog::checkBox1_CheckedChanged);
			// 
			// comboBox1
			// 
			this->comboBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->comboBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->comboBox1->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Location = System::Drawing::Point(553, 209);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(103, 21);
			this->comboBox1->TabIndex = 13;
			this->comboBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &CopyDialog::comboBox1_SelectedIndexChanged);
			// 
			// label9
			// 
			this->label9->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->label9->AutoSize = true;
			this->label9->Location = System::Drawing::Point(563, 193);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(78, 13);
			this->label9->TabIndex = 14;
			this->label9->Text = L"On completion:";
			// 
			// label10
			// 
			this->label10->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->label10->AutoSize = true;
			this->label10->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label10->Location = System::Drawing::Point(310, 105);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(52, 16);
			this->label10->TabIndex = 15;
			this->label10->Text = L"label10";
			// 
			// CopyDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->ClientSize = System::Drawing::Size(664, 282);
			this->ControlBox = false;
			this->Controls->Add(this->label8);
			this->Controls->Add(this->label10);
			this->Controls->Add(this->label9);
			this->Controls->Add(this->comboBox1);
			this->Controls->Add(this->checkBox1);
			this->Controls->Add(this->label7);
			this->Controls->Add(this->label6);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->progressBar2);
			this->Controls->Add(this->progressBar1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->MinimumSize = System::Drawing::Size(16, 282);
			this->Name = L"CopyDialog";
			this->Padding = System::Windows::Forms::Padding(5);
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"CopyDialog";
			this->TransparencyKey = System::Drawing::Color::Fuchsia;
			this->Load += gcnew System::EventHandler(this, &CopyDialog::CopyDialog_Load);
			this->ResizeBegin += gcnew System::EventHandler(this, &CopyDialog::CopyDialog_ResizeBegin);
			this->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &CopyDialog::CopyDialog_Layout);
			this->Move += gcnew System::EventHandler(this, &CopyDialog::CopyDialog_Move);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &CopyDialog::CopyDialog_FormClosing);
			this->Resize += gcnew System::EventHandler(this, &CopyDialog::CopyDialog_Resize);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &CopyDialog::CopyDialog_KeyDown);
			this->ResizeEnd += gcnew System::EventHandler(this, &CopyDialog::CopyDialog_ResizeEnd);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion



	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->button1->Enabled = false;
				 this->cancelled = true;
			 }

	private: System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				// printf("CheckedChanged\n");
				 this->turbo_request  = this->checkBox1->Checked;
				 if (this->checkBox1->Checked) this->settings->changeSetting("TurboMode","on"); else this->settings->changeSetting("TurboMode","off");

				 if (this->parent_checkbox != nullptr)
					 this->parent_checkbox->Checked = this->checkBox1->Checked;

			 }
	private: System::Void CopyDialog_Load(System::Object^  sender, System::EventArgs^  e) {
				 //printf("CopyDialog loaded\n");
				 this->loaded=true;
			 }

	private: System::Void CopyDialog_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
				 if (e->KeyCode == Keys::Escape)
				 {
					 this->button1->Enabled=false;
					 this->cancelled=true;
				 }
			 }
	private: System::Void CopyDialog_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
				 //printf("Form closing, for some reason.\n");
				 this->button1->Enabled=false;
				 this->cancelled=true;
				 if (!this->close_requested)
					 e->Cancel = true;


			 }
	private: System::Void CopyDialog_Resize(System::Object^  sender, System::EventArgs^  e) {
				 if (FormWindowState::Minimized == this->WindowState)
					 this->parent_form->WindowState = FormWindowState::Minimized;
			 }
	private: System::Void comboBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
				 this->on_completion = this->comboBox1->SelectedIndex;
			 }

	private: System::Void CopyDialog_ResizeBegin(System::Object^  sender, System::EventArgs^  e) {
				// printf("CopyDialog resizebegin\n");
				 this->is_resizing=true;



				 if (this->Dock == DockStyle::Bottom || this->Dock == DockStyle::Top)
				 {
					 this->resizing_was_docked=true;
					 this->SuspendLayout();
					 if (nullptr==this->commandline || this->commandline->showgui) this->BringToFront();
					 this->Dock = DockStyle::None;
					 this->FormBorderStyle = Windows::Forms::FormBorderStyle::SizableToolWindow;
					 this->proper_size();
					 this->ResumeLayout();
					 this->proper_size();

				 }
				 else this->resizing_was_docked=false;

			 }

			 System::Void CentreInParent(int offset)
			 {

				 this->Location = System::Drawing::Point(
					 (this->parent_form->Width - this->Width)/2, offset+(this->parent_form->Height - this->Height)/2);
				 if (nullptr==this->commandline || this->commandline->showgui) this->BringToFront();

				 //this->panel1->Dock=DockStyle::Fill;
			 }


			 void arrange_centred_labels(void)
			 {

				 Point p = this->label1->Location;
				 p.X = this->Width/2 - this->label1->Width/2;// + 5;
				 this->label1->Location=p;

				
				 p = this->label2->Location;
				 p.X = this->Width/2 - this->label2->Width/2;// + 5;
				 this->label2->Location=p;

				// p = this->label8->Location;
				// p.X = this->Width/2 - this->label8->Width/2;//-5;
				// this->label8->Location=p;

				 p = this->label8->Location;
				p.X = this->Width - this->label8->Width-27;
				 this->label8->Location=p;


				 p = this->label10->Location;
				 p.X = this->Width/2 - this->label10->Width/2;//-10;
				 this->label10->Location=p;
			 }
	private: System::Void CopyDialog_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {

				 // Arrange cancel button
				 int W=124;
				 this->button1->Width=W;
				 Point p = this->button1->Location;
				 p.X = this->Width/2 - W/2;
				 p.Y = this->Height - 91 + bottom_trim;
				 this->button1->Location=p;

	
				 this->arrange_centred_labels();


				 // "on completion..."
				 p=this->comboBox1->Location;

				 int ww1 = this->comboBox1->Width;
				 int ww2 = this->label9->Width;
				 int ww = ww1>ww2 ? ww1 : ww2; 

				 p.Y = this->Height - 95 + 16    + bottom_trim - 8;
				 p.X = this->Width - ww - 24;
				 int X = p.X;
				 this->comboBox1->Location=p;

				 p=this->label9->Location;
				 p.Y = this->Height - 90 - 3   + bottom_trim - 8;
				 p.X = X;
				 this->label9->Location = p;

				 p = this->checkBox1->Location;
				 p.Y = this->Height - 85 + bottom_trim;
				 this->checkBox1->Location=p;





			 }


			 void test(void)
			 {
				 bool low = this->Location.Y + this->Height > this->parent_panel1->Height;
				 bool high = this->Location.Y < 0;
				 if (low && !high)
				 {
					 //printf("Dock low\n");

					 this->Dock = DockStyle::Bottom;
					 if (nullptr==this->commandline || this->commandline->showgui) this->parent_panel1->BringToFront();

				 }
				 else if (high && !low)
				 {
					// printf("Dock high\n");

					 this->Dock = DockStyle::Top;
					if (nullptr==this->commandline || this->commandline->showgui)  this->parent_panel1->BringToFront();

				 }
				 else
				 {
					 this->Dock = DockStyle::None;
					 if (nullptr==this->commandline || this->commandline->showgui) this->BringToFront();
					 
					 if (this->resizing_was_docked)
						 this->proper_size();

					 //this->CenterToParent();
					 
				 }
			 }

	private: System::Void CopyDialog_Move(System::Object^  sender, System::EventArgs^  e) {
				// printf ("CD move. \n");
				 if (this->is_resizing)
				 {
					 //this->test();
				 }

			 }
	private: System::Void CopyDialog_ResizeEnd(System::Object^  sender, System::EventArgs^  e) {
				 //printf ("CD resize end\n");


				 
				 this->test();

				 this->is_resizing=false;
			 }


	protected:  virtual void OnPaint(PaintEventArgs ^e) override
				{
					//baseOnPaint(e);

					//Graphics ^g = e->Graphics;
					//g->DrawString("www.java2s.com", Font, Brushes::Black, 5, 5);

				}

	protected: virtual void OnPaintBackground(PaintEventArgs  ^ e) override
			   {
				   Form::OnPaintBackground(e);


			   }




	private: System::Void checkBox1_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
				 //printf("MouseClick\n");
			 }
};
}
