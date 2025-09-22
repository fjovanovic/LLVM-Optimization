#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Attributes.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

static bool isDbgIntrinsic(const Instruction *I) {
    if (auto *II = dyn_cast<IntrinsicInst>(I)) {
        switch (II->getIntrinsicID()) {
            case Intrinsic::dbg_declare:
            case Intrinsic::dbg_value:
                return true;
            default: return false;
        }
    }
    return false;
}

static bool isTriviallyUnusedArg(Argument &Arg) {
    for (User *U : Arg.users()) {
        if (auto *SI = dyn_cast<StoreInst>(U)) {
            if (SI->getValueOperand() != &Arg) return false;
            Value *Ptr = SI->getPointerOperand();
            for (User *PU : Ptr->users())
                if (isa<LoadInst>(PU))
                    return false;
        } else if (auto *I = dyn_cast<Instruction>(U)) {
            if (isDbgIntrinsic(I)) continue;
            return false;
        } else {
            return false;
        }
    }
    return true;
}

static void pruneArtifacts(Argument &Arg) {
    SmallVector<Instruction*, 8> Dead;
    for (User *U : make_early_inc_range(Arg.users())) {
        if (auto *SI = dyn_cast<StoreInst>(U)) {
            if (SI->getValueOperand() != &Arg) continue;
            Value *Ptr = SI->getPointerOperand();
            bool HasLoad = false;
            for (User *PU : Ptr->users())
                if (isa<LoadInst>(PU)) HasLoad = true;
            if (!HasLoad) {
                Dead.push_back(SI);
                if (auto *AI = dyn_cast<AllocaInst>(Ptr)) {
                    bool OnlyDbgOrStore = true;
                    for (User *AU : AI->users()) {
                        if (AU == SI) continue;
                        if (auto *I = dyn_cast<Instruction>(AU)) {
                            if (isDbgIntrinsic(I)) continue;
                        }
                        OnlyDbgOrStore = false;
                    }
                    if (OnlyDbgOrStore) {
                        SmallVector<Instruction*,4> Dbg;
                        for (User *AU : AI->users())
                            if (auto *I = dyn_cast<Instruction>(AU))
                                if (isDbgIntrinsic(I)) Dbg.push_back(I);
                        for (Instruction *D : Dbg) D->eraseFromParent();
                        Dead.push_back(AI);
                    }
                }
            }
        } else if (auto *I = dyn_cast<Instruction>(U)) {
            if (isDbgIntrinsic(I))
                Dead.push_back(I);
        }
    }
    for (Instruction *I : Dead)
        I->eraseFromParent();
}

static bool isSafe(Function &F) {
    if (F.isDeclaration() || F.isVarArg() || F.getName() == "main")
        return false;
    for (User *U : F.users()) {
        if (auto *CI = dyn_cast<CallInst>(U)) {
            if (CI->getCalledFunction() == &F) continue;
            return false;
        } else if (auto *II = dyn_cast<InvokeInst>(U)) {
            if (II->getCalledFunction() == &F) continue;
            return false;
        } else {
            return false;
        }
    }
    return true;
}

struct RemoveUnusedFunctionArguments : public ModulePass {
    static char ID;
    RemoveUnusedFunctionArguments() : ModulePass(ID) {}

    bool runOnModule(Module &M) override {
        SmallVector<Function*,16> Targets;
        for (Function &F : M) {
            if (!isSafe(F)) continue;
            bool Any = false;
            for (Argument &A : F.args())
                if (isTriviallyUnusedArg(A)) { Any = true; break; }
            if (Any) Targets.push_back(&F);
        }
        bool Changed = false;
        for (Function *F : Targets)
            Changed |= shrinkSignature(*F);
        return Changed;
    }

    bool shrinkSignature(Function &F) {
        if (F.arg_empty())
            return false;

        std::vector<bool> RemoveMask(F.arg_size(), false);
        for (Argument &A : F.args()) {
            if (isTriviallyUnusedArg(A)) {
                pruneArtifacts(A);
                if (A.use_empty())
                    RemoveMask[A.getArgNo()] = true;
            }
        }
        unsigned ToRemove = 0;
        for (bool R : RemoveMask) if (R) ++ToRemove;
        if (!ToRemove) return false;

        for (Argument &A : F.args())
            if (RemoveMask[A.getArgNo()] && !A.use_empty())
                return false;

        SmallVector<Type*, 8> NewParamTypes;
        SmallVector<unsigned, 8> OldIndexForNew;
        for (unsigned i=0;i<RemoveMask.size();++i)
            if (!RemoveMask[i]) {
                NewParamTypes.push_back(F.getFunctionType()->getParamType(i));
                OldIndexForNew.push_back(i);
            }

        FunctionType *NewFTy =
            FunctionType::get(F.getReturnType(), NewParamTypes, false);

        Module *M = F.getParent();
        std::string OrigName = F.getName().str();
        Function *NewF =
            Function::Create(NewFTy, F.getLinkage(), OrigName + ".new_sig", M);

        {
            AttributeList OldAL = F.getAttributes();
            LLVMContext &Ctx = M->getContext();
            SmallVector<AttributeSet, 8> ParamSets;
            for (unsigned p=0; p<OldIndexForNew.size(); ++p)
                ParamSets.push_back(OldAL.getParamAttrs(OldIndexForNew[p]));
            AttributeList NewAL = AttributeList::get(
                Ctx, OldAL.getFnAttrs(), OldAL.getRetAttrs(), ParamSets);
            NewF->setAttributes(NewAL);
        }
        NewF->setCallingConv(F.getCallingConv());

        {
            unsigned newPos = 0;
            for (Argument &OldA : F.args()) {
                if (!RemoveMask[OldA.getArgNo()]) {
                    Argument &NewA = *std::next(NewF->arg_begin(), newPos);
                    NewA.setName(OldA.getName());
                    OldA.replaceAllUsesWith(&NewA);
                    ++newPos;
                }
            }
        }

        while (!F.empty()) {
            BasicBlock &BB = F.front();
            BB.removeFromParent();
            BB.insertInto(NewF);
        }

        SmallVector<Instruction*,16> InternalCalls;
        for (BasicBlock &BB : *NewF)
            for (Instruction &I : BB) {
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (CI->getCalledFunction() == &F)
                        InternalCalls.push_back(&I);
                } else if (auto *II = dyn_cast<InvokeInst>(&I)) {
                    if (II->getCalledFunction() == &F)
                        InternalCalls.push_back(&I);
                }
            }

        SmallVector<Instruction*,16> ExternalCalls;
        for (User *U : F.users())
            if (auto *I = dyn_cast<Instruction>(U))
                ExternalCalls.push_back(I);

        auto rebuild = [&](Instruction *I) {
            IRBuilder<> B(I);
            SmallVector<Value*,8> NewArgs;
            if (auto *CI = dyn_cast<CallInst>(I)) {
                for (unsigned oi : OldIndexForNew)
                    NewArgs.push_back(CI->getArgOperand(oi));
                CallInst *NC = B.CreateCall(NewF, NewArgs);
                NC->setCallingConv(CI->getCallingConv());
                if (!CI->use_empty())
                    CI->replaceAllUsesWith(NC);
                CI->eraseFromParent();
            } else if (auto *II = dyn_cast<InvokeInst>(I)) {
                for (unsigned oi : OldIndexForNew)
                    NewArgs.push_back(II->getArgOperand(oi));
                InvokeInst *NI = B.CreateInvoke(
                    NewF, II->getNormalDest(), II->getUnwindDest(), NewArgs);
                NI->setCallingConv(II->getCallingConv());
                if (!II->use_empty())
                    II->replaceAllUsesWith(NI);
                II->eraseFromParent();
            }
        };

        for (Instruction *I : InternalCalls)
            rebuild(I);
        for (Instruction *I : ExternalCalls)
            rebuild(I);

        F.eraseFromParent();
        NewF->setName(OrigName);
        return true;
    }
};

}

char RemoveUnusedFunctionArguments::ID = 0;
static RegisterPass<RemoveUnusedFunctionArguments> X(
    "remove-unused-args",
    "Remove Unused Function Arguments Pass"
);