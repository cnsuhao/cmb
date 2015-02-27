TEMPLATE      = app
QT           += opengl
CONFIG       += console
HEADERS       = aabb.h \
                Accelerator.h \
                cAOI.h \
                cObjectDefinition.h \
                cObjectGeometry.h \
                cOmicron.h \
                ControlSettings.h \
                cOSDL.h \
                cOutputs.h \
                CrossPlatform.h \
                cSurfaceObjectDefinition.h \
                cVeg.h \
                EulerAngles.h \
                gm.h \
                gmConst.h \
                gmMat3.h \
                gmMat4.h \
                gmUtils.h \
                gmVec2.h \
                gmVec3.h \
                gmVec4.h \
                OGLCamera.h \
                OGLMaterial.h \
                OrthoControl.h \
                PathFix.h \
                PerspControl.h \
                QuatTypes.h \
                SceneGen.h \
                triangle.h \
                ViewMenu.h \
                ViewWnd.h \
                SceneGenMainWnd.h \
                ViewComb.h \
		    cTWI.h\
                dlgBaseMatID.h \
                dlgCameraLocation.h \
		    dlgScale.h \
                dlgObjectImport.h \
                dlgNew.h

SOURCES       = main.cpp \
                ViewWnd.cpp \
                aabb.cpp \
                Accelerator.cpp \
                cAOI.cpp \
                cObjectDefinition.cpp \
                cObjectGeometry.cpp \
                cOmicron.cpp \
                cOSDL.cpp \
                cOutputs.cpp \
                cSurfaceObjectDefinition.cpp \
                cVeg.cpp \
                EulerAngles.cpp \
                gmMat3.cpp \
                gmMat4.cpp \
                OGLCamera.cpp \
                OGLMaterial.cpp \
                OrthoControl.cpp \
                PathFix.cpp \
                PerspControl.cpp \
                triangle.cpp \
		    ViewMenu.cpp \
                SceneGenMainWnd.cpp \
                ViewComb.cpp \
		    cTWI.cpp \
                dlgBaseMatID.cpp \
                dlgCameraLocation.cpp \
		    dlgScale.cpp \
                dlgObjectImport.cpp \
                dlgNew.cpp

RESOURCES	= SceneGen.qrc
