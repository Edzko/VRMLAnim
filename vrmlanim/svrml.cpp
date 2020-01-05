/*
 * RDDATA.CPP.
 *
 * G.E. Smid
 * Oakland University
 */

#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME svrml
#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#define u(element) (*uPtrs[element])  /* Pointer to Input Port0 */

#include "simstruc.h"
#include <afx.h>
#include "lib.h"

static bool initialized = false;
static bool DoAnimate = false;
static int DEC, dec = 0, FirstBlock[2];
static unsigned long VRMLANIMStyle;

static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 4);
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
		 ssSetErrorStatus(S,"Wrong number of parameters.");
        return;
    }
	 if (mxGetScalar(ssGetSFcnParam(S,0))==0) {
		mxGetString(ssGetSFcnParam(S,1),fname,500);  // (0) string represents WRL filename
		if (mxGetScalar(ssGetSFcnParam(S,2))==0) 
			DoAnimate = false;
		else 
			DoAnimate = true;
		 DEC = (int) mxGetScalar(ssGetSFcnParam(S,3));
	 }

    ssSetNumContStates( S, 0);
    ssSetNumDiscStates( S, 0);
	 ssSetNumInputPorts(S, 1);
    ssSetInputPortWidth( S, 0, 3);
    ssSetNumOutputPorts( S, 0);
    ssSetInputPortDirectFeedThrough( S, 0, 1);
    ssSetNumSampleTimes( S, 1); 
    ssSetNumRWork( S, 0);
    ssSetNumIWork( S, 0);
    ssSetNumPWork( S, 0);
    ssSetNumModes( S, 0);
    ssSetNumNonsampledZCs( S, 0); 
    ssSetOptions( S, 0);
}

static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, CONTINUOUS_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, 0.0);
}

void CloseVRMLANIM() {
	::SendMessage( VRMLWND, WM_VRML1, 0, 1 );
	VRMLWND = NULL;
}

#define MDL_INITIALIZE_CONDITIONS   
static void mdlInitializeConditions(SimStruct *S)
{
	char name[MAX_NAME_LENGTH];
	int nodeIndex;

	// Query the node names in the VRML file. Do only once.
	if (DoAnimate) {
		if (!initialized) {
			if (!ScanWRL(fname)) {
				ssSetErrorStatus(S,"Filename not found (see Command line)");
				return;
			}
			initialized = true;
		}
		if (mxGetScalar(ssGetSFcnParam(S,0))==0) {
			// Check for the current active window of VRMLANIM each time simulation starts
			if (!ScanHWND()) {
				ssSetErrorStatus(S,"Could not find VRML ANIMATION handle.");
				return;
			} else {
				// get the windows style parameter list
				//VRMLANIMStyle = ::GetWindowLong(VRMLWND,GWL_STYLE);
				// when Matlab exits, then remove Close VRMLANIM
				mexAtExit(CloseVRMLANIM);
			}
			// make sure that the window is visible
			//::SetWindowLong(VRMLWND,GWL_STYLE,VRMLANIMStyle | WS_VISIBLE);
			::SendMessage(VRMLWND,WM_VRML1,0,0);
		} else { // SFcnParam(S,0)>0, string in SFcnParam(S,1) represents node name
			mxGetString(ssGetSFcnParam(S,1),name,MAX_NAME_LENGTH);  
			nodeIndex = FindLib(name);
			if (nodeIndex<0) {
				mexPrintf("Node name \"%s\" not found in Geometry file.\n",name);
				ssSetErrorStatus(S,"Node name not found (see Command line)");
				return;
			}
			if (FirstBlock[0]==0 && FirstBlock[1]==0) {
				FirstBlock[0] = nodeIndex;
				FirstBlock[1] = (int) mxGetScalar(ssGetSFcnParam(S,0));
				dec = 1;
			}
		}
	} else 
		if (VRMLWND>0) // hide VRMLANIM if we do not want it.
			::SendMessage( VRMLWND, WM_VRML1, 0, 2);
}

static void mdlOutputs(SimStruct *S, int_T tid)
{
	InputRealPtrsType uPtrs = ssGetInputPortRealSignalPtrs(S,0);
	const double U[3] = {*uPtrs[0], *uPtrs[1], *uPtrs[2]};
	int Operation = (int) mxGetScalar(ssGetSFcnParam(S,0));
	char Name[MAX_NAME_LENGTH];
	mxGetString(ssGetSFcnParam(S,1),Name,MAX_NAME_LENGTH);

	if (dec==0) {
		// Operation = 0 for the block that has the WRL filename
		if ((Operation>0) && (DoAnimate)) {     
			SendVRMLData(Operation,Name,U);
		}
	}
	if ((FirstBlock[1]==Operation) && (FirstBlock[0]==FindLib(Name))) {
		dec--;
		if (dec<0) dec = DEC;
	}
}

static void mdlUpdate(SimStruct *S, int_T tid){}
static void mdlDerivatives(SimStruct *S, int_T tid){}

static void mdlTerminate(SimStruct *S)
{
	//initialized = false;
	FirstBlock[0] = 0;
	FirstBlock[1] = 0;
	/*
	if (DoAnimate) 
		// hide the window when simulation ends
		::SendMessage( VRMLWND, WM_VRML1, 0, 2);
	*/
}

#ifdef	MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
