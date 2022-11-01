/* Parsegen autogenerated definition file - do not edit! */
/* clang-format off */

enum {
    predef_tt_error = 258,
    tt_end_of_file,
    tt_end_of_input,
    tt_bool_literal,
    tt_int_literal,
    tt_float_literal,
    tt_string_literal,
    tt_id,
    tt_ellipsis,
    tt_scope_resolution,
    tt_concatenate,
    tt_shl,
    tt_shr,
    tt_eq,
    tt_ne,
    tt_le,
    tt_ge,
    tt_and,
    tt_or,
    tt_add_assign,
    tt_sub_assign,
    tt_mul_assign,
    tt_div_assign,
    tt_mod_assign,
    tt_and_assign,
    tt_or_assign,
    tt_xor_assign,
    tt_shl_assign,
    tt_shr_assign,
    tt_arrow,
    tt_namespace,
    tt_const,
    tt_let,
    tt_func,
    tt_if,
    tt_else,
    tt_loop,
    tt_while,
    total_token_count
};

enum {
    predef_act_shift = 0,
    predef_act_reduce = 1,
    act_begin_namespace,
    act_end_block,
    act_concatenate_string_const,
    act_relative_name,
    act_absolute_name,
    act_concatenate_name_path,
    act_bool_const_literal,
    act_int_const_literal,
    act_float_const_literal,
    act_string_const_literal,
    act_expr_op_u_minus,
    act_expr_op_u_plus,
    act_expr_op_add,
    act_expr_op_sub,
    act_expr_op_mul,
    act_expr_op_div,
    act_expr_op_mod,
    act_expr_op_shl,
    act_expr_op_shr,
    act_expr_op_bitwise_not,
    act_expr_op_bitwise_and,
    act_expr_op_bitwise_or,
    act_expr_op_bitwise_xor,
    act_expr_op_eq,
    act_expr_op_ne,
    act_expr_op_lt,
    act_expr_op_le,
    act_expr_op_ge,
    act_expr_op_gt,
    act_expr_op_logical_not,
    act_expr_op_logical_and,
    act_expr_op_logical_or,
    act_expr_op_conditional,
    act_expr_brackets,
    act_expr_name_ref,
    act_define_const,
    act_define_variable,
    act_define_variable_default,
    act_begin_block_expr,
    act_push_expr_result,
    act_discard_expr_result,
    act_if_expr,
    act_if_else_expr,
    act_endless_loop_expr,
    act_loop_expr,
    act_loop_with_else_expr,
    act_begin_func_proto,
    act_end_func_decl,
    act_end_func_def,
    act_add_func_formal_arg,
    act_preproc_op_u_minus,
    act_preproc_op_u_plus,
    act_preproc_op_add,
    act_preproc_op_sub,
    act_preproc_op_mul,
    act_preproc_op_div,
    act_preproc_op_mod,
    act_preproc_op_shl,
    act_preproc_op_shr,
    act_preproc_op_bitwise_not,
    act_preproc_op_bitwise_and,
    act_preproc_op_bitwise_or,
    act_preproc_op_bitwise_xor,
    act_preproc_op_eq,
    act_preproc_op_ne,
    act_preproc_op_lt,
    act_preproc_op_le,
    act_preproc_op_ge,
    act_preproc_op_gt,
    act_preproc_op_logical_not,
    act_preproc_op_logical_and,
    act_preproc_op_logical_or,
    act_preproc_op_conditional,
    act_preproc_brackets,
    act_preproc_operator_begin,
    act_preproc_operator_end,
    total_action_count
};

enum {
    sc_initial = 0,
    sc_preproc_condition
};
