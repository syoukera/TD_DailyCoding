/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "DAT_CPlusPlusBase.h"
#include <string>

/*
 This is a basic sample project to represent the usage of CPlusPlus DAT API.
 To get more help about these functions, look at DAT_CPlusPlusBase.h
*/

class CPlusPlusDATExample : public DAT_CPlusPlusBase
{
public:
	CPlusPlusDATExample(const OP_NodeInfo* info);
	virtual ~CPlusPlusDATExample();

	virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

	virtual void		execute(DAT_Output*,
								const OP_Inputs*,
								void* reserved) override;


	virtual int32_t		getNumInfoCHOPChans(void* reserved1) override;
	virtual void		getInfoCHOPChan(int index,
										OP_InfoCHOPChan* chan, 
										void* reserved1) override;

	virtual bool		getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1) override;
	virtual void		getInfoDATEntries(int32_t index,
											int32_t nEntries,
											OP_InfoDATEntries* entries,
											void* reserved1) override;

	virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
	virtual void		pulsePressed(const char* name, void* reserved1) override;

private:

	void				makeTable(DAT_Output* output, int numRows, int numCols);
	void				makeText(DAT_Output* output);

	void                initializeVoids();
	void                updateVoids();

	// We don't need to store this pointer, but we do for the example.
	// The OP_NodeInfo class store information about the node that's using
	// this instance of the class (like its name).
	const OP_NodeInfo*	myNodeInfo;

	// In this example this value will be incremented each time the execute()
	// function is called, then passes back to the DAT
	int32_t				myExecuteCount;

	double				myOffset;

	std::string         myChopChanName;
	float               myChopChanVal;
	std::string         myChop;

	std::string         myDat;

	static const int length_array = 1000;

	double x[length_array][3];
	double v[length_array][3];
	double distance[length_array];
	double angle[length_array];

	const double minVelocity = 0.005;
	const double maxVelocity = 0.03;

	const double cohesionForce = 0.008;
	const double separationForce = 0.4;
	const double alignmentForce = 0.06;

	const double cohesionDistance = 0.5;
	const double separationDistance = 0.05;
	const double alignmentDistance = 0.1;

	const double PI = 3.141592653589793;

	const double cohesionAngle = PI/2.0;
	const double separationAngle = PI/2.0;
	const double alignmentAngle = PI/3.0;
	
	int numVoids = 0;
	
};
