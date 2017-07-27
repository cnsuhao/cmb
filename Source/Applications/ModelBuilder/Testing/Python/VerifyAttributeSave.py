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
import smtk
from smtk.simple import *
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.bridge.discrete
    import smtk.model
    import smtk.attribute


def verifyAssociation():
    mgr = smtk.model.Manager.create()
    sess = mgr.createSession('discrete')
    sess.assignDefaultName()
    SetActiveSession(sess)

    sref = GetActiveSession()
    r = sref.op('read')

    if len(sys.argv) < 4:
        print("Wrong number of arguments %i should be %i" % (len(sys.argv), 4))
        return
    elif len(sys.argv) > 4:
        print("Note: only 4 arguments used")

    try:
        datadir = sys.argv[1]
        print("datadir=%s" % datadir)
        modelpath = os.path.join(
            datadir, 'ThirdParty', 'SMTK', 'data', 'model', '2d', 'cmb', 'test2D.cmb')
    except:
        print 'Failed to determine model path'
        return -1

    print "modelpath", modelpath
    model = smtk.model.Model(Read(modelpath)[0])
    # read the model tree
    faces = model.cells()  # list of faces
    edges = []
    print('Faces:')
    for x in model.cells():  # faces
        print x.name(), x.entity()
        for y in x.boundingCells():  # edges
            edges.append(y)
            # print ' ', y.name(), y.entity()
            for z in y.boundingCells():  # vertices
                # print '  ', z.name(), z.entity()
                pass  # if ever testing vertex association, add them here

    # Get the resource for the CRF
    print 'CMB:'
    rs = getCRF()
    if rs == None:
        return 2

    # rs now contains the CRF
    resource = rs.get('simbuilder')
    if smtk.wrappingProtocol() == "pybind11":
        attCollection = resource
    else:
        attCollection = smtk.attribute.System.CastTo(resource)

    attributeTypes = attCollection.definitions()

    # list of tuples with attribute instance names and entity names
    matches = list()

    print "attributeTypes:"
    for aType in attributeTypes:
        defs = attCollection.findAttributes(aType.type())
        print aType.type()
        for attribute in defs:
            print " ", attribute.name(), "| Associations:", attribute.associations()
            if attribute.associations() is None:
                continue
            for uuid in attribute.associatedModelEntityIds():
                for face in faces:
                    if uuid == face.entity():
                        print "\tMatched", attribute.name(), "with", face.name()
                        matches.append((attribute.name(), face.name()))
                for edge in edges:
                    if uuid == edge.entity():
                        print "\tMatched", attribute.name(), "with", edge.name()
                        matches.append((attribute.name(), edge.name()))

    # verify the matches
    matches = list(set(matches))  # remove duplicate matches

    correctMatches = [('mat1', 'Face1'), ('mat1', 'Face4'), ('mat2', 'Face2'), (
        'VelocityBound-0', 'Edge4'), ('VelocityBound-0', 'Edge7')]
    for m in correctMatches:
        if m not in matches:
            print m, "match not found"
            return -2
    if len(correctMatches) != len(matches):
        print "Expected", len(correctMatches), "matches, got", len(matches)
        print "Matches found:"
        for match in matches:
            print match
        return -3

    return 0


def getCRF():
    rs = smtk.common.ResourceSet()
    reader = smtk.io.ResourceSetReader()
    logger = smtk.io.Logger()

    datadir = sys.argv[3]
    crfpath = os.path.join(datadir, 'VerifyAttributeSave.crf')
    print('crfpath: %s' % crfpath)
    if reader.readFile(crfpath, rs, logger) == True:
        print('Error reading file ' + logger.convertToString())
        return None

    # file successfully read...
    return rs


def test():
    print("In Test")
    return verifyAssociation()

if __name__ == '__main__':
    sys.exit(test())
