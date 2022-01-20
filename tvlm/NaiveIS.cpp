#include "NaiveIS.h"
#include "t86/program/helpers.h"
#include "t86/instruction.h"

#define ZeroFlag 0x02
#define SignFlag 0x01
namespace tvlm{

    void NaiveIS::visit(Instruction *ins) {}
    
    void NaiveIS::visit(Halt *ins) {
        lastIns_ = add(tiny::t86::HALT());
    }
    
    void NaiveIS::visit(Jump *ins) {
        tiny::t86::Label jmp = add(tiny::t86::JMP( tiny::t86::Label::empty()));
        future_patch_.emplace_back(jmp, ins->getTarget(1));
//        return jmp;
        lastIns_ = jmp;
    }
    
    void NaiveIS::visit(CondJump *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::CMP(fillIntRegister(ins->condition()), 0));
        clearInt(ins->condition());
        tiny::t86::Label condJump = add(tiny::t86::JZ(tiny::t86::Label::empty()));
        tiny::t86::Label jumpToTrue = add(tiny::t86::JMP(tiny::t86::Label::empty()));
//        t86::Label jumpToFalse = add(t86::JMP(t86::Label::empty()));
//        patch(condJump, jumpToFalse);

        future_patch_.emplace_back(jumpToTrue, ins->getTarget(1));
        future_patch_.emplace_back(/*jumpToFalse*/condJump, ins->getTarget(0));

//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(Return *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::MOV(tiny::t86::Reg(0 /*or Somwhere on stack*/  ), fillIntRegister(ins->returnValue())));
        clearInt(ins->returnValue());
        add(tiny::t86::MOV(tiny::t86::Sp(), tiny::t86::Bp()));
        add(tiny::t86::POP(tiny::t86::Bp()));
        add(tiny::t86::RET());

        lastIns_= ret;
    }
    
    void NaiveIS::visit(CallStatic *ins) {

        auto ret = Label(lastIns_+1);

        //spill everything
        spillAllReg();
        //args
        for( auto it = ins->args().crbegin() ; it != ins->args().crend();it++){
            add(tiny::t86::PUSH(fillIntRegister(*it)));
            clearInt(*it);

        }
        clearAllIntReg();
        //call
        tiny::t86::Label callLabel = add(tiny::t86::CALL{tiny::t86::Label::empty()});
        add(tiny::t86::MOV(fillIntRegister(ins),tiny::t86::Reg(0)));
        add(tiny::t86::ADD(tiny::t86::Sp(), ins->args().size()));//clear arguments; need?

        unpatchedCalls_.emplace_back(callLabel, ins->f()->name());
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(Call *ins) {
        auto ret = Label(lastIns_+1);

        //spilll everything
        spillAllReg();

        //parameters
        for( auto it = ins->args().crbegin() ; it != ins->args().crend();it++){
            add(tiny::t86::PUSH(fillIntRegister(*it)));
            clearInt(*it);

        }

        clearAllReg();
        //call itself
        add(tiny::t86::CALL(fillIntRegister(ins->f())));
        clearInt(ins->f());

        add(tiny::t86::MOV(fillIntRegister(ins),tiny::t86::Reg(0)));
        add(tiny::t86::ADD(tiny::t86::Sp(), ins->args().size()));//clear arguments; need?

//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(BinOp *ins) {
        auto ret = Label(lastIns_+1);

        switch (ins->resultType()) {
            case ResultType::Integer:{

            auto lhsreg = fillIntRegister(ins->lhs());
            auto rhsreg = fillIntRegister(ins->rhs());
            switch (ins->opType()) {
                case BinOpType::ADD:
                    add(tiny::t86::ADD(lhsreg, rhsreg));
                    //        add(MOV(fillIntRegister(instr), fillIntRegister(instr->lhs())));
                    break;
                case BinOpType::SUB:
                    add(tiny::t86::SUB(lhsreg, rhsreg));
                    break;
                case BinOpType::MOD:
                    add(tiny::t86::MOD(lhsreg, rhsreg));
                    break;
                case BinOpType::MUL:
                    add(tiny::t86::MUL(lhsreg, rhsreg));
                    break;
                case BinOpType::DIV:
                    add(tiny::t86::DIV(lhsreg, rhsreg));
                    break;

                case BinOpType::AND:
                    add(tiny::t86::AND(lhsreg, rhsreg));
                    break;
                case BinOpType::OR:
                    add(tiny::t86::OR(lhsreg, rhsreg));
                    break;
                case BinOpType::XOR:
                    add(tiny::t86::XOR(lhsreg, rhsreg));
                    break;

                case BinOpType::LSH:
                    add(tiny::t86::LSH(lhsreg, rhsreg));
                    break;
                case BinOpType::RSH:
                    add(tiny::t86::RSH(lhsreg, rhsreg));
                    break;

                case BinOpType::NEQ:
                    add(tiny::t86::SUB(lhsreg,rhsreg));
                    break;
                case BinOpType::EQ:
                    add(tiny::t86::SUB(lhsreg,rhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, ZeroFlag));//ZeroFlag
                    break;
                case BinOpType::LTE:
                    add(tiny::t86::SUB(rhsreg, lhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    add(tiny::t86::NOT(lhsreg));
                    break;
                case BinOpType::LT:
                    add(tiny::t86::SUB(lhsreg, rhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    break;
                case BinOpType::GT:
                    add(tiny::t86::SUB(rhsreg, lhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    break;
                case BinOpType::GTE:
                    add(tiny::t86::SUB(lhsreg, rhsreg));
                    add(tiny::t86::MOV(lhsreg, tiny::t86::Flags()));
                    add(tiny::t86::AND(lhsreg, SignFlag));//SignFlag
                    add(tiny::t86::NOT(lhsreg));
                    break;
            };
                regAllocator->alloc_regs_[getIntRegister(ins->lhs()).index()] = ins;
                clearInt(ins->rhs());
                lastIns_ = ret;//        return ret;
                return;


                break;
            }
            case ResultType::Double:{

                throw "not implmented";
                break;
            }
            case ResultType::Void:
                throw "not implmented";
                break;
        }

        add(tiny::t86::SUB(fillIntRegister(ins->lhs()), fillIntRegister(ins->rhs())));

        add(tiny::t86::MOV(fillIntRegister(ins->rhs()), tiny::t86::Flags()));
        add(tiny::t86::AND(fillIntRegister(ins->rhs()),SignFlag));//SignFlag
        regAllocator->alloc_regs_[getIntRegister(ins->rhs()).index()] = ins;
        clearInt(ins->rhs());
        lastIns_ = ret;//        return ret;
    }
    
    void NaiveIS::visit(UnOp *ins) {
        auto ret = Label(lastIns_+1);
        auto reg =
        fillIntRegister(ins->operand());
        switch (ins->opType()) {
            case UnOpType::NOT:
                add(tiny::t86::NOT(reg));
                break;
            case UnOpType::UNSUB:
                add(tiny::t86::SUB(0 , reg));
                break;
            case UnOpType::INC:
                add(tiny::t86::INC( reg));
                break;
            case UnOpType::DEC:
                add(tiny::t86::DEC( reg));
                break;
        }
        regAllocator->alloc_regs_[getIntRegister(ins->operand()).index()] = ins;
//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(LoadImm *ins) {
        auto ret = lastIns_ +1;

        switch (ins->resultType()) {
            case ResultType::Integer:{

                int64_t value = ins->valueInt();
                if(!instructionToEmplace.empty() ){
                    auto it = instructionToEmplace.find(ins);
                    if( it!= instructionToEmplace.end()){
                        auto * ins = dynamic_cast<LoadImm * >(it->second);
                        value = ins->valueInt();
                        instructionToEmplace.erase(it);
                        delete ins;
                    }
                }
                add( tiny::t86::MOV(fillIntRegister(ins), value ));
                break;
            }
            case ResultType::Double:
                add(tiny::t86::MOV(getFloatRegister(ins), ins->valueFloat() ));
                break;
            case ResultType::Void:
                break;
        }

    }
    
    void NaiveIS::visit(ArgAddr *ins) {
        //TODO double args ? struct args?
        auto ret = Label(lastIns_+1);
        auto reg = fillIntRegister(ins);
        add(tiny::t86::MOV(reg, tiny::t86::Bp() ));
        add(tiny::t86::ADD(reg, (int64_t)ins->index()+2 ));

//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(AllocL *ins) {
        Label ret = lastIns_ +1;
        Register reg = fillIntRegister(ins);
        functionLocalAllocSize +=ins->size()/4;//TODO make size decomposition according target memory laout
        // already allocated, now just find addr for this allocation
                add(tiny::t86::MOV(reg ,tiny::t86::Bp()));
        add(tiny::t86::SUB(reg, (int64_t)functionLocalAllocSize));
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(AllocG *ins) {
        throw "allocG?? done in globals";
        DataLabel glob = pb_.addData((int64_t)(ins->size()/4));
        int64_t addr = (int64_t)glob;
        globalPointer_ += 1;
        Label ret = add( tiny::t86::MOV(fillIntRegister(ins) , addr));

        std::cerr << "label ret val: " << ret.address() << "| and addr val:  " << addr << std::endl;
//        return ret;
        lastIns_ = ret;
    }


    void NaiveIS::visit(Copy *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::MOV(fillIntRegister(ins), fillIntRegister(ins->src()) ));

//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(Extend *ins) {
//TODO
    }

    void NaiveIS::visit(Truncate *ins) {
//TODO
    }


    void NaiveIS::visit(PutChar *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::PUTCHAR(fillIntRegister(ins->src()) ));
        clearInt(ins->src());
        lastIns_ = ret;
    }

    void NaiveIS::visit(GetChar *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::GETCHAR( fillIntRegister(ins) ));
        lastIns_ = ret;
    }

    void NaiveIS::visit(Load *ins) {
        auto ret = Label(lastIns_+1);

        auto it = globalTable_.find(ins->address());
        if(it != globalTable_.end()){
            add(tiny::t86::MOV(fillIntRegister(ins), (int64_t)it->second));
            lastIns_ = ret; //return ret;
            return;
        }

        add(tiny::t86::MOV(fillIntRegister(ins), tiny::t86::Mem(fillIntRegister(ins->address()))));
        lastIns_ = ret; //return ret;
        return;
    }

    void NaiveIS::visit(Store *ins) {
        auto ret = Label(lastIns_+1);
        add(tiny::t86::MOV(Mem(fillIntRegister(ins->address())), fillIntRegister(ins->value())));
        clearInt(ins->value());
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(Phi *ins) {
        for(auto & content : ins->contents() ){
            auto f = std::find(regAllocator->alloc_regs_.begin(), regAllocator->alloc_regs_.end(), content.second);
            if(f != regAllocator->alloc_regs_.end()){
                *f = ins;
                break;
            }
        }
//        return Label(lastIns_ + 1);
        lastIns_ = lastIns_ + 1;
    }

    void NaiveIS::visit(ElemAddrOffset *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::ADD(fillIntRegister(ins->base()),
                fillIntRegister(ins->offset())));
        regAllocator->alloc_regs_[getIntRegister(ins->base()).index()] = ins;
        clearInt(ins->offset());
//        return ret;
        lastIns_ = ret;
    }

    void NaiveIS::visit(ElemAddrIndex *ins) {
        auto ret = Label(lastIns_+1);

        add(tiny::t86::ADD(fillIntRegister(ins->index()),
                           fillIntRegister(ins->offset())));
        clearInt(ins->offset());
        add(tiny::t86::ADD(fillIntRegister(ins->base()),
                           fillIntRegister(ins->index())));
        regAllocator->alloc_regs_[getIntRegister(ins->base()).index()] = ins;
        clearInt(ins->index());
//        return ret;
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(BasicBlock * bb) {
        Label ret = Label::empty();
        for (auto & i : getBBsInstructions(bb)) {
            Label tmp = visitChild(i);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(Function * fce) {
        Label ret = Label::empty();
    
        for (auto & bb : getFunctionBBs(fce)) {
            Label tmp = visitChild(bb);
            if(ret == Label::empty()){
                ret = tmp;
            }
        }
        lastIns_ = ret;
    }
    
    void NaiveIS::visit(Program * p) {
    
        Label globals = visitChild(getProgramsGlobals(p));
    
        Label callMain = add(nullptr, tiny::t86::CALL{Label::empty()});
        for ( auto & f : getProgramsFunctions(p)) {
            Label fncLabel = visitChild(f.second);
            functionTable_.emplace(f.first, fncLabel);
        }
    
        Label main = functionTable_.find(Symbol("main"))->second;
        pb_.patch(callMain, main);
        add(nullptr, tiny::t86::HALT{});
    
    }

    tiny::t86::Program NaiveIS::translate(Program &prog) {
        //TODO
        NaiveIS v;
        v.visit( &prog);
        tiny::t86::Program rawProg = v.pb_.program();
        std::vector<tiny::t86::Instruction*> instrs = rawProg.moveInstructions();
        int line = 0;
        for(const tiny::t86::Instruction * i : instrs){
            std::cerr << tiny::color::blue << line++ << ": " << tiny::color::green << i->toString() << std::endl;
        }


        return {instrs, rawProg.data()};
//            return v.pb_.program();
    }



}
