#pragma once
#include "il.h"
#include "t86/program/label.h"
#include "t86/program/programbuilder.h"
#include "RegisterAllocator.h"

namespace tvlm{



    class NaiveIS  : public ILVisitor{
    public:
        using Label = tiny::t86::Label;
        using DataLabel = tiny::t86::DataLabel;
        using Register = tiny::t86::Register;
        using FRegister = tiny::t86::FloatRegister;


        static tiny::t86::Program translate(tvlm::Program & prog);

        void visit(Program *p) override;
    protected:
        NaiveIS(): pb_(tiny::t86::ProgramBuilder()), lastIns_(Label::empty()){

        }
        void visit(Instruction *ins) override;

    public:
        void visit(Jump *ins) override;
        void visit(CondJump *ins) override;
        void visit(Return *ins) override;
        void visit(CallStatic *ins) override;
        void visit(Call *ins) override;
        void visit(Copy *ins) override;
        void visit(Extend *ins) override;
        void visit(Truncate *ins) override;
        void visit(BinOp *ins) override;
        void visit(UnOp *ins) override;
        void visit(LoadImm *ins) override;
        void visit(AllocL *ins) override;
        void visit(AllocG *ins) override;
        void visit(ArgAddr *ins) override;
        void visit(PutChar *ins) override;
        void visit(GetChar *ins) override;
        void visit(Load *ins) override;
        void visit(Store *ins) override;
        void visit(Phi *ins) override;
        void visit(ElemAddrOffset *ins) override;
        void visit(ElemAddrIndex *ins) override;
        void visit(Halt *ins) override;

    protected:
        void visit(BasicBlock *bb) override;
        void visit(Function *fce) override;

    private:
        Label visitChild(IL * il) {
            ILVisitor::visitChild(il);
            return lastIns_;
        }

        template<typename T>
        Label visitChild(std::unique_ptr<T> const &ptr) {
            return visitChild(ptr.get());
        }

        template<typename T>
        Label add(Instruction * ins, const T& instruction) {
            lastIns_ = pb_.add( instruction);
            if(ins != nullptr ) compiled_.emplace(ins, lastIns_);
            return lastIns_;
        }

        template<typename T>
        Label add( const T& instruction) {
            lastIns_ = pb_.add( instruction);
            return lastIns_;
        }

        Label find(Instruction * ins){
            auto it = compiled_.find(ins);
            if(it == compiled_.end()){
                return Label::empty();
            }
            return it->second;
        }

        /**Call getIntRegister and occupy register
         * */
        auto fillIntRegister(Instruction * ins){
            return regAllocator->fillIntRegister(ins);
        }
        /**FindA free register
         * */
        auto getIntRegister(Instruction * ins){
            return regAllocator->getIntRegister(ins);
        }
        auto fillFloatRegister(Instruction * ins){
            return regAllocator->fillFloatRegister(ins);
        }
        auto getFloatRegister(Instruction * ins){
            return regAllocator->getFloatRegister(ins);
        }
        auto clearInt(Instruction * ins){
            return regAllocator->clearInt(ins);
        }

        auto clearAllIntReg(){
            return regAllocator->clearAllIntReg();
        }

        void clearAllFloatReg(){
            return regAllocator->clearAllFloatReg();
        }

        auto clearAllReg(){
            return regAllocator->clearAllReg();
        }

        auto spillAllReg(){
            return regAllocator->spillAllReg();
        }



        tiny::t86::ProgramBuilder pb_;
        Label lastIns_;
        std::unordered_map<tiny::Symbol, Label> functionTable_;
        std::unordered_map<Instruction*, uint64_t> globalTable_;
        std::unordered_map<Instruction *, Label> compiled_;
        std::vector<std::pair<Label, BasicBlock*>> future_patch_;
        std::vector<std::pair<Label, Symbol>> unpatchedCalls_;
        size_t functionLocalAllocSize = 0;
        size_t globalPointer_ = 0;


        std::unordered_map<const Instruction* ,Instruction * > instructionToEmplace;
        RegisterAllocator * regAllocator;



    };
}