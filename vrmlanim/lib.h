
#define MAXLIB 8192    // max number of names in scene. (limit is 8192)
#define MAX_NAME_LENGTH 100

#define WM_VRML1 WM_USER+3
#define WM_VRML2 WM_USER+4

#include <io.h>
#include <fcntl.h>
#include <math.h>
#include <SYS\STAT.H>
static char vrml_lib[MAXLIB][MAX_NAME_LENGTH];
char fname[500];
static HWND VRMLWND;

// File Handle
int fid;

// Get the handle to the VRML window from a temp file.
static bool ScanHWND()
{
	int n=50;
	while ((VRMLWND=FindWindow("#32770","VRML Animation interface for MATLAB/Simulink"))==0 && n>0 ) 
		{Sleep(100);n--;}
	if (n>0)
		return true;
	else
		return false;
}

// Collect and index the node names in the VRML file
static int ScanWRL(const char *_fn)
{
	int i = 0, j=0;
	int index = 0;
	CString Buffer;
	CFile fid;
	char defname[200];
	if (!fid.Open(_fn,CFile::modeRead)) return false;
	fid.Read(Buffer.GetBufferSetLength((int)fid.GetLength()),(int)fid.GetLength());
	fid.Close();
	Buffer.ReleaseBuffer();
	
	while (((i=Buffer.Find("DEF ",i))>0) && (index<MAXLIB)) {
		while ((Buffer[i]!=' ')&&(Buffer[i]!='\t')) i++;j=0;
		while ((Buffer[i]==' ')||(Buffer[i]=='\t')) i++;
		while ((Buffer[i]!=' ')&&(Buffer[i]!='\t')) defname[j++] = Buffer[i++];defname[j]=0;
		strcpy_s(vrml_lib[index],MAX_NAME_LENGTH,defname);
		i+=5;index++;
	}
	Buffer.Empty();
	return true;
}

// Find the index for a DEF node name in the file (if there is)
static int FindLib(const char* Name)
{
	int i = 0;
	while (strcmp(Name,vrml_lib[i]) & (i<MAXLIB)) i++;
	if (i==MAXLIB) return -1;
	else return i;
}

// Send the two Windows messages to the VRMLANIM
static void SendVRMLData(int Op, char *Name, const double U[])
{
	WPARAM wParam;
	LPARAM lParam;
	float Value;
	double Norm, Data[3];

	// Make sure that Data has length 1 for Op=6 (quaternion)
	if (Op==6) {
		Norm = sqrt( U[0]*U[0] + U[1]*U[1] ); 
		if (Norm>1) {
			Data[0] = U[0]/Norm;
			Data[1] = U[1]/Norm;
			Data[2] = U[2];
		}
		else {
			Data[0] = U[0];
			Data[1] = U[1];
			Data[2] = U[2];
		}
	} else {
		Data[0] = U[0];
		Data[1] = U[1];
		Data[2] = U[2];
	}

	// first message is operation and data(0)
	wParam = (int) Op;
	wParam += (int) FindLib(Name) * 8;              // wParam is the node name
	Value = (float) Data[0];
	memcpy( &lParam, &Value, sizeof(LPARAM) );     // lParam is the input value (x,y,z)
	::SendMessage( VRMLWND, WM_VRML1, wParam, lParam );

	// second message is data(1) and data(2) and data(3)
	Value = (float) Data[1];
	memcpy( &wParam, &Value, sizeof(WPARAM) );     // lParam is the input value (x,y,z)
	Value = (float) Data[2];
	memcpy( &lParam, &Value, sizeof(LPARAM) );     // lParam is the input value (x,y,z)

	::SendMessage( VRMLWND, WM_VRML2, wParam, lParam );
}
