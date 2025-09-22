#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


namespace {

struct FFTTransform : public FunctionPass {
    static char ID;
    FFTTransform() : FunctionPass(ID) {}


    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.addRequired<LoopInfoWrapperPass>();
        FunctionPass::getAnalysisUsage(AU);
    }

    
    bool checkArgTypes(Function &F, ArrayRef<Type*> expectedArgs, Type *expectedRet) {
        if(F.arg_size() != expectedArgs.size()) 
            return false;
        
        if(F.getReturnType() != expectedRet) 
            return false;
        
        auto it = F.arg_begin();
        for(size_t i=0; i<expectedArgs.size(); i++,++it) {
            if(it->getType() != expectedArgs[i]) 
                return false;
        }

        return true;
    }


    bool checkArraysNotModified(Function &F, Value *A, Value *B) {
        auto resolvePtrBase = [&](Value *V) -> Value* {
            while(true) {
                if(auto *BC = dyn_cast<BitCastInst>(V)) {
                    V = BC->getOperand(0);
                } else if(auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
                    V = GEP->getPointerOperand();
                } else if(auto *LI = dyn_cast<LoadInst>(V)) {
                    V = LI->getPointerOperand();
                } else {
                    break;
                }
            }

            return V;
        };

        for(auto &BB : F) {
            for(auto &I : BB) {
                if(auto *SI = dyn_cast<StoreInst>(&I)) {
                    Value *ptr = resolvePtrBase(SI->getPointerOperand());
                    if(ptr == A || ptr == B) {
                        return false;
                    }
                }
            }
        }

        return true;
    }


    bool checkCInitializedSafely(Function &F, Value *C) {
        auto resolvePtrBase = [&](Value *V) -> Value* {
            while (true) {
                if (auto *BC = dyn_cast<BitCastInst>(V)) {
                    V = BC->getOperand(0);
                } else if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
                    V = GEP->getPointerOperand();
                } else if (auto *LI = dyn_cast<LoadInst>(V)) {
                    V = LI->getPointerOperand();
                } else {
                    break;
                }
            }
            return V;
        };

        for(auto &BB : F) {
            for(auto &I : BB) {
                if(auto *SI = dyn_cast<StoreInst>(&I)) {
                    Value *ptr = resolvePtrBase(SI->getPointerOperand());
                    if(ptr == C) {
                        Value *val = SI->getValueOperand();

                        if(auto *CI = dyn_cast<ConstantInt>(val)) {
                            if(CI->isZero())
                                continue;
                        }

                        if(auto *Call = dyn_cast<CallBase>(val)) {
                            if(Function *calledF = Call->getCalledFunction()) {
                                if(calledF->getName() == "malloc") {
                                    continue;
                                }
                            }
                        }

                        return false;
                    }
                }
            }
        }

        return true;
    }


    bool isPolyMul(Function &F) {
        LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        std::unordered_map<const Value*, const Argument*> allocaToArg;
        if(!F.empty()) {
            BasicBlock &entry = F.getEntryBlock();
            for(Instruction &I : entry) {
                if(auto *S = dyn_cast<StoreInst>(&I)) {
                    Value *ptr = S->getPointerOperand();
                    Value *val = S->getValueOperand();
                    if(auto *AI = dyn_cast<AllocaInst>(ptr)) {
                        if(auto *Arg = dyn_cast<Argument>(val)) {
                            allocaToArg[AI] = Arg;
                        }
                    }
                }
            }
        }

        std::function<Value*(Value*)> resolvePtrBase = [&](Value *v) -> Value* {
            if(!v) {
                return nullptr;
            }

            if(auto *BC = dyn_cast<BitCastInst>(v)) {
                return resolvePtrBase(BC->getOperand(0));
            }

            if(auto *G = dyn_cast<GetElementPtrInst>(v)) {
                return resolvePtrBase(G->getPointerOperand());
            }

            if(auto *L = dyn_cast<LoadInst>(v)) {
                return resolvePtrBase(L->getPointerOperand());
            }

            return v;
        };

        std::function<const Argument*(Value*)> getUnderlyingArg = [&](Value *p) -> const Argument* {
            if(!p) {
                return nullptr;
            }

            if(auto *AI = dyn_cast<AllocaInst>(p)) {
                auto it = allocaToArg.find(AI);
                if(it != allocaToArg.end()) {
                    return it->second;
                }
            }

            if(auto *Arg = dyn_cast<Argument>(p)) {
                return Arg;
            }
            
            if(auto *L = dyn_cast<LoadInst>(p)) {
                return getUnderlyingArg(L->getPointerOperand());
            }

            if(auto *G = dyn_cast<GetElementPtrInst>(p)) {
                return getUnderlyingArg(G->getPointerOperand());
            }

            if(auto *BC = dyn_cast<BitCastInst>(p)) {
                return getUnderlyingArg(BC->getOperand(0));
            }

            return nullptr;
        };

        SmallVector<Loop*, 8> candidates;
        for(Loop *Top : LI) {
            SmallVector<Loop*, 8> q;
            q.push_back(Top);
            for(size_t qi = 0; qi < q.size(); ++qi) {
                Loop *L = q[qi];
                if(L->getLoopDepth() >= 2) {
                    candidates.push_back(L);
                }

                for(Loop *Sub : L->getSubLoops()) {
                    q.push_back(Sub);
                }
            }
        }

        if(candidates.empty()) {
            return false;
        }

        for(Loop *L : candidates) {
            if(L->getLoopDepth() != 2) {
                continue;
            }

            for(BasicBlock *BB : L->getBlocks()) {
                for(Instruction &I : *BB) {
                    auto *store = dyn_cast<StoreInst>(&I);
                    if(!store) {
                        continue;
                    }

                    auto *gepStore = dyn_cast<GetElementPtrInst>(store->getPointerOperand());
                    if(!gepStore) {
                        continue;
                    }

                    auto *add = dyn_cast<BinaryOperator>(store->getValueOperand());
                    if(!add || add->getOpcode() != Instruction::Add) {
                        continue;
                    }

                    LoadInst *loadC = dyn_cast<LoadInst>(add->getOperand(0));
                    BinaryOperator *mul = dyn_cast<BinaryOperator>(add->getOperand(1));
                    if(!(loadC && mul && mul->getOpcode() == Instruction::Mul)) {
                        loadC = dyn_cast<LoadInst>(add->getOperand(1));
                        mul = dyn_cast<BinaryOperator>(add->getOperand(0));
                        if(!(loadC && mul && mul->getOpcode() == Instruction::Mul)) {
                            continue;
                        }
                    }

                    auto *gepLoadC = dyn_cast<GetElementPtrInst>(loadC->getPointerOperand());
                    if(!gepLoadC) {
                        continue;
                    }

                    Value *storeBaseRaw = resolvePtrBase(gepStore->getPointerOperand());
                    Value *loadBaseRaw  = resolvePtrBase(gepLoadC->getPointerOperand());
                    const Argument *storeArgBase = getUnderlyingArg(storeBaseRaw);
                    const Argument *loadArgBase  = getUnderlyingArg(loadBaseRaw);

                    bool sameBase = false;
                    if(storeBaseRaw == loadBaseRaw) {
                        sameBase = true;
                    } else if(storeArgBase && loadArgBase && storeArgBase == loadArgBase) {
                        sameBase = true;
                    } else{
                        if(auto *SA = dyn_cast<AllocaInst>(storeBaseRaw)) {
                            if(auto *LA = dyn_cast<AllocaInst>(loadBaseRaw)) {
                                auto itS = allocaToArg.find(SA);
                                auto itL = allocaToArg.find(LA);
                                if(itS != allocaToArg.end() && itL != allocaToArg.end() && itS->second == itL->second) {
                                    sameBase = true;
                                }
                            }
                        }
                    }

                    if(!sameBase) {
                        continue;
                    }

                    LoadInst *loadA = dyn_cast<LoadInst>(mul->getOperand(0));
                    LoadInst *loadB = dyn_cast<LoadInst>(mul->getOperand(1));
                    if(!loadA || !loadB) {
                        continue;
                    }

                    auto *gepA = dyn_cast<GetElementPtrInst>(loadA->getPointerOperand());
                    auto *gepB = dyn_cast<GetElementPtrInst>(loadB->getPointerOperand());
                    if(!gepA || !gepB) {
                        continue;
                    }

                    Value *aBaseRaw = resolvePtrBase(gepA->getPointerOperand());
                    Value *bBaseRaw = resolvePtrBase(gepB->getPointerOperand());
                    const Argument *aArg = getUnderlyingArg(aBaseRaw);
                    const Argument *bArg = getUnderlyingArg(bBaseRaw);
                    if(aArg && bArg) {
                        if(storeArgBase && (aArg == storeArgBase || bArg == storeArgBase)) {
                            continue;
                        }

                        if(aArg == bArg) {
                            continue;
                        }
                    } else {
                        if(aBaseRaw == storeBaseRaw || bBaseRaw == storeBaseRaw) {
                            continue;
                        }
                    }

                    return true;
                }
            }
        }

        return false;
    }


    bool runOnFunction(Function &F) override {
        Module *M = F.getParent();
        bool modified = false;

        Type *IntTy = Type::getInt32Ty(F.getContext());
        Type *IntPtrTy = PointerType::get(IntTy, 0);
        Type *VoidTy = Type::getVoidTy(F.getContext());

        SmallVector<Type*, 5> void_fnc = { IntPtrTy, IntTy, IntPtrTy, IntTy, IntPtrTy };
        SmallVector<Type*, 4> ret_fnc  = { IntPtrTy, IntTy, IntPtrTy, IntTy };

        bool candidate = false;
        bool isVoid = false;

        if(checkArgTypes(F, void_fnc, VoidTy)) {
            candidate = true;
            isVoid = true;
        } else if(checkArgTypes(F, ret_fnc, IntPtrTy)) {
            candidate = true;
            isVoid = false;
        }

        if(!candidate) {
            return false;
        }

        auto argIt = F.arg_begin();
        Value *A = &*argIt++;
        Value *N = &*argIt++;
        Value *B = &*argIt++;
        Value *Mval = &*argIt++;
        Value *C = isVoid ? &*argIt++ : nullptr;

        if(!checkArraysNotModified(F, A, B)) {
            errs() << "[FFT PASS] Rejected: A or B are modified inside function.\n";
            return false;
        }

        if(C && !checkCInitializedSafely(F, C)) {
            errs() << "[FFT PASS] Rejected: C initialized with unsupported value.\n";
            return false;
        }

        bool foundPolyMul = isPolyMul(F);
        if(!foundPolyMul) {
            return false;
        }

        errs() << "[FFT PASS] Found polynomial multiplication.\n";

        while(!F.empty()) {
            BasicBlock &BB = F.back();
            BB.dropAllReferences();
            BB.eraseFromParent();
        }

        BasicBlock *entry = BasicBlock::Create(F.getContext(), "entry", &F);
        IRBuilder<> Builder(entry);

        if(isVoid) {
            FunctionType *FFTType = FunctionType::get(
                VoidTy, 
                { 
                    IntPtrTy, 
                    IntTy, 
                    IntPtrTy, 
                    IntTy, 
                    IntPtrTy
                }, 
                false
            );
            FunctionCallee FFTFunc = M->getOrInsertFunction("fft_polynomial_mul_void", FFTType);
            Builder.CreateCall(FFTFunc, { A, N, B, Mval, C });
            Builder.CreateRetVoid();
        } else {
            FunctionType *FFTType = FunctionType::get(
                IntPtrTy, 
                {
                    IntPtrTy, 
                    IntTy, 
                    IntPtrTy, 
                    IntTy
                }, 
                false
            );
            FunctionCallee FFTFunc = M->getOrInsertFunction("fft_polynomial_mul", FFTType);
            Value *Cptr = Builder.CreateCall(FFTFunc, { A, N, B, Mval });
            Builder.CreateRet(Cptr);
        }

        return true;
    }
};

char FFTTransform::ID = 0;
static RegisterPass<FFTTransform> X("fft-pass", "FFT Polynomial Multiplication Pass", false, false);

}