/*
Highly Optimized Object-oriented Many-particle Dynamics -- Blue Edition
(HOOMD-blue) Open Source Software License Copyright 2008, 2009 Ames Laboratory
Iowa State University and The Regents of the University of Michigan All rights
reserved.

HOOMD-blue may contain modifications ("Contributions") provided, and to which
copyright is held, by various Contributors who have granted The Regents of the
University of Michigan the right to modify and/or distribute such Contributions.

Redistribution and use of HOOMD-blue, in source and binary forms, with or
without modification, are permitted, provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions, and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
list of conditions, and the following disclaimer in the documentation and/or
other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of HOOMD-blue's
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND/OR
ANY WARRANTIES THAT THIS SOFTWARE IS FREE OF INFRINGEMENT ARE DISCLAIMED.

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// $Id: HOOMDBinaryDumpWriter.cc 2213 2009-10-20 11:42:07Z joaander $
// $URL: https://codeblue.umich.edu/hoomd-blue/svn/trunk/src/analyzers/HOOMDBinaryDumpWriter.cc $
// Maintainer: joaander

/*! \file HOOMDBinaryDumpWriter.cc
    \brief Defines the HOOMDBinaryDumpWriter class
*/

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif

#include <boost/python.hpp>
using namespace boost::python;

#include <sstream>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <boost/shared_ptr.hpp>

#include "HOOMDBinaryDumpWriter.h"
#include "BondData.h"
#include "AngleData.h"
#include "DihedralData.h"
#include "WallData.h"

using namespace std;
using namespace boost;

/*! \param sysdef SystemDefinition containing the ParticleData to dump
    \param base_fname The base name of the file xml file to output the information

    \note .timestep.xml will be apended to the end of \a base_fname when analyze() is called.
*/
HOOMDBinaryDumpWriter::HOOMDBinaryDumpWriter(boost::shared_ptr<SystemDefinition> sysdef, std::string base_fname)
        : Analyzer(sysdef), m_base_fname(base_fname), m_output_position(true), m_output_image(false),
        m_output_velocity(false), m_output_mass(false), m_output_diameter(false), m_output_type(false),
        m_output_bond(false), m_output_angle(false), m_output_wall(false), m_output_dihedral(false),
        m_output_improper(false), m_output_accel(false)
    {
    }

/*! \param enable Set to true to enable the writing of particle positions to the files in analyze()
*/
void HOOMDBinaryDumpWriter::setOutputPosition(bool enable)
    {
    m_output_position = enable;
    }

/*! \param enable Set to true to enable the writing of particle images to the files in analyze()
*/
void HOOMDBinaryDumpWriter::setOutputImage(bool enable)
    {
    m_output_image = enable;
    }

/*!\param enable Set to true to output particle velocities to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputVelocity(bool enable)
    {
    m_output_velocity = enable;
    }

/*!\param enable Set to true to output particle masses to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputMass(bool enable)
    {
    m_output_mass = enable;
    }

/*!\param enable Set to true to output particle diameters to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputDiameter(bool enable)
    {
    m_output_diameter = enable;
    }

/*! \param enable Set to true to output particle types to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputType(bool enable)
    {
    m_output_type = enable;
    }
/*! \param enable Set to true to output bonds to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputBond(bool enable)
    {
    m_output_bond = enable;
    }
/*! \param enable Set to true to output angles to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputAngle(bool enable)
    {
    m_output_angle = enable;
    }
/*! \param enable Set to true to output walls to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputWall(bool enable)
    {
    m_output_wall = enable;
    }
/*! \param enable Set to true to output dihedrals to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputDihedral(bool enable)
    {
    m_output_dihedral = enable;
    }
/*! \param enable Set to true to output impropers to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputImproper(bool enable)
    {
    m_output_improper = enable;
    }
/*! \param enable Set to true to output acceleration to the XML file on the next call to analyze()
*/
void HOOMDBinaryDumpWriter::setOutputAccel(bool enable)
    {
    m_output_accel = enable;
    }

/*! \param fname File name to write
    \param timestep Current time step of the simulation
*/
void HOOMDBinaryDumpWriter::writeFile(std::string fname, unsigned int timestep)
    {
    // open the file for writing
    ofstream f(fname.c_str(), ios::out|ios::binary);
    
    if (!f.good())
        {
        cerr << endl << "***Error! Unable to open dump file for writing: " << fname << endl << endl;
        throw runtime_error("Error writing hoomd binary dump file");
        }
    
		
    // write the version of the binary format used
    int version = 1;
    f.write((char*)&version, sizeof(int));

    // acquire the particle data
    ParticleDataArraysConst arrays = m_pdata->acquireReadOnly();
    BoxDim box = m_pdata->getBox();
    Scalar Lx,Ly,Lz;
    Lx=Scalar(box.xhi-box.xlo);
    Ly=Scalar(box.yhi-box.ylo);
    Lz=Scalar(box.zhi-box.zlo);
        
    //write out the timestep and box
    f.write((char*)&timestep, sizeof(int));	
    f.write((char*)&Lx, sizeof(Scalar));	
    f.write((char*)&Ly, sizeof(Scalar));	
    f.write((char*)&Lz, sizeof(Scalar));	
    
    //output the position of all particles to the file
    {
    unsigned int np = m_pdata->getN();
    f.write((char*)&np, sizeof(unsigned int));	
    for (unsigned int j = 0; j < arrays.nparticles; j++)
        {
        // use the rtag data to output the particles in the order they were read in
        int i;
        i= arrays.rtag[j];
        
        Scalar x = (arrays.x[i]);
        Scalar y = (arrays.y[i]);
        Scalar z = (arrays.z[i]);
        
        f.write((char*)&x, sizeof(Scalar));	
        f.write((char*)&y, sizeof(Scalar));	
        f.write((char*)&z, sizeof(Scalar));	
        
        if (!f.good())
            {
            cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
            throw runtime_error("Error writing HOOMD dump file");
            }
        }
    }
        
    // Output the image of each particle to the file
    {
    unsigned int np = m_pdata->getN();
    f.write((char*)&np, sizeof(unsigned int));	
    for (unsigned int j = 0; j < arrays.nparticles; j++)
        {
        // use the rtag data to output the particles in the order they were read in
        int i;
        i= arrays.rtag[j];
        
        int x = (arrays.ix[i]);
        int y = (arrays.iy[i]);
        int z = (arrays.iz[i]);
        
        f.write((char*)&x, sizeof(int));	
        f.write((char*)&y, sizeof(int));	
        f.write((char*)&z, sizeof(int));	
        
        if (!f.good())
            {
            cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
            throw runtime_error("Error writing HOOMD dump file");
            }
        }
    }
    
    // Output the velocity of all particles to the file
    {
    unsigned int np = m_pdata->getN();
    f.write((char*)&np, sizeof(unsigned int));	
    
    for (unsigned int j = 0; j < arrays.nparticles; j++)
        {
        // use the rtag data to output the particles in the order they were read in
        int i;
        i= arrays.rtag[j];
        
        Scalar vx = arrays.vx[i];
        Scalar vy = arrays.vy[i];
        Scalar vz = arrays.vz[i];

        f.write((char*)&vx, sizeof(Scalar));	
        f.write((char*)&vy, sizeof(Scalar));	
        f.write((char*)&vz, sizeof(Scalar));	

        if (!f.good())
            {
            cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
            throw runtime_error("Error writing HOOMD dump file");
            }
        }            
    }

    /*
    // Output the acceleration of all particles to the file
    {
    unsigned int np = m_pdata->getN();
    f.write((char*)&np, sizeof(unsigned int));	
    
    for (unsigned int j = 0; j < arrays.nparticles; j++)
        {
        // use the rtag data to output the particles in the order they were read in
        int i;
        i= arrays.rtag[j];
        
        Scalar ax = arrays.ax[i];
        Scalar ay = arrays.ay[i];
        Scalar az = arrays.az[i];

        f.write((char*)&ax, sizeof(Scalar));	
        f.write((char*)&ay, sizeof(Scalar));	
        f.write((char*)&az, sizeof(Scalar));	

        if (!f.good())
            {
            cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
            throw runtime_error("Error writing HOOMD dump file");
            }
        }            
    }
    */
    
    // Output the mass of all particles to the file
    {
    unsigned int np = m_pdata->getN();
    f.write((char*)&np, sizeof(unsigned int));	
    
    for (unsigned int j = 0; j < arrays.nparticles; j++)
        {
        // use the rtag data to output the particles in the order they were read in
        int i;
        i= arrays.rtag[j];
        
        Scalar mass = arrays.mass[i];
        f.write((char*)&mass, sizeof(Scalar));	
        if (!f.good())
            {
            cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
            throw runtime_error("Error writing HOOMD dump file");
            }
        }            
    }
        
    // Output the diameter of all particles to the file
    {
    unsigned int np = m_pdata->getN();
    f.write((char*)&np, sizeof(unsigned int));	
    
    for (unsigned int j = 0; j < arrays.nparticles; j++)
        {
        // use the rtag data to output the particles in the order they were read in
        int i;
        i= arrays.rtag[j];
        
        Scalar diameter = arrays.diameter[i];
        f.write((char*)&diameter, sizeof(Scalar));	
        if (!f.good())
            {
            cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
            throw runtime_error("Error writing HOOMD dump file");
            }
        }            
    }
        
    // Output the types of all particles to an xml file
    {
    unsigned int np = m_pdata->getN();
    f.write((char*)&np, sizeof(unsigned int));	

    for (unsigned int j = 0; j < arrays.nparticles; j++)
        {
        int i;
        i= arrays.rtag[j];
        std::string name = m_pdata->getNameByType(arrays.type[i]);
        unsigned int len = name.size();
        f.write((char*)&len, sizeof(unsigned int));	
        f.write(name.c_str(), len*sizeof(char));	
        }
    }
    
    //Output the integrator states to the binary file
    {
    const std::vector<IntegratorVariables>& v = m_pdata->getIntegratorVariables();
    unsigned int ni = v.size();
    f.write((char*)&ni, sizeof(unsigned int));	
    for (unsigned int j = 0; j < ni; j++)
        {
        unsigned int len = v[j].type.size();
        f.write((char*)&len, sizeof(unsigned int));	
        if (len != 0) 
            {
            f.write(v[j].type.c_str(), len*sizeof(char));
            }
        unsigned int nv = v[j].variable.size();
        f.write((char*)&nv, sizeof(unsigned int));	
        for (unsigned int k=0; k<nv; k++)
            {
            Scalar var = v[j].variable[k];
            f.write((char*)&var, sizeof(Scalar));	
            }
        }
    }
    
    // Output the bonds to the binary file
    {
    unsigned int nb = m_sysdef->getBondData()->getNumBonds();
    f.write((char*)&nb, sizeof(unsigned int));	
    shared_ptr<BondData> bond_data = m_sysdef->getBondData();
    
    // loop over all bonds and write them out
    for (unsigned int i = 0; i < bond_data->getNumBonds(); i++)
        {
        Bond bond = bond_data->getBond(i);

        std::string name = bond_data->getNameByType(bond.type);
        unsigned int len = name.size();
        f.write((char*)&len, sizeof(unsigned int));	
        f.write(name.c_str(), name.size()*sizeof(char));	
        unsigned int a = bond.a;
        unsigned int b = bond.b;
        f.write((char*)&a, sizeof(unsigned int));	
        f.write((char*)&b, sizeof(unsigned int));	            
        }            
    }
        
    // Output the angles to the binary file
    {
    unsigned int na = m_sysdef->getAngleData()->getNumAngles();
    f.write((char*)&na, sizeof(unsigned int));	

    shared_ptr<AngleData> angle_data = m_sysdef->getAngleData();
    
    // loop over all angles and write them out
    for (unsigned int i = 0; i < angle_data->getNumAngles(); i++)
        {
        Angle angle = angle_data->getAngle(i);
        
        std::string name = angle_data->getNameByType(angle.type);
        unsigned int len = name.size();
        f.write((char*)&len, sizeof(unsigned int));	
        f.write(name.c_str(), name.size()*sizeof(char));	
        unsigned int a = angle.a;
        unsigned int b = angle.b;
        unsigned int c = angle.c;
        f.write((char*)&a, sizeof(unsigned int));	
        f.write((char*)&b, sizeof(unsigned int));	            
        f.write((char*)&c, sizeof(unsigned int));	            
        }            
    }
        
    // Write out dihedrals to the binary file
    {
    unsigned int nd = m_sysdef->getDihedralData()->getNumDihedrals();
    f.write((char*)&nd, sizeof(unsigned int));	

    shared_ptr<DihedralData> dihedral_data = m_sysdef->getDihedralData();
    
    // loop over all angles and write them out
    for (unsigned int i = 0; i < dihedral_data->getNumDihedrals(); i++)
        {
        Dihedral dihedral = dihedral_data->getDihedral(i);
        
        std::string name = dihedral_data->getNameByType(dihedral.type);
        unsigned int len = name.size();
        f.write((char*)&len, sizeof(unsigned int));	
        f.write(name.c_str(), name.size()*sizeof(char));	
        unsigned int a = dihedral.a;
        unsigned int b = dihedral.b;
        unsigned int c = dihedral.c;
        unsigned int d = dihedral.d;
        f.write((char*)&a, sizeof(unsigned int));	
        f.write((char*)&b, sizeof(unsigned int));	            
        f.write((char*)&c, sizeof(unsigned int));	            
        f.write((char*)&d, sizeof(unsigned int));	            
        }            
    }
        
    // Write out impropers to the binary file
    {
    unsigned int ni = m_sysdef->getImproperData()->getNumDihedrals();
    f.write((char*)&ni, sizeof(unsigned int));	
            
    shared_ptr<DihedralData> improper_data = m_sysdef->getImproperData();
    
    // loop over all angles and write them out
    for (unsigned int i = 0; i < improper_data->getNumDihedrals(); i++)
        {
        Dihedral dihedral = improper_data->getDihedral(i);

        std::string name = improper_data->getNameByType(dihedral.type);
        unsigned int len = name.size();
        f.write((char*)&len, sizeof(unsigned int));	
        f.write(name.c_str(), name.size()*sizeof(char));	
        unsigned int a = dihedral.a;
        unsigned int b = dihedral.b;
        unsigned int c = dihedral.c;
        unsigned int d = dihedral.d;
        f.write((char*)&a, sizeof(unsigned int));	
        f.write((char*)&b, sizeof(unsigned int));	            
        f.write((char*)&c, sizeof(unsigned int));	            
        f.write((char*)&d, sizeof(unsigned int));            
        }            
    }
        
    // Output the walls to the binary file
    {
    shared_ptr<WallData> wall_data = m_sysdef->getWallData();

    unsigned int nw = wall_data->getNumWalls();
    f.write((char*)&nw, sizeof(unsigned int));	
    
    // loop over all walls and write them out
    for (unsigned int i = 0; i < nw; i++)
        {
        Wall wall = wall_data->getWall(i);
        
        f.write((char*)&(wall.origin_x), sizeof(Scalar));	
        f.write((char*)&(wall.origin_y), sizeof(Scalar));	
        f.write((char*)&(wall.origin_z), sizeof(Scalar));	
        f.write((char*)&(wall.normal_x), sizeof(Scalar));	
        f.write((char*)&(wall.normal_y), sizeof(Scalar));	
        f.write((char*)&(wall.normal_z), sizeof(Scalar));	
        }
    }
        
    if (!f.good())
        {
        cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
        throw runtime_error("Error writing HOOMD dump file");
        }
        
    f.close();
    m_pdata->release();
    
    }

/*! \param timestep Current time step of the simulation
    Writes a snapshot of the current state of the ParticleData to a hoomd_xml file.
*/
void HOOMDBinaryDumpWriter::analyze(unsigned int timestep)
    {
    ostringstream full_fname;
    string filetype = ".bin";
    
    // Generate a filename with the timestep padded to ten zeros
    full_fname << m_base_fname << "." << setfill('0') << setw(10) << timestep << filetype;
    writeFile(full_fname.str(), timestep);
    }

void export_HOOMDBinaryDumpWriter()
    {
    class_<HOOMDBinaryDumpWriter, boost::shared_ptr<HOOMDBinaryDumpWriter>, bases<Analyzer>, boost::noncopyable>
    ("HOOMDBinaryDumpWriter", init< boost::shared_ptr<SystemDefinition>, std::string >())
    .def("setOutputPosition", &HOOMDBinaryDumpWriter::setOutputPosition)
    .def("setOutputImage", &HOOMDBinaryDumpWriter::setOutputImage)
    .def("setOutputVelocity", &HOOMDBinaryDumpWriter::setOutputVelocity)
    .def("setOutputMass", &HOOMDBinaryDumpWriter::setOutputMass)
    .def("setOutputDiameter", &HOOMDBinaryDumpWriter::setOutputDiameter)
    .def("setOutputType", &HOOMDBinaryDumpWriter::setOutputType)
    .def("setOutputBond", &HOOMDBinaryDumpWriter::setOutputBond)
    .def("setOutputAngle", &HOOMDBinaryDumpWriter::setOutputAngle)
    .def("setOutputDihedral", &HOOMDBinaryDumpWriter::setOutputDihedral)
    .def("setOutputImproper", &HOOMDBinaryDumpWriter::setOutputImproper)
    .def("setOutputWall", &HOOMDBinaryDumpWriter::setOutputWall)
    .def("setOutputAccel", &HOOMDBinaryDumpWriter::setOutputAccel)
    .def("writeFile", &HOOMDBinaryDumpWriter::writeFile)
    ;
    }

#ifdef WIN32
#pragma warning( pop )
#endif

