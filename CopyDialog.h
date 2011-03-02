#pragma once

extern "C"
{
#include <stdio.h>
#include <time.h>
#include <math.h>

}
#include "antares.h"




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

	public enum class CopyDirection {PVR_TO_PC, PC_TO_PVR, UNDEFINED};

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
			//
			//TODO: Add the constructor code here
			//
			this->cancelled=false;
			this->is_closed=false;
			this->label3->Text = "";
			this->label4->Text = "";

			this->label6->Text = "";
			this->label5->Text = "";
			this->label7->Text = "";
			this->label8->Text = "";
			this->window_title="";
			//this->full_src_filename="";
			//this->full_dest_filename="";
			this->current_file="";
			this->last_current_delta=0;
			//this->current_offset=0; 
			//this->total_offset=0;
			this->has_initialised=false;

			this->rate_stopwatch = gcnew System::Diagnostics::Stopwatch();
			this->rate_bytes = 0;
			this->rate_milliseconds = 0;
			this->parent_win = nullptr;
			this->parent_form = nullptr;
			
			this->usb_error=false;
			this->file_error="";
			this->loaded = false;
			this->maximum_successful_index=-1;
			this->copydirection = CopyDirection::UNDEFINED;
			this->current_error = "";
			this->freespace_check_needed=false;
			this->close_requested=false;
			this->thread  = nullptr;
			this->parent_checkbox = nullptr;

		}
	
		/*
		CopyDialog(IWin32Window ^win)
		{
			CopyDialog();
			this->parent_win = win;
			//this->turbo_mode = form1->turbo_mode;

		}
		*/
	

		void new_packet(long long bytes)
		{
		   
           if ( false == this->rate_stopwatch->IsRunning)
		   {
			   this->rate_stopwatch->Reset();
			   this->rate_bytes=0;
			   this->rate_milliseconds=0;
			   this->rate_stopwatch->Start();
		   }
		   else
		   {
			   this->rate_bytes += bytes;
			   this->rate_milliseconds = this->rate_stopwatch->ElapsedMilliseconds;
		   }

		   //printf("bytes = %lld  milliseconds=%lld  \n",this->rate_bytes, this->rate_milliseconds);
		}
		void reset_rate(void)
		{
			this->rate_stopwatch->Reset();
			this->rate_bytes=0;
			this->rate_milliseconds=0;
		}
		double get_rate(void)
		{
			double ret;
			if (this->rate_milliseconds == 0) 
				ret= -1;
			else
			{
				ret=(double) this->rate_bytes / (double) this->rate_milliseconds  * 1000.0;
			}

			//printf(" rate = %f\n",ret);

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

			//System::Threading::Thread^ thread = gcnew System::Threading::Thread( gcnew System::Threading::ThreadStart( &this->copydialog->ShowDialog) );

			//thread->Start();
			//System::Threading::Thread^ thread = (gcnew System::Threading::Thread(()=> { 
			//	this->copydialog->Show(); 
			//}));
			//	Start(); 


			ThreadStart^ threadDelegate = gcnew ThreadStart( this, &CopyDialog::showDialog_thread);
			Thread^ newThread = gcnew Thread( threadDelegate );
			newThread->Start();

		}

		static String^ time_remaining_string(double rate, double bytes)
		{
			String^ str;
			str="--:--:--";
			if (rate==0) return(str);
			double t = bytes/rate;
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
					this->Invoke(d);
				}
				catch (...)     // (in case the window has been closed)
				{
				}
			}
			else
			{
				this->close_requested=true;
				printf("Actually called close.\n");
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
		
			while (!this->IsHandleCreated) 				
			{
				Thread::Sleep(100);
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

		void update_dialog(void)
		{
			if ( ! this->Visible) return;
			
			this->Text = this->window_title;

			if (this->current_error->Length >  0)
			{
				this->label3->Text = this->current_error;
				this->label3->ForeColor = Color::FromArgb(200,0,0);
				
			}
			else
			{
				this->label3->Text = current_file;
				this->label3->ForeColor = System::Drawing::SystemColors::ControlText;
			}

			
			//double current_delta = (double) (time(NULL) - this->current_start_time);
			//double total_delta = (double) (time(NULL) - this->total_start_time);
			double current_delta = (double) this->rate_milliseconds / 1000.0;
			double current_rate;//, total_rate;
			int i = this->current_index;

			long long size = this->filesizes[i];
			long long offset = this->current_offsets[i]; 

			//if (current_delta>0)
			//	current_rate = (double) (this->current_bytes_received) / current_delta;
			//else current_rate=0;

			current_rate = this->get_rate();


			//if (total_delta>0)
			//	total_rate = (double) (this->total_bytes_received) / total_delta;
			//else
			//	total_rate=0;
            
			if (this->has_initialised==false) {
				if (this->parent_checkbox !=nullptr)
					this->checkBox1->Checked = this->parent_checkbox->Checked;
				else
				this->checkBox1->Checked = *this->turbo_mode;
				
				this->has_initialised=true;

				if (this->numfiles<=1)
				{
					this->label2->Visible=false;
					this->label5->Visible=false;
					this->label7->Visible=false;
					this->progressBar2->Visible=false;
					this->Height=225;

				}

			};
			if (*this->turbo_mode != this->turbo_request && this->current_error->Length == 0)
			{
				this->checkBox1->Text = "Turbo mode [Changing...]";
			}
			else
			{
				this->checkBox1->Text = "Turbo mode";
			}

			//this->label4->Text =  (offset / 1024).ToString("#,#,#")+"KB / "+(size/1024).ToString("#,#,#")+"KB";
			long long int offset_MB = offset / 1024LL/1024LL;
			int offset_dec_MB = (offset - offset_MB * 1024LL*1024LL)*10/1024/1024;
			String^ offset_int_MB =  (offset_MB).ToString("#,#,#"); if (offset_int_MB->Length==0) offset_int_MB="0";
			this->label4->Text = offset_int_MB+"."+offset_dec_MB.ToString()+" MB / "+(size/1024/1024).ToString("#,#,#")+" MB";


			long long total_offset=0;  long long total_size=0;
			for (int j=0; j < this->numfiles; j++)
			{
				total_offset += this->current_offsets[j];
				total_size += this->filesizes[j];
			}


//			this->label5->Text =  (total_offset / 1024).ToString("#,#,#")+"KB / "+(total_size/1024).ToString("#,#,#")+"KB";
			long long int total_offset_MB = total_offset / 1024LL/1024LL;
			int total_offset_dec_MB = (total_offset - total_offset_MB * 1024LL*1024LL)*10/1024/1024;
			this->label5->Text =  (total_offset_MB).ToString("#,#,#")+"."+total_offset_dec_MB.ToString()+" MB / "+(total_size/1024/1024).ToString("#,#,#")+" MB";

			if (size>0)
			{
				int val1 = (int)  (  (double) this->progressBar1->Maximum * (double) offset / (double) size  );
				if (val1<= this->progressBar1->Maximum && val1>=this->progressBar1->Minimum)
					this->progressBar1->Value= val1; 
				else printf("Warning: progressBar1 out of bounds!\n");
			}
			if (current_rate<0)
			{
				this->label8->Text = "-- MB/sec";
				this->label6->Text = "--:--:--";
				this->label7->Text = "--:--:--";
			}
			else
			{
				if ( floor(current_delta) > floor(this->last_current_delta) && current_delta>1.0)
				{
					this->label8->Text = (current_rate/1024.0/1024.0).ToString("F2")+" MB/sec";
					this->label6->Text = time_remaining_string(current_rate,(double) (size - offset ) );
                    this->label7->Text = time_remaining_string(current_rate, (double) ( total_size - total_offset ) );
				}



			}



			if (total_size>0)
			{
				int val2 = (int)  ( (double) this->progressBar2->Maximum * (double) total_offset / (double) total_size  );
				if (val2<= this->progressBar2->Maximum && val2>=this->progressBar2->Minimum)
				    this->progressBar2->Value = val2;
				else printf("Warning: progressBar2 out of bounds!\n");
			}


				//if ( total_delta > 3.0)
				//{
				//	if (current_delta > this->last_current_delta)
				//	{
				//		
//
//					}
//				}
//				else
//					this->label7->Text = "--:--:--";
	//		}
			// }
			int perc1 = ((int)(100.00 * (double) offset / (double) size));
			int perc2 = ((int)(100.00 * (double) total_offset / (double) total_size));
			perc1 = perc1<0 ? 0 : perc1;
            perc2 = perc2<0 ? 0 : perc2;
			perc1 = perc1>100 ? 100 : perc1;
            perc2 = perc2>100 ? 100 : perc2;

            this->label1->Text =  perc1.ToString() + "%";
			this->label2->Text =  perc2.ToString() + "%";
			this->last_current_delta = current_delta;

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
	

		long long rate_milliseconds;
		long long rate_bytes;
		System::Diagnostics::Stopwatch ^rate_stopwatch;


		//long long int current_filesize;
		//long long int total_filesize;
		//long long int current_offset;
		//long long int total_offset;
		long long current_bytes_received;
		long long total_bytes_received;
		array<long long int>^ filesizes;
		//array<long long int>^ current_offsets;
		int current_index;
        int numfiles;

		time_t current_start_time, total_start_time;
		String^ window_title;
		String^ current_file;
		double last_current_delta;
		bool has_initialised;
		IWin32Window ^parent_win;


		//String^ full_dest_filename, ^full_src_filename;
		array<bool>^          dest_exists;
		array<long long int>^ dest_size;
		array<int>^           overwrite_action;
		array<long long int>^ current_offsets;
		array<String^>^       dest_filename;
		array<FileItem^>^     src_items;
		array<array<TopfieldItem^>^>^ topfield_items_by_folder;
		bool usb_error; 
		String^ file_error; 
		bool loaded;
		int maximum_successful_index;

        CopyDirection copydirection;
		String^ current_error;

		bool freespace_check_needed;
		bool close_requested;
		System::Windows::Forms::Form^ parent_form;
		System::Threading::Thread^ thread;
		System::Windows::Forms::CheckBox^ parent_checkbox;
  


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
			this->SuspendLayout();
			// 
			// progressBar1
			// 
			this->progressBar1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->progressBar1->Location = System::Drawing::Point(12, 75);
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
			this->progressBar2->Location = System::Drawing::Point(12, 141);
			this->progressBar2->MarqueeAnimationSpeed = 0;
			this->progressBar2->Maximum = 1000;
			this->progressBar2->Name = L"progressBar2";
			this->progressBar2->Size = System::Drawing::Size(644, 30);
			this->progressBar2->TabIndex = 1;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->BackColor = System::Drawing::SystemColors::Control;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(335, 82);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(27, 16);
			this->label1->TabIndex = 2;
			this->label1->Text = L"0%";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->BackColor = System::Drawing::SystemColors::Control;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(335, 148);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(27, 16);
			this->label2->TabIndex = 3;
			this->label2->Text = L"0%";
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->Location = System::Drawing::Point(270, 196);
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
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->Location = System::Drawing::Point(17, 20);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(45, 16);
			this->label3->TabIndex = 5;
			this->label3->Text = L"label3";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label4->Location = System::Drawing::Point(9, 56);
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
			this->label5->Location = System::Drawing::Point(8, 122);
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
			this->label6->Location = System::Drawing::Point(585, 56);
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
			this->label7->Location = System::Drawing::Point(585, 122);
			this->label7->Margin = System::Windows::Forms::Padding(5, 0, 15, 0);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(45, 16);
			this->label7->TabIndex = 9;
			this->label7->Text = L"label7";
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label8->Location = System::Drawing::Point(566, 20);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(45, 16);
			this->label8->TabIndex = 10;
			this->label8->Text = L"label8";
			// 
			// checkBox1
			// 
			this->checkBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->checkBox1->AutoSize = true;
			this->checkBox1->Location = System::Drawing::Point(11, 209);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 11;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = true;
			this->checkBox1->CheckedChanged += gcnew System::EventHandler(this, &CopyDialog::checkBox1_CheckedChanged);
			// 
			// CopyDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->ClientSize = System::Drawing::Size(664, 248);
			this->Controls->Add(this->checkBox1);
			this->Controls->Add(this->label8);
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
			this->MaximumSize = System::Drawing::Size(680, 282);
			this->Name = L"CopyDialog";
			this->Padding = System::Windows::Forms::Padding(5);
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"CopyDialog";
			this->TransparencyKey = System::Drawing::Color::Fuchsia;
			this->Load += gcnew System::EventHandler(this, &CopyDialog::CopyDialog_Load);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &CopyDialog::CopyDialog_FormClosing);
			this->Resize += gcnew System::EventHandler(this, &CopyDialog::CopyDialog_Resize);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &CopyDialog::CopyDialog_KeyDown);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion



	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->button1->Enabled = false;
				 this->cancelled = true;
			 }

private: System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 this->turbo_request  = this->checkBox1->Checked;
			 if (this->parent_checkbox != nullptr)
				 this->parent_checkbox->Checked = this->checkBox1->Checked;
			
		 }
private: System::Void CopyDialog_Load(System::Object^  sender, System::EventArgs^  e) {
			 printf("CopyDialog loaded\n");
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
			 printf("Form closing, for some reason.\n");
			 this->button1->Enabled=false;
			 this->cancelled=true;
			 if (!this->close_requested)
		    	 e->Cancel = true;
			
			 
		 }
private: System::Void CopyDialog_Resize(System::Object^  sender, System::EventArgs^  e) {
			 if (FormWindowState::Minimized == this->WindowState)
				 this->parent_form->WindowState = FormWindowState::Minimized;
		 }
};
}
