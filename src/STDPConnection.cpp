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
*/

#include "STDPConnection.h"

void STDPConnection::init(AurynFloat eta, AurynFloat maxweight)
{
	if ( dst->get_post_size() == 0 ) return;

	tau_pre  = 20.0e-3;
	tau_post = 20.0e-3;

	A = eta; // post-pre
	B = eta; // pre-post

	logger->parameter("eta",eta);
	logger->parameter("A",A);
	logger->parameter("B",B);

	tr_pre  = src->get_pre_trace(tau_pre);
	tr_post = dst->get_post_trace(tau_post);

	set_min_weight(0.0);
	set_max_weight(maxweight);

	stdp_active = true;
}


void STDPConnection::finalize() {
	DuplexConnection::finalize();
}

void STDPConnection::free()
{
}

STDPConnection::STDPConnection(SpikingGroup * source, NeuronGroup * destination, TransmitterType transmitter) : DuplexConnection(source, destination, transmitter)
{
}

STDPConnection::STDPConnection(SpikingGroup * source, NeuronGroup * destination, 
		const char * filename, 
		AurynFloat eta, 
		AurynFloat maxweight , 
		TransmitterType transmitter) 
: DuplexConnection(source, 
		destination, 
		filename, 
		transmitter)
{
	init(eta, maxweight);
}

STDPConnection::STDPConnection(SpikingGroup * source, NeuronGroup * destination, 
		AurynWeight weight, AurynFloat sparseness, 
		AurynFloat eta, 
		AurynFloat maxweight , 
		TransmitterType transmitter,
		string name) 
: DuplexConnection(source, 
		destination, 
		weight, 
		sparseness, 
		transmitter, 
		name)
{
	init(eta, maxweight);
	if ( name.empty() )
		set_name("STDPConnection");
}

STDPConnection::~STDPConnection()
{
	if ( dst->get_post_size() > 0 ) 
		free();
}


AurynWeight STDPConnection::dw_pre(NeuronID post)
{
	NeuronID translated_spike = dst->global2rank(post); // only to be used for post traces
	AurynDouble dw = A*tr_post->get(translated_spike);
	return dw;
}

AurynWeight STDPConnection::dw_post(NeuronID pre)
{
	AurynDouble dw = B*tr_pre->get(pre);
	return dw;
}


void STDPConnection::propagate_forward()
{
	// loop over all spikes
	for (SpikeContainer::const_iterator spike = src->get_spikes()->begin() ; // spike = pre_spike
			spike != src->get_spikes()->end() ; ++spike ) {
		// loop over all postsynaptic partners
		for (const NeuronID * c = w->get_row_begin(*spike) ; 
				c != w->get_row_end(*spike) ; 
				++c ) { // c = post index

			// transmit signal to target at postsynaptic neuron
			AurynWeight * weight = w->get_data_ptr(c); 
			transmit( *c , *weight );

			// handle plasticity
			if ( stdp_active ) {
				// performs weight update
			    *weight += dw_pre(*c);

			    // clips too small weights
			    if ( *weight < get_min_weight() ) 
					*weight = get_min_weight();
			}
		}
	}
}

void STDPConnection::propagate_backward()
{
	if (stdp_active) { 
		SpikeContainer::const_iterator spikes_end = dst->get_spikes_immediate()->end();
		// loop over all spikes
		for (SpikeContainer::const_iterator spike = dst->get_spikes_immediate()->begin() ; // spike = post_spike
				spike != spikes_end ; 
				++spike ) {

			// loop over all presynaptic partners
			for (const NeuronID * c = bkw->get_row_begin(*spike) ; c != bkw->get_row_end(*spike) ; ++c ) {

				#ifdef CODE_ACTIVATE_PREFETCHING_INTRINSICS
				// prefetches next memory cells to reduce number of last-level cache misses
				_mm_prefetch((const char *)bkw->get_data(c)+2,  _MM_HINT_NTA);
				#endif

				// computes plasticity update
				AurynWeight * weight = bkw->get_data(c); 
				*weight += dw_post(*c);

				// clips too large weights
				if (*weight>get_max_weight()) *weight=get_max_weight();
			}
		}
	}
}

void STDPConnection::propagate()
{
	propagate_forward();
	propagate_backward();
}

void STDPConnection::evolve()
{
}

