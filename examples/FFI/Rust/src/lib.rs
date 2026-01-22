use std::os::raw::{c_char, c_int};

#[repr(C)]
pub struct PhasorVM {
    _private: [u8; 0],
}

#[repr(C)]
#[derive(Copy, Clone)]
pub enum PhasorValueType {
    PHASOR_TYPE_NULL,
    PHASOR_TYPE_BOOL,
    PHASOR_TYPE_INT,
    PHASOR_TYPE_FLOAT,
    PHASOR_TYPE_STRING,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub union PhasorValueUnion {
    pub b: bool,
    pub i: i64,
    pub f: f64,
    pub s: *const c_char,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct PhasorValue {
    pub type_: PhasorValueType,
    pub as_: PhasorValueUnion,
}

#[repr(C)]
pub struct PhasorAPI {
    pub register_function: Option<
        extern "C" fn(
            vm: *mut PhasorVM,
            name: *const c_char,
            func: PhasorNativeFunction,
        ),
    >,
}

pub type PhasorNativeFunction =
    extern "C" fn(vm: *mut PhasorVM, argc: c_int, argv: *const PhasorValue) -> PhasorValue;

fn make_int(value: i64) -> PhasorValue {
    PhasorValue {
        type_: PhasorValueType::PHASOR_TYPE_INT,
        as_: PhasorValueUnion { i: value },
    }
}

extern "C" fn add_integers(
    _vm: *mut PhasorVM,
    argc: c_int,
    argv: *const PhasorValue,
) -> PhasorValue {
    if argc != 2 || argv.is_null() {
        return make_int(0);
    }

    unsafe {
        let a = (*argv.add(0)).as_.i;
        let b = (*argv.add(1)).as_.i;
        make_int(a + b)
    }
}

#[no_mangle]
pub extern "C" fn phasor_plugin_entry(api: *const PhasorAPI, vm: *mut PhasorVM) {
    if api.is_null() || vm.is_null() {
        return;
    }

    unsafe {
        let api = &*api;
        if let Some(register) = api.register_function {
            let name = b"add\0";
            register(vm, name.as_ptr() as *const c_char, add_integers);
        }
    }
}
