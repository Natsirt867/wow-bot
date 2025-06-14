#pragma once

#ifndef DIRECT_X_MGR_H
#define DIRECT_X_MGR_H
#include <stdbool.h>

void DirectX_ThrottleFPS(void);
bool DirectX_InstallHooks(void);
void DirectX_RemoveHooks(void);

#endif //DIRECT_X_MGR_H
