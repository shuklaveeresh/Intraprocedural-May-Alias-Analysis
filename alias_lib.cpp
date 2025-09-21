#include "llvm/ADT/SmallString.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

using PointsToMap = map<Value *, set<Value *>>;
namespace {
struct alias_c : public FunctionPass {
  static char ID;
  alias_c() : FunctionPass(ID) {}
  void initializingBasicBlockMaps(Function &F,
                                  map<BasicBlock *, PointsToMap> &dfinBB,
                                  map<BasicBlock *, PointsToMap> &dfoutBB) {
    for (BasicBlock &BB : F) {
      dfinBB[&BB] = PointsToMap();
      dfoutBB[&BB] = PointsToMap();
    }
  }

  void aliasingFormalParameters(Function &F,
                                map<BasicBlock *, PointsToMap> &dfinBB) {
    vector<AllocaInst *> paramAddrs;
    BasicBlock &entry = F.getEntryBlock();

    for (auto &inst : entry) {
      switch (inst.getOpcode()) {
      case Instruction::Alloca: {
        AllocaInst *allocaInst = dyn_cast<AllocaInst>(&inst);
        if (allocaInst && allocaInst->getAllocatedType()->isPointerTy()) {
          StringRef name = allocaInst->getNameOrAsOperand();
          if (name.endswith(".addr")) {
            paramAddrs.push_back(allocaInst);
          }
        }
        break;
      }
      default:
        break;
      }
    }
    BasicBlock *entryBlock = &entry;
    size_t i = 0;
    while (i < paramAddrs.size()) {
      size_t j = i + 1;
      while (j < paramAddrs.size()) {
        Value *param1 = paramAddrs[i];
        Value *param2 = paramAddrs[j];

        dfinBB[entryBlock][param1].insert(param2);
        dfinBB[entryBlock][param2].insert(param1);
        j++;
      }
      i++;
    }
  }

  map<Value *, set<Value *>> computeAliasMapping(const PointsToMap &pointsTo) {
    map<Value *, set<Value *>> aliasMap;

    for (const auto &outer : pointsTo) {
      for (const auto &inner : pointsTo) {
        if (outer.first == inner.first)
          continue;

        bool hasCommon = false;
        for (Value *val : outer.second) {
          if (inner.second.find(val) != inner.second.end()) {
            hasCommon = true;
            break;
          }
        }

        if (hasCommon) {
          aliasMap[outer.first].insert(inner.first);
        }
      }
    }

    return aliasMap;
  }

  PointsToMap mergePredecessorMaps(const PointsToMap &incoming,
                                   const PointsToMap &predOut) {
    PointsToMap result = incoming;
    for (const auto &pair : predOut) {
      Value *key = pair.first;
      const set<Value *> &aliases = pair.second;
      result[key].insert(aliases.begin(), aliases.end());
    }
    return result;
  }

  void expandingPointsToTransitively(PointsToMap &ptMap) {
    for (auto &entry : ptMap) {
      set<Value *> visitedNodes;
      function<set<Value *>(Value *)> recursiveExpand =
          [&](Value *node) -> set<Value *> {
        set<Value *> accumulated;
        if (visitedNodes.count(node) > 0)
          return accumulated;

        visitedNodes.insert(node);

        auto found = ptMap.find(node);
        if (found != ptMap.end()) {
          for (Value *neighbor : found->second) {
            accumulated.insert(neighbor);
            set<Value *> extraTargets = recursiveExpand(neighbor);
            accumulated.insert(extraTargets.begin(), extraTargets.end());
          }
        }
        return accumulated;
      };

      set<Value *> fullExpansion = recursiveExpand(entry.first);
      entry.second.insert(fullExpansion.begin(), fullExpansion.end());
    }
  }

  string stripAddrSuffix(const string &s) {
    StringRef strRef(s);
    if (strRef.endswith(".addr"))
      return strRef.drop_back(5).str();
    return s;
  }

  void displayAliasMapping(Function &F,
                           const map<Value *, set<Value *>> &aliasMap,
                           const set<Value *> &livePointers) {
    Module *mod = F.getParent();

    string sourcePath = mod->getSourceFileName();

    size_t delimPos = sourcePath.find_last_of("/\\");
    string destPath = (delimPos == string::npos)
                          ? "output.txt"
                          : sourcePath.substr(0, delimPos + 1) + "output.txt";

    ofstream out(destPath.c_str(), ios_base::app);
    if (!out.is_open()) {
      errs() << "Unable to open " << destPath << " for writing.\n";
      return;
    }

    out << "Function: " << F.getName().str() << "\n";

    BasicBlock &entryBB = F.getEntryBlock();
    for (Instruction &inst : entryBB) {
      if (auto *allocInst = dyn_cast<AllocaInst>(&inst)) {
        if (allocInst->getAllocatedType()->isPointerTy()) {
          Value *ptrVal = allocInst;
          if (livePointers.find(ptrVal) != livePointers.end()) {

            string baseName = stripAddrSuffix(ptrVal->getName().str());
            out << baseName << " -> {";

            auto aliasIt = aliasMap.find(ptrVal);
            vector<string> aliasNames;
            if (aliasIt != aliasMap.end()) {
              for (Value *aliasPtr : aliasIt->second) {
                if (livePointers.find(aliasPtr) != livePointers.end()) {
                  aliasNames.push_back(
                      stripAddrSuffix(aliasPtr->getName().str()));
                }
              }
            }

            for (size_t i = 0; i < aliasNames.size(); ++i) {
              out << aliasNames[i];
              if (i != aliasNames.size() - 1)
                out << ", ";
            }
            out << "}\n";
          }
        }
      }
    }

    out << "\n";
    out.close();
  }

 
  
  void processFinalAliasAnalysis(Function &F,
                                 map<BasicBlock *, PointsToMap> &dfoutBB,
                                 set<Value *> &allocatedPointers) {
    BasicBlock *exitBlock = nullptr;
    for (BasicBlock &BB : F) {
      if (isa<ReturnInst>(BB.getTerminator())) {
        exitBlock = &BB;
        break;
      }
    }

    if (!exitBlock) {
      errs() << "Error: No exit block found in function " << F.getName()
             << "\n";
      return;
    }

    PointsToMap finalMap = dfoutBB[exitBlock];
    expandingPointsToTransitively(finalMap);

    map<Value *, set<Value *>> aliasMapping = computeAliasMapping(finalMap);

    displayAliasMapping(F, aliasMapping, allocatedPointers);
  }

  PointsToMap flowFunction(Instruction &inst, PointsToMap &state,
                           set<Value *> &allocSet) {
    switch (inst.getOpcode()) {
    case Instruction::Alloca: {
      AllocaInst *allocInst = cast<AllocaInst>(&inst);
      if (allocInst->getAllocatedType()->isPointerTy()) {
        allocSet.insert(allocInst);
      }
      break;
    }
    case Instruction::Store: {
      StoreInst *storeInst = cast<StoreInst>(&inst);
      Value *srcVal = storeInst->getValueOperand();
      Value *dstVal = storeInst->getPointerOperand();
    
      if (!srcVal->getType()->isPointerTy() || !dstVal->getType()->isPointerTy())
        break;
    
      bool dstIsAllocated = (allocSet.find(dstVal) != allocSet.end());
      bool srcIsAllocated = (allocSet.find(srcVal) != allocSet.end());
    
      if (!dstIsAllocated) {
        if (state.count(dstVal)) {
          set<Value*> tempSet = state[dstVal];
          for (Value *alias : tempSet) {
            if (srcIsAllocated) {
              state[alias].insert(srcVal);
            } else {
              if (state.count(srcVal))
              {
                state[alias].insert(state[srcVal].begin(), state[srcVal].end());
              }
              else
                {
                  state[alias].clear();
                  state[alias].insert(srcVal);
                }

            }
          }
        }
      } else {
        state[dstVal].clear();
        if (!srcIsAllocated) {
          if (state.count(srcVal))
          {
            state[dstVal].insert(state[srcVal].begin(), state[srcVal].end());
          }
          else{
            state[dstVal].insert(srcVal);
          }
        } else {
          state[dstVal].insert(srcVal);
        }
      }
      break;
    }
    
    case Instruction::Load: {
      LoadInst *loadInst = cast<LoadInst>(&inst);
      Value *srcPtr = loadInst->getPointerOperand();
      if (srcPtr->getType()->isPointerTy()) {
        Value *result = &inst; 
        if (state.find(srcPtr) != state.end() &&
            allocSet.find(srcPtr) != allocSet.end()) {
          for (Value *alias : state[srcPtr]) {
            state[result].insert(alias);
          }
        } else {
          set<Value *> tempSet = state[srcPtr];
          for (Value *alias : tempSet) {
            state[result].insert(state[alias].begin(), state[alias].end());
          }
        }
      }
      break;
    }
    case Instruction::GetElementPtr: {
      GetElementPtrInst *gepInst = cast<GetElementPtrInst>(&inst);
      Value *base = gepInst->getPointerOperand();
      Value *result = &inst;
      state[result].insert(base);
      break;
    }

    case Instruction::BitCast: {

      BitCastInst *bcInst = cast<BitCastInst>(&inst);
      Value *source = bcInst->getOperand(0);
      Value *result = &inst;
      state[result] = state[source];
      break;
    }

    case Instruction::Call: {
      CallInst *callInst = cast<CallInst>(&inst);
      if (callInst->getType()->isPointerTy()) {
        Value *result = &inst;
        state[result].clear();
      }
      break;
    }
    
    default:
      break;
    }
    return state;
  }

  void dataFlowEquation(Function &F, map<BasicBlock *, PointsToMap> &dfinBB,
                        map<BasicBlock *, PointsToMap> &dfoutBB,
                        set<Value *> &allocatedPointers) {
    list<BasicBlock *> worklist;
    for (BasicBlock &BB : F) {
      worklist.push_back(&BB);
    }

    while (!worklist.empty()) {
      BasicBlock *currentBlock = worklist.front();
      worklist.pop_front();

      PointsToMap blockIn;
      bool firstPred = true;
      for (BasicBlock *pred : predecessors(currentBlock)) {
        if (dfoutBB.find(pred) != dfoutBB.end()) {
          PointsToMap predOut = dfoutBB[pred];
          if (firstPred) {
            blockIn = predOut;
            firstPred = false;
          } else {
            blockIn = mergePredecessorMaps(blockIn, predOut);
          }
        }
      }

      if (firstPred && currentBlock == &F.getEntryBlock()) {
        blockIn = PointsToMap();
      }

      dfinBB[currentBlock] = blockIn;

      PointsToMap currentState = blockIn;
      for (Instruction &I : *currentBlock) {
        currentState = flowFunction(I, currentState, allocatedPointers);
      }

      bool changed = (dfoutBB.find(currentBlock) == dfoutBB.end() ||
                      currentState != dfoutBB[currentBlock]);
      if (changed) {
        dfoutBB[currentBlock] = currentState;
        for (BasicBlock *succ : successors(currentBlock)) {
          if (find(worklist.begin(), worklist.end(), succ) == worklist.end()) {
            worklist.push_back(succ);
          }
        }
      }
    }
  }

  bool runOnFunction(Function &F) override {
    map<BasicBlock *, PointsToMap> dfinBB;
    map<BasicBlock *, PointsToMap> dfoutBB;
    set<Value *> allocatedPointers;
    initializingBasicBlockMaps(F, dfinBB, dfoutBB);
    aliasingFormalParameters(F, dfinBB);
    dataFlowEquation(F, dfinBB, dfoutBB, allocatedPointers);
    processFinalAliasAnalysis(F, dfoutBB, allocatedPointers);

    return false;
  }
};
} // namespace

char alias_c::ID = 0;
static RegisterPass<alias_c> X("alias_lib_given",
                               "Alias Analysis Pass for Assignment",
                               false /* Only looks at CFG */,
                               false /* Analysis Pass */);
