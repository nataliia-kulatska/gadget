#include "optinfo.h"
#include "mathfunc.h"
#include "gadget.h"

/* global ecosystem used to store whether the model converged */
extern Ecosystem* EcoSystem;

/* global variable, defined and initialised in gadget.cc and not modified here */
extern int FuncEval;

extern ErrorHandler handle;

double func(double* x, int n) {
  return EcoSystem->SimulateAndUpdate(x, n);
}

double quadmin(double f0, double f1, double f0m, double alpha, double beta);
double cubmin(double f0, double f1, double f0m, double f1m, double alpha, double beta);

OptInfoBfgs::OptInfoBfgs()
  : OptSearch(), rho(0.01), tau(0.5), maxiter(100000), eps(0.001) {
  handle.logMessage("Initialising BFGS");
  numvar = EcoSystem->NoOptVariables();
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
  int i, count;
  DoubleVector val(numvar);
  double* startpoint = new double[numvar];
  EcoSystem->ScaledOptValues(val);
  for (i = 0; i < numvar; i++)
    startpoint[i] = val[i];

  count = iteration(startpoint);
  cout << "\nBFGS finished with final likelihood score of " << EcoSystem->getLikelihood()
    << "\nafter " << EcoSystem->getFuncEval() << " function evaluations at the point\n";
  EcoSystem->writeOptValues();
  delete[] startpoint;
}

void OptInfoBfgs::Read(CommentStream& infile, char* text) {
  while (!infile.eof() && strcasecmp(text, "seed") && strcasecmp(text, "[simann]") && strcasecmp(text, "[hooke]")) {
    if (strcasecmp(text,"rho") == 0) {
      infile >> rho >> ws >> text >> ws;

    } else if (strcasecmp(text, "tau") == 0) {
      infile >> tau >> ws >> text >> ws;

    } else if (strcasecmp(text, "maxiter") == 0) {
      infile >> maxiter >> ws >> text >> ws;

    } else if (strcasecmp(text, "eps") == 0) {
      infile >> eps >> ws >> text >> ws;

    } else {
      handle.logWarning("Warning in optinfofile - unknown option", text);
      infile >> text >> ws >> text >> ws;
    }
  }
}

int OptInfoBfgs::gaussian(double mult) {
  int i, j, k, maxrow;
  double tmp, Atemp[numvar + 1][numvar];
  for (i = 0; i < numvar; i++) {
    for (j = 0; j < numvar; j++)
      Atemp[i][j] = Bk[i][j];

    Atemp[numvar][i] = mult * gk[i];
  }

  for (i = 0; i < numvar; i++) {
    maxrow = i;
    for (j = i + 1; j < numvar; j++)
      if (absolute(Atemp[i][j]) > absolute(Atemp[i][maxrow]))
	maxrow = j;

    for (k = i; k < numvar + 1; k++) {
      tmp = Atemp[k][i];
      Atemp[k][i] = Atemp[k][maxrow];
      Atemp[k][maxrow] = tmp;
    }

    if (absolute(Atemp[i][i]) < verysmall) {
      handle.logWarning("Error in BFGS - Singular matrix approximation - Resetting BFGS");
      return 0;
    }

    for (j = i + 1; j < numvar; j++)
      for (k = numvar; k >= i; k--)
	Atemp[k][j] -= Atemp[k][i] * Atemp[i][j] / Atemp[i][i];
  }

  for (j = numvar - 1; j >= 0; j--) {
    tmp = 0.0;
    for (k = j + 1; k < numvar; k++)
      tmp += Atemp[k][j] * s[k];

    s[j] = (Atemp[numvar][j] - tmp) / Atemp[j][j];
  }

  return 1;
}

int OptInfoBfgs::iteration(double* x0) {
  double h[numvar], y[numvar], u[numvar];
  double hy = 0.0, hu = 0.0, alpha = 1, normgrad, temp;
  int i, j, k = 0, check = 0, offset = FuncEval;

  fk = (*f)(x0, numvar);
  gradient(x0, fk);
  for (i = 0; i < numvar; i++)
    g0[i] = gk[i];

  while (k < maxiter) {
    k++;
    if (check == 0 || alpha == 0) {
      for (i = 0; i < numvar; i++) { // linesearch the algoritm is restarted
	for (j = 0; j < numvar; j++)
	  Bk[i][j] = 0.0;
	Bk[i][i] = 1.0;
	x[i] = x0[i];
      }
    }

    check = gaussian(-1.0);
    if (check == 0)
      continue;

    alpha = linesearch();
    if (alpha == 0)
      continue;

    normgrad = 0.0;
    hy = 0.0;
    hu = 0.0;
    for (i = 0; i < numvar; i++) {
      h[i] = alpha * s[i];
      x[i] += h[i];
      y[i] = gk[i] - g0[i];
      g0[i] = gk[i];
      hy += h[i] * y[i];
      normgrad += gk[i] * gk[i];
    }

    for (i = 0; i < numvar; i++) {
      u[i] = 0.0;
      for (j = 0; j < numvar; j++)
	u[i] += Bk[i][j]*h[j];
      hu += h[i] * u[i];
    }

    if (hy != 0 && hu != 0)
      for (i = 0; i < numvar; i++)
	for (j = 0; j < numvar; j++)
	  Bk[i][j] += y[i] * y[j] / hy - u[i] * u[j] / hu;
    else
      check = 0;


    if (normgrad < eps) {
      EcoSystem->setConvergeBFGS(1);
      break;
    }
    cout << "\nNew optimum after " << FuncEval << " function evaluations, f(x) = " << fk << " at\n";
    for (i = 0; i < numvar; i++)
      cout << x[i] << sep;
    cout << endl;
  }

  EcoSystem->setFuncEvalBFGS(FuncEval - offset);
  EcoSystem->setLikelihoodBFGS(fk);
  return k;
}

void OptInfoBfgs::gradient(double* p, double fp) {
  int i, j;
  double h = 0.00001, tmp[numvar], temp;
  for (i = 0; i < numvar; i++) {
    for (j = 0; j < numvar; j++)
      tmp[j] = p[j];
    tmp[i] += h;
    gk[i] = ((*f)(tmp,numvar) - fp) / h;
  }
}

double quadmin(double f0, double f1, double f0m, double alpha, double beta) {
  double S, qa, qb, qt, t, tmin;
  S = f1 - f0 - f0m;
  qa = f0 + alpha * (f0m + alpha * S);
  qb = f0 + beta * (f0m + beta * S);
  if (absolute(S) > 1e-6)
    t = -f0m / (2.0 * S);
  else
    t = alpha;

  t = min(beta, max(alpha,t));
  qt = f0 + t * (f0m + t * S);
  if (qa < min(qt,qb))
    tmin = alpha;
  else if (qt < qb)
    tmin = t;
  else
    tmin = beta;
  return tmin;
}

double cubmin(double f0, double f1, double f0m, double f1m, double alpha, double beta) {
  double eta, ksi, ca, cb, ct, D, t, tmin;
  eta = 3.0 * (f1 - f0) - 2.0 * f0m - f1m;
  ksi = f0m + f1m - 2.0 * (f1 - f0);
  ca = f0 + alpha * (f0m + alpha * (eta + alpha * ksi));
  cb = f0 + beta * (f0m + beta * (eta + beta * ksi));
  D = eta * eta - 3.0 * ksi * f0m;
  if (absolute(ksi) < 1e-6)
    t = -f0m / (2.0 * eta);
  else if (D > 0)
    t = (-eta + sqrt(D)) / (3.0 * ksi);
  else
    t = alpha;
  t = min(beta, max(alpha,t));
  ct = f0 + t * (f0m + t * (eta + t * ksi));
  if (ca < min(ct, cb))
    tmin = alpha;
  else if (ct < cb)
    tmin = t;
  else
    tmin = beta;
  return tmin;
}

double OptInfoBfgs::linesearch() {
  int i, stop;
  double alpha, a, b, bnytt, fimin, fa, fim0 = 0.0, fi0, mu, fia, fima ,fib, Lb, temp, fialpha, Lalpha, fimalpha, d, fimb;
  double tmp[numvar];
  fimin = min(-1000.0, -100 * absolute(fk));
  fa = fk;
  for (i = 0; i < numvar; i++)
    fim0 += gk[i] * s[i];
  if (fim0 >= 0) {
    cout << "Error in Linesearch - Search direction not descending - BFGS restarted" << endl;
    return 0.0;
  }
  fi0 = fk;
  mu = (fimin - fk) / (rho * fim0);
  a = 0.0;
  fia = fi0;
  fima = fim0;
  b = 1.0;
  stop = 0;
  fimb = 0.0;
  for (;;) {
    if (isZero(a - b)) {
      cout << "Error in Linesearch - Empty search interval" << endl;
      return a;
    }
    for (i = 0; i < numvar; i++)
      tmp[i] = x[i] + b * s[i];

    fib = (*f)(tmp, numvar);

    if (fib != fib)
      fib = verybig;

    if (fib < fimin)
      break;

    Lb = fi0 + b * rho * fim0;
    if (fib >= Lb || fib > fia) {
      stop = 1;
      break;
    }

    gradient(tmp, fib);
    fimb = 0.0;
    for (i = 0; i < numvar; i++)
      fimb += gk[i] * s[i];

    if (absolute(fimb) < -tau * fim0)
      break;

    if (fimb > 0) {
      stop = 2;
      break;
    }

    d = b - a;
    if (mu < b + d)
      bnytt = mu;
    else
      bnytt = b + d * cubmin(fia, fib, fima * d, fimb * d, 2.0, min(9.0, (mu - b) / d));
    a = b;
    b = bnytt;
    fia = fib;
    fima = fimb;
    fimb = 0.0;
  }

  alpha = b;
  fa = fib;
  if (stop == 2) { //switching values of a and b
    temp = b;
    b = a;
    a = temp;

    temp = fib;
    fib = fia;
    fia = temp;

    temp = fimb;
    fimb = fima;
    fima = temp;
  }
  while (stop != 0) {
    d = absolute(b - a);
    if (a < b)
      if (fimb == 0)
	alpha = a + d * quadmin(fia, fib, fima * d, 0.1, 0.5);
      else
	alpha = a + d * cubmin(fia, fib, fima * d, fimb * d, 0.1, 0.5);
    else
      if (fimb == 0)
	alpha = a - d * quadmin(fia, fib, fima * d, 0.1, 0.5);
      else
	alpha = a + d * cubmin(fia, fib, fima * d, fimb * d, 0.5, 0.9);

    if (alpha <= min(a, b) || alpha >= max(a, b))
      alpha = (a + b) * 0.5;

    for (i = 0; i < numvar; i++)
      tmp[i] = x[i] + alpha * s[i];

    fa = (*f)(tmp, numvar);

    if (fa != fa) //check for nan
      fa = verybig;

    fialpha = fa;
    if (d < max(absolute(a), absolute(b)) * rathersmall)
      break;

    Lalpha = fi0 + alpha * rho * fim0;
    if (fialpha > Lalpha || fialpha > fia) {
      b = alpha;
      fib = fialpha;
      fimb = 0;
    } else {
      gradient(tmp, fialpha);
      fimalpha = 0.0;
      for (i = 0; i < numvar; i++)
	fimalpha += gk[i] * s[i];

      if (absolute(fimalpha) < -tau * fim0)
	break;

      if ((b < a && fimalpha < 0) || (b > a && fimalpha > 0)) {
	b = a;
	fib = fia;
	fimb = fima;
      }

      a = alpha;
      fia = fialpha;
      fima = fimalpha;
    }
  }
  fk = fa;
  return alpha;
}