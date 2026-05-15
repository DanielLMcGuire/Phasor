#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>

#include <Value.hpp>
#include "../../../Codegen/CodeGen.hpp"
#include "../../../ISA/ISA.hpp"

namespace Phasor
{

using JitFn = void(*)();

class PhasorJIT
{
public:
    PhasorJIT();

    PhasorJIT(const PhasorJIT&)            = delete;
    PhasorJIT& operator=(const PhasorJIT&) = delete;

    JitFn compile(const Bytecode& bc,
                  size_t          startPC,
                  size_t          endPC,
                  Value*          registers,
                  Value*          variables,
                  const Value*    constants);

private:
    std::unique_ptr<llvm::orc::LLJIT> lljit;
    uint64_t regionCounter = 0;

    struct EC
    {
        llvm::IRBuilder<>& b;
        llvm::Module&      mod;
        llvm::LLVMContext& ctx;

        llvm::Type* voidTy;
        llvm::Type* i1Ty;
        llvm::Type* i64Ty;
        llvm::Type* f64Ty;
        llvm::Type* ptrTy;

        llvm::Value* regs;
        llvm::Value* vars;
        llvm::Value* consts;

        llvm::FunctionCallee fn_is_int;
        llvm::FunctionCallee fn_is_float;
        llvm::FunctionCallee fn_get_int;
        llvm::FunctionCallee fn_get_float;
        llvm::FunctionCallee fn_set_int;
        llvm::FunctionCallee fn_set_float;
        llvm::FunctionCallee fn_set_bool;
        llvm::FunctionCallee fn_copy;
    };

    void setupShims(EC& ec);
    void emitInstruction(EC& ec, const Instruction& instr);

    llvm::Value* regPtr  (EC& ec, uint8_t idx);
    llvm::Value* varPtr  (EC& ec, uint8_t idx);
    llvm::Value* constPtr(EC& ec, uint8_t idx);

    llvm::Value* extractInt  (EC& ec, llvm::Value* ptr);
    llvm::Value* extractFloat(EC& ec, llvm::Value* ptr);

    void storeInt  (EC& ec, llvm::Value* ptr, llvm::Value* v);
    void storeFloat(EC& ec, llvm::Value* ptr, llvm::Value* v);
    void storeBool (EC& ec, llvm::Value* ptr, llvm::Value* v);

    using BinOp   = std::function<llvm::Value*(llvm::Value*, llvm::Value*)>;
    using UnaryOp = std::function<llvm::Value*(llvm::Value*)>;

    void emitBinaryNumeric(EC& ec, llvm::Value* dst, llvm::Value* lhs, llvm::Value* rhs,
                           BinOp opInt, BinOp opFloat);

    void emitUnaryNumeric(EC& ec, llvm::Value* dst, llvm::Value* src,
                          UnaryOp opInt, UnaryOp opFloat);

    void emitBinaryCmp(EC& ec, llvm::Value* dst, llvm::Value* lhs, llvm::Value* rhs,
                       BinOp cmpInt, BinOp cmpFloat);
};

} // namespace Phasor