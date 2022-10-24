#pragma once

#include <stack>
#include "RegisterAllocator.h"
#include "t86/instruction.h"
#include "t86/t86_target"
#include "t86/program.h"
#include "t86/program/programbuilder.h"
#include "tvlm/codeGeneration/ProgramBuilder.h"

#include "tvlm/analysis/liveness_analysis.h"


/**
 * Preparation:
 *      Instruction Selection done - TInstruction known
 *          - Nontrivial
 *      Naively assigned Registers
 *      Liveness analysis done //TODO
 *          - Cfg construction //TODO
 *      ColorPicking
 *          -make graph:
 *              -verticies: live range
 *              -edges: overlapping ranges
 *          - each live range gets color (Register) by rules:
 *              1) K colors: node with degree k-1 safely colored
 *              2) coloring this node remove them from graph (might enable other nodes)
 *              3) no such colorable? -> "(spill)" : might enable condition 1)
 *              removing from graph == push to regStack -> determines color
 *
 *
 *
 * Work:
 *      Rework assigned registers by idea of graph coloring
 * */
namespace tvlm{

    class ColorInfo{
    public:
        int color;
    };



    class LiveRange {
    public:
        LiveRange(const Instruction * start){

        }

        void setEnd(const Instruction *){

        }
    private:

    };

    class ColoringAllocator : public RegisterAllocator{
    public:


        TargetProgram run()override{

        //----Preparation----
            //LivenessAnalysis
            Program * prog = getProgram();
            auto la = new LivenessAnalysis<ColorInfo>(prog);
            analysisResult_ = la->analyze(); //TODO check memory allocation

            //ColorPicking
            generateLiveRanges();


            //implement logic of passing through the program;
            return RegisterAllocator::run();
        }


//        void ReassignRegisters(ILBuilder & ilb /*or ProgramBuilderOLD and res of analysis*/){
//            auto prog = ilb.finish();
//            auto la = new LivenessAnalysis<ColorInfo>(&prog); // Integrate ILBuilder and ProgramBuilderOLD
//            analysisResult_ = la->analyze();
////            analysis_mapping_ = la->analysis_mapping()
//        }
//        void ReassignRegisters(Program * prog /*or ProgramBuilderOLD and res of analysis*/){
//            auto la = new LivenessAnalysis<ColorInfo>( prog); // Integrate ILBuilder and ProgramBuilderOLD
//            analysisResult_ = la->analyze();
//
//        }
//
//        Register getReg(const Instruction *ins) override;
//        Register getRegOutro(const Register & reg, const Instruction *ins);
//        void spillIntReg(const Instruction * ins);
//        Register pickForSpill(const Instruction * ins);
//
//        FRegister getFloatReg(const Instruction *ins) override;
//
//        void clearInt(const Instruction *ins) override;
//
//        void clearFloat(const Instruction *ins) override;
//
//        void spillCallReg() override;
//
//        void clearAllReg() override;
//
//        void spillAllReg() override;
//
//        void prepareReturnValue(size_t size, const Instruction * ret) override;
//
//        void makeLocalAllocation(size_t size, const Register &reg, const Instruction * ins) override;
//
//        void allocateStructArg(const Type *type, const Instruction *ins) override;
//
//        void resetAllocSize() override;
//
//        void correctStackAlloc(size_t patch) override;
//
//        bool isInsInRegister(const Instruction * ins) const;
//        bool isInsAtStack(const Instruction * ins) const;
//        bool isInsInMem(const Instruction * ins) const;

    protected:
//        Register getIntRegister(const Instruction *ins) override;
//
//        FRegister getFloatRegister(const Instruction *ins) override;
//
//        Register getFreeIntRegister() override;
//
//        FRegister getFreeFloatRegister() override;

        MAP<const CfgNode<ColorInfo> * , std::unordered_set<IL*>> analysisResult_;
        std::map<const CfgNode<ColorInfo> * , const Instruction *> analysis_mapping_;

        //incidence graph
        std::vector<LiveRange> liveRanges_;
        std::vector<std::set<int>> LRincidence_;

        std::map<int, int> spillIndexes_; //int -> index in liveRanges_ //both: 1st where to spill ; 2nd: what to spill
        std::stack<int> colorPickingStack_; //int -> index in liveRanges_
        std::map<const Instruction * , ColorInfo> colorPickingResult_;


        //create live ranges, and create incidence graph
        void generateLiveRanges();

        VirtualRegister getReg(const Instruction *currentIns) override;

        VirtualRegister getFReg(const Instruction *currentIns) override;

        VirtualRegister getLastRegister(const Instruction *currentIns) override;

        void releaseRegister(const VirtualRegister &reg) override;

    };
}
