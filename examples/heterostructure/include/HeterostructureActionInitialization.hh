/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

// $Id: e58a61fedbb99b167e16dafebc9c8664ae0c7b94 $

#ifndef HeterostructureActionInitialization_hh
#define HeterostructureActionInitialization_hh 1

#include "G4VUserActionInitialization.hh"

class HeterostructureActionInitialization : public G4VUserActionInitialization {
public:
  HeterostructureActionInitialization() {;}
  virtual ~HeterostructureActionInitialization() {;}
  virtual void Build() const;
};

#endif	/* HeterostructureActionInitialization_hh */
