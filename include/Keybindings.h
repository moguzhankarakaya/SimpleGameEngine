#pragma once
#ifndef KEYBINDING_G
#include <windows.h>

#include "ConfigParser.h"

class KeyBindings
{
public:
	static void destroyBindings();
	static KeyBindings* getBindings();
	
	UCHAR getPlasmaKey() { return plasma; }
	UCHAR getGatlingKey() { return gatling; }
	
	void changeGatlingKey(UCHAR _gatling) { gatling = _gatling; }
	void changePlasmeKey(UCHAR _plasma) { plasma = _plasma; }

private:
	// Only one instance of set of key bindings will be allowed

	static KeyBindings* bindings;
	
	KeyBindings();

	// Key bindings
	UCHAR gatling;
	UCHAR plasma;

};

#endif // KEYBINDING_G
