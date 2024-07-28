#pragma once

class CAttributeList;
class CGameEntitySystem;
class CGlobalVars;

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
typedef void (*CAttributeListSetOrAddAttributeValueByName)(CAttributeList&, const char*, float);
typedef void (*ToolsInspectWeaponPostBuild)(void*);

extern CAttributeListSetOrAddAttributeValueByName g_pSetAttribute;
CGameEntitySystem* GameEntitySystem();
CGlobalVars* GetGameGlobals();