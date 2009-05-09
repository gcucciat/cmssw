#include "DQM/SiStripMonitorClient/interface/SiStripQualityChecker.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "CondFormats/SiStripObjects/interface/SiStripFedCabling.h"
#include "CalibFormats/SiStripObjects/interface/SiStripDetCabling.h"

#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/SiStripDetId/interface/TECDetId.h"
#include "DataFormats/SiStripDetId/interface/TIBDetId.h"
#include "DataFormats/SiStripDetId/interface/TOBDetId.h"
#include "DataFormats/SiStripDetId/interface/TIDDetId.h"

#include "DQM/SiStripCommon/interface/SiStripFolderOrganizer.h"
#include "DQM/SiStripMonitorClient/interface/SiStripUtility.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"


#include <iomanip>
using namespace std;
//
// -- Constructor
// 
SiStripQualityChecker::SiStripQualityChecker(edm::ParameterSet const& ps):pSet_(ps) {
  edm::LogInfo("SiStripQualityChecker") << 
    " Creating SiStripQualityChecker " << "\n" ;

  bookedStatus_ = false;
  SubDetFolderMap.insert(pair<string, string>("TIB",  "TIB"));
  SubDetFolderMap.insert(pair<string, string>("TOB",  "TOB"));
  SubDetFolderMap.insert(pair<string, string>("TECF", "TEC/side_2"));
  SubDetFolderMap.insert(pair<string, string>("TECB", "TEC/side_1"));
  SubDetFolderMap.insert(pair<string, string>("TIDF", "TID/side_2"));
  SubDetFolderMap.insert(pair<string, string>("TIDB", "TID/side_1"));
  badModuleList.clear();
}
//
// --  Destructor
// 
SiStripQualityChecker::~SiStripQualityChecker() {
  edm::LogInfo("SiStripQualityChecker") << 
    " Deleting SiStripQualityChecker " << "\n" ;
}
//
// -- create reportSummary MEs
//
void SiStripQualityChecker::bookStatus(DQMStore* dqm_store) {

  if (!bookedStatus_) {
    dqm_store->cd();
    dqm_store->setCurrentFolder("SiStrip/EventInfo"); 

    string hname, htitle;
    hname  = "detFractionReportMap";
    htitle = "SiStrip Report for Good Detector Fraction";
    DetFractionReportMap  = dqm_store->book2D(hname, htitle, 6,0.5,6.5,9,0.5,9.5);
    DetFractionReportMap->setAxisTitle("Sub Detector Type", 1);
    DetFractionReportMap->setAxisTitle("Layer/Disc Number", 2);
    hname  = "sToNReportMap";
    htitle = "SiStrip Report for Signal-to-Noise";
    SToNReportMap         = dqm_store->book2D(hname, htitle, 6,0.5,6.5,9,0.5,9.5);
    SToNReportMap->setAxisTitle("Sub Detector Type", 1);
    SToNReportMap->setAxisTitle("Layer/Disc Number", 2);

    hname  = "reportSummaryMap";
    htitle = "SiStrip Report Summary Map";
    SummaryReportMap      = dqm_store->book2D(hname, htitle, 6,0.5,6.5,9,0.5,9.5);
    SummaryReportMap->setAxisTitle("Sub Detector Type", 1);
    SummaryReportMap->setAxisTitle("Layer/Disc Number", 2);

    SummaryReportGlobal = dqm_store->bookFloat("reportSummary");
    int ibin = 0;
    dqm_store->setCurrentFolder("SiStrip/EventInfo/reportSummaryContents");      
    for (map<string, string>::const_iterator it = SubDetFolderMap.begin(); 
	 it != SubDetFolderMap.end(); it++) {
      ibin++;
      string det = it->first;
      DetFractionReportMap->setBinLabel(ibin,det);
      SToNReportMap->setBinLabel(ibin,det);
      SummaryReportMap->setBinLabel(ibin,det);

      SubDetMEs local_mes;
      string me_name;
      dqm_store->setCurrentFolder("SiStrip/EventInfo/reportSummaryContents");
      me_name = "SiStrip_" + det;
      local_mes.SummaryFlag = dqm_store->bookFloat(me_name);

      dqm_store->setCurrentFolder("SiStrip/EventInfo/reportSummaryContents");  
      me_name = "SiStrip_DetFraction_" + det;
     local_mes.DetFraction = dqm_store->bookFloat(me_name);


      dqm_store->setCurrentFolder("SiStrip/EventInfo/reportSummaryContents");  
      me_name = "SiStrip_SToNFlag_" + det;
      local_mes.SToNFlag    = dqm_store->bookFloat(me_name);
      SubDetMEsMap.insert(pair<string, SubDetMEs>(det, local_mes));
    }

    dqm_store->cd();
    dqm_store->setCurrentFolder("Tracking/EventInfo"); 
    TrackSummaryReportGlobal = dqm_store->bookFloat("reportSummary");

    dqm_store->setCurrentFolder("Tracking/EventInfo/reportSummaryContents");  
    ReportTrackRate = dqm_store->bookFloat("TrackRate");     

    bookedStatus_ = true;
  }
}
//
// -- Fill Dummy  Status
//
void SiStripQualityChecker::fillDummyStatus(){
  if (bookedStatus_) {
    resetStatus();  
    for (map<string, SubDetMEs>::const_iterator it = SubDetMEsMap.begin(); 
	 it != SubDetMEsMap.end(); it++) {
      SubDetMEs local_mes = it->second;
      local_mes.DetFraction->Fill(-1.0);
      local_mes.SToNFlag->Fill(-1.0);
      local_mes.SummaryFlag->Fill(-1.0);
    }
    
    for (unsigned int xbin = 1; xbin < 7; xbin++) {
      for (unsigned int ybin = 1; ybin < 10; ybin++) {
	DetFractionReportMap->Fill(xbin, ybin, -1.0);
	SToNReportMap->Fill(xbin, ybin, -1.0);
	SummaryReportMap->Fill(xbin, ybin, -1.0);
      }
    }
    SummaryReportGlobal->Fill(-1.0);
    
    TrackSummaryReportGlobal->Fill(-1.0);
    ReportTrackRate->Fill(-1.0);
  }
}
//
// -- Reset Status
//
void SiStripQualityChecker::resetStatus() {
  if (bookedStatus_) {
    DetFractionReportMap->Reset();
    SToNReportMap->Reset();
    SummaryReportMap->Reset();
    for (map<string, SubDetMEs>::const_iterator it = SubDetMEsMap.begin(); 
	 it != SubDetMEsMap.end(); it++) {
      SubDetMEs local_mes = it->second;
      local_mes.DetFraction->Reset();
      local_mes.SToNFlag->Reset();
      local_mes.SummaryFlag->Reset();
    }
    SummaryReportGlobal->Reset();

    TrackSummaryReportGlobal->Reset();
    ReportTrackRate->Reset();
  }
}
//
// -- Fill Status
//
void SiStripQualityChecker::fillStatus(DQMStore* dqm_store) {
  if (bookedStatus_) bookStatus(dqm_store);

  resetStatus(); 
  fillDummyStatus();
  fillDetectorStatus(dqm_store);
  fillTrackingStatus(dqm_store);
}
//
// Fill Detector Status
//
void SiStripQualityChecker::fillDetectorStatus(DQMStore* dqm_store) {
  unsigned int xbin = 0;
  float global_flag = 0;
  dqm_store->cd();
  string mdir = "MechanicalView"; 
  if (!SiStripUtility::goToDir(dqm_store, mdir)) return;
  string mechanicalview_dir = dqm_store->pwd();
  for (map<string, SubDetMEs>::const_iterator it = SubDetMEsMap.begin(); 
       it != SubDetMEsMap.end(); it++) {
    string det = it->first;
    map<string, string>::const_iterator cPos = SubDetFolderMap.find(det);
    if (cPos == SubDetFolderMap.end()) continue; 
    string dname = mechanicalview_dir + "/" + cPos->second;
    if (!dqm_store->dirExists(dname)) continue;
    dqm_store->cd(dname);
    SubDetMEs local_mes = it->second;
    xbin++;
    float flag;
    fillSubDetStatus(dqm_store, local_mes, xbin,flag);
    dqm_store->cd();
    global_flag += flag; 
  }
  global_flag = global_flag/xbin*1.0;
  SummaryReportGlobal->Fill(global_flag);
}
//
// -- Fill Tracking Status
//
void SiStripQualityChecker::fillTrackingStatus(DQMStore* dqm_store) {
  dqm_store->cd();
  string dir = "Tracking"; 
  if (!SiStripUtility::goToDir(dqm_store, dir)) return;
  dir = "TrackParameter"; 
  if (!SiStripUtility::goToDir(dqm_store, dir)) return;
  vector<MonitorElement*> meVec = dqm_store->getContents(dqm_store->pwd());
  float gstatus = 1.0;
  for (vector<MonitorElement*>::const_iterator it = meVec.begin(); it != meVec.end(); it++) {
    MonitorElement * me = (*it);     
    if (!me) continue;
    if (me->getQReports().size() == 0) continue;
    string name = me->getName();
    int istat =  SiStripUtility::getMEStatus((*it)); 
    float status = 1.0; 
    if (name.find("NumberOfTracks") != string::npos) {
      if (istat == dqm::qstatus::ERROR) status = 0.0;
      ReportTrackRate->Fill(status);
    }
    gstatus = gstatus * status; 
  }
  TrackSummaryReportGlobal->Fill(gstatus);
}
//
// -- Get Errors from Module level histograms
//
void SiStripQualityChecker::getModuleStatus(DQMStore* dqm_store,int& ndet,int& errdet) {
  vector<string> mids;
  SiStripUtility::getModuleFolderList(dqm_store, mids);
  for (vector<string>::const_iterator im = mids.begin();
       im != mids.end(); im++) {
    string det_str = (*im);
    det_str = det_str.substr(det_str.find("module_")+7);
    uint32_t detId = atoi((det_str).c_str());

    SiStripFolderOrganizer folder_organizer;

    vector<MonitorElement*> meVec = dqm_store->getContents((*im));
    if (meVec.size() == 0) continue;
    ndet++; 
    int err_me = 0;
    for (vector<MonitorElement*>::const_iterator it = meVec.begin();
	 it != meVec.end(); it++) {
      MonitorElement * me = (*it);     
      if (!me) continue;
      if (me->getQReports().size() == 0) continue;
      int istat =  SiStripUtility::getMEStatus((*it)); 
      if (istat == dqm::qstatus::ERROR)   err_me++;
    }
    if (err_me > 0) {
      errdet++;
      map<uint32_t,uint16_t>::iterator iPos = badModuleList.find(detId);
      if (iPos != badModuleList.end()){    
	if (errdet > iPos->second) iPos->second = errdet;
      } else {
	badModuleList.insert(pair<uint32_t,uint16_t>(detId,err_me));
      }
    }
  }
}
//
// -- Fill Sub detector Reports
//
void SiStripQualityChecker::fillSubDetStatus(DQMStore* dqm_store, 
                         SubDetMEs& mes, unsigned int xbin, float& gflag) {
  int status_flag  = pSet_.getUntrackedParameter<int>("GlobalStatusFilling", 1);
  
  vector<string> subDirVec = dqm_store->getSubdirs();

  unsigned int ybin   = 0;
  int tot_ndet        = 0;
  int tot_errdet      = 0;
  float tot_ston_stat = 0;

  for (vector<string>::const_iterator ic = subDirVec.begin();
       ic != subDirVec.end(); ic++) {
    vector<MonitorElement*> meVec;
    ybin++;
    dqm_store->cd((*ic));
    meVec = dqm_store->getContents((*ic));
    int ndet = 0;
    int errdet = 0;       

    int ston_stat = 1;
    vector<DQMChannel>  bad_channels;


    if (status_flag == 1) getModuleStatus(dqm_store, ndet, errdet);
    
    for (vector<MonitorElement*>::const_iterator it = meVec.begin();
	 it != meVec.end(); it++) {
      MonitorElement * me = (*it);
      if (!me) continue;
      if (me->getQReports().size() == 0) continue;
      string name = me->getName();
      vector<DQMChannel>  bad_channels_me;
      
      if( name.find("Summary_ClusterStoNCorr__OnTrack") != string::npos){
	int istat =  SiStripUtility::getMEStatus((*it)); 
	if (me->getEntries() > 100 && istat == dqm::qstatus::ERROR) ston_stat = 0;
      } else {
        if (status_flag == 2) {
	  getModuleStatus(me, ndet, bad_channels); 
	  errdet = bad_channels.size();
        }
      }
    }

    if (ndet > 0) {
      float eff_fac = 1 - (errdet*1.0/ndet);

      fillStatusHistogram(SToNReportMap,        xbin, ybin, ston_stat);
      fillStatusHistogram(DetFractionReportMap, xbin, ybin, eff_fac);
      if (ston_stat > 0) fillStatusHistogram(SummaryReportMap, xbin, ybin, eff_fac);
      else               fillStatusHistogram(SummaryReportMap, xbin, ybin, 0.0);

      tot_ndet      += ndet;
      tot_errdet    += errdet;
      tot_ston_stat += ston_stat;  
    }
    dqm_store->cd((*ic));
  }
  if (tot_ndet > 0) { 
    float tot_eff_fac = 1 - (tot_errdet*1.0/tot_ndet);
    if (mes.DetFraction) mes.DetFraction->Fill(tot_eff_fac);
    float tot_ston_fac = tot_ston_stat/ybin;
    if (mes.SToNFlag) mes.SToNFlag->Fill(tot_ston_fac);
    gflag = min(tot_eff_fac,tot_ston_fac);    
    if (mes.SummaryFlag) mes.SummaryFlag->Fill(gflag);
  }
}    
//
// -- Print Status Report
//
void SiStripQualityChecker::printStatusReport() {
  ostringstream det_summary_str;
  for (map<string, SubDetMEs>::const_iterator it = SubDetMEsMap.begin(); 
       it != SubDetMEsMap.end(); it++) {
    string det = it->first;
    det_summary_str << setprecision(4);
    det_summary_str << setiosflags(ios::fixed);

    det_summary_str << " Printing Status for " <<   det << " : " << endl;
    SubDetMEs local_mes = it->second;

    string sval;
    float fval1, fval2, fval3;
    fval1 = fval2 = fval3 = -1.0;   
    

    SiStripUtility::getMEValue(local_mes.DetFraction, sval); 
    if (sval.size() > 0) fval1 = atof(sval.c_str());
    SiStripUtility::getMEValue(local_mes.SToNFlag, sval); 
    if (sval.size() > 0) fval2 = atof(sval.c_str());
    SiStripUtility::getMEValue(local_mes.SummaryFlag, sval); 
    if (sval.size() > 0) fval3 = atof(sval.c_str());

    det_summary_str << setw(7) << " % of good detectors " << fval1
		    << " SToN Flag           " << fval2
		    << " Summary Flag        " << fval3 << endl;
  }
  cout << det_summary_str.str() << endl;
}
//
// -- Get Module Status from Layer Level Histograms
//
void SiStripQualityChecker::getModuleStatus(MonitorElement* me, int& ndet, 
                                            vector<DQMChannel>& bad_channels){
  int ndet_me = 0;
  vector<DQMChannel> bad_channels_me;
  std::vector<QReport *> qreports = me->getQReports();
  if (me->kind() == MonitorElement::DQM_KIND_TPROFILE) {
    ndet_me = me->getNbinsX();
    bad_channels_me = qreports[0]->getBadChannels();
  } else if (me->kind() == MonitorElement::DQM_KIND_TPROFILE2D) {
    TProfile2D* h  = me->getTProfile2D();
    float frac = me->getEntries() *1.0/ h->GetBinEntries(h->GetBin(1, 1));
    ndet_me = static_cast<int> (frac);
    bad_channels_me = qreports[0]->getBadChannels();
  }
  if (ndet_me > ndet)  ndet = ndet_me;
  // Check Bad Channels 
  if (bad_channels.size() == 0) bad_channels.insert(bad_channels.end(), bad_channels_me.begin(), bad_channels_me.end());
  else {
    size_t v1_size = bad_channels_me.size();
    size_t v2_size = bad_channels.size();
    for (size_t it = 0;  it != v1_size; it++){
      
      int xval = bad_channels_me[it].getBinX();
      int yval = bad_channels_me[it].getBinY();
      int zval = bad_channels_me[it].getBinZ();
      bool already_exist = false;
      for (size_t im = 0; im != v2_size; im++){
	if (xval == bad_channels[im].getBinX() && 
	    yval == bad_channels[im].getBinY() && 
	    zval == bad_channels[im].getBinZ()) {
	  already_exist = true;
	  break;
	} else {
	  already_exist = false;
	}
      }
      if (!already_exist) {
	bad_channels.push_back(bad_channels_me[it]);
      } 
      
    }
  }
}
//
// -- Fill Report Summary Map
//
 void SiStripQualityChecker::fillStatusHistogram(MonitorElement* me, int xbin, int ybin, float val){
   if (me &&  me->kind() == MonitorElement::DQM_KIND_TH2F) {
     TH2F*  th2d = me->getTH2F();
     th2d->SetBinContent(xbin, ybin, val);
   }
 }
//
// -- Get List of Bad Modules
//
const std::map<uint32_t,uint16_t> & SiStripQualityChecker::getBadModuleList(DQMStore* dqm_store)  {

  if (badModuleList.size() == 0) fillStatus(dqm_store);
  return badModuleList;
}
//
// -- Create Moneitor Elements for Modules
//
void SiStripQualityChecker::printFaultyModuleList(DQMStore* dqm_store, ostringstream& str_val) {
  if (badModuleList.size() == 0) return;
  dqm_store->cd();
  str_val << endl;
  string mdir = "MechanicalView"; 
  if (!SiStripUtility::goToDir(dqm_store, mdir)) return;
  string mechanicalview_dir = dqm_store->pwd();

  int nDetsTotal  = 0;
  int nTotalError = 0;

  for (map<string, string>::const_iterator it = SubDetFolderMap.begin(); 
       it != SubDetFolderMap.end(); it++) {
    string subdet_name   = it->first;
    string subdet_folder = it->second;
    string dname = mechanicalview_dir + "/" + subdet_folder;
    if (!dqm_store->dirExists(dname)) continue;
    string bad_module_folder = dname + "/" + "BadModuleList";
    str_val << "============"<< endl;                                                                          
    str_val << subdet_folder << endl;                                                    
    str_val << "============"<< endl;                                                                          
    str_val << endl;      
    dqm_store->cd(dname);
    vector<string> module_folders;
    SiStripUtility::getModuleFolderList(dqm_store, module_folders);
    int nDets = module_folders.size();
    int nDetsWithError = 0;
    for (map<uint32_t,uint16_t>::const_iterator it =  badModuleList.begin() ; it != badModuleList.end(); it++) {
      uint32_t detId =  it->first;
      string subdet_tag;
      SiStripUtility::getSubDetectorTag(detId,subdet_tag);

      if (subdet_tag == (subdet_name)) {
        nDetsWithError++;
	str_val << " Module Id " << detId << " : Errors " << it->second << endl;         
        // Check if the ME exists otherwise book it
	dqm_store->setCurrentFolder(bad_module_folder);
	ostringstream detid_str;
	detid_str << detId;
	string full_path = bad_module_folder + "/" + detid_str.str();
	MonitorElement* me = dqm_store->get(full_path);
	if (me) me->Reset();
	else me = dqm_store->bookInt(detid_str.str());
	me->Fill(it->second);
      }
    }
    str_val << "--------------------------------------------------------------------"<< endl;                  
    str_val << " Detectors :  Total "<<   nDets
            << " with Error " << nDetsWithError<< endl;   
    str_val << "--------------------------------------------------------------------"<< endl;                  
    nTotalError += nDetsWithError;
    nDetsTotal  += nDets;    
  }
  str_val << endl;                                                                                             
  str_val << endl;                                                          
  str_val << " Total Detectors " << nDetsTotal << endl;                                                        
  str_val << " # of Detectors with Error " << nTotalError<< endl;                                          
}

