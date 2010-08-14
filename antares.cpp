// antares.cpp : main project file.

#include "stdafx.h"
#include "Form1.h"
#include "copydialog.h"

extern "C" 
{
	#include <time.h>
    #include <types.h>
}


#include <stdio.h>

using namespace Antares;
using namespace System;
using namespace System::Text;
using namespace System::Threading;
FILE old_stdout;
FILE old_stderr;
FILE* hf;




String^ safeString( String^ filename_str)
{
	int i;
	System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder(filename_str);
    wchar_t c;
    for (i=0;i<filename_str->Length; i++)
	{
		c = sb[i];
		if (c<32) c='_';
		if (c==':') c='-';
		if (c=='\"') c='\'';
		if (c=='<' || c=='>' || c=='/' || c=='\\' || c=='|' || c=='?' || c=='*')  c='_';
        sb[i]=c;
	}
    return sb->ToString();
}

String^ safeString( char* filename )
{
	return safeString(gcnew System::String(filename));
}


System::DateTime Time_T2DateTime(time_t t)
{
	struct tm *newtime;
	newtime = localtime(&t);
	DateTime datetime = DateTime(1900+newtime->tm_year, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_min, System::DateTimeKind::Local);
	//datetime = datetime.ToLocalTime();
	//TimeSpan delta = TimeSpan::FromSeconds( (double) t);
    //datetime=datetime + delta; 
    return datetime;
}


String^ DateString(time_t time)
{
	struct tm *newtime;
	char str[100];
	newtime = localtime(&time);
	sprintf_s(str,99,"%4d - %02d - %02d    %02d:%02d",1900+newtime->tm_year,newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,newtime->tm_min);
	System::String ^s = gcnew System::String(str);
	
	return s;
  
}

String^ DateString(DateTime time)
{
	//struct tm *newtime;
	char str[100];
	//newtime = localtime(&time);
	sprintf_s(str,99,"%4d - %02d - %02d    %02d:%02d",time.Year,time.Month,time.Day,time.Hour,time.Minute);
	System::String ^s = gcnew System::String(str);
	return s;
  
}

time_t DateTimeToTime_T(DateTime datetime)
{
	DateTime startTime = DateTime(1970,1,1,0,0,0,0,System::DateTimeKind::Utc);
    TimeSpan dt = datetime - startTime;   

    time_t t = (time_t) (dt.TotalSeconds);
	printf("\n  time_t=%d  totalseconds=%g   \n",(int) t, dt.TotalSeconds);
	Console::WriteLine(datetime.ToString());
    return t;

}


String^ HumanReadableSize(__u64 size)
{
	char str[100];
	double x=size;
	int j;
	
	for (j=1; j<=4; j++)
	{
		if (x<1024)
		{
			if (x<999)
              sprintf_s(str,99,"%5.3g",x);
			else
				sprintf_s(str,99,"%3d",(int) x);
		  break;
		}
		x=x/1024;
	}
	System::String^ s = gcnew System::String(str);
	switch(j)
	{
	case 1:
		s = s + " bytes";
		break;
	case 2:
		s = s + " KB";
		break;
	case 3:
		s = s + " MB";
		break;
	case 4:
		s = s + " GB";
		break;
	}
	return s;
}

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{


	//Thread::Sleep(20000);

	libusb_init(NULL);

	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 
	

	int hCrt, i;
	
	



#ifdef _DEBUG
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
    

	// Create the main window and run it
	Form1^ form = gcnew Form1();
	//form->copydialog = gcnew CopyDialog();


    
	Application::Run(form);

	if (form->fd != NULL)
	{
		disconnect_device(form->fd);
	}
	libusb_exit(NULL);




	return 0;
}
