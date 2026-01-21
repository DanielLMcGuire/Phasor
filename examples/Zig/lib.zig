const c = @cImport({
    @cInclude("stdint.h");
    @cInclude("stddef.h");
    @cInclude("stdbool.h");
});

pub const PhasorVM = opaque {};

pub const PhasorValueType = enum(u32) {
    PHASOR_TYPE_NULL,
    PHASOR_TYPE_BOOL,
    PHASOR_TYPE_INT,
    PHASOR_TYPE_FLOAT,
    PHASOR_TYPE_STRING,
};

pub const PhasorValueUnion = extern union {
    b: bool,
    i: i64,
    f: f64,
    s: [*c]const u8,
};

pub const PhasorValue = extern struct {
    type_: PhasorValueType,
    as_: PhasorValueUnion,
};

pub fn phasor_make_null() PhasorValue {
    return PhasorValue{ .type_ = PhasorValueType.PHASOR_TYPE_NULL, .as_ = undefined };
}

pub fn phasor_make_bool(b: bool) PhasorValue {
    return PhasorValue{ .type_ = PhasorValueType.PHASOR_TYPE_BOOL, .as_ = .{ .b = b } };
}

pub fn phasor_make_int(i: i64) PhasorValue {
    return PhasorValue{ .type_ = PhasorValueType.PHASOR_TYPE_INT, .as_ = .{ .i = i } };
}

pub fn phasor_make_float(f: f64) PhasorValue {
    return PhasorValue{ .type_ = PhasorValueType.PHASOR_TYPE_FLOAT, .as_ = .{ .f = f } };
}

pub fn phasor_make_string(s: [*c]const u8) PhasorValue {
    return PhasorValue{ .type_ = PhasorValueType.PHASOR_TYPE_STRING, .as_ = .{ .s = s } };
}

pub fn phasor_is_null(val: PhasorValue) bool {
    return val.type_ == PhasorValueType.PHASOR_TYPE_NULL;
}
pub fn phasor_is_bool(val: PhasorValue) bool {
    return val.type_ == PhasorValueType.PHASOR_TYPE_BOOL;
}
pub fn phasor_is_int(val: PhasorValue) bool {
    return val.type_ == PhasorValueType.PHASOR_TYPE_INT;
}
pub fn phasor_is_float(val: PhasorValue) bool {
    return val.type_ == PhasorValueType.PHASOR_TYPE_FLOAT;
}
pub fn phasor_is_string(val: PhasorValue) bool {
    return val.type_ == PhasorValueType.PHASOR_TYPE_STRING;
}
pub fn phasor_is_number(val: PhasorValue) bool {
    return phasor_is_int(val) or phasor_is_float(val);
}

pub fn phasor_to_bool(val: PhasorValue) bool {
    return val.as_.b;
}
pub fn phasor_to_int(val: PhasorValue) i64 {
    return val.as_.i;
}
pub fn phasor_to_float(val: PhasorValue) f64 {
    if (phasor_is_int(val)) {
        return @floatFromInt(val.as_.i);
    }
    return val.as_.f;
}

pub fn phasor_to_string(val: PhasorValue) [*c]const u8 {
    return val.as_.s;
}

pub const PhasorNativeFunction = *const fn (vm: *PhasorVM, argc: c_int, argv: ?*const PhasorValue) callconv(.c) PhasorValue;

pub const PhasorAPI = extern struct {
    register_function: ?*const fn (vm: *PhasorVM, name: [*c]const u8, func: PhasorNativeFunction) callconv(.c) void,
};

pub export fn add_integers(
    _: *PhasorVM,
    argc: c_int,
    argv: ?*const PhasorValue,
) callconv(.c) PhasorValue {
    if (argv == null or argc != 2) {
        return phasor_make_int(0);
    }

    const args: [*]const PhasorValue = @ptrCast(argv.?);

    if (!phasor_is_int(args[0]) or !phasor_is_int(args[1])) {
        return phasor_make_int(0);
    }

    return phasor_make_int(phasor_to_int(args[0]) + phasor_to_int(args[1]));
}

pub export fn phasor_plugin_entry(api: *const PhasorAPI, vm: *PhasorVM) callconv(.c) void {
    if (api.register_function) |register| {
        register(vm, "add", add_integers);
    }
}