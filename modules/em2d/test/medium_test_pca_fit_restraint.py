from __future__ import print_function
import IMP
import IMP.test
import IMP.atom
import IMP.em2d
import sys
import os


class EM2DFitRestraintTest(IMP.test.TestCase):

    def test_simple(self):
        """Simple test of EM2D single PDB score application"""
        m = IMP.Model()
        #! read PDB
        mp = IMP.atom.read_pdb(self.get_input_file_name('complex-2d.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector())

        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        for image_name in ('image_1_text.pgm', 'image_1_binary.pgm'):
            image_list = [self.get_input_file_name(image_name)]

            pca_fit_restraint = IMP.em2d.PCAFitRestraint(
                particles, image_list, 2.2, 20, 100)
            self.check_standard_object_methods(pca_fit_restraint)
            score = pca_fit_restraint.evaluate(False)
            print('initial score = ' + str(score))
            self.assertAlmostEqual(score, 0.052, delta=0.01)
            os.unlink('images.pgm')


if __name__ == '__main__':
    IMP.test.main()
