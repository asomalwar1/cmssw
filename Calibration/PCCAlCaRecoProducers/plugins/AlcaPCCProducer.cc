/**_________________________________________________________________
class:   AlcaPCCProducer.cc



authors:Sam Higginbotham (shigginb@cern.ch) and Chris Palmer (capalmer@cern.ch) 

________________________________________________________________**/


// C++ standard
#include <string>
// CMS
#include "DataFormats/Luminosity/interface/PixelClusterCounts.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"
#include "DataFormats/Luminosity/interface/PixelClusterCounts.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

#include "TMath.h"
//The class 
class AlcaPCCProducer : public edm::one::EDProducer<edm::EndLuminosityBlockProducer,edm::one::WatchLuminosityBlocks>{
  public:
    explicit AlcaPCCProducer(const edm::ParameterSet&);
    ~AlcaPCCProducer();

  private:
    virtual void beginLuminosityBlock     (edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup) ;
    virtual void endLuminosityBlock       (edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup) ;
    virtual void endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) ;
    virtual void produce                  (edm::Event& iEvent, const edm::EventSetup& iSetup) ;
 
    edm::EDGetTokenT<edmNew::DetSetVector<SiPixelCluster> >  pixelToken;
    edm::InputTag   fPixelClusterLabel;

    std::string trigstring_;      //specifies the trigger Rand or ZeroBias 
    int ftotalevents;
    int countEvt_;       //counter
    int countLumi_;      //counter
    int beginLumiOfPCC_;
    int endLumiOfPCC_;

    std::unique_ptr<reco::PixelClusterCounts> thePCCob;

};

//--------------------------------------------------------------------------------------------------
AlcaPCCProducer::AlcaPCCProducer(const edm::ParameterSet& iConfig)
{
    fPixelClusterLabel = iConfig.getParameter<edm::ParameterSet>("AlcaPCCProducerParameters").getParameter<edm::InputTag>("pixelClusterLabel");
    trigstring_ = iConfig.getParameter<edm::ParameterSet>("AlcaPCCProducerParameters").getUntrackedParameter<std::string>("trigstring","alcaPCC");

    ftotalevents = 0;
    countLumi_ = 0;
    beginLumiOfPCC_ = endLumiOfPCC_ = -1;

    produces<reco::PixelClusterCounts, edm::InLumi>(trigstring_);
    pixelToken=consumes<edmNew::DetSetVector<SiPixelCluster> >(fPixelClusterLabel);
}

//--------------------------------------------------------------------------------------------------
AlcaPCCProducer::~AlcaPCCProducer(){
}

//--------------------------------------------------------------------------------------------------
void AlcaPCCProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){
    ftotalevents++;

    unsigned int bx=iEvent.bunchCrossing();
    //std::cout<<"The Bunch Crossing"<<bx<<std::endl;
    thePCCob->eventCounter(bx);

    //Looping over the clusters and adding the counts up  
    edm::Handle< edmNew::DetSetVector<SiPixelCluster> > hClusterColl;
    iEvent.getByToken(pixelToken,hClusterColl);

    const edmNew::DetSetVector<SiPixelCluster>& clustColl = *(hClusterColl.product()); 
    // ----------------------------------------------------------------------
    // -- Clusters without tracks
    for (auto const & mod: clustColl) {
        if(mod.empty()) { continue; }
        DetId detId = mod.id();

        // -- clusters on this det
        edmNew::DetSet<SiPixelCluster>::const_iterator  di;
        int nClusterCount=0;
        for (di = mod.begin(); di != mod.end(); ++di) {
            nClusterCount++;
        }
        int nCluster = mod.size();
        if(nCluster!=nClusterCount) {
            std::cout<<"counting yields "<<nClusterCount<<" but the size is "<<nCluster<<"; they should match."<<std::endl;
        }
        thePCCob->increment(detId(), bx, nCluster);
    }
}

//--------------------------------------------------------------------------------------------------
void AlcaPCCProducer::beginLuminosityBlock(edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup){
    //New PCC object at the beginning of each lumi section
    thePCCob = std::make_unique<reco::PixelClusterCounts>(); 
    beginLumiOfPCC_ = lumiSeg.luminosityBlock();
    countLumi_++;

}

//--------------------------------------------------------------------------------------------------
void AlcaPCCProducer::endLuminosityBlock(edm::LuminosityBlock const& lumiSeg, const edm::EventSetup& iSetup){
}

//--------------------------------------------------------------------------------------------------
void AlcaPCCProducer::endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup){

    endLumiOfPCC_ = lumiSeg.luminosityBlock();

    //Saving the PCC object 
    //std::cout<<"Saving Object "<<std::endl;
    lumiSeg.put(std::move(thePCCob), std::string(trigstring_)); 

}

DEFINE_FWK_MODULE(AlcaPCCProducer);
