#ifndef ecosystem_h
#define ecosystem_h

#include "stock.h"
#include "baseclassptrvector.h"
#include "otherfoodptrvector.h"
#include "readfunc.h"
#include "errorhandler.h"
#include "tagptrvector.h"
#include "printerptrvector.h"
#include "printer.h"
#include "fleet.h"
#include "printinfo.h"
#include "gadget.h"

class Ecosystem {
public:
  /**
   * \brief This is the default Ecosystem constructor
   */
  Ecosystem();
  /**
   * \brief This is the Ecosystem constructor specifying details about the model
   * \param filename is the name of the file to read the main model parameters from
   * \param optimise is a flag to denote whether the current run will optimise the model or not
   * \param netrun is a flag to denote whether the current run has been started in network mode or not
   * \param calclikelihood is a flag to denote whether the current run should calculate the likelihood score or not
   * \param inputdir is the name of the directory containing the input files to the model
   * \param workingdir is the name of the directory used for the output from the model
   * \param pi is the PrintInfo specifying the parameter output from the model run
   */
  Ecosystem(const char* const filename, int optimise, int netrun, int calclikelihood,
    const char* const inputdir, const char* const workingdir, const PrintInfo& pi);
  /**
   * \brief This is the default Ecosystem destructor
   */
  ~Ecosystem();
  /**
   * \brief This function will read the model data from the main input file
   * \param infile is the CommentStream to read the likelihood data from
   * \param optimise is a flag to denote whether the current run will optimise the model or not
   * \param netrun is a flag to denote whether the current run has been started in network mode or not
   * \param calclikelihood is a flag to denote whether the current run should calculate the likleihood score or not
   * \param inputdir is the name of the directory containing the input files to the model
   * \param workingdir is the name of the directory used for the output from the model
   */
  void readMain(CommentStream& infile, int optimise, int netrun, int calclikelihood,
    const char* const inputdir, const char* const workingdir);
  /**
   * \brief This function will read the likelihood data from the input file
   * \param infile is the CommentStream to read the likelihood data from
   */
  void readLikelihood(CommentStream& infile);
  /**
   * \brief This function will read the printer data from the input file
   * \param infile is the CommentStream to read the printer data from
   */
  void readPrinters(CommentStream& infile);
  /**
   * \brief This function will read the fleet data from the input file
   * \param infile is the CommentStream to read the fleet data from
   */
  void readFleet(CommentStream& infile);
  /**
   * \brief This function will read the tagging data from the input file
   * \param infile is the CommentStream to read the tagging data from
   */
  void readTagging(CommentStream& infile);
  /**
   * \brief This function will read the otherfood data from the input file
   * \param infile is the CommentStream to read the otherfood data from
   */
  void readOtherFood(CommentStream& infile);
  /**
   * \brief This function will read the stock data from the input file
   * \param infile is the CommentStream to read the stock data from
   */
  void readStock(CommentStream& infile);
  /**
   * \brief This function will write the current model status to file
   * \param filename is the name of the file to write the model information to
   */
  void writeStatus(const char* filename) const;
  /**
   * \brief This function will write header information about the model parameters to file
   * \param filename is the name of the file to write the model information to
   */
  void writeInitialInformation(const char* const filename) const;
  /**
   * \brief This function will write header information about the model parameters to file in a column format
   * \param filename is the name of the file to write the model information to
   */
  void writeInitialInformationInColumns(const char* const filename) const;
  /**
   * \brief This function will write current information about the model parameters to file
   * \param filename is the name of the file to write the model information to
   * \param prec is the precision to use in the output file
   */
  void writeValues(const char* const filename, int prec) const;
  /**
   * \brief This function will write current information about the model parameters to file in a column format
   * \param filename is the name of the file to write the model information to
   * \param prec is the precision to use in the output file
   */
  void writeValuesInColumns(const char* const filename, int prec) const;
  /**
   * \brief This function will write final information about the model parameters to file in a column format (which can then be used as the starting point for a subsequent model run)
   * \param filename is the name of the file to write the model information to
   * \param prec is the precision to use in the output file
   */
  void writeParamsInColumns(const char* const filename, int prec) const;
  /**
   * \brief This function will write information about the likelihood components to file
   * \param filename is the name of the file to write the model information to
   */
  void writeLikelihoodInformation(const char* filename) const;
  /**
   * \brief This function will write information about a single likelihood component to file
   * \param filename is the name of the file to write the model information to
   * \param id is the identifier of the likelihood component to write to file
   */
  void writeLikelihoodInformation(const char* filename, int id) const;
  /**
   * \brief This function will write summary information about the likelihood components to file
   * \param filename is the name of the file to write the model information to
   */
  void writeLikeSummaryInformation(const char* filename) const;
  /**
   * \brief This function will display information about the optimised values of the parameters
   */
  void writeOptValues() const;
  /**
   * \brief This function will initialise the Ecosystem parameters
   */
  void Initialise();
  /**
   * \brief This function will update the Ecosystem parameters with new values from StochasticData
   * \param Stochastic is the StochasticData containing the new values of the parameters
   */
  void Update(const StochasticData* const Stochastic) const;
  /**
   * \brief This function will update the Ecosystem parameters with new values
   * \param values is the DoubleVector containing the new values of the parameters
   */
  void Update(const DoubleVector& values) const;
  /**
   * \brief This function will reset the Ecosystem information
   */
  void Reset();
  /**
   * \brief This function will store the current value of the variables from the optimisation process
   * \param likvalue is the current likelihood value
   * \param point is the vector of the current parameter values
   */
  void StoreVariables(double likvalue, double* point) { keeper->StoreVariables(likvalue, point); };
  /**
   * \brief This function will scale the variables to be optimised (for the Hooke & Jeeves optimisation algorithm)
   */
  void ScaleVariables() { keeper->ScaleVariables(); };
  /**
   * \brief This function will return a copy the flags to denote which variables will be optimsised
   * \param opt is the IntVector that will contain a copy of the flags
   */
  void Opt(IntVector& opt) const;
  /**
   * \brief This function will return a copy the names of the variables
   * \param sw is the ParameterVector that will contain a copy of the names
   */
  void OptSwitches(ParameterVector& sw) const;
  /**
   * \brief This function will return a copy the initial value of the variables
   * \param val is the DoubleVector that will contain a copy of the initial values
   */
  void InitialValues(DoubleVector& val) const;
  /**
   * \brief This function will return a copy the current value of the variables
   * \param val is the DoubleVector that will contain a copy of the current values
   */
  void CurrentValues(DoubleVector& val) const;
  /**
   * \brief This function will return a copy the initial value of the variables to be optimised
   * \param val is the DoubleVector that will contain a copy of the initial values
   */
  void InitialOptValues(DoubleVector& val) const;
  /**
   * \brief This function will return a copy the scaled value of the variables to be optimised
   * \param val is the DoubleVector that will contain a copy of the scaled values
   */
  void ScaledOptValues(DoubleVector& val) const;
  /**
   * \brief This function will return a copy the lower bounds of the variables to be optimised
   * \param lbs is the DoubleVector that will contain a copy of the lower bounds
   */
  void LowerOptBds(DoubleVector& lbs) const;
  /**
   * \brief This function will return a copy the upper bounds of the variables to be optimised
   * \param ubs is the DoubleVector that will contain a copy of the upper bounds
   */
  void UpperOptBds(DoubleVector& ubs) const;
  /**
   * \brief This function will check that the values of the parameters are within the specified bounds
   */
  void checkBounds() const { keeper->checkBounds(Likely); };
  /**
   * \brief This function will return the number of variables to be optimised
   * \return number of variables to be optimised
   */
  int numOptVariables() const { return keeper->numOptVariables(); };
  void Simulate(int optimise, int print);
  void SimulateOneTimestep();
  void SimulateOneAreaOneTimeSubstep(int area);
  void updatePopulationOneArea(int area);
  void updateAgesOneArea(int area);
  double SimulateAndUpdate(double* x, int n);
  /**
   * \brief This function will return the likelihood score from the current simulation
   * \return likelihood score
   */
  double getLikelihood() const { return likelihood; };
  /**
   * \brief This function will return the total number of iterations completed
   * \return number of iterations
   */
  int getFuncEval() const { return funceval; };
  /**
   * \brief This function will return the flag used to determine whether the Simulated Annealing optimisation has converged or not
   * \return flag
   */
  int getConvergeSA() const { return convergeSA; };
  /**
   * \brief This function will set the flag used to denote whether the Simulated Annealing optimisation has converged or not
   * \param set is the value of the flag (0 or 1)
   */
  void setConvergeSA(int set) { convergeSA = set; };
  /**
   * \brief This function will return the number of iterations that took place during the Simulated Annealing optimisation
   * \return number of iterations
   */
  int getFuncEvalSA() const { return funcevalSA; };
  /**
   * \brief This function will set the number of iterations that took place during the Simulated Annealing optimisation
   * \param set is the number of iterations
   */
  void setFuncEvalSA(int set) { funcevalSA = set; };
  /**
   * \brief This function will return the best likelihood score from the Simulated Annealing optimisation
   * \return likelihood score
   */
  double getLikelihoodSA() const { return likelihoodSA; };
  /**
   * \brief This function will set the best likelihood score for the Simulated Annealing optimisation
   * \param set is the best likelihood score
   */
  void setLikelihoodSA(double set) { likelihoodSA = set; };
  /**
   * \brief This function will return the flag used to determine whether the Hooke & Jeeves optimisation has converged or not
   * \return flag
   */
  int getConvergeHJ() const { return convergeHJ; };
  /**
   * \brief This function will set the flag used to denote whether the Hooke & Jeeves optimisation has converged or not
   * \param set is the value of the flag (0 or 1)
   */
  void setConvergeHJ(int set) { convergeHJ = set; };
  /**
   * \brief This function will return the number of iterations that took place during the Hooke & Jeeves optimisation
   * \return number of iterations
   */
  int getFuncEvalHJ() const { return funcevalHJ; };
  /**
   * \brief This function will set the number of iterations that took place during the Hooke & Jeeves optimisation
   * \param set is the number of iterations
   */
  void setFuncEvalHJ(int set) { funcevalHJ = set; };
  /**
   * \brief This function will return the best likelihood score from the Hooke & Jeeves optimisation
   * \return likelihood score
   */
  double getLikelihoodHJ() const { return likelihoodHJ; };
  /**
   * \brief This function will set the best likelihood score for the Hooke & Jeeves optimisation
   * \param set is the best likelihood score
   */
  void setLikelihoodHJ(double set) { likelihoodHJ = set; };
  /**
   * \brief This function will return the flag used to determine whether the BFGS optimisation has converged or not
   * \return flag
   */
  int getConvergeBFGS() const { return convergeBFGS; };
  /**
   * \brief This function will set the flag used to denote whether the BFGS optimisation has converged or not
   * \param set is the value of the flag (0 or 1)
   */
  void setConvergeBFGS(int set) { convergeBFGS = set; };
  /**
   * \brief This function will return the number of iterations that took place during the BFGS optimisation
   * \return number of iterations
   */
  int getFuncEvalBFGS() const { return funcevalBFGS; };
  /**
   * \brief This function will set the number of iterations that took place during the BFGS optimisation
   * \param set is the number of iterations
   */
  void setFuncEvalBFGS(int set) { funcevalBFGS = set; };
  /**
   * \brief This function will return the best likelihood score from the BFGS optimisation
   * \return likelihood score
   */
  double getLikelihoodBFGS() const { return likelihoodBFGS; };
  /**
   * \brief This function will set the best likelihood score for the BFGS optimisation
   * \param set is the best likelihood score
   */
  void setLikelihoodBFGS(double set) { likelihoodBFGS = set; };
  /**
   * \brief This is the flag used to denote whether the user has interrupted the current model run
   */
  volatile int interrupted;
protected:
  /**
   * \brief This is the value of the likelihood score for the current simulation
   */
  double likelihood;
  /**
   * \brief This is the current iteration
   */
  int funceval;
  /**
   * \brief This is the flag used to denote whether the Simulated Annealing optimisation converged or not
   */
  int convergeSA;
  /**
   * \brief This is the number of iterations that took place during the Simulated Annealing optimisation
   */
  int funcevalSA;
  /**
   * \brief This is the value of the best likelihood score from the Simulated Annealing optimisation
   */
  double likelihoodSA;
  /**
   * \brief This is the flag used to denote whether the Hooke & Jeeves optimisation converged or not
   */
  int convergeHJ;
  /**
   * \brief This is the number of iterations that took place during the Hooke & Jeeves optimisation
   */
  int funcevalHJ;
  /**
   * \brief This is the value of the best likelihood score from the Hooke & Jeeves optimisation
   */
  double likelihoodHJ;
  /**
   * \brief This is the flag used to denote whether the BFGS optimisation converged or not
   */
  int convergeBFGS;
  /**
   * \brief This is the number of iterations that took place during the BFGS optimisation
   */
  int funcevalBFGS;
  /**
   * \brief This is the value of the best likelihood score from the BFGS optimisation
   */
  double likelihoodBFGS;
  /**
   * \brief This is the BaseClassPtrVector of the stocks, fleets and otherfood classes for the current model
   */
  BaseClassPtrVector basevec;
  /**
   * \brief This is the LikelihoodPtrVector of the likelihood components for the current model
   */
  LikelihoodPtrVector Likely;
  /**
   * \brief This is the PrinterPtrVector of the printer classes for the current model
   */
  PrinterPtrVector printvec;
  /**
   * \brief This is the TimeClass for the current model
   */
  TimeClass* TimeInfo;
  /**
   * \brief This is the AreaClass for the current model
   */
  AreaClass* Area;
  /**
   * \brief This is the Keeper for the current model
   */
  Keeper* keeper;
  /**
   * \brief This is the CharPtrVector of the names of the stocks for the current model
   */
  CharPtrVector stocknames;
  /**
   * \brief This is the StockPtrVector of the stocks for the current model
   */
  StockPtrVector stockvec;
  /**
   * \brief This is the CharPtrVector of the names of the tagging experiments for the current model
   */
  CharPtrVector tagnames;
  /**
   * \brief This is the TagPtrVector of the tagging experiments for the current model
   */
  TagPtrVector tagvec;
  /**
   * \brief This is the CharPtrVector of the names of the otherfood for the current model
   */
  CharPtrVector otherfoodnames;
  /**
   * \brief This is the OtherFoodPtrVector of the otherfood for the current model
   */
  OtherFoodPtrVector otherfoodvec;
  /**
   * \brief This is the CharPtrVector of the names of the fleets for the current model
   */
  CharPtrVector fleetnames;
  /**
   * \brief This is the FleetPtrVector of the fleets for the current model
   */
  FleetPtrVector fleetvec;
  /**
   * \brief This is the PrintInfo specifying the parameter output from the current model
   */
  PrintInfo printinfo;
  friend class InterruptInterface;
};

#endif
