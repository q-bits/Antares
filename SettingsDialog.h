#pragma once

#include "Settings.h"
#include "language.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace Antares {

	/// <summary>
	/// Summary for SettingsDialog
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SettingsDialog : public System::Windows::Forms::Form
	{




	public:



		SettingsDialog(Antares::Settings^ settings_in)
		{

			InitializeComponent();
			this->apply_language();

            this->settings = settings_in;


			int lang_index=0;
			this->comboBox1->Items->Clear();
			String^ lang_setting = this->settings["language"];

			for (int j=0; j < this->settings->language_names->Length ; j++)
			{
				this->comboBox1->Items->Add( this->settings->language_names[j]);
				if (this->settings->language_codes[j] == lang_setting)
					lang_index = j;
			}
			this->comboBox1->SelectedIndex = lang_index;


			 for (int j=0; j<num_columns; j++)
			 {
				 String^ str = this->settings["PVR_Column"+j.ToString()+"Visible"];			 
				 this->checkedListBox1->SetItemChecked(j,str=="1");

				 str = this->settings["PC_Column"+j.ToString()+"Visible"];			 
				 this->checkedListBox2->SetItemChecked(j,str=="1");

			 }

			 if (this->settings["TurboMode"] == "on")
				 this->checkBox1->Checked=true;
			 else
				 this->checkBox1->Checked=false;

			 this->rescaleCheck->Checked = this->settings["RescaleColumns"]=="1";
			 this->nosleep_check->Checked = this->settings["prevent_sleep_during_transfer"]=="1";


		}

		
		void apply_language(void)
		{

			this->rescaleCheck->Text = lang::s_rescale;
			this->nosleep_check->Text = lang::s_idle;

			System::Drawing::Size sz = this->nosleep_check->Size;
			sz.Width = 280;
			sz.Height = 2*sz.Height;
			this->nosleep_check->AutoSize=false;
			this->nosleep_check->Size=sz;


			this->Text = lang::s_title;
			this->Name = lang::s_title;
			this->groupBox1->Text = lang::s_choose;
			
			this->button1->Text = lang::b_ok;
			this->button2->Text = lang::b_cancel;
			this->groupBox2->Text = lang::s_language;

			array<System::Object^>^ arr = gcnew cli::array< System::Object^  >(6) {lang::h_name, lang::h_size, lang::h_type, lang::h_date, lang::h_channel, lang::h_description};
			this->checkedListBox1->Items->Clear();
			this->checkedListBox1->Items->AddRange(arr);

			this->checkedListBox2->Items->Clear();
			this->checkedListBox2->Items->AddRange(arr);

//			this->checkedListBox1->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"Name", L"Size", L"Type", L"Date", L"Channel", 
//				L"Description"});


		}



	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SettingsDialog()
		{
			if (components)
			{
				delete components;
			}
		}

	public:
		Antares::Settings^ settings;
		static int num_columns=6;


	public: System::Windows::Forms::Button^  button2;
	private: 
	public: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::CheckedListBox^  checkedListBox1;
	public: 
	private: System::Windows::Forms::CheckedListBox^  checkedListBox2;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::CheckBox^  checkBox1;
	private: System::Windows::Forms::CheckBox^  rescaleCheck;
	private: System::Windows::Forms::CheckBox^  nosleep_check;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::ComboBox^  comboBox1;




	protected: 

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
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->checkedListBox1 = (gcnew System::Windows::Forms::CheckedListBox());
			this->checkedListBox2 = (gcnew System::Windows::Forms::CheckedListBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->rescaleCheck = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->nosleep_check = (gcnew System::Windows::Forms::CheckBox());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->groupBox1->SuspendLayout();
			this->groupBox2->SuspendLayout();
			this->SuspendLayout();
			// 
			// button2
			// 
			this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->button2->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button2->Location = System::Drawing::Point(289, 258);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(154, 24);
			this->button2->TabIndex = 3;
			this->button2->Text = L"Cancel";
			this->button2->UseVisualStyleBackColor = true;
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->button1->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button1->Location = System::Drawing::Point(72, 258);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(154, 24);
			this->button1->TabIndex = 2;
			this->button1->Text = L"OK";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &SettingsDialog::button1_Click);
			// 
			// checkedListBox1
			// 
			this->checkedListBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->checkedListBox1->CheckOnClick = true;
			this->checkedListBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->checkedListBox1->FormattingEnabled = true;
			this->checkedListBox1->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"Name", L"Size", L"Type", L"Date", L"Channel", 
				L"Description"});
			this->checkedListBox1->Location = System::Drawing::Point(8, 52);
			this->checkedListBox1->Name = L"checkedListBox1";
			this->checkedListBox1->Size = System::Drawing::Size(105, 116);
			this->checkedListBox1->TabIndex = 0;
			// 
			// checkedListBox2
			// 
			this->checkedListBox2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->checkedListBox2->CheckOnClick = true;
			this->checkedListBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->checkedListBox2->FormattingEnabled = true;
			this->checkedListBox2->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"Name", L"Size", L"Type", L"Date", L"Channel", 
				L"Description"});
			this->checkedListBox2->Location = System::Drawing::Point(121, 52);
			this->checkedListBox2->Name = L"checkedListBox2";
			this->checkedListBox2->Size = System::Drawing::Size(105, 116);
			this->checkedListBox2->TabIndex = 1;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(39, 30);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(31, 15);
			this->label1->TabIndex = 2;
			this->label1->Text = L"PVR";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(155, 30);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(23, 15);
			this->label2->TabIndex = 3;
			this->label2->Text = L"PC";
			// 
			// groupBox1
			// 
			this->groupBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->groupBox1->Controls->Add(this->rescaleCheck);
			this->groupBox1->Controls->Add(this->label2);
			this->groupBox1->Controls->Add(this->label1);
			this->groupBox1->Controls->Add(this->checkedListBox2);
			this->groupBox1->Controls->Add(this->checkedListBox1);
			this->groupBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox1->Location = System::Drawing::Point(12, 21);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(235, 207);
			this->groupBox1->TabIndex = 1;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Choose Columns";
			// 
			// rescaleCheck
			// 
			this->rescaleCheck->AutoSize = true;
			this->rescaleCheck->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F));
			this->rescaleCheck->Location = System::Drawing::Point(8, 184);
			this->rescaleCheck->Name = L"rescaleCheck";
			this->rescaleCheck->Size = System::Drawing::Size(199, 17);
			this->rescaleCheck->TabIndex = 5;
			this->rescaleCheck->Text = L"Rescale column widths automatically";
			this->rescaleCheck->UseVisualStyleBackColor = true;
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->Location = System::Drawing::Point(477, 343);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(83, 17);
			this->checkBox1->TabIndex = 4;
			this->checkBox1->Text = L"Turbo mode";
			this->checkBox1->UseVisualStyleBackColor = true;
			this->checkBox1->Visible = false;
			// 
			// nosleep_check
			// 
			this->nosleep_check->AutoSize = true;
			this->nosleep_check->Location = System::Drawing::Point(267, 160);
			this->nosleep_check->Name = L"nosleep_check";
			this->nosleep_check->Size = System::Drawing::Size(280, 30);
			this->nosleep_check->TabIndex = 5;
			this->nosleep_check->Text = L"During a transfer, prevent Windows from automatically\r\ngoing to sleep when idle";
			this->nosleep_check->UseVisualStyleBackColor = true;
			// 
			// groupBox2
			// 
			this->groupBox2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->groupBox2->Controls->Add(this->comboBox1);
			this->groupBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox2->Location = System::Drawing::Point(267, 21);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(286, 69);
			this->groupBox2->TabIndex = 6;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Language Selection";
			// 
			// comboBox1
			// 
			this->comboBox1->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Location = System::Drawing::Point(44, 29);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(176, 23);
			this->comboBox1->TabIndex = 0;
			// 
			// SettingsDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->ClientSize = System::Drawing::Size(565, 301);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->nosleep_check);
			this->Controls->Add(this->checkBox1);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->groupBox1);
			this->Name = L"SettingsDialog";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Antares Settings";
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->groupBox2->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void checkedListBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
				 

				 /////////////////////////////////////
				 // Collect the settings from the GUI before closing
				 /////////////////////////////////////

				 for (int j=0; j<num_columns; j++)
				 {
					 this->settings->changeSetting("PVR_Column"+j.ToString()+"Visible" ,  (  (int)  this->checkedListBox1->GetItemChecked(j) ).ToString() );

					 this->settings->changeSetting("PC_Column"+j.ToString()+"Visible" ,  (  (int)  this->checkedListBox2->GetItemChecked(j) ).ToString() );


				 }

				 if (this->checkBox1->Checked)
					 this->settings->changeSetting("TurboMode","on");
				 else
					  this->settings->changeSetting("TurboMode","off");

				 this->settings->changeSetting("RescaleColumns", (  (int)  this->rescaleCheck->Checked ).ToString() );
				 this->settings->changeSetting("prevent_sleep_during_transfer", (  (int)  this->nosleep_check->Checked ).ToString() );


				 int lang_index = this->comboBox1->SelectedIndex;
				 try{
					 this->settings->changeSetting("language",this->settings->language_codes[lang_index]);
				 }
				 catch(...){}
			
				

			 }
};
}
