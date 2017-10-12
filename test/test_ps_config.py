from sphinxwrapper import PocketSphinx
import unittest


class SetGetConfigArgumentCase(unittest.TestCase):
    def setUp(self):
        # Initialise a default Pocket Sphinx decoder
        self.ps = PocketSphinx()
    
    def test_get_config_argument(self):
        ps = self.ps
        ps.set_config_argument("-verbose", "yes", False)
        self.assertTrue(ps.get_config_argument("-verbose"))
        ps.set_config_argument("-verbose", "no", False)
        self.assertFalse(ps.get_config_argument("-verbose"))
    
    def test_get_config_argument_reinit(self):
        ps = self.ps
        # Ensure -verbose is True
        ps.set_config_argument("-verbose", "yes", False)
        self.assertTrue(ps.get_config_argument("-verbose"))

        # Test with reinit enabled
        ps.set_config_argument("-verbose", "no", True)
        self.assertFalse(ps.get_config_argument("-verbose"))

        ps.set_config_argument("-verbose", "yes", True)
        self.assertTrue(ps.get_config_argument("-verbose"))

    def test_get_different_types(self):
        ps = self.ps
        cmn = ps.get_config_argument("-cmn")
        self.assertTrue(isinstance(cmn, str))
        
        lw = ps.get_config_argument("-lw")
        self.assertTrue(isinstance(lw, float))

    def test_get_unset_string(self):
        # By default, -keyphrase isn't set.
        keyphrase = self.ps.get_config_argument("-keyphrase")
        self.assertEqual("", keyphrase)
        
    def test_invalid_name(self):
        self.assertRaises(KeyError, self.ps.get_config_argument, "-test")


if __name__ == '__main__':
    unittest.main()
