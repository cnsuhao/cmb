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
import smtk
from smtk.simple import *

def verifyExport():
    if len(sys.argv) < 4:
        print("Wrong number of arguments %i should be %i" % (len(sys.argv), 3))
        return
    elif len(sys.argv) > 4:
        print("Note: only 4 arguments used")

    testScript = sys.argv[2]
    testName = os.path.basename(testScript)[:-4] # get filename, strip '.xml'

    datadir = os.path.join(sys.argv[1],'SimulationFiles')
    baselinedir = os.path.join(datadir, testName+'Test')
    # baselinedir now contains the folder with the baselines to diff against

    testDir = sys.argv[3] #the path to testing/temporary

    # Some of special files comparison )not nessisarily a bc file
    if testName == 'ExportSimAdHShallow':
        return verifyAdHShallow(baselinedir,testDir, testName)
    elif testName == 'ExportSimProteus':
        return verifyExportSimProteus(baselinedir, testDir, testName)

    basefile = os.path.join(baselinedir, 'test.bc')
    testfile = os.path.join(testDir,testName+'.bc')

    # compare files, return 0 if contents are the same
    if filecmp.cmp(basefile, testfile):
        print("Output is the same!")
        return 0
    else:
        print("Output not the same!")
        return -1

def verifyAdHShallow(baselinedir, testDir, testName):
    basefile_2dm = os.path.join(baselinedir, 'test.2dm')
    testfile_2dm = os.path.join(testDir,testName+'.2dm')

    basefile_bc = os.path.join(baselinedir, 'test.bc')
    testfile_bc = os.path.join(testDir,testName+'.bc')

    basefile_hot = os.path.join(baselinedir, 'test.hot')
    testfile_hot = os.path.join(testDir,testName+'.hot')

    basefile_mt = os.path.join(baselinedir, 'test.mt')
    testfile_mt = os.path.join(testDir,testName+'.mt')

    if  filecmp.cmp(basefile_2dm,testfile_2dm) == True and \
        filecmp.cmp(basefile_bc,testfile_bc) == True and \
        filecmp.cmp(basefile_hot,testfile_hot) == True and \
        filecmp.cmp(basefile_mt,testfile_mt) == True:
        print("Output is the same!")
        return 0
    else:
        print("Output not the same!")
        return -1

def verifyExportSimProteus(baselinedir, testDir, testName):
    basefile_n = os.path.join(baselinedir, 'test_n.py')
    testfile_n = os.path.join(testDir,testName+'_n.py')

    basefile_p = os.path.join(baselinedir, 'test_p.py')
    testfile_p = os.path.join(testDir,testName+'_p.py')

    if filecmp.cmp(basefile_n,testfile_n) == True and \
        filecmp.cmp(basefile_p,testfile_p) == True:
        print("Output is the same!1")
        return 0
    else:
        print("Output not the same!1")
        return -1


def test():
    print("In Test")
    return verifyExport()

if __name__ == '__main__':
    sys.exit(test())
