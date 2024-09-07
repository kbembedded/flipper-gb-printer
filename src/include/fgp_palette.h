#ifndef FGP_PALETTE_H
#define FGP_PALETTE_H

#pragma once

#include <furi.h>

typedef enum {
    BlackAndWhite,
    Original,
    SplashUp,
    GBLight,
    Pocket,
    AudiQuattroPikesPeak,
    AzureClouds,
    Theresalwaysmoney,
    BGBEmulator,
    GameBoyBlackZeropalette,
    CandyCottonTowerRaid,
    CaramelFudgeParanoia,
    CGAPaletteCrush1,
    CGAPaletteCrush2,
    ChildhoodinGreenland,
    CMYKeystone,
    CyanideBlues,
    Dune2000remastered,
    Drowningatnight,
    DeepHazeGreen,
    DiesistmeineWassermelone,
    Flowerfeldstrabe,
    FloydSteinberginLove,
    GameBoyColorSplashDown,
    GameBoyColorSplashDownA,
    GameBoyColorSplashDownB,
    GameBoyColorSplashRightAGameBoyCamera,
    GameBoyColorSplashLeft,
    GameBoyColorSplashLeftA,
    GameBoyColorSplashLeftB,
    GameBoyColorSplashRight,
    GameBoyColorSplashRightB,
    GameBoyColorSplashUpA,
    GameBoyColorSplashUpB,
    GoldenElephantCurry,
    GlowingMountains,
    GrafixkidGray,
    GrafixkidGreen,
    ArtisticCaffeinatedLactose,
    KneeDeepintheWood,
    LinkslateAwakening,
    MetroidAranremixed,
    NortoriousComandante,
    PurpleRain,
    RustedCitySign,
    RomerosGarden,
    SunflowerHolidays,
    SuperHyperMegaGameboy,
    SpaceHazeOverload,
    StarlitMemories,
    MyFriendfromBavaria,
    ThedeathofYungColumbus,
    TramontoalParcodegliAcquedotti,
    Thestarryknight,
    VirtualBoy1985,
    WaterfrontPlaza,
    YouthIkarusreloaded
} PaletteItem;


typedef struct {
    uint32_t palette_color_hex_a;
    uint32_t palette_color_hex_b;
    uint32_t palette_color_hex_c;
    uint32_t palette_color_hex_d;
} Palette;

extern Palette palettes[];
#define PALETTE_COUNT (sizeof(palettes) / sizeof(palettes[0]))
#endif // FGP_PALETTE_H