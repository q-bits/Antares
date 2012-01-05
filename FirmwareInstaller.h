#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include "tf_models.h"


namespace Antares {

	delegate void UpdateCallback(void);



	public ref class FirmwareInstaller : public System::Windows::Forms::Form
	{
	public:
		FirmwareInstaller(void)
		{
			InitializeComponent();
		
			this->error_str="";
			this->path="";
			this->status_text="";
			this->cancelled=false;
			this->show_reboot=false;
			this->reboot_requested=false;

		}

		void update_status_text(void)
		{


			if (this->InvokeRequired)
			{
				UpdateCallback^ d = 
					gcnew UpdateCallback(this, &FirmwareInstaller::update_status_text);
				if (this->TopLevel)
					this->BeginInvoke(d, gcnew array<Object^> { });
				else
					this->parent_form->BeginInvoke(d, gcnew array<Object^> { });
			}
			else 
			{
				this->textBox2->Text = this->status_text;
				this->button4->Enabled=this->show_reboot;
				this->button1->Text = this->cancel_text;
			}


		}

		void update_status_text(String^ str)
		{
			this->status_text = str;
			this->update_status_text();
		}


		void update_status_text(String^ str, bool show_reboot_in)
		{
			this->status_text = str;
			this->show_reboot=show_reboot_in;
			this->update_status_text();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~FirmwareInstaller()
		{
			if (components)
			{
				delete components;
			}
		}
	public:
		System::Threading::Thread^ thread;
		System::Windows::Forms::Form^ parent_form;
		System::String^ error_str;
		System::String^ path;
		System::String^ status_text;
		System::String^ cancel_text;
		bool cancelled;
		bool show_reboot;
		bool reboot_requested;
		int file_sysid;
		String ^file_model;
	public: System::Windows::Forms::Button^  button1;
	protected: 

	private: System::Windows::Forms::Label^  label1;
	public: System::Windows::Forms::TextBox^  textBox1;
	private: 



	public: System::Windows::Forms::TextBox^  textBox2;
	private: 


	private: System::Windows::Forms::Label^  label2;
	public: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::Label^  label3;
	public: System::Windows::Forms::TextBox^  systemID_textbox;
	private: 

	private: 

	private: 

	private: 
	private: System::Windows::Forms::Label^  label4;
	public: System::Windows::Forms::TextBox^  model_textbox;
	private: 
	public: 

	private: 
	public: 
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
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->systemID_textbox = (gcnew System::Windows::Forms::TextBox());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->model_textbox = (gcnew System::Windows::Forms::TextBox());
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F));
			this->button1->Location = System::Drawing::Point(133, 237);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(196, 30);
			this->button1->TabIndex = 0;
			this->button1->Text = L"Cancel";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F));
			this->label1->Location = System::Drawing::Point(12, 18);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(67, 16);
			this->label1->TabIndex = 1;
			this->label1->Text = L"Filename:";
			// 
			// textBox1
			// 
			this->textBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->textBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->textBox1->Location = System::Drawing::Point(133, 18);
			this->textBox1->Name = L"textBox1";
			this->textBox1->ReadOnly = true;
			this->textBox1->Size = System::Drawing::Size(546, 20);
			this->textBox1->TabIndex = 2;
			// 
			// textBox2
			// 
			this->textBox2->BackColor = System::Drawing::SystemColors::ControlLightLight;
			this->textBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->textBox2->Location = System::Drawing::Point(88, 92);
			this->textBox2->Multiline = true;
			this->textBox2->Name = L"textBox2";
			this->textBox2->ReadOnly = true;
			this->textBox2->Size = System::Drawing::Size(591, 129);
			this->textBox2->TabIndex = 5;
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F));
			this->label2->Location = System::Drawing::Point(12, 93);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(48, 16);
			this->label2->TabIndex = 6;
			this->label2->Text = L"Status:";
			// 
			// button4
			// 
			this->button4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F));
			this->button4->Location = System::Drawing::Point(405, 236);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(199, 31);
			this->button4->TabIndex = 7;
			this->button4->Text = L"Reboot PVR";
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &FirmwareInstaller::button4_Click);
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F));
			this->label3->Location = System::Drawing::Point(12, 51);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(187, 16);
			this->label3->TabIndex = 8;
			this->label3->Text = L"Compatible with:       System ID";
			// 
			// systemID_textbox
			// 
			this->systemID_textbox->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->systemID_textbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->systemID_textbox->Location = System::Drawing::Point(201, 47);
			this->systemID_textbox->Name = L"systemID_textbox";
			this->systemID_textbox->ReadOnly = true;
			this->systemID_textbox->Size = System::Drawing::Size(75, 21);
			this->systemID_textbox->TabIndex = 9;
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F));
			this->label4->Location = System::Drawing::Point(307, 51);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(46, 16);
			this->label4->TabIndex = 10;
			this->label4->Text = L"Model";
			// 
			// model_textbox
			// 
			this->model_textbox->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->model_textbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->model_textbox->Location = System::Drawing::Point(359, 47);
			this->model_textbox->Name = L"model_textbox";
			this->model_textbox->ReadOnly = true;
			this->model_textbox->Size = System::Drawing::Size(320, 21);
			this->model_textbox->TabIndex = 11;
			// 
			// FirmwareInstaller
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(746, 305);
			this->ControlBox = false;
			this->Controls->Add(this->model_textbox);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->systemID_textbox);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->button4);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->textBox2);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->button1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = L"FirmwareInstaller";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Firmware Installation";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) {
			 this->reboot_requested=true;
		 }

};
}
