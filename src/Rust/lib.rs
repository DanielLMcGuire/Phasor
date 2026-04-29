use libc::{c_char, c_int, c_uchar, c_void, size_t};
use libloading::{Library, Symbol};
use std::ffi::{CStr, CString, NulError};
use std::ptr;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum PhasorError {
    #[error("Null byte found in input string")]
    StringError(#[from] NulError),

    #[error("Compilation failed")]
    CompileError,

    #[error("Execution exception (Code: {0})")]
    ExecutionException(i32),

    #[error("Function returned null pointer")]
    NullReturn,

    #[error("UTF-8 encoding error")]
    Utf8Error(#[from] std::str::Utf8Error),

    #[error("Failed to load or query Phasor DLL: {0}")]
    LibraryError(#[from] libloading::Error),
}

type GetVersionFn = unsafe extern "C" fn() -> *const c_char;
type ExecFn = unsafe extern "C" fn(
    state: *mut c_void,
    bytecode: *const c_uchar,
    bytecodeSize: size_t,
    moduleName: *const c_char,
    argc: c_int,
    argv: *const *const c_char,
) -> c_int;
type ExecFuncIntFn = unsafe extern "C" fn(
    state: *mut c_void,
    bytecode: *const c_uchar,
    bytecodeSize: size_t,
    moduleName: *const c_char,
    argc: c_int,
    argv: *const *const c_char,
    functionName: *const c_char,
) -> c_int;
type ExecFuncStringFn = unsafe extern "C" fn(
    state: *mut c_void,
    bytecode: *const c_uchar,
    bytecodeSize: size_t,
    moduleName: *const c_char,
    argc: c_int,
    argv: *const *const c_char,
    functionName: *const c_char,
) -> *const c_char;
type EvaluatePHSFn = unsafe extern "C" fn(
    state: *mut c_void,
    script: *const c_char,
    moduleName: *const c_char,
    modulePath: *const c_char,
    verbose: bool,
) -> c_int;
type EvaluatePULFn = unsafe extern "C" fn(
    state: *mut c_void,
    script: *const c_char,
    moduleName: *const c_char,
) -> c_int;
type CompilePHSFn = unsafe extern "C" fn(
    script: *const c_char,
    moduleName: *const c_char,
    modulePath: *const c_char,
    buffer: *mut c_uchar,
    bufferSize: size_t,
    outSize: *mut size_t,
) -> bool;
type CompilePULFn = unsafe extern "C" fn(
    script: *const c_char,
    moduleName: *const c_char,
    buffer: *mut c_uchar,
    bufferSize: size_t,
    outSize: *mut size_t,
) -> bool;
type CreateStateFn = unsafe extern "C" fn() -> *mut c_void;
type InitStdLibFn = unsafe extern "C" fn(state: *mut c_void);
type FreeStateFn = unsafe extern "C" fn(state: *mut c_void) -> bool;
type ResetStateFn = unsafe extern "C" fn(state: *mut c_void, resetFunctions: bool, resetVariables: bool) -> bool;

pub struct PhasorVM {
    state: *mut c_void,
    _lib: Library, // keep library loaded for the lifetime of the VM
}

impl PhasorVM {
    pub fn new() -> Result<Self, PhasorError> {
        let dll_path = std::env::var("PHASOR_DLL_PATH")
            .unwrap_or_else(|_| "phasorrt.dll".to_string());

        unsafe {
            let lib = Library::new(&dll_path)?;
            let create_state: Symbol<CreateStateFn> = lib.get(b"createState")?;
            let state = create_state();
            if state.is_null() {
                panic!("Failed to create Phasor state instance.");
            }
            Ok(PhasorVM { state, _lib: lib })
        }
    }

    unsafe fn get_stdlib_init(&self) -> Symbol<InitStdLibFn> {
        self._lib.get(b"initStdLib").unwrap()
    }

    unsafe fn get_reset_state(&self) -> Symbol<ResetStateFn> {
        self._lib.get(b"resetState").unwrap()
    }

    unsafe fn get_exec(&self) -> Symbol<ExecFn> {
        self._lib.get(b"exec").unwrap()
    }

    unsafe fn get_exec_func_int(&self) -> Symbol<ExecFuncIntFn> {
        self._lib.get(b"execFuncInt").unwrap()
    }

    unsafe fn get_exec_func_string(&self) -> Symbol<ExecFuncStringFn> {
        self._lib.get(b"execFuncString").unwrap()
    }

    unsafe fn get_evaluate_phs(&self) -> Symbol<EvaluatePHSFn> {
        self._lib.get(b"evaluatePHS").unwrap()
    }

    unsafe fn get_evaluate_pul(&self) -> Symbol<EvaluatePULFn> {
        self._lib.get(b"evaluatePUL").unwrap()
    }

    pub fn init_stdlib(&mut self) -> Result<(), PhasorError> {
        unsafe {
            self.get_stdlib_init()(self.state);
        }
        Ok(())
    }

    pub fn reset(&mut self, reset_functions: bool, reset_variables: bool) -> Result<bool, PhasorError> {
        unsafe { Ok(self.get_reset_state()(self.state, reset_functions, reset_variables)) }
    }

    pub fn exec(&mut self, bytecode: &[u8], module: &str, args: &[&str]) -> Result<i32, PhasorError> {
        let c_module = CString::new(module)?;
        let c_args: Vec<CString> = args.iter().map(|&s| CString::new(s).unwrap()).collect();
        let arg_ptrs: Vec<*const c_char> = c_args.iter().map(|s| s.as_ptr()).collect();

        unsafe {
            let res = self.get_exec()(
                self.state,
                bytecode.as_ptr(),
                bytecode.len(),
                c_module.as_ptr(),
                arg_ptrs.len() as c_int,
                arg_ptrs.as_ptr(),
            );
            if res == -1 {
                Err(PhasorError::ExecutionException(-1))
            } else {
                Ok(res)
            }
        }
    }

    pub fn exec_func_int(
        &mut self,
        bytecode: &[u8],
        module: &str,
        function: &str,
        args: &[&str],
    ) -> Result<i32, PhasorError> {
        let c_module = CString::new(module)?;
        let c_function = CString::new(function)?;
        let c_args: Vec<CString> = args.iter().map(|&s| CString::new(s).unwrap()).collect();
        let arg_ptrs: Vec<*const c_char> = c_args.iter().map(|s| s.as_ptr()).collect();

        unsafe {
            let res = self.get_exec_func_int()(
                self.state,
                bytecode.as_ptr(),
                bytecode.len(),
                c_module.as_ptr(),
                arg_ptrs.len() as c_int,
                arg_ptrs.as_ptr(),
                c_function.as_ptr(),
            );
            if res == -1 {
                Err(PhasorError::ExecutionException(-1))
            } else {
                Ok(res)
            }
        }
    }

    pub fn exec_func_string(
        &mut self,
        bytecode: &[u8],
        module: &str,
        function: &str,
        args: &[&str],
    ) -> Result<String, PhasorError> {
        let c_module = CString::new(module)?;
        let c_function = CString::new(function)?;
        let c_args: Vec<CString> = args.iter().map(|&s| CString::new(s).unwrap()).collect();
        let arg_ptrs: Vec<*const c_char> = c_args.iter().map(|s| s.as_ptr()).collect();

        unsafe {
            let res_ptr = self.get_exec_func_string()(
                self.state,
                bytecode.as_ptr(),
                bytecode.len(),
                c_module.as_ptr(),
                arg_ptrs.len() as c_int,
                arg_ptrs.as_ptr(),
                c_function.as_ptr(),
            );
            if res_ptr.is_null() {
                Err(PhasorError::NullReturn)
            } else {
                Ok(CStr::from_ptr(res_ptr).to_str()?.to_owned())
            }
        }
    }

    pub fn evaluate_phs(
        &mut self,
        script: &str,
        module: &str,
        path: Option<&str>,
        verbose: bool,
    ) -> Result<i32, PhasorError> {
        let c_script = CString::new(script)?;
        let c_module = CString::new(module)?;
        let c_path = match path {
            Some(p) => CString::new(p)?,
            None => CString::new("")?,
        };
        let path_ptr = c_path.as_ptr();

        unsafe {
            let res = self.get_evaluate_phs()(self.state, c_script.as_ptr(), c_module.as_ptr(), path_ptr, verbose);
            if res == -1 {
                Err(PhasorError::ExecutionException(-1))
            } else {
                Ok(res)
            }
        }
    }

    pub fn evaluate_pul(&mut self, script: &str, module: &str) -> Result<i32, PhasorError> {
        let c_script = CString::new(script)?;
        let c_module = CString::new(module)?;

        unsafe {
            let res = self.get_evaluate_pul()(self.state, c_script.as_ptr(), c_module.as_ptr());
            if res == -1 {
                Err(PhasorError::ExecutionException(-1))
            } else {
                Ok(res)
            }
        }
    }
}

impl Drop for PhasorVM {
    fn drop(&mut self) {
        unsafe {
            if let Ok(free) = self._lib.get::<FreeStateFn>(b"freeState") {
                free(self.state);
            }
        }
    }
}

pub fn get_version() -> Result<String, PhasorError> {
    unsafe {
        let lib = Library::new(
            std::env::var("PHASOR_DLL_PATH")
                .unwrap_or_else(|_| "phasorrt.dll".to_string())
        )?;
        let get_version: Symbol<GetVersionFn> = lib.get(b"getVersion")?;
        let v = get_version();
        if v.is_null() {
            Ok("unknown".to_string())
        } else {
            Ok(CStr::from_ptr(v).to_string_lossy().into_owned())
        }
    }
}

pub fn compile_phs(script: &str, module: &str, path: Option<&str>) -> Result<Vec<u8>, PhasorError> {
    unsafe {
        let lib = Library::new(
            std::env::var("PHASOR_DLL_PATH")
                .unwrap_or_else(|_| "phasorrt.dll".to_string())
        )?;
        let compile_phs: Symbol<CompilePHSFn> = lib.get(b"compilePHS")?;

        let c_script = CString::new(script)?;
        let c_module = CString::new(module)?;
        let c_path = path.map(CString::new).transpose()?;
        let path_ptr = c_path.as_ref().map_or(ptr::null(), |p| p.as_ptr());

        let mut out_size: size_t = 0;
        let success = compile_phs(c_script.as_ptr(), c_module.as_ptr(), path_ptr, ptr::null_mut(), 0, &mut out_size);
        if !success {
            return Err(PhasorError::CompileError);
        }

        let mut buffer = vec![0u8; out_size];
        let final_success = compile_phs(
            c_script.as_ptr(),
            c_module.as_ptr(),
            path_ptr,
            buffer.as_mut_ptr(),
            buffer.len(),
            &mut out_size,
        );
        if final_success {
            Ok(buffer)
        } else {
            Err(PhasorError::CompileError)
        }
    }
}

pub fn compile_pul(script: &str, module: &str) -> Result<Vec<u8>, PhasorError> {
    unsafe {
        let lib = Library::new(
            std::env::var("PHASOR_DLL_PATH")
                .unwrap_or_else(|_| "phasorrt.dll".to_string())
        )?;
        let compile_pul: Symbol<CompilePULFn> = lib.get(b"compilePUL")?;

        let c_script = CString::new(script)?;
        let c_module = CString::new(module)?;

        let mut out_size: size_t = 0;
        let success = compile_pul(c_script.as_ptr(), c_module.as_ptr(), ptr::null_mut(), 0, &mut out_size);
        if !success {
            return Err(PhasorError::CompileError);
        }

        let mut buffer = vec![0u8; out_size];
        let final_success = compile_pul(
            c_script.as_ptr(),
            c_module.as_ptr(),
            buffer.as_mut_ptr(),
            buffer.len(),
            &mut out_size,
        );
        if final_success {
            Ok(buffer)
        } else {
            Err(PhasorError::CompileError)
        }
    }
}