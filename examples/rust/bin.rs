use phasorrt_rs::{get_version, compile_phs, PhasorVM};
use std::process::Command;
use std::path::Path;
use std::fs;

fn get_phasorvm_path() -> &'static Path {
    if cfg!(target_os = "windows") {
        Path::new("out\\bin\\")
    } else if cfg!(target_os = "linux") {
        Path::new("out/usr/bin/")
    } else if cfg!(target_os = "macos") {
        Path::new("out/usr/local/bin/")
    } else {
        Path::new("out/usr/bin")
    }
}

fn run_disassembly_command(bytecode_path: &Path) -> Result<(), Box<dyn std::error::Error>> {
    let phasorvm_exe = get_phasorvm_path().join(Path::new("phasordecomp"));

    let output = Command::new(phasorvm_exe)
        .arg(bytecode_path)
        .output()?;

    if output.status.success() {
        println!("Disassembly Complete");
    } else {
        eprintln!("Disassembly failed: {}", String::from_utf8_lossy(&output.stderr));
    }

    let disassembly_path = bytecode_path.with_extension("phir");

    match fs::read_to_string(&disassembly_path) {
        Ok(disassembly) => {
            println!("Disassembly Output:\n{}", disassembly);
        }
        Err(err) => {
            eprintln!("Failed to read disassembly file: {}", err);
        }
    }

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Phasor Version: {}", get_version()?);

    let mut vm = PhasorVM::new()?;
    vm.init_stdlib()?;

    let script = "
using(\"stdsys\");
fn main() -> int {
    using(\"stdio\", \"stdfile\");
    puts(fcd());
    var x = 15;
    var y = 22;
    var z = x + y;
    putf(\"Hello, World! %d + %d = %d\", x, y, z);
    return z;
}
shutdown(main());";

    println!("script = \"{}\"", script);
    let expected_code = 15 + 22;

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
            if code == expected_code {
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
            if code == expected_code {
                println!("evaluate_phs executed successfully");
            } else {
                println!("evaluate_phs returned code: {}", code);
            }
        }

        Err(err) => {
            eprintln!("VM execution failed: {:?}", err);
        }
    }

    let phasorvm_exe = get_phasorvm_path().join(Path::new("phasorvm"));

    match Command::new(phasorvm_exe).arg("script.phsb").status() {
        Ok(status) => {
            if status.code() == Some(expected_code) {
                println!("PhasorVM executed successfully");
            } else {
                eprintln!("PhasorVM exited with status: {}", status);
            }
        }
        Err(err) => {
            eprintln!("Failed to execute PhasorVM: {}", err);
        }
    }

    run_disassembly_command(&Path::new("script.phsb"))?;

    Ok(())
}
