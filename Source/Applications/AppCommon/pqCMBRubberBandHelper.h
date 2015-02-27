/*=========================================================================

  Program:   CMB
  Module:    pqCMBRubberBandHelper.h

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


#ifndef __pqCMBRubberBandHelper_h
#define __pqCMBRubberBandHelper_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QObject>

class pqRenderView;
class pqView;
class vtkObject;

/*! \brief Utility to switch interactor styles in 3D views.
 *
 * pqCMBRubberBandHelper standardizes the mechanism by which 3D views
 * are switched between interaction and rubber band modes.
 * TODO: We may want to extend this class to create different type of selections
 * i.e. surface/frustrum.
 *
 * @deprecated Use pqRenderViewSelectionReaction instead.
 */
class CMBAPPCOMMON_EXPORT pqCMBRubberBandHelper : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;
public:
  // @deprecated Please modify your code to use pqRenderViewSelectionReaction
  // instead.
  pqCMBRubberBandHelper(QObject* parent=NULL);
  virtual ~pqCMBRubberBandHelper();

  /// Returns the currently selected render view.
  pqRenderView* getRenderView() const;

  //BTX
  enum Modes
  {
    INTERACT,
    SELECT, //aka, Surface selection
    SELECT_POINTS,
    FRUSTUM,
    FRUSTUM_POINTS,
    BLOCKS,
    ZOOM,
    PICK,
    PICK_ON_CLICK,
    FAST_INTERSECT,
    POLYGON_POINTS,
    POLYGON_CELLS
  };
  // PICK_ON_CLICK mode is same as pick, except that the helper does not change
  // the interactor or draw any rubber bands, now change the cursor. It just
  // sets up observer to call "pick" when user clicks.
  //ETX

public slots:
  /// Set active view. If a view has been set previously, this method ensures
  /// that it is not in selection mode.
  void setView(pqView*);

  /// Begin rubber band surface selection on the view.
  /// Has any effect only if active view is a render view.
  void beginSurfaceSelection();
  void beginSurfacePointsSelection();
  void beginFrustumSelection();
  void beginFrustumPointsSelection();
  void beginBlockSelection();
  void beginZoom();
  void beginPick();
  void beginPickOnClick();
  void beginFastIntersect();

  /// Instantly trigger a FastIntersect processing which will fire
  /// intersectionFinished(xyz) based on the curent mouse location.
  void triggerFastIntersect();

  /// End rubber band selection.
  /// Has any effect only if active view is a render view.
  void endSelection();
  void endPick()
    { this->endSelection(); }
  void endZoom()
    { this->endSelection(); }

  /// Called to disable selection.
  void DisabledPush();

  /// Called to pop disabling of selection. If there are as many DisabledPop() as
  /// DisabledPush(), the selection will be enabled.
  void DisabledPop();

signals:
  /// fired after mouse up in selection mode
  void selectionFinished(int xmin, int ymin, int xmax, int ymax);

  /// Fired to indicate whether the selection can be created on the currently set
  /// view.
  void enableSurfaceSelection(bool enabled);
  void enableSurfacePointsSelection(bool enabled);
  void enableFrustumSelection(bool enabled);
  void enableFrustumPointSelection(bool enabled);
  void enableBlockSelection(bool enabled);
  void enableZoom(bool enabled);
  void enablePick(bool enabled);
  void enablePolygonPointsSelection(bool enabled);
  void enablePolygonCellsSelection(bool enabled);

  /// Fired with selection mode changes.
  /// \c selectionMode is enum Modes{...}.
  void selectionModeChanged(int selectionMode);

  /// This is inverse of selectionModeChanged signal, provided for convenience.
  void interactionModeChanged(bool notselectable);

  /// Fired to mark the start and ends of selection.
  void startSelection();
  void stopSelection();
  void selecting(bool);

  /// Fired in FAST_INTERSECT_MODE
  void intersectionFinished(double x, double y, double z);

protected slots:
  void emitEnabledSignals();
  void delayedSelectionChanged()
    {
    this->onSelectionChanged(NULL, 0, NULL);
    }

protected:
  int setRubberBandOn(int mode);
  int setRubberBandOff();
  int Mode;
  int Xs, Ys, Xe, Ye;
  int DisableCount;

  // Called whenever a zoom is made in the view
  void onZoom(vtkObject*, unsigned long, void*);

  // Called whenever a pick_on_click is made in the view
  void onPickOnClick(vtkObject*, unsigned long, void*);

  // Called whenever a selection is made in the view.
  void onSelectionChanged(vtkObject*, unsigned long, void*);

  // Called whenever a polygon is drawn in the view
  void onPolygonSelection(vtkObject*, unsigned long, void*);

private:
  class pqInternal;
  pqInternal* Internal;
};

#endif
