#pragma once

#include "Settings.h"

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
		SettingsDialog(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

		SettingsDialog(Antares::Settings^ settings_in)
		{
			 
			 InitializeComponent();
             this->settings = settings_in;



			 for (int j=0; j<=num_columns; j++)
			 {
				 String^ str = this->settings["PVR_Column"+j.ToString()+"Visible"];			 
				 this->checkedListBox1->SetItemChecked(j,str=="1");

				 str = this->settings["PC_Column"+j.ToString()+"Visible"];			 
				 this->checkedListBox2->SetItemChecked(j,str=="1");

			 }
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
		static int num_columns=5;


	public: System::Windows::Forms::Button^  button2;
	private: 
	public: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::CheckedListBox^  checkedListBox1;
	public: 
	private: System::Windows::Forms::CheckedListBox^  checkedListBox2;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::GroupBox^  groupBox1;
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
			this->groupBox1->SuspendLayout();
			this->SuspendLayout();
			// 
			// button2
			// 
			this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button2->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button2->Location = System::Drawing::Point(426, 345);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(174, 24);
			this->button2->TabIndex = 3;
			this->button2->Text = L"Cancel";
			this->button2->UseVisualStyleBackColor = true;
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button1->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button1->Location = System::Drawing::Point(142, 345);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(139, 24);
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
			this->checkedListBox1->MultiColumn = true;
			this->checkedListBox1->Name = L"checkedListBox1";
			this->checkedListBox1->Size = System::Drawing::Size(100, 116);
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
			this->checkedListBox2->Location = System::Drawing::Point(114, 52);
			this->checkedListBox2->MultiColumn = true;
			this->checkedListBox2->Name = L"checkedListBox2";
			this->checkedListBox2->Size = System::Drawing::Size(100, 116);
			this->checkedListBox2->TabIndex = 1;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(37, 30);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(31, 15);
			this->label1->TabIndex = 2;
			this->label1->Text = L"PVR";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(141, 30);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(23, 15);
			this->label2->TabIndex = 3;
			this->label2->Text = L"PC";
			// 
			// groupBox1
			// 
			this->groupBox1->BackColor = System::Drawing::SystemColors::Control;
			this->groupBox1->Controls->Add(this->label2);
			this->groupBox1->Controls->Add(this->label1);
			this->groupBox1->Controls->Add(this->checkedListBox2);
			this->groupBox1->Controls->Add(this->checkedListBox1);
			this->groupBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox1->Location = System::Drawing::Point(12, 21);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(235, 187);
			this->groupBox1->TabIndex = 1;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Choose Columns";
			// 
			// SettingsDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->ClientSize = System::Drawing::Size(770, 381);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->groupBox1);
			this->Name = L"SettingsDialog";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Antares Settings";
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void checkedListBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
				 

				 /////////////////////////////////////
				 // Collect the settings from the GUI before closing
				 /////////////////////////////////////

				 for (int j=0; j<=num_columns; j++)
				 {
					 this->settings->changeSetting("PVR_Column"+j.ToString()+"Visible" ,  (  (int)  this->checkedListBox1->GetItemChecked(j) ).ToString() );

					 this->settings->changeSetting("PC_Column"+j.ToString()+"Visible" ,  (  (int)  this->checkedListBox2->GetItemChecked(j) ).ToString() );


				 }



			 }
};
}
