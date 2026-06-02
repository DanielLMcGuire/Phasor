#ifndef CMAKE_PCH
#include "VM.hpp" // avoid breaking IDEs
#endif
#include <phsint.hpp>

namespace Phasor
{

void VM::evalLoop()
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

    if (pc >= m_bytecode->instructions.size()) return;

#ifdef TRACING
#define TRACE_INSTR(_op) \
    do { \
        log(std::format("\nVM::evalLoop(): RUN (pc={})\n", pc - 1)); \
        log(std::format("VM::evalLoop({}, {}, {}, {})\n", \
            opCodeToString(_op), operand1, operand2, operand3)); \
        flush(); \
    } while (0)
#else
#define TRACE_INSTR(_op) do {} while (0)
#endif

    static constexpr unsigned TABLE_SIZE = 512;
    static void*              s_table[TABLE_SIZE];
    static bool               s_ready = false;

    if (!s_ready) [[unlikely]]
    {
        for (auto& e : s_table) e = &&LABEL_UNKNOWN;

        s_table[(unsigned)OpCode::JUMP]                       = &&LABEL_JUMP;
        s_table[(unsigned)OpCode::CALL]                       = &&LABEL_CALL;
        s_table[(unsigned)OpCode::RETURN]                     = &&LABEL_RETURN;
        s_table[(unsigned)OpCode::CALL_NATIVE]                = &&LABEL_CALL_NATIVE;
        s_table[(unsigned)OpCode::JUMP_IF_FALSE]              = &&LABEL_JUMP_IF_FALSE;
        s_table[(unsigned)OpCode::JUMP_IF_TRUE]               = &&LABEL_JUMP_IF_TRUE;
        s_table[(unsigned)OpCode::JUMP_BACK]                  = &&LABEL_JUMP_BACK;
        s_table[(unsigned)OpCode::IMPORT]                     = &&LABEL_IMPORT;
        s_table[(unsigned)OpCode::HALT]                       = &&LABEL_HALT;

        s_table[(unsigned)OpCode::PUSH_CONST]                 = &&LABEL_PUSH_CONST;
        s_table[(unsigned)OpCode::POP]                        = &&LABEL_POP;
        s_table[(unsigned)OpCode::STORE_VAR]                  = &&LABEL_STORE_VAR;
        s_table[(unsigned)OpCode::LOAD_VAR]                   = &&LABEL_LOAD_VAR;
        s_table[(unsigned)OpCode::TRUE_P]                     = &&LABEL_TRUE_P;
        s_table[(unsigned)OpCode::FALSE_P]                    = &&LABEL_FALSE_P;
        s_table[(unsigned)OpCode::NULL_VAL]                   = &&LABEL_NULL_VAL;

        s_table[(unsigned)OpCode::IADD]                       = &&LABEL_IADD;
        s_table[(unsigned)OpCode::ISUBTRACT]                  = &&LABEL_ISUBTRACT;
        s_table[(unsigned)OpCode::IMULTIPLY]                  = &&LABEL_IMULTIPLY;
        s_table[(unsigned)OpCode::IDIVIDE]                    = &&LABEL_IDIVIDE;
        s_table[(unsigned)OpCode::IMODULO]                    = &&LABEL_IMODULO;
        s_table[(unsigned)OpCode::FLADD]                      = &&LABEL_FLADD;
        s_table[(unsigned)OpCode::FLSUBTRACT]                 = &&LABEL_FLSUBTRACT;
        s_table[(unsigned)OpCode::FLMULTIPLY]                 = &&LABEL_FLMULTIPLY;
        s_table[(unsigned)OpCode::FLDIVIDE]                   = &&LABEL_FLDIVIDE;
        s_table[(unsigned)OpCode::FLMODULO]                   = &&LABEL_FLMODULO;
        s_table[(unsigned)OpCode::SQRT]                       = &&LABEL_SQRT;
        s_table[(unsigned)OpCode::POW]                        = &&LABEL_POW;
        s_table[(unsigned)OpCode::LOG]                        = &&LABEL_LOG;
        s_table[(unsigned)OpCode::EXP]                        = &&LABEL_EXP;
        s_table[(unsigned)OpCode::SIN]                        = &&LABEL_SIN;
        s_table[(unsigned)OpCode::COS]                        = &&LABEL_COS;
        s_table[(unsigned)OpCode::TAN]                        = &&LABEL_TAN;

        s_table[(unsigned)OpCode::NEGATE]                     = &&LABEL_NEGATE;
        s_table[(unsigned)OpCode::NOT]                        = &&LABEL_NOT;
        s_table[(unsigned)OpCode::IAND]                       = &&LABEL_IAND;
        s_table[(unsigned)OpCode::IOR]                        = &&LABEL_IOR;
        s_table[(unsigned)OpCode::IEQUAL]                     = &&LABEL_IEQUAL;
        s_table[(unsigned)OpCode::INOT_EQUAL]                 = &&LABEL_INOT_EQUAL;
        s_table[(unsigned)OpCode::ILESS_THAN]                 = &&LABEL_ILESS_THAN;
        s_table[(unsigned)OpCode::IGREATER_THAN]              = &&LABEL_IGREATER_THAN;
        s_table[(unsigned)OpCode::ILESS_EQUAL]                = &&LABEL_ILESS_EQUAL;
        s_table[(unsigned)OpCode::IGREATER_EQUAL]             = &&LABEL_IGREATER_EQUAL;
        s_table[(unsigned)OpCode::FLAND]                      = &&LABEL_FLAND;
        s_table[(unsigned)OpCode::FLOR]                       = &&LABEL_FLOR;
        s_table[(unsigned)OpCode::FLEQUAL]                    = &&LABEL_FLEQUAL;
        s_table[(unsigned)OpCode::FLNOT_EQUAL]                = &&LABEL_FLNOT_EQUAL;
        s_table[(unsigned)OpCode::FLLESS_THAN]                = &&LABEL_FLLESS_THAN;
        s_table[(unsigned)OpCode::FLGREATER_THAN]             = &&LABEL_FLGREATER_THAN;
        s_table[(unsigned)OpCode::FLLESS_EQUAL]               = &&LABEL_FLLESS_EQUAL;
        s_table[(unsigned)OpCode::FLGREATER_EQUAL]            = &&LABEL_FLGREATER_EQUAL;

        s_table[(unsigned)OpCode::PRINT]                      = &&LABEL_PRINT;
        s_table[(unsigned)OpCode::PRINTERROR]                 = &&LABEL_PRINTERROR;
        s_table[(unsigned)OpCode::READLINE]                   = &&LABEL_READLINE;

        s_table[(unsigned)OpCode::SYSTEM]                     = &&LABEL_SYSTEM;
        s_table[(unsigned)OpCode::SYSTEM_OUT]                 = &&LABEL_SYSTEM_OUT;
        s_table[(unsigned)OpCode::SYSTEM_ERR]                 = &&LABEL_SYSTEM_ERR;

        s_table[(unsigned)OpCode::LEN]                        = &&LABEL_LEN;
        s_table[(unsigned)OpCode::CHAR_AT]                    = &&LABEL_CHAR_AT;
        s_table[(unsigned)OpCode::SUBSTR]                     = &&LABEL_SUBSTR;

        s_table[(unsigned)OpCode::NEW_STRUCT_INSTANCE_STATIC] = &&LABEL_NEW_STRUCT_INSTANCE_STATIC;
        s_table[(unsigned)OpCode::GET_FIELD_STATIC]           = &&LABEL_GET_FIELD_STATIC;
        s_table[(unsigned)OpCode::SET_FIELD_STATIC]           = &&LABEL_SET_FIELD_STATIC;
        s_table[(unsigned)OpCode::NEW_STRUCT]                 = &&LABEL_NEW_STRUCT;
        s_table[(unsigned)OpCode::SET_FIELD]                  = &&LABEL_SET_FIELD;
        s_table[(unsigned)OpCode::GET_FIELD]                  = &&LABEL_GET_FIELD;

        s_table[(unsigned)OpCode::MOV]                        = &&LABEL_MOV;
        s_table[(unsigned)OpCode::LOAD_CONST_R]               = &&LABEL_LOAD_CONST_R;
        s_table[(unsigned)OpCode::LOAD_VAR_R]                 = &&LABEL_LOAD_VAR_R;
        s_table[(unsigned)OpCode::STORE_VAR_R]                = &&LABEL_STORE_VAR_R;
        s_table[(unsigned)OpCode::PUSH_R]                     = &&LABEL_PUSH_R;
        s_table[(unsigned)OpCode::PUSH2_R]                    = &&LABEL_PUSH2_R;
        s_table[(unsigned)OpCode::POP_R]                      = &&LABEL_POP_R;
        s_table[(unsigned)OpCode::POP2_R]                     = &&LABEL_POP2_R;

        s_table[(unsigned)OpCode::IADD_R]                     = &&LABEL_IADD_R;
        s_table[(unsigned)OpCode::ISUB_R]                     = &&LABEL_ISUB_R;
        s_table[(unsigned)OpCode::IMUL_R]                     = &&LABEL_IMUL_R;
        s_table[(unsigned)OpCode::IDIV_R]                     = &&LABEL_IDIV_R;
        s_table[(unsigned)OpCode::IMOD_R]                     = &&LABEL_IMOD_R;
        s_table[(unsigned)OpCode::FLADD_R]                    = &&LABEL_FLADD_R;
        s_table[(unsigned)OpCode::FLSUB_R]                    = &&LABEL_FLSUB_R;
        s_table[(unsigned)OpCode::FLMUL_R]                    = &&LABEL_FLMUL_R;
        s_table[(unsigned)OpCode::FLDIV_R]                    = &&LABEL_FLDIV_R;
        s_table[(unsigned)OpCode::FLMOD_R]                    = &&LABEL_FLMOD_R;
        s_table[(unsigned)OpCode::SQRT_R]                     = &&LABEL_SQRT_R;
        s_table[(unsigned)OpCode::POW_R]                      = &&LABEL_POW_R;
        s_table[(unsigned)OpCode::LOG_R]                      = &&LABEL_LOG_R;
        s_table[(unsigned)OpCode::EXP_R]                      = &&LABEL_EXP_R;
        s_table[(unsigned)OpCode::SIN_R]                      = &&LABEL_SIN_R;
        s_table[(unsigned)OpCode::COS_R]                      = &&LABEL_COS_R;
        s_table[(unsigned)OpCode::TAN_R]                      = &&LABEL_TAN_R;

        s_table[(unsigned)OpCode::NEG_R]                      = &&LABEL_NEG_R;
        s_table[(unsigned)OpCode::NOT_R]                      = &&LABEL_NOT_R;
        s_table[(unsigned)OpCode::IEQ_R]                      = &&LABEL_IEQ_R;
        s_table[(unsigned)OpCode::INE_R]                      = &&LABEL_INE_R;
        s_table[(unsigned)OpCode::ILT_R]                      = &&LABEL_ILT_R;
        s_table[(unsigned)OpCode::IGT_R]                      = &&LABEL_IGT_R;
        s_table[(unsigned)OpCode::ILE_R]                      = &&LABEL_ILE_R;
        s_table[(unsigned)OpCode::IGE_R]                      = &&LABEL_IGE_R;
        s_table[(unsigned)OpCode::IAND_R]                     = &&LABEL_IAND_R;
        s_table[(unsigned)OpCode::IOR_R]                      = &&LABEL_IOR_R;
        s_table[(unsigned)OpCode::FLEQ_R]                     = &&LABEL_FLEQ_R;
        s_table[(unsigned)OpCode::FLNE_R]                     = &&LABEL_FLNE_R;
        s_table[(unsigned)OpCode::FLLT_R]                     = &&LABEL_FLLT_R;
        s_table[(unsigned)OpCode::FLGT_R]                     = &&LABEL_FLGT_R;
        s_table[(unsigned)OpCode::FLLE_R]                     = &&LABEL_FLLE_R;
        s_table[(unsigned)OpCode::FLGE_R]                     = &&LABEL_FLGE_R;
        s_table[(unsigned)OpCode::FLAND_R]                    = &&LABEL_FLAND_R;
        s_table[(unsigned)OpCode::FLOR_R]                     = &&LABEL_FLOR_R;

        s_table[(unsigned)OpCode::PRINT_R]                    = &&LABEL_PRINT_R;
        s_table[(unsigned)OpCode::PRINTERROR_R]               = &&LABEL_PRINTERROR_R;
        s_table[(unsigned)OpCode::READLINE_R]                 = &&LABEL_READLINE_R;

        s_table[(unsigned)OpCode::SYSTEM_R]                   = &&LABEL_SYSTEM_R;
        s_table[(unsigned)OpCode::SYSTEM_OUT_R]               = &&LABEL_SYSTEM_OUT_R;
        s_table[(unsigned)OpCode::SYSTEM_ERR_R]               = &&LABEL_SYSTEM_ERR_R;

		s_table[(unsigned)OpCode::EXIT_SCOPE]                 = &&LABEL_EXIT_SCOPE;

        s_ready = true;
    }

    int operand1 = 0, operand2 = 0, operand3 = 0;
    u8  rA = 0, rB = 0, rC = 0;

#define NEXT() \
    do { \
        if (pc >= m_bytecode->instructions.size()) [[unlikely]] return; \
        { \
            const Instruction& _i = m_bytecode->instructions[pc++]; \
            operand1 = _i.operand1; \
            operand2 = _i.operand2; \
            operand3 = _i.operand3; \
            rA       = static_cast<u8>(operand1); \
            rB       = static_cast<u8>(operand2); \
            rC       = static_cast<u8>(operand3); \
            TRACE_INSTR(_i.op); \
            const unsigned _op = static_cast<unsigned>(_i.op); \
            goto *(_op < TABLE_SIZE ? s_table[_op] : &&LABEL_UNKNOWN); \
        } \
    } while (0)

    NEXT();


    LABEL_JUMP:
    {
#ifdef TRACING
        log(std::format("JUMP: {} -> {}\n", pc - 1, operand1));
        flush();
#endif
        pc = operand1;
        NEXT();
    }

    LABEL_CALL:
    {
        {
            Value       funcNameVal = m_bytecode->constants[operand1];
            std::string funcName    = funcNameVal.asString();
            auto        it          = m_bytecode->functionEntries.find(funcName);
            if (it == m_bytecode->functionEntries.end())
                throw std::runtime_error("Unknown function: " + funcName);
#ifdef TRACING
            log(std::format("CALL: {} -> {}: {}\n", pc - 1, funcName, it->second));
            flush();
#endif
            callStack.push_back(static_cast<int>(pc));
            pc = it->second;
        }
        NEXT();
    }

    LABEL_RETURN:
    {
        if (isDirectCall)
        {
            pc = 0;
            throw VM::Halt();
        }
        if (callStack.empty()) [[unlikely]]
        {
            pc = m_bytecode->instructions.size();
            throw std::runtime_error("Cannot return from outside a function");
        }
#ifdef TRACING
        log(std::format("RETURN: {} -> {}\n", pc - 1, callStack.back()));
        flush();
#endif
        pc = callStack.back();
        callStack.pop_back();
        NEXT();
    }

    LABEL_CALL_NATIVE:
    {
        {
            Value       funcNameVal = m_bytecode->constants[operand1];
            std::string funcName    = funcNameVal.asString();
            auto        it          = nativeFunctions.find(funcName);
            if (it == nativeFunctions.end())
                throw std::runtime_error("Unknown native function: " + funcName);

            int                argCount = static_cast<int>(pop().asInt());
            std::vector<Value> args(argCount);
            for (int i = argCount - 1; i >= 0; --i)
                args[i] = pop();

#ifdef TRACING
            std::string argsText;
            for (auto& arg : args)
            {
                argsText += std::format("{:T}", arg);
                if (arg != args.back()) argsText += ", ";
            }
            log(std::format("CALL_NATIVE: {}({})\n", funcName, argsText));
            flush();
#endif
            push(it->second(args, this));
        }
        NEXT();
    }

    LABEL_JUMP_IF_FALSE:
    {
#ifdef TRACING
        log(std::format("JUMP_IF_FALSE: {} {} -> {}\n",
            peek().isTruthy() ? "TRUE" : "FALSE", pc - 1, operand1));
        flush();
#endif
        if (!pop().isTruthy()) pc = operand1;
        NEXT();
    }

    LABEL_JUMP_IF_TRUE:
    {
#ifdef TRACING
        log(std::format("JUMP_IF_TRUE: {} {} -> {}\n",
            peek().isTruthy() ? "TRUE" : "FALSE", pc - 1, operand1));
        flush();
#endif
        if (pop().isTruthy()) pc = operand1;
        NEXT();
    }

    LABEL_JUMP_BACK:
    {
#ifdef TRACING
        log(std::format("JUMP_BACK: {} -> {}\n", pc - 1, operand1));
        flush();
#endif
        pc = operand1;
        NEXT();
    }

    LABEL_IMPORT:
    {
        {
            Value       pathVal = m_bytecode->constants[operand1];
            std::string path    = pathVal.asString();
            if (importHandler)
                importHandler(path);
            else
                throw std::runtime_error("Import handler not set");
        }
        NEXT();
    }

    LABEL_HALT:
    {
        pc = m_bytecode->instructions.size();
        throw VM::Halt();
    }

    LABEL_PUSH_CONST:
    {
        if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
            throw std::runtime_error("Invalid constant index");
        push(m_bytecode->constants[operand1]);
        NEXT();
    }

    LABEL_POP:
    {
        pop();
        NEXT();
    }

    LABEL_STORE_VAR:
    {
        if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
            throw std::runtime_error("Invalid variable index");
        variables[operand1] = pop();
        NEXT();
    }

    LABEL_LOAD_VAR:
    {
        if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
            throw std::runtime_error("Invalid variable index");
        push(variables[operand1]);
        NEXT();
    }

    LABEL_TRUE_P:   { push(Value(true));  NEXT(); }
    LABEL_FALSE_P:  { push(Value(false)); NEXT(); }
    LABEL_NULL_VAL: { push(Value());      NEXT(); }

    // STACK ARITHMETIC

    LABEL_IADD:      { { Value b = pop(), a = pop(); push(asm_iadd(a.asInt(),     b.asInt()));     } NEXT(); }
    LABEL_ISUBTRACT: { { Value b = pop(), a = pop(); push(asm_isub(a.asInt(),     b.asInt()));     } NEXT(); }
    LABEL_IMULTIPLY: { { Value b = pop(), a = pop(); push(asm_imul(a.asInt(),     b.asInt()));     } NEXT(); }
    LABEL_IDIVIDE:   { { Value b = pop(), a = pop(); push(asm_idiv(a.asInt(),     b.asInt()));     } NEXT(); }
    LABEL_IMODULO:   { { Value b = pop(), a = pop(); push(asm_imod(a.asInt(),     b.asInt()));     } NEXT(); }
    LABEL_FLADD:     { { Value b = pop(), a = pop(); push(asm_fladd(a.asFloat(),  b.asFloat()));   } NEXT(); }
    LABEL_FLSUBTRACT:{ { Value b = pop(), a = pop(); push(asm_flsub(a.asFloat(),  b.asFloat()));   } NEXT(); }
    LABEL_FLMULTIPLY:{ { Value b = pop(), a = pop(); push(asm_flmul(a.asFloat(),  b.asFloat()));   } NEXT(); }
    LABEL_FLDIVIDE:  { { Value b = pop(), a = pop(); push(asm_fldiv(a.asFloat(),  b.asFloat()));   } NEXT(); }
    LABEL_FLMODULO:  { { Value b = pop(), a = pop(); push(asm_flmod(a.asFloat(),  b.asFloat()));   } NEXT(); }
    LABEL_SQRT:      { push(asm_sqrt(pop().asFloat()));                                              NEXT(); }
    LABEL_POW:       { { Value b = pop(), a = pop(); push(asm_pow(a.asFloat(),    b.asFloat()));   } NEXT(); }
    LABEL_LOG:       { push(asm_log(pop().asFloat()));                                               NEXT(); }
    LABEL_EXP:       { push(asm_exp(pop().asFloat()));                                               NEXT(); }
    LABEL_SIN:       { push(asm_sin(pop().asFloat()));                                               NEXT(); }
    LABEL_COS:       { push(asm_cos(pop().asFloat()));                                               NEXT(); }
    LABEL_TAN:       { push(asm_tan(pop().asFloat()));                                               NEXT(); }

    // STACK LOGICAL

    LABEL_NEGATE: { push(asm_flneg(pop().asFloat()));                               NEXT(); }
    LABEL_NOT:    { push(Value(asm_flnot(pop().isTruthy() ? 1 : 0)));               NEXT(); }

    LABEL_IAND: { { Value b=pop(),a=pop(); push(Value(asm_iand(a.isTruthy()?1:0, b.isTruthy()?1:0))); } NEXT(); }
    LABEL_IOR:  { { Value b=pop(),a=pop(); push(Value(asm_ior (a.isTruthy()?1:0, b.isTruthy()?1:0))); } NEXT(); }

    LABEL_IEQUAL:         { { Value b=pop(),a=pop(); push(a.isInt()&&b.isInt() ? Value(asm_iequal(a.asInt(),b.asInt()))          : Value(a==b)); } NEXT(); }
    LABEL_INOT_EQUAL:     { { Value b=pop(),a=pop(); push(a.isInt()&&b.isInt() ? Value(asm_inot_equal(a.asInt(),b.asInt()))      : Value(a!=b)); } NEXT(); }
    LABEL_ILESS_THAN:     { { Value b=pop(),a=pop(); push(a.isInt()&&b.isInt() ? Value(asm_iless_than(a.asInt(),b.asInt()))      : Value(a< b)); } NEXT(); }
    LABEL_IGREATER_THAN:  { { Value b=pop(),a=pop(); push(a.isInt()&&b.isInt() ? Value(asm_igreater_than(a.asInt(),b.asInt()))   : Value(a> b)); } NEXT(); }
    LABEL_ILESS_EQUAL:    { { Value b=pop(),a=pop(); push(a.isInt()&&b.isInt() ? Value(asm_iless_equal(a.asInt(),b.asInt()))     : Value(a<=b)); } NEXT(); }
    LABEL_IGREATER_EQUAL: { { Value b=pop(),a=pop(); push(a.isInt()&&b.isInt() ? Value(asm_igreater_equal(a.asInt(),b.asInt()))  : Value(a>=b)); } NEXT(); }

    LABEL_FLAND:          { { Value b=pop(),a=pop(); push(Value(asm_fland(a.isTruthy()?1:0, b.isTruthy()?1:0)));                                 } NEXT(); }
    LABEL_FLOR:           { { Value b=pop(),a=pop(); push(Value(asm_flor (a.isTruthy()?1:0, b.isTruthy()?1:0)));                                 } NEXT(); }
    LABEL_FLEQUAL: { { 
		Value b = pop(), a = pop(); 
		push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flequal(a.asFloat(), b.asFloat())) : Value(a == b)); 
	} NEXT(); }
    LABEL_FLNOT_EQUAL: { {
        Value b = pop(), a = pop();
        push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flnot_equal(a.asFloat(), b.asFloat())) : Value(a != b));
    } NEXT(); }
    LABEL_FLLESS_THAN: { {
        Value b = pop(), a = pop();
            push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flless_than(a.asFloat(), b.asFloat())) : Value(a < b));
    } NEXT(); }
    LABEL_FLGREATER_THAN: { {
        Value b = pop(), a = pop();
        push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flgreater_than(a.asFloat(), b.asFloat())) : Value(a > b));
    } NEXT(); }
    LABEL_FLLESS_EQUAL: { {
            Value b = pop(), a = pop();
            push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flless_equal(a.asFloat(), b.asFloat())) : Value(a <= b));
        } NEXT(); }
    LABEL_FLGREATER_EQUAL: { {
            Value b = pop(), a = pop();
            push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flgreater_equal(a.asFloat(), b.asFloat())) : Value(a >= b));
    } NEXT(); }

    // STACK I/O

    LABEL_PRINT:
    {
        {
            Value       v = pop();
            std::string s = v.toString();
#ifdef TRACING
            log(std::format("PRINT: (stdout) {:T}\n", v));
#else
            c_print_stdout(s.c_str(), (i64)s.length());
#endif
            flush();
        }
        NEXT();
    }

    LABEL_PRINTERROR:
    {
        {
            Value       v = pop();
            std::string s = v.toString();
#ifdef TRACING
            log(std::format("PRINTERROR: (stderr) {:T}\n", v));
#else
            c_print_stderr(s.c_str(), (i64)s.length());
#endif
            flusherr();
        }
        NEXT();
    }

    LABEL_READLINE:
    {
        {
            std::string s;
#ifdef TRACING
            log("READLINE:");
            flush();
#endif
            std::getline(std::cin, s);
#ifdef TRACING
            log(std::format("\nREADLINE: {}\n", s));
#endif
            push(s);
        }
        NEXT();
    }

    // STACK SYSTEM

    LABEL_SYSTEM:
    {
#ifdef SANDBOXED
        logerr("CANNOT ESCAPE SANDBOX");
        push(Value());
#else
        {
#ifdef TRACING
            Value cmd = pop();
            int   ret = c_system(cmd.c_str());
            log(std::format("SYSTEM: {:T} -> {}\n", cmd, ret));
            push(ret);
#else
            push(c_system(pop().c_str()));
#endif
        }
#endif
        NEXT();
    }

    LABEL_SYSTEM_OUT:
    {
#ifdef SANDBOXED
        logerr("CANNOT ESCAPE SANDBOX");
        push(Value());
#else
        {
#ifdef TRACING
            Value       cmd = pop();
            std::string ret = c_system_out(cmd.c_str());
            log(std::format("SYSTEM_OUT: {:T} -> {}\n", cmd, ret));
            push(ret);
#else
            push(c_system_out(pop().c_str()));
#endif
        }
#endif
        NEXT();
    }

    LABEL_SYSTEM_ERR:
    {
#ifdef SANDBOXED
        logerr("CANNOT ESCAPE SANDBOX");
        push(Value());
#else
        {
#ifdef TRACING
            Value       cmd = pop();
            std::string ret = c_system_err(cmd.c_str());
            log(std::format("SYSTEM_ERR: {:T} -> {}\n", cmd, ret));
            push(ret);
#else
            push(c_system_err(pop().c_str()));
#endif
        }
#endif
        NEXT();
    }

    
    // STACK STRING
    

    LABEL_LEN:
    {
        {
            Value v = pop();
            if (v.isArray())
            {
                push(Value(static_cast<i64>(v.asArray()->size())));
            }
            else
            {
                push(Value(static_cast<i64>(v.asString().length())));
            }
        }
        NEXT();
    }

    LABEL_CHAR_AT:
    {
        {
            Value       idxVal = pop();
            Value       strVal = pop();
            std::string s;
            if (strVal.isString()) s = strVal.asString();
            else                   s = strVal.toString();

            i64 idx = 0;
            if (idxVal.isInt())         idx = idxVal.asInt();
            else if (idxVal.isFloat())  idx = static_cast<i64>(idxVal.asFloat());
            else if (idxVal.isString())
            {
                try { idx = std::stoll(idxVal.asString()); }
                catch (...) { throw std::runtime_error("char_at() expects index convertible to integer"); }
            }
            else throw std::runtime_error("char_at() expects string and integer");

            if (idx < 0 || idx >= static_cast<i64>(s.length()))
                push(Value(""));
            else
                push(Value(std::string(1, s[static_cast<size_t>(idx)])));
        }
        NEXT();
    }

    LABEL_SUBSTR:
    {
        {
            Value lenVal   = pop();
            Value startVal = pop();
            Value strVal   = pop();
            if (strVal.isString() && startVal.isInt() && lenVal.isInt())
            {
                const std::string& s     = strVal.asString();
                i64                start = startVal.asInt();
                i64                len   = lenVal.asInt();
                if (start < 0 || start >= static_cast<i64>(s.length()))
                    push(Value(""));
                else
                    push(Value(s.substr(start, len)));
            }
            else
            {
                throw std::runtime_error("substr() expects string, int, int");
            }
        }
        NEXT();
    }

    
    // STACK STRUCT
    

    LABEL_NEW_STRUCT_INSTANCE_STATIC:
    {
        {
            if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
                throw std::runtime_error("Invalid struct index for NEW_STRUCT_INSTANCE_STATIC");
            const StructInfo& info     = m_bytecode->structs[operand1];
            Value             instance = Value::createStruct(info.name);
            for (int i = 0; i < info.fieldCount; ++i)
            {
                int constIndex = info.firstConstIndex + i;
                if (constIndex < 0 || constIndex >= static_cast<int>(m_bytecode->constants.size()))
                    throw std::runtime_error("Invalid default constant index for struct field");
                instance.setField(info.fieldNames[i], m_bytecode->constants[constIndex]);
            }
            push(instance);
        }
        NEXT();
    }

    LABEL_GET_FIELD_STATIC:
    {
        {
            if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
                throw std::runtime_error("Invalid struct index for GET_FIELD_STATIC");
            const StructInfo& info        = m_bytecode->structs[operand1];
            int               fieldOffset = operand2;
            if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
                throw std::runtime_error("Invalid field offset for GET_FIELD_STATIC");
            Value obj = pop();
            push(obj.getField(info.fieldNames[fieldOffset]));
        }
        NEXT();
    }

    LABEL_SET_FIELD_STATIC:
    {
        {
            if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
                throw std::runtime_error("Invalid struct index for SET_FIELD_STATIC");
            const StructInfo& info        = m_bytecode->structs[operand1];
            int               fieldOffset = operand2;
            if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
                throw std::runtime_error("Invalid field offset for SET_FIELD_STATIC");
            Value value = pop();
            Value obj   = pop();
            obj.setField(info.fieldNames[fieldOffset], value);
            push(obj);
        }
        NEXT();
    }

    LABEL_NEW_STRUCT:
    {
        if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
            throw std::runtime_error("Invalid constant index for NEW_STRUCT");
        push(Value::createStruct(m_bytecode->constants[operand1].asString()));
        NEXT();
    }

    LABEL_SET_FIELD:
    {
        {
            if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
                throw std::runtime_error("Invalid constant index for SET_FIELD");
            std::string fieldName = m_bytecode->constants[operand1].asString();
            Value       value     = pop();
            Value       obj       = pop();
            obj.setField(fieldName, value);
            push(obj);
        }
        NEXT();
    }

    LABEL_GET_FIELD:
    {
        {
            if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
                throw std::runtime_error("Invalid constant index for GET_FIELD");
            Value obj = pop();
            push(obj.getField(m_bytecode->constants[operand1].asString()));
        }
        NEXT();
    }

    
    // REGISTER CORE
    

    LABEL_MOV:         { registers[rA] = registers[rB]; NEXT(); }

    LABEL_LOAD_CONST_R:
    {
        int constIndex = operand2;
        if (constIndex < 0 || constIndex >= static_cast<int>(m_bytecode->constants.size()))
            throw std::runtime_error("Invalid constant index");
        registers[rA] = m_bytecode->constants[constIndex];
        NEXT();
    }

    LABEL_LOAD_VAR_R:
    {
        int varIndex = operand2;
        if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
            throw std::runtime_error("Invalid variable index");
        registers[rA] = variables[varIndex];
        NEXT();
    }

    LABEL_STORE_VAR_R:
    {
        int varIndex = operand2;
        if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
            throw std::runtime_error("Invalid variable index");
        variables[varIndex] = registers[rA];
        NEXT();
    }

    LABEL_PUSH_R:  { push(registers[rA]);                         NEXT(); }
    LABEL_PUSH2_R: { push(registers[rA]); push(registers[rB]);    NEXT(); }
    LABEL_POP_R:   { registers[rA] = pop();                       NEXT(); }
    LABEL_POP2_R:  { registers[rA] = pop(); registers[rB] = pop();NEXT(); }

    
    // REGISTER ARITHMETIC
    

    LABEL_IADD_R:  { registers[rA] = Value(asm_iadd (registers[rB].asInt(),    registers[rC].asInt()));    NEXT(); }
    LABEL_ISUB_R:  { registers[rA] = Value(asm_isub (registers[rB].asInt(),    registers[rC].asInt()));    NEXT(); }
    LABEL_IMUL_R:  { registers[rA] = Value(asm_imul (registers[rB].asInt(),    registers[rC].asInt()));    NEXT(); }
    LABEL_IDIV_R:  { registers[rA] = Value(asm_idiv (registers[rB].asInt(),    registers[rC].asInt()));    NEXT(); }
    LABEL_IMOD_R:  { registers[rA] = Value(asm_imod (registers[rB].asInt(),    registers[rC].asInt()));    NEXT(); }
    LABEL_FLADD_R: { registers[rA] = Value(asm_fladd(registers[rB].asFloat(),  registers[rC].asFloat()));  NEXT(); }
    LABEL_FLSUB_R: { registers[rA] = Value(asm_flsub(registers[rB].asFloat(),  registers[rC].asFloat()));  NEXT(); }
    LABEL_FLMUL_R: { registers[rA] = Value(asm_flmul(registers[rB].asFloat(),  registers[rC].asFloat()));  NEXT(); }
    LABEL_FLDIV_R: { registers[rA] = Value(asm_fldiv(registers[rB].asFloat(),  registers[rC].asFloat()));  NEXT(); }
    LABEL_FLMOD_R: { registers[rA] = Value(asm_flmod(registers[rB].asFloat(),  registers[rC].asFloat()));  NEXT(); }
    LABEL_SQRT_R:  { registers[rA] = Value(asm_sqrt (registers[rB].asFloat()));                            NEXT(); }
    LABEL_POW_R:   { registers[rA] = Value(asm_pow  (registers[rB].asFloat(),  registers[rC].asFloat()));  NEXT(); }
    LABEL_LOG_R:   { registers[rA] = Value(asm_log  (registers[rB].asFloat()));                              NEXT(); }
    LABEL_EXP_R:   { registers[rA] = Value(asm_exp  (registers[rB].asFloat()));                              NEXT(); }
    LABEL_SIN_R:   { registers[rA] = Value(asm_sin  (registers[rB].asFloat()));                              NEXT(); }
    LABEL_COS_R:   { registers[rA] = Value(asm_cos  (registers[rB].asFloat()));                              NEXT(); }
    LABEL_TAN_R:   { registers[rA] = Value(asm_tan  (registers[rB].asFloat()));                              NEXT(); }

    
    // REGISTER LOGICAL
    

    LABEL_NEG_R:   { registers[rA] = Value(asm_flneg(registers[rB].asFloat()));                                    NEXT(); }
    LABEL_NOT_R:   { registers[rA] = Value(asm_flnot(registers[rB].isTruthy() ? 1 : 0));                           NEXT(); }
    LABEL_IAND_R:  { Value &b=registers[rB],&c=registers[rC]; registers[rA]=Value(asm_iand(b.isTruthy()?1:0,c.isTruthy()?1:0)); NEXT(); }
    LABEL_IOR_R:   { Value &b=registers[rB],&c=registers[rC]; registers[rA]=Value(asm_ior (b.isTruthy()?1:0,c.isTruthy()?1:0)); NEXT(); }

    LABEL_IEQ_R:   { Value &b=registers[rB],&c=registers[rC]; registers[rA]=(b.isInt()&&c.isInt())     ? Value(asm_iequal(b.asInt(),c.asInt()))             : Value(b==c); NEXT(); }
    LABEL_INE_R:   { Value &b=registers[rB],&c=registers[rC]; registers[rA]=(b.isInt()&&c.isInt())     ? Value(asm_inot_equal(b.asInt(),c.asInt()))         : Value(b!=c); NEXT(); }
    LABEL_ILT_R:   { Value &b=registers[rB],&c=registers[rC]; registers[rA]=(b.isInt()&&c.isInt())     ? Value(asm_iless_than(b.asInt(),c.asInt()))         : Value(b< c); NEXT(); }
    LABEL_IGT_R:   { Value &b=registers[rB],&c=registers[rC]; registers[rA]=(b.isInt()&&c.isInt())     ? Value(asm_igreater_than(b.asInt(),c.asInt()))      : Value(b> c); NEXT(); }
    LABEL_ILE_R:   { Value &b=registers[rB],&c=registers[rC]; registers[rA]=(b.isInt()&&c.isInt())     ? Value(asm_iless_equal(b.asInt(),c.asInt()))        : Value(b<=c); NEXT(); }
    LABEL_IGE_R:   { Value &b=registers[rB],&c=registers[rC]; registers[rA]=(b.isInt()&&c.isInt())     ? Value(asm_igreater_equal(b.asInt(),c.asInt()))     : Value(b>=c); NEXT(); }
    LABEL_FLEQ_R: {
        Value &b = registers[rB], &c = registers[rC];
        registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flequal(b.asFloat(), c.asFloat())) : Value(b == c);
        NEXT();
    }
    LABEL_FLNE_R: {
        Value &b = registers[rB], &c = registers[rC];
        registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flnot_equal(b.asFloat(), c.asFloat())) : Value(b != c);
        NEXT();
    }
    LABEL_FLLT_R: {
        Value &b = registers[rB], &c = registers[rC];
        registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flless_than(b.asFloat(), c.asFloat())) : Value(b < c);
        NEXT();
    }
    LABEL_FLGT_R: {
        Value &b = registers[rB], &c = registers[rC];
        registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flgreater_than(b.asFloat(), c.asFloat())) : Value(b > c);
        NEXT();
    }
    LABEL_FLLE_R: {
        Value &b = registers[rB], &c = registers[rC];
        registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flless_equal(b.asFloat(), c.asFloat())) : Value(b <= c);
        NEXT();
    }
    LABEL_FLGE_R: {
        Value &b = registers[rB], &c = registers[rC];
        registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flgreater_equal(b.asFloat(), c.asFloat())) : Value(b >= c);
        NEXT();
    }
    LABEL_FLAND_R: { Value &b=registers[rB],&c=registers[rC]; registers[rA]=Value(asm_fland(b.isTruthy()?1:0,c.isTruthy()?1:0)); NEXT(); }
    LABEL_FLOR_R:  { Value &b=registers[rB],&c=registers[rC]; registers[rA]=Value(asm_flor (b.isTruthy()?1:0,c.isTruthy()?1:0)); NEXT(); }

    
    // REGISTER I/O
    

    LABEL_PRINT_R:
    {
        {
            std::string s = registers[rA].toString();
#ifdef TRACING
            log(std::format("PRINT_R: (stdout) {:T}\n", registers[rA]));
#else
            c_print_stdout(s.c_str(), (i64)s.length());
#endif
            flush();
        }
        NEXT();
    }

    LABEL_PRINTERROR_R:
    {
        {
            std::string s = registers[rA].toString();
#ifdef TRACING
            log(std::format("PRINTERROR_R: (stderr) {:T}\n", registers[rA]));
#else
            c_print_stderr(s.c_str(), (i64)s.length());
#endif
            flusherr();
        }
        NEXT();
    }

    LABEL_READLINE_R:
    {
        {
            std::string s;
#ifdef TRACING
            log("READLINE_R:");
            flush();
#endif
            std::getline(std::cin, s);
#ifdef TRACING
            log(std::format("\nREADLINE_R: {}\n", s));
#endif
            registers[rA] = s;
        }
        NEXT();
    }
    
    // REGISTER SYSTEM

    LABEL_SYSTEM_R:
    {
#ifdef SANDBOXED
        logerr("CANNOT ESCAPE SANDBOX");
        registers[rA] = Value();
#else
        {
#ifdef TRACING
            Value cmd = registers[rA];
            i64   ret = c_system(cmd.c_str());
            log(std::format("SYSTEM_R: {} -> {}\n", cmd, ret));
            registers[rA] = ret;
#else
            registers[rA] = c_system(registers[rA].c_str());
#endif
        }
#endif
        NEXT();
    }

    LABEL_SYSTEM_OUT_R:
    {
#ifdef SANDBOXED
        logerr("CANNOT ESCAPE SANDBOX");
        registers[rA] = Value();
#else
        {
#ifdef TRACING
            Value       cmd = registers[rA];
            std::string ret = c_system_out(cmd.c_str());
            log(std::format("SYSTEM_R: {:T} -> {}\n", cmd, ret));
            registers[rA] = ret;
#else
            registers[rA] = c_system_out(registers[rA].c_str());
#endif
        }
#endif
        NEXT();
    }

    LABEL_SYSTEM_ERR_R:
    {
#ifdef SANDBOXED
        logerr("CANNOT ESCAPE SANDBOX");
        registers[rA] = Value();
#else
        {
#ifdef TRACING
            Value       cmd = registers[rA];
            std::string ret = c_system_err(cmd.c_str());
            log(std::format("SYSTEM_ERR_R: {:T} -> {}\n", cmd, ret));
            registers[rA] = ret;
#else
            registers[rA] = c_system_err(registers[rA].c_str());
#endif
        }
#endif
        NEXT();
    }

	LABEL_EXIT_SCOPE:
	{
		int scopeId = operand1;
		for (const auto &[idx, name] : m_bytecode->scopeVarLists[scopeId])
		{
			if (idx < 0 || idx >= static_cast<int>(variables.size()))
				throw std::runtime_error("Invalid variable index in scope exit");
			variables[idx] = Value();
		}
		NEXT();
	}
    
    // UNKNOWN
    
    LABEL_UNKNOWN:
        throw std::runtime_error("Unknown opcode");

#undef NEXT
#undef TRACE_INSTR

#else
    while (pc < m_bytecode->instructions.size())
    {
        const Instruction& instr = m_bytecode->instructions[pc++];
#ifdef TRACING
        log(std::format("\nVM::{}(): RUN (pc={})\n", __func__, pc - 1));
        flush();
#endif
        operation(instr.op, instr.operand1, instr.operand2, instr.operand3);
    }

#pragma GCC diagnostic pop
#endif // defined(__GNUC__) || defined(__clang__)
}

Value VM::operation(const OpCode &op, const int &operand1, const int &operand2, const int &operand3)
{
	u8 rA = static_cast<u8>(operand1);
	u8 rB = static_cast<u8>(operand2);
	u8 rC = static_cast<u8>(operand3);
#ifdef TRACING
	log(std::format("VM::{}({}, {}, {}, {})\n", __func__, opCodeToString(op), operand1, operand2, operand3));
	flush();
#endif
	switch (op)
	{

#pragma region CONTROL FLOW

	[[likely]] case OpCode::JUMP: {
#ifdef TRACING
		log(std::format("JUMP: {} -> {}\n", pc - 1, operand1));
		flush();
#endif
		pc = operand1;
		break;
	}

	[[likely]] case OpCode::CALL: {
		Value       funcNameVal = m_bytecode->constants[operand1];
		std::string funcName = funcNameVal.asString();
		auto        it = m_bytecode->functionEntries.find(funcName);
		if (it == m_bytecode->functionEntries.end())
			throw std::runtime_error("Unknown function: " + funcName);
#ifdef TRACING
		log(std::format("CALL: {} -> {}: {}\n", pc - 1, funcName, it->second));
		flush();
#endif
		callStack.push_back(static_cast<int>(pc));
		pc = it->second;
		break;
	}
	[[likely]] case OpCode::RETURN: {
		if (isDirectCall)
		{
			pc = 0;
			throw VM::Halt();
			break;
		}
		if (callStack.empty()) [[unlikely]]
		{
			pc = m_bytecode->instructions.size();
			throw std::runtime_error("Cannot return from outside a function");
			break;
		}
#ifdef TRACING
		log(std::format("RETURN: {} -> {}\n", pc - 1, callStack.back()));
		flush();
#endif
		pc = callStack.back();
		callStack.pop_back();
		break;
	}

	[[likely]] case OpCode::CALL_NATIVE: {
		Value       funcNameVal = m_bytecode->constants[operand1];
		std::string funcName = funcNameVal.asString();
		auto        it = nativeFunctions.find(funcName);
		if (it == nativeFunctions.end())
			throw std::runtime_error("Unknown native function: " + funcName);

		int                argCount = static_cast<int>(pop().asInt());
		std::vector<Value> args(argCount);
		for (int i = argCount - 1; i >= 0; --i)
			args[i] = pop();

#ifdef TRACING
		std::string argsText;
		for (auto &arg : args)
		{
			argsText += std::format("{:T}", arg);
			if (arg != args.back())
				argsText += ", ";
		}
		log(std::format("CALL_NATIVE: {}({})\n", funcName, argsText));
		flush();
#endif

		push(it->second(args, this));

		break;
	}

	case OpCode::JUMP_IF_FALSE: {
#ifdef TRACING
		log(std::format("JUMP_IF_FALSE: {} {} -> {}\n", peek().isTruthy() ? "TRUE" : "FALSE", pc - 1, operand1));
		flush();
#endif
		if (!pop().isTruthy())
			pc = operand1;
		break;
	}

	case OpCode::JUMP_IF_TRUE: {
#ifdef TRACING
		log(std::format("JUMP_IF_TRUE: {} {} -> {}\n", peek().isTruthy() ? "TRUE" : "FALSE", pc - 1, operand1));
		flush();
#endif
		if (pop().isTruthy())
			pc = operand1;
		break;
	}

	case OpCode::JUMP_BACK: {
#ifdef TRACING
		log(std::format("JUMP_BACK: {} -> {}\n", pc - 1, operand1));
		flush();
#endif
		pc = operand1;
		break;
	}

	[[unlikely]] case OpCode::IMPORT: {
		Value       pathVal = m_bytecode->constants[operand1];
		std::string path = pathVal.asString();
		if (importHandler)
			importHandler(path);
		else
			throw std::runtime_error("Import handler not set");
		break;
	}

	[[unlikely]] case OpCode::HALT: {
		pc = m_bytecode->instructions.size();
		throw VM::Halt();
		break;
	}

#pragma endregion
#pragma region STACK CORE

	[[likely]] case OpCode::PUSH_CONST: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index");
		push(m_bytecode->constants[operand1]);
		break;
	}

	[[likely]] case OpCode::POP: {
		pop();
		break;
	}

	[[likely]] case OpCode::STORE_VAR: {
		if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		variables[operand1] = pop();
		break;
	}

	[[likely]] case OpCode::LOAD_VAR: {
		if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		push(variables[operand1]);
		break;
	}

	case OpCode::TRUE_P: {
		push(Value(true));
		break;
	}

	case OpCode::FALSE_P: {
		push(Value(false));
		break;
	}

	case OpCode::NULL_VAL: {
		push(Value());
		break;
	}

#pragma endregion
#pragma region STACK ARITHMETIC

	case OpCode::IADD: {
		Value b = pop();
		Value a = pop();
		push(asm_iadd(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::ISUBTRACT: {
		Value b = pop();
		Value a = pop();
		push(asm_isub(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::IMULTIPLY: {
		Value b = pop();
		Value a = pop();
		push(asm_imul(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::IDIVIDE: {
		Value b = pop();
		Value a = pop();
		push(asm_idiv(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::IMODULO: {
		Value b = pop();
		Value a = pop();
		push(asm_imod(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::FLADD: {
		Value b = pop();
		Value a = pop();
		push(asm_fladd(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLSUBTRACT: {
		Value b = pop();
		Value a = pop();
		push(asm_flsub(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLMULTIPLY: {
		Value b = pop();
		Value a = pop();
		push(asm_flmul(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLDIVIDE: {
		Value b = pop();
		Value a = pop();
		push(asm_fldiv(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLMODULO: {
		Value b = pop();
		Value a = pop();
		push(asm_flmod(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::SQRT: {
		Value a = pop();
		push(asm_sqrt(a.asFloat()));
		break;
	}

	case OpCode::POW: {
		Value b = pop();
		Value a = pop();
		push(asm_pow(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::LOG: {
		Value a = pop();
		push(asm_log(a.asFloat()));
		break;
	}

	case OpCode::EXP: {
		Value a = pop();
		push(asm_exp(a.asFloat()));
		break;
	}

	case OpCode::SIN: {
		Value a = pop();
		push(asm_sin(a.asFloat()));
		break;
	}

	case OpCode::COS: {
		Value a = pop();
		push(asm_cos(a.asFloat()));
		break;
	}

	case OpCode::TAN: {
		Value a = pop();
		push(asm_tan(a.asFloat()));
		break;
	}

#pragma endregion
#pragma region STACK LOGICAL

	case OpCode::NEGATE: {
		push(asm_flneg(pop().asFloat()));
		break;
	}

	case OpCode::NOT: {
		push(Value(asm_flnot(pop().isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::IAND: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_iand(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::IOR: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_ior(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::IEQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_iequal(a.asInt(), b.asInt())) : Value(a == b));
		break;
	}

	case OpCode::INOT_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_inot_equal(a.asInt(), b.asInt())) : Value(a != b));
		break;
	}

	case OpCode::ILESS_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_iless_than(a.asInt(), b.asInt())) : Value(a < b));
		break;
	}

	case OpCode::IGREATER_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_igreater_than(a.asInt(), b.asInt())) : Value(a > b));
		break;
	}

	case OpCode::ILESS_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_iless_equal(a.asInt(), b.asInt())) : Value(a <= b));
		break;
	}

	case OpCode::IGREATER_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_igreater_equal(a.asInt(), b.asInt())) : Value(a >= b));
		break;
	}

	case OpCode::FLAND: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_fland(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::FLOR: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_flor(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::FLEQUAL: {
		Value b = pop();
		Value a = pop();
		push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flequal(a.asFloat(), b.asFloat())) : Value(a == b));
		break;
	}

	case OpCode::FLNOT_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flnot_equal(a.asFloat(), b.asFloat())) : Value(a != b));
		break;
	}

	case OpCode::FLLESS_THAN: {
		Value b = pop();
		Value a = pop();
		push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flless_than(a.asFloat(), b.asFloat())) : Value(a < b));
		break;
	}

	case OpCode::FLGREATER_THAN: {
		Value b = pop();
		Value a = pop();
		push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flgreater_than(a.asFloat(), b.asFloat())) : Value(a > b));
		break;
	}

	case OpCode::FLLESS_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flless_equal(a.asFloat(), b.asFloat())) : Value(a <= b));
		break;
	}

	case OpCode::FLGREATER_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(((a.isFloat() || a.isInt()) && (b.isFloat() || b.isInt())) ? Value(asm_flgreater_equal(a.asFloat(), b.asFloat())) : Value(a >= b));
		break;
	}

#pragma endregion
#pragma region STACK I/O

	case OpCode::PRINT: {
		Value       v = pop();
		std::string s = v.toString();
#ifdef TRACING
		log(std::format("PRINT: (stdout) {:T}\n", v));
#else
		c_print_stdout(s.c_str(), (i64)s.length());
#endif
		flush();
		break;
	}

	[[unlikely]] case OpCode::PRINTERROR: {
		Value       v = pop();
		std::string s = v.toString();
#ifdef TRACING
		log(std::format("PRINTERROR: (stderr) {:T}\n", v));
#else
		c_print_stderr(s.c_str(), (i64)s.length());
#endif
		flusherr();
		break;
	}

	case OpCode::READLINE: {
		std::string s;
#ifdef TRACING
		log("READLINE:");
		flush();
#endif
		std::getline(std::cin, s);
#ifdef TRACING
		log(std::format("\nREADLINE: {}\n", s));
#endif
		push(s);
		break;
	}

#pragma endregion
#pragma region STACK SYSTEM

	case OpCode::SYSTEM: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		push(Value());
#else
#ifdef TRACING
		Value cmd = pop();
		int   ret = c_system(cmd.c_str());
		log(std::format("SYSTEM: {:T} -> {}\n", cmd, ret));
		push(ret);
#else
		push(c_system(pop().c_str()));
#endif
#endif
		break;
	}

	case OpCode::SYSTEM_OUT: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		push(Value());
#else
#ifdef TRACING
		Value       cmd = pop();
		std::string ret = c_system_out(cmd.c_str());
		log(std::format("SYSTEM_OUT: {:T} -> {}\n", cmd, ret));
		push(ret);
#else
		push(c_system_out(pop().c_str()));
#endif
#endif
		break;
	}

	case OpCode::SYSTEM_ERR: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		push(Value());
#else
#ifdef TRACING
		Value       cmd = pop();
		std::string ret = c_system_err(cmd.c_str());
		log(std::format("SYSTEM_ERR: {:T} -> {}\n", cmd, ret));
		push(ret);
#else
		push(c_system_err(pop().c_str()));
#endif
#endif
		break;
	}

#pragma endregion
#pragma region STACK STRING

	case OpCode::LEN: {
		Value v = pop();
		if (v.isArray())
		{
			push(Value(static_cast<i64>(v.asArray()->size())));
		}
		else
		{
			push(Value(static_cast<i64>(v.asString().length())));
		}
		break;
	}

	case OpCode::CHAR_AT: {
		Value idxVal = pop();
		Value strVal = pop();

		std::string s;
		if (strVal.isString())
			s = strVal.asString();
		else
			s = strVal.toString();

		i64 idx = 0;
		if (idxVal.isInt())
			idx = idxVal.asInt();
		else if (idxVal.isFloat())
			idx = static_cast<i64>(idxVal.asFloat());
		else if (idxVal.isString())
		{
			try
			{
				idx = std::stoll(idxVal.asString());
			}
			catch (...)
			{
				throw std::runtime_error("char_at() expects index convertible to integer");
			}
		}
		else
			throw std::runtime_error("char_at() expects string and integer");

		if (idx < 0 || idx >= static_cast<i64>(s.length()))
			push(Value(""));
		else
			push(Value(std::string(1, s[static_cast<size_t>(idx)])));
		break;
	}

	case OpCode::SUBSTR: {
		Value lenVal   = pop();
		Value startVal = pop();
		Value strVal   = pop();

		if (strVal.isString() && startVal.isInt() && lenVal.isInt())
		{
			const std::string &s     = strVal.asString();
			i64                start = startVal.asInt();
			i64                len   = lenVal.asInt();

			if (start < 0 || start >= static_cast<i64>(s.length()))
			{
				push(Value(""));
			}
			else
			{
				push(Value(s.substr(start, len)));
			}
		}
		else
		{
			throw std::runtime_error("substr() expects string, int, int");
		}
		break;
	}

#pragma endregion
#pragma region STACK STRUCT

	case OpCode::NEW_STRUCT_INSTANCE_STATIC: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for NEW_STRUCT_INSTANCE_STATIC");

		const StructInfo &info     = m_bytecode->structs[operand1];
		Value             instance = Value::createStruct(info.name);
		for (int i = 0; i < info.fieldCount; ++i)
		{
			int constIndex = info.firstConstIndex + i;
			if (constIndex < 0 || constIndex >= static_cast<int>(m_bytecode->constants.size()))
				throw std::runtime_error("Invalid default constant index for struct field");
			const Value       &defVal    = m_bytecode->constants[constIndex];
			const std::string &fieldName = info.fieldNames[i];
			instance.setField(fieldName, defVal);
		}
		push(instance);
		break;
	}

	case OpCode::GET_FIELD_STATIC: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for GET_FIELD_STATIC");
		const StructInfo &info        = m_bytecode->structs[operand1];
		int               fieldOffset = operand2;
		if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
			throw std::runtime_error("Invalid field offset for GET_FIELD_STATIC");
		const std::string &fieldName = info.fieldNames[fieldOffset];
		Value              obj       = pop();
		push(obj.getField(fieldName));
		break;
	}

	case OpCode::SET_FIELD_STATIC: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for SET_FIELD_STATIC");
		const StructInfo &info        = m_bytecode->structs[operand1];
		int               fieldOffset = operand2;
		if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
			throw std::runtime_error("Invalid field offset for SET_FIELD_STATIC");
		const std::string &fieldName = info.fieldNames[fieldOffset];
		Value              value     = pop();
		Value              obj       = pop();
		obj.setField(fieldName, value);
		push(obj);
		break;
	}

	case OpCode::NEW_STRUCT: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for NEW_STRUCT");
		Value       nameVal    = m_bytecode->constants[operand1];
		std::string structName = nameVal.asString();
		push(Value::createStruct(structName));
		break;
	}

	case OpCode::SET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for SET_FIELD");
		std::string fieldName = m_bytecode->constants[operand1].asString();
		Value       value     = pop();
		Value       obj       = pop();
		obj.setField(fieldName, value);
		push(obj);
		break;
	}

	case OpCode::GET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for GET_FIELD");
		std::string fieldName = m_bytecode->constants[operand1].asString();
		Value       obj       = pop();
		push(obj.getField(fieldName));
		break;
	}

#pragma endregion
#pragma region REGISTER CORE

	[[likely]] case OpCode::MOV: {
		registers[rA] = registers[rB];
		break;
	}

	[[likely]] case OpCode::LOAD_CONST_R: {
		int constIndex = operand2;
		if (constIndex < 0 || constIndex >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index");
		registers[rA] = m_bytecode->constants[constIndex];
		break;
	}

	[[likely]] case OpCode::LOAD_VAR_R: {
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		registers[rA] = variables[varIndex];
		break;
	}

	[[likely]] case OpCode::STORE_VAR_R: {
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		variables[varIndex] = registers[rA];
		break;
	}

	case OpCode::PUSH_R: {
		push(registers[rA]);
		break;
	}

	case OpCode::PUSH2_R: {
		push(registers[rA]);
		push(registers[rB]);
		break;
	}

	case OpCode::POP_R: {
		registers[rA] = pop();
		break;
	}

	case OpCode::POP2_R: {
		registers[rA] = pop();
		registers[rB] = pop();
		break;
	}

#pragma region REG ARITHMETIC

	case OpCode::IADD_R: {
		registers[rA] = Value(asm_iadd(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::ISUB_R: {
		registers[rA] = Value(asm_isub(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::IMUL_R: {
		registers[rA] = Value(asm_imul(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::IDIV_R: {
		registers[rA] = Value(asm_idiv(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::IMOD_R: {
		registers[rA] = Value(asm_imod(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::FLADD_R: {
		registers[rA] = Value(asm_fladd(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLSUB_R: {
		registers[rA] = Value(asm_flsub(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLMUL_R: {
		registers[rA] = Value(asm_flmul(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLDIV_R: {
		registers[rA] = Value(asm_fldiv(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLMOD_R: {
		registers[rA] = Value(asm_flmod(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::SQRT_R: {
		registers[rA] = Value(asm_sqrt(registers[rB].asFloat()));
		break;
	}

	case OpCode::POW_R: {
		registers[rA] = Value(asm_pow(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::LOG_R: {
		registers[rA] = Value(asm_log(registers[rB].asFloat()));
		break;
	}

	case OpCode::EXP_R: {
		registers[rA] = Value(asm_exp(registers[rB].asFloat()));
		break;
	}

	case OpCode::SIN_R: {
		registers[rA] = Value(asm_sin(registers[rB].asFloat()));
		break;
	}

	case OpCode::COS_R: {
		registers[rA] = Value(asm_cos(registers[rB].asFloat()));
		break;
	}

	case OpCode::TAN_R: {
		registers[rA] = Value(asm_tan(registers[rB].asFloat()));
		break;
	}

#pragma endregion
#pragma region REG LOGICAL

	case OpCode::NEG_R: {
		registers[rA] = Value(asm_flneg(registers[rB].asFloat()));
		break;
	}

	case OpCode::NOT_R: {
		registers[rA] = Value(asm_flnot(registers[rB].isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::IEQ_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iequal(b.asInt(), c.asInt())) : Value(b == c);
		break;
	}

	case OpCode::INE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_inot_equal(b.asInt(), c.asInt())) : Value(b != c);
		break;
	}

	case OpCode::ILT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iless_than(b.asInt(), c.asInt())) : Value(b < c);
		break;
	}

	case OpCode::IGT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_igreater_than(b.asInt(), c.asInt())) : Value(b > c);
		break;
	}

	case OpCode::ILE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iless_equal(b.asInt(), c.asInt())) : Value(b <= c);
		break;
	}

	case OpCode::IGE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_igreater_equal(b.asInt(), c.asInt())) : Value(b >= c);
		break;
	}

	case OpCode::IAND_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_iand(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::IOR_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_ior(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::FLEQ_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flequal(b.asFloat(), c.asFloat())) : Value(b == c);
		break;
	}

	case OpCode::FLNE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flnot_equal(b.asFloat(), c.asFloat())) : Value(b != c);
		break;
	}

	case OpCode::FLLT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flless_than(b.asFloat(), c.asFloat())) : Value(b < c);
		break;
	}

	case OpCode::FLGT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] =
		    ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flgreater_than(b.asFloat(), c.asFloat())) : Value(b > c);
		break;
	}

	case OpCode::FLLE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] =
		    ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flless_equal(b.asFloat(), c.asFloat())) : Value(b <= c);
		break;
	}

	case OpCode::FLGE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] =
		    ((b.isFloat() || b.isInt()) && (c.isFloat() || c.isInt())) ? Value(asm_flgreater_equal(b.asFloat(), c.asFloat())) : Value(b >= c);
		break;
	}

	case OpCode::FLAND_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_fland(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::FLOR_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_flor(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

#pragma endregion
#pragma region REG I/O

	case OpCode::PRINT_R: {
		std::string s = registers[rA].toString();
#ifdef TRACING
		log(std::format("PRINT_R: (stdout) {:T}\n", registers[rA]));
#else
		c_print_stdout(s.c_str(), (i64)s.length());
#endif
		flush();
		break;
	}

	[[unlikely]] case OpCode::PRINTERROR_R: {
		std::string s = registers[rA].toString();
#ifdef TRACING
		log(std::format("PRINTERROR_R: (stderr) {:T}\n", registers[rA]));
#else
		c_print_stderr(s.c_str(), (i64)s.length());
#endif
		flusherr();
		break;
	}

	case OpCode::READLINE_R: {
		std::string s;
#ifdef TRACING
		log("READLINE_R:");
		flush();
#endif
		std::getline(std::cin, s);
#ifdef TRACING
		log(std::format("\nREADLINE_R: {}\n", s));
#endif
		registers[rA] = s;
		break;
	}

#pragma endregion
#pragma region REG SYSTEM

	case OpCode::SYSTEM_R: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		registers[rA] = Value();
#else
#ifdef TRACING
		Value   cmd = registers[rA];
		i64 ret = c_system(cmd.c_str());
		log(std::format("SYSTEM_R: {} -> {}\n", cmd, ret));
		registers[rA] = ret;
#else
		registers[rA] = c_system(registers[rA].c_str());
#endif
#endif
		break;
	}

	case OpCode::SYSTEM_OUT_R: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		registers[rA] = Value();
#else
#ifdef TRACING
		Value       cmd = registers[rA];
		std::string ret = c_system_out(cmd.c_str());
		log(std::format("SYSTEM_R: {:T} -> {}\n", cmd, ret));
		registers[rA] = ret;
#else
		registers[rA] = c_system_out(registers[rA].c_str());
#endif
#endif
		break;
	}

	case OpCode::SYSTEM_ERR_R: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		registers[rA] = Value();
#else
#ifdef TRACING
		Value       cmd = registers[rA];
		std::string ret = c_system_err(cmd.c_str());
		log(std::format("SYSTEM_ERR_R: {:T} -> {}\n", cmd, ret));
		registers[rA] = ret;
#else
		registers[rA] = c_system_err(registers[rA].c_str());
#endif
#endif
		break;
	}

	case OpCode::EXIT_SCOPE: 
	{
		int scopeId = operand1;
		for (const auto &[idx, name] : m_bytecode->scopeVarLists[scopeId])
			freeVariable(static_cast<size_t>(idx));
		break;
	}

#pragma endregion

#pragma endregion
#pragma region DEFAULT
	default: {
		throw std::runtime_error("Unknown opcode");
		return Value();
	}
#pragma endregion
	
	}
	return Value(operand1);
}

} // namespace Phasor