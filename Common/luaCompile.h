#pragma once

int compileLua(const char* name, const char* src, int size, char* dest, bool isStripLua);
void initLua();
void closeLua();
