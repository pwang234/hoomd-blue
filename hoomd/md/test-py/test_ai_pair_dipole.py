# -*- coding: iso-8859-1 -*-
# Maintainer: mspells

from hoomd import *
from hoomd import md
context.initialize()
import unittest
import os

# md.pair.dipole
class pair_dipole_tests (unittest.TestCase):
    def setUp(self):
        print
        system = init.create_random(N=100, phi_p=0.05);
        snap = system.take_snapshot(all=True)
        snap.particles.angmom[:] = 1
        system.restore_snapshot(snap)

        self.nl = md.nlist.cell()
        context.current.sorter.set_params(grid=8)

    # basic test of creation
    def test(self):
        dipole = md.pair.dipole(r_cut=3.0, nlist = self.nl);
        dipole.pair_coeff.set('A', 'A', mu=1.0, A=1.0, kappa=1.0)
        dipole.update_coeffs();

    # test missing coefficients
    def test_set_missing_mu(self):
        dipole = md.pair.dipole(r_cut=3.0, nlist = self.nl);
        dipole.pair_coeff.set('A', 'A', A=1.0, kappa=1.0)
        self.assertRaises(RuntimeError, dipole.update_coeffs);

    # test missing coefficients
    def test_set_missing_kappa(self):
        dipole = md.pair.dipole(r_cut=3.0, nlist = self.nl);
        dipole.pair_coeff.set('A', 'A', mu=1.0, A=1.0)
        self.assertRaises(RuntimeError, dipole.update_coeffs);

    # test missing coefficients
    def test_missing_AA(self):
        dipole = md.pair.dipole(r_cut=3.0, nlist = self.nl);
        self.assertRaises(RuntimeError, dipole.update_coeffs);

    # test set params
    def test_set_params(self):
        dipole = md.pair.dipole(r_cut=3.0, nlist = self.nl);
        dipole.set_params(mode="no_shift");
        dipole.set_params(mode="shift");
        # xplor is not implemented for anisotropic pair potentials
        self.assertRaises(RuntimeError, dipole.set_params, mode="blah");

    # test nlist subscribe
    def test_nlist_subscribe(self):
        dipole = md.pair.dipole(r_cut=2.5, nlist = self.nl);
        dipole.pair_coeff.set('A', 'A', mu=1.0, kappa=1.0)
        self.nl.update_rcut();
        self.assertAlmostEqual(2.5, self.nl.r_cut.get_pair('A','A'));

        dipole.pair_coeff.set('A', 'A', r_cut = 2.0)
        self.nl.update_rcut();
        self.assertAlmostEqual(2.0, self.nl.r_cut.get_pair('A','A'));

    def tearDown(self):
        del self.nl
        context.initialize();


if __name__ == '__main__':
    unittest.main(argv = ['test.py', '-v'])
