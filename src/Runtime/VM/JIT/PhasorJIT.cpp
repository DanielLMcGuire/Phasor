#include "PhasorJIT.hpp"
#include "shims.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <stdexcept>
#include <format>

namespace Phasor
{

PhasorJIT::PhasorJIT()
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto j = llvm::orc::LLJITBuilder().create();
    if (!j)
        throw std::runtime_error("PhasorJIT: " + llvm::toString(j.takeError()));
    lljit = std::move(*j);

    auto& es  = lljit->getExecutionSession();
    auto& jd  = lljit->getMainJITDylib();

    llvm::orc::SymbolMap syms;
    auto addSym = [&](const char* name, void* ptr) {
        auto sym = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr(reinterpret_cast<uint64_t>(ptr)),
            llvm::JITSymbolFlags::Exported);
        syms[es.intern(name)] = sym;
    };

    addSym("value_is_int",   reinterpret_cast<void*>(value_is_int));
    addSym("value_is_float", reinterpret_cast<void*>(value_is_float));
    addSym("value_is_bool",  reinterpret_cast<void*>(value_is_bool));
    addSym("value_get_int",  reinterpret_cast<void*>(value_get_int));
    addSym("value_get_float",reinterpret_cast<void*>(value_get_float));
    addSym("value_get_bool", reinterpret_cast<void*>(value_get_bool));
    addSym("value_set_int",  reinterpret_cast<void*>(value_set_int));
    addSym("value_set_float",reinterpret_cast<void*>(value_set_float));
    addSym("value_set_bool", reinterpret_cast<void*>(value_set_bool));
    addSym("value_copy",     reinterpret_cast<void*>(value_copy));

    if (auto err = jd.define(llvm::orc::absoluteSymbols(std::move(syms))))
        throw std::runtime_error("PhasorJIT: symbol registration failed: " +
                                 llvm::toString(std::move(err)));
}

JitFn PhasorJIT::compile(const Bytecode& bc,
                          size_t startPC, size_t endPC,
                          Value* registers, Value* variables, const Value* constants)
{
    std::string name = std::format("__phasor_{}", regionCounter++);

    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto mod = std::make_unique<llvm::Module>(name, *ctx);
    llvm::IRBuilder<> builder(*ctx);

    auto* fn = llvm::Function::Create(
        llvm::FunctionType::get(builder.getVoidTy(), false),
        llvm::Function::ExternalLinkage, name, mod.get());

    builder.SetInsertPoint(llvm::BasicBlock::Create(*ctx, "entry", fn));

    auto bakePtr = [&](const void* p) -> llvm::Value* {
        auto* asInt = llvm::ConstantInt::get(builder.getInt64Ty(),
                                             reinterpret_cast<uintptr_t>(p));
        return builder.CreateIntToPtr(asInt,
                                      llvm::PointerType::getUnqual(*ctx));
    };

    EC ec {
        .b      = builder,
        .mod    = *mod,
        .ctx    = *ctx,
        .voidTy = builder.getVoidTy(),
        .i1Ty   = builder.getInt1Ty(),
        .i64Ty  = builder.getInt64Ty(),
        .f64Ty  = builder.getDoubleTy(),
        .ptrTy  = llvm::PointerType::getUnqual(*ctx),
        .regs   = bakePtr(registers),
        .vars   = bakePtr(variables),
        .consts = bakePtr(constants),
    };

    setupShims(ec);

    for (size_t i = startPC; i < endPC; ++i)
        emitInstruction(ec, bc.instructions[i]);

    builder.CreateRetVoid();

    std::string errStr;
    llvm::raw_string_ostream es2(errStr);
    if (llvm::verifyFunction(*fn, &es2))
        throw std::runtime_error("PhasorJIT IR verification failed: " + errStr);

    if (auto err = lljit->addIRModule(
            llvm::orc::ThreadSafeModule(std::move(mod), std::move(ctx))))
        throw std::runtime_error("PhasorJIT addIRModule: " +
                                 llvm::toString(std::move(err)));

    auto sym = lljit->lookup(name);
    if (!sym)
        throw std::runtime_error("PhasorJIT lookup: " +
                                 llvm::toString(sym.takeError()));

    return reinterpret_cast<JitFn>(sym->getValue());
}

void PhasorJIT::setupShims(EC& ec)
{
    auto& m = ec.mod;
    auto  p = ec.ptrTy;
    auto  i1  = ec.i1Ty;
    auto  i64 = ec.i64Ty;
    auto  f64 = ec.f64Ty;
    auto  v   = ec.voidTy;

    ec.fn_is_int = m.getOrInsertFunction("value_is_int",
        llvm::FunctionType::get(i1, {p}, false));

    ec.fn_is_float = m.getOrInsertFunction("value_is_float",
        llvm::FunctionType::get(i1, {p}, false));

    ec.fn_get_int = m.getOrInsertFunction("value_get_int",
        llvm::FunctionType::get(i64, {p}, false));

    ec.fn_get_float = m.getOrInsertFunction("value_get_float",
        llvm::FunctionType::get(f64, {p}, false));

    ec.fn_set_int = m.getOrInsertFunction("value_set_int",
        llvm::FunctionType::get(v, {p, i64}, false));

    ec.fn_set_float = m.getOrInsertFunction("value_set_float",
        llvm::FunctionType::get(v, {p, f64}, false));

    ec.fn_set_bool = m.getOrInsertFunction("value_set_bool",
        llvm::FunctionType::get(v, {p, i1}, false));

    ec.fn_copy = m.getOrInsertFunction("value_copy",
        llvm::FunctionType::get(v, {p, p}, false));
}

llvm::Value* PhasorJIT::regPtr(EC& ec, uint8_t idx)
{
    auto* base   = ec.regs;
    auto* offset = llvm::ConstantInt::get(ec.i64Ty, idx * sizeof(Value));
    return ec.b.CreateGEP(ec.b.getInt8Ty(), base, offset);
}

llvm::Value* PhasorJIT::varPtr(EC& ec, uint8_t idx)
{
    auto* offset = llvm::ConstantInt::get(ec.i64Ty, idx * sizeof(Value));
    return ec.b.CreateGEP(ec.b.getInt8Ty(), ec.vars, offset);
}

llvm::Value* PhasorJIT::constPtr(EC& ec, uint8_t idx)
{
    auto* offset = llvm::ConstantInt::get(ec.i64Ty, idx * sizeof(Value));
    return ec.b.CreateGEP(ec.b.getInt8Ty(), ec.consts, offset);
}

llvm::Value* PhasorJIT::extractInt(EC& ec, llvm::Value* ptr)
{
    return ec.b.CreateCall(ec.fn_get_int, {ptr});
}

llvm::Value* PhasorJIT::extractFloat(EC& ec, llvm::Value* ptr)
{
    return ec.b.CreateCall(ec.fn_get_float, {ptr});
}

void PhasorJIT::storeInt(EC& ec, llvm::Value* ptr, llvm::Value* v)
{
    ec.b.CreateCall(ec.fn_set_int, {ptr, v});
}

void PhasorJIT::storeFloat(EC& ec, llvm::Value* ptr, llvm::Value* v)
{
    ec.b.CreateCall(ec.fn_set_float, {ptr, v});
}

void PhasorJIT::storeBool(EC& ec, llvm::Value* ptr, llvm::Value* v)
{
    ec.b.CreateCall(ec.fn_set_bool, {ptr, v});
}

void PhasorJIT::emitBinaryNumeric(EC& ec,
                                   llvm::Value* dst,
                                   llvm::Value* lhs,
                                   llvm::Value* rhs,
                                   BinOp opInt,
                                   BinOp opFloat)
{
    auto& b = ec.b;
    auto* fn = b.GetInsertBlock()->getParent();

    auto* intBB   = llvm::BasicBlock::Create(ec.ctx, "int_op",   fn);
    auto* floatBB = llvm::BasicBlock::Create(ec.ctx, "float_op", fn);
    auto* mergeBB = llvm::BasicBlock::Create(ec.ctx, "merge",    fn);

    auto* isInt = b.CreateCall(ec.fn_is_int, {lhs});
    b.CreateCondBr(isInt, intBB, floatBB);

    b.SetInsertPoint(intBB);
    auto* lhsI = extractInt(ec, lhs);
    auto* rhsI = extractInt(ec, rhs);
    auto* resI = opInt(lhsI, rhsI);
    storeInt(ec, dst, resI);
    b.CreateBr(mergeBB);

    b.SetInsertPoint(floatBB);
    auto* lhsF = extractFloat(ec, lhs);
    auto* rhsF = extractFloat(ec, rhs);
    auto* resF = opFloat(lhsF, rhsF);
    storeFloat(ec, dst, resF);
    b.CreateBr(mergeBB);

    b.SetInsertPoint(mergeBB);
}

void PhasorJIT::emitUnaryNumeric(EC& ec,
                                  llvm::Value* dst,
                                  llvm::Value* src,
                                  UnaryOp opInt,
                                  UnaryOp opFloat)
{
    auto& b = ec.b;
    auto* fn = b.GetInsertBlock()->getParent();

    auto* intBB   = llvm::BasicBlock::Create(ec.ctx, "int_op",   fn);
    auto* floatBB = llvm::BasicBlock::Create(ec.ctx, "float_op", fn);
    auto* mergeBB = llvm::BasicBlock::Create(ec.ctx, "merge",    fn);

    auto* isInt = b.CreateCall(ec.fn_is_int, {src});
    b.CreateCondBr(isInt, intBB, floatBB);

    b.SetInsertPoint(intBB);
    storeInt(ec, dst, opInt(extractInt(ec, src)));
    b.CreateBr(mergeBB);

    b.SetInsertPoint(floatBB);
    storeFloat(ec, dst, opFloat(extractFloat(ec, src)));
    b.CreateBr(mergeBB);

    b.SetInsertPoint(mergeBB);
}

void PhasorJIT::emitBinaryCmp(EC& ec,
                               llvm::Value* dst,
                               llvm::Value* lhs,
                               llvm::Value* rhs,
                               BinOp cmpInt,
                               BinOp cmpFloat)
{
    auto& b = ec.b;
    auto* fn = b.GetInsertBlock()->getParent();

    auto* intBB   = llvm::BasicBlock::Create(ec.ctx, "cmp_int",   fn);
    auto* floatBB = llvm::BasicBlock::Create(ec.ctx, "cmp_float", fn);
    auto* mergeBB = llvm::BasicBlock::Create(ec.ctx, "merge",     fn);

    auto* isInt = b.CreateCall(ec.fn_is_int, {lhs});
    b.CreateCondBr(isInt, intBB, floatBB);

    b.SetInsertPoint(intBB);
    storeBool(ec, dst, cmpInt(extractInt(ec, lhs), extractInt(ec, rhs)));
    b.CreateBr(mergeBB);

    b.SetInsertPoint(floatBB);
    storeBool(ec, dst, cmpFloat(extractFloat(ec, lhs), extractFloat(ec, rhs)));
    b.CreateBr(mergeBB);

    b.SetInsertPoint(mergeBB);
}

void PhasorJIT::emitInstruction(EC& ec, const Instruction& instr)
{
    auto& b = ec.b;
    uint8_t rA = instr.operand1;
    uint8_t rB = instr.operand2;
    uint8_t rC = instr.operand3;

    auto A = [&]{ return regPtr(ec, rA); };
    auto B = [&]{ return regPtr(ec, rB); };
    auto C = [&]{ return regPtr(ec, rC); };

    switch (instr.op)
    {
    case OpCode::MOV:
        b.CreateCall(ec.fn_copy, {A(), B()});
        break;

    case OpCode::LOAD_CONST_R:
        b.CreateCall(ec.fn_copy, {A(), constPtr(ec, rB)});
        break;

    case OpCode::LOAD_VAR_R:
        b.CreateCall(ec.fn_copy, {A(), varPtr(ec, rB)});
        break;

    case OpCode::STORE_VAR_R:
        b.CreateCall(ec.fn_copy, {varPtr(ec, rA), B()});
        break;

    case OpCode::IADD_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateAdd(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFAdd(a, b_); });
        break;

    case OpCode::ISUB_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateSub(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFSub(a, b_); });
        break;

    case OpCode::IMUL_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateMul(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFMul(a, b_); });
        break;

    case OpCode::IDIV_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateSDiv(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFDiv(a, b_); });
        break;

    case OpCode::IMOD_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateSRem(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFRem(a, b_); });
        break;

    case OpCode::FLADD_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateAdd(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFAdd(a, b_); });
        break;

    case OpCode::FLSUB_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateSub(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFSub(a, b_); });
        break;

    case OpCode::FLMUL_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateMul(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFMul(a, b_); });
        break;

    case OpCode::FLDIV_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateSDiv(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFDiv(a, b_); });
        break;

    case OpCode::FLMOD_R:
        emitBinaryNumeric(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateSRem(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFRem(a, b_); });
        break;

    case OpCode::SQRT_R: {
        auto* src  = extractFloat(ec, B());
        auto* decl = llvm::Intrinsic::getDeclaration(&ec.mod, llvm::Intrinsic::sqrt,
                                                      {ec.f64Ty});
        storeFloat(ec, A(), b.CreateCall(decl, {src}));
        break;
    }
    case OpCode::LOG_R: {
        auto* src  = extractFloat(ec, B());
        auto* decl = llvm::Intrinsic::getDeclaration(&ec.mod, llvm::Intrinsic::log,
                                                      {ec.f64Ty});
        storeFloat(ec, A(), b.CreateCall(decl, {src}));
        break;
    }
    case OpCode::EXP_R: {
        auto* src  = extractFloat(ec, B());
        auto* decl = llvm::Intrinsic::getDeclaration(&ec.mod, llvm::Intrinsic::exp,
                                                      {ec.f64Ty});
        storeFloat(ec, A(), b.CreateCall(decl, {src}));
        break;
    }
    case OpCode::SIN_R: {
        auto* src  = extractFloat(ec, B());
        auto* decl = llvm::Intrinsic::getDeclaration(&ec.mod, llvm::Intrinsic::sin,
                                                      {ec.f64Ty});
        storeFloat(ec, A(), b.CreateCall(decl, {src}));
        break;
    }
    case OpCode::COS_R: {
        auto* src  = extractFloat(ec, B());
        auto* decl = llvm::Intrinsic::getDeclaration(&ec.mod, llvm::Intrinsic::cos,
                                                      {ec.f64Ty});
        storeFloat(ec, A(), b.CreateCall(decl, {src}));
        break;
    }
    case OpCode::TAN_R: {
        auto* src    = extractFloat(ec, B());
        auto  tanFn  = ec.mod.getOrInsertFunction("tan",
            llvm::FunctionType::get(ec.f64Ty, {ec.f64Ty}, false));
        storeFloat(ec, A(), b.CreateCall(tanFn, {src}));
        break;
    }
    case OpCode::POW_R: {
        auto* srcA = extractFloat(ec, B());
        auto* srcB = extractFloat(ec, C());
        auto* decl = llvm::Intrinsic::getDeclaration(&ec.mod, llvm::Intrinsic::pow,
                                                      {ec.f64Ty});
        storeFloat(ec, A(), b.CreateCall(decl, {srcA, srcB}));
        break;
    }

    case OpCode::NEG_R:
        emitUnaryNumeric(ec, A(), B(),
            [&](auto* v){ return b.CreateNeg(v); },
            [&](auto* v){ return b.CreateFNeg(v); });
        break;

    case OpCode::NOT_R: {
        auto* fn2 = b.GetInsertBlock()->getParent();
        auto* intBB   = llvm::BasicBlock::Create(ec.ctx, "not_int",   fn2);
        auto* floatBB = llvm::BasicBlock::Create(ec.ctx, "not_float", fn2);
        auto* mergeBB = llvm::BasicBlock::Create(ec.ctx, "merge",     fn2);

        b.CreateCondBr(b.CreateCall(ec.fn_is_int, {B()}), intBB, floatBB);

        b.SetInsertPoint(intBB);
        storeBool(ec, A(), b.CreateICmpEQ(extractInt(ec, B()),
                                          llvm::ConstantInt::get(ec.i64Ty, 0)));
        b.CreateBr(mergeBB);

        b.SetInsertPoint(floatBB);
        storeBool(ec, A(), b.CreateFCmpOEQ(extractFloat(ec, B()),
                                            llvm::ConstantFP::get(ec.f64Ty, 0.0)));
        b.CreateBr(mergeBB);

        b.SetInsertPoint(mergeBB);
        break;
    }

    case OpCode::IAND_R:
    case OpCode::FLAND_R: {
        auto* lhsI = b.CreateICmpNE(extractInt(ec, B()), llvm::ConstantInt::get(ec.i64Ty, 0));
        auto* rhsI = b.CreateICmpNE(extractInt(ec, C()), llvm::ConstantInt::get(ec.i64Ty, 0));
        storeBool(ec, A(), b.CreateAnd(lhsI, rhsI));
        break;
    }
    case OpCode::IOR_R:
    case OpCode::FLOR_R: {
        auto* lhsI = b.CreateICmpNE(extractInt(ec, B()), llvm::ConstantInt::get(ec.i64Ty, 0));
        auto* rhsI = b.CreateICmpNE(extractInt(ec, C()), llvm::ConstantInt::get(ec.i64Ty, 0));
        storeBool(ec, A(), b.CreateOr(lhsI, rhsI));
        break;
    }

    case OpCode::IEQ_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpEQ(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOEQ(a, b_); });
        break;

    case OpCode::INE_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpNE(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpONE(a, b_); });
        break;

    case OpCode::ILT_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSLT(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOLT(a, b_); });
        break;

    case OpCode::IGT_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSGT(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOGT(a, b_); });
        break;

    case OpCode::ILE_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSLE(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOLE(a, b_); });
        break;

    case OpCode::IGE_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSGE(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOGE(a, b_); });
        break;

    case OpCode::FLEQ_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpEQ(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOEQ(a, b_); });
        break;

    case OpCode::FLNE_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpNE(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpONE(a, b_); });
        break;

    case OpCode::FLLT_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSLT(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOLT(a, b_); });
        break;

    case OpCode::FLGT_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSGT(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOGT(a, b_); });
        break;

    case OpCode::FLLE_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSLE(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOLE(a, b_); });
        break;

    case OpCode::FLGE_R:
        emitBinaryCmp(ec, A(), B(), C(),
            [&](auto* a, auto* b_){ return b.CreateICmpSGE(a, b_); },
            [&](auto* a, auto* b_){ return b.CreateFCmpOGE(a, b_); });
        break;

    default:
        throw std::runtime_error(
            std::format("PhasorJIT: opcode {} cannot be JIT compiled",
                        static_cast<int>(instr.op)));
    }
}

} // namespace Phasor