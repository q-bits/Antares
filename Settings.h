#pragma once


namespace Antares {


	using namespace System;
	using namespace System::Collections::Generic;
	using namespace System::Configuration;

	using namespace System::IO;
	using namespace System::Xml;

	public ref class Settings
	{
	public:
		Dictionary<String^, String^> ^dic;

		Dictionary<String^, String^> ^backup_dic;

		String^ filename;

		static int maximum_history_length = 12;

        System::Configuration::Configuration^ config;
		System::Configuration::KeyValueConfigurationCollection^ settings;


		static array<String^> ^language_codes = {"auto", "en-gb",          "en-au",          "fi",    "de"     };
		static array<String^> ^language_names = {"auto", "English (UK)", "English (Aust.)", "suomi", "Deutsch"};

		void backup_settings(void)
		{
			backup_dic = gcnew Dictionary<String^,String^>(dic);
		}
		void restore_settings(void)
		{
			if (backup_dic != nullptr)
				dic = gcnew Dictionary<String^, String^>(backup_dic);
		}


		Settings(void)
		{
			dic = gcnew Dictionary<String^,String^>();
			backup_dic = nullptr;

			//////////////////////////
			// Apply default settings
			///////////////////////////
			dic->Add("ComputerDir","C:\\");
			dic->Add("TopfieldDir","\\DataFiles");
			dic->Add("PC_SortOrder","Ascending");
			dic->Add("PC_SortColumn","0");
			dic->Add("TurboMode","on");
			dic->Add("PVR_SortOrder","Ascending");
			dic->Add("PVR_SortColumn","0");

			dic->Add("PVR_Column0Visible","1"); // Name
			dic->Add("PVR_Column1Visible","1"); // Size 
			dic->Add("PVR_Column2Visible","0"); // Type
			dic->Add("PVR_Column3Visible","1"); // Date
			dic->Add("PVR_Column4Visible","1"); // Channel
			dic->Add("PVR_Column5Visible","0"); // Description

			dic->Add("PC_Column0Visible","1"); // Name
			dic->Add("PC_Column1Visible","1"); // Size 
			dic->Add("PC_Column2Visible","0"); // Type
			dic->Add("PC_Column3Visible","1"); // Date
			dic->Add("PC_Column4Visible","0"); // Channel
			dic->Add("PC_Column5Visible","0"); // Description

			dic->Add("Maximized","0");
			dic->Add("Width","1012");
			dic->Add("Height","666");

			dic->Add("RescaleColumns","1");

			dic->Add("PVR_Column0Width","190");
			dic->Add("PVR_Column1Width","65");
			dic->Add("PVR_Column2Width","60");
			dic->Add("PVR_Column3Width","120");
			dic->Add("PVR_Column4Width","60");
			dic->Add("PVR_Column5Width","140");

			dic->Add("PVR_ColumnScale","380");


			dic->Add("PC_Column0Width","190");
			dic->Add("PC_Column1Width","65");
			dic->Add("PC_Column2Width","60");
			dic->Add("PC_Column3Width","120");
			dic->Add("PC_Column4Width","60");
			dic->Add("PC_Column5Width","140");

			dic->Add("PC_ColumnScale","380");

			dic->Add("prevent_sleep_during_transfer","1");

			dic->Add("language","auto");   // auto, en-gb, en-au, fi, de

			dic->Add("prevent_turbo_while_recording","1");

			dic->Add("read_MyStuff","1");


			String ^folder = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData) , "Antares\\");
			
			if (!Directory::Exists(folder))
			{

				try{
					Directory::CreateDirectory(folder);
				}
				catch(...)
				{
					Console::WriteLine("Unhandled exception in Settings::Settings");
				}
			}

			filename = Path::Combine(folder, "antares.setttings.xml");   // Yes, I can see the typo, but I may as well stick with it now!

			//Console::WriteLine("----------------" + filename + "---------------");

			//loadOldSettings();
			readXmlSettings();

		}

		void readXmlSettings(void)
		{
            
			String ^key, ^val;
			try {
				XmlReader^ r = XmlReader::Create(filename);
				r->MoveToContent();
				r->ReadToDescendant("add");
				do {
					if (r->NodeType == XmlNodeType::Element)
					{
						key=""; val="";
						//Console::WriteLine(" -- Node --");
						while(r->MoveToNextAttribute())
						{
							//Console::WriteLine( " Name = "+r->Name   +  "Value =" +r->Value);
							if ( r->Name == "key") key = r->Value;
							else if ( r->Name == "value") val = r->Value;
						}
						if (key->Length >0)
						{
							if (dic->ContainsKey(key))
								dic[key]=val;
							else
								dic->Add(key,val);
						}
					}
				} while(r->Read());
               r->Close();
			}
			catch(...)
			{
				Console::WriteLine("Unhandled exception in readXmlSettings");
			}

		}


		void writeXmlSettings(void)
		{
			try {
			XmlWriterSettings^ writer_settings = gcnew XmlWriterSettings();
			writer_settings->Indent = true;
			XmlWriter^ xml = XmlWriter::Create(filename, writer_settings);
			xml->WriteStartDocument();
			xml->WriteStartElement("AntaresSettings");
			for each( KeyValuePair<String ^, String ^> kvp in dic )
			{
				// kvp.Key, kvp.Value);

				xml->WriteStartElement("add");
				xml->WriteAttributeString("key",kvp.Key);
				xml->WriteAttributeString("value",kvp.Value);
				xml->WriteEndElement();

			}
			xml->WriteEndElement();
			xml->WriteEndDocument();
			xml->Flush();
			xml->Close();
			
			}
			catch (...)
			{
				Console::WriteLine("Unhandled exception in writeXmlSettings");
			}
          
		}


       
		void copyOldSetting(String^ key)
		{
			if (nullptr != settings[key])
			{
				if (dic->ContainsKey(key))
					dic[key]=settings[key]->Value;
				else
					dic->Add(key, settings[key]->Value);
			}
			
		}

		/*
		// Load settings from an old-style config file, if present
		void loadOldSettings(void)
		{
			try{

				config =  ConfigurationManager::OpenExeConfiguration(System::Configuration::ConfigurationUserLevel::None);
				settings=config->AppSettings->Settings;

				array<String^> ^old_keys = {
					"TurboMode", "TopfieldDir", "ComputerDir", "PVR_SortColumn", "PC_SortColumn", "PVR_SortOrder", "PC_SortOrder"};

				for each (String^ str in old_keys)
					copyOldSetting(str);
	
			}
			catch(...)
			{
                 Console::WriteLine("Unhandled exception in loadOldSettings");
			}

		}
		*/

		void changeSetting(String^ key,  String^ val)
		{
           if (dic->ContainsKey(key))
					dic[key]=val;
				else
					dic->Add(key, val);
		   //Console::WriteLine("Key = "+key+", val = "+val);
		}

		void clearSetting(String^ key)
		{
			if (dic->ContainsKey(key))
				dic->Remove(key);
		}

		String^ getSetting(String^ key)
		{
			if (dic->ContainsKey(key))
				return dic[key];
			else
				return gcnew String("");
		}


	    String^ getSettingOrNull(String^ key)
		{
			if (dic->ContainsKey(key))
				return dic[key];
			else
				return nullptr;
		}



        String^ operator[] (String^ key)   //not to be used for assignment
		{
			return getSetting(key);
		}

	};


}