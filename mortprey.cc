#include "mortprey.h"
#include "print.h"
#include "intvector.h"
#include "gadget.h"

MortPrey::MortPrey(CommentStream& infile, const intvector& Areas,
  const char* givenname, int minage, int maxage,
  Keeper* const keeper, const LengthGroupDivision* const stock_lgrp)
  : Prey(infile, Areas, givenname, keeper) {

  prey_lgrp = new LengthGroupDivision(*LgrpDiv);
  delete LgrpDiv; //wrong dimensions set in Prey's constructor
  LgrpDiv = new LengthGroupDivision(*stock_lgrp);

  this->InitializeObjects();

  int numlength = LgrpDiv->NoLengthGroups();
  int numarea = areas.Size();
  intvector size(maxage - minage + 1, numlength);
  intvector minlength(maxage - minage + 1, 0);
  Alkeys.resize(numarea, minage, minlength, size);
  mean_n.resize(numarea, minage, minlength, size);
  haveCalculatedMeanN.resize(numarea, 0);
  z.AddRows(numarea, numlength, 0.0);
  mort_fact.AddRows(numarea, numlength, 0.0);
  prop_surv.AddRows(numarea, numlength, 0.0);
  cannibalism.AddRows(numarea, numlength, 0.0);
  cann_is_true = 0;
}

MortPrey::MortPrey(const doublevector& lengths, const intvector& Areas, int minage,
  int maxage, const char* givenname, const LengthGroupDivision* const stock_lgrp)
  : Prey(lengths, Areas, givenname) {

  prey_lgrp = new LengthGroupDivision(*LgrpDiv);
  delete LgrpDiv; //wrong dimensions set in Prey's constructor
  LgrpDiv = new LengthGroupDivision(*stock_lgrp);

  this->InitializeObjects();

  int numlength = LgrpDiv->NoLengthGroups();
  int numarea = areas.Size();
  intvector size(maxage - minage + 1, numlength);
  intvector minlength(maxage - minage + 1, 0);
  Alkeys.resize(numarea, minage, minlength, size);
  mean_n.resize(numarea, minage, minlength, size);
  haveCalculatedMeanN.resize(numarea, 0);
  z.AddRows(numarea, numlength, 0.0);
  mort_fact.AddRows(numarea, numlength, 0.0);
  prop_surv.AddRows(numarea, numlength, 0.0);
  cannibalism.AddRows(numarea, numlength, 0.0);
  cann_is_true = 0;
}

MortPrey::~MortPrey() {
  int i;
  delete prey_lgrp;
  for (i = 0; i < cannprednames.Size(); i++)
    delete[] cannprednames[i];
}

void MortPrey::InitializeObjects() {
  popinfo nullpop;

  while (Number.Nrow())
    Number.DeleteRow(0);
  while (numberPriortoEating.Nrow())
    numberPriortoEating.DeleteRow(0);
  while (biomass.Nrow())
    biomass.DeleteRow(0);
  while (cons.Nrow())
    cons.DeleteRow(0);
  while (consumption.Nrow())
    consumption.DeleteRow(0);
  while (tooMuchConsumption.Size())
    tooMuchConsumption.Delete(0);
  while (total.Size())
    total.Delete(0);
  while (ratio.Nrow())
    ratio.DeleteRow(0);
  while (overcons.Nrow())
    overcons.DeleteRow(0);
  while (overconsumption.Nrow())
    overconsumption.DeleteRow(0);

  //Now we can resize the objects.
  int numlength = LgrpDiv->NoLengthGroups();
  int numarea = areas.Size();

  Number.AddRows(numarea, numlength, nullpop);
  numberPriortoEating.AddRows(numarea, numlength, nullpop);
  biomass.AddRows(numarea, numlength, 0.0);
  cons.AddRows(numarea, numlength, 0.0);
  consumption.AddRows(numarea, numlength, 0.0);
  tooMuchConsumption.resize(numarea, 0);
  total.resize(numarea, 0.0);
  ratio.AddRows(numarea, numlength, 0.0);
  overcons.AddRows(numarea, numlength, 0.0);
  overconsumption.AddRows(numarea, numlength, 0.0);
}

void MortPrey::Sum(const Agebandmatrix& stock, int area, int CurrentSubstep) {
  //written by kgf 22/6 98
  int inarea = AreaNr[area];
  int i, j;

  tooMuchConsumption[inarea] = 0;
  for (i = 0; i < cons.Ncol(inarea); i++)
    cons[inarea][i] = 0.0;
  for (i = 0; i < Number[inarea].Size(); i++)
    Number[inarea][i].N = 0.0;
  for (i = 0; i < cannibalism[inarea].Size(); i++)
    cannibalism[inarea][i] = 0.0;

  mean_n[inarea].SettoZero();
  Alkeys[inarea].SettoZero(); //The next line copies stock to Alkeys and mean_n
  AgebandmAdd(mean_n[inarea], stock, *CI);
  AgebandmAdd(Alkeys[inarea], stock, *CI);
  Alkeys[inarea].Colsum(Number[inarea]);
  haveCalculatedMeanN[inarea] = 0;

  popinfo sum;
  for (i = 0; i < Number.Ncol(inarea); i++) {
    sum += Number[inarea][i];
    biomass[inarea][i] = Number[inarea][i].N * Number[inarea][i].W;
  }

  total[inarea] = sum.N * sum.W;
  for (i = 0; i < Number[inarea].Size(); i++)
    numberPriortoEating[inarea][i] = Number[inarea][i]; //should be inside if

  if (CurrentSubstep == 1) {
    for (j = 0; j < consumption.Ncol(inarea); j++) {
      consumption[inarea][j] = 0.0;
      overconsumption[inarea][j] = 0.0;
    }
  }
}

const Agebandmatrix& MortPrey::AlkeysPriorToEating(int area) const {
  return Alkeys[AreaNr[area]];
}

const Agebandmatrix& MortPrey::getMeanN(int area) const {
  return mean_n[AreaNr[area]];
}

void MortPrey::Print(ofstream& outfile) const {
  //must be modified! kgf 7/7 98
  outfile << "MortPrey\n";
  int area;
  for (area = 0; area < areas.Size(); area++) {
    outfile << "Alkeys on area " << areas[area] << endl;
    Printagebandm(outfile, Alkeys[area]);
  }
  Prey::Print(outfile);
}

void MortPrey::Reset() {
  this->Prey::Reset();
  int area, age, l;
  for (area = 0; area < areas.Size(); area++)   {
    haveCalculatedMeanN[area] = 0;
    for (age = Alkeys[area].Minage(); age <= Alkeys[area].Maxage(); age++) {
      for (l = Alkeys[area].Minlength(age); l < Alkeys[area].Maxlength(age); l++) {
        Alkeys[area][age][l].N = 0.0;
        Alkeys[area][age][l].W = 0.0;
        mean_n[area][age][l].N = 0.0;
        mean_n[area][age][l].W = 0.0;
      }
    }
    for (l = 0; l < z[area].Size(); l++) {
      z[area][l] = 0.0;
      mort_fact[area][l] = 0.0;
      cannibalism[area][l] = 0.0;
    }
  }
}

void MortPrey::calcMeanN(int area) {
  //written by kgf 24/6 98
  assert(haveCalculatedMeanN[area] == 0);
  haveCalculatedMeanN[area] = 1;
  const int inarea = AreaNr[area];
  int l;
  for (l = 0; l < LgrpDiv->NoLengthGroups(); l++) {
    prop_surv[inarea][l] = exp(- z[inarea][l]);
    if (iszero(z[inarea][l]))
      mort_fact[inarea][l] = 1.0;
    else
      mort_fact[inarea][l] = (1.0 - prop_surv[inarea][l]) / z[inarea][l];
  }
  mean_n[inarea].Multiply(mort_fact[inarea], *CI);
}

void MortPrey::calcZ(int area, const doublevector& natural_m) {
  //written by kgf 25/6 98
  //cannibalism added by kgf 13/7 98
  int i, upp_lim, inarea = AreaNr[area];

  if (z.Ncol(inarea) != natural_m.Size())
    upp_lim = min(z.Ncol(inarea), natural_m.Size());
  else
    upp_lim = z.Ncol(inarea);

  if (upp_lim < z.Ncol(inarea)) {
    for (i = 0; i < upp_lim; i++)
      z[inarea][i] = natural_m[i] + cons[inarea][i] + cannibalism[inarea][i];
    for (i = upp_lim; i < z.Ncol(inarea); i++)
      z[inarea][i] = cons[inarea][i] + cannibalism[inarea][i];
  } else {
    for (i = 0; i < z.Ncol(inarea); i++)
      z[inarea][i] = natural_m[i] + cons[inarea][i] + cannibalism[inarea][i];
  }
}

void MortPrey::setCannibalism(int area, const doublevector& cann) {
  //written by kgf 13/7 98
  int i, upp_lim, inarea = AreaNr[area];

  if (cannibalism.Ncol(inarea) != cann.Size())
    upp_lim = min(cannibalism.Ncol(inarea), cann.Size());
  else
    upp_lim = cannibalism.Ncol(inarea);
  for (i = 0; i < upp_lim; i++)
    cannibalism[inarea][i] = cann[i];
}

void MortPrey::addAgeGroupMatrix(doublematrix* const agematrix) {
  agegroupmatrix.resize(1, agematrix);
}

void MortPrey::setAgeMatrix(int pred_no, int area, const doublevector& agegroupno) {
  (*agegroupmatrix[pred_no])[area] = agegroupno;
}

void MortPrey::setConsumption(int area, int pred_no, const bandmatrix& consum) {
  cann_cons.ChangeElement(AreaNr[area], pred_no, consum);
}

void MortPrey::addCannPredName(const char* predname) {
  char* tempName = new char[strlen(predname) + 1];
  strcpy(tempName, predname);
  cannprednames.resize(1, tempName);
}

void MortPrey::addConsMatrix(int pred_no, const bandmatrix& cons_mat) {
  //written by kgf 4/3 99
  //Note that the predator number and the dimensions of the band
  //matrices are supposed to be equal for all areas.
  int a;
  for (a = 0; a < cann_cons.Nrow(); a++)
    cann_cons.ChangeElement(a, pred_no, cons_mat);
}
