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

		String^ filename;

        System::Configuration::Configuration^ config;
		System::Configuration::KeyValueConfigurationCollection^ settings;

		Settings(void)
		{
			dic = gcnew Dictionary<String^,String^>();

			// Apply default settings
			dic->Add("ComputerDir","C:\\");
			dic->Add("TopfieldDir","\\DataFiles");
			dic->Add("PC_SortOrder","Ascending");
			dic->Add("PC_SortColumn","0");
			dic->Add("TurboMode","on");
			dic->Add("PVR_SortOrder","Ascending");
			dic->Add("PVR_SortColumn","0");

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

			filename = Path::Combine(folder, "antares.setttings.xml");


			Console::WriteLine("----------------" + filename + "---------------");

			loadOldSettings();
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
						Console::WriteLine(" -- Node --");
						while(r->MoveToNextAttribute())
						{
							Console::WriteLine( " Name = "+r->Name   +  "Value =" +r->Value);
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

		void changeSetting(String^ key,  String^ val)
		{
           if (dic->ContainsKey(key))
					dic[key]=val;
				else
					dic->Add(key, val);
		   Console::WriteLine("Key = "+key+", val = "+val);
		}

		String^ getSetting(String^ key)
		{
			if (dic->ContainsKey(key))
				return dic[key];
			else
				return gcnew String("");
		}

        String^ operator[] (String^ key)   //not to be used for assignment
		{
			return getSetting(key);
		}

	};


}