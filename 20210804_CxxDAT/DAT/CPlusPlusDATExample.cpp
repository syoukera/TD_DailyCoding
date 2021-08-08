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

#include "CPlusPlusDATExample.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <array>
#include <random>
#include <float.h>
#include <cmath>

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillDATPluginInfo(DAT_PluginInfo *info)
{
	// Always return DAT_CPLUSPLUS_API_VERSION in this function.
	info->apiVersion = DATCPlusPlusAPIVersion;

	// The opType is the unique name for this TOP. It must start with a
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Customdat");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Custom DAT");

	// Will be turned into a 3 letter icon on the nodes
	info->customOPInfo.opIcon->setString("CDT");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Author Name");
	info->customOPInfo.authorEmail->setString("email@email.com");

	// This DAT works with 0 or 1 inputs
	info->customOPInfo.minInputs = 0;
	info->customOPInfo.maxInputs = 1;

}

DLLEXPORT
DAT_CPlusPlusBase*
CreateDATInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per DAT that is using the .dll
	return new CPlusPlusDATExample(info);
}

DLLEXPORT
void
DestroyDATInstance(DAT_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the DAT using that instance is deleted, or
	// if the DAT loads a different DLL
	delete (CPlusPlusDATExample*)instance;
}

};

CPlusPlusDATExample::CPlusPlusDATExample(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;

	myChop = "";

	myChopChanName = "";
	myChopChanVal = 0;
}

CPlusPlusDATExample::~CPlusPlusDATExample()
{
}

void
CPlusPlusDATExample::getGeneralInfo(DAT_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = false;
}

void
CPlusPlusDATExample::makeTable(DAT_Output* output, int numVoids, int numVals)
{
	output->setOutputDataType(DAT_OutDataType::Table);
	output->setTableSize(numVoids+1, numVals);

	std::array<const char*, 6> columns = { "tx", "ty", "tz", "vx", "vy", "vz"};

	{
		int i = 0;
		for (int j = 0; j < numVals; ++j)
		{
			output->setCellString(i, j, columns[j]);
		}
	}

	for (int i = 0; i < numVoids; i++)
	{
		int i2 = i + 1;
		for (int j = 0; j < numVals; j++)
		{
			if (j < 3)
				output->setCellDouble(i2, j, x[i][j]);
			else
				output->setCellDouble(i2, j, v[i][j-3]);
		}
	}
}

void
CPlusPlusDATExample::makeText(DAT_Output* output)
{
	output->setOutputDataType(DAT_OutDataType::Text);
	output->setText("This is some test data.");
}

void
CPlusPlusDATExample::initializeVoids()
{
    std::mt19937 mt{ std::random_device{}() };
    std::uniform_real_distribution<double> dist(0.0, 1.0);

	for (int i = 0; i != numVoids; ++i)
	{
		x[i][0] = dist(mt)*2.0 - 1.0;
		x[i][1] = dist(mt)*2.0 - 1.0;
		x[i][2] = dist(mt)*2.0 - 1.0;
	}

	for (int i = 0; i != numVoids; ++i)
	{
		v[i][0] = (dist(mt)*2.0 - 1.0)*minVelocity;
		v[i][1] = (dist(mt)*2.0 - 1.0)*minVelocity;
		v[i][2] = (dist(mt)*2.0 - 1.0)*minVelocity;
	}
}

void
CPlusPlusDATExample::updateVoids()
{

	double x_coh[3] = {0.0, 0.0, 0.0};
	double x_sep[3] = {0.0, 0.0, 0.0};
	double x_ali[3] = {0.0, 0.0, 0.0};

	int count_coh = 0;
	int count_sep = 0;
	int count_ali = 0;

	for (int i = 0; i < numVoids; ++i)
	{
		double x_this[3] = {x[i][0], x[i][1], x[i][2]};
		double v_this[3] = {v[i][0], v[i][1], v[i][2]};

		for (int j = 0; j < numVoids; ++j)
		{
			if (j == i)
				distance[j] = FLT_MAX;
			else
				distance[j] = sqrt(
					std::pow(x_this[0] - x[j][0], 2) 
				  + std::pow(x_this[1] - x[j][1], 2)
				  + std::pow(x_this[2] - x[j][2], 2)
				);

			if (j == i)
				angle[j] = FLT_MAX;
			else
				angle[j] = acos(
					(
						v_this[0]*(x[j][0] - x_this[0])
					  + v_this[1]*(x[j][1] - x_this[1])
					  + v_this[2]*(x[j][2] - x_this[2])
					)
					/sqrt(
						std::pow(v_this[0], 2) 
					  + std::pow(v_this[1], 2) 
					  + std::pow(v_this[2], 2)
					)
					/distance[j]
				);
		}

		for (int j = 0; j < numVoids; ++j)
		{
			if (distance[j] < cohesionDistance && angle[j] < cohesionAngle)
			{
				for (int k = 0; k < 3; ++k)
				{
					x_coh[k] += x[j][k];
				}
				count_coh++;
			}
			
			if (distance[j] < separationDistance && angle[j] < separationAngle)
			{
				for (int k = 0; k < 3; ++k)
				{
					x_sep[k] += x_this[k] - x[j][k];
				}
				count_sep++;
			}
			
			if (distance[j] < alignmentDistance && angle[j] < alignmentAngle)
			{
				for (int k = 0; k < 3; ++k)
				{
					x_ali[k] += x[j][k];
				}
				count_ali++;
			}
		}

		for (int k = 0; k < 3; ++k)
		{
			// get average
			if (count_coh != 0)
			{
				x_coh[k] /= count_coh;
				x_coh[k] -= x_this[k];
			}
			if (count_ali != 0)
			{
				x_ali[k] /= count_ali;
				x_ali[k] -= x_this[k];
			}
		}

		double dist_center = sqrt(
				std::pow(x_this[i], 2) 
			  + std::pow(x_this[i], 2) 
			  + std::pow(x_this[i], 2)
			);

		for (int k = 0; k < 3; ++k)
		{
			v[i][k] += cohesionForce*x_coh[k];
			v[i][k] += separationForce*x_sep[k];
			v[i][k] += alignmentForce*x_ali[k];
		}

		if (dist_center > 1.0)
		{
			for (int k = 0; k < 3; ++k)
			{
				v[i][k] -= boundaryForce*x_this[k]*(dist_center - 1)/dist_center;
			}
		}

		double v_abs = sqrt(
			std::pow(v[i][0], 2) 
		  + std::pow(v[i][1], 2) 
		  + std::pow(v[i][2], 2)
		);

		if (v_abs < minVelocity)
			for (int k = 0; k < 3; ++k)
				v[i][k] = minVelocity*v[i][k]/v_abs;
		else if (v_abs > maxVelocity)
			for (int k = 0; k < 3; ++k)
				v[i][k] = maxVelocity*v[i][k]/v_abs;

		for (int k = 0; k < 3; ++k)
			x[i][k] += v[i][k];
	}
}

void
CPlusPlusDATExample::execute(DAT_Output* output,
							const OP_Inputs* inputs,
							void* reserved)
{
	myExecuteCount++;

	if (!output)
		return;
	
	inputs->enablePar("Voids", 1);
	inputs->enablePar("Maxvel", 1);
	inputs->enablePar("Minvel", 1);

	int numVoids = inputs->getParInt("Voids");
	this->maxVelocity = inputs->getParDouble("Maxvel");
	this->minVelocity = inputs->getParDouble("Minvel");
	this->cohesionForce = inputs->getParDouble("Cohforce");
	this->separationForce = inputs->getParDouble("Sepforce");
	this->alignmentForce = inputs->getParDouble("Aliforce");
	this->boundaryForce = inputs->getParDouble("Bdrforce");
	this->cohesionDistance = inputs->getParDouble("Cohdist");
	this->separationDistance = inputs->getParDouble("Sepdist");
	this->alignmentDistance = inputs->getParDouble("Alidist");

	if (numVoids != this->numVoids) {
		this->numVoids = numVoids;
		this->initializeVoids();
	}

	// this->maxVelocity = maxVel;
	// this->minVelocity = minVel;

	this->updateVoids();

	makeTable(output, numVoids, 6);

}

int32_t
CPlusPlusDATExample::getNumInfoCHOPChans(void* reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 4;
}

void
CPlusPlusDATExample::getInfoCHOPChan(int32_t index,
									OP_InfoCHOPChan* chan, void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}

	if (index == 2)
	{
		chan->name->setString(myChop.c_str());
		chan->value = (float)myOffset;
	}

	if (index == 3)
	{
		chan->name->setString(myChopChanName.c_str());
		chan->value = myChopChanVal;
	}
}

bool
CPlusPlusDATExample::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 3;
	infoSize->cols = 3;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
CPlusPlusDATExample::getInfoDATEntries(int32_t index,
									int32_t nEntries,
									OP_InfoDATEntries* entries,
									void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "executeCount");
#else // macOS
		strlcpy(tempBuffer, "executeCount", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "offset");
#else // macOS
		strlcpy(tempBuffer, "offset", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 2)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "DAT input name");
#else // macOS
		strlcpy(tempBuffer, "offset", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		strcpy_s(tempBuffer, myDat.c_str());
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(tempBuffer);
	}
}

void
CPlusPlusDATExample::setupParameters(OP_ParameterManager* manager, void* reserved1)
{

	// Number of Voids
	{
		OP_NumericParameter	np;

		np.name = "Voids";
		np.label = "Voids";
		np.defaultValues[0] = 30;
		np.minSliders[0] = 1;
		np.maxSliders[0] = 300;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Maximum velocity
	{
		OP_NumericParameter	np;

		np.name = "Maxvel";
		np.label = "Maximum Velocity";
		np.defaultValues[0] = 0.3;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Minimum velocity
	{
		OP_NumericParameter	np;

		np.name = "Minvel";
		np.label = "Minimum Velocity";
		np.defaultValues[0] = 0.01;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
	
	// Cohesion Force
	{
		OP_NumericParameter	np;

		np.name = "Cohforce";
		np.label = "Cohesion Force";
		np.defaultValues[0] = 0.008;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
		
	// Separation Force
	{
		OP_NumericParameter	np;

		np.name = "Sepforce";
		np.label = "Separation Force";
		np.defaultValues[0] = 0.4;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
	
	// Alignment Force
	{
		OP_NumericParameter	np;

		np.name = "Aliforce";
		np.label = "Alignment Force";
		np.defaultValues[0] = 0.06;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
		
	// Boundary Force
	{
		OP_NumericParameter	np;

		np.name = "Bdrforce";
		np.label = "Boundary Force";
		np.defaultValues[0] = 0.06;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
		
	// Cohesion Distance
	{
		OP_NumericParameter	np;

		np.name = "Cohdist";
		np.label = "Cohesion Distance";
		np.defaultValues[0] = 0.5;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
		
	// Separation Distance
	{
		OP_NumericParameter	np;

		np.name = "Sepdist";
		np.label = "Separation Distance";
		np.defaultValues[0] = 0.05;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
			
	// Alignment Distance
	{
		OP_NumericParameter	np;

		np.name = "Alidist";
		np.label = "Alignmnet Distance";
		np.defaultValues[0] = 0.1;
		np.minSliders[0] = 0.0;
		np.maxSliders[0] = 1.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";

		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void
CPlusPlusDATExample::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}
