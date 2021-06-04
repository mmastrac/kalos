// id=: name=begin_module
// id=: name=end_module
// id=: name=function_export
// id=: name=handle_export
// id=: name=open
// id=: name=property_export
void kalos_module_idl_builtin(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=2 name=print arg_count=1 symbol=kalos_idl_compiler_print
	case 2: {
		LOG("Invoke %s", "print");
		int arg_count = 1;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_idl_compiler_print(state, &peek(stack, -1)->value.string);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		break;
	}
	// id=1 name=println arg_count=1 symbol=kalos_idl_compiler_println
	case 1: {
		LOG("Invoke %s", "println");
		int arg_count = 1;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_idl_compiler_println(state, &peek(stack, -1)->value.string);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		break;
	}
	}
}

// id=: name=begin
// id=: name=end
void kalos_module_idl_module(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=1 name=name symbol=kalos_idl_module_name
	case 1: {
		push_string(stack, kalos_idl_module_name(state));
		break;
	}
	// id=2 name=prefix symbol=kalos_idl_module_prefix
	case 2: {
		push_string(stack, kalos_idl_module_prefix(state));
		break;
	}
	}
}

void kalos_module_idl_function(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=6 name=arg_count symbol=kalos_idl_function_arg_count
	case 6: {
		push_number(stack, kalos_idl_function_arg_count(state));
		break;
	}
	// id=8 name=arg_type arg_count=1 symbol=kalos_idl_function_arg_type
	case 8: {
		LOG("Invoke %s", "arg_type");
		int arg_count = 1;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_function_arg_type(state, peek(stack, -1)->value.number);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	// id=5 name=args symbol=kalos_idl_function_args
	case 5: {
		push_object(stack, kalos_idl_function_args(state));
		break;
	}
	// id=1 name=id symbol=kalos_idl_function_id
	case 1: {
		push_number(stack, kalos_idl_function_id(state));
		break;
	}
	// id=3 name=name symbol=kalos_idl_function_name
	case 3: {
		push_string(stack, kalos_idl_function_name(state));
		break;
	}
	// id=4 name=realname symbol=kalos_idl_function_realname
	case 4: {
		push_string(stack, kalos_idl_function_realname(state));
		break;
	}
	// id=2 name=return_type symbol=kalos_idl_function_return_type
	case 2: {
		push_string(stack, kalos_idl_function_return_type(state));
		break;
	}
	// id=7 name=varargs symbol=kalos_idl_function_varargs
	case 7: {
		push_string(stack, kalos_idl_function_varargs(state));
		break;
	}
	}
}

void kalos_module_idl_property(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=1 name=name symbol=kalos_idl_function_name
	case 1: {
		push_string(stack, kalos_idl_function_name(state));
		break;
	}
	// id=2 name=read_id symbol=kalos_idl_property_read_id
	case 2: {
		push_number(stack, kalos_idl_property_read_id(state));
		break;
	}
	// id=5 name=read_symbol symbol=kalos_idl_property_read_symbol
	case 5: {
		push_string(stack, kalos_idl_property_read_symbol(state));
		break;
	}
	// id=4 name=type symbol=kalos_idl_property_type
	case 4: {
		push_string(stack, kalos_idl_property_type(state));
		break;
	}
	// id=3 name=write_id symbol=kalos_idl_property_write_id
	case 3: {
		push_number(stack, kalos_idl_property_write_id(state));
		break;
	}
	// id=6 name=write_symbol symbol=kalos_idl_property_write_symbol
	case 6: {
		push_string(stack, kalos_idl_property_write_symbol(state));
		break;
	}
	}
}

void kalos_module_idl_handles(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=3 name=handle_index symbol=kalos_idl_handle_index
	case 3: {
		push_number(stack, kalos_idl_handle_index(state));
		break;
	}
	// id=2 name=module_index symbol=kalos_idl_handle_module_index
	case 2: {
		push_number(stack, kalos_idl_handle_module_index(state));
		break;
	}
	// id=1 name=name symbol=kalos_idl_function_name
	case 1: {
		push_string(stack, kalos_idl_function_name(state));
		break;
	}
	}
}

