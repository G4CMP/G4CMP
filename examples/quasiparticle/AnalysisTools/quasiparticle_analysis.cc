//------------------------------------------------------------------
//
// quasiparticle_analysis.cc
// linehan3@fnal.gov
//
// This macro is meant to analyze the output of the quasiparticle
// example provided in the advanced tutorial session of RISQ 2026.
//
//------------------------------------------------------------------

//C++ includes
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>

//ROOT includes
#include "TH2F.h"
#include "TH1F.h"

//-----------------------------------------------------------------------------
// There is a "step" struct that is used in this analysis for
// storing step information into a nice package and there is also an
// event struct for storing all of the steps from various tracks
// into events.

struct Step
{
  int runID;
  int eventID;
  double preStepX_mm;
  double preStepY_mm;
  double preStepZ_mm;
  double preStepT_ns;
  double preStepE_eV;
  double preStepKinE_eV;  
  double postStepX_mm;
  double postStepY_mm;
  double postStepZ_mm;
  double postStepT_ns;
  double postStepE_eV;
  double postStepKinE_eV;
  std::string process;
  std::string preStepVolume;
  std::string postStepVolume;
  std::string particleName;
  int nReflections;
};

struct Event
{
  int runID;
  int eventID;
  std::map<int,std::vector<Step> > trackVect;
};


//-----------------------------------------------------------------------------
// Parsing and Processing Functions. These only build the events, which are
// composed of tracks (built from steps). Right now this is just a single
// function, which reads in steps and builds events.

// Parsing function: steps. the output is an int (event ID for this
// file) to event. The key to the map is pair(runID,eventID)
std::map<std::pair<int,int>,Event>
ParseStepTextFileForEvents(std::string filename) {
  std::map<std::pair<int,int>,Event> output;
  Event dummy;
  std::map<int,std::vector<Step> > thisEvtTrackVect;
  std::vector<Step> thisTrackStepVect;
  
  std::ifstream infile;
  infile.open(filename.c_str());
  std::string wholeLine;  
  
  //Begin to loop through file
  int eventID = -1;
  int runID = -1;
  int trackID = -1;
  std::string particleName;
  int counter = 0;
  std::pair<int,int> lastKey(-1,-1);
  while(1){
    if(!infile.good()) break;
    if(infile.is_open()){
      std::getline(infile,wholeLine);

      //Tokenize the string (split between commas)
      stringstream check1(wholeLine);
      string token;
      std::vector<std::string> tokens;
      while(getline(check1,token,' ')){
        tokens.push_back(token);
      }
      if( tokens.size() == 0 ) break;

      if( tokens.size() != 20 ){
        std::cout << "Tokens size is not 20. It is: " << tokens.size()
                  << ". Problem!" << std::endl;
        break;
      }

      //Identify the output run and event IDs
      int thisRunID = std::atoi(tokens[0].c_str());
      int thisEventID = std::atoi(tokens[1].c_str());
      std::pair<int,int> thisKey(thisRunID,thisEventID);

      //If the output vector does not have an event with this pair, push back
      //an Event into the output vector with this pair.
      if( output.count(thisKey) == 0 ){
        Event dummy;
        output.emplace(thisKey,dummy);
        output[thisKey].runID = thisKey.first;
        output[thisKey].eventID = thisKey.second;
      }
      
      //Now, look at the track ID. If the output vector does not have a track
      //ID corresponding to this track ID, then emplace the track map back
      //with a new track
      int thisTrackID = std::atoi(tokens[2].c_str());
      if (output[thisKey].trackVect.count(thisTrackID) == 0) {
        std::vector<Step> dummyStepVect;
        output[thisKey].trackVect.emplace(thisTrackID,dummyStepVect);
      }

      //At this point, there should be a.) a runID and eventID for this step in
      //the output, and b.) a trackID with at least a dummy step vector inside
      //the output. Now we gather the step information and push it into the
      //step vect      
      Step thisStep;
      thisStep.runID = runID;
      thisStep.eventID = eventID;
      thisStep.particleName = tokens[3];
      thisStep.preStepX_mm = std::atof(tokens[4].c_str());
      thisStep.preStepY_mm = std::atof(tokens[5].c_str());
      thisStep.preStepZ_mm = std::atof(tokens[6].c_str());
      thisStep.preStepT_ns = std::atof(tokens[7].c_str());
      thisStep.preStepE_eV = std::atof(tokens[8].c_str());
      thisStep.preStepKinE_eV = std::atof(tokens[9].c_str());
      thisStep.postStepX_mm = std::atof(tokens[10].c_str());
      thisStep.postStepY_mm = std::atof(tokens[11].c_str());
      thisStep.postStepZ_mm = std::atof(tokens[12].c_str());
      thisStep.postStepT_ns = std::atof(tokens[13].c_str());
      thisStep.postStepE_eV = std::atof(tokens[14].c_str());
      thisStep.postStepKinE_eV = std::atof(tokens[15].c_str());
      thisStep.nReflections = std::atoi(tokens[16].c_str());
      thisStep.process = tokens[17];
      thisStep.preStepVolume = tokens[18];
      thisStep.postStepVolume = tokens[19];
      output[thisKey].trackVect[thisTrackID].push_back(thisStep);
    }
  }
  return output;
}

//------------------------------------------------------------------------------
// Parsing function: concatenating event vectors (in case you find this
// useful)
std::vector<Event> ConcatenateEventVects(std::vector<std::vector<Event> > eventVect_VECT)
{
  std::vector<Event> output;
  for( int iV = 0; iV < eventVect_VECT.size(); ++iV ){
    std::vector<Event> thisVect = eventVect_VECT[iV];
    for( int iE = 0; iE < thisVect.size(); ++iE ){
      output.push_back(thisVect[iE]);
    }
  }
  return output;
}


//------------------------------------------------------------------------------
// Parsing function: test printing
void TestPrintEvents(std::vector<Event> eventList)
{
  for( int iE = 0; iE < eventList.size(); ++iE ){
    if( !(eventList[iE].eventID == 27 || eventList[iE].eventID == 33)  ) continue;

    //Print the run ID and event ID
    std::cout << "-> Run ID: " << eventList[iE].runID << ", Event ID: " << eventList[iE].eventID << std::endl;
    std::cout << "-> nTracks: " << eventList[iE].trackVect.size() << std::endl;

    //Now loop through the tracks
    for( std::map<int,std::vector<Step> >::iterator it = eventList[iE].trackVect.begin(); it != eventList[iE].trackVect.end(); ++it ){

      //Print the track ID and then the steps
      std::cout << "---> Track ID: " << it->first << ", nSteps: " << it->second.size() << std::endl;
      for( int iS = 0; iS < it->second.size(); ++iS ){
        std::cout << "-----> Step: " << iS
                  << ", preX: " << it->second[iS].preStepX_mm 
                  << ", preY: " << it->second[iS].preStepY_mm
                  << ", preZ: " << it->second[iS].preStepZ_mm
                  << ", preT: " << it->second[iS].preStepT_ns
                  << ", preE: " << it->second[iS].preStepE_eV
                  << ", preKE: " << it->second[iS].preStepKinE_eV
                  << ", postX: " << it->second[iS].postStepX_mm
                  << ", postY: " << it->second[iS].postStepY_mm
                  << ", postZ: " << it->second[iS].postStepZ_mm
                  << ", postT: " << it->second[iS].postStepT_ns
                  << ", postE: " << it->second[iS].postStepE_eV
                  << ", postKE: " << it->second[iS].postStepKinE_eV
                  << ", particle: " << it->second[iS].particleName
                  << ", process: " << it->second[iS].process << std::endl;
      }      
    }
  }
}

//------------------------------------------------------------------------------
// Parsing function: bundling the creation of the final event list
std::vector<Event>
CreateEventListFromStepFiles(std::string stepFile) {
  std::vector<Event> output;
  std::map<std::pair<int,int>,Event> map_runIDEvtID_Event =
    ParseStepTextFileForEvents(stepFile);  
  for( std::map<std::pair<int,int>,Event>::iterator it = map_runIDEvtID_Event.begin(); it != map_runIDEvtID_Event.end(); ++it ){
    output.push_back(it->second);
  }
  return output;
}

//------------------------------------------------------------------------------
// Physics analysis functions
//Add a QP track's time evolution in a target resonator region to the total
//junction to the total QP engagement in that region
void FillResonatorHistoWithSingleTrackHisto(TH1F * h_totalHist,
                                            TH1F * h_thisTrackHist) {
  if( h_thisTrackHist->GetEntries() == 0 ) return;
  for( int iB = 0; iB < h_thisTrackHist->GetNbinsX(); ++iB ){
    h_totalHist->SetBinContent(iB,h_totalHist->GetBinContent(iB) + h_thisTrackHist->GetBinContent(iB));
  }
}

//------------------------------------------------------------------------------
//Generic analysis
void DoAnalysis(TFile * fOut, std::vector<Event> eventList, std::string name ){
  
  fOut->cd();
  TDirectory * thisDir = fOut->mkdir(name.c_str());
  thisDir->cd();
  
  //----------------------------------------  
  //Set up histogram bins and bounds
  //----------------------------------------
  //Bins/bounds for QP XYZ info
  int nBinsXY = 500;
  double minXY_mm = -4.1;
  double maxXY_mm = 4.1;
  int nBinsZ = 1000;
  double minZ_mm = 4.5;
  double maxZ_mm = 5.1;
  
  //Bins/bounds for QP T info
  int nBinsT = 1000;
  double maxT_ns = 1e7;
  double maxDeltaT_ns = 1e7;
  int nBinsTimeResonator = 1000;
  double maxTimeResonator_ns = 1e7;
  
  double nBinsLength = 10000;
  double maxLength_nm = 10000;
  double nBinsConstrainedDeltaT = 10000;
  double maxConstrainedDeltaT_ns = 1.0;

  //----------------------------------------  
  //Set up histograms
  //----------------------------------------
  TH1F * h_qpBouncesAtLastStep =
    new TH1F(("h_qpBouncesAtLastStep_"+name).c_str(),
             "h_qpBouncesAtLastStep; QP Bounces at Last Step; QPs",
             2000,0,200000);

  std::cout << "name is: " << name << std::endl;
  
  TH2F * h_qpStartXY =
    new TH2F(("h_qpStartXY_"+name).c_str(),
             "h_qpStartXY; Start X [mm]; Start Y [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsXY,minXY_mm,maxXY_mm);
  
  TH2F * h_qpStartXZ =
    new TH2F(("h_qpStartXZ_"+name).c_str(),
             "h_qpStartXZ; Start X [mm]; Start Y [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsZ,minZ_mm,maxZ_mm);
  
  TH2F * h_qpStartYZ =
    new TH2F(("h_qpStartYZ_"+name).c_str(),
             "h_qpStartYZ; Start X [mm]; Start Y [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsZ,minZ_mm,maxZ_mm);

  TH2F * h_qpAllStepsXY =
    new TH2F(("h_qpAllStepsXY_"+name).c_str(),
             "h_qpAllStepsXY; Step X [mm]; Step Y [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsXY,minXY_mm,maxXY_mm);
  
  TH2F * h_qpAllStepsXZ =
    new TH2F(("h_qpAllStepsXZ_"+name).c_str(),
             "h_qpAllStepsXZ; Step X [mm]; Step Y [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsZ,minZ_mm,maxZ_mm);
  
  TH2F * h_qpAllStepsYZ =
    new TH2F(("h_qpAllStepsYZ_"+name).c_str(),
             "h_qpAllStepsYZ; Step X [mm]; Step Y [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsZ,minZ_mm,maxZ_mm);
  
  TH2F * h_qpLastStepPostXY =
    new TH2F(("h_qpLastStepPostXY_"+name).c_str(),
             "h_qpLastStepPostXY; X [mm]; Y [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsXY,minXY_mm,maxXY_mm);
    
  TH2F * h_qpLastStepPostXZ =
    new TH2F(("h_qpLastStepPostXZ_"+name).c_str(),
             "h_qpLastStepPostXZ; X [mm]; Z [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsZ,minZ_mm,maxZ_mm);
  
  TH2F * h_qpLastStepPostYZ =
    new TH2F(("h_qpLastStepPostYZ_"+name).c_str(),
             "h_qpLastStepPostYZ; Y [mm]; Z [mm]",
             nBinsXY,minXY_mm,maxXY_mm,nBinsZ,minZ_mm,maxZ_mm);
  
  TH1F * h_qpLifetime =
    new TH1F(("h_qpLifetime_"+name).c_str(),
             "h_qpLifetime; Last Step PostT-First Step PreT; Delta T [ns]; QPs",
             nBinsT,0,maxDeltaT_ns);
  
  TH1F * h_qpCreationTime =
    new TH1F(("h_qpCreationTime_"+name).c_str(),
             "h_qpCreationTime; Time [ns]; QPs", nBinsT,0,maxT_ns);
  
  TH1F * h_qpDeathTime =
    new TH1F(("h_qpDeathTime_"+name).c_str(),
             "h_qpDeathTime; Time [ns]; QPs",nBinsT,0,maxT_ns);

  TH1F * h_fullResQPTimesInRes0HalfCircle1 =
    new TH1F("h_fullResQPTimesInRes0HalfCircle1",
             "h_fullResQPTimesInRes0HalfCircle1; Time [ns]; Time-averaged QP occupation [QPs];",
             nBinsTimeResonator,0,maxTimeResonator_ns);
  
  TH1F * h_fullResQPTimesInRes0HalfCircle6 =
    new TH1F("h_fullResQPTimesInRes0HalfCircle6",
             "h_fullResQPTimesInRes0HalfCircle6; Time [ns]; Time-averaged QP occupation [QPs];",
             nBinsTimeResonator,0,maxTimeResonator_ns);

  TH1F * h_fullResQPTimesInRes0Any =
    new TH1F("h_fullResQPTimesInRes0Any",
             "h_fullResQPTimesInRes0Any; Time [ns]; Time-averaged QP occupation [QPs];",
             nBinsTimeResonator,0,maxTimeResonator_ns);
  
  
  //----------------------------------------  
  // Loop over the event list 
  //----------------------------------------
  for( int iE = 0; iE < eventList.size(); ++iE ){
    std::cout << "Starting to analyze " << iE << " events." << std::endl;
    
    //Loop through tracks/steps
    for( std::map<int,std::vector<Step> >::iterator it = eventList[iE].trackVect.begin(); it != eventList[iE].trackVect.end(); ++it ){
      std::vector<Step> thisTrack = it->second;

      //Check to confirm that this track is a QP
      if( !(thisTrack[0].particleName == "BogoliubovQP") ) continue;

      //Fill lifetime and time-in-junction (if at all)
      h_qpLifetime->Fill(thisTrack[thisTrack.size()-1].postStepT_ns -
                         thisTrack[0].preStepT_ns);
      
      // Create histograms that fill the two resonator pieces with QP
      // occupancies for "this track"
      TH1F * h_thisTrackTimesInRes0HalfCircle1 =
        new TH1F("h_thisTrackTimesInRes0HalfCircle1",
                 "h_thisTrackTimesInRes0HalfCircle1; Time [ns]; QPs per bin;",
                 nBinsTimeResonator,0,maxTimeResonator_ns);
      
      TH1F * h_thisTrackTimesInRes0HalfCircle6 =
        new TH1F("h_thisTrackTimesInRes0HalfCircle6",
                 "h_thisTrackTimesInRes0HalfCircle6; Time [ns]; QPs per bin;",
                 nBinsTimeResonator,0,maxTimeResonator_ns);

      TH1F * h_thisTrackTimesInRes0Any =
        new TH1F("h_thisTrackTimesInRes0Any",
                 "h_thisTrackTimesInRes0Any; Time [ns]; QPs per bin;",
                 nBinsTimeResonator,0,maxTimeResonator_ns);
      
      //Now we loop over steps in this track
      for( int iS = 0; iS < thisTrack.size(); ++iS ){

        //Compute step length
        double dX = thisTrack[iS].postStepX_mm - thisTrack[iS].preStepX_mm;
        double dY = thisTrack[iS].postStepY_mm - thisTrack[iS].preStepY_mm;
        double dZ = thisTrack[iS].postStepZ_mm - thisTrack[iS].preStepZ_mm;
        double stepLength = TMath::Power(dX*dX+dY*dY+dZ*dZ,0.5);
                
        //For first-step things:
        if( iS == 0 ){
          h_qpStartXY->Fill(thisTrack[iS].preStepX_mm,
                            thisTrack[iS].preStepY_mm);
          h_qpStartXZ->Fill(thisTrack[iS].preStepX_mm,
                            thisTrack[iS].preStepZ_mm);
          h_qpStartYZ->Fill(thisTrack[iS].preStepY_mm,
                            thisTrack[iS].preStepZ_mm);
          h_qpCreationTime->Fill(thisTrack[iS].preStepT_ns);
        }
        
        //For last-step things:
        if( iS == thisTrack.size()-1 ){
          h_qpBouncesAtLastStep->Fill(thisTrack[iS].nReflections);
          h_qpLastStepPostXY->Fill(thisTrack[iS].postStepX_mm,
                                   thisTrack[iS].postStepY_mm);
          h_qpLastStepPostXZ->Fill(thisTrack[iS].postStepX_mm,
                                   thisTrack[iS].postStepZ_mm);
          h_qpLastStepPostYZ->Fill(thisTrack[iS].postStepY_mm,
                                   thisTrack[iS].postStepZ_mm);
          h_qpDeathTime->Fill(thisTrack[iS].postStepT_ns);
        }

        //For any steps...
        h_qpAllStepsXY->Fill(thisTrack[iS].postStepX_mm,
                             thisTrack[iS].postStepY_mm);
        h_qpAllStepsXZ->Fill(thisTrack[iS].postStepX_mm,
                             thisTrack[iS].postStepZ_mm);
        h_qpAllStepsYZ->Fill(thisTrack[iS].postStepY_mm,
                             thisTrack[iS].postStepZ_mm);
        
        //Select on the geometry we want
        bool res0_halfCircle1 =
          (thisTrack[iS].preStepVolume.find("0_halfCircle1Conductor")
           != std::string::npos);
        bool res0_halfCircle6 =
          (thisTrack[iS].preStepVolume.find("0_halfCircle6Conductor")
           != std::string::npos);
        bool res0_any =
          ( (thisTrack[iS].preStepVolume.find("0_halfCircle6Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_tlCouplingConductor")
             != std::string::npos) ||            
            (thisTrack[iS].preStepVolume.find("0_halfCircle5Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_halfCircle4Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_halfCircle3Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_halfCircle2Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_halfCircle1Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_curve1Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_curve2Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_curve3Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shl1Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shl2Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shl3Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shl4Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shl5Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shl6Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shl7Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_svl1Conductor")
             != std::string::npos) ||
            (thisTrack[iS].preStepVolume.find("0_shuntCoupler")
             != std::string::npos) );
               
        //Now we fill the resonator target halfCircle pieces, with weights corresp-
        //onding to the time that the QP was present for this step. This is
        //done so that steps of unequal time do not contribute equally to a
        //measure of xqp. We will divide out by the time bin width later to get
        //an "time-averaged Nqp" for a given bin, which can be lower than 1 if
        //a QP briefly pops into the target region before quickly popping back
        //out.
        if( res0_halfCircle1 ){
          h_thisTrackTimesInRes0HalfCircle1->
            Fill(thisTrack[iS].postStepT_ns,
                 (thisTrack[iS].postStepT_ns-thisTrack[iS].preStepT_ns));
        }
        if( res0_halfCircle6 ){
          h_thisTrackTimesInRes0HalfCircle6->
            Fill(thisTrack[iS].postStepT_ns,
                 (thisTrack[iS].postStepT_ns-thisTrack[iS].preStepT_ns));
        }
        if( res0_any ){
          h_thisTrackTimesInRes0Any->
            Fill(thisTrack[iS].postStepT_ns,
                 (thisTrack[iS].postStepT_ns-thisTrack[iS].preStepT_ns));
        }

      }       
      
      //Now that we've finished steps for this track, push back this track's
      //info into the "all tracks" histogram
      FillResonatorHistoWithSingleTrackHisto(h_fullResQPTimesInRes0HalfCircle1,
                                             h_thisTrackTimesInRes0HalfCircle1);
      FillResonatorHistoWithSingleTrackHisto(h_fullResQPTimesInRes0HalfCircle6,
                                             h_thisTrackTimesInRes0HalfCircle6);
      FillResonatorHistoWithSingleTrackHisto(h_fullResQPTimesInRes0Any,
                                             h_thisTrackTimesInRes0Any);

      //Free these histograms' memory
      delete h_thisTrackTimesInRes0HalfCircle1;
      delete h_thisTrackTimesInRes0HalfCircle6;
      delete h_thisTrackTimesInRes0Any;
    }    
  }

  //Need to finish up by doing an appropriate scaling of the Nqp histograms. 
  h_fullResQPTimesInRes0HalfCircle1->
    Scale(1.0/((double)h_fullResQPTimesInRes0HalfCircle1->GetBinWidth(1)));
  h_fullResQPTimesInRes0HalfCircle6->
    Scale(1.0/((double)h_fullResQPTimesInRes0HalfCircle6->GetBinWidth(1)));  
  h_fullResQPTimesInRes0Any->
    Scale(1.0/((double)h_fullResQPTimesInRes0Any->GetBinWidth(1)));  

  
  //Now take the qubit QPs-in-junction histograms into bonafide Xqp waveforms.
  //Here, all we should need to do is scale one more time by the volume and
  //the density of cooper pairs to get an Xqp.
  

  //Save histograms
  h_qpBouncesAtLastStep->Write();
  h_qpStartXY->Write();
  h_qpStartXZ->Write();
  h_qpStartYZ->Write();
  h_qpAllStepsXY->Write();
  h_qpAllStepsXZ->Write();
  h_qpAllStepsYZ->Write();
  h_qpLastStepPostXY->Write();
  h_qpLastStepPostXZ->Write();
  h_qpLastStepPostYZ->Write();
  h_qpLifetime->Write();
  h_qpCreationTime->Write();
  h_qpDeathTime->Write();
  h_fullResQPTimesInRes0HalfCircle1->Write();
  h_fullResQPTimesInRes0HalfCircle6->Write();
  h_fullResQPTimesInRes0Any->Write();  
}



//------------------------------------------------------------------------------
// The main function here
void run_quasiparticle_analysis(std::string infilePoint1, std::string infilePoint2) {
  
  //Establish output file
  TFile * fOut = new TFile("quasiparticle_analysis_output.root","RECREATE");
  
  //Input event list
  std::map<std::string,std::string> map_condition_file;
  map_condition_file.emplace("Point1",infilePoint1.c_str());
  map_condition_file.emplace("Point2",infilePoint2.c_str());
  
  //Loop over the files in the file vect and run the analysis
  for( std::map<std::string,std::string>::iterator it = map_condition_file.begin(); it != map_condition_file.end(); ++it ){
    std::cout << "Analyzing events in file " << it->second << std::endl;
    std::vector<Event> eventList = CreateEventListFromStepFiles(it->second);
    DoAnalysis(fOut,eventList,it->first);    
  }
}
 
