#pragma once

/* Initialize a namespace. */
void kalos_namespace_init();

/* Push a new namespace. */
void kalos_namespace_push();

/* Pop a namespace. */
void kalos_namespace_pop();

/* Register a name inside of the current namespace. */
void* kalos_namespace_register(const char* name, int type, size_t context_size);

void kalos_lookup(const char* name);
