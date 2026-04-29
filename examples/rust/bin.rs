use phasorrt_rs::{get_version, PhasorError, PhasorVM};

fn main() -> Result<(), PhasorError> {
    println!("Phasor Version: {}", get_version()?);

    let mut vm = PhasorVM::new()?;
    vm.init_stdlib()?;

    let script = "print(\"Hello, World!\\n\");";
    if let Err(e) = vm.evaluate_phs(script, "rusttest", Some(""), false) {
        eprintln!("Failed to run script: {}", e);
    }

    Ok(())
}
