pub mod error;
pub mod ffi;
pub mod vm;

pub use error::PhasorError;
pub use vm::PhasorVM;

#[cfg(feature = "dynamic")]
use libloading::{Library, Symbol};

use std::ffi::{CStr, CString};
use std::ptr;

#[cfg(feature = "dynamic")]
fn dll_path() -> String {
    std::env::var("PHASOR_DLL_PATH").unwrap_or_else(|_| "phasorrt.dll".to_string())
}

pub fn get_version() -> Result<String, PhasorError> {
    #[cfg(feature = "dynamic")]
    unsafe {
        let lib = Library::new(dll_path())?;
        let get_version: Symbol<ffi::GetVersionFn> = lib.get(b"getVersion")?;
        let v = get_version();
        if v.is_null() {
            Ok("unknown".to_string())
        } else {
            Ok(CStr::from_ptr(v).to_string_lossy().into_owned())
        }
    }

    #[cfg(not(feature = "dynamic"))]
    unsafe {
        let v = ffi::getVersion();
        if v.is_null() {
            Ok("unknown".to_string())
        } else {
            Ok(CStr::from_ptr(v).to_string_lossy().into_owned())
        }
    }
}

pub fn compile_phs(script: &str, module: &str, path: Option<&str>) -> Result<Vec<u8>, PhasorError> {
    let c_script = CString::new(script)?;
    let c_module = CString::new(module)?;
    let c_path = path.map(CString::new).transpose()?;
    let path_ptr = c_path.as_ref().map_or(ptr::null(), |p| p.as_ptr());

    #[cfg(feature = "dynamic")]
    unsafe {
        let lib = Library::new(dll_path())?;
        let compile_phs: Symbol<ffi::CompilePHSFn> = lib.get(b"compilePHS")?;

        let mut out_size: libc::size_t = 0;
        if !compile_phs(c_script.as_ptr(), c_module.as_ptr(), path_ptr, ptr::null_mut(), 0, &mut out_size) {
            return Err(PhasorError::CompileError);
        }
        let mut buffer = vec![0u8; out_size];
        if compile_phs(c_script.as_ptr(), c_module.as_ptr(), path_ptr, buffer.as_mut_ptr(), buffer.len(), &mut out_size) {
            Ok(buffer)
        } else {
            Err(PhasorError::CompileError)
        }
    }

    #[cfg(not(feature = "dynamic"))]
    unsafe {
        let mut out_size: libc::size_t = 0;
        if !ffi::compilePHS(c_script.as_ptr(), c_module.as_ptr(), path_ptr, ptr::null_mut(), 0, &mut out_size) {
            return Err(PhasorError::CompileError);
        }
        let mut buffer = vec![0u8; out_size];
        if ffi::compilePHS(c_script.as_ptr(), c_module.as_ptr(), path_ptr, buffer.as_mut_ptr(), buffer.len(), &mut out_size) {
            Ok(buffer)
        } else {
            Err(PhasorError::CompileError)
        }
    }
}

pub fn compile_pul(script: &str, module: &str) -> Result<Vec<u8>, PhasorError> {
    let c_script = CString::new(script)?;
    let c_module = CString::new(module)?;

    #[cfg(feature = "dynamic")]
    unsafe {
        let lib = Library::new(dll_path())?;
        let compile_pul: Symbol<ffi::CompilePULFn> = lib.get(b"compilePUL")?;

        let mut out_size: libc::size_t = 0;
        if !compile_pul(c_script.as_ptr(), c_module.as_ptr(), ptr::null_mut(), 0, &mut out_size) {
            return Err(PhasorError::CompileError);
        }
        let mut buffer = vec![0u8; out_size];
        if compile_pul(c_script.as_ptr(), c_module.as_ptr(), buffer.as_mut_ptr(), buffer.len(), &mut out_size) {
            Ok(buffer)
        } else {
            Err(PhasorError::CompileError)
        }
    }

    #[cfg(not(feature = "dynamic"))]
    unsafe {
        let mut out_size: libc::size_t = 0;
        if !ffi::compilePUL(c_script.as_ptr(), c_module.as_ptr(), ptr::null_mut(), 0, &mut out_size) {
            return Err(PhasorError::CompileError);
        }
        let mut buffer = vec![0u8; out_size];
        if ffi::compilePUL(c_script.as_ptr(), c_module.as_ptr(), buffer.as_mut_ptr(), buffer.len(), &mut out_size) {
            Ok(buffer)
        } else {
            Err(PhasorError::CompileError)
        }
    }
}