#include "abstrpredstdinfo.h"

AbstrPredStdInfo::AbstrPredStdInfo(const IntVector& areas, int predminage,
  int predmaxage, int preyminage, int preymaxage) : LivesOnAreas(areas) {

  IntVector minage(predmaxage - predminage + 1, preyminage);
  IntVector size(predmaxage - predminage + 1, preymaxage - preyminage + 1);
  BandMatrix bm(minage, size, predminage);
  NconbyAge.resize(areas.Size(), bm);
  BconbyAge.resize(areas.Size(), bm);
  MortbyAge.resize(areas.Size(), bm);
}

const BandMatrix& AbstrPredStdInfo::NconsumptionByAge(int area) const {
  return NconbyAge[this->areaNum(area)];
}

const BandMatrix& AbstrPredStdInfo::BconsumptionByAge(int area) const {
  return BconbyAge[this->areaNum(area)];
}

const BandMatrix& AbstrPredStdInfo::MortalityByAge(int area) const {
  return MortbyAge[this->areaNum(area)];
}
