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
# This script runs ModelBuilder (assuming it produces something)
# and then a python verification script (calls a function called "test")

import os
import sys
from subprocess import call, check_call
from pprint import pprint


def RunMBAndVerify():

    if len(sys.argv) != 7:
        print("Incorrect Number of arguments. Got %i expected %i" %
              (len(sys.argv), 7))
        print("Format: %s ModelBuilderPath testdir testScript pythonExecutable verifyScript testDataRoot" %
              (sys.argv[0]))
        pprint("Got: %s" % sys.argv)
        return 1

    pprint(sys.argv)

    # sys.argv[0] is the script (RunModelBuilderAndVerify)
    modelBuilder = sys.argv[1]
    testdir = sys.argv[2]
    testScript = sys.argv[3]
    pythonExecutable = sys.argv[4]
    verifyScript = sys.argv[5]
    testDataRoot = sys.argv[6]

    status = 0

    # run ModelBuilder
    try:
        # Remove any test env vars
        modelbuilder_env = os.environ.copy()
        modelbuilder_env.pop('DYLD_LIBRARY_PATH', None)
        modelbuilder_env.pop('LD_LIBRARY_PATH', None)
        modelbuilder_env.pop('PYTHONPATH', None)

        c = [modelBuilder, "-dr", "--test-directory=%s" %
             testdir, "--test-script=%s" % testScript, "--exit"]
        print("Trying call: %s" % c)
        status = call(c, env=modelbuilder_env)
    except Exception, e:
        print("ModelBuilder call failed: %s" % e)
        status = -1
        return status

    print("ModelBuilder export: %i. Now verifying" % (status))

    # run verify script
    # note: we use the command-line call rather than import because
    # 	import-by-filename is depreciated in 3.4
    try:
        c = [pythonExecutable, verifyScript, testDataRoot, testScript, testdir]
        # print("Trying call: %s" % c)
        r = check_call(c)
        print("Verification done: %i" % r)
        status = status + r
    except Exception, e:
        print('Verify script call failed: %s' % e)
        return -2

    print('Done. Status: %i' % status)

    return status

if __name__ == '__main__':
    sys.exit(RunMBAndVerify())
