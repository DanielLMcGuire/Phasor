#include "../../Runtime/VM/VM.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../ISA/map.hpp"

namespace Phasor 
{

inline Value bytecodeToValue(const Bytecode &bc)
{
	Value root = Value::createStruct("Bytecode");

	std::vector<Value> instrElems;
	instrElems.reserve(bc.instructions.size());
	for (const auto &instr : bc.instructions)
	{
		Value iv = Value::createStruct("Instruction");
		iv["op"] = Value(static_cast<i64>(static_cast<int>(instr.op)));
		iv["o1"] = Value(static_cast<i64>(instr.operand1));
		iv["o2"] = Value(static_cast<i64>(instr.operand2));
		iv["o3"] = Value(static_cast<i64>(instr.operand3));
		instrElems.push_back(std::move(iv));
	}
	root["instructions"] = Value::createArray(std::move(instrElems));

	root["constants"] = Value::createArray(bc.constants);

	Value variablesMap = Value::createStruct("Map");
	for (const auto &[varKey, varVal] : bc.variables)
		variablesMap[PhsString(varKey)] = Value(static_cast<i64>(varVal));
	root["variables"] = variablesMap;

	std::vector<Value> scopeOuter;
	scopeOuter.reserve(bc.scopeVarLists.size());
	for (const auto &scope : bc.scopeVarLists)
	{
		std::vector<Value> scopeInner;
		scopeInner.reserve(scope.size());
		for (const auto &[scopeIdx, scopeName] : scope)
		{
			Value pair = Value::createStruct("ScopeVar");
			pair["index"] = Value(static_cast<i64>(scopeIdx));
			pair["name"]  = Value(scopeName);
			scopeInner.push_back(std::move(pair));
		}
		scopeOuter.push_back(Value::createArray(std::move(scopeInner)));
	}
	root["scopeVarLists"] = Value::createArray(std::move(scopeOuter));

	Value functionEntriesMap = Value::createStruct("Map");
	for (const auto &[feKey, feVal] : bc.functionEntries)
		functionEntriesMap[PhsString(feKey)] = Value(static_cast<i64>(feVal));
	root["functionEntries"] = functionEntriesMap;

	Value functionParamCountsMap = Value::createStruct("Map");
	for (const auto &[fpcKey, fpcVal] : bc.functionParamCounts)
		functionParamCountsMap[PhsString(fpcKey)] = Value(static_cast<i64>(fpcVal));
	root["functionParamCounts"] = functionParamCountsMap;

	Value functionParamTypeNamesMap = Value::createStruct("Map");
	for (const auto &[fptnKey, fptnVec] : bc.functionParamTypeNames)
	{
		std::vector<Value> fptnElems;
		fptnElems.reserve(fptnVec.size());
		for (const auto &fptnStr : fptnVec)
			fptnElems.emplace_back(fptnStr);
		functionParamTypeNamesMap[PhsString(fptnKey)] = Value::createArray(std::move(fptnElems));
	}
	root["functionParamTypeNames"] = functionParamTypeNamesMap;

	Value functionParamArrayDimsMap = Value::createStruct("Map");
	for (const auto &[fpadKey, fpadOuterVec] : bc.functionParamArrayDims)
	{
		std::vector<Value> fpadOuter;
		fpadOuter.reserve(fpadOuterVec.size());
		for (const auto &fpadInnerVec : fpadOuterVec)
		{
			std::vector<Value> fpadInner;
			fpadInner.reserve(fpadInnerVec.size());
			for (int fpadDim : fpadInnerVec)
				fpadInner.emplace_back(static_cast<i64>(fpadDim));
			fpadOuter.push_back(Value::createArray(std::move(fpadInner)));
		}
		functionParamArrayDimsMap[PhsString(fpadKey)] = Value::createArray(std::move(fpadOuter));
	}
	root["functionParamArrayDims"] = functionParamArrayDimsMap;

	Value functionReturnTypeNamesMap = Value::createStruct("Map");
	for (const auto &[frtnKey, frtnVal] : bc.functionReturnTypeNames)
		functionReturnTypeNamesMap[PhsString(frtnKey)] = Value(frtnVal);
	root["functionReturnTypeNames"] = functionReturnTypeNamesMap;

	Value functionReturnArrayDimsMap = Value::createStruct("Map");
	for (const auto &[fradKey, fradVec] : bc.functionReturnArrayDims)
	{
		std::vector<Value> fradElems;
		fradElems.reserve(fradVec.size());
		for (int fradDim : fradVec)
			fradElems.emplace_back(static_cast<i64>(fradDim));
		functionReturnArrayDimsMap[PhsString(fradKey)] = Value::createArray(std::move(fradElems));
	}
	root["functionReturnArrayDims"] = functionReturnArrayDimsMap;

	root["nextVarIndex"] = Value(static_cast<i64>(bc.nextVarIndex));

	std::vector<Value> structElems;
	structElems.reserve(bc.structs.size());
	for (const auto &si : bc.structs)
	{
		Value sv = Value::createStruct("StructInfo");
		sv["name"]            = Value(si.name);
		sv["firstConstIndex"] = Value(static_cast<i64>(si.firstConstIndex));
		sv["fieldCount"]      = Value(static_cast<i64>(si.fieldCount));

		std::vector<Value> fieldNamesElems;
		fieldNamesElems.reserve(si.fieldNames.size());
		for (const auto &fn : si.fieldNames)
			fieldNamesElems.emplace_back(fn);
		sv["fieldNames"] = Value::createArray(std::move(fieldNamesElems));

		std::vector<Value> fieldArrayDimsElems;
		fieldArrayDimsElems.reserve(si.fieldArrayDims.size());
		for (const auto &dims : si.fieldArrayDims)
		{
			std::vector<Value> dimElems;
			dimElems.reserve(dims.size());
			for (int d : dims)
				dimElems.emplace_back(static_cast<i64>(d));
			fieldArrayDimsElems.push_back(Value::createArray(std::move(dimElems)));
		}
		sv["fieldArrayDims"] = Value::createArray(std::move(fieldArrayDimsElems));

		std::vector<Value> fieldTypeNamesElems;
		fieldTypeNamesElems.reserve(si.fieldTypeNames.size());
		for (const auto &ftn : si.fieldTypeNames)
			fieldTypeNamesElems.emplace_back(ftn);
		sv["fieldTypeNames"] = Value::createArray(std::move(fieldTypeNamesElems));

		structElems.push_back(std::move(sv));
	}
	root["structs"] = Value::createArray(std::move(structElems));

	Value structEntriesMap = Value::createStruct("Map");
	for (const auto &[seKey, seVal] : bc.structEntries)
		structEntriesMap[PhsString(seKey)] = Value(static_cast<i64>(seVal));
	root["structEntries"] = structEntriesMap;

	return root;
}

inline Bytecode bytecodeFromValue(const Value &root)
{
	Bytecode bc;

	auto instrArr = root.getField("instructions").asArray();
	bc.instructions.reserve(instrArr->size());
	for (const auto &iv : *instrArr)
	{
		Instruction instr;
		instr.op       = static_cast<OpCode>(iv.getField("op").asInt());
		instr.operand1 = static_cast<i32>(iv.getField("o1").asInt());
		instr.operand2 = static_cast<i32>(iv.getField("o2").asInt());
		instr.operand3 = static_cast<i32>(iv.getField("o3").asInt());
		bc.instructions.push_back(instr);
	}

	auto constArr = root.getField("constants").asArray();
	bc.constants.reserve(constArr->size());
	for (const auto &cv : *constArr)
		bc.constants.push_back(cv);

	auto variablesStruct = root.getField("variables").asStruct();
	for (const auto &[varKey, varVal] : variablesStruct->fields)
		bc.variables[varKey.str()] = static_cast<int>(varVal.asInt());

	auto scopeOuterArr = root.getField("scopeVarLists").asArray();
	bc.scopeVarLists.reserve(scopeOuterArr->size());
	for (const auto &scopeVal : *scopeOuterArr)
	{
		std::vector<std::pair<int, std::string>> scope;
		auto scopeInnerArr = scopeVal.asArray();
		scope.reserve(scopeInnerArr->size());
		for (const auto &pairVal : *scopeInnerArr)
			scope.emplace_back(static_cast<int>(pairVal.getField("index").asInt()),
			                    pairVal.getField("name").stl_string());
		bc.scopeVarLists.push_back(std::move(scope));
	}

	auto functionEntriesStruct = root.getField("functionEntries").asStruct();
	for (const auto &[feKey, feVal] : functionEntriesStruct->fields)
		bc.functionEntries[feKey.str()] = static_cast<int>(feVal.asInt());

	auto functionParamCountsStruct = root.getField("functionParamCounts").asStruct();
	for (const auto &[fpcKey, fpcVal] : functionParamCountsStruct->fields)
		bc.functionParamCounts[fpcKey.str()] = static_cast<int>(fpcVal.asInt());

	auto functionParamTypeNamesStruct = root.getField("functionParamTypeNames").asStruct();
	for (const auto &[fptnKey, fptnVal] : functionParamTypeNamesStruct->fields)
	{
		std::vector<std::string> fptnVec;
		auto fptnArr = fptnVal.asArray();
		fptnVec.reserve(fptnArr->size());
		for (const auto &fptnElem : *fptnArr)
			fptnVec.push_back(fptnElem.stl_string());
		bc.functionParamTypeNames[fptnKey.str()] = std::move(fptnVec);
	}

	auto functionParamArrayDimsStruct = root.getField("functionParamArrayDims").asStruct();
	for (const auto &[fpadKey, fpadVal] : functionParamArrayDimsStruct->fields)
	{
		std::vector<std::vector<int>> fpadOuterVec;
		auto fpadOuterArr = fpadVal.asArray();
		fpadOuterVec.reserve(fpadOuterArr->size());
		for (const auto &fpadInnerVal : *fpadOuterArr)
		{
			std::vector<int> fpadInnerVec;
			auto fpadInnerArr = fpadInnerVal.asArray();
			fpadInnerVec.reserve(fpadInnerArr->size());
			for (const auto &fpadDimVal : *fpadInnerArr)
				fpadInnerVec.push_back(static_cast<int>(fpadDimVal.asInt()));
			fpadOuterVec.push_back(std::move(fpadInnerVec));
		}
		bc.functionParamArrayDims[fpadKey.str()] = std::move(fpadOuterVec);
	}

	auto functionReturnTypeNamesStruct = root.getField("functionReturnTypeNames").asStruct();
	for (const auto &[frtnKey, frtnVal] : functionReturnTypeNamesStruct->fields)
		bc.functionReturnTypeNames[frtnKey.str()] = frtnVal.stl_string();

	auto functionReturnArrayDimsStruct = root.getField("functionReturnArrayDims").asStruct();
	for (const auto &[fradKey, fradVal] : functionReturnArrayDimsStruct->fields)
	{
		std::vector<int> fradVec;
		auto fradArr = fradVal.asArray();
		fradVec.reserve(fradArr->size());
		for (const auto &fradElem : *fradArr)
			fradVec.push_back(static_cast<int>(fradElem.asInt()));
		bc.functionReturnArrayDims[fradKey.str()] = std::move(fradVec);
	}

	bc.nextVarIndex = static_cast<int>(root.getField("nextVarIndex").asInt());

	auto structsArr = root.getField("structs").asArray();
	bc.structs.reserve(structsArr->size());
	for (const auto &sv : *structsArr)
	{
		StructInfo si;
		si.name            = sv.getField("name").stl_string();
		si.firstConstIndex = static_cast<int>(sv.getField("firstConstIndex").asInt());
		si.fieldCount      = static_cast<int>(sv.getField("fieldCount").asInt());

		auto fieldNamesArr = sv.getField("fieldNames").asArray();
		si.fieldNames.reserve(fieldNamesArr->size());
		for (const auto &fnVal : *fieldNamesArr)
			si.fieldNames.push_back(fnVal.stl_string());

		auto fieldArrayDimsArr = sv.getField("fieldArrayDims").asArray();
		si.fieldArrayDims.reserve(fieldArrayDimsArr->size());
		for (const auto &dimsVal : *fieldArrayDimsArr)
		{
			std::vector<int> dims;
			auto dimsInnerArr = dimsVal.asArray();
			dims.reserve(dimsInnerArr->size());
			for (const auto &dimVal : *dimsInnerArr)
				dims.push_back(static_cast<int>(dimVal.asInt()));
			si.fieldArrayDims.push_back(std::move(dims));
		}

		auto fieldTypeNamesArr = sv.getField("fieldTypeNames").asArray();
		si.fieldTypeNames.reserve(fieldTypeNamesArr->size());
		for (const auto &ftnVal : *fieldTypeNamesArr)
			si.fieldTypeNames.push_back(ftnVal.stl_string());

		bc.structs.push_back(std::move(si));
	}

	auto structEntriesStruct = root.getField("structEntries").asStruct();
	for (const auto &[seKey, seVal] : structEntriesStruct->fields)
		bc.structEntries[seKey.str()] = static_cast<int>(seVal.asInt());

	return bc;
}

}