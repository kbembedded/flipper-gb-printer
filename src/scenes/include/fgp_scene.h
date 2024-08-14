// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#ifndef FGP_SCENE_H
#define FGP_SCENE_H

#pragma once

#include <gui/scene_manager.h>

// Generate scene id and total number
#define ADD_SCENE(prefix, name, id) fgpScene##id,
typedef enum {
#include "fgp_scene_config.h"
    fgpSceneNum,
    /* Magic number to send on an event to search through scene history and
     * change to a previous scene.
     */
    fgpSceneSearch = (1 << 30),
    /* Magic number to send on an event to trigger going back a scene */
    fgpSceneBack = (1 << 31),
} fgpScene;
#undef ADD_SCENE

extern const SceneManagerHandlers fgp_scene_handlers;

// Generate scene on_enter handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "fgp_scene_config.h"
#undef ADD_SCENE

// Generate scene on_event handlers declaration
#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "fgp_scene_config.h"
#undef ADD_SCENE

// Generate scene on_exit handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "fgp_scene_config.h"
#undef ADD_SCENE

#endif // FGP_SCENE_H
