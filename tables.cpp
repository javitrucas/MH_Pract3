#include <iostream>
#include <problem.h>
#include <random.hpp>
#include <string>
#include <util.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <cmath>
#include "pincrem.h"
#include "agg.h"
#include "age.h"
#include "am.h"
#include <chrono>
#include <vector>
#include <map>

using namespace std;
namespace fs = std::filesystem;

// Estadísticas simples
struct Stats { double mean, min, max; };

Stats calcStats(const vector<double>& v) {
    Stats s;
    s.min = *min_element(v.begin(), v.end());
    s.max = *max_element(v.begin(), v.end());
    s.mean = accumulate(v.begin(), v.end(), 0.0)/v.size();
    return s;
}

// Cargar mejores valores conocidos
map<string,double> loadBest(const string& path) {
    map<string,double> m;
    ifstream f(path);
    if(!f) return m;
    string hdr; getline(f,hdr);
    string inst; double val;
    while(f>>inst>>val) m[inst]=val;
    return m;
}

struct RunResults {
    vector<double> fitness, evals, times;
    vector<ResultMH> results;
};

void runAlg(const string& name, MH* alg, ProblemIncrem& prob, long seed, RunResults& R) {
    Random::seed(seed);
    auto t0=chrono::high_resolution_clock::now();
    ResultMH r=alg->optimize(&prob,100000);
    auto t1=chrono::high_resolution_clock::now();
    double dt=chrono::duration<double>(t1-t0).count();
    R.fitness.push_back(r.fitness);
    R.evals.push_back(r.evaluations);
    R.times.push_back(dt);
    R.results.push_back(r);
}

void writeStats(const string& alg, const string& inst, RunResults& R, map<string,double>& bestKnown) {
    fs::create_directories("../output");
    ofstream o("../output/"+alg+".txt", ios::app);
    Stats sf=calcStats(R.fitness), se=calcStats(R.evals), st=calcStats(R.times);
    bool hasBest = bestKnown.count(inst);
    double best= hasBest? bestKnown[inst]: sf.min;
    double desv = sf.mean>0?100*(sf.mean-best)/sf.mean:0;
    o<<"=== "<<inst<<" | "<<alg<<" ===\n"
     <<fixed<<setprecision(4)
     <<"Fit: mean="<<sf.mean<<" min="<<sf.min<<" max="<<sf.max
     << (hasBest?(" bestKnown="+to_string(best)):"")<<"\n"
     <<"Desv%: "<<desv<<"\n"
     <<"Evals: mean="<<se.mean<<" min="<<se.min<<" max="<<se.max<<"\n"
     <<"Time:  mean="<<st.mean<<" min="<<st.min<<" max="<<st.max<<"\n"
     <<"Runs detail:\n";
    for(int i=0;i<R.results.size();i++){
      o<<" Run"<<i+1<<": fit="<<R.fitness[i]
       <<" evals="<<R.evals[i]
       <<" time="<<R.times[i]<<"\n  Sol: [";
      R.results[i].printSolution(o);
      o<<"]\n";
    }
    o<<"\n";
}

int main(int argc, char** argv){
  // Semillas
  vector<long> seeds={42,123,567,1024,9999};
  if(argc>=2){
    long base=atol(argv[1]);
    seeds.clear();
    for(int i=0;i<5;i++) seeds.push_back(base+i*100);
  }
  // Mejores conocidos
  auto bestKnown=loadBest("../datos_MDD/best_fitness.txt");
  // Iterar instancias
  for(auto& e: fs::directory_iterator("../datos_MDD")){
    if(!e.is_regular_file()) continue;
    auto p=e.path();
    if(p.filename()=="best_fitness.txt") continue;
    if(p.extension()!=".txt") continue;
    string instPath=p.string(), instName=p.stem().string();
    cout<<"\nProcessing "<<instName<<"\n";
    ProblemIncrem prob; prob.leerArchivo(instPath);

    // AGG
    for(auto op: {AGGCrossover::UNIFORM,AGGCrossover::POSITION}){
      string tag="AGG-"+string(op==AGGCrossover::UNIFORM?"Uniform":"Position");
      RunResults R;
      for(auto s: seeds){
        AGG agg(50,0.7,0.1);
        agg.setCrossoverOperator(op);
        runAlg(tag, &agg, prob, s, R);
      }
      writeStats(tag, instName, R, bestKnown);
    }

    // AGE
    for(auto strat: {CrossoverStrategy::UNIFORM,CrossoverStrategy::POSITION}){
      string tag="AGE-"+string(strat==CrossoverStrategy::UNIFORM?"Uniform":"Position");
      RunResults R;
      for(auto s: seeds){
        AGE age(50,0.1);
        age.setCrossoverStrategy(strat);
        runAlg(tag, &age, prob, s, R);
      }
      writeStats(tag, instName, R, bestKnown);
    }

    // Encontrar mejor operador AGG para AM
    AGGCrossover bestOp=AGGCrossover::UNIFORM;
    { RunResults R;
      for(auto s: seeds){
        AGG a(50,0.7,0.1); a.setCrossoverOperator(AGGCrossover::UNIFORM);
        runAlg("tmp",&a,prob,s,R);
      }
      double u=calcStats(R.fitness).mean;
      R=RunResults();
      for(auto s: seeds){
        AGG a(50,0.7,0.1); a.setCrossoverOperator(AGGCrossover::POSITION);
        runAlg("tmp",&a,prob,s,R);
      }
      double p=calcStats(R.fitness).mean;
      bestOp = u<p?AGGCrossover::UNIFORM:AGGCrossover::POSITION;
    }

    // AM variantes con búsqueda local randLS (predeterminada)
    vector<pair<AMStrategy,string>> ams={
      {AMStrategy::All,"AM-(10,1.0)"},
      {AMStrategy::RandomSubset,"AM-(10,0.1)"},
      {AMStrategy::BestSubset,"AM-(10,0.1mej)"}
    };
    
    // Primero ejecutamos las variantes con randLS (predeterminada)
    for(auto &pr: ams){
      auto strat=pr.first; string tag=pr.second;
      RunResults R;
      for(auto s: seeds){
        AM am(50,0.7,0.1,0.1,strat);
        // si AM soporta constructor con operador:
        // AM am(50,0.7,0.1,0.1,strat,bestOp);
        runAlg(tag, &am, prob, s, R);
      }
      writeStats(tag, instName, R, bestKnown);
    }
    
    // Ahora ejecutamos las mismas variantes pero con heurLS
    for(auto &pr: ams){
      auto strat=pr.first; string tag=pr.second + "+heurLS";
      RunResults R;
      for(auto s: seeds){
        AM am(50,0.7,0.1,0.1,strat,SearchStrategy::heurLS);
        // si AM soporta constructor con operador:
        // AM am(50,0.7,0.1,0.1,strat,SearchStrategy::heurLS,bestOp);
        runAlg(tag, &am, prob, s, R);
      }
      writeStats(tag, instName, R, bestKnown);
    }
  }
  return 0;
}