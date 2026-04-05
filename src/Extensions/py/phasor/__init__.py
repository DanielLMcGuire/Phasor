"""
phasor
======
Python module for reading, writing, manipulating, and executing Phasor VM bytecode.

-----------
Load a ``.phsb`` file::

    from phasor import Bytecode
    bc = Bytecode.load("program.phsb")
    print(bc.disassemble())

Round-trip a bytecode object::

    bc.save("copy.phsb")
    bc2 = Bytecode.from_bytes(bc.to_bytes())

Extract bytecode from a compiled native binary (requires ``lief``)::

    bc = Bytecode.from_native_binary("program")

Build bytecode programmatically::

    from phasor import Bytecode, Value, OpCode

    bc = Bytecode()
    ci = bc.add_constant(Value.from_int(42))
    bc.emit(OpCode.PUSH_CONST, ci)
    bc.emit(OpCode.HALT)
    bc.save("hello.phsb")

Compile and run via the libphasorrt library::

    from phasor import new_state, free_state, evaluate_phs, compile_phs, run

    evaluate_phs('print("hello");')

    vm = new_state()
    evaluate_phs('var x = 42;, state=vm)
    evaluate_phs('print(x);',   state=vm)
    free_state(vm)

    bytecode = compile_phs('print("hello");')
    run(bytecode)

    evaluate_phs_file("scripts/hello.phs", state=vm)
    run_file("scripts/hello.phsb")
"""

from .Bytecode      import Bytecode
from .Deserializer  import BytecodeDeserializer
from .Instruction   import Instruction
from .Metadata      import MAGIC, VERSION
from .Native        import extract_phsb_bytes
from .OpCode        import OpCode
from .Runtime       import (
    new_state,
    free_state,
    reset_state,
    compile_phs,
    compile_phs_file,
    compile_pul,
    compile_pul_file,
    run,
    run_file,
    evaluate_phs,
    evaluate_phs_file,
    evaluate_pul,
    evaluate_pul_file,
)
from .Serializer    import BytecodeSerializer
from .Value         import Value, ValueType

__all__ = [
    "Bytecode",
    "BytecodeDeserializer",
    "BytecodeSerializer",
    "Instruction",
    "OpCode",
    "Value",
    "ValueType",
    "extract_phsb_bytes",
    "MAGIC",
    "VERSION",
    "new_state",
    "free_state",
    "reset_state",
    "compile_phs",
    "compile_phs_file",
    "compile_pul",
    "compile_pul_file",
    "run",
    "run_file",
    "evaluate_phs",
    "evaluate_phs_file",
    "evaluate_pul",
    "evaluate_pul_file",
]
