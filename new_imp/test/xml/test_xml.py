import unittest
import os
import IMP.test, IMP
import IMP.xml_loader

class XMLTests(IMP.test.TestCase):
    """Test XML file loading"""

    def setUp(self):
        """Set up IMP model"""
        self.imp_model = IMP.Model()

    def test_invalid_xml(self):
        """Check reading of invalid XML files"""
        self.assertRaises(IMP.xml_loader.MalformedError,
                          IMP.xml_loader.load_imp_model, self.imp_model,
                          "xml/invalid.xml")

    def test_xml(self):
        """Check reading of XML files"""
        self.doc = IMP.xml_loader.load_imp_model(self.imp_model,
                                                 "xml/model.xml")
        model_data = self.imp_model.get_model_data()

        # test particles
        num_particles = 0
        for p in self.imp_model.get_particles():
            radius = p.get_value(IMP.FloatKey("radius"))
            protein = p.get_value(IMP.IntKey("protein"))
            num_particles = num_particles + 1
            if protein == 1:
                self.assert_(radius == 2.0, "protein1 beads should have radius 2.0")
            elif protein == 2:
                self.assert_(radius == 1.5, "protein2 beads should have radius 1.5")
            elif protein == 3:
                self.assert_(radius == 1.0, "protein3 beads should have radius 1.0")

        self.assert_(num_particles == 12, "number of particles is incorrect")

        # test restraint sets
        #restraint_sets = self.imp_model.get_restraints
        self.assertEqual(self.imp_model.number_of_restraints(), 3,
                         "xml file contains three restraint sets")

        # test restraints
        score = self.imp_model.get_restraint(IMP.RestraintIndex(0)).evaluate(None)
        self.assert_((score > 16000) and (score < 17000), "expecting a score of 16198.2236328 not : " + str(score))
        score = self.imp_model.get_restraint(IMP.RestraintIndex(1)).evaluate(None)
        self.assert_((score > 624000) and (score < 625000), "expecting a score of 624531.0625 not : " + str(score))
        score = self.imp_model.get_restraint(IMP.RestraintIndex(2)).evaluate(None)
        self.assert_((score > -100) and (score < 100), "expecting a score of 0.0 not : " + str(score))

if __name__ == '__main__':
    unittest.main()
