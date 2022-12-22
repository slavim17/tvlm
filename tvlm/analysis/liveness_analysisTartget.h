#pragma once

#include <map>
#include <queue>
#include "tvlm/tvlm/il/il_builder.h"
#include "tvlm/tvlm/il/il_insns.h"
#include "instruction_analysis.h"
#include "cfg.h"
#include "DeclarationAnalysis.h"

class CLiveRange {
    using IL = ::tvlm::IL;
public:
    CLiveRange(Instruction * il, IL * end):
            il_({il}),
            start_(end),
            end_(end){

    }

    void add(Instruction * in) {
        il_.emplace(in);
    }

    void setStart(IL *start){
        start_ = start;
    }
    void setEnd(IL *end){
        end_ = end;
    }

    const std::set<IL *>& il() const {
        return il_;
    }
    IL * start() const {
        return start_;
    }
    IL * end() const {
        return end_;
    }
private:
    std::set<IL *> il_;
    IL * start_;
    IL * end_;
};

//class CliveRangeComparator{
//public:
//    bool operator()(const CLiveRange * l, const CLiveRange * r)const{
//        return l->il() < r->il();
//    }
//    bool operator()(const Instruction * l, const CLiveRange * r)const{
//        return l < r->start();
//    }
//    bool operator()(const CLiveRange * l, const Instruction * r)const{
//        return l->start() < r;
//    }
//};

namespace tvlm{
    using Instruction = ::tvlm::Instruction;

    template<class T>
    using CLiveVars = MAP<const CfgNode<T> *, std::set<CLiveRange*>>;
    ;

    template<class Info>
    class ColoringLiveAnalysis : public BackwardAnalysis<CLiveVars<Info>, Info>{
    protected:
        Program * getProgram(TargetProgram * p)const {
            return TargetProgramFriend::getProgram(p);
        }
    private:
//        using Declaration = tvlm::Instruction;
        using NodeState = std::set<CLiveRange*>;
        //    using Declarations = MAP< ILInstruction *, Declaration*>;

        NodeState join(const CfgNode<Info> * node, CLiveVars<Info> & state);

        ColoringLiveAnalysis(ProgramCfg<Info> * cfg, const Declarations & declarations, TargetProgram * program);

        std::set<CLiveRange*> getSubtree(const CfgNode<Info> *pNode);

        NodeState transferFun(const CfgNode<Info> * node, const  NodeState & state );


        NodeState funOne(const CfgNode<Info> * node, CLiveVars<Info> & state);
    public:
//        static LivenessAnalysisTMP<Info> * create(Program * p);
        virtual ~ColoringLiveAnalysis(){
            delete cfg_;
        }
        explicit ColoringLiveAnalysis(TargetProgram * p);
        CLiveVars<Info> analyze() override;
        std::vector<CLiveRange*> & getLiveRanges(){
            return allocatedLR_;
        }

        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping(){
            return instr_mapping_;
        };
    private:
        std::vector<CLiveRange*> allocatedLR_;
        std::map<const CfgNode<Info>*, const Instruction *>instr_mapping_;
        std::map<const IL*, CLiveRange *> varMaps_;
        NodeState allVars_;
        CPowersetLattice<CLiveRange*> nodeLattice_;
        MapLattice<const CfgNode<Info> *, NodeState> lattice_;
        ProgramCfg<Info> * cfg_;
        TargetProgram * program_;

    };
//************************************************************************************************************

    template<class I>
    ColoringLiveAnalysis<I>::ColoringLiveAnalysis(TargetProgram *p):
            ColoringLiveAnalysis(BackwardAnalysis<CLiveVars<I>, I>::getCfg(getProgram(p)), InstructionAnalysis(getProgram(p)).analyze(), p)
    {
    }

    template<class I>
    std::set<CLiveRange*> ColoringLiveAnalysis<I>::getSubtree(const CfgNode<I> *node) {
        DeclarationAnalysis v(getProgram(program_));
        std::set<CLiveRange*> res;
        v.begin(node->il());
        auto children = v.result();
        for (const auto * ch : children) {
            auto lr = varMaps_.find(ch);
            if(lr != varMaps_.end()){
                   lr->second->setEnd(node->il());
                   res.emplace(lr->second);
            }
        }
        return res;
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::transferFun(const CfgNode<I> *node, const ColoringLiveAnalysis::NodeState &state){
        if(dynamic_cast<const CfgFunExitNode<I> *>(node)){
            return nodeLattice_.bot();
        }else if (dynamic_cast<const CfgGlobExitNode<I> *>(node)){

            return state;
        }else if(dynamic_cast<const CfgStmtNode<I> *>(node)){
            auto * stmtNode = dynamic_cast<const CfgStmtNode<I> *>(node);
            auto instr = dynamic_cast<ILInstruction *>(stmtNode->il());
            if(instr){ //TODO rules to add and remove from states


                if (dynamic_cast<AllocL *>(stmtNode->il())){
                    auto newState = state;
//                    std::set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
//                    return ;
                }else if (dynamic_cast<AllocG *>(stmtNode->il())){
                    auto newState = state;
//                    std::set<Declaration*> children = getSubtree(node);
//                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (auto store = dynamic_cast<Store *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
                }else if (dynamic_cast<Load *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
                }else if (dynamic_cast<LoadImm *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
                }else if (dynamic_cast<Return *>(stmtNode->il())){
                    auto ret  = dynamic_cast<Return *>(stmtNode->il());
                    auto newState = state;
                    auto res = varMaps_.find(ret->returnValue());
                    if(res != varMaps_.end()){
                        newState.emplace(res->second);
                    }
//                    auto res = varMaps_.find(node->il());
//                    if(res!=varMaps_.end()){
//                        newState.erase(newState.find(res->second));
//                    }
                    return newState;
                }else if (dynamic_cast<CondJump *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
//                    auto res = varMaps_.find(node->il());
//                    if(res!=varMaps_.end()){
//                        newState.erase(newState.find(res->second));
//                    }
                    return newState;
                }else if (dynamic_cast<Jump *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
//                    auto res = varMaps_.find(node->il());
//                    if(res!=varMaps_.end()){
//                        newState.erase(newState.find(res->second));
//                    }
                    return newState;
                }else if (dynamic_cast<PutChar *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;
                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<GetChar *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Copy *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Extend *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Truncate *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<BinOp *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<UnOp *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Return *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Phi *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<Call *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrIndex *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<ElemAddrOffset *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<StructAssign *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }else if (dynamic_cast<CallStatic *>(stmtNode->il())){
                    auto newState = state;
                    std::set<CLiveRange*> children = getSubtree(node);
                    newState.insert(children.begin(), children.end());
                    auto res = varMaps_.find(node->il());
                    if(res!=varMaps_.end()){
                        auto r = newState.find(res->second);
                        if(r != newState.end()) {
                            newState.erase(r);
                        }
                    }
                    return newState;

                    return state; // TODO create state transfer - liveness analysis
                }
            }


        } else{
        }
        return state;
    }




    template<class Info>
    CLiveVars<Info> ColoringLiveAnalysis<Info>::analyze() {
        CLiveVars<Info> X = lattice_.bot();
        std::set<const CfgNode<Info>*> W;
        for( auto & n : cfg_->nodes()){
            W.emplace(n);
        }

        while (!W.empty()) {
            const CfgNode<Info> * n = *W.begin();
            W.erase(W.begin());
            auto x = X.access(n);
            auto y = funOne(n, X);

            if (y != x) {
                X.update(std::make_pair(n, y));//X += n -> y
                //W ++= n.pred;
                W.insert( n->pred_.begin(), n->pred_.end());
            } else if (y.empty()) {
                X.update(std::make_pair(n, y));//X += n -> y;
            }
        }

        return std::move(X);
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::funOne(const CfgNode<I> * node, CLiveVars<I> & state)
    {
        return ColoringLiveAnalysis<I>::transferFun(node, join(node, state));
    }

    template<class I>
    typename ColoringLiveAnalysis<I>::NodeState
    ColoringLiveAnalysis<I>::join(const CfgNode<I> * node, CLiveVars<I> & state)
    {
        auto acc = nodeLattice_.bot();
        for (const CfgNode<I> * s : node->succ_) {
            auto pred = state.access(s);
            acc = nodeLattice_.lub(acc, pred);
        }
        return acc;
    }

    template<typename Info>
    ColoringLiveAnalysis<Info>::ColoringLiveAnalysis(ProgramCfg<Info> * cfg, const Declarations &declarations, TargetProgram * program):
            varMaps_()
            ,allVars_([&](){
                std::set< CLiveRange*> tmp;
                for ( auto & n : cfg->nodes()){
                    if(auto t = dynamic_cast<Instruction *>(n->il())){
                        auto lr = new CLiveRange(t, t);
                        allocatedLR_.push_back(lr);
                        tmp.emplace(lr);
                        varMaps_.emplace(n->il(), lr);
                    }
                }
                return tmp;
            }())
            ,
            nodeLattice_(CPowersetLattice<CLiveRange*>(allVars_)),
            lattice_(MapLattice<const CfgNode<Info>*, std::set<CLiveRange*>>(cfg->nodes(), &nodeLattice_)),
    cfg_(cfg),
    program_(program){

    }
} //namespace tvlm
