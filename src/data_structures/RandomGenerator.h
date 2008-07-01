/*
Highly Optimized Object-Oriented Molecular Dynamics (HOOMD) Open
Source Software License
Copyright (c) 2008 Ames Laboratory Iowa State University
All rights reserved.

Redistribution and use of HOOMD, in source and binary forms, with or
without modification, are permitted, provided that the following
conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names HOOMD's
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND
CONTRIBUTORS ``AS IS''  AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS  BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

// $Id$
// $URL$

/*! \file RandomGenerator.h
 	\brief Contains declarations for RandomGenerator and related classes.
 */

#include "ParticleData.h"

#include <string>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/random.hpp>

#ifndef __RANDOM_GENERATOR_H__
#define __RANDOM_GENERATOR_H__

//! Stores particles as they are generated in RandomGenerator
/*! \ingroup data_structs
	GeneratedParticles is a holding area where particles are stored when
	being generated by RandomGenerator and ParticleGenerator instances.
	
	It includes helper functions and data structures for placing particles
	that do not overlap. These helpers use the radius for each particle type
	as specified by setSeparationRadius(). Every particle type that will be 
	generated must be specified before generation can begin.	
	
	After all particles are placed in GeneratedParticles, RandomGenerator will
	then translate that data over to ParticleData in the initializer.
*/
class GeneratedParticles
	{
	public:
		struct particle
			{
			//! Default constructor
			particle() : x(0.0), y(0.0), z(0.0), type("") {}
			Scalar x,y,z;
			std::string type;
			unsigned int type_id;
			};
			
		//! Constructor
		GeneratedParticles(unsigned int n_particles, const BoxDim& box, const std::map< std::string, Scalar >& radii);
		//! Constructor
		GeneratedParticles() { }
		
		//! Check if a particle can be placed while obeying the separation radii
		bool canPlace(const particle& p);
		
		//! Place a particle
		void place(const particle& p, unsigned int idx);
		
		//! Undo the placement of a particle
		void undoPlace(unsigned int idx);
		
		//! Get the box
		const BoxDim& getBox() { return m_box; }
		
	private:
		friend class RandomGenerator;
	
		std::vector<particle> m_particles;					//!< The generated particles
		BoxDim m_box;										//!< Box the particles are in
		std::vector< std::vector<unsigned int> > m_bins;	//! Bins the particles are placed in for efficient distance checks
		int m_Mx;		//!< Number of bins in the x direction
		int m_My;		//!< Number of bins in the y direction
		int m_Mz;		//!< Number of bins in the z direction
		Scalar m_scalex;		//!< Scale factor to convert x to a bin coord
		Scalar m_scaley;		//!< Scale factor to convert y to a bin coord
		Scalar m_scalez;		//!< Scale factor to convert z to a bin coord
		std::map< std::string, Scalar > m_radii;	//!< Separation radii accessed by particle type
	};
	
//! Abstract interface for classes that generate particles
/*! \ingroup data_structs
	A ParticleGenerator is the workhorse that actually chooses where to place particles.
	A single ParticleGenerator should only place a small number of inter-related particles
	on each call to generateParticles() (i.e. a single polymer or a small cluster of particles).
	Larger systems are to be built from multiple calls of generateParticles() by RandomGenerator.
*/
class ParticleGenerator
	{
	public:
		//! Destructor
		virtual ~ParticleGenerator() {}
		
		//! Returns the number of particles that will be generated
		/*! Derived classes must implement this method so that RandomGenerator
			can properly allocate the space for the particles.
			
			The value returned by this function cannot vary from call to call
			for a particular instance of ParticleGenerator. Once instantiated, 
			a ParticleGenerator must always generate the same number of particles
			each time it is called.
		*/
		virtual unsigned int getNumToGenerate()=0;
		
		//! Actually generate the requested particles
		/*! \param particles Place generated particles here after a GeneratedParticles::canPlace() check
			\param starT_idx Starting index to generate particles at
			Derived classes must implement this method. RandomGenerator will 
			call it to generate the particles. Particles should be placed at indices
			\a start_idx, \a start_idx + 1, ... \a start_idx + getNumToGenerate()-1
		*/
		virtual void generateParticles(GeneratedParticles& particles, boost::mt19937& rnd, unsigned int start_idx)=0;
	};
	
//! Generates random polymers
/*! This ParticleGenerator can be used to generate systems of bead-spring polymers of any combination of
	partile types specified in an array.
*/
class PolymerParticleGenerator : public ParticleGenerator
	{
	public:
		//! Constructor
		PolymerParticleGenerator(Scalar bond_len, const std::vector<std::string>& types, unsigned int max_attempts);
		
		//! Returns the number of particles in each polymer
		virtual unsigned int getNumToGenerate()
			{
			return m_types.size();
			}
		
		//! Generates a single polymer
		virtual void generateParticles(GeneratedParticles& particles, boost::mt19937& rnd, unsigned int start_idx);
		
	private:
		Scalar m_bond_len;					//!< Bond length
		std::vector<std::string> m_types;	//!< Particle types for each polymer bead
		unsigned int m_max_attempts;		//!< Number of attemps to make for each particle placement
		
		//! helper function to place particles recursively
		bool generateNextParticle(GeneratedParticles& particles, boost::mt19937& rnd, unsigned int i, unsigned int start_idx, const GeneratedParticles::particle& prev_particle);
		
	};

	
//! Generates a random particle system given a set of ParticleGenerator classes
/*! \ingroup data_structs
	RandomGenerator is the high level Initializer that brings all the pieces together to
	generate a random system of particles. The structure and types of the particles generated
	(i.e. a polymer system of A6B7A6 polymers) is determined by the ParticleGenerator classes
	that are added. 
	
	Mixture systems can be created by adding more than one ParticleGenerator. 
	class. Testing should be done to confirm this, but it is probably best to
	add the largest objects first so that smaller ones can be generated around them.
	
	\b Usage:<br>
	Before the initializer can be passed to a ParticleData for initialization, the following
	steps must be performed.
	 -# Contstruct a RandomGenerator (duh) with a given box size
	 -# Set radii for all particle types to be generated
	 -# Construct and add any number of ParticleGenerator instances to the RandomGenerator
	 -# Call generate() to actually place the particles
*/
class RandomGenerator : public ParticleDataInitializer
	{
	public:
		//! Set the parameters
		RandomGenerator(const BoxDim& box, unsigned int seed);
		//! Empty Destructor
		virtual ~RandomGenerator() { }
		
		//! Returns the number of particles to be initialized
		virtual unsigned int getNumParticles() const;
		
		//! Returns the number of particle types to be initialized
		virtual unsigned int getNumParticleTypes() const;

		//! Returns the box the particles will sit in
		virtual BoxDim getBox() const;
		
		//! Initializes the particle data arrays
		virtual void initArrays(const ParticleDataArrays &pdata) const;
		
		//! Initialize the type name mapping
		std::vector<std::string> getTypeMapping() const;
		
		//! Sets the separation radius for a particle
		void setSeparationRadius(string type, Scalar radius);
		
		//! Adds a generator
		void addGenerator(unsigned int repeat, boost::shared_ptr<ParticleGenerator> generator);
		
		//! Place the particles
		void generate();
		
	private:
		BoxDim m_box;								//!< Precalculated box
		unsigned int m_seed;							//!< Random seed to use
		GeneratedParticles m_data;					//!< Actual particle data genreated
		std::map< std::string, Scalar > m_radii;	//!< Separation radii accessed by particle type
		std::vector< boost::shared_ptr<ParticleGenerator> > m_generators;	//!< Generators to place particles
		std::vector< unsigned int > m_generator_repeat;	//!< Repeat count for each generator
		std::vector<std::string> m_type_mapping;	//!< The created mapping between particle types and ids
		
		//! Helper function for identifying the particle type id
		unsigned int getTypeId(const std::string& name);
	};
#endif
