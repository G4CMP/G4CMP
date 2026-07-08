/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

// $Id: 539f524339ae53ad098a07cfa3bebd07784d23dd $

#include "HeterostructureActionInitialization.hh"
#include "HeterostructurePrimaryGeneratorAction.hh"
#include "G4CMPStackingAction.hh"

void HeterostructureActionInitialization::Build() const {
  SetUserAction(new HeterostructurePrimaryGeneratorAction);
  SetUserAction(new G4CMPStackingAction);
} 
