#include "StdLib.hpp"
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#else
#include <cstdlib>
#endif

namespace Phasor
{

std::unordered_map<PhsString, std::function<void(Phasor::VM *)>> StdLib::modules{
	    {"stdio", registerIOFunctions},
	    {"stdsys", registerSysFunctions},
	    {"stdmath", registerMathFunctions},
	    {"stdstr", registerStringFunctions},
	    {"stdtype", registerTypeConvFunctions},
	    {"stdmeta", registerMetaFunctions},
	    {"stdmem", registerMemoryFunctions},
	    {"stdrand", registerRandomFunctions},
		{"stdarray", registerArrayFunctions},
#ifndef SANDBOXED
	    {"stdfile", registerFileFunctions},
#endif
	    {"std*",
	     [](Phasor::VM *vm) {
		     registerIOFunctions(vm);
		     registerSysFunctions(vm);
		     registerMathFunctions(vm);
		     registerStringFunctions(vm);
		     registerTypeConvFunctions(vm);
		     registerMetaFunctions(vm);
		     registerMemoryFunctions(vm);
		     registerRandomFunctions(vm);
			 registerArrayFunctions(vm);
#ifndef SANDBOXED
		     registerFileFunctions(vm);
#endif
	     }},
	};

std::unordered_map<PhsString, std::function<Value(const std::vector<Value> &args, VM *vm)>> StdLib::functions{
#ifndef SANDBOXED
	{"fabsolute", file_absolute},
	{"fexists", file_exists},
	{"fread", file_read},
	{"fwrite", file_write},
	{"fdelete", file_delete},
	{"fread", file_read},
	{"fwrite", file_write},
	{"fexists", file_exists},
	{"freadln", file_read_line},
	{"fwriteln", file_write_line},
	{"fappend", file_append},
	{"fmkdir", file_create_directory},
	{"frmdir", file_remove_directory},
	{"frm", file_delete},
	{"frn", file_rename},
	{"fcd", file_current_directory},
	{"fcp", file_copy},
	{"fmv", file_move},
	{"fpropset", file_property_edit},
	{"fproget", file_property_get},
	{"fmk", file_create},
	{"fjoin", file_join_path},
	{"fparent", file_parent},
	{"fexists", file_exists},
	{"fread", file_read},
	{"fwrite", file_write},
	{"fdelete", file_delete},
	{"gets", io_gets},
	{"clear", io_clear},
	{"sys_env", sys_env},
	{"sys_argv", sys_argv},
	{"sys_argc", sys_argc},
	{"sys_args", sys_args},
	{"sys_shell", sys_shell},
	{"sys_fork", sys_fork},
	{"sys_fork_detached", sys_fork_detached},
	{"sys_crash", sys_crash},
	{"sys_reset", sys_reset},
	{"sys_pid", sys_pid},
	{"sys_os", sys_os},
	{"isatty", sys_isatty},
	{"error", sys_crash},
	{"reset", sys_reset},
	{"wait_for_input", sys_wait_for_input},
	{"sys_get_memory", sys_get_free_memory},
	{"phs_op", meta_operation},
	{"phs_stack_run", meta_stack_run},
#endif
	{"arr_resize", array_resize},
	{"arr_length", array_length},
	{"arr_push", array_push},
	{"arr_pop", array_pop},
	{"arr_insert", array_insert},
	{"c_fmt", io_c_format},
	{"prints", io_prints},
	{"printf", io_printf},
	{"puts", io_puts},
	{"putf", io_putf},
	{"puts_error", io_puts_error},
	{"putf_error", io_putf_error},
	{"time", sys_time},
	{"time_fmt", sys_time_formatted},
	{"sleep", sys_sleep},
	{"shutdown", sys_shutdown},
	{"to_int", to_int},
	{"to_float", to_float},
	{"to_string", to_string},
	{"to_bool", to_bool},
	{"to_json", to_json},
	{"from_json", from_json},
	{"math_sqrt", math_sqrt},
	{"math_pow", math_pow},
	{"math_abs", math_abs},
	{"math_floor", math_floor},
	{"math_ceil", math_ceil},
	{"math_round", math_round},
	{"math_min", math_min},
	{"math_max", math_max},
	{"math_log", math_log},
	{"math_exp", math_exp},
	{"math_sin", math_sin},
	{"math_cos", math_cos},
	{"math_tan", math_tan},
	{"free", var_free},
	{"phs_version", meta_get_version},
	{"phs_alloc_info", meta_get_alloc_info},
	{"get_elements", meta_get_struct_elements},
	{"get_elements_values", meta_get_struct_elements_values},
	{"get_self", meta_get_self},
	{"get_registers", meta_get_registers},
	{"get_type", meta_get_type},
	{"rand_seed", rand_seed},
	{"rand_next_range", rand_next_range},
	{"rand_next_float", rand_next_float},
	{"find", str_find},
	{"len", str_len},
	{"char_at", str_char_at},
	{"substr", str_substr},
	{"concat", str_concat},
	{"to_upper", str_upper},
	{"to_lower", str_lower},
	{"starts_with", str_starts_with},
	{"ends_with", str_ends_with},
	{"sb_new", sb_new},
	{"sb_append", sb_append},
	{"sb_to_string", sb_to_string},
	{"sb_clear", sb_clear},
	{"sb_free", sb_free},
};

char **StdLib::argv = nullptr;
int    StdLib::argc = 0;

void StdLib::checkArgCount(const std::vector<Value> &args, size_t minimumArguments, const std::string &name,
                           bool allowMoreArguments)
{
	if (args.size() < minimumArguments)
	{
		throw std::runtime_error("Function '" + name + "' expects at least " + std::to_string(minimumArguments) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
	if (!allowMoreArguments && args.size() > minimumArguments)
	{
		throw std::runtime_error("Function '" + name + "' expects exactly " + std::to_string(minimumArguments) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
}

bool StdLib::std_import(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "using", true);

	for (const auto &arg : args)
	{
		auto it = modules.find(arg.string());
		if (it != modules.end())
		{
			it->second(vm);
		}
		else
		{
			throw std::runtime_error("Unknown module: " + arg.string());
		}
	}
	return true;
}

Value StdLib::run_internal(const std::vector<Value> &args, VM *vm)
{
	PhsString name = args[0].asString();
	std::vector<Value> fnArgs(args.begin() + 1, args.end());

	auto it = functions.find(name);
	if (it != functions.end())
	{
		return it->second(fnArgs, vm);
	}

	throw std::runtime_error("Unknown function: " + name.str());
}

#ifndef SANDBOXED
#if defined(_DEBUG) || defined(TRACING)
Value StdLib::std_assert(const std::vector<Value> &args, VM *vm)
#else
Value StdLib::std_assert(const std::vector<Value> &args, VM *)
#endif
{
	checkArgCount(args, 1, "assert", true);

	if (args.size() > 2)
	{ [[unlikely]]
		throw std::runtime_error("Assert expects 1 or 2 arguments, but got " + std::to_string(args.size()));
	}

#ifdef _DEBUG
	bool haveMessage = false;
	const char* message = nullptr;

	if (args.size() == 2)
	{
		message = args[1].c_str();
	}
#endif

#ifdef TRACING
#ifdef _DEBUG
	vm->log(std::format("StdLib::{}({:T})\n", __func__, args[0]));
#else
	vm->log(std::format("StdLib::{}({:T}): Assertion skipped (NDEBUG)\n", __func__, args[0]));
#endif
	vm->flush();
#endif

#ifdef _DEBUG
	if (!args[0].isTruthy())
	{ [[unlikely]]
		vm->logerr(std::format("StdLib::{}({:T}): Assertion failed!\n", __func__, args[0]));
		if (haveMessage) vm->logerr(std::format("{}\n", message));
		vm->flusherr();
	}
	assert(args[0].isTruthy());
#endif
	return phsnull;
}
#endif

} // namespace Phasor