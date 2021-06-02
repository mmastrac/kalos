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

void kalos_module_idl_module(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	case 1: {
		push_string(stack, kalos_idl_module_name(state));
		break;
	}
	case 2: {
		push_string(stack, kalos_idl_module_prefix(state));
		break;
	}
	}
}

void kalos_module_idl_function(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
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
	case 5: {
		push_object(stack, kalos_idl_function_args(state));
		break;
	}
	case 1: {
		push_number(stack, kalos_idl_function_id(state));
		break;
	}
	case 3: {
		push_string(stack, kalos_idl_function_name(state));
		break;
	}
	case 4: {
		push_string(stack, kalos_idl_function_realname(state));
		break;
	}
	case 2: {
		push_string(stack, kalos_idl_function_return_type(state));
		break;
	}
	case 7: {
		push_string(stack, kalos_idl_function_varargs(state));
		break;
	}
	}
}

void kalos_module_idl_property(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	case 1: {
		push_string(stack, kalos_idl_function_name(state));
		break;
	}
	case 2: {
		push_number(stack, kalos_idl_property_read_id(state));
		break;
	}
	case 5: {
		push_string(stack, kalos_idl_property_read_symbol(state));
		break;
	}
	case 4: {
		push_string(stack, kalos_idl_property_type(state));
		break;
	}
	case 3: {
		push_number(stack, kalos_idl_property_write_id(state));
		break;
	}
	case 6: {
		push_string(stack, kalos_idl_property_write_symbol(state));
		break;
	}
	}
}

