use libc::c_int;
use std::ffi::{CStr, CString};

#[cfg(feature = "dynamic")]
use libloading::{Library, Symbol};

use crate::error::PhasorError;
use crate::ffi::*;

pub struct PhasorVM {
    state: *mut std::ffi::c_void,
    #[cfg(feature = "dynamic")]
    _lib: Library,
}

impl PhasorVM {
    pub fn new() -> Result<Self, PhasorError> {
        #[cfg(feature = "dynamic")]
        {
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

        #[cfg(not(feature = "dynamic"))]
        unsafe {
            let state = createState();
            if state.is_null() {
                panic!("Failed to create Phasor state instance.");
            }
            Ok(PhasorVM { state })
        }
    }

    unsafe fn is_error_status(&self) -> Result<bool, PhasorError> { unsafe {
        #[cfg(feature = "dynamic")]
        {
            let f: Symbol<IsErrorStatusFn> = self._lib.get(b"isErrorStatus")?;
            Ok(f(self.state))
        }
        #[cfg(not(feature = "dynamic"))]
        Ok(isErrorStatus(self.state))
    }}

    pub fn init_stdlib(&mut self) -> Result<(), PhasorError> {
        unsafe {
            #[cfg(feature = "dynamic")]
            {
                let f: Symbol<InitStdLibFn> = self._lib.get(b"initStdLib")?;
                f(self.state);
            }
            #[cfg(not(feature = "dynamic"))]
            initStdLib(self.state);
        }
        Ok(())
    }

    pub fn reset(
        &mut self,
        reset_functions: bool,
        reset_variables: bool,
    ) -> Result<bool, PhasorError> {
        unsafe {
            #[cfg(feature = "dynamic")]
            {
                let f: Symbol<ResetStateFn> = self._lib.get(b"resetState")?;
                Ok(f(self.state, reset_functions, reset_variables))
            }
            #[cfg(not(feature = "dynamic"))]
            Ok(resetState(self.state, reset_functions, reset_variables))
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
            #[cfg(feature = "dynamic")]
            let res = {
                let f: Symbol<ExecFn> = self._lib.get(b"exec")?;
                f(
                    self.state,
                    bytecode.as_ptr(),
                    bytecode.len(),
                    c_module.as_ptr(),
                    arg_ptrs.len() as c_int,
                    arg_ptrs.as_ptr(),
                )
            };
            #[cfg(not(feature = "dynamic"))]
            let res = exec(
                self.state,
                bytecode.as_ptr(),
                bytecode.len(),
                c_module.as_ptr(),
                arg_ptrs.len() as c_int,
                arg_ptrs.as_ptr(),
            );

            if self.is_error_status()? {
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
            #[cfg(feature = "dynamic")]
            let res = {
                let f: Symbol<ExecFuncIntFn> = self._lib.get(b"execFuncInt")?;
                f(
                    self.state,
                    bytecode.as_ptr(),
                    bytecode.len(),
                    c_module.as_ptr(),
                    arg_ptrs.len() as c_int,
                    arg_ptrs.as_ptr(),
                    c_function.as_ptr(),
                )
            };
            #[cfg(not(feature = "dynamic"))]
            let res = execFuncInt(
                self.state,
                bytecode.as_ptr(),
                bytecode.len(),
                c_module.as_ptr(),
                arg_ptrs.len() as c_int,
                arg_ptrs.as_ptr(),
                c_function.as_ptr(),
            );

            if self.is_error_status()? {
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
            #[cfg(feature = "dynamic")]
            let res_ptr = {
                let f: Symbol<ExecFuncStringFn> = self._lib.get(b"execFuncString")?;
                f(
                    self.state,
                    bytecode.as_ptr(),
                    bytecode.len(),
                    c_module.as_ptr(),
                    arg_ptrs.len() as c_int,
                    arg_ptrs.as_ptr(),
                    c_function.as_ptr(),
                )
            };
            #[cfg(not(feature = "dynamic"))]
            let res_ptr = execFuncString(
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
        let c_path = CString::new(path.unwrap_or(""))?;

        unsafe {
            #[cfg(feature = "dynamic")]
            let res = {
                let f: Symbol<EvaluatePHSFn> = self._lib.get(b"evaluatePHS")?;
                f(
                    self.state,
                    c_script.as_ptr(),
                    c_module.as_ptr(),
                    c_path.as_ptr(),
                    verbose,
                )
            };
            #[cfg(not(feature = "dynamic"))]
            let res = evaluatePHS(
                self.state,
                c_script.as_ptr(),
                c_module.as_ptr(),
                c_path.as_ptr(),
                verbose,
            );

            if self.is_error_status()? {
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
            #[cfg(feature = "dynamic")]
            {
                if let Ok(free) = self._lib.get::<FreeStateFn>(b"freeState") {
                    free(self.state);
                }
            }
            #[cfg(not(feature = "dynamic"))]
            {
                freeState(self.state);
            }
        }
    }
}