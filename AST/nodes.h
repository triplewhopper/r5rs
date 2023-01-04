#ifndef R5RS_NODES_H
#define R5RS_NODES_H

#include "../Include/object.h"

typedef struct expression_node ExprNode;
// ⟨expression⟩ −→ ⟨variable⟩
// | ⟨literal⟩
// | ⟨procedure call⟩
// | ⟨lambda expression⟩
// | ⟨conditional⟩
// | ⟨assignment⟩
// | ⟨derived expression⟩
// | ⟨macro use⟩
// | ⟨macro block⟩
typedef struct ast_node ASTNode;
typedef struct literal_node LiteralNode;
typedef struct variable_node VariableNode;
typedef struct procedure_call_node ProcedureCallNode;
typedef struct lambda_expression_node LambdaExprNode;
typedef struct conditional_node ConditionalNode;
typedef struct assignment_node AssignmentNode;
typedef struct derived_expression_node DerivedExprNode;
typedef struct macro_use_node MacroUseNode;
typedef struct macro_block_node MacroBlockNode;

typedef struct quotation_node QuoteNode;
TypeObject ASTNode_Type;
TypeObject LiteralNode_Type;
TypeObject VariableNode_Type;
TypeObject ProcedureCallNode_Type;
TypeObject LambdaExprNode_Type;
TypeObject ConditionalNode_Type;
TypeObject AssignmentNode_Type;
TypeObject DerivedExprNode_Type;
TypeObject MacroUseNode_Type;
TypeObject MacroBlockNode_Type;
struct ast_node {
	Object ob_base;
};
struct literal_node{
	enum literal_node_kind{
		LITERAL_QUOTATION,
		LITERAL_BOOLEAN, LITERAL_NUMBER,
		LITERAL_CHARACTER, LITERAL_STRING
	}kind;
	union{

	};
};
#endif //R5RS_NODES_H
