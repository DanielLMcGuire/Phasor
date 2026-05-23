use std::ffi::NulError;
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

    #[cfg(feature = "dynamic")]
    #[error("Failed to load or query Phasor DLL: {0}")]
    LibraryError(#[from] libloading::Error),
}