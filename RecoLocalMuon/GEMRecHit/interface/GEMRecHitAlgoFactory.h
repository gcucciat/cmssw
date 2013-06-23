#ifndef RecoLocalMuon_GEMRecHitAlgoFactory_H
#define RecoLocalMuon_GEMRecHitAlgoFactory_H

/** \class GEMRecHitAlgoFactory
 *  Factory of seal plugins for 1D RecHit reconstruction algorithms.
 *  The plugins are concrete implementations of GEMRecHitBaseAlgo base class.
 *
 *  $Date: 2013/04/04 13:31:56 $
 *  $Revision: 1.2 $
 *  \author G. Cerminara - INFN Torino
 */
#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "RecoLocalMuon/GEMRecHit/interface/GEMRecHitBaseAlgo.h"

typedef edmplugin::PluginFactory<GEMRecHitBaseAlgo *(const edm::ParameterSet &)> GEMRecHitAlgoFactory;
#endif




