#include "optinfo.h"
#include "ecosystem.h"
#include "gadget.h"

extern Ecosystem* EcoSystem;
extern ErrorHandler handle;

double func(double* x, int n) {
  return EcoSystem->SimulateAndUpdate(x, n);
}

OptInfoBfgs::OptInfoBfgs()
  : OptSearch(), bfgsiter(100000), bfgseps(0.001), beta(0.3), sigma(0.01), st(1.0) {
  handle.logMessage("Initialising BFGS optimisation algorithm");
  numvar = EcoSystem->numOptVariables();
  int i;
  x = new double[numvar];
  s = new double[numvar];
  gk = new double[numvar];
  g0 = new double[numvar];
  Bk = new double*[numvar];
  f = &func;
  for (i = 0; i < numvar; i++) {
    Bk[i] = new double[numvar];
  }
}

OptInfoBfgs::~OptInfoBfgs() {
  int i;
  delete[] x;
  delete[] s;
  delete[] gk;
  delete[] g0;
  for (i = 0; i < numvar; i++)
    delete[] Bk[i];
}

void OptInfoBfgs::MaximizeLikelihood() {
  int i, opt;

  handle.logInformation("\nStarting BFGS optimisation algorithm");

  DoubleVector val(numvar);
  double* startpoint = new double[numvar];
  EcoSystem->ScaledOptValues(val);
  for (i = 0; i < numvar; i++)
    startpoint[i] = val[i];

  opt = iteration(startpoint);
  cout << "\nBFGS finished with final likelihood score of " << EcoSystem->getLikelihood()
    << "\nafter a total of " << EcoSystem->getFuncEval() << " function evaluations at the point\n";
  EcoSystem->writeOptValues();

  delete[] startpoint;
}

void OptInfoBfgs::Read(CommentStream& infile, char* text) {
  while (!infile.eof() && strcasecmp(text, "seed") && strcasecmp(text, "[simann]") && strcasecmp(text, "[hooke]")) {
    infile >> ws;
    if (strcasecmp(text,"beta") == 0) {
      infile >> beta;

    } else if (strcasecmp(text, "sigma") == 0) {
      infile >> sigma;

    } else if (strcasecmp(text, "st") == 0) {
      infile >> st;

    } else if ((strcasecmp(text, "maxiterations") == 0) || (strcasecmp(text, "maxiter") == 0) || (strcasecmp(text, "bfgsiter") == 0)) {
      infile >> bfgsiter;

    } else if ((strcasecmp(text, "eps") == 0) || (strcasecmp(text, "bfgseps") == 0)) {
      infile >> bfgseps;

    } else {
      handle.logWarning("Warning in optinfofile - unknown option", text);
      infile >> text;  //read and ignore the next entry
    }
    infile >> text;
  }
}
