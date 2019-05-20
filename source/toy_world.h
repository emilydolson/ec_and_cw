#ifndef _TOY_WORLD_H
#define _TOY_WORLD_H

// #include "ResourceGradient.h"
#include "config/ArgManager.h"
#include "Evolve/World.h"


EMP_BUILD_CONFIG( ToyConfig,
  GROUP(DEFAULT, "Default settings for box experiment"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 100, "Number of organisms in the population."),
  VALUE(START_POP_SIZE, uint32_t, 1, "Number of organisms to initialize population with."),
  VALUE(UPDATES, uint32_t, 1000, "How many generations should we process?"),
  VALUE(SELECTION, std::string, "ROULETTE", "What selection scheme should we use?"),
  VALUE(RESOURCE_INFLOW, double, 100, "How much resource enters the world each update"),
  VALUE(MUT_RATE, double, .02, "Standard deviation of normal distribution mutations are seelcted from"),
  VALUE(PROBLEM_DIMENSIONS, int, 1, "How many axes does the fitness landscape have?"),
  VALUE(TOURNAMENT_SIZE, int, 5, "Tournament size"),
  VALUE(COST, double, 0, "Cost of doing task unsuccessfully"),
  VALUE(FRAC, double, .0025, "Percent of resource individual can use"),
  VALUE(SHARE_THRESHOLD, double, 10, "How similar do two solutions need to be to be in the same niche?"),  
  VALUE(MAX_RES_USE, double, 5, "Maximum quantity of resource that individual can use")
);

class ToyWorld : public emp::World<double> {

    double MUT_RATE;
    uint32_t START_POP_SIZE;
    uint32_t POP_SIZE;
    int TOURNAMENT_SIZE;
    uint32_t UPDATES;
    std::string SELECTION;
    
    public:
    ToyWorld(emp::Random & r) : emp::World<double>(r) {;}
    ToyWorld() {;}

    void InitConfigs(ToyConfig & config) {
        MUT_RATE = config.MUT_RATE();
        START_POP_SIZE = config.START_POP_SIZE();
        POP_SIZE = config.POP_SIZE();
        SELECTION = config.SELECTION();
        UPDATES = config.UPDATES();
        TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();
    }

    void Setup(ToyConfig & config) {
        InitConfigs(config);
        SetFitFun([](double & org){
            if (org < 0) {
                return org * org;
            }
            
            if (org > 0) {
                return sqrt(org);
            }

            return org; // zero
        });

        SetMutFun([this](double & org, emp::Random & random){
            org += random.GetRandNormal(0, MUT_RATE);
            if (org < -1) {
                org = -1;
            }

            if (org > 1) {
                org = 1;
            }
            return 1;
        });
        
        for (size_t i = 0; i < START_POP_SIZE; i++) {
            Inject(0);
        }

        SetPopStruct_Mixed(true);
        SetAutoMutate();
    }

    void RunStep() {
        if (SELECTION == "TOURNAMENT" || SELECTION == "SHARING") {
            TournamentSelect(*this, TOURNAMENT_SIZE, POP_SIZE);
        // } else if (SELECTION == "LEXICASE") {
        //     emp::LexicaseSelect(*this, fit_set, POP_SIZE);
        // } else if (SELECTION == "RESOURCE") {
        //     emp::ResourceSelect(*this, fit_set, resources, TOURNAMENT_SIZE, POP_SIZE-1, FRAC, MAX_RES_USE, RESOURCE_INFLOW, COST, true);
        //     for (emp::Ptr<ORG_TYPE> org : pop) {
        //         if (per_genotype_data[GetGenome(*org)].always_used.size() == 0) {
        //             std::set<size_t> niches;
        //             for (size_t i = 0; i < per_genotype_data[GetGenome(*org)].error_vec.size(); i++) {
        //                 if (per_genotype_data[GetGenome(*org)].error_vec[i] > 0) {
        //                     niches.insert(i);
        //                 }
        //             }
        //             per_genotype_data[GetGenome(*org)].always_used = niches;
        //         }
        //         niche_width.Add(per_genotype_data[GetGenome(*org)].always_used.size());
        //     }
        } else if (SELECTION == "ROULETTE") {
            RouletteSelect(*this, POP_SIZE);
        } else {
            std::cout << "ERROR: INVALID SELECTION SCHEME: " << SELECTION << std::endl;
            exit(1);
        }
        Update();
    }

    void Run() {
        for (size_t u = 0; u <= UPDATES; u++) {
            RunStep();
            for (size_t i = 0; i < POP_SIZE; i++) {
                std::cout << GetOrg(i) << " ";
            }
            std::cout << std::endl;
        }  
    }

};

#endif