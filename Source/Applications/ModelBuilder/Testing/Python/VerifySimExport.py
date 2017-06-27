#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
import os
import sys
from pprint import pprint
import filecmp


def verifyExport():
    if len(sys.argv) < 4:
        print("Wrong number of arguments %i should be %i" % (len(sys.argv), 3))
        return
    elif len(sys.argv) > 4:
        print("Note: only 4 arguments used")

    testScript = sys.argv[2]
    testName = os.path.basename(testScript)[:-4]  # get filename, strip '.xml'

    datadir = os.path.join(sys.argv[1], 'simulation_export')
    baselinedir = os.path.join(datadir, testName + 'Test')
    # baselinedir now contains the folder with the baselines to diff against

    testDir = sys.argv[3]  # the path to testing/temporary

    # Some of special files comparison )not nessisarily a bc file
    if testName == 'ExportSimAdHShallow':
        return verifyAdHShallow(baselinedir, testDir, testName)
    elif testName == 'ExportSimProteus':
        return verifyExportSimProteus(baselinedir, testDir, testName)

    basefile = os.path.join(baselinedir, 'test.bc')
    testfile = os.path.join(testDir, testName + '.bc')

    if not os.path.isfile(basefile):
        print("No base file", basefile, "!")
        return -1

    if not os.path.isfile(testfile):
        print("No test file", testfile, "!")
        return -2

    # compare files, return 0 if contents are the same
    if filecmp.cmp(basefile, testfile):
        print("Output is the same!")
        return 0
    else:
        print("Output not the same!")
        return -3


def verifyAdHShallow(baselinedir, testDir, testName):
    basefile_2dm = os.path.join(baselinedir, 'test.2dm')
    testfile_2dm = os.path.join(testDir, testName + '.2dm')

    basefile_bc = os.path.join(baselinedir, 'test.bc')
    testfile_bc = os.path.join(testDir, testName + '.bc')

    basefile_hot = os.path.join(baselinedir, 'test.hot')
    testfile_hot = os.path.join(testDir, testName + '.hot')

    basefile_mt = os.path.join(baselinedir, 'test.mt')
    testfile_mt = os.path.join(testDir, testName + '.mt')

    # Compare .bc files
    retval = 0

    cmp_bc = filecmp.cmp(basefile_bc, testfile_bc)
    if not (cmp_bc):
        print("ERROR: .bc output not the same!")
        retval = -4

    # Compare .mt files
    cmp_mt = filecmp.cmp(basefile_mt, testfile_mt)
    if not cmp_mt:
        print("ERROR: .mt output not the same!")
        retval = -5

    # Check that .2dm file exists
    if not os.path.isfile(testfile_2dm):
        print('ERROR: missing 2dm file %s' % testfile_2dm)
        retval = -6

    # Todo check .hot file

    return retval


def verifyExportSimProteus(baselinedir, testDir, testName):
    basefile_n = os.path.join(baselinedir, 'test_n.py')
    testfile_n = os.path.join(testDir, testName + '_n.py')

    basefile_p = os.path.join(baselinedir, 'test_p.py')
    testfile_p = os.path.join(testDir, testName + '_p.py')

    if filecmp.cmp(basefile_n, testfile_n) == True and \
            filecmp.cmp(basefile_p, testfile_p) == True:
        print("Output is the same!")
        return 0
    else:
        print("Output not the same!")
        return -7


def test():
    print("In Test")
    return verifyExport()

if __name__ == '__main__':
    sys.exit(test())
