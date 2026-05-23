use libc::{c_char, c_int, c_uchar, c_void, size_t};

#[cfg(feature = "dynamic")]
pub type GetVersionFn = unsafe extern "C" fn() -> *const c_char;

#[cfg(feature = "dynamic")]
pub type ExecFn = unsafe extern "C" fn(
    state: *mut c_void,
    bytecode: *const c_uchar,
    bytecode_size: size_t,
    module_name: *const c_char,
    argc: c_int,
    argv: *const *const c_char,
) -> c_int;

#[cfg(feature = "dynamic")]
pub type ExecFuncIntFn = unsafe extern "C" fn(
    state: *mut c_void,
    bytecode: *const c_uchar,
    bytecode_size: size_t,
    module_name: *const c_char,
    argc: c_int,
    argv: *const *const c_char,
    function_name: *const c_char,
) -> c_int;

#[cfg(feature = "dynamic")]
pub type ExecFuncStringFn = unsafe extern "C" fn(
    state: *mut c_void,
    bytecode: *const c_uchar,
    bytecode_size: size_t,
    module_name: *const c_char,
    argc: c_int,
    argv: *const *const c_char,
    function_name: *const c_char,
) -> *const c_char;

#[cfg(feature = "dynamic")]
pub type EvaluatePHSFn = unsafe extern "C" fn(
    state: *mut c_void,
    script: *const c_char,
    module_name: *const c_char,
    module_path: *const c_char,
    verbose: bool,
) -> c_int;

#[cfg(feature = "dynamic")]
pub type EvaluatePULFn = unsafe extern "C" fn(
    state: *mut c_void,
    script: *const c_char,
    module_name: *const c_char,
) -> c_int;

#[cfg(feature = "dynamic")]
pub type CompilePHSFn = unsafe extern "C" fn(
    script: *const c_char,
    module_name: *const c_char,
    module_path: *const c_char,
    buffer: *mut c_uchar,
    buffer_size: size_t,
    out_size: *mut size_t,
) -> bool;

#[cfg(feature = "dynamic")]
pub type CompilePULFn = unsafe extern "C" fn(
    script: *const c_char,
    module_name: *const c_char,
    buffer: *mut c_uchar,
    buffer_size: size_t,
    out_size: *mut size_t,
) -> bool;

#[cfg(feature = "dynamic")]
pub type CreateStateFn = unsafe extern "C" fn() -> *mut c_void;

#[cfg(feature = "dynamic")]
pub type InitStdLibFn = unsafe extern "C" fn(state: *mut c_void);

#[cfg(feature = "dynamic")]
pub type FreeStateFn = unsafe extern "C" fn(state: *mut c_void) -> bool;

#[cfg(feature = "dynamic")]
pub type ResetStateFn = unsafe extern "C" fn(
    state: *mut c_void,
    reset_functions: bool,
    reset_variables: bool,
) -> bool;

#[cfg(feature = "dynamic")]
pub type IsErrorStatusFn = unsafe extern "C" fn(state: *mut c_void) -> bool;

#[cfg(not(feature = "dynamic"))]
unsafe extern "C" {
    pub fn getVersion() -> *const c_char;
    pub fn createState() -> *mut c_void;
    pub fn freeState(state: *mut c_void) -> bool;
    pub fn initStdLib(state: *mut c_void);
    pub fn resetState(
        state: *mut c_void,
        reset_functions: bool,
        reset_variables: bool,
    ) -> bool;
    pub fn compilePHS(
        script: *const c_char,
        module_name: *const c_char,
        module_path: *const c_char,
        buffer: *mut c_uchar,
        buffer_size: size_t,
        out_size: *mut size_t,
    ) -> bool;
    pub fn compilePUL(
        script: *const c_char,
        module_name: *const c_char,
        buffer: *mut c_uchar,
        buffer_size: size_t,
        out_size: *mut size_t,
    ) -> bool;
    pub fn exec(
        state: *mut c_void,
        bytecode: *const c_uchar,
        bytecode_size: size_t,
        module_name: *const c_char,
        argc: c_int,
        argv: *const *const c_char,
    ) -> c_int;
    pub fn execFuncInt(
        state: *mut c_void,
        bytecode: *const c_uchar,
        bytecode_size: size_t,
        module_name: *const c_char,
        argc: c_int,
        argv: *const *const c_char,
        function_name: *const c_char,
    ) -> c_int;
    pub fn execFuncString(
        state: *mut c_void,
        bytecode: *const c_uchar,
        bytecode_size: size_t,
        module_name: *const c_char,
        argc: c_int,
        argv: *const *const c_char,
        function_name: *const c_char,
    ) -> *const c_char;
    pub fn evaluatePHS(
        state: *mut c_void,
        script: *const c_char,
        module_name: *const c_char,
        module_path: *const c_char,
        verbose: bool,
    ) -> c_int;
    pub fn evaluatePUL(
        state: *mut c_void,
        script: *const c_char,
        module_name: *const c_char,
    ) -> c_int;
    pub fn isErrorStatus(state: *mut c_void) -> bool;
}