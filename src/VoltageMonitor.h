/* 
* Copyright 2014-2015 Friedemann Zenke
*
* This file is part of Auryn, a simulation package for plastic
* spiking neural networks.
* 
* Auryn is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* Auryn is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with Auryn.  If not, see <http://www.gnu.org/licenses/>.
*
* If you are using Auryn or parts of it for your work please cite:
* Zenke, F. and Gerstner, W., 2014. Limits to high-speed simulations 
* of spiking neural networks using general-purpose computers. 
* Front Neuroinform 8, 76. doi: 10.3389/fninf.2014.00076
*/

#ifndef VOLTAGEMONITOR_H_
#define VOLTAGEMONITOR_H_

#define VOLTAGEMONITOR_PASTED_SPIKE_HEIGHT 20e-3

#include "auryn_definitions.h"
#include "Monitor.h"
#include "System.h"
#include "Connection.h"
#include <fstream>
#include <iomanip>

using namespace std;

/*! \brief Records the membrane potential from one unit from the source neuron group to a file. 
 *
 * The Monitor records the membrane potential of a single cell from a NeuronGroup. Per default
 * the timestep is simulator precision and the Monitor pastes a spike of height 
 * VOLTAGEMONITOR_PASTED_SPIKE_HEIGHT by reading out the the current spikes. For 
 * performance it is adviced to use the StateMonitor which does not do any
 * spike pasting. */
class VoltageMonitor : protected Monitor
{
private:
	/*! Global neuron id to record from */
	NeuronID gid;


protected:
	/*! The source neuron group to record from */
	NeuronGroup * src;

	/*! The source neuron id to record from */
	NeuronID nid;

	/*! The step size (sampling interval) in units of dt */
	AurynTime ssize;

	/*! Standard initialization */
	void init(NeuronGroup * source, NeuronID id, string filename, AurynTime stepsize);
	
public:
	/*! Paste spikes switch (default = true) */
	AurynTime paste_spikes;

	/*! Defines the maximum recording time in AurynTime to save space. */
	AurynTime tStop;

	VoltageMonitor(NeuronGroup * source, NeuronID id, string filename,  AurynDouble stepsize=dt);
	virtual ~VoltageMonitor();
	void propagate();
};

#endif /*VOLTAGEMONITOR_H_*/
