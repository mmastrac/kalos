void kalos_module_dispatch_builtin(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=0 name=print arg_count=1 symbol=kalos_idl_compiler_print
	case 0: {
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

void kalos_module_dispatch_module(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=0 name=name arg_count=0 symbol=kalos_idl_module_name
	case 0: {
		LOG("Invoke %s", "name");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_module_name(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	}
}

void kalos_module_dispatch_function(kalos_state state, int function, kalos_stack* stack) {
	switch (function) {
	// id=0 name=arg_count arg_count=0 symbol=kalos_idl_function_arg_count
	case 0: {
		LOG("Invoke %s", "arg_count");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_int x = kalos_idl_function_arg_count(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_number(stack, x);
		break;
	}
	// id=1 name=arg_type arg_count=1 symbol=kalos_idl_function_arg_type
	case 1: {
		LOG("Invoke %s", "arg_type");
		int arg_count = 1;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_function_arg_type(state, peek(stack, -1)->value.number);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	// id=2 name=args arg_count=0 symbol=kalos_idl_function_args
	case 2: {
		LOG("Invoke %s", "args");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_object* x = kalos_idl_function_args(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_object(stack, x);
		break;
	}
	// id=3 name=id arg_count=0 symbol=kalos_idl_function_id
	case 3: {
		LOG("Invoke %s", "id");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_int x = kalos_idl_function_id(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_number(stack, x);
		break;
	}
	// id=4 name=name arg_count=0 symbol=kalos_idl_function_name
	case 4: {
		LOG("Invoke %s", "name");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_function_name(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	// id=5 name=realname arg_count=0 symbol=kalos_idl_function_realname
	case 5: {
		LOG("Invoke %s", "realname");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_function_realname(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	// id=6 name=return_type arg_count=0 symbol=kalos_idl_function_return_type
	case 6: {
		LOG("Invoke %s", "return_type");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_function_return_type(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	// id=7 name=type arg_count=0 symbol=kalos_idl_function_type
	case 7: {
		LOG("Invoke %s", "type");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_function_type(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	// id=8 name=varargs arg_count=0 symbol=kalos_idl_function_varargs
	case 8: {
		LOG("Invoke %s", "varargs");
		int arg_count = 0;
		kalos_stack_fixup_no_varargs(arg_count, stack);
		kalos_string x = kalos_idl_function_varargs(state);
		kalos_stack_cleanup_no_varargs(arg_count, 0, state, stack);
		push_string(stack, x);
		break;
	}
	}
}

