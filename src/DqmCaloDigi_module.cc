//
// Offline calorimeter digi DQM: fills mu2e::CaloDigiDQM (Offline/CaloDQM)
// from CaloDigis. Needs no geometry or conditions.
//

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art_root_io/TFileService.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Table.h"

#include "Offline/CaloDQM/inc/CaloDigiDQM.hh"
#include "Offline/DQMHelpers/inc/DQMHistSetConfig.hh"
#include "Offline/RecoDataProducts/inc/CaloDigi.hh"

#include <fstream>
#include <string>

namespace mu2e {

class DqmCaloDigi : public art::EDAnalyzer {
 public:
  struct Config {
    using Name = fhicl::Name;
    using Comment = fhicl::Comment;
    fhicl::Atom<art::InputTag> digiTag{Name("digiTag"), Comment("CaloDigi collection")};
    fhicl::Table<DQMClientFhicl> dqm{
        Name("dqm"), Comment("CaloDigiDQM configuration; see Offline/CaloDQM/fcl/prolog.fcl")};
    fhicl::Atom<std::string> catalogueFile{
        Name("catalogueFile"),
        Comment("write the histogram catalogue (name, type, axes) here at end of job; empty skips it"),
        ""};
  };
  typedef art::EDAnalyzer::Table<Config> Parameters;

  explicit DqmCaloDigi(const Parameters& conf);

  void beginJob() override;
  void beginSubRun(const art::SubRun& sr) override;
  void endSubRun(const art::SubRun& sr) override;
  void analyze(const art::Event& event) override;
  void endJob() override;

 private:
  art::InputTag _digiTag;
  std::string _catalogueFile;
  CaloDigiDQM _dqm;
};

DqmCaloDigi::DqmCaloDigi(const Parameters& conf) :
    art::EDAnalyzer(conf), _digiTag(conf().digiTag()), _catalogueFile(conf().catalogueFile()),
    _dqm(toConfig(conf().dqm().hists())) {}

void DqmCaloDigi::beginJob() {
  art::ServiceHandle<art::TFileService> tfs;
  _dqm.Book(*tfs);
}

void DqmCaloDigi::beginSubRun(const art::SubRun& sr) { _dqm.BeginSubRun(sr.run(), sr.subRun()); }

void DqmCaloDigi::endSubRun(const art::SubRun&) { _dqm.EndSubRun(); }

void DqmCaloDigi::analyze(const art::Event& event) {
  //a missing input throws: an empty DQM file must not look like a quiet calorimeter
  _dqm.Fill(*event.getValidHandle<CaloDigiCollection>(_digiTag));
}

void DqmCaloDigi::endJob() {
  _dqm.EndJob();
  if (!_catalogueFile.empty()) {
    std::ofstream out(_catalogueFile);
    _dqm.hists().WriteCatalogue(out);
  }
}

}  // namespace mu2e

DEFINE_ART_MODULE(mu2e::DqmCaloDigi)
