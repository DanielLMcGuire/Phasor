from phasor import Bytecode, Value, OpCode

bc = Bytecode()

# print("Hello, World!\n")
bc.emit(OpCode.PUSH_CONST, bc.add_constant(Value.from_string("Hello, World!\n")))
bc.emit(OpCode.PRINT)

# print(14 + 21)
bc.emit(OpCode.PUSH_CONST, bc.add_constant(Value.from_int(14)))
bc.emit(OpCode.PUSH_CONST, bc.add_constant(Value.from_int(21))) 
bc.emit(OpCode.POP2_R, 1, 0)
bc.emit(OpCode.IADD_R, 0, 1, 0)
bc.emit(OpCode.PRINT_R, 0)

bc.emit(OpCode.HALT)
bc.save("test.phsb")