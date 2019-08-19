#include "Keybindings.h"

KeyBindings* KeyBindings::bindings = nullptr;

void KeyBindings::destroyBindings()
{
	delete bindings;
}

KeyBindings::KeyBindings()
{	
	// Default initialization
	gatling = 0x10;
	plasma  = 0x20;
}

KeyBindings* KeyBindings::getBindings()
{
	if (bindings == nullptr)
		bindings = new KeyBindings();
	return bindings;
}