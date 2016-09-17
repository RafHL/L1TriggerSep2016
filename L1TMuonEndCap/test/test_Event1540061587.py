import unittest

from FWLiteAnalyzer import FWLiteAnalyzer


class MyAnalyzer(FWLiteAnalyzer):

  def __init__(self):
    inputFiles = ["Event1540061587_out.root"]
    # 3 1 3 2 1 1 15 10 28 1 0 65
    # 3 1 3 2 1 1 12 5 28 1 1 82
    # 3 1 3 0 2 1 11 2 64 3 0 34

    handles = {
      "emtf": ("std::vector<L1TMuonEndCap::EMTFHitExtra>", "simEmtfDigisData"),
    }
    super(MyAnalyzer, self).__init__(inputFiles, handles)

  def process(self, event):
    self.getHandles(event)
    emtf = self.handles["emtf"].product()

    hit = emtf[0]
    assert(hit.phi_fp      == 3454)
    assert(hit.theta_fp    == 36)
    assert((1<<hit.ph_hit) == 4096)
    assert(hit.phzvl       == 1)

    hit = emtf[1]
    assert(hit.phi_fp      == 3543)
    assert(hit.theta_fp    == 36)
    assert((1<<hit.ph_hit) == 32768)
    assert(hit.phzvl       == 1)

    hit = emtf[2]
    assert(hit.phi_fp      == 3983)
    assert(hit.theta_fp    == 29)
    assert((1<<hit.ph_hit) == 2048)
    assert(hit.phzvl       == 1)

    return

class TestEvent1540061587(unittest.TestCase):
  def setUp(self):
    self.analyzer = MyAnalyzer()

  def test_one(self):
    self.analyzer.analyze()


# ______________________________________________________________________________
if __name__ == "__main__":

  unittest.main()
