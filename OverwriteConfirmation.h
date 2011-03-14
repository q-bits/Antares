#pragma once

#include <antares.h>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include <stdio.h>



namespace Antares {


	/// <summary>
	/// Summary for OverwriteConfirmation
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class OverwriteConfirmation : public System::Windows::Forms::Form
	{
	public:
		OverwriteConfirmation(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

		OverwriteConfirmation(String^ f1, String^ f2, String^ f3)
		{
			InitializeComponent();

			array<String^>^ fa1 = f1->Split(L'\n');
            array<String^>^ fa2 = f2->Split(L'\n');
            array<String^>^ fa3 = f3->Split(L'\n');
            int na1=fa1->Length;
			int na2=fa2->Length;
			int na3=fa3->Length;
            const int max_rows = 35;

			String^ s1="", ^s2="", ^s3="";

			int i1=0, i2=0, i3=0;
			//bool done=false;
			while(1)
			{
				if (i1<na1) { s1=s1+ fa1[i1]+"\n"; i1++;}
				if (i2<na2) { s2=s2+ fa2[i2]+"\n"; i2++;}
				if (i3<na3) { s3=s3+ fa3[i3]+"\n"; i3++;}
				if (i1+i2+i3 + 4*(i1>0) + 4*(i2>0) + 4*(i3>0) >=max_rows || (i1==na1 && i2==na2 && i3==na3)) break;
				
			}   
			if (i1==na1-1) { s1=s1+ fa1[i1]+"\n"; i1++;}
			if (i2==na2-1) { s2=s2+ fa2[i2]+"\n"; i2++;}
			if (i3==na3-1) { s3=s3+ fa3[i3]+"\n"; i3++;}
			if (i1<na1) s1=s1+" ... ("+(na1-i1).ToString() + " more) ...";
			if (i2<na2) s2=s2+" ... ("+(na2-i2).ToString() + " more) ...";
			if (i3<na3) s3=s3+" ... ("+(na3-i3).ToString() + " more) ...";

			this->files1->Text = s1->Trim();
			this->files2->Text = s2->Trim();
			this->files3->Text = s3->Trim();

		}




	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~OverwriteConfirmation()
		{
			if (components)
			{
				delete components;
			}
		}
	public: System::Windows::Forms::Label^  title_label;
	protected: 
	public: System::Windows::Forms::Panel^  panel1;
	public: System::Windows::Forms::RadioButton^  overwrite1;
	public: System::Windows::Forms::Label^  label1;
	public: System::Windows::Forms::Label^  files1;
	public: System::Windows::Forms::RadioButton^  overwrite2;
	public: System::Windows::Forms::Panel^  panel2;
	public: System::Windows::Forms::RadioButton^  skip2;
	public: System::Windows::Forms::Label^  label2;
	public: System::Windows::Forms::Label^  files2;
	public: System::Windows::Forms::RadioButton^  skip1;
	public: System::Windows::Forms::RadioButton^  resume2;
	public: System::Windows::Forms::Panel^  panel3;
	public: System::Windows::Forms::RadioButton^  skip3;
	public: System::Windows::Forms::RadioButton^  overwrite3;
	public: System::Windows::Forms::Label^  label3;
	public: System::Windows::Forms::Label^  files3;
	public: System::Windows::Forms::Panel^  panel4;
	public: System::Windows::Forms::Button^  button2;
	public: System::Windows::Forms::Button^  button1;
	public: System::Windows::Forms::CheckBox^  checkBox1;

	public: 

		CopyMode copymode;
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
			this->title_label = (gcnew System::Windows::Forms::Label());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->skip1 = (gcnew System::Windows::Forms::RadioButton());
			this->overwrite1 = (gcnew System::Windows::Forms::RadioButton());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->files1 = (gcnew System::Windows::Forms::Label());
			this->overwrite2 = (gcnew System::Windows::Forms::RadioButton());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->resume2 = (gcnew System::Windows::Forms::RadioButton());
			this->skip2 = (gcnew System::Windows::Forms::RadioButton());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->files2 = (gcnew System::Windows::Forms::Label());
			this->panel3 = (gcnew System::Windows::Forms::Panel());
			this->skip3 = (gcnew System::Windows::Forms::RadioButton());
			this->overwrite3 = (gcnew System::Windows::Forms::RadioButton());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->files3 = (gcnew System::Windows::Forms::Label());
			this->panel4 = (gcnew System::Windows::Forms::Panel());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->panel1->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel3->SuspendLayout();
			this->panel4->SuspendLayout();
			this->SuspendLayout();
			// 
			// title_label
			// 
			this->title_label->AutoSize = true;
			this->title_label->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->title_label->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->title_label->Dock = System::Windows::Forms::DockStyle::Top;
			this->title_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->title_label->Location = System::Drawing::Point(0, 0);
			this->title_label->Name = L"title_label";
			this->title_label->Padding = System::Windows::Forms::Padding(15, 15, 15, 25);
			this->title_label->Size = System::Drawing::Size(910, 62);
			this->title_label->TabIndex = 0;
			this->title_label->Text = L"Warning:   files with these names already exist                                  " 
				L"                                                                  ";
			this->title_label->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			// 
			// panel1
			// 
			this->panel1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel1->Controls->Add(this->skip1);
			this->panel1->Controls->Add(this->overwrite1);
			this->panel1->Controls->Add(this->label1);
			this->panel1->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel1->Location = System::Drawing::Point(0, 62);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(627, 27);
			this->panel1->TabIndex = 3;
			// 
			// skip1
			// 
			this->skip1->AutoSize = true;
			this->skip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip1->Checked = true;
			this->skip1->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->skip1->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip1->Location = System::Drawing::Point(371, 0);
			this->skip1->Name = L"skip1";
			this->skip1->Size = System::Drawing::Size(170, 27);
			this->skip1->TabIndex = 5;
			this->skip1->TabStop = true;
			this->skip1->Text = L"Skip (recommended)";
			this->skip1->UseVisualStyleBackColor = false;
			this->skip1->CheckedChanged += gcnew System::EventHandler(this, &OverwriteConfirmation::skip1_CheckedChanged);
			// 
			// overwrite1
			// 
			this->overwrite1->AutoSize = true;
			this->overwrite1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite1->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite1->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite1->Location = System::Drawing::Point(541, 0);
			this->overwrite1->Name = L"overwrite1";
			this->overwrite1->Size = System::Drawing::Size(86, 27);
			this->overwrite1->TabIndex = 4;
			this->overwrite1->Text = L"Overwrite";
			this->overwrite1->UseVisualStyleBackColor = false;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label1->Dock = System::Windows::Forms::DockStyle::Left;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->ForeColor = System::Drawing::Color::Navy;
			this->label1->Location = System::Drawing::Point(0, 0);
			this->label1->Name = L"label1";
			this->label1->Padding = System::Windows::Forms::Padding(15, 5, 10, 0);
			this->label1->Size = System::Drawing::Size(270, 37);
			this->label1->TabIndex = 2;
			this->label1->Text = L"This file has correct size and date:\r\n ";
			// 
			// files1
			// 
			this->files1->AutoSize = true;
			this->files1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files1->Dock = System::Windows::Forms::DockStyle::Top;
			this->files1->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files1->Location = System::Drawing::Point(0, 89);
			this->files1->Name = L"files1";
			this->files1->Padding = System::Windows::Forms::Padding(30, 5, 0, 15);
			this->files1->Size = System::Drawing::Size(190, 54);
			this->files1->TabIndex = 4;
			this->files1->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			// 
			// overwrite2
			// 
			this->overwrite2->AutoSize = true;
			this->overwrite2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite2->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite2->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite2->Location = System::Drawing::Point(541, 0);
			this->overwrite2->Name = L"overwrite2";
			this->overwrite2->Size = System::Drawing::Size(86, 27);
			this->overwrite2->TabIndex = 4;
			this->overwrite2->Text = L"Overwrite";
			this->overwrite2->UseVisualStyleBackColor = false;
			// 
			// panel2
			// 
			this->panel2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel2->Controls->Add(this->resume2);
			this->panel2->Controls->Add(this->skip2);
			this->panel2->Controls->Add(this->overwrite2);
			this->panel2->Controls->Add(this->label2);
			this->panel2->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel2->Location = System::Drawing::Point(0, 143);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(627, 27);
			this->panel2->TabIndex = 5;
			// 
			// resume2
			// 
			this->resume2->AutoSize = true;
			this->resume2->Checked = true;
			this->resume2->Dock = System::Windows::Forms::DockStyle::Right;
			this->resume2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->resume2->ForeColor = System::Drawing::Color::DarkGreen;
			this->resume2->Location = System::Drawing::Point(292, 0);
			this->resume2->Name = L"resume2";
			this->resume2->Size = System::Drawing::Size(196, 27);
			this->resume2->TabIndex = 6;
			this->resume2->TabStop = true;
			this->resume2->Text = L"Resume (recommended)";
			this->resume2->UseVisualStyleBackColor = true;
			// 
			// skip2
			// 
			this->skip2->AutoSize = true;
			this->skip2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip2->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip2->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip2->Location = System::Drawing::Point(488, 0);
			this->skip2->Name = L"skip2";
			this->skip2->Size = System::Drawing::Size(53, 27);
			this->skip2->TabIndex = 5;
			this->skip2->Text = L"Skip";
			this->skip2->UseVisualStyleBackColor = false;
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label2->Dock = System::Windows::Forms::DockStyle::Left;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->ForeColor = System::Drawing::Color::Navy;
			this->label2->Location = System::Drawing::Point(0, 0);
			this->label2->Name = L"label2";
			this->label2->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label2->Size = System::Drawing::Size(137, 21);
			this->label2->TabIndex = 2;
			this->label2->Text = L"Partial file:";
			// 
			// files2
			// 
			this->files2->AutoSize = true;
			this->files2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files2->Dock = System::Windows::Forms::DockStyle::Top;
			this->files2->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files2->Location = System::Drawing::Point(0, 170);
			this->files2->Name = L"files2";
			this->files2->Padding = System::Windows::Forms::Padding(30, 5, 0, 15);
			this->files2->Size = System::Drawing::Size(190, 54);
			this->files2->TabIndex = 6;
			this->files2->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			// 
			// panel3
			// 
			this->panel3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel3->Controls->Add(this->skip3);
			this->panel3->Controls->Add(this->overwrite3);
			this->panel3->Controls->Add(this->label3);
			this->panel3->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel3->Location = System::Drawing::Point(0, 224);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(627, 27);
			this->panel3->TabIndex = 7;
			// 
			// skip3
			// 
			this->skip3->AutoSize = true;
			this->skip3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip3->Checked = true;
			this->skip3->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip3->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip3->Location = System::Drawing::Point(488, 0);
			this->skip3->Name = L"skip3";
			this->skip3->Size = System::Drawing::Size(53, 27);
			this->skip3->TabIndex = 5;
			this->skip3->TabStop = true;
			this->skip3->Text = L"Skip";
			this->skip3->UseVisualStyleBackColor = false;
			// 
			// overwrite3
			// 
			this->overwrite3->AutoSize = true;
			this->overwrite3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite3->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite3->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite3->Location = System::Drawing::Point(541, 0);
			this->overwrite3->Name = L"overwrite3";
			this->overwrite3->Size = System::Drawing::Size(86, 27);
			this->overwrite3->TabIndex = 4;
			this->overwrite3->Text = L"Overwrite";
			this->overwrite3->UseVisualStyleBackColor = false;
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label3->Dock = System::Windows::Forms::DockStyle::Left;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->ForeColor = System::Drawing::Color::Navy;
			this->label3->Location = System::Drawing::Point(0, 0);
			this->label3->Name = L"label3";
			this->label3->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label3->Size = System::Drawing::Size(341, 21);
			this->label3->TabIndex = 2;
			this->label3->Text = L"This file has an unexpected size or date:";
			// 
			// files3
			// 
			this->files3->AutoSize = true;
			this->files3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files3->Dock = System::Windows::Forms::DockStyle::Top;
			this->files3->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files3->Location = System::Drawing::Point(0, 251);
			this->files3->Name = L"files3";
			this->files3->Padding = System::Windows::Forms::Padding(30, 5, 0, 15);
			this->files3->Size = System::Drawing::Size(190, 54);
			this->files3->TabIndex = 8;
			this->files3->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			// 
			// panel4
			// 
			this->panel4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel4->Controls->Add(this->button2);
			this->panel4->Controls->Add(this->button1);
			this->panel4->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->panel4->Location = System::Drawing::Point(0, 315);
			this->panel4->Name = L"panel4";
			this->panel4->Size = System::Drawing::Size(627, 51);
			this->panel4->TabIndex = 9;
			// 
			// button2
			// 
			this->button2->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button2->Location = System::Drawing::Point(386, 13);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(174, 24);
			this->button2->TabIndex = 1;
			this->button2->Text = L"Cancel transfer";
			this->button2->UseVisualStyleBackColor = true;
			// 
			// button1
			// 
			this->button1->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button1->Location = System::Drawing::Point(102, 13);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(139, 24);
			this->button1->TabIndex = 0;
			this->button1->Text = L"OK";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->Checked = true;
			this->checkBox1->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->checkBox1->ForeColor = System::Drawing::Color::DarkGreen;
			this->checkBox1->Location = System::Drawing::Point(371, 89);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(129, 19);
			this->checkBox1->TabIndex = 10;
			this->checkBox1->Text = L"Delete the PC copy";
			this->checkBox1->UseVisualStyleBackColor = true;
			this->checkBox1->Visible = false;
			// 
			// OverwriteConfirmation
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->ClientSize = System::Drawing::Size(627, 366);
			this->Controls->Add(this->checkBox1);
			this->Controls->Add(this->panel4);
			this->Controls->Add(this->files3);
			this->Controls->Add(this->panel3);
			this->Controls->Add(this->files2);
			this->Controls->Add(this->panel2);
			this->Controls->Add(this->files1);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->title_label);
			this->Name = L"OverwriteConfirmation";
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Files exist";
			this->Shown += gcnew System::EventHandler(this, &OverwriteConfirmation::OverwriteConfirmation_Shown);
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel2->PerformLayout();
			this->panel3->ResumeLayout(false);
			this->panel3->PerformLayout();
			this->panel4->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void OverwriteConfirmation_Shown(System::Object^  sender, System::EventArgs^  e) {
				  printf("%d %d (%d) %d (%d) %d (%d) %d\n",this->Height, this->panel1->Height,this->files1->Height, this->panel2->Height,this->files2->Height,  this->panel3->Height, this->files3->Height,this->panel4->Height);
			     this->Height = this->panel1->Visible * (this->panel1->Height+this->files1->Height+1) + 
					this->panel2->Visible*( this->panel2->Height+this->files2->Height  +1) + 
					this->panel3->Visible*( this->panel3->Height+this->files3->Height+1) + 
					this->panel4->Height  +  this->title_label->Height + 48;
			    
			 }
private: System::Void skip1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			 if (this->skip1->Checked) 
				 this->checkBox1->Enabled=true;
			 else
				 this->checkBox1->Enabled=false;
		 }
};
}
