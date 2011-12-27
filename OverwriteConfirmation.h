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
			this->apply_language();
		}

		OverwriteConfirmation(array<String^>^ dest_filename, array<long long int>^ dest_size, array<bool>^ dest_exists,
			array<long long int>^ src_size, array<DateTime>^ dest_date, array<DateTime>^ src_date, 
			Antares::CopyDirection copydirection, Antares::CopyMode copymode,
			array<int>^ overwrite_category, array<int>^ overwrite_action)
		{
			this->copymode = copymode;
			this->overwrite_category=overwrite_category;
			this->overwrite_action=overwrite_action;
			this->dest_exists = dest_exists;
			InitializeComponent();
			this->apply_language();

			int numitems = dest_filename->Length;

			

			array<int> ^num_cat = {0,0,0,0,0,0,0};

			// 0. Correct size
			// 1. Undersized
			// 2. Destination larger
			// 3. Correct size and date
			// 4. Existing file Older
			// 5. Existing file newer
			// 6. Correct date, wrong size

			const long long int large_size = 50*1024*1024;
			int num_exist=0, num_exist_a=0, num_exist_b=0;


			bool cat_0s_have_correct_date = true;
			for (int i=0; i<numitems; i++)
			{
				if (!dest_exists[i]) continue;
				bool isrec = dest_filename[i]->EndsWith(".rec",StringComparison::CurrentCultureIgnoreCase)
					|| dest_size[i] > large_size || src_size[i] > large_size;
				int cat;
				double dt = Math::Abs(  (src_date[i] - dest_date[i]).TotalHours  );
				bool same_date = (dt < 1.0/60.0);
				//printf("dt=%f %s %s\n",dt, src_date[i].ToString(), dest_date[i].ToString());
				bool dest_is_older =  dest_date[i] > src_date[i];
				num_exist++;
				if (isrec)
				{
					num_exist_a++;
					cat=2;
					if (dest_size[i] == src_size[i])
					{
						cat=0;
						cat_0s_have_correct_date = cat_0s_have_correct_date & same_date;
					}
					else if (dest_size[i] < src_size[i]) cat=1;
				}
				else
				{
					num_exist_b++;
					if (same_date)
					{
						if (dest_size[i]==src_size[i])
							cat=3;
						else 
							cat=6;
					}
					else
					{
						if (dest_is_older)
							cat = 4;
						else
							cat = 5;
					}
				}
				overwrite_category[i]=cat;
				num_cat[cat]++;
			}
			
			this->title_label->Text = num_exist > 1 ? lang::o_exist_plural : lang::o_exist;


			// Combine cat 3 into cat 0 in certain circumstances
			if (num_exist_b >0 && num_cat[3]==num_exist_b && cat_0s_have_correct_date)
			{
				num_cat[0] += num_cat[3]; num_cat[3]=0;num_exist_a+=num_exist_b; num_exist_b=0;
				for (int i=0; i<numitems; i++) if (overwrite_category[i]==3) overwrite_category[i]=0;
			}

			if (num_exist_a == 0 || num_exist_b==0)
			{
				this->group_a_label->Visible = false;
				this->group_b_label->Visible = false;

			}

			array<Panel^>^ panels = {panel1,panel2,panel3,panel5,panel6,panel7,panel8};
			array<Label^>^ file_labels = {files1,files2,files3,files4,files5,files6,files7};
			array<CheckBox^>^ checkboxes = {checkBox1, nullptr, nullptr, checkBox2, nullptr, nullptr, nullptr};
			skips      = gcnew array<RadioButton^>{skip1, skip2, skip3, skip4, skip5, skip6, skip7};
			overwrites = gcnew array<RadioButton^>{overwrite1, overwrite2, overwrite3, overwrite4, overwrite5, overwrite6, overwrite7};
			resumes    = gcnew array<RadioButton^>{nullptr, resume2, nullptr, nullptr, nullptr, nullptr, nullptr};
			array<Label^>^ blue_labels = {label1, label2, label3, label4, label6, label9, label11};
			array<String^>^ blue_strings = {lang::o_correct, lang::o_undersized, lang::o_oversized, lang::o_correct2, lang::o_older,lang::o_newer, lang::o_wrong_size};
			array<String^>^ blue_strings_plural = {lang::o_correct_plural, lang::o_undersized_plural, lang::o_oversized_plural, lang::o_correct2_plural, lang::o_older_plural,lang::o_newer_plural, lang::o_wrong_size_plural};

			// Set visibility of the 7 categories, and set the blue labels to have the correct language and plurality
			for (int i=0; i<7; i++)
			{
				if (checkboxes[i]) 
				{
					if (num_cat[i]==0 || copymode == CopyMode::COPY)
						checkboxes[i]->Visible=false;
					else
					{
						checkboxes[i]->Visible=true;
						if (copydirection==CopyDirection::PC_TO_PVR)
							checkboxes[i]->Text = num_cat[i] > 1 ? lang::o_delete_pc_plural : lang::o_delete_pc;
						else
							checkboxes[i]->Text = num_cat[i] > 1 ? lang::o_delete_pvr_plural : lang::o_delete_pvr;

					}
				}

				if (num_cat[i]==0)
				{
					panels[i]->Visible=false;
					file_labels[i]->Visible=false;

				}
				else 
				{
					String^ str;
					if (num_cat[i]==1)
						str = blue_strings[i];
					else
						str = blue_strings_plural[i];
					if (!str->EndsWith("!"))
						str = str+":";

					blue_labels[i]->Text = str;
					file_labels[i]->Text = "";
				}
			}
			if (cat_0s_have_correct_date)
			{
				if (num_cat[0]>0)
				{
					String^ str = blue_strings_plural[3];
					if (num_cat[0]==1) str = blue_strings[3];
					blue_labels[0]->Text = str+":";
				}
			}

			array< array<String^>^>^ filearrays = gcnew array< array<String^>^>(7);
			for (int i=0; i<7; i++)
			{
				filearrays[i] = gcnew array<String^>(num_cat[i]);
				num_cat[i]=0;
			}
			for (int i=0; i<numitems; i++)
			{
				if (!dest_exists[i]) continue;
				int cat = overwrite_category[i];
				filearrays[cat][ num_cat[cat]  ] = dest_filename[i];
				num_cat[cat]++;
			}


			int maxheight = Screen::PrimaryScreen->WorkingArea.Height;
			if (maxheight<580) maxheight=580;


			int max_in_category=16;
			array<int> ^used = {0,0,0,0,0,0,0};
			for (int passes=0; passes<500; passes++)
			{
				bool changed=false;
				bool full=false;
				for (int i=0; i<7; i++)
				{
					if (used[i] >= num_cat[i]) continue;
					if (used[i] >= max_in_category) continue;
					String^ str = file_labels[i]->Text;

					if (str->Length>0) str = str + "\n";
					str = str + filearrays[i][used[i]];
					changed=true;
					file_labels[i]->Text = str;
					int h=0;
					if (passes>1)
					{
						h=this->visible_height(panels, file_labels,num_cat);
						if (h>maxheight)
						{
							full=true;
							used[i]--;
						}
					}
					used[i]++;
					if (full) break;
				}
				if (!changed) break; 
				if (full) break;
			}
			for (int i=0; i<7; i++)
			{
				if (used[i]>=num_cat[i]) continue;
				int more = num_cat[i] - ( used[i] - 1 );
				filearrays[i][used[i]-1] = "... ("+more.ToString() +" more) ...";
				String^ str = filearrays[i][0];
				for (int k=1; k<used[i]; k++)
					str += "\n" + filearrays[i][k];
				file_labels[i]->Text = str;
			}

			for (int i=0; i<7; i++)
			{
				if (checkboxes[i]==nullptr) continue;
				Point p = checkboxes[i]->Location;
				p.Y = file_labels[i]->Location.Y + 4;
				checkboxes[i]->Location=p;
			}



			//this->Height = this->visible_height(panels, file_labels, num_cat);
			System::Drawing::Size s = this->Size;
			s.Height = this->visible_height(panels, file_labels, num_cat);
			this->Size = s;
			//file_labels[0]->Text = "Some text here";
			this->PerformLayout();




		}


		// OLD VERSION
		OverwriteConfirmation(String^ f1, String^ f2, String^ f3)
		{
			InitializeComponent();
			this->apply_language();

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

		array<OverwriteAction>^ overwrite_actions_per_category(void)
		{
			int ncat = this->skips->Length;
			array<OverwriteAction>^ actions_per_category = gcnew array<OverwriteAction>(ncat);
			for (int i=0; i<ncat; i++)
			{
				if (this->resumes[i])
					if (this->resumes[i]->Checked) {actions_per_category[i]=RESUME;continue;};

				if (this->skips[i]->Checked) actions_per_category[i]=SKIP;
				else
					actions_per_category[i]=OVERWRITE;
			}
			return actions_per_category;

		}


		void apply_language(void)
		{
			this->skip1->Text = lang::o_skip_r;
			this->overwrite1->Text = lang::o_overwrite;

			this->resume2->Text = lang::o_resume;
			this->skip2->Text = lang::o_skip;
			this->overwrite2->Text = lang::o_overwrite;


			this->skip3->Text = lang::o_skip;
			this->overwrite3->Text = lang::o_overwrite;

			this->button1->Text = lang::b_ok;
			this->button2->Text = lang::b_cancel_transfer;


			this->Text = "";  // Do I want a window title?

			this->group_a_label->Text = "a) "+lang::o_category_a;
			this->group_b_label->Text = "b) "+lang::o_category_b;



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
		array<int> ^overwrite_action;
		array<int> ^overwrite_category;
		array<bool> ^dest_exists;

		array<RadioButton^>^ skips ;
		array<RadioButton^>^ overwrites;
		array<RadioButton^>^ resumes;

	public: System::Windows::Forms::Panel^  panel5;
	public: System::Windows::Forms::RadioButton^  skip4;

	public: System::Windows::Forms::RadioButton^  overwrite4;

	public: System::Windows::Forms::Label^  label4;
	public: System::Windows::Forms::Label^  files4;

	public: System::Windows::Forms::Panel^  panel6;
	public: System::Windows::Forms::RadioButton^  skip5;

	public: System::Windows::Forms::RadioButton^  overwrite5;

	public: System::Windows::Forms::Label^  label6;
	public: System::Windows::Forms::Label^  files5;
	public: System::Windows::Forms::Label^  files6;


	public: System::Windows::Forms::Panel^  panel7;
	public: System::Windows::Forms::RadioButton^  skip6;

	public: System::Windows::Forms::RadioButton^  overwrite6;

	public: System::Windows::Forms::Label^  label9;
	public: System::Windows::Forms::Label^  files7;

	public: System::Windows::Forms::Panel^  panel8;
	public: System::Windows::Forms::RadioButton^  skip7;

	public: System::Windows::Forms::RadioButton^  overwrite7;

	public: System::Windows::Forms::Label^  label11;
	public: System::Windows::Forms::Label^  group_a_label;
	public: System::Windows::Forms::Label^  group_b_label;


	public: System::Windows::Forms::CheckBox^  checkBox2;

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
			this->panel5 = (gcnew System::Windows::Forms::Panel());
			this->skip4 = (gcnew System::Windows::Forms::RadioButton());
			this->overwrite4 = (gcnew System::Windows::Forms::RadioButton());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->files4 = (gcnew System::Windows::Forms::Label());
			this->panel6 = (gcnew System::Windows::Forms::Panel());
			this->skip5 = (gcnew System::Windows::Forms::RadioButton());
			this->overwrite5 = (gcnew System::Windows::Forms::RadioButton());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->files5 = (gcnew System::Windows::Forms::Label());
			this->files6 = (gcnew System::Windows::Forms::Label());
			this->panel7 = (gcnew System::Windows::Forms::Panel());
			this->skip6 = (gcnew System::Windows::Forms::RadioButton());
			this->overwrite6 = (gcnew System::Windows::Forms::RadioButton());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->files7 = (gcnew System::Windows::Forms::Label());
			this->panel8 = (gcnew System::Windows::Forms::Panel());
			this->skip7 = (gcnew System::Windows::Forms::RadioButton());
			this->overwrite7 = (gcnew System::Windows::Forms::RadioButton());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->group_a_label = (gcnew System::Windows::Forms::Label());
			this->group_b_label = (gcnew System::Windows::Forms::Label());
			this->checkBox2 = (gcnew System::Windows::Forms::CheckBox());
			this->panel1->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel3->SuspendLayout();
			this->panel4->SuspendLayout();
			this->panel5->SuspendLayout();
			this->panel6->SuspendLayout();
			this->panel7->SuspendLayout();
			this->panel8->SuspendLayout();
			this->SuspendLayout();
			this->title_label->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->title_label->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->title_label->Dock = System::Windows::Forms::DockStyle::Top;
			this->title_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11, System::Drawing::FontStyle::Bold));
			this->title_label->Location = System::Drawing::Point(0, 0);
			this->title_label->Name = L"title_label";
			this->title_label->Padding = System::Windows::Forms::Padding(15, 15, 15, 0);
			this->title_label->Size = System::Drawing::Size(718, 47);
			this->title_label->TabIndex = 0;
			this->title_label->Text = L"Files with these names already exist                                             " 
				L"    ";
			this->panel1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel1->Controls->Add(this->skip1);
			this->panel1->Controls->Add(this->overwrite1);
			this->panel1->Controls->Add(this->label1);
			this->panel1->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel1->Location = System::Drawing::Point(0, 75);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(718, 27);
			this->panel1->TabIndex = 3;
			this->skip1->AutoSize = true;
			this->skip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip1->Checked = true;
			this->skip1->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->skip1->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip1->Location = System::Drawing::Point(462, 0);
			this->skip1->Name = L"skip1";
			this->skip1->Size = System::Drawing::Size(170, 27);
			this->skip1->TabIndex = 5;
			this->skip1->TabStop = true;
			this->skip1->Text = L"Skip (recommended)";
			this->skip1->UseVisualStyleBackColor = false;
			this->skip1->CheckedChanged += gcnew System::EventHandler(this, &OverwriteConfirmation::skip1_CheckedChanged);
			this->overwrite1->AutoSize = true;
			this->overwrite1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite1->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite1->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite1->Location = System::Drawing::Point(632, 0);
			this->overwrite1->Name = L"overwrite1";
			this->overwrite1->Size = System::Drawing::Size(86, 27);
			this->overwrite1->TabIndex = 4;
			this->overwrite1->Text = L"Overwrite";
			this->overwrite1->UseVisualStyleBackColor = false;
			this->label1->AutoSize = true;
			this->label1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label1->Dock = System::Windows::Forms::DockStyle::Left;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->ForeColor = System::Drawing::Color::Navy;
			this->label1->Location = System::Drawing::Point(0, 0);
			this->label1->Name = L"label1";
			this->label1->Padding = System::Windows::Forms::Padding(15, 5, 10, 0);
			this->label1->Size = System::Drawing::Size(218, 21);
			this->label1->TabIndex = 2;
			this->label1->Text = L"Files have the correct size:";
			this->files1->AutoSize = true;
			this->files1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files1->Dock = System::Windows::Forms::DockStyle::Top;
			this->files1->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files1->Location = System::Drawing::Point(0, 102);
			this->files1->Name = L"files1";
			this->files1->Padding = System::Windows::Forms::Padding(30, 5, 0, 5);
			this->files1->Size = System::Drawing::Size(190, 44);
			this->files1->TabIndex = 4;
			this->files1->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			this->overwrite2->AutoSize = true;
			this->overwrite2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite2->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite2->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite2->Location = System::Drawing::Point(632, 0);
			this->overwrite2->Name = L"overwrite2";
			this->overwrite2->Size = System::Drawing::Size(86, 27);
			this->overwrite2->TabIndex = 4;
			this->overwrite2->Text = L"Overwrite";
			this->overwrite2->UseVisualStyleBackColor = false;
			this->panel2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel2->Controls->Add(this->resume2);
			this->panel2->Controls->Add(this->skip2);
			this->panel2->Controls->Add(this->overwrite2);
			this->panel2->Controls->Add(this->label2);
			this->panel2->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel2->Location = System::Drawing::Point(0, 146);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(718, 27);
			this->panel2->TabIndex = 5;
			this->resume2->AutoSize = true;
			this->resume2->Checked = true;
			this->resume2->Dock = System::Windows::Forms::DockStyle::Right;
			this->resume2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->resume2->ForeColor = System::Drawing::Color::DarkGreen;
			this->resume2->Location = System::Drawing::Point(383, 0);
			this->resume2->Name = L"resume2";
			this->resume2->Size = System::Drawing::Size(196, 27);
			this->resume2->TabIndex = 6;
			this->resume2->TabStop = true;
			this->resume2->Text = L"Resume (recommended)";
			this->resume2->UseVisualStyleBackColor = true;
			this->skip2->AutoSize = true;
			this->skip2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip2->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip2->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip2->Location = System::Drawing::Point(579, 0);
			this->skip2->Name = L"skip2";
			this->skip2->Size = System::Drawing::Size(53, 27);
			this->skip2->TabIndex = 5;
			this->skip2->Text = L"Skip";
			this->skip2->UseVisualStyleBackColor = false;
			this->label2->AutoSize = true;
			this->label2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label2->Dock = System::Windows::Forms::DockStyle::Left;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->ForeColor = System::Drawing::Color::Navy;
			this->label2->Location = System::Drawing::Point(0, 0);
			this->label2->Name = L"label2";
			this->label2->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label2->Size = System::Drawing::Size(179, 21);
			this->label2->TabIndex = 2;
			this->label2->Text = L"Undersized files:";
			this->files2->AutoSize = true;
			this->files2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files2->Dock = System::Windows::Forms::DockStyle::Top;
			this->files2->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files2->Location = System::Drawing::Point(0, 173);
			this->files2->Name = L"files2";
			this->files2->Padding = System::Windows::Forms::Padding(30, 5, 0, 5);
			this->files2->Size = System::Drawing::Size(190, 44);
			this->files2->TabIndex = 6;
			this->files2->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			this->panel3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel3->Controls->Add(this->skip3);
			this->panel3->Controls->Add(this->overwrite3);
			this->panel3->Controls->Add(this->label3);
			this->panel3->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel3->Location = System::Drawing::Point(0, 217);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(718, 27);
			this->panel3->TabIndex = 7;
			this->skip3->AutoSize = true;
			this->skip3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip3->Checked = true;
			this->skip3->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip3->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip3->Location = System::Drawing::Point(579, 0);
			this->skip3->Name = L"skip3";
			this->skip3->Size = System::Drawing::Size(53, 27);
			this->skip3->TabIndex = 5;
			this->skip3->TabStop = true;
			this->skip3->Text = L"Skip";
			this->skip3->UseVisualStyleBackColor = false;
			this->overwrite3->AutoSize = true;
			this->overwrite3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite3->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite3->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite3->Location = System::Drawing::Point(632, 0);
			this->overwrite3->Name = L"overwrite3";
			this->overwrite3->Size = System::Drawing::Size(86, 27);
			this->overwrite3->TabIndex = 4;
			this->overwrite3->Text = L"Overwrite";
			this->overwrite3->UseVisualStyleBackColor = false;
			this->label3->AutoSize = true;
			this->label3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label3->Dock = System::Windows::Forms::DockStyle::Left;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->ForeColor = System::Drawing::Color::Navy;
			this->label3->Location = System::Drawing::Point(0, 0);
			this->label3->Name = L"label3";
			this->label3->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label3->Size = System::Drawing::Size(273, 21);
			this->label3->TabIndex = 2;
			this->label3->Text = L"These existing files are larger!";
			this->files3->AutoSize = true;
			this->files3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files3->Dock = System::Windows::Forms::DockStyle::Top;
			this->files3->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files3->Location = System::Drawing::Point(0, 244);
			this->files3->Name = L"files3";
			this->files3->Padding = System::Windows::Forms::Padding(30, 5, 0, 5);
			this->files3->Size = System::Drawing::Size(190, 44);
			this->files3->TabIndex = 8;
			this->files3->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			this->panel4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel4->Controls->Add(this->button2);
			this->panel4->Controls->Add(this->button1);
			this->panel4->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->panel4->Location = System::Drawing::Point(0, 597);
			this->panel4->Name = L"panel4";
			this->panel4->Size = System::Drawing::Size(718, 51);
			this->panel4->TabIndex = 9;
			this->button2->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->button2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button2->Location = System::Drawing::Point(407, 13);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(174, 24);
			this->button2->TabIndex = 1;
			this->button2->Text = L"Cancel transfer";
			this->button2->UseVisualStyleBackColor = true;
			this->button1->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->button1->Location = System::Drawing::Point(116, 13);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(139, 24);
			this->button1->TabIndex = 0;
			this->button1->Text = L"OK";
			this->button1->UseVisualStyleBackColor = true;
			this->checkBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->checkBox1->AutoSize = true;
			this->checkBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(255)), 
				static_cast<System::Int32>(static_cast<System::Byte>(230)));
			this->checkBox1->Checked = true;
			this->checkBox1->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->checkBox1->ForeColor = System::Drawing::Color::DarkGreen;
			this->checkBox1->Location = System::Drawing::Point(589, 89);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(129, 19);
			this->checkBox1->TabIndex = 10;
			this->checkBox1->Text = L"Delete the PC copy";
			this->checkBox1->UseVisualStyleBackColor = false;
			this->checkBox1->Visible = false;
			this->checkBox1->CheckedChanged += gcnew System::EventHandler(this, &OverwriteConfirmation::checkBox1_CheckedChanged);
			this->panel5->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel5->Controls->Add(this->skip4);
			this->panel5->Controls->Add(this->overwrite4);
			this->panel5->Controls->Add(this->label4);
			this->panel5->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel5->Location = System::Drawing::Point(0, 316);
			this->panel5->Name = L"panel5";
			this->panel5->Size = System::Drawing::Size(718, 27);
			this->panel5->TabIndex = 11;
			this->skip4->AutoSize = true;
			this->skip4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip4->Checked = true;
			this->skip4->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip4->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip4->Location = System::Drawing::Point(579, 0);
			this->skip4->Name = L"skip4";
			this->skip4->Size = System::Drawing::Size(53, 27);
			this->skip4->TabIndex = 5;
			this->skip4->TabStop = true;
			this->skip4->Text = L"Skip";
			this->skip4->UseVisualStyleBackColor = false;
			this->overwrite4->AutoSize = true;
			this->overwrite4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite4->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite4->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite4->Location = System::Drawing::Point(632, 0);
			this->overwrite4->Name = L"overwrite4";
			this->overwrite4->Size = System::Drawing::Size(86, 27);
			this->overwrite4->TabIndex = 4;
			this->overwrite4->Text = L"Overwrite";
			this->overwrite4->UseVisualStyleBackColor = false;
			this->label4->AutoSize = true;
			this->label4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label4->Dock = System::Windows::Forms::DockStyle::Left;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label4->ForeColor = System::Drawing::Color::Navy;
			this->label4->Location = System::Drawing::Point(0, 0);
			this->label4->Name = L"label4";
			this->label4->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label4->Size = System::Drawing::Size(313, 21);
			this->label4->TabIndex = 2;
			this->label4->Text = L"Files have the correct size and date:";
			this->files4->AutoSize = true;
			this->files4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files4->Dock = System::Windows::Forms::DockStyle::Top;
			this->files4->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files4->Location = System::Drawing::Point(0, 343);
			this->files4->Name = L"files4";
			this->files4->Padding = System::Windows::Forms::Padding(30, 5, 0, 5);
			this->files4->Size = System::Drawing::Size(190, 44);
			this->files4->TabIndex = 12;
			this->files4->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			this->panel6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel6->Controls->Add(this->skip5);
			this->panel6->Controls->Add(this->overwrite5);
			this->panel6->Controls->Add(this->label6);
			this->panel6->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel6->Location = System::Drawing::Point(0, 387);
			this->panel6->Name = L"panel6";
			this->panel6->Size = System::Drawing::Size(718, 27);
			this->panel6->TabIndex = 13;
			this->skip5->AutoSize = true;
			this->skip5->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip5->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip5->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip5->Location = System::Drawing::Point(579, 0);
			this->skip5->Name = L"skip5";
			this->skip5->Size = System::Drawing::Size(53, 27);
			this->skip5->TabIndex = 5;
			this->skip5->Text = L"Skip";
			this->skip5->UseVisualStyleBackColor = false;
			this->overwrite5->AutoSize = true;
			this->overwrite5->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite5->Checked = true;
			this->overwrite5->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite5->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite5->Location = System::Drawing::Point(632, 0);
			this->overwrite5->Name = L"overwrite5";
			this->overwrite5->Size = System::Drawing::Size(86, 27);
			this->overwrite5->TabIndex = 4;
			this->overwrite5->TabStop = true;
			this->overwrite5->Text = L"Overwrite";
			this->overwrite5->UseVisualStyleBackColor = false;
			this->label6->AutoSize = true;
			this->label6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label6->Dock = System::Windows::Forms::DockStyle::Left;
			this->label6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label6->ForeColor = System::Drawing::Color::Navy;
			this->label6->Location = System::Drawing::Point(0, 0);
			this->label6->Name = L"label6";
			this->label6->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label6->Size = System::Drawing::Size(221, 21);
			this->label6->TabIndex = 2;
			this->label6->Text = L"Existing files are older:";
			this->files5->AutoSize = true;
			this->files5->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files5->Dock = System::Windows::Forms::DockStyle::Top;
			this->files5->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files5->Location = System::Drawing::Point(0, 414);
			this->files5->Name = L"files5";
			this->files5->Padding = System::Windows::Forms::Padding(30, 5, 0, 5);
			this->files5->Size = System::Drawing::Size(190, 44);
			this->files5->TabIndex = 14;
			this->files5->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			this->files6->AutoSize = true;
			this->files6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files6->Dock = System::Windows::Forms::DockStyle::Top;
			this->files6->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files6->Location = System::Drawing::Point(0, 485);
			this->files6->Name = L"files6";
			this->files6->Padding = System::Windows::Forms::Padding(30, 5, 0, 5);
			this->files6->Size = System::Drawing::Size(190, 44);
			this->files6->TabIndex = 16;
			this->files6->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			this->panel7->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel7->Controls->Add(this->skip6);
			this->panel7->Controls->Add(this->overwrite6);
			this->panel7->Controls->Add(this->label9);
			this->panel7->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel7->Location = System::Drawing::Point(0, 458);
			this->panel7->Name = L"panel7";
			this->panel7->Size = System::Drawing::Size(718, 27);
			this->panel7->TabIndex = 15;
			this->skip6->AutoSize = true;
			this->skip6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip6->Checked = true;
			this->skip6->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip6->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip6->Location = System::Drawing::Point(579, 0);
			this->skip6->Name = L"skip6";
			this->skip6->Size = System::Drawing::Size(53, 27);
			this->skip6->TabIndex = 5;
			this->skip6->TabStop = true;
			this->skip6->Text = L"Skip";
			this->skip6->UseVisualStyleBackColor = false;
			this->overwrite6->AutoSize = true;
			this->overwrite6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite6->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite6->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite6->Location = System::Drawing::Point(632, 0);
			this->overwrite6->Name = L"overwrite6";
			this->overwrite6->Size = System::Drawing::Size(86, 27);
			this->overwrite6->TabIndex = 4;
			this->overwrite6->Text = L"Overwrite";
			this->overwrite6->UseVisualStyleBackColor = false;
			this->label9->AutoSize = true;
			this->label9->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label9->Dock = System::Windows::Forms::DockStyle::Left;
			this->label9->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label9->ForeColor = System::Drawing::Color::Navy;
			this->label9->Location = System::Drawing::Point(0, 0);
			this->label9->Name = L"label9";
			this->label9->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label9->Size = System::Drawing::Size(226, 21);
			this->label9->TabIndex = 2;
			this->label9->Text = L"Existing files are newer:";
			this->files7->AutoSize = true;
			this->files7->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->files7->Dock = System::Windows::Forms::DockStyle::Top;
			this->files7->Font = (gcnew System::Drawing::Font(L"Courier New", 10));
			this->files7->Location = System::Drawing::Point(0, 556);
			this->files7->Name = L"files7";
			this->files7->Padding = System::Windows::Forms::Padding(30, 5, 0, 5);
			this->files7->Size = System::Drawing::Size(190, 44);
			this->files7->TabIndex = 18;
			this->files7->Text = L"C:\\Some file\\\r\nC:\\Some other file\\";
			this->panel8->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->panel8->Controls->Add(this->skip7);
			this->panel8->Controls->Add(this->overwrite7);
			this->panel8->Controls->Add(this->label11);
			this->panel8->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel8->Location = System::Drawing::Point(0, 529);
			this->panel8->Name = L"panel8";
			this->panel8->Size = System::Drawing::Size(718, 27);
			this->panel8->TabIndex = 17;
			this->skip7->AutoSize = true;
			this->skip7->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->skip7->Dock = System::Windows::Forms::DockStyle::Right;
			this->skip7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->skip7->ForeColor = System::Drawing::Color::DarkGreen;
			this->skip7->Location = System::Drawing::Point(579, 0);
			this->skip7->Name = L"skip7";
			this->skip7->Size = System::Drawing::Size(53, 27);
			this->skip7->TabIndex = 5;
			this->skip7->Text = L"Skip";
			this->skip7->UseVisualStyleBackColor = false;
			this->overwrite7->AutoSize = true;
			this->overwrite7->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->overwrite7->Checked = true;
			this->overwrite7->Dock = System::Windows::Forms::DockStyle::Right;
			this->overwrite7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->overwrite7->ForeColor = System::Drawing::Color::DarkGreen;
			this->overwrite7->Location = System::Drawing::Point(632, 0);
			this->overwrite7->Name = L"overwrite7";
			this->overwrite7->Size = System::Drawing::Size(86, 27);
			this->overwrite7->TabIndex = 4;
			this->overwrite7->TabStop = true;
			this->overwrite7->Text = L"Overwrite";
			this->overwrite7->UseVisualStyleBackColor = false;
			this->label11->AutoSize = true;
			this->label11->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(235)), static_cast<System::Int32>(static_cast<System::Byte>(235)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->label11->Dock = System::Windows::Forms::DockStyle::Left;
			this->label11->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label11->ForeColor = System::Drawing::Color::Navy;
			this->label11->Location = System::Drawing::Point(0, 0);
			this->label11->Name = L"label11";
			this->label11->Padding = System::Windows::Forms::Padding(15, 5, 40, 0);
			this->label11->Size = System::Drawing::Size(386, 21);
			this->label11->TabIndex = 2;
			this->label11->Text = L"Existing files have same date but different size:";
			this->group_a_label->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->group_a_label->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->group_a_label->Dock = System::Windows::Forms::DockStyle::Top;
			this->group_a_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->group_a_label->Location = System::Drawing::Point(0, 47);
			this->group_a_label->Name = L"group_a_label";
			this->group_a_label->Padding = System::Windows::Forms::Padding(15, 5, 0, 5);
			this->group_a_label->Size = System::Drawing::Size(718, 28);
			this->group_a_label->TabIndex = 19;
			this->group_a_label->Text = L"a)  REC files or large files";
			this->group_b_label->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(240)), static_cast<System::Int32>(static_cast<System::Byte>(240)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->group_b_label->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->group_b_label->Dock = System::Windows::Forms::DockStyle::Top;
			this->group_b_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->group_b_label->Location = System::Drawing::Point(0, 288);
			this->group_b_label->Name = L"group_b_label";
			this->group_b_label->Padding = System::Windows::Forms::Padding(15, 5, 0, 5);
			this->group_b_label->Size = System::Drawing::Size(718, 28);
			this->group_b_label->TabIndex = 20;
			this->group_b_label->Text = L"b) Other file types";
			this->checkBox2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->checkBox2->AutoSize = true;
			this->checkBox2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(255)), 
				static_cast<System::Int32>(static_cast<System::Byte>(230)));
			this->checkBox2->Checked = true;
			this->checkBox2->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->checkBox2->ForeColor = System::Drawing::Color::DarkGreen;
			this->checkBox2->Location = System::Drawing::Point(589, 349);
			this->checkBox2->Name = L"checkBox2";
			this->checkBox2->Size = System::Drawing::Size(129, 19);
			this->checkBox2->TabIndex = 21;
			this->checkBox2->Text = L"Delete the PC copy";
			this->checkBox2->UseVisualStyleBackColor = false;
			this->checkBox2->Visible = false;
			this->checkBox2->CheckedChanged += gcnew System::EventHandler(this, &OverwriteConfirmation::checkBox2_CheckedChanged);
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)), 
				static_cast<System::Int32>(static_cast<System::Byte>(255)));
			this->ClientSize = System::Drawing::Size(718, 648);
			this->Controls->Add(this->checkBox2);
			this->Controls->Add(this->files7);
			this->Controls->Add(this->panel8);
			this->Controls->Add(this->files6);
			this->Controls->Add(this->panel7);
			this->Controls->Add(this->files5);
			this->Controls->Add(this->panel6);
			this->Controls->Add(this->files4);
			this->Controls->Add(this->panel5);
			this->Controls->Add(this->group_b_label);
			this->Controls->Add(this->checkBox1);
			this->Controls->Add(this->panel4);
			this->Controls->Add(this->files3);
			this->Controls->Add(this->panel3);
			this->Controls->Add(this->files2);
			this->Controls->Add(this->panel2);
			this->Controls->Add(this->files1);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->group_a_label);
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
			this->panel5->ResumeLayout(false);
			this->panel5->PerformLayout();
			this->panel6->ResumeLayout(false);
			this->panel6->PerformLayout();
			this->panel7->ResumeLayout(false);
			this->panel7->PerformLayout();
			this->panel8->ResumeLayout(false);
			this->panel8->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void OverwriteConfirmation_Shown(System::Object^  sender, System::EventArgs^  e) {
				 return;
				 this->Height = this->panel1->Visible * (this->panel1->Height+this->files1->Height+1) + 
					 this->panel2->Visible*( this->panel2->Height+this->files2->Height  +1) + 
					 this->panel3->Visible*( this->panel3->Height+this->files3->Height+1) + 
					 this->panel4->Height  +  this->title_label->Height + 48;

			 }
			 int visible_height(array<Panel^>^ panels, array<Label^>^ labels, array<int>^ num_cat)
			 {
				 int h=this->panel4->Height  +  this->title_label->Height+48;
				 for (int j=0; j<panels->Length; j++)
				 {
					 h +=   (num_cat[j]>0 )  * ( panels[j]->Height + labels[j]->Height);
				 }
				 int num_a = num_cat[0]+num_cat[1]+num_cat[2];
				 int num_b = num_cat[3]+num_cat[4]+num_cat[5]+num_cat[6];
				 if (num_a > 0 && num_b > 0)
					 h += (this->group_a_label->Height + this->group_b_label->Height);
				 return h;
			 }
	private: System::Void skip1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

				 if (this->skip1->Checked) 
					 this->checkBox1->Enabled=true;
				 else
					 this->checkBox1->Enabled=false;
			 }
	private: System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 this->checkBox2->Checked = this->checkBox1->Checked; 
			 }
	private: System::Void checkBox2_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
				 this->checkBox1->Checked = this->checkBox2->Checked; 

			 }
	};
}
