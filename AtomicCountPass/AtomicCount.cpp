#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
  struct AtomicCount : public ModulePass {

    static char ID;

    AtomicCount() : ModulePass(ID) {}

    bool runOnModule(Module &M);
    bool runOnFunction(Function &F, Module &M);
    bool runOnBasicBlock(BasicBlock &BB, Module &M);
    
    bool initialize(Module &M);
  };
}

bool AtomicCount::runOnModule(Module &M)
{
  bool modified = initialize(M);

  for (auto it = M.begin(); it != M.end(); it++) {
    modified |= runOnFunction(*it, M);
  }

  // M.dump();

  return modified;
}

bool AtomicCount::runOnFunction(Function &F, Module &M)
{
  bool modified = false;

  for (auto it = F.begin(); it != F.end(); it++) {
    modified |= runOnBasicBlock(*it, M);
  }

  return modified;
}

bool AtomicCount::runOnBasicBlock(BasicBlock &bb, Module &M)
{
  // Get the global variable for atomic counter
  Type *I64Ty = Type::getInt64Ty(M.getContext());
  Constant *atomicCounter = M.getOrInsertGlobal("atomicCounter", I64Ty);
  assert(atomicCounter && "Could not declare or find atomicCounter global");
  /*
  if (!atomicCounter) {
    atomicCounter = new GlobalVariable(I64Ty, false, GlobalValue::ExternalLinkage, ConstantInt::get(I64Ty, 0), "atomicCounter", GlobalValue::NotThreadLocal, 0, true);
  }
  */

  int num_atomic_inst = 0;
  for (auto it = bb.begin(); it != bb.end(); it++) {
    if ((std::string)it->getOpcodeName() == "ret") {
      if (num_atomic_inst) {
	new AtomicRMWInst(AtomicRMWInst::Add,
			  atomicCounter,
			  ConstantInt::get(Type::getInt64Ty(bb.getContext()), num_atomic_inst),
			  AtomicOrdering::SequentiallyConsistent,
			  CrossThread, &*it);
      }
    }
    switch (it->getOpcode()) {
    case Instruction::AtomicCmpXchg:
    case Instruction::AtomicRMW:
      num_atomic_inst++;
    default:
      break;
    }
  }

  return true;
}

bool AtomicCount::initialize(Module &M)
{
  IRBuilder<> Builder(M.getContext());
  Function *mainFunc = M.getFunction("main");

  // not the main module
  if (!mainFunc)
    return false;

  Type *I64Ty = Type::getInt64Ty(M.getContext());
  // Create atomic counter global variable
  new GlobalVariable(M, I64Ty, false, GlobalValue::CommonLinkage, ConstantInt::get(I64Ty, 0), "atomicCounter");

  // Build printf function handle
  std::vector<Type *> FTyArgs;
  FTyArgs.push_back(Type::getInt8PtrTy(M.getContext()));
  FunctionType *FTy = FunctionType::get(Type::getInt32Ty(M.getContext()), FTyArgs, true);
  Function *printF = (Function *)M.getOrInsertFunction("printf", FTy);
  assert(printF != NULL);

  for (auto bb = mainFunc->begin(); bb != mainFunc->end(); bb++) {
    for(auto it = bb->begin(); it != bb->end(); it++) {
      // insert at the end of main function
      if ((std::string)it->getOpcodeName() == "ret") {
	Builder.SetInsertPoint(&*bb, it);
	// Build Arguments
	Value *format_long = Builder.CreateGlobalStringPtr("Atomic Counter: %ld\n", "formatLong");
	std::vector<Value *> argVec;
	argVec.push_back(format_long);

	Value *atomicCounter = M.getGlobalVariable("atomicCounter");
	Value* finalVal = new LoadInst(atomicCounter, atomicCounter->getName()+".val", &*it);
	argVec.push_back(finalVal);
	CallInst::Create(printF, argVec, "printf", &*it);
      }
    }
  }

  return true;
}

char AtomicCount::ID = 0;

/* Register as following to run through opt tool
static RegisterPass<AtomicCount>
    X("AtomicCount", "Count number of atomics executed", false, false);
*/

// Registration to run by default using clang compiler
static void registerMyPass(const PassManagerBuilder &, legacy::PassManagerBase &PM)
{
  PM.add(new AtomicCount());
}

static RegisterStandardPasses RegisterMyPass1(PassManagerBuilder::EP_ModuleOptimizerEarly, registerMyPass);
static RegisterStandardPasses RegisterMyPass2(PassManagerBuilder::EP_EnabledOnOptLevel0, registerMyPass);
