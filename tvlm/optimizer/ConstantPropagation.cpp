#include "ConstantPropagation.h"
#include "tvlm/il/il.h"

#include "tvlm/tvlm/analysis/constantPropagation_analysis.h"

#include <cmath>



namespace tvlm{


    int64_t ConstantPropagation::resolveUnOperator(UnOp * un, int64_t op) {

        if (un->opType() == UnOpType::DEC) {
            return --op;
        } else if (un->opType() == UnOpType::INC) {
            return ++op;
        } else if (un->opType() == UnOpType::NOT) {
            return ~op;
        } else if (un->opType() == UnOpType::UNSUB) {
            return -op;
        } else {
            throw "[ConstantPropagation] unknown UnOpType (integer)";
        }
    }


    double ConstantPropagation::resolveUnOperator(UnOp * un, double op) {

        if (un->opType() == UnOpType::DEC) {
            return --op;
        } else if (un->opType() == UnOpType::INC) {
            return ++op;

        }else  if (un->opType() == UnOpType::UNSUB) {
            return -op;

        } else {
            throw "[ConstantPropagation] unknown UnOpType (integer)";
        }

    }

    int64_t ConstantPropagation::resolveBinOperator(BinOp * bin, int64_t lhs, int64_t rhs){

        auto binIns = dynamic_cast<Instruction::BinaryOperator *>(bin);
        if(binIns){
            if(binIns->opType() == BinOpType ::ADD){
                return lhs + rhs;
            }else if(binIns->opType() == BinOpType::SUB){
                return lhs - rhs;

            }else if(binIns->opType() == BinOpType::MUL){
                return lhs * rhs;

            }else if(binIns->opType() == BinOpType::DIV){
                return lhs / rhs;

            }else if(binIns->opType() == BinOpType::LSH){
                return lhs<< rhs;

            }else if(binIns->opType() == BinOpType::RSH){
                return lhs >> rhs;

            }else if(binIns->opType() == BinOpType::MOD){
                return lhs % rhs;

            }else if(binIns->opType() == BinOpType::AND){
                return lhs & rhs;

            }else if(binIns->opType() == BinOpType::OR){
                return lhs | rhs;

            }else if(binIns->opType() == BinOpType::XOR){
                return lhs ^ rhs;

            }else if(binIns->opType() == BinOpType::LT){
                return lhs < rhs;

            }else if(binIns->opType() == BinOpType::LTE){
                return lhs <= rhs;

            }else if(binIns->opType() == BinOpType::GT){
                return lhs > rhs;

            }else if(binIns->opType() == BinOpType::GTE){
                return lhs >= rhs;

            }else if(binIns->opType() == BinOpType::EQ){
                return lhs == rhs;

            }else if(binIns->opType() == BinOpType::NEQ){
                return lhs != rhs;

            }else {
                throw "[ConstantPropagation] unknown BinOpType";
            }
        }else{
            throw "[ConstantPropagation]Cannot bin optimize non-BionOp instruciton";

        }
    }
    double ConstantPropagation::resolveBinOperator(BinOp * bin, double lhs, double rhs){

        auto binIns = dynamic_cast<Instruction::BinaryOperator *>(bin);
        if(binIns){
            if(binIns->opType() == BinOpType ::ADD){
                return lhs + rhs;
            }else if(binIns->opType() == BinOpType::SUB){
                return lhs - rhs;

            }else if(binIns->opType() == BinOpType::MUL){
                return lhs * rhs;

            }else if(binIns->opType() == BinOpType::DIV){
                return lhs / rhs;

            }else if(binIns->opType() == BinOpType::LT){
                return lhs < rhs;

            }else if(binIns->opType() == BinOpType::LTE){
                return lhs <= rhs;

            }else if(binIns->opType() == BinOpType::GT){
                return lhs > rhs;

            }else if(binIns->opType() == BinOpType::GTE){
                return lhs >= rhs;

            }else if(binIns->opType() == BinOpType::EQ){
                return lhs == rhs;

            }else if(binIns->opType() == BinOpType::NEQ){
                return lhs != rhs;

            }else {
                throw "[ConstantPropagation] unknown BinOpType";
            }
        }else{
            throw "[ConstantPropagation]Cannot bin optimize non-BionOp instruciton";

        }
    }

    bool ConstantPropagation::isPowerOfTwo(int64_t num){
        return (num & (num - 1)) == 0; // value == 2^(whatever)
    }

    void ConstantPropagation::optimizeStrengthReduction(BasicBlock * bb) {
        auto insns = getBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size(); idx < len; idx++) {
            auto &first = insns[idx];
            auto binOp = dynamic_cast<BinOp *>(first);
            if (binOp && binOp->resultType() == ResultType::Integer) {
                if(binOp->opType() == BinOpType::MUL){
                    if (auto lhs = dynamic_cast<Instruction::ImmValue *>(binOp->lhs())) {
                        if (isPowerOfTwo(lhs->valueInt())) {
                            auto instrName = first->name();
                            auto lhsName = lhs->name();
                            int64_t newValue = (int64_t)log2(lhs->valueInt());

                            auto newInstr = bb->replaceInstr(lhs, new LoadImm(newValue, lhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::LSH, Instruction::Opcode::LSH, binOp->rhs(), newInstr, first->ast()));
                            first->setName(instrName);
                            lhs->setName(lhsName);

                        }
                    }else if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->lhs())){
                        if (isPowerOfTwo(rhs->valueInt())) {
                            auto instrName = first->name();
                            auto lhsName = rhs->name();
                            int64_t newValue = (int64_t)log2(rhs->valueInt());

                            auto newInstr = bb->replaceInstr(lhs, new LoadImm(newValue, rhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::LSH, Instruction::Opcode::LSH, binOp->lhs(), newInstr, first->ast()));
                            first->setName(instrName);
                            rhs->setName(lhsName);

                        }
                    }

                }
                else if(binOp->opType() == BinOpType::DIV){

                    if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->rhs())) {
                        if (isPowerOfTwo(rhs->valueInt())) {
                            auto instrName = first->name();
                            auto rhsName = rhs->name();
                            int64_t newValue = (int64_t)log2(rhs->valueInt());
                            auto newInstr = bb->replaceInstr(rhs, new LoadImm(newValue, rhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::RSH,Instruction::Opcode::RSH, binOp->lhs(), newInstr, first->ast()));

                            first->setName(instrName);
                            rhs->setName(rhsName);

                        }
                    }


                }
                else if(binOp->opType() == BinOpType::MOD){

                    if (auto rhs = dynamic_cast<Instruction::ImmValue *>(binOp->rhs())) {
                        if (isPowerOfTwo(rhs->valueInt())) {
                            auto instrName = first->name();
                            auto rhsName = rhs->name();
                            int64_t newValue = (int64_t)log2(rhs->valueInt());
                            auto newInstr = bb->replaceInstr(rhs, new LoadImm(newValue, rhs->ast()));
                            bb->replaceInstr(first, new BinOp( BinOpType::AND,Instruction::Opcode::AND, binOp->lhs(), rhs, first->ast()));

                            first->setName(instrName);
                            rhs->setName(rhsName);

                        }
                    }


                }



            }else if(binOp && binOp->resultType() == ResultType::Double){

            }

        }
    }


    void ConstantPropagation::optimizeBasicBlock(BasicBlock* bb) {
        optimizeConstantPropagation(bb);
        optimizeStrengthReduction(bb);
    }


    void ConstantPropagation::run(IL & il){
        auto constProp = new ConstantPropagationAnalysis<> (&il);


        for (int i = 0; i < 1000; i++) {
            for(auto & fnc : il.functions()){
                for( auto & bb : getpureFunctionBBs(fnc.second.get())){
                    optimizeBasicBlock(bb.get());
                }
            }
        }
    }

    void ConstantPropagation::optimizeConstantPropagation(BasicBlock * bb) {
        auto & insns = getpureBBsInstructions(bb);

        for (size_t idx = 0, len = insns.size();  idx < len; idx++) {
            auto & first = insns[idx];
            auto binOp = dynamic_cast<BinOp*>(first.get());
            if (binOp ){
                    if(binOp->lhs()->resultType() == ResultType::Double){
                        if(binOp->rhs()->resultType() == ResultType::Double){
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    double value = resolveBinOperator(binOp,lhs->valueFloat() , rhs->valueFloat()) ;
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }else{
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    double value = resolveBinOperator(binOp,lhs->valueFloat() , (double)rhs->valueInt());
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }
                    }else {
                        if(binOp->rhs()->resultType() == ResultType::Double){
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    double value = resolveBinOperator(binOp,(double)lhs->valueInt() , rhs->valueFloat());
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }else{
                            if(auto lhs = dynamic_cast<LoadImm*>(binOp->lhs())){
                                if(auto rhs = dynamic_cast<LoadImm*>(binOp->rhs())){
                                    int64_t value = resolveBinOperator(binOp,lhs->valueInt() , rhs->valueInt()) ;
                                    auto regName = first->name();
                                    bb->replaceInstr(first.get(), new LoadImm(value, binOp->ast()));
                                    first->setName(regName);
                                    bb->removeInstr(lhs);
                                    bb->removeInstr(rhs);
                                }
                            }
                        }
                    }
            }
            else if(auto un = dynamic_cast<UnOp*>(first.get())){
                if(auto op = dynamic_cast<LoadImm*>(un->operand())){
                    if(op->resultType() == ResultType::Double){
                        double value = resolveUnOperator(un,op->valueFloat() ) ;
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, un->ast()));
                        first->setName(regName);
                        bb->removeInstr(op);
                    }else{

                        int64_t value = resolveUnOperator(un,op->valueInt() ) ;
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, un->ast()));
                        first->setName(regName);
                        bb->removeInstr(op);
                    }
                }
            }
            else if(auto trunc = dynamic_cast<Truncate *>(first.get())){

                if(auto src = dynamic_cast<LoadImm*>(trunc->src())) {
                    if (src->resultType() == ResultType::Double) {

                        int64_t value = (int64_t)src->valueFloat();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, trunc->ast()));
                        first->setName(regName);

                        bb->removeInstr(src);
                    } else {

                        int64_t value = src->valueInt();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, trunc->ast()));
                        first->setName(regName);
                        bb->removeInstr(src);
                    }
                }
            }else if(auto extend = dynamic_cast<Extend *>(first.get())){

                if(auto src = dynamic_cast<LoadImm*>(extend->src())) {
                    if (src->resultType() == ResultType::Double) {

                        double value = src->valueFloat();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, trunc->ast()));
                        first->setName(regName);

                        bb->removeInstr(src);
                    } else {

                        double value = (double)src->valueInt();
                        auto regName = first->name();
                        bb->replaceInstr(first.get(), new LoadImm(value, trunc->ast()));
                        first->setName(regName);
                        bb->removeInstr(src);
                    }
                }
            }
        }

    }



}