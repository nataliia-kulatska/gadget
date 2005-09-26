#include "stockpredator.h"
#include "keeper.h"
#include "errorhandler.h"
#include "readfunc.h"
#include "prey.h"
#include "areatime.h"
#include "suits.h"
#include "readword.h"
#include "gadget.h"

extern ErrorHandler handle;

StockPredator::StockPredator(CommentStream& infile, const char* givenname, const IntVector& Areas,
  const LengthGroupDivision* const OtherLgrpDiv, const LengthGroupDivision* const GivenLgrpDiv,
  int minage, int maxage, const TimeClass* const TimeInfo, Keeper* const keeper)
  : PopPredator(givenname, Areas, OtherLgrpDiv, GivenLgrpDiv) {

  type = STOCKPREDATOR;
  keeper->addString("predator");
  int i;
  char text[MaxStrLength];
  strncpy(text, "", MaxStrLength);

  //first read in the suitability parameters
  infile >> text >> ws;
  if (!(strcasecmp(text, "suitability") == 0))
    handle.logFileUnexpected(LOGFAIL, "suitability", text);
  this->readSuitability(infile, TimeInfo, keeper);

  //now we read in the prey preference parameters - should be one for each prey
  keeper->addString("preypreference");
  int count = 0;
  infile >> text >> ws;
  while (!(strcasecmp(text, "maxconsumption") == 0) && (!infile.eof())) {
    for (i = 0; i < preference.Size(); i++) {
      if (strcasecmp(text, this->getPreyName(i)) == 0) {
        infile >> preference[i] >> ws;
        count++;
      }
    }
    infile >> text >> ws;
  }      
  if (count != preference.Size())
    handle.logMessage(LOGFAIL, "Error in stockpredator - missing prey preference data");
  preference.Inform(keeper);
  keeper->clearLast();  

  //then read in the maximum consumption parameters
  if (!(strcasecmp(text, "maxconsumption") == 0))
    handle.logFileUnexpected(LOGFAIL, "maxconsumption", text);
  keeper->addString("consumption");
  consParam.resize(5, keeper);
  for (i = 0; i < 4; i++)
    if (!(infile >> consParam[i]))
      handle.logFileMessage(LOGFAIL, "invalid format for maxconsumption vector");

  readWordAndVariable(infile, "halffeedingvalue", consParam[4]);
  consParam.Inform(keeper);
  keeper->clearLast();
  keeper->clearLast();

  //everything has been read from infile ... resize objects
  int numlength = LgrpDiv->numLengthGroups();
  int numarea = areas.Size();
  IntVector size(maxage - minage + 1, numlength);
  IntVector minlength(maxage - minage + 1, 0);
  BandMatrix bm(minlength, size, minage);

  Alkeys.resize(numarea, minage, minlength, size);
  Alprop.resize(numarea, bm);
  maxcons.AddRows(numarea, numlength, 0.0);
  Phi.AddRows(numarea, numlength, 0.0);
  fphi.AddRows(numarea, numlength, 0.0);
  subfphi.AddRows(numarea, numlength, 0.0);
}

void StockPredator::Print(ofstream& outfile) const {
  int i, area;
  outfile << "\nStock predator\n";
  PopPredator::Print(outfile);
  for (area = 0; area < areas.Size(); area++) {
    outfile << "\tPhi on internal area " << areas[area] << ":\n\t";
    for (i = 0; i < fphi.Ncol(area); i++)
      outfile << setw(smallwidth) << setprecision(smallprecision) << fphi[area][i] << sep;
    outfile << "\n\tAlkeys (numbers) on internal area " << areas[area] << ":\n";
    Alkeys[area].printNumbers(outfile);
    outfile << "\tAlkeys (mean weights) on internal area " << areas[area] << ":\n";
    Alkeys[area].printWeights(outfile);
    outfile << "\tAge-length proportion on internal area " << areas[area] << ":\n";
    Alprop[area].Print(outfile);
    outfile << "\tMaximum consumption by length on internal area " << areas[area] << ":\n\t";
    for (i = 0; i < maxcons.Ncol(); i++)
      outfile << setw(smallwidth) << setprecision(smallprecision) << maxcons[area][i] << sep;
    outfile << endl;
  }
  outfile << endl;
}

void StockPredator::Sum(const AgeBandMatrix& stock, int area) {
  int age, len, inarea = this->areaNum(area);
  Alkeys[inarea].setToZero();
  Alkeys[inarea].Add(stock, *CI);
  Alkeys[inarea].sumColumns(prednumber[inarea]);

  for (age = Alprop[inarea].minRow(); age <= Alprop[inarea].maxRow(); age++) {
    for (len = Alprop[inarea].minCol(age); len < Alprop[inarea].maxCol(age); len++) {
      if (!(isZero(prednumber[inarea][len].N)))
        Alprop[inarea][age][len] = Alkeys[inarea][age][len].N / prednumber[inarea][len].N;
      else
        Alprop[inarea][age][len] = 0.0;
    }
  }
}

void StockPredator::Reset(const TimeClass* const TimeInfo) {
  PopPredator::Reset(TimeInfo);

  // check that the various parameters that can be estimated are sensible
  if ((TimeInfo->getTime() == 1) && (handle.getLogLevel() >= LOGWARN)) {
    int i;
    for (i = 0; i < consParam.Size(); i++)
      if (consParam[i] < 0)
        handle.logMessage(LOGWARN, "Warning in stockpredator - negative consumption parameter", consParam[i]);
  }
}

void StockPredator::Eat(int area, const AreaClass* const Area, const TimeClass* const TimeInfo) {

  int prey, predl, preyl;
  int inarea = this->areaNum(area);
  double tmp, temperature;

  for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++)
    Phi[inarea][predl] = 0.0;

  if (TimeInfo->getSubStep() == 1) {
    for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++)
      fphi[inarea][predl] = 0.0;

    temperature = Area->getTemperature(area, TimeInfo->getTime());
    tmp = exp(temperature * (consParam[1] - temperature * temperature * consParam[2]))
           * consParam[0] * TimeInfo->LengthOfCurrent() / TimeInfo->numSubSteps();

    for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++)
      maxcons[inarea][predl] = tmp * pow(LgrpDiv->meanLength(predl), consParam[3]);
  }

  //Now maxconbylength contains the maximum consumption by length
  //Calculating Phi(L) and O(l,L,prey) based on energy requirements
  for (prey = 0; prey < this->numPreys(); prey++) {
    if (this->getPrey(prey)->isPreyArea(area)) {
      for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++) {
        for (preyl = 0; preyl < this->getPrey(prey)->getLengthGroupDiv()->numLengthGroups(); preyl++) {
          tmp = this->getSuitability(prey)[predl][preyl] *
            this->getPrey(prey)->getBiomass(area, preyl) * this->getPrey(prey)->getEnergy();
          (*cons[inarea][prey])[predl][preyl] = pow(tmp, preference[prey]);
          Phi[inarea][predl] += (*cons[inarea][prey])[predl][preyl];
        }
      }

    } else {
      for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++)
        for (preyl = 0; preyl < this->getPrey(prey)->getLengthGroupDiv()->numLengthGroups(); preyl++)
          (*cons[inarea][prey])[predl][preyl] = 0.0;
    }
  }

  //Calculating fphi(L) and totalcons of predator in area
  tmp = Area->getSize(area) * consParam[4];
  for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++) {
    if (isZero(tmp))
      subfphi[inarea][predl] = 1.0;
    else if (isZero(Phi[inarea][predl]) || isZero(Phi[inarea][predl] + tmp))
      subfphi[inarea][predl] = 0.0;
    else
      subfphi[inarea][predl] = Phi[inarea][predl] / (Phi[inarea][predl] + tmp);

    totalcons[inarea][predl] = subfphi[inarea][predl] *
      maxcons[inarea][predl] * prednumber[inarea][predl].N;
  }

  //Distributing the total consumption on the preys and converting to biomass
  for (prey = 0; prey < this->numPreys(); prey++) {
    if (this->getPrey(prey)->isPreyArea(area)) {
      for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++) {
        if (!(isZero(Phi[inarea][predl]))) {
          tmp = totalcons[inarea][predl] / (Phi[inarea][predl] * this->getPrey(prey)->getEnergy());
          for (preyl = 0; preyl < this->getPrey(prey)->getLengthGroupDiv()->numLengthGroups(); preyl++)
            (*cons[inarea][prey])[predl][preyl] *= tmp;
        }
      }
    }
  }

  //Add the calculated consumption to the preys in question
  for (prey = 0; prey < this->numPreys(); prey++)
    if (this->getPrey(prey)->isPreyArea(area))
      for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++)
        this->getPrey(prey)->addBiomassConsumption(area, (*cons[inarea][prey])[predl]);
}

//Check if any of the preys of the predator are eaten up.
//adjust the consumption according to that.
void StockPredator::adjustConsumption(int area, const TimeClass* const TimeInfo) {
  double maxRatio = pow(MaxRatioConsumed, TimeInfo->numSubSteps());
  int inarea = this->areaNum(area);
  int over, preyl, predl, prey;
  double ratio, rat1, rat2, tmp;

  for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++)
    overcons[inarea][predl] = 0.0;

  over = 0;
  for (prey = 0; prey < this->numPreys(); prey++) {
    if (this->getPrey(prey)->isPreyArea(area)) {
      if (this->getPrey(prey)->isOverConsumption(area)) {
        over = 1;
        for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++) {
          for (preyl = 0; preyl < this->getPrey(prey)->getLengthGroupDiv()->numLengthGroups(); preyl++) {
            ratio = this->getPrey(prey)->getRatio(area, preyl);
            if (ratio > maxRatio) {
              tmp = maxRatio / ratio;
              overcons[inarea][predl] += (1.0 - tmp) * (*cons[inarea][prey])[predl][preyl];
              (*cons[inarea][prey])[predl][preyl] *= tmp;
            }
          }
        }
      }
    }
  }

  if (over == 1) {
    for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++) {
      if (totalcons[inarea][predl] > verysmall) {
        ratio = 1.0 - (overcons[inarea][predl] / totalcons[inarea][predl]);
        subfphi[inarea][predl] *= ratio;
        totalcons[inarea][predl] -= overcons[inarea][predl];
      }
    }
  }

  rat2 = 1.0 / TimeInfo->getSubStep();
  rat1 = 1.0 - rat2;
  for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++) {
    totalconsumption[inarea][predl] += totalcons[inarea][predl];
    overconsumption[inarea][predl] += overcons[inarea][predl];
    fphi[inarea][predl] = (rat2 * subfphi[inarea][predl]) + (rat1 * fphi[inarea][predl]);
  }

  for (prey = 0; prey < this->numPreys(); prey++)
    if (this->getPrey(prey)->isPreyArea(area))
      for (predl = 0; predl < LgrpDiv->numLengthGroups(); predl++)
        for (preyl = 0; preyl < this->getPrey(prey)->getLengthGroupDiv()->numLengthGroups(); preyl++)
          (*consumption[inarea][prey])[predl][preyl] += (*cons[inarea][prey])[predl][preyl];
}
