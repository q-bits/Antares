#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
extern "C" {
#include "FBLib_rec.h"
}

namespace Antares {

	/// <summary>
	/// Summary for ProgInfo
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class ProgInfo : public System::Windows::Forms::Form
	{
	public:
		ProgInfo(void)
		{
			InitializeComponent();

		}

		ProgInfo(tRECHeaderInfo *ri, String^ windowtitle)
		{
			InitializeComponent();

			String^ channel = gcnew String(ri->SISvcName);
			String^ title = gcnew String(ri->EventEventName);
			String^ description = gcnew String(ri->EventEventDescription);
			String^ ext = gcnew String(ri->ExtEventText);
			if (description->Length >0 && ext->Length >0 )
			{
				description = description + "\n  --- \n";
			}
			description = description + ext;

			this->filename->Text = windowtitle;// "Program Information, "+fname;
			this->channel->Text = channel;
			this->title->Text = title;
			this->description->Text = description;
			String^ duration = ri->EventDurationMin.ToString() + "min";
			if (ri->EventDurationHour > 0 ) duration =  ri->EventDurationHour.ToString() + "hr " + duration;
			this->duration->Text = duration;
			int recorded_min = ri->HeaderDuration; 
            String^ recorded_duration = (recorded_min % 60).ToString() + "min";
			int recorded_hr = recorded_min/60;
			if (recorded_hr>0) recorded_duration = recorded_hr.ToString() + "hr " + recorded_duration;
			this->recorded_duration->Text = recorded_duration;
           
		}




	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~ProgInfo()
		{
			if (components)
			{
				delete components;
			}
		}
	public: System::Windows::Forms::Label^  label2;
	protected: 
	public: System::Windows::Forms::Label^  label3;
	public: System::Windows::Forms::Label^  label4;
	public: System::Windows::Forms::Label^  filename;
	public: System::Windows::Forms::Label^  channel;
	public: System::Windows::Forms::Label^  title;
	public: System::Windows::Forms::TextBox^  description;
	public: System::Windows::Forms::Label^  label5;
	public: System::Windows::Forms::Panel^  panel1;
	public: System::Windows::Forms::Label^  duration;
	public: System::Windows::Forms::Button^  button1;
	public: System::Windows::Forms::Label^  recorded_duration;

	public: System::Windows::Forms::Label^  label6;

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
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->filename = (gcnew System::Windows::Forms::Label());
			this->channel = (gcnew System::Windows::Forms::Label());
			this->title = (gcnew System::Windows::Forms::Label());
			this->description = (gcnew System::Windows::Forms::TextBox());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->duration = (gcnew System::Windows::Forms::Label());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->recorded_duration = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->panel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label2->ForeColor = System::Drawing::Color::Green;
			this->label2->Location = System::Drawing::Point(87, 52);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(74, 18);
			this->label2->TabIndex = 1;
			this->label2->Text = L"Channel:";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label3->ForeColor = System::Drawing::Color::Green;
			this->label3->Location = System::Drawing::Point(116, 81);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(45, 18);
			this->label3->TabIndex = 2;
			this->label3->Text = L"Title:";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label4->ForeColor = System::Drawing::Color::Green;
			this->label4->Location = System::Drawing::Point(62, 143);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(99, 18);
			this->label4->TabIndex = 3;
			this->label4->Text = L"Description:";
			// 
			// filename
			// 
			this->filename->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->filename->AutoSize = true;
			this->filename->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->filename->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(80)), static_cast<System::Int32>(static_cast<System::Byte>(80)), 
				static_cast<System::Int32>(static_cast<System::Byte>(80)));
			this->filename->Location = System::Drawing::Point(177, 8);
			this->filename->Name = L"filename";
			this->filename->Size = System::Drawing::Size(341, 18);
			this->filename->TabIndex = 4;
			this->filename->Text = L"Program information,   c:\\some filename.rec";
			// 
			// channel
			// 
			this->channel->AutoSize = true;
			this->channel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->channel->ForeColor = System::Drawing::Color::Navy;
			this->channel->Location = System::Drawing::Point(187, 52);
			this->channel->Name = L"channel";
			this->channel->Size = System::Drawing::Size(118, 18);
			this->channel->TabIndex = 5;
			this->channel->Text = L"Some Channel";
			// 
			// title
			// 
			this->title->AutoSize = true;
			this->title->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->title->ForeColor = System::Drawing::Color::Navy;
			this->title->Location = System::Drawing::Point(187, 81);
			this->title->Name = L"title";
			this->title->Size = System::Drawing::Size(204, 18);
			this->title->TabIndex = 6;
			this->title->Text = L"A really good show, part 2";
			// 
			// description
			// 
			this->description->BackColor = System::Drawing::SystemColors::Control;
			this->description->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->description->ForeColor = System::Drawing::Color::Navy;
			this->description->Location = System::Drawing::Point(190, 143);
			this->description->Multiline = true;
			this->description->Name = L"description";
			this->description->ReadOnly = true;
			this->description->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->description->Size = System::Drawing::Size(490, 111);
			this->description->TabIndex = 7;
			this->description->TabStop = false;
			this->description->Text = L"In this episode, some really interesting stuff happens. Starring Joe Blogs, Mary " 
				L"Smith. 2010.";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label5->ForeColor = System::Drawing::Color::Green;
			this->label5->Location = System::Drawing::Point(4, 111);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(157, 18);
			this->label5->TabIndex = 8;
			this->label5->Text = L"Scheduled duration:";
			// 
			// panel1
			// 
			this->panel1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel1->Controls->Add(this->filename);
			this->panel1->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel1->Location = System::Drawing::Point(0, 0);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(689, 37);
			this->panel1->TabIndex = 9;
			this->panel1->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &ProgInfo::panel1_Layout);
			// 
			// duration
			// 
			this->duration->AutoSize = true;
			this->duration->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->duration->ForeColor = System::Drawing::Color::Navy;
			this->duration->Location = System::Drawing::Point(187, 111);
			this->duration->Name = L"duration";
			this->duration->Size = System::Drawing::Size(82, 18);
			this->duration->TabIndex = 10;
			this->duration->Text = L"1hr 45min";
			// 
			// button1
			// 
			this->button1->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->button1->ForeColor = System::Drawing::SystemColors::ControlText;
			this->button1->Location = System::Drawing::Point(19, 228);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(140, 26);
			this->button1->TabIndex = 11;
			this->button1->Text = L"Close";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// recorded_duration
			// 
			this->recorded_duration->AutoSize = true;
			this->recorded_duration->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->recorded_duration->ForeColor = System::Drawing::Color::Navy;
			this->recorded_duration->Location = System::Drawing::Point(595, 111);
			this->recorded_duration->Name = L"recorded_duration";
			this->recorded_duration->Size = System::Drawing::Size(82, 18);
			this->recorded_duration->TabIndex = 13;
			this->recorded_duration->Text = L"1hr 45min";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label6->ForeColor = System::Drawing::Color::Green;
			this->label6->Location = System::Drawing::Point(440, 111);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(152, 18);
			this->label6->TabIndex = 12;
			this->label6->Text = L"Recorded duration:";
			// 
			// ProgInfo
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(689, 266);
			this->Controls->Add(this->recorded_duration);
			this->Controls->Add(this->label6);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->duration);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->description);
			this->Controls->Add(this->title);
			this->Controls->Add(this->channel);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->ForeColor = System::Drawing::Color::Green;
			this->Name = L"ProgInfo";
			this->Text = L"Program Information";
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void panel1_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e) {
				 int tw = this->filename->Width;
				 int pw = this->panel1->Width;
				 Point p = this->filename->Location;
				 int x = (pw-tw)/2;
				 //this->filename->Font->
				 if (x<0) {x=0;}
				 p.X = x;
				 this->filename->Location = p;
			 }
};
}
