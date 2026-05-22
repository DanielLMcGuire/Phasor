use libc::c_int;
use libloading::{Library, Symbol};
use std::ffi::{CStr, CString};

use crate::error::PhasorError;
use crate::ffi::*;

pub struct PhasorVM {
    state: *mut std::ffi::c_void,
    _lib: Library,
}

impl PhasorVM {
    pub fn new() -> Result<Self, PhasorError> {
        let dll_path =
            std::env::var("PHASOR_DLL_PATH").unwrap_or_else(|_| "phasorrt.dll".to_string());

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

    unsafe fn get_stdlib_init(&self) -> Symbol<'_, InitStdLibFn> {
        self._lib.get(b"initStdLib").unwrap()
    }

    unsafe fn get_reset_state(&self) -> Symbol<'_, ResetStateFn> {
        self._lib.get(b"resetState").unwrap()
    }

    unsafe fn get_exec(&self) -> Symbol<'_, ExecFn> {
        self._lib.get(b"exec").unwrap()
    }

    unsafe fn get_exec_func_int(&self) -> Symbol<'_, ExecFuncIntFn> {
        self._lib.get(b"execFuncInt").unwrap()
    }

    unsafe fn get_exec_func_string(&self) -> Symbol<'_, ExecFuncStringFn> {
        self._lib.get(b"execFuncString").unwrap()
    }

    unsafe fn get_evaluate_phs(&self) -> Symbol<'_, EvaluatePHSFn> {
        self._lib.get(b"evaluatePHS").unwrap()
    }

    unsafe fn get_evaluate_pul(&self) -> Symbol<'_, EvaluatePULFn> {
        self._lib.get(b"evaluatePUL").unwrap()
    }

    unsafe fn get_is_error_status(&self) -> Symbol<'_, IsErrorStatusFn> {
        self._lib.get(b"isErrorStatus").unwrap()
    }

    pub fn init_stdlib(&mut self) -> Result<(), PhasorError> {
        unsafe {
            self.get_stdlib_init()(self.state);
        }
        Ok(())
    }

    pub fn reset(
        &mut self,
        reset_functions: bool,
        reset_variables: bool,
    ) -> Result<bool, PhasorError> {
        unsafe {
            Ok(self.get_reset_state()(
                self.state,
                reset_functions,
                reset_variables,
            ))
        }
    }

    pub fn exec(
        &mut self,
        bytecode: &[u8],
        module: &str,
        args: &[&str],
    ) -> Result<i32, PhasorError> {
        let c_module = CString::new(module)?;
        let c_args: Vec<CString> = args.iter().map(|&s| CString::new(s).unwrap()).collect();
        let arg_ptrs: Vec<*const libc::c_char> = c_args.iter().map(|s| s.as_ptr()).collect();

        unsafe {
            let res = self.get_exec()(
                self.state,
                bytecode.as_ptr(),
                bytecode.len(),
                c_module.as_ptr(),
                arg_ptrs.len() as c_int,
                arg_ptrs.as_ptr(),
            );
            if self.get_is_error_status()(self.state) {
                Err(PhasorError::ExecutionException(res))
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
        let arg_ptrs: Vec<*const libc::c_char> = c_args.iter().map(|s| s.as_ptr()).collect();

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
            if self.get_is_error_status()(self.state) {
                Err(PhasorError::ExecutionException(res))
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
        let arg_ptrs: Vec<*const libc::c_char> = c_args.iter().map(|s| s.as_ptr()).collect();

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

        unsafe {
            let res = self.get_evaluate_phs()(
                self.state,
                c_script.as_ptr(),
                c_module.as_ptr(),
                c_path.as_ptr(),
                verbose,
            );
            if self.get_is_error_status()(self.state) {
                Err(PhasorError::ExecutionException(res))
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
            if self.get_is_error_status()(self.state) {
                Err(PhasorError::ExecutionException(res))
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