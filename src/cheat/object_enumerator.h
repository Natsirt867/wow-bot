#pragma once
#include "../common/common.h"


#ifndef OBJECT_ENUMERATOR_H
#define OBJECT_ENUMERATOR_H

int __stdcall enumerate_callback(unsigned long long  guid);


void click_to_move_func();
void click_to_move_stop_func();
void set_target_func();
int read_int(void* ptr);
#endif //OBJECT_ENUMERATOR_H
