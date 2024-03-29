/* Generated by kalos_idl_compiler - DO NOT MODIFY */

#define K__pstring      kalos_string*
#define K__string       kalos_string
#define K__pnumber      kalos_int
#define K__number       kalos_int
#define K__pbool        kalos_int
#define K__bool         kalos_int
#define K__pobject      kalos_object_ref*
#define K__object       kalos_object_ref
#define K__pany         kalos_value*
#define K__any          kalos_value

#define K__t_string     KALOS_VALUE_STRING
#define K__t_number     KALOS_VALUE_NUMBER
#define K__t_bool       KALOS_VALUE_BOOL
#define K__t_object     KALOS_VALUE_OBJECT
#define K__t_any        KALOS_VALUE_ANY

#define K__final_string(stack, x) if (retval) { push_string(stack, x); } else { kalos_string_release((kalos_state*)state, x);; }
#define K__final_number(stack, x) if (retval) { push_number(stack, x); } else { ;; }
#define K__final_bool(stack, x)   if (retval) { push_bool(stack, x); } else { ;; }
#define K__final_object(stack, x) if (retval) { push_object(stack, x); } else { kalos_object_release((kalos_state*)state, &x);; }
#define K__final_any(stack, x)    if (retval) { push_any(stack, x); } else { kalos_clear((kalos_state*)state, &x);; }
#define K__arg(n, x)              peek(stack, (n)-(x))
#define K__arg_string(x)          &((x)->value.string)
#define K__arg_number(x)          (x)->value.number
#define K__arg_bool(x)            (x)->value.number
#define K__arg_object(x)          &((x)->value.object)
#define K__arg_any(x)             (x)
#define K__match(id, n, s)        (param_count == n && strcmp(name, #s) == 0)

bool kalos_module_idl_builtin_dynamic_dispatch(kalos_run_state* state, const char* name, int param_count, kalos_stack* stack, bool retval) {
	// id=1 name=println arg_count=1 binding=fn kalos_idl_compiler_println
	if (K__match(/*id=*/1, /*args=*/1, /*name=*/println)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_string)) {
			kalos_idl_compiler_println((void*)state, K__arg_string(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=2 name=print arg_count=1 binding=fn kalos_idl_compiler_print
	if (K__match(/*id=*/2, /*args=*/1, /*name=*/print)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_string)) {
			kalos_idl_compiler_print((void*)state, K__arg_string(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=3 name=log arg_count=1 binding=fn kalos_idl_compiler_log
	if (K__match(/*id=*/3, /*args=*/1, /*name=*/log)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_string)) {
			kalos_idl_compiler_log((void*)state, K__arg_string(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	return false;
}

// id=0001:0001 name=main
void kalos_module_idl_sys_trigger_main(kalos_run_state* state, K__pobject a0) {
	kalos_load_arg_object(state, 0, a0);
	kalos_trigger_address(state, kalos_make_address(0x1, 0x1), false);
}

bool kalos_module_idl_sys_dynamic_dispatch(kalos_run_state* state, const char* name, int param_count, kalos_stack* stack, bool retval) {
	// id=1 name=args binding=fn kalos_sys_get_args
	if (K__match(/*id=*/1, /*args=*/0, /*name=*/args)) {
		const int arg_count = 0;
		if (kalos_stack_setup_0(state, arg_count)) {
			K__object x = kalos_sys_get_args((void*)state);
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=2 name=env arg_count=1 binding=fn kalos_sys_get_env
	if (K__match(/*id=*/2, /*args=*/1, /*name=*/env)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_string)) {
			K__string x = kalos_sys_get_env((void*)state, K__arg_string(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=3 name=exit arg_count=1 binding=c exit(@0)
	if (K__match(/*id=*/3, /*args=*/1, /*name=*/exit)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_number)) {
			(exit((K__arg_number(K__arg(arg_count, 1)))));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	return false;
}

bool kalos_module_idl_file_dynamic_dispatch(kalos_run_state* state, const char* name, int param_count, kalos_stack* stack, bool retval) {
	// id=1 name=open arg_count=2 binding=fn kalos_file_open
	if (K__match(/*id=*/1, /*args=*/2, /*name=*/open)) {
		const int arg_count = 2;
		if (kalos_stack_setup_2(state, arg_count, K__t_string, K__t_number)) {
			K__object x = kalos_file_open((void*)state, K__arg_string(K__arg(arg_count, 1)), K__arg_number(K__arg(arg_count, 2)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=2 name=readline arg_count=1 binding=fn kalos_file_readline
	if (K__match(/*id=*/2, /*args=*/1, /*name=*/readline)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_file_readline((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=3 name=read arg_count=1 binding=fn kalos_file_read_all
	if (K__match(/*id=*/3, /*args=*/1, /*name=*/read)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_file_read_all((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=3 name=read arg_count=2 binding=fn kalos_file_read
	if (K__match(/*id=*/3, /*args=*/2, /*name=*/read)) {
		const int arg_count = 2;
		if (kalos_stack_setup_2(state, arg_count, K__t_object, K__t_number)) {
			K__string x = kalos_file_read((void*)state, K__arg_object(K__arg(arg_count, 1)), K__arg_number(K__arg(arg_count, 2)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=4 name=write arg_count=2 binding=fn kalos_file_write
	if (K__match(/*id=*/4, /*args=*/2, /*name=*/write)) {
		const int arg_count = 2;
		if (kalos_stack_setup_2(state, arg_count, K__t_object, K__t_string)) {
			kalos_file_write((void*)state, K__arg_object(K__arg(arg_count, 1)), K__arg_string(K__arg(arg_count, 2)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=5 name=close arg_count=1 binding=fn kalos_file_close
	if (K__match(/*id=*/5, /*args=*/1, /*name=*/close)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			kalos_file_close((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=6 name=stdin binding=fn kalos_file_get_stdin
	if (K__match(/*id=*/6, /*args=*/0, /*name=*/stdin)) {
		const int arg_count = 0;
		if (kalos_stack_setup_0(state, arg_count)) {
			K__object x = kalos_file_get_stdin((void*)state);
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=7 name=stdout binding=fn kalos_file_get_stdout
	if (K__match(/*id=*/7, /*args=*/0, /*name=*/stdout)) {
		const int arg_count = 0;
		if (kalos_stack_setup_0(state, arg_count)) {
			K__object x = kalos_file_get_stdout((void*)state);
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=8 name=stderr binding=fn kalos_file_get_stderr
	if (K__match(/*id=*/8, /*args=*/0, /*name=*/stderr)) {
		const int arg_count = 0;
		if (kalos_stack_setup_0(state, arg_count)) {
			K__object x = kalos_file_get_stderr((void*)state);
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	return false;
}

//kalos_module_idl_kalos_object_idl_props
bool kalos_module_idl_kalos_object_idl_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=1 name=data binding=fn kalos_compiler_get_buffer
	if (K__match(/*id=*/1, /*args=*/0, /*name=*/data)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_compiler_get_buffer((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=2 name=modules binding=fn kalos_idl_get_modules
	if (K__match(/*id=*/2, /*args=*/0, /*name=*/modules)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_idl_get_modules((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=3 name=flags binding=fn kalos_idl_get_flags
	if (K__match(/*id=*/3, /*args=*/0, /*name=*/flags)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__number x = kalos_idl_get_flags((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_number(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_kalos_object_idl_props = {
	.dispatch_name = kalos_module_idl_kalos_object_idl_props_fn
};

//kalos_module_idl_kalos_object_script_props
bool kalos_module_idl_kalos_object_script_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=1 name=data binding=fn kalos_compiler_get_buffer
	if (K__match(/*id=*/1, /*args=*/0, /*name=*/data)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_compiler_get_buffer((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_kalos_object_script_props = {
	.dispatch_name = kalos_module_idl_kalos_object_script_props_fn
};

//kalos_module_idl_kalos_object_lexer_props
bool kalos_module_idl_kalos_object_lexer_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=4 name=peek binding=fn kalos_lexer_peek_token
	if (K__match(/*id=*/4, /*args=*/0, /*name=*/peek)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_lexer_peek_token((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=5 name=read binding=fn kalos_lexer_read_token
	if (K__match(/*id=*/5, /*args=*/0, /*name=*/read)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_lexer_read_token((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_kalos_object_lexer_props = {
	.dispatch_name = kalos_module_idl_kalos_object_lexer_props_fn
};

//kalos_module_idl_kalos_object_token_props
bool kalos_module_idl_kalos_object_token_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=6 name=string binding=fn kalos_lexer_token_read_string
	if (K__match(/*id=*/6, /*args=*/0, /*name=*/string)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_lexer_token_read_string((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=7 name=token binding=fn kalos_lexer_token_read_token
	if (K__match(/*id=*/7, /*args=*/0, /*name=*/token)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_lexer_token_read_token((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=8 name=is_word binding=fn kalos_lexer_token_read_is_word
	if (K__match(/*id=*/8, /*args=*/0, /*name=*/is_word)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__bool x = kalos_lexer_token_read_is_word((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_bool(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_kalos_object_token_props = {
	.dispatch_name = kalos_module_idl_kalos_object_token_props_fn
};

bool kalos_module_idl_kalos_dynamic_dispatch(kalos_run_state* state, const char* name, int param_count, kalos_stack* stack, bool retval) {
	// id=1 name=compile_script arg_count=2 binding=fn kalos_compiler_compile_script
	if (K__match(/*id=*/1, /*args=*/2, /*name=*/compile_script)) {
		const int arg_count = 2;
		if (kalos_stack_setup_2(state, arg_count, K__t_object, K__t_string)) {
			K__object x = kalos_compiler_compile_script((void*)state, K__arg_object(K__arg(arg_count, 1)), K__arg_string(K__arg(arg_count, 2)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=2 name=compile_idl arg_count=1 binding=fn kalos_compiler_compile_idl
	if (K__match(/*id=*/2, /*args=*/1, /*name=*/compile_idl)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_string)) {
			K__object x = kalos_compiler_compile_idl((void*)state, K__arg_string(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=3 name=run_script arg_count=2 binding=fn kalos_compiler_run_script
	if (K__match(/*id=*/3, /*args=*/2, /*name=*/run_script)) {
		const int arg_count = 2;
		if (kalos_stack_setup_2(state, arg_count, K__t_object, K__t_object)) {
			kalos_compiler_run_script((void*)state, K__arg_object(K__arg(arg_count, 1)), K__arg_object(K__arg(arg_count, 2)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=4 name=lex_script arg_count=1 binding=fn kalos_compiler_lex_script
	if (K__match(/*id=*/4, /*args=*/1, /*name=*/lex_script)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_string)) {
			K__object x = kalos_compiler_lex_script((void*)state, K__arg_string(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=5 name=module binding=fn kalos_get_compiler_module
	if (K__match(/*id=*/5, /*args=*/0, /*name=*/module)) {
		const int arg_count = 0;
		if (kalos_stack_setup_0(state, arg_count)) {
			K__object x = kalos_get_compiler_module((void*)state);
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	return false;
}

// id=0004:0001 name=resolve_list_object_prop
K__any kalos_module_idl_kidl_trigger_resolve_list_object_prop(kalos_run_state* state, K__pstring a0, K__pany a1, K__pobject a2) {
	kalos_load_arg_string(state, 0, a0);
	kalos_load_arg_any(state, 1, a1);
	kalos_load_arg_object(state, 2, a2);
	kalos_trigger_address(state, kalos_make_address(0x4, 0x1), true);
	kalos_value return_value = *pop(state->stack);
	return return_value;
}

bool kalos_module_idl_kidl_dynamic_dispatch(kalos_run_state* state, const char* name, int param_count, kalos_stack* stack, bool retval) {
	// id=1 name=walk_modules arg_count=1 binding=fn kalos_idl_walk_modules
	if (K__match(/*id=*/1, /*args=*/1, /*name=*/walk_modules)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_any)) {
			kalos_idl_walk_modules((void*)state, K__arg_any(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=2 name=module binding=fn kalos_idl_set_module
	if (K__match(/*id=*/2, /*args=*/1, /*name=*/module)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			kalos_idl_set_module((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=3 name=prefix binding=fn kalos_idl_module_prefix
	if (K__match(/*id=*/3, /*args=*/0, /*name=*/prefix)) {
		const int arg_count = 0;
		if (kalos_stack_setup_0(state, arg_count)) {
			K__string x = kalos_idl_module_prefix((void*)state);
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=4 name=create_list_object arg_count=2 binding=fn kalos_idl_create_list_object
	if (K__match(/*id=*/4, /*args=*/2, /*name=*/create_list_object)) {
		const int arg_count = 2;
		if (kalos_stack_setup_2(state, arg_count, K__t_string, K__t_object)) {
			K__object x = kalos_idl_create_list_object((void*)state, K__arg_string(K__arg(arg_count, 1)), K__arg_object(K__arg(arg_count, 2)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	return false;
}

// id=0005:0001 name=begin
void kalos_module_idl_module_trigger_begin(kalos_run_state* state, K__pany a0, K__pobject a1) {
	kalos_load_arg_any(state, 0, a0);
	kalos_load_arg_object(state, 1, a1);
	kalos_trigger_address(state, kalos_make_address(0x5, 0x1), false);
}

// id=0005:0002 name=function
void kalos_module_idl_module_trigger_function(kalos_run_state* state, K__pany a0, K__pobject a1) {
	kalos_load_arg_any(state, 0, a0);
	kalos_load_arg_object(state, 1, a1);
	kalos_trigger_address(state, kalos_make_address(0x5, 0x2), false);
}

// id=0005:0003 name=handler
void kalos_module_idl_module_trigger_handler(kalos_run_state* state, K__pany a0, K__pobject a1) {
	kalos_load_arg_any(state, 0, a0);
	kalos_load_arg_object(state, 1, a1);
	kalos_trigger_address(state, kalos_make_address(0x5, 0x3), false);
}

// id=0005:0004 name=property
void kalos_module_idl_module_trigger_property(kalos_run_state* state, K__pany a0, K__pobject a1) {
	kalos_load_arg_any(state, 0, a0);
	kalos_load_arg_object(state, 1, a1);
	kalos_trigger_address(state, kalos_make_address(0x5, 0x4), false);
}

// id=0005:0005 name=object
void kalos_module_idl_module_trigger_object(kalos_run_state* state, K__pany a0, K__pobject a1) {
	kalos_load_arg_any(state, 0, a0);
	kalos_load_arg_object(state, 1, a1);
	kalos_trigger_address(state, kalos_make_address(0x5, 0x5), false);
}

//kalos_module_idl_module_object_idl_obj_props
bool kalos_module_idl_module_object_idl_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=9 name=class binding=c kalos_string_allocate(kalos_state_from_run_state(state), "")
	if (K__match(/*id=*/9, /*args=*/0, /*name=*/class)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (kalos_string_allocate(kalos_state_from_run_state(state), ""));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=10 name=prefix binding=c kalos_string_allocate(kalos_state_from_run_state(state), "")
	if (K__match(/*id=*/10, /*args=*/0, /*name=*/prefix)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (kalos_string_allocate(kalos_state_from_run_state(state), ""));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=11 name=dispatch binding=c kalos_string_allocate(kalos_state_from_run_state(state), "")
	if (K__match(/*id=*/11, /*args=*/0, /*name=*/dispatch)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (kalos_string_allocate(kalos_state_from_run_state(state), ""));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=2 name=modules binding=c kalos_string_allocate(kalos_state_from_run_state(state), "")
	if (K__match(/*id=*/2, /*args=*/0, /*name=*/modules)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (kalos_string_allocate(kalos_state_from_run_state(state), ""));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_idl_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_idl_obj_props_fn
};

//kalos_module_idl_module_object_module_obj_props
bool kalos_module_idl_module_object_module_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=12 name=name binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/12, /*args=*/0, /*name=*/name)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=13 name=items binding=c kalos_string_allocate(kalos_state_from_run_state(state), "")
	if (K__match(/*id=*/13, /*args=*/0, /*name=*/items)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (kalos_string_allocate(kalos_state_from_run_state(state), ""));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=14 name=invoke_id binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/14, /*args=*/0, /*name=*/invoke_id)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_module_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_module_obj_props_fn
};

//kalos_module_idl_module_object_const_obj_props
bool kalos_module_idl_module_object_const_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=12 name=name binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/12, /*args=*/0, /*name=*/name)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=15 name=type binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/15, /*args=*/0, /*name=*/type)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=16 name=value binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/16, /*args=*/0, /*name=*/value)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_const_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_const_obj_props_fn
};

//kalos_module_idl_module_object_function_obj_props
bool kalos_module_idl_module_object_function_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=12 name=name binding=fn kalos_idl_export_name
	if (K__match(/*id=*/12, /*args=*/0, /*name=*/name)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_export_name((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=17 name=overloads binding=fn kalos_idl_function_overloads
	if (K__match(/*id=*/17, /*args=*/0, /*name=*/overloads)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_idl_function_overloads((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_function_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_function_obj_props_fn
};

//kalos_module_idl_module_object_overload_obj_props
bool kalos_module_idl_module_object_overload_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=18 name=return_type binding=fn kalos_idl_overload_return_type
	if (K__match(/*id=*/18, /*args=*/0, /*name=*/return_type)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_overload_return_type((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=19 name=args binding=fn kalos_idl_overload_args
	if (K__match(/*id=*/19, /*args=*/0, /*name=*/args)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_idl_overload_args((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=20 name=arg_count binding=fn kalos_idl_overload_arg_count
	if (K__match(/*id=*/20, /*args=*/0, /*name=*/arg_count)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__number x = kalos_idl_overload_arg_count((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_number(stack, x);
			return true;
		}
	}

	// id=21 name=varargs binding=fn kalos_idl_overload_varargs
	if (K__match(/*id=*/21, /*args=*/0, /*name=*/varargs)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_overload_varargs((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=22 name=binding binding=fn kalos_idl_overload_binding
	if (K__match(/*id=*/22, /*args=*/0, /*name=*/binding)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_idl_overload_binding((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_overload_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_overload_obj_props_fn
};

//kalos_module_idl_module_object_arg_obj_props
bool kalos_module_idl_module_object_arg_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=12 name=name binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/12, /*args=*/0, /*name=*/name)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=15 name=type binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/15, /*args=*/0, /*name=*/type)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=23 name=is_vararg binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/23, /*args=*/0, /*name=*/is_vararg)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_arg_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_arg_obj_props_fn
};

//kalos_module_idl_module_object_property_obj_props
bool kalos_module_idl_module_object_property_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=12 name=name binding=fn kalos_idl_property_name
	if (K__match(/*id=*/12, /*args=*/0, /*name=*/name)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_property_name((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=15 name=type binding=fn kalos_idl_property_type
	if (K__match(/*id=*/15, /*args=*/0, /*name=*/type)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_property_type((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=24 name=read_binding binding=fn kalos_idl_property_read_binding
	if (K__match(/*id=*/24, /*args=*/0, /*name=*/read_binding)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_idl_property_read_binding((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=25 name=write_binding binding=fn kalos_idl_property_write_binding
	if (K__match(/*id=*/25, /*args=*/0, /*name=*/write_binding)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_idl_property_write_binding((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_property_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_property_obj_props_fn
};

//kalos_module_idl_module_object_handler_obj_props
bool kalos_module_idl_module_object_handler_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=12 name=name binding=fn kalos_idl_export_name
	if (K__match(/*id=*/12, /*args=*/0, /*name=*/name)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_export_name((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=26 name=module_index binding=fn kalos_idl_handler_module_index
	if (K__match(/*id=*/26, /*args=*/0, /*name=*/module_index)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__number x = kalos_idl_handler_module_index((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_number(stack, x);
			return true;
		}
	}

	// id=27 name=handler_index binding=fn kalos_idl_handler_index
	if (K__match(/*id=*/27, /*args=*/0, /*name=*/handler_index)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__number x = kalos_idl_handler_index((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_number(stack, x);
			return true;
		}
	}

	// id=19 name=args binding=fn kalos_idl_handler_args
	if (K__match(/*id=*/19, /*args=*/0, /*name=*/args)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__object x = kalos_idl_handler_args((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_object(stack, x);
			return true;
		}
	}

	// id=18 name=return_type binding=fn kalos_idl_handler_return_type
	if (K__match(/*id=*/18, /*args=*/0, /*name=*/return_type)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_handler_return_type((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=14 name=invoke_id binding=c IDL_STRING(IDL_OBJECT(kalos_module)->name_index)
	if (K__match(/*id=*/14, /*args=*/0, /*name=*/invoke_id)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (IDL_STRING(IDL_OBJECT(kalos_module)->name_index));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_handler_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_handler_obj_props_fn
};

//kalos_module_idl_module_object_object_obj_props
bool kalos_module_idl_module_object_object_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=12 name=name binding=fn kalos_idl_export_name
	if (K__match(/*id=*/12, /*args=*/0, /*name=*/name)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_export_name((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=13 name=items binding=c kalos_string_allocate(kalos_state_from_run_state(state), "")
	if (K__match(/*id=*/13, /*args=*/0, /*name=*/items)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = (kalos_string_allocate(kalos_state_from_run_state(state), ""));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_object_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_object_obj_props_fn
};

//kalos_module_idl_module_object_binding_obj_props
bool kalos_module_idl_module_object_binding_obj_props_fn(kalos_run_state* state, kalos_object_ref* object, const char* name, int param_count, kalos_stack* stack) {
	bool retval = true;
	// id=15 name=type binding=fn kalos_idl_binding_type
	if (K__match(/*id=*/15, /*args=*/0, /*name=*/type)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_binding_type((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=16 name=value binding=fn kalos_idl_binding_value
	if (K__match(/*id=*/16, /*args=*/0, /*name=*/value)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__string x = kalos_idl_binding_value((void*)state, K__arg_object(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			K__final_string(stack, x);
			return true;
		}
	}

	// id=14 name=invoke_id binding=c IDL_OBJECT(kalos_binding)->invoke_id
	if (K__match(/*id=*/14, /*args=*/0, /*name=*/invoke_id)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_object)) {
			K__number x = (IDL_OBJECT(kalos_binding)->invoke_id);
			kalos_stack_cleanup(state, arg_count);
			K__final_number(stack, x);
			return true;
		}
	}

	return false;
}

kalos_object_dispatch kalos_module_idl_module_object_binding_obj_props = {
	.dispatch_name = kalos_module_idl_module_object_binding_obj_props_fn
};

bool kalos_module_idl_module_dynamic_dispatch(kalos_run_state* state, const char* name, int param_count, kalos_stack* stack, bool retval) {
	// id=1 name=walk_exports arg_count=1 binding=fn kalos_idl_walk_exports
	if (K__match(/*id=*/1, /*args=*/1, /*name=*/walk_exports)) {
		const int arg_count = 1;
		if (kalos_stack_setup_1(state, arg_count, K__t_any)) {
			kalos_idl_walk_exports((void*)state, K__arg_any(K__arg(arg_count, 1)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	// id=2 name=walk_object_properties arg_count=2 binding=fn kalos_idl_walk_object_properties
	if (K__match(/*id=*/2, /*args=*/2, /*name=*/walk_object_properties)) {
		const int arg_count = 2;
		if (kalos_stack_setup_2(state, arg_count, K__t_any, K__t_object)) {
			kalos_idl_walk_object_properties((void*)state, K__arg_any(K__arg(arg_count, 1)), K__arg_object(K__arg(arg_count, 2)));
			kalos_stack_cleanup(state, arg_count);
			return true;
		}
	}

	return false;
}

bool kalos_module_idl_dynamic_dispatch(kalos_run_state* state, const char* module, const char* name, int param_count, kalos_stack* stack, bool retval) {
	if (strcmp(module, "builtin") == 0) { return kalos_module_idl_builtin_dynamic_dispatch(state, name, param_count, stack, retval); }
	if (strcmp(module, "sys") == 0) { return kalos_module_idl_sys_dynamic_dispatch(state, name, param_count, stack, retval); }
	if (strcmp(module, "file") == 0) { return kalos_module_idl_file_dynamic_dispatch(state, name, param_count, stack, retval); }
	if (strcmp(module, "kalos") == 0) { return kalos_module_idl_kalos_dynamic_dispatch(state, name, param_count, stack, retval); }
	if (strcmp(module, "kidl") == 0) { return kalos_module_idl_kidl_dynamic_dispatch(state, name, param_count, stack, retval); }
	if (strcmp(module, "module") == 0) { return kalos_module_idl_module_dynamic_dispatch(state, name, param_count, stack, retval); }

	LOG("Module %s not found", module);
	return false;
}

#undef K__pstring
#undef K__string
#undef K__pnumber
#undef K__number
#undef K__pbool
#undef K__bool
#undef K__pobject
#undef K__object
#undef K__pany
#undef K__any
#undef K__t_string
#undef K__t_number
#undef K__t_bool
#undef K__t_object
#undef K__t_any
#undef K__final_string
#undef K__final_number
#undef K__final_bool
#undef K__final_object
#undef K__final_any
#undef K__arg
#undef K__arg_string
#undef K__arg_number
#undef K__arg_bool
#undef K__arg_object
#undef K__arg_any
#undef K__match
