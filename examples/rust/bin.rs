use phasorrt_rs::{get_version, compile_phs, PhasorVM};
use std::fs;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Phasor Version: {}", get_version()?);

    let mut vm = PhasorVM::new()?;
    vm.init_stdlib()?;

    let script = "
using(\"stdsys\");
fn main() -> int {
    using(\"stdio\", \"stdfile\");
    puts(fcd());
    putf(\"Hello, World! %d + %d = %d\", 15, 22, 15 + 22);
    return 15 + 22;
}
shutdown(main());";

    println!("script = \"{}\"", script);

    let bc = match compile_phs(script, "compile_phs Test", None) {
        Ok(bytecode) => {
            println!("Script compiled successfully, bytecode size: {}", bytecode.len());
            bytecode
        }
        Err(err) => {
            eprintln!("Compilation failed: {:?}", err);
            return Err(err.into());
        }
    };

    match fs::write("script.phsb", &bc) {
        Ok(_) => println!("Bytecode written to ./script.phsb"),
        Err(err) => {
            eprintln!("Failed to write bytecode file: {}", err);
            return Err(err.into());
        }
    }

    let new_bytecode = match fs::read("script.phsb") {
        Ok(bytecode) => {
            println!("Read bytecode from ./script.phsb");
            bytecode
        }
        Err(err) => {
            eprintln!("Failed to read bytecode file: {}", err);
            return Err(err.into());
        }
    };

    // args are command line, i might add function args in a future C API version
    match vm.exec_func_int(&new_bytecode, "exec_func_int Test", "main", &[]) {
        Ok(code) => {
            if code == 15 + 22 {
                println!("exec_func_int executed successfully");
            } else {
                println!("Function returned code: {}", code);
            }
        }

        Err(err) => {
            eprintln!("VM execution failed: {:?}", err);
            return Err(err.into());
        }
    }

    match vm.evaluate_phs(script, "evaluate_phs Test", None, false) {
        Ok(code) => {
            if code == 15 + 22 {
                println!("evaluate_phs executed successfully");
            } else {
                println!("evaluate_phs returned code: {}", code);
            }
        }

        Err(err) => {
            eprintln!("VM execution failed: {:?}", err);
        }
    }

    Ok(())
}
