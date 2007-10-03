import unittest

# set the appropriate search path
import sys
sys.path.append("../python_libs/")
import IMP_Utils
import IMP_Test, imp2

# Class to test particles
class test_particles(IMP_Test.IMPTestCase):
    """test particles"""

    def setUp(self):
        """set up Modeller with the torus restraints """

        self.imp_model = imp2.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        # create particles 0 - 11
        for i in range(0,12):
            self.particles.append(IMP_Utils.Particle(self.imp_model, i*2, i*3, i*4))
            p1 = self.particles[i].imp()
            p1.add_float("radius", 1.5 * i, False)
            p1.add_int("id", i)
            p1.add_string("id_str", "name_"+str(i))

        # add additional attributes to particle 11
        for i in range(0,6):
            p1.add_float("attr_" + str(i), 3.5 * i, False)


    def test_particles(self):
        """ test that particle attributes are available and correct """

        model_data = self.imp_model.get_model_data()

         # check particles 0 - 11
        for i in range(0,12):
            p1 = self.particles[i].imp()
            fidx = p1.float_index("radius")
            self.assert_(model_data.get_float(fidx) == 1.5 * i, "expecting particle "+str(i)+" radius to be "+str(1.5*i) + " not " + str(model_data.get_float(fidx)))
            iidx = p1.int_index("id")
            self.assert_(model_data.get_int(iidx) == i, "expecting particle "+str(i)+" id to be "+str(i) + " not " + str(model_data.get_int(iidx)))
            # strings not fully implemented yet
            # sidx = p1.string_index("id_str")
            # self.assert_(model_data.get_string(sidx) == '', "expecting particle "+str(i)+" id_str to be name_"+str(i) + " not " + model_data.get_string(sidx))

        # test additional attributes in particle 11
        p1 = self.particles[11].imp()
        for i in range(0,6):
            fidx = p1.float_index("attr_" + str(i))
            self.assert_(model_data.get_float(fidx) == 3.5 * i, "expecting particle "+str(i)+" radius to be "+str(3.2*i) + " not " + str(model_data.get_float(fidx)))


if __name__ == '__main__':
    unittest.main()
