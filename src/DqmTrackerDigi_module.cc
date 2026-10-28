//
// Offline tracker digi DQM: fills mu2e::TrackerDigiDQM (Offline/TrackerDQM)
// from StrawDigis. Needs no geometry or conditions.
//

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art_root_io/TFileService.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Table.h"

#include "Offline/DQMHelpers/inc/DQMHistSetConfig.hh"
#include "Offline/RecoDataProducts/inc/StrawDigi.hh"
#include "Offline/TrackerDQM/inc/TrackerDigiDQM.hh"

#include <fstream>
#include <string>

namespace mu2e {

class DqmTrackerDigi : public art::EDAnalyzer {
 public:
  struct Config {
    using Name = fhicl::Name;
    using Comment = fhicl::Comment;
    fhicl::Atom<art::InputTag> digiTag{Name("digiTag"), Comment("StrawDigi collection")};
    fhicl::Table<DQMClientFhicl> dqm{
        Name("dqm"), Comment("TrackerDigiDQM configuration; see Offline/TrackerDQM/fcl/prolog.fcl")};
    fhicl::Atom<std::string> catalogueFile{
        Name("catalogueFile"),
        Comment("write the histogram catalogue (name, type, axes) here at end of job; empty skips it"),
        ""};
  };
  typedef art::EDAnalyzer::Table<Config> Parameters;

  explicit DqmTrackerDigi(const Parameters& conf);

  void beginJob() override;
  void beginSubRun(const art::SubRun& sr) override;
  void endSubRun(const art::SubRun& sr) override;
  void analyze(const art::Event& event) override;
  void endJob() override;

 private:
  art::InputTag _digiTag;
  std::string _catalogueFile;
  TrackerDigiDQM _dqm;
};

DqmTrackerDigi::DqmTrackerDigi(const Parameters& conf) :
    art::EDAnalyzer(conf), _digiTag(conf().digiTag()), _catalogueFile(conf().catalogueFile()),
    _dqm(toConfig(conf().dqm().hists())) {}

void DqmTrackerDigi::beginJob() {
  art::ServiceHandle<art::TFileService> tfs;
  _dqm.Book(*tfs);
}

void DqmTrackerDigi::beginSubRun(const art::SubRun& sr) { _dqm.BeginSubRun(sr.run(), sr.subRun()); }

void DqmTrackerDigi::endSubRun(const art::SubRun&) { _dqm.EndSubRun(); }

void DqmTrackerDigi::analyze(const art::Event& event) {
  //a missing input throws: an empty DQM file must not look like a quiet tracker
  _dqm.Fill(*event.getValidHandle<StrawDigiCollection>(_digiTag));
}

void DqmTrackerDigi::endJob() {
  _dqm.EndJob();
  if (!_catalogueFile.empty()) {
    std::ofstream out(_catalogueFile);
    _dqm.hists().WriteCatalogue(out);
  }
}

}  // namespace mu2e

DEFINE_ART_MODULE(mu2e::DqmTrackerDigi)
