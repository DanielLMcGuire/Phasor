const std = @import("std");
const builtin = @import("builtin");

pub const PhasorError = error{
    LibraryNotFound,
    SymbolNotFound,
    StateCreationFailed,
    VmException,
    NullStringReturn,
    CompilationFailed,
    StateFailed,
    OutOfMemory,
};

const lib_name = switch (builtin.os.tag) {
    .windows => "phasorrt.dll",
    .macos, .ios, .tvos, .watchos => "libphasorrt.dylib",
    else => "libphasorrt.so",
};

const Sym = struct {
    getVersion: *const fn () callconv(.C) [*:0]const u8,
    exec: *const fn (?*anyopaque, [*]const u8, usize, [*:0]const u8, c_int, [*]const [*:0]const u8) callconv(.C) c_int,
    execFuncInt: *const fn (?*anyopaque, [*]const u8, usize, [*:0]const u8, c_int, [*]const [*:0]const u8, [*:0]const u8) callconv(.C) c_int,
    execFuncString: *const fn (?*anyopaque, [*]const u8, usize, [*:0]const u8, c_int, [*]const [*:0]const u8, [*:0]const u8) callconv(.C) ?[*:0]const u8,
    evaluatePHS: *const fn (?*anyopaque, [*:0]const u8, [*:0]const u8, [*:0]const u8, bool) callconv(.C) c_int,
    evaluatePUL: *const fn (?*anyopaque, [*:0]const u8, [*:0]const u8) callconv(.C) c_int,
    compilePHS: *const fn ([*:0]const u8, [*:0]const u8, [*:0]const u8, ?[*]u8, usize, *usize) callconv(.C) bool,
    compilePUL: *const fn ([*:0]const u8, [*:0]const u8, ?[*]u8, usize, *usize) callconv(.C) bool,
    createState: *const fn () callconv(.C) ?*anyopaque,
    initStdLib: *const fn (?*anyopaque) callconv(.C) void,
    freeState: *const fn (?*anyopaque) callconv(.C) bool,
    resetState: *const fn (?*anyopaque, bool, bool) callconv(.C) bool,
};

var lib_once = std.once(loadLib);
var lib_sym: ?Sym = null;
var lib_err: PhasorError = PhasorError.LibraryNotFound;

fn loadLib() void {
    var lib = std.DynLib.open(lib_name) catch return;

    inline for (std.meta.fields(Sym)) |f| {
        if (lib.lookup(f.type, f.name ++ "") == null) {
            lib_err = PhasorError.SymbolNotFound;
            lib.close();
            return;
        }
    }

    var resolved: Sym = undefined;
    inline for (std.meta.fields(Sym)) |f| {
        @field(resolved, f.name) = lib.lookup(f.type, f.name ++ "").?;
    }

    lib_sym = resolved;
}

fn sym() PhasorError!*const Sym {
    lib_once.call();
    if (lib_sym) |*s| return s;
    return lib_err;
}

const empty_argv: [0][*:0]const u8 = .{};

fn argv_ptr(args: []const [*:0]const u8) [*]const [*:0]const u8 {
    return if (args.len > 0) args.ptr else &empty_argv;
}

fn check(code: c_int) PhasorError!i32 {
    if (code == -1) return PhasorError.VmException;
    return @intCast(code);
}

// public api:

/// returns the runtime version string
pub fn getVersion() PhasorError![]const u8 {
    return std.mem.span((try sym()).getVersion());
}

/// Compile Phasor source into caller-owned `[]u8`.
pub fn compilePHS(
    allocator: std.mem.Allocator,
    script: [:0]const u8,
    module_name: [:0]const u8,
    module_path: [:0]const u8,
) PhasorError![]u8 {
    const s = try sym();
    var required: usize = 0;
    if (!s.compilePHS(script, module_name, module_path, null, 0, &required))
        return PhasorError.CompilationFailed;
    const buf = allocator.alloc(u8, required) catch return PhasorError.OutOfMemory;
    errdefer allocator.free(buf);
    var actual: usize = 0;
    if (!s.compilePHS(script, module_name, module_path, buf.ptr, buf.len, &actual))
        return PhasorError.CompilationFailed;
    return allocator.realloc(buf, actual) catch buf[0..actual];
}

/// Compile Pulsar source into caller-owned `[]u8`.
pub fn compilePUL(
    allocator: std.mem.Allocator,
    script: [:0]const u8,
    module_name: [:0]const u8,
) PhasorError![]u8 {
    const s = try sym();
    var required: usize = 0;
    if (!s.compilePUL(script, module_name, null, 0, &required))
        return PhasorError.CompilationFailed;
    const buf = allocator.alloc(u8, required) catch return PhasorError.OutOfMemory;
    errdefer allocator.free(buf);
    var actual: usize = 0;
    if (!s.compilePUL(script, module_name, buf.ptr, buf.len, &actual))
        return PhasorError.CompilationFailed;
    return allocator.realloc(buf, actual) catch buf[0..actual];
}

/// Phasor VM state
pub const State = struct {
    ptr: *anyopaque,

    pub fn create() PhasorError!State {
        const s = try sym();
        const raw = s.createState() orelse return PhasorError.StateCreationFailed;
        s.initStdLib(raw);
        return .{ .ptr = raw };
    }

    pub fn deinit(self: State) void {
        _ = (sym() catch return).freeState(self.ptr);
    }

    pub fn reset(self: State, reset_functions: bool, reset_variables: bool) PhasorError!void {
        if (!(try sym()).resetState(self.ptr, reset_functions, reset_variables))
            return PhasorError.StateFailed;
    }

    pub fn exec(
        self: State,
        bytecode: []const u8,
        module_name: [:0]const u8,
        args: []const [*:0]const u8,
    ) PhasorError!i32 {
        return check((try sym()).exec(
            self.ptr,
            bytecode.ptr,
            bytecode.len,
            module_name,
            @intCast(args.len),
            argv_ptr(args),
        ));
    }

    pub fn execFuncInt(
        self: State,
        bytecode: []const u8,
        module_name: [:0]const u8,
        args: []const [*:0]const u8,
        function_name: [:0]const u8,
    ) PhasorError!i32 {
        return check((try sym()).execFuncInt(
            self.ptr,
            bytecode.ptr,
            bytecode.len,
            module_name,
            @intCast(args.len),
            argv_ptr(args),
            function_name,
        ));
    }

    /// The return is owned by the Phasor runtime, it is overwritten
    /// on the next call. Copy it before calling again.
    pub fn execFuncString(
        self: State,
        bytecode: []const u8,
        module_name: [:0]const u8,
        args: []const [*:0]const u8,
        function_name: [:0]const u8,
    ) PhasorError![:0]const u8 {
        const raw = (try sym()).execFuncString(
            self.ptr,
            bytecode.ptr,
            bytecode.len,
            module_name,
            @intCast(args.len),
            argv_ptr(args),
            function_name,
        ) orelse return PhasorError.NullStringReturn;
        return std.mem.span(raw);
    }

    pub fn evaluatePHS(
        self: State,
        script: [:0]const u8,
        module_name: [:0]const u8,
        module_path: [:0]const u8,
        verbose: bool,
    ) PhasorError!i32 {
        return check((try sym()).evaluatePHS(
            self.ptr,
            script,
            module_name,
            module_path,
            verbose,
        ));
    }

    pub fn evaluatePUL(
        self: State,
        script: [:0]const u8,
        module_name: [:0]const u8,
    ) PhasorError!i32 {
        return check((try sym()).evaluatePUL(self.ptr, script, module_name));
    }
};
