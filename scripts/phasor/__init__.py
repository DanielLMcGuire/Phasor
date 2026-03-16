"""
phasor
======
Python module for reading, writing, and manipulating Phasor VM bytecode.

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
"""

from .bytecode      import Bytecode
from .deserializer  import BytecodeDeserializer
from .instructions  import Instruction
from .metadata      import MAGIC, VERSION
from .native        import extract_phsb_bytes
from .opcodes       import OpCode
from .serializer    import BytecodeSerializer
from .value        import Value, ValueType

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
]
