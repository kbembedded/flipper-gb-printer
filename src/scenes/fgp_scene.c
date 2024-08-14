// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#include "include/fgp_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const fgp_on_enter_handlers[])(void*) = {
#include "include/fgp_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const fgp_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "include/fgp_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const fgp_on_exit_handlers[])(void* context) = {
#include "include/fgp_scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers fgp_scene_handlers = {
    .on_enter_handlers = fgp_on_enter_handlers,
    .on_event_handlers = fgp_on_event_handlers,
    .on_exit_handlers = fgp_on_exit_handlers,
    .scene_num = fgpSceneNum,
};
