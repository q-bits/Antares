#pragma once

extern "C"
{
#include <stdio.h>
#include <time.h>

}

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Threading;


namespace Antares {

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
			this->current_file="";
			this->last_current_delta=0;
			this->current_offset=0; 
			this->total_offset=0;

		}
		void showDialog_(void)
		{
			this->ShowDialog();
		}

		void showCopyDialog(void)
		{

			//System::Threading::Thread^ thread = gcnew System::Threading::Thread( gcnew System::Threading::ThreadStart( &this->copydialog->ShowDialog) );

			//thread->Start();
			//System::Threading::Thread^ thread = (gcnew System::Threading::Thread(()=> { 
			//	this->copydialog->Show(); 
			//}));
			//	Start(); 


			ThreadStart^ threadDelegate = gcnew ThreadStart( this, &CopyDialog::showDialog_);
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
				this->Invoke(d, gcnew array<Object^> { });
			}
			else
			{
				this->Close();
			}
		}



		void update_dialog_threadsafe(void)
		{

			// InvokeRequired required compares the thread ID of the
			// calling thread to the thread ID of the creating thread.
			// If these threads are different, it returns true.
			if (this->cancelled) return;
			while (!this->IsHandleCreated) 
				
			{
				Thread::Sleep(100);

			}
			// if (this->label3->InvokeRequired)
			// {
			UpdateDialogCallback^ d = 
				gcnew UpdateDialogCallback(this, &CopyDialog::update_dialog);
			this->Invoke(d, gcnew array<Object^> { });
			// }
			// else
			// {
		}
		void update_dialog(void)
		{
			this->Text = this->window_title;
			this->label3->Text = current_file;
			double current_delta = (double) (time(NULL) - this->current_start_time);
			double total_delta = (double) (time(NULL) - this->total_start_time);
			double current_rate = (double) (this->current_offset) / current_delta;
			double total_rate = (double) (this->total_offset) / total_delta;

			//this->label3->Text = this->teststring;
			this->label4->Text =  (this->current_offset / 1024).ToString("#,#,#")+"KB / "+(this->current_filesize/1024).ToString("#,#,#")+"KB";

			this->label5->Text =  (this->total_offset / 1024).ToString("#,#,#")+"KB / "+(this->total_filesize/1024).ToString("#,#,#")+"KB";

			if (this->current_filesize>0)
			{
				this->progressBar1->Value = (double) this->progressBar1->Maximum * (double) this->current_offset / (double) this->current_filesize;

				if ( current_delta > this->last_current_delta && current_delta>3.0)
				{
					this->label8->Text = (current_rate/1024.0/1024.0).ToString("F2")+" MB/sec";
					this->label6->Text = time_remaining_string(current_rate,(double) (this->current_filesize - this->current_offset ) );

				}
			}


			if (this->total_filesize>0)
			{
				this->progressBar2->Value = (double) this->progressBar2->Maximum * (double) this->total_offset / (double) this->total_filesize;

				if ( total_delta > 3.0 && current_delta > this->last_current_delta)
				{
					this->label7->Text = time_remaining_string(total_rate, (double) (this->total_filesize - this->total_offset ) );

				}
			}
			// }
			int perc1 = ((int)(100.00 * (double) this->current_offset / (double) this->current_filesize));
			int perc2 = ((int)(100.00 * (double) this->total_offset / (double) this->total_filesize));
			perc1 = perc1<0 ? 0 : perc1;
            perc2 = perc2<0 ? 0 : perc1;
			perc1 = perc1>100 ? 100 : perc1;
            perc2 = perc2>100 ? 100 : perc1;

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
		long long int current_filesize;
		long long int total_filesize;
		long long int current_offset;
		long long int total_offset;
		time_t current_start_time, total_start_time;
		String^ window_title;
		String^ current_file;
		double last_current_delta;



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
			this->SuspendLayout();
			// 
			// progressBar1
			// 
			this->progressBar1->Location = System::Drawing::Point(12, 87);
			this->progressBar1->Maximum = 1000;
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(656, 30);
			this->progressBar1->TabIndex = 0;
			// 
			// progressBar2
			// 
			this->progressBar2->Location = System::Drawing::Point(12, 164);
			this->progressBar2->Maximum = 1000;
			this->progressBar2->Name = L"progressBar2";
			this->progressBar2->Size = System::Drawing::Size(656, 30);
			this->progressBar2->TabIndex = 1;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->BackColor = System::Drawing::SystemColors::Control;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(335, 94);
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
			this->label2->Location = System::Drawing::Point(335, 172);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(27, 16);
			this->label2->TabIndex = 3;
			this->label2->Text = L"0%";
			// 
			// button1
			// 
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->Location = System::Drawing::Point(270, 220);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(140, 31);
			this->button1->TabIndex = 4;
			this->button1->Text = L"Cancel";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &CopyDialog::button1_Click);
			// 
			// label3
			// 
			this->label3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->label3->AutoSize = true;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->Location = System::Drawing::Point(17, 32);
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
			this->label4->Location = System::Drawing::Point(8, 68);
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
			this->label5->Location = System::Drawing::Point(8, 145);
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
			this->label6->Location = System::Drawing::Point(601, 68);
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
			this->label7->Location = System::Drawing::Point(601, 145);
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
			this->label8->Location = System::Drawing::Point(566, 32);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(45, 16);
			this->label8->TabIndex = 10;
			this->label8->Text = L"label8";
			// 
			// CopyDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(680, 282);
			this->ControlBox = false;
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
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"CopyDialog";
			this->Padding = System::Windows::Forms::Padding(5);
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->Text = L"CopyDialog";
			this->TransparencyKey = System::Drawing::Color::Fuchsia;
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion



	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->button1->Enabled = false;
				 this->cancelled = true;
			 }

};
}
