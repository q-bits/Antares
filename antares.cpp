// antares.cpp : main project file.

#include "stdafx.h"
#include "Form1.h"
#include "copydialog.h"
#include "Settings.h"
#include "antares.h"
#include "commandline.h"

extern "C" 
{
#include <time.h>
#include <types.h>
//	extern int verbose;
//	extern int packet_trace;
}


#include <stdio.h>

using namespace Antares;
using namespace System;
using namespace System::Text;
using namespace System::Threading;
using namespace System::Xml;


// These namespaces for testing stdout redirection to original terminal
using namespace Microsoft::Win32::SafeHandles;
using namespace System::Diagnostics;

FILE old_stdout;
FILE old_stderr;
FILE* hf;

namespace Antares {



	void disable_sleep_mode(void)
	{

		try
		{
			SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
		}
		catch(...){}
	}

	void enable_sleep_mode(void)
	{
		try
		{
			SetThreadExecutionState(ES_CONTINUOUS);
		}
		catch(...){}

	}


	String^ safeString( String^ filename_str)
	{
		int i, dest_i, len;
		System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder(filename_str);
		len = filename_str->Length;
		wchar_t c;
		for (i=0, dest_i=0;i<len; i++)
		{
			c = sb[i];
			if (c<32) continue; //c='_';
			if (c==':') c='-';
			if (c=='\"') c='\'';
			if (c=='<' || c=='>' || c=='/' || c=='|' || c=='?' || c=='*')  c='_';
			sb[dest_i]=c;
			dest_i++;
		}
		if (dest_i < len && dest_i>0) sb->Length = dest_i;
		return sb->ToString();
	}

	String^ safeString( char* filename )
	{
		return safeString(gcnew System::String(filename));
	}


	System::DateTime Time_T2DateTime(time_t t)
	{
		//tf_datetime ttt;
		//ttt.mjd=0;
		//t=tfdt_to_time(&ttt); printf("time_t = %uld\n",t);
		struct tm *newtime;
		newtime = localtime(&t);
		DateTime datetime;
		if (newtime==NULL)
			datetime = DateTime(1999, 9, 9, 9, 9, 9, System::DateTimeKind::Local);
		else 
			datetime = DateTime(1900+newtime->tm_year, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_min, System::DateTimeKind::Local);
		//datetime = datetime.ToLocalTime();
		//TimeSpan delta = TimeSpan::FromSeconds( (double) t);
		//datetime=datetime + delta; 
		return datetime;
	}


	/*
	String^ DateString(time_t time)
	{
		struct tm *newtime;
		char str[100];
		newtime = localtime(&time);
		sprintf_s(str,99,"%4d - %02d - %02d    %02d:%02d",1900+newtime->tm_year,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,newtime->tm_min);
		System::String ^s = gcnew System::String(str);

		return s;

	}
	*/

	String^ DateString(DateTime time)
	{


		return time.ToString("g"); 
		/*
		char str[100];
		sprintf_s(str,99,"%4d - %02d - %02d    %02d:%02d",time.Year,time.Month,time.Day,time.Hour,time.Minute);
		System::String ^s = gcnew System::String(str);
		return s;
		*/
		
	}

	String^ combineTopfieldPath(String^ path1,  String^ path2)
	{

		String^ path;

		path = path1; 
		if (path->Length == 0 || !path->EndsWith("\\"))
			path = path + "\\";

		path = path + path2;

		return path;
	}


	time_t DateTimeToTime_T(DateTime datetime)
	{
		DateTime startTime = DateTime(1970,1,1,0,0,0,0,System::DateTimeKind::Utc);
		TimeSpan dt = datetime - startTime;   

		time_t t = (time_t) (dt.TotalSeconds);
		//printf("\n  time_t=%d  totalseconds=%g   \n",(int) t, dt.TotalSeconds);
		//Console::WriteLine(datetime.ToString());
		return t;

	}
	// Compute the containing directory and the filename of the specified absolute filename path
	array<String^>^ TopfieldFileParts(String^ filename)
	{
		array<String^>^ out = {"",""};
		try
		{

			while(filename->EndsWith("\\"))   // remove trailing backslash
				filename=filename->Substring(0,filename->Length - 1);      
			int index=-1;
			bool last_slash=false;
			for (int j=0; j<filename->Length; j++)
			{
				String^ c  = filename->Substring(j,1);
				bool is_slash = String::Compare(c, "\\")==0;
				if (is_slash && !last_slash)
					index = j;
				last_slash=is_slash;
			}        
			if (index>=0) 
			{
				if (index>0) out[0]=filename->Substring(0,index);
				if (index+1 < filename->Length)
					out[1]=filename->Substring(index+1);	
			}
			return out;
		}
		catch(...)
		{
			return out;
		}
	}

	// Compute the containing directory of the specified absolute filename
	String^ ComputeTopfieldUpDir(String^ filename)
	{
		return TopfieldFileParts(filename)[0];
	}


	String^ HumanReadableSize(__u64 size)
	{
		//char str[100];
		double x = (double) size;
		int j;

		System::String^ s = "";
		for (j=1; j<=5; j++)
		{
			if (x<1024)
			{
				if (x<999)
					//sprintf_s(str,99,"%5.3g",x);
					s = x.ToString("G3");
				else
				s=x.ToString("###0");
					//sprintf_s(str,99,"%3d",(int) x);
				break;
			}
			x=x/1024;
		}
		//System::String^ s = gcnew System::String(str);
		switch(j)
		{
		case 1:
			s = s + " "+lang::u_bytes;//" bytes";
			break;
		case 2:
			s = s + " "+lang::u_kb;//" KB";
			break;
		case 3:
			s = s + " "+lang::u_mb;//" MB";
			break;
		case 4:
			s = s + " "+lang::u_gb;//" GB";
			break;
		case 5:
			s = s + " "+lang::u_tb;//" TB";
		}
		return s;
	}

}


void InitConsoleHandles()
{
	//SafeFileHandle ^hStdOut, ^hStdErr, ^hStdOutDup, ^hStdErrDup;
	HANDLE hStdOut, hStdErr, hStdOutDup, hStdErrDup;

	BY_HANDLE_FILE_INFORMATION bhfi;

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdErr = GetStdHandle(STD_ERROR_HANDLE);

	// Get current process handle
	HANDLE hProcess = (HANDLE) (void*) Process::GetCurrentProcess()->Handle;
	// Duplicate Stdout handle to save initial value
	DuplicateHandle(  hProcess, hStdOut, hProcess, &hStdOutDup,
		0, true, DUPLICATE_SAME_ACCESS);
	// Duplicate Stderr handle to save initial value
	DuplicateHandle(hProcess, hStdErr, hProcess, &hStdErrDup,
		0, true, DUPLICATE_SAME_ACCESS);
	// Attach to console window – this may modify the standard handles
	AttachConsole(ATTACH_PARENT_PROCESS);
	// Adjust the standard handles
	if (GetFileInformationByHandle(GetStdHandle(STD_OUTPUT_HANDLE), &bhfi))
	{
		SetStdHandle(STD_OUTPUT_HANDLE, hStdOutDup);
	}
	else
	{
		SetStdHandle(STD_OUTPUT_HANDLE, hStdOut);
	}
	if (GetFileInformationByHandle(GetStdHandle(STD_ERROR_HANDLE), &bhfi))
	{
		SetStdHandle(STD_ERROR_HANDLE, hStdErrDup);
	}
	else
	{
		SetStdHandle(STD_ERROR_HANDLE, hStdErr);
	}
}



[STAThreadAttribute]
int main(array<System::String ^> ^args)
//int main(void)

{


	if (args->Length > 0)
	{
		//InitConsoleHandles();


		//Console::CursorTop = Console::CursorTop-2;
		///Console::SetIn(TextReader::Null);
		//printf("Hi There!\n");
		//fprintf(stderr,"Error something\n");
		//System::Console::WriteLine("Hello again!\n");
		//Console::WindowHeight = 10;

		//for each (String^ str in args)
		//{
		//	printf("Argument: %s\n",str);
		//}
		//printf("cmdline: %s\n",Environment::CommandLine);

		




		//return 0;
	}
	else
	{
#ifndef _DEBUG
		FreeConsole();
#endif
	}

	

	//Thread::Sleep(20000);

	libusb_init(NULL);

	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 


	int hCrt, i;




#ifdef _DEBUG____
	AllocConsole(); 

	hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	hf = _fdopen( hCrt, "w" );

	old_stdout = *stdout;
	old_stderr = *stderr;

	*stdout = *hf;
	i = setvbuf( stdout, NULL, _IONBF, 0 ); 
	*stderr = *hf;

	printf("Test\n");
	fprintf(stderr,"Test stderr \n");




#endif




	/*
	bool ^x;
	bool ^y=gcnew System::Boolean;
	x=y;
	*x=false;
	Console::WriteLine(x->ToString() + " " + y->ToString());
	*x=true;

	Console::WriteLine(x->ToString() + " " + y->ToString());
	*y=false;
	Console::WriteLine(x->ToString() + " " + y->ToString());
	*y=true;
	Console::WriteLine(x->ToString() + " " + y->ToString());
	*/

	lang::set_en_au();

	//Console::WriteLine(System::Globalization::CultureInfo::CurrentCulture->Name);
	//Console::WriteLine(System::Globalization::CultureInfo::InstalledUICulture->Name);
	//Console::WriteLine(System::Globalization::CultureInfo::CurrentUICulture->Name);

	// Create the main window and run it
	Form1^ form = gcnew Form1();
	//form->copydialog = gcnew CopyDialog();


	if (form->commandline->dont_free_console)
	InitConsoleHandles();


	Settings ^settings = form->settings;

	//form->Focus();
	//form->textBox1->Select(0,0);
	//Console::WriteLine(form->Visible);
	//Console::WriteLine(form->WindowState);
	//Application::Run(form);

	//form->Form1_Load(nullptr,nullptr);
	//Console::WriteLine("Application::Run");


	if (form->commandline->showgui)
		Application::Run(form);
	else
		Application::Run();
	//Console::WriteLine("End.");
	form->topfield_background_enumerator=nullptr;


    if (nullptr == settings->backup_dic)
		settings->writeXmlSettings();

	if (form->fd != NULL)
	{
		//disconnect_device(form->fd);
		//TODO: write proper disconnect code
	}
	form->cbthread->Abort();
	form->tbthread->Abort();
	libusb_exit(NULL);





	return 0;
}
