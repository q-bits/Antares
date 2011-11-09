#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include "language.h"

namespace Antares {

	/// <summary>
	/// Summary for LowSpaceAlert
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class LowSpaceAlert : public System::Windows::Forms::Form
	{
	public:
		LowSpaceAlert(void)
		{
			InitializeComponent();
			
			this->apply_language();
		}
		void apply_language(void)
		{
			this->label1->Text = lang::f_warning;
			this->label4->Text = lang::f_proceed;
			this->button1->Text = lang::b_ok;
			this->button2->Text = lang::b_cancel_transfer;
			this->Text = lang::f_title;
			this->Name = lang::f_title;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~LowSpaceAlert()
		{
			if (components)
			{
				delete components;
			}
		}
	public: System::Windows::Forms::Button^  button2;
	protected: 
	public: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Label^  label1;
	public: System::Windows::Forms::Label^  required_label;
	public: System::Windows::Forms::Label^  available_label;
	public: System::Windows::Forms::Label^  label4;
	private: 

	private: 

	public: 




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
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->required_label = (gcnew System::Windows::Forms::Label());
			this->available_label = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// button2
			// 
			this->button2->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button2->Location = System::Drawing::Point(302, 135);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(174, 24);
			this->button2->TabIndex = 3;
			this->button2->Text = L"Cancel transfer";
			this->button2->UseVisualStyleBackColor = true;
			// 
			// button1
			// 
			this->button1->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button1->Location = System::Drawing::Point(54, 135);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(139, 24);
			this->button1->TabIndex = 2;
			this->button1->Text = L"OK";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->ForeColor = System::Drawing::Color::Blue;
			this->label1->Location = System::Drawing::Point(51, 21);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(252, 18);
			this->label1->TabIndex = 4;
			this->label1->Text = L"Warning: not enough free space!";
			// 
			// required_label
			// 
			this->required_label->AutoSize = true;
			this->required_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->required_label->Location = System::Drawing::Point(51, 58);
			this->required_label->Name = L"required_label";
			this->required_label->Size = System::Drawing::Size(80, 18);
			this->required_label->TabIndex = 5;
			this->required_label->Text = L"Required:";
			// 
			// available_label
			// 
			this->available_label->AutoSize = true;
			this->available_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->available_label->Location = System::Drawing::Point(299, 58);
			this->available_label->Name = L"available_label";
			this->available_label->Size = System::Drawing::Size(79, 18);
			this->available_label->TabIndex = 6;
			this->available_label->Text = L"Available:";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label4->ForeColor = System::Drawing::Color::Blue;
			this->label4->Location = System::Drawing::Point(51, 96);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(140, 18);
			this->label4->TabIndex = 7;
			this->label4->Text = L"Proceed anyway\?";
			// 
			// LowSpaceAlert
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(529, 186);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->available_label);
			this->Controls->Add(this->required_label);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->button1);
			this->Name = L"LowSpaceAlert";
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Low space";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

};
}
