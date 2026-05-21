use phasorrt_rs::{get_version, PhasorError, PhasorVM, compile_phs};

fn main() -> Result<(), PhasorError> {
    println!("Phasor Version: {}", get_version()?);

    let mut vm = PhasorVM::new()?;
    vm.init_stdlib()?;

    let script = "fn main() -> int { using(\"stdio\", \"stdsys\"); puts(\"Hello, World!\"); return 0;} shutdown(main());";
    let bc: Vec<u8> = compile_phs(script, "rusttest", Some(""))?;

    // args are command line, i might add function args in a future C API version
    match vm.exec_func_int(&bc, "rusttest", "main", &[]) {
        Ok(code) => {
            if code == 0 {
                println!("exec_func_int executed successfully!");
            } else {
                println!("Function returned code: {}", code);
            }
        }

        Err(err) => {
            eprintln!("VM execution failed: {:?}", err);
        }
    }

    Ok(())
}
