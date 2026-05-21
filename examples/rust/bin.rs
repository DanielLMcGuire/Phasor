use phasorrt_rs::{get_version, PhasorError, PhasorVM, compile_phs};

fn main() -> Result<(), PhasorError> {
    println!("Phasor Version: {}", get_version()?);

    let mut vm = PhasorVM::new()?;
    vm.init_stdlib()?;

    let script = "using(\"stdsys\");
                        fn main() -> int {
                            using(\"stdio\");
                            puts(\"Hello, World!\");
                            return 0;
                        }
                        shutdown(main());";
    let bc: Vec<u8> = compile_phs(script, "Hello World Script", Some(""))?;

    // args are command line, i might add function args in a future C API version
    match vm.exec_func_int(&bc, "exec_func_int Test", "main", &[]) {
        Ok(code) => {
            if code == 0 {
                println!("exec_func_int executed successfully");
            } else {
                println!("Function returned code: {}", code);
            }
        }

        Err(err) => {
            eprintln!("VM execution failed: {:?}", err);
        }
    }

    match vm.evaluate_phs(script, "evaluate_phs Test", None, false) {
        Ok(code) => {
            if code == 0 {
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
