#include "plier/dialect.hpp"

#include <mlir/IR/DialectImplementation.h>
#include <mlir/IR/StandardTypes.h>
#include <mlir/IR/Builders.h>

#include <mlir/Dialect/StandardOps/IR/Ops.h>
#include <llvm/ADT/TypeSwitch.h>

namespace plier
{

namespace detail
{
struct PyTypeStorage : public mlir::TypeStorage
{
    using KeyTy = mlir::StringRef;

    PyTypeStorage(mlir::StringRef name): name(name) {}

    bool operator==(const KeyTy& key) const
    {
        return key == name;
    }

    static PyTypeStorage* construct(mlir::TypeStorageAllocator& allocator,
                                    const KeyTy& key)
    {
        return new(allocator.allocate<PyTypeStorage>())
            PyTypeStorage(allocator.copyInto(key));
    }

    mlir::StringRef name;
};
}

void PlierDialect::initialize()
{
    addOperations<
#define GET_OP_LIST
#include "plier/PlierOps.cpp.inc"
        >();
    addTypes<plier::PyType>();
}

mlir::Type PlierDialect::parseType(mlir::DialectAsmParser &parser) const {
    parser.emitError(parser.getNameLoc(), "unknown type");
    return mlir::Type();
}

void PlierDialect::printType(mlir::Type type, mlir::DialectAsmPrinter &os) const {
    llvm::TypeSwitch<mlir::Type>(type)
        .Case<plier::PyType>([&](auto t){ os << "PyType<" << t.getName() << ">"; })
        .Default([](auto){ llvm_unreachable("unexpected type"); });
}

PyType PyType::get(mlir::MLIRContext* context, llvm::StringRef name)
{
    assert(!name.empty());
    return Base::get(context, name);
}

PyType PyType::getUndefined(MLIRContext* context)
{
    return Base::get(context, "");
}

llvm::StringRef PyType::getName() const
{
    return getImpl()->name;
}

void ArgOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                  unsigned index, mlir::StringRef name) {
    ArgOp::build(builder, state, PyType::getUndefined(state.getContext()),
                 index, name);
}

mlir::OpFoldResult ArgOp::fold(llvm::ArrayRef<mlir::Attribute> /*operands*/)
{
    auto func = getParentOfType<mlir::FuncOp>();
    auto ind = index();
    if (ind >= func.getNumArguments() ||
        func.getArgument(ind).getType() != getType())
    {
        emitError("Invalid function args");
        return nullptr;
    }
    return func.getArgument(ind);
}

void ConstOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,

                   mlir::Attribute val) {
    ConstOp::build(builder, state, PyType::getUndefined(state.getContext()),
                   val);
}

void GlobalOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                     mlir::StringRef name) {
    GlobalOp::build(builder, state, PyType::getUndefined(state.getContext()),
                    name);
}

void BinOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                  mlir::Value lhs, mlir::Value rhs, mlir::StringRef op) {
    BinOp::build(builder, state, PyType::getUndefined(state.getContext()), lhs,
                 rhs, op);
}

mlir::OpFoldResult CastOp::fold(llvm::ArrayRef<mlir::Attribute> /*operands*/)
{
    auto op_type = getOperand().getType();
    auto ret_type = getType();
    if (op_type == ret_type && op_type != PyType::getUndefined(getContext()))
    {
        return getOperand();
    }
    return nullptr;
}

void PyCallOp::build(OpBuilder &builder, OperationState &state, mlir::Value func,
                     mlir::ValueRange args,
                     mlir::ArrayRef<std::pair<std::string, mlir::Value>> kwargs) {
    auto ctx = builder.getContext();
    mlir::SmallVector<mlir::Value, 16> all_args;
    all_args.reserve(args.size() + kwargs.size());
    std::copy(args.begin(), args.end(), std::back_inserter(all_args));
    auto kw_start = static_cast<uint32_t>(all_args.size());
    mlir::SmallVector<mlir::Attribute, 8> kw_names;
    kw_names.reserve(kwargs.size());
    for (auto& a : kwargs)
    {
        kw_names.push_back(mlir::StringAttr::get(a.first, ctx));
        all_args.push_back(a.second);
    }
    PyCallOp::build(builder, state, PyType::getUndefined(state.getContext()),
        func, all_args, kw_start, mlir::ArrayAttr::get(kw_names, ctx));
}

void BuildTupleOp::build(OpBuilder &builder, OperationState &state,
                         ::mlir::ValueRange args)
{
    BuildTupleOp::build(builder, state,
                        PyType::getUndefined(state.getContext()), args);
}

//mlir::LogicalResult BuildTupleOp::fold(
//    llvm::ArrayRef<mlir::Attribute> /*operands*/,
//    llvm::SmallVectorImpl<mlir::OpFoldResult> &results)
//{
//    auto res_types = getResultTypes();
//    auto args = getOperands();
//    if (res_types.size() == args.size())
//    {
//        std::copy(args.begin(), args.end(), std::back_inserter(results));
//        return mlir::success();
//    }
//    return mlir::failure();
//}

void StaticGetItemOp::build(OpBuilder &builder, OperationState &state,
                            ::mlir::Value value, ::mlir::Value index_var,
                            unsigned int index)
{
    StaticGetItemOp::build(builder, state,
                           PyType::getUndefined(state.getContext()),
                           value, index_var, index);
}

//mlir::OpFoldResult StaticGetItemOp::fold(llvm::ArrayRef<mlir::Attribute> /*operands*/)
//{
//    auto index = this->index();
//    auto args = getOperands();
//    if ((index + 1) < args.size() && // skip last arg
//        args[index].getType() == getResult().getType())
//    {
//        return args[index];
//    }
//    return nullptr;
//}

void GetiterOp::build(OpBuilder &builder, OperationState &state,
                            ::mlir::Value value)
{
    GetiterOp::build(builder, state, PyType::getUndefined(state.getContext()),
                     value);
}

void IternextOp::build(OpBuilder &builder, OperationState &state,
                            ::mlir::Value value)
{
    IternextOp::build(builder, state, PyType::getUndefined(state.getContext()),
                      value);
}

void PairfirstOp::build(OpBuilder &builder, OperationState &state,
                            ::mlir::Value value)
{
    PairfirstOp::build(builder, state, PyType::getUndefined(state.getContext()),
                       value);
}

//mlir::OpFoldResult PairfirstOp::fold(llvm::ArrayRef<mlir::Attribute> /*operands*/)
//{
//    if (getNumOperands() == 2)
//    {
//        return getOperand(0);
//    }
//    return nullptr;
//}

void PairsecondOp::build(OpBuilder &builder, OperationState &state,
                            ::mlir::Value value)
{
    PairsecondOp::build(builder, state,
                        PyType::getUndefined(state.getContext()), value);
}

//mlir::OpFoldResult PairsecondOp::fold(llvm::ArrayRef<mlir::Attribute> /*operands*/)
//{
//    if (getNumOperands() == 2)
//    {
//        return getOperand(1);
//    }
//    return nullptr;
//}

}

#define GET_OP_CLASSES
#include "plier/PlierOps.cpp.inc"

#include "plier/PlierOpsEnums.cpp.inc"
