#include "global.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "sprite.h"
#include "task.h"
#include "sound.h"
#include "constants/songs.h"
#include "string_util.h"
#include "text.h"
#include "speedchoice.h"
#include "gpu_regs.h"
#include "bg.h"
#include "scanline_effect.h"
#include "text_window.h"
#include "text_window_graphics.h"
#include "done_button.h"
#include "naming_screen.h"
#include "random.h"
#include "new_menu_helpers.h"

// We reuse the option menu and main menu palette data to simplfy things.
extern u16 sOptionMenuPalette[1];
extern u16 sMainMenuTextPal[16];

// ----------------------------------------------
// Window IDs enumeration for the defined
// Speedchoice menu templates.
// ----------------------------------------------
enum
{
    SPD_WIN_TEXT_OPTION,
    SPD_WIN_OPTIONS,
    SPD_WIN_TOOLTIP,
    SPD_WIN_YESNO,
};

// ----------------------------------------------
// BG TEMPLATES
// ----------------------------------------------
const struct BgTemplate sSpeedchoiceMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    // 0x000001ec (I forget why this was put here)
    {
        .bg = 2,
        .charBaseIndex = 1,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

const struct WindowTemplate sSpeedchoiceMenuWinTemplates[] =
{
    [SPD_WIN_TEXT_OPTION] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [SPD_WIN_OPTIONS]     = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 54
    },
    [SPD_WIN_TOOLTIP]     = {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 427
    },
    [SPD_WIN_YESNO]       = {
        .bg = 2,
        .tilemapLeft = 23,
        .tilemapTop = 9,
        .width = 4,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 531
    }, DUMMY_WIN_TEMPLATE
};

enum
{
    SPC_COLOR_GREY,
    SPC_COLOR_RED,
    SPC_COLOR_BLUE,
    SPC_COLOR_GREEN,
};

static const u8 sTextColors[][3] = {
    [SPC_COLOR_GREY]  = { TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GREY, TEXT_COLOR_LIGHT_GREY },
    [SPC_COLOR_RED]   = { TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED },
    [SPC_COLOR_BLUE]  = { TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE },
    [SPC_COLOR_GREEN] = { TEXT_COLOR_WHITE, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN },
};

/* ----------------------------------------------- */
/* SPEEDCHOICE MENU TEXT (System Text)             */
/* ----------------------------------------------- */
const u8 gSystemText_TerminatorS[] = _("$");

/* ----------------------------------------------- */
/* SPEEDCHOICE MENU TEXT (Header Text)             */
/* ----------------------------------------------- */
const u8 gSpeedchoiceTextHeader[] = _("SPEEDCHOICE MENU");
const u8 gSpeedchoiceCurrentVersion[] = _("v" SPEEDCHOICE_VERSION);

/* ----------------------------------------------- */
/* SPEEDCHOICE MENU TEXT (Option Choices)          */
/* ----------------------------------------------- */
const u8 gSpeedchoiceTextYes[]    = _("YES");
const u8 gSpeedchoiceTextNo[]     = _("NO");

const u8 gSpeedchoiceTextOn[]     = _("ON");
const u8 gSpeedchoiceTextOff[]    = _("OFF");

const u8 gSpeedchoiceTextNerf[]   = _("PURGE");
const u8 gSpeedchoiceTextKeep[]   = _("KEEP");
const u8 gSpeedchoiceTextHell[]   = _("HELL");
const u8 gSpeedchoiceTextWhy[]   = _("WHY");

const u8 gSpeedchoiceTextSemi[]   = _("SEMI");
const u8 gSpeedchoiceTextFull[]   = _("FULL");

const u8 gSpeedchoiceTextStatic[] = _("SAME");
const u8 gSpeedchoiceTextRand[]   = _("RAND");
const u8 gSpeedchoiceTextSane[]   = _("SANE");

const u8 gSpeedchoiceTextBW[]     = _("BW");
const u8 gSpeedchoiceTextNone[]   = _("NONE");

const u8 gSpeedchoiceTextTutor[] = _("TUTOR");
const u8 gSpeedchoiceTextHM05[]   = _("HM05");

const u8 gSpeedchoiceTextManual[] = _("MANUAL");
const u8 gSpeedchoiceTextHoF[] = _("HOF");
const u8 gSpeedchoiceTextE4R2[] = _("E4R2");

/* ----------------------------------------------- */
/* SPEEDCHOICE MENU TEXT (Option Names)            */
/* ----------------------------------------------- */

// PAGE 1
const u8 gSpeedchoiceOptionPreset[] = _("PRESET");
const u8 gSpeedchoiceOptionName[] = _("NAME");
const u8 gSpeedchoiceOptionEXP[] = _("EXP");
const u8 gSpeedchoiceOptionPlotless[] = _("PLOTLESS");
const u8 gSpeedchoiceOptionEarlySaffron[] = _("EARLY SAFFRON");

// PAGE 2
const u8 gSpeedchoiceOptionRaceGoal[] = _("RACE GOAL");
const u8 gSpeedchoiceOptionSpinners[] = _("SPINNERS");
const u8 gSpeedchoiceOptionEarlySurf[] = _("EARLY SURF");
const u8 gSpeedchoiceOptionMaxVision[] = _("MAX VISION");
const u8 gSpeedchoiceOptionGoodEarlyWilds[] = _("GOOD EARLY WILDS");

// PAGE 3
const u8 gSpeedchoiceOptionEasyFalseSwipe[] = _("EASY FALSE SWIPE");
const u8 gSpeedchocieOptionEasyDexRewards[] = _("EASY DEX REWARDS");
const u8 gSpeedchoiceOptionFastCatch[] = _("FAST CATCH");
const u8 gSpeedchoiceOptionEarlyBike[] = _("EARLY BIKE");
const u8 gSpeedchoiceOptionGen7XItems[] = _("GEN 7 X ITEMS");

// PAGE 4
const u8 gSpeedchoiceOptionEvoEveryLv[] = _("EVO EVERY LV");
const u8 gSpeedchoiceOptionHmBadgeChk[] = _("HM BADGE CHK");
const u8 gSpeedchoiceOptionEasySurgeCans[] = _("EASY SURGE");
const u8 gSpeedchoiceOptionNerfBrock[] = _("NERF BROCK");

// CONSTANT OPTIONS
const u8 gSpeedchoiceOptionPage[] = _("PAGE");
const u8 gSpeedchoiceOptionStartGame[] = _("START GAME");

// ARROWS
const u8 gSpeedchoiceOptionLeftArrow[] = _("{LEFT_ARROW}");
const u8 gSpeedchoiceOptionRightArrow[] = _("{RIGHT_ARROW}");

/* ----------------------------------------------- */
/* SPEEDCHOICE MENU TEXT (Tooltip Text)            */
/* ----------------------------------------------- */

// INTRODUCTION
const u8 gSpeedchoiceTooltipExplanation[] = _("This is the Speedchoice menu where\nvarious options can be selected.\pTo get an explanation of each option,\npress SELECT when over the option.");

// TOOLTIPS
const u8 gSpeedchoiceTooltipPreset[] = _("Sets of predetermined options\nthat help speedrunners quickly select\pa set of options for a type of\nspeedrunning category.");
const u8 gSpeedchoiceTooltipName[] = _("Set your trainer name here.");
const u8 gSpeedchoiceTooltipEXP[] = _("Modifies the experience system\nto the desired input.\pCan be Gen 5, or\nno exp at all.");
const u8 gSpeedchoiceTooltipRaceGoal[] = _("Sets the endpoint of the race.\pMANUAL: Puts a DONE BUTTON\nin your bag.\pHOF: Elite Four Round 1\nE4R2: Elite Four Round 2");
const u8 gSpeedchoiceTooltipEarlySaffron[] = _("Affects whether the guards have\ntheir thirst quenched after\lCERULEAN RIVAL.");
const u8 gSpeedchoiceTooltipPlotless[] = _("Affects whether the Rocket sections\nare required for progression.\pKEEP: Everything is vanilla\nSEMI: Plot items are elsewhere\pFULL: Same as SEMI but the\nGRUNTs are removed too.");
const u8 gSpeedchoiceTooltipSpinners[] = _("PURGE: Makes spinners on a static\nspinning pattern at a fixed rate.\pHELL: Rapidly spins\nevery spinner every frame.\pWHY: Same as HELL but 4 frames\ninstead of 16.\pHELL and WHY also fix bag\nmanip.");
const u8 gSpeedchoiceTooltipMaxVision[] = _("SANE: Will extend trainer vision\nto 8, but prevent trainers from\pwalking through walls or solid\nobjects.\pHELL: No collision or\nelevation detection.");
const u8 gSpeedchoiceTooltipGoodEarlyWilds[] = _("SAME: Depending on the\nrandomizer check value, wild\pencounters in the grass for\n{PKMN} below lv 10 will have\ltheir final evolution.\pRAND: If they have a branching\nevolution, it will be randomly\lgenerated instead of being static.");
const u8 gSpeedchoiceTooltipEarlySurf[] = _("Receive HM03 from Rival 2\ninstead of FAME CHECKER.");
const u8 gSpeedchoiceTooltipEasyFalseSwipe[] = _("Makes FALSE SWIPE guaranteed\nOFF: Vanilla game behavior\pTUTOR: The tutor in Silph 2F teaches\nFALSE SWIPE.\pHM05: Replaces HM05 FLASH with\nHM05 FALSE SWIPE.");
const u8 gSpeedchoiceTooltipFastCatch[] = _("All Pokeballs are guaranteed to\ncatch.");
const u8 gSpeedchoiceTooltipEarlyBike[] = _("Start game with Bicycle.");
const u8 gSpeedchoiceTooltipGen7XItems[] = _("Stat boost +2 instead of +1.");
const u8 gSpeedchoiceTooltipEvoEveryLv[] = _("STATIC: {PKMN} evolve into a random\nbut set species every lv.\pRAND: Same thing as STATIC but\nrandom non-static every lv.");
const u8 gSpeedchoiceTooltipHmBadgeChk[] = _("PURGE: There are no badge checks\nfor using HM moves in the field.\pKEEP: Vanilla behaviour.");
const u8 gSpeedchoiceTooltipEasyDexRewards[] = _("Removes Pokédex caught conditions\nfor receiving certain items.");
const u8 gSpeedchoiceTooltipEasySurgeCans[] = _("PURGE: The bottom left can will\nalways contain the first switch,\land the can to the right of it\lwill always contain the second.\pKEEP: Vanilla randomization\nHELL: Anything-goes randomization\lWHY: HELL + no save-scumming");
const u8 gSpeedchoiceTooltipNerfBrock[] = _("Nerfs LEADER BROCK by\nreducing his party levels by 2.");

// START GAME
const u8 gSpeedchoiceStartGameText[] = _("CV: {STR_VAR_1}\nStart the game?");

// MISC STRINGS
const u8 gSpeedchoiceEscapeText[] = _("ESCAPE");

/* ----------------- */
/* ---- PRESETS ---- */
/* ----------------- */

// -------------------------
// Enumeration for Preset.
// -------------------------
enum Preset {
    PRESET_VANILLA,
    PRESET_BINGO,
    PRESET_CEA,
    PRESET_RACE,
    NUM_PRESETS
};

const u8 gPresetNames[][20] = {
    [PRESET_VANILLA] =  _("VANILLA"),
    [PRESET_BINGO]   =  _("BINGO"),
    [PRESET_CEA]     =  _("CEA"),
    [PRESET_RACE]    =  _("RACE"),
};

// use local config optionConfig[0] for preset!

// When creating the task that manages the speedchoice menu, store the task ID in RAM to
// be used later when destroying the task.
static EWRAM_DATA int gSpeedchoiceTaskId = 0;

// ----------------------------------------------------------------
// Preset Data for Speedchoice Menu
// ----------------------------------------------------------------

static const u8 gPresets[NUM_PRESETS][CURRENT_OPTIONS_NUM] = {
    [PRESET_VANILLA] = {
        [PRESET]           = PRESET_VANILLA,
        [PLAYER_NAME_SET]  = 0xFF,
        [EXPMATH]          = EXP_KEEP,
        [PLOTLESS]         = PLOT_KEEP,
        [EARLY_SAFFRON]    = SAFFRON_NO,
        [RACE_GOAL]        = GOAL_MANUAL,
        [SPINNERS]         = SPIN_KEEP,
        [EARLYSURF]        = EARLY_SURF_NO,
        [MAXVISION]        = MAX_OFF,
        [GOOD_EARLY_WILDS] = GOOD_OFF,
        [EASY_FALSE_SWIPE] = EASY_FALSE_SWIPE_OFF,
        [EASY_DEX_REWARDS] = EASY_DEX_REWARDS_OFF,
        [FAST_CATCH]       = FAST_CATCH_OFF,
        [EARLY_BIKE]       = EARLY_BIKE_NO,
        [GEN_7_X_ITEMS]    = GEN_7_X_ITEMS_OFF,
        [EVO_EVERY_LEVEL]  = EVO_EV_OFF,
        [HM_BADGE_CHECKS]  = BADGE_KEEP,
        [EASY_SURGE_CANS]  = SURGE_KEEP,
        [NERF_BROCK]       = NERF_NO,
    },
    [PRESET_BINGO]   = {
        [PRESET]           = PRESET_BINGO,
        [PLAYER_NAME_SET]  = 0xFF,
        [EXPMATH]          = EXP_BW,
        [PLOTLESS]         = PLOT_FULL,
        [EARLY_SAFFRON]    = SAFFRON_YES,
        [RACE_GOAL]        = GOAL_MANUAL,
        [SPINNERS]         = SPIN_NERF,
        [EARLYSURF]        = EARLY_SURF_YES,
        [MAXVISION]        = MAX_OFF,
        [GOOD_EARLY_WILDS] = GOOD_STATIC,
        [EASY_FALSE_SWIPE] = EASY_FALSE_SWIPE_TUTOR,
        [EASY_DEX_REWARDS] = EASY_DEX_REWARDS_ON,
        [FAST_CATCH]       = FAST_CATCH_OFF,
        [EARLY_BIKE]       = EARLY_BIKE_YES,
        [GEN_7_X_ITEMS]    = GEN_7_X_ITEMS_ON,
        [EVO_EVERY_LEVEL]  = EVO_EV_OFF,
        [HM_BADGE_CHECKS]  = BADGE_PURGE,
        [EASY_SURGE_CANS]  = SURGE_NERF,
        [NERF_BROCK]       = NERF_YES,
    },
    [PRESET_CEA]     = {
        [PRESET]           = PRESET_CEA,
        [PLAYER_NAME_SET]  = 0xFF,
        [EXPMATH]          = EXP_BW,
        [PLOTLESS]         = PLOT_FULL,
        [EARLY_SAFFRON]    = SAFFRON_YES,
        [RACE_GOAL]        = GOAL_E4R2,
        [SPINNERS]         = SPIN_NERF,
        [EARLYSURF]        = EARLY_SURF_YES,
        [MAXVISION]        = MAX_OFF,
        [GOOD_EARLY_WILDS] = GOOD_OFF,
        [EASY_FALSE_SWIPE] = EASY_FALSE_SWIPE_TUTOR,
        [EASY_DEX_REWARDS] = EASY_DEX_REWARDS_OFF,
        [FAST_CATCH]       = FAST_CATCH_ON,
        [EARLY_BIKE]       = EARLY_BIKE_YES,
        [GEN_7_X_ITEMS]    = GEN_7_X_ITEMS_ON,
        [EVO_EVERY_LEVEL]  = EVO_EV_OFF,
        [HM_BADGE_CHECKS]  = BADGE_PURGE,
        [EASY_SURGE_CANS]  = SURGE_NERF,
        [NERF_BROCK]       = NERF_YES,
    },
    [PRESET_RACE]    = {
        [PRESET]           = PRESET_RACE,
        [PLAYER_NAME_SET]  = 0xFF,
        [EXPMATH]          = EXP_BW,
        [PLOTLESS]         = PLOT_FULL,
        [EARLY_SAFFRON]    = SAFFRON_YES,
        [RACE_GOAL]        = GOAL_E4R2,
        [SPINNERS]         = SPIN_NERF,
        [EARLYSURF]        = EARLY_SURF_YES,
        [MAXVISION]        = MAX_OFF,
        [GOOD_EARLY_WILDS] = GOOD_STATIC,
        [EASY_FALSE_SWIPE] = EASY_FALSE_SWIPE_TUTOR,
        [EASY_DEX_REWARDS] = EASY_DEX_REWARDS_ON,
        [FAST_CATCH]       = FAST_CATCH_OFF,
        [EARLY_BIKE]       = EARLY_BIKE_YES,
        [GEN_7_X_ITEMS]    = GEN_7_X_ITEMS_ON,
        [EVO_EVERY_LEVEL]  = EVO_EV_OFF,
        [HM_BADGE_CHECKS]  = BADGE_PURGE,
        [EASY_SURGE_CANS]  = SURGE_NERF,
        [NERF_BROCK]       = NERF_YES,
    },
};

/*
 * Fetch the Preset array from the set Preset ID.
 */
const u8 *GetPresetPtr(enum Preset presetID) {
    if (presetID >= NUM_PRESETS)
        presetID = PRESET_VANILLA;
    return gPresets[presetID];
}

// ---------------------------------------
// Speedchoice menu Option Config data
// ---------------------------------------
const struct OptionChoiceConfig OptionChoiceConfigYesNo[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceTextYes },
        { 150, gSpeedchoiceTextNo  },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigOnOff[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceTextOn  },
        { 150, gSpeedchoiceTextOff },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigNerfKeep[MAX_CHOICES] =
    {
        { 85, gSpeedchoiceTextNerf },
        { 120, gSpeedchoiceTextKeep },
        { 150, gSpeedchoiceTextHell },
        { 180, gSpeedchoiceTextWhy },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigSemiFull[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceTextSemi },
        { 150, gSpeedchoiceTextKeep },
        { 180, gSpeedchoiceTextFull },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigOffRand[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceTextOff    },
        { 150, gSpeedchoiceTextStatic },
        { 180, gSpeedchoiceTextRand   },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigOffTutorHM[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceTextOff   },
        { 145, gSpeedchoiceTextTutor },
        { 180, gSpeedchoiceTextHM05  },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigSaneHell[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceTextOff  },
        { 150, gSpeedchoiceTextSane },
        { 180, gSpeedchoiceTextHell },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigKeepNone[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceTextKeep },
        { 150, gSpeedchoiceTextBW   },
        { 180, gSpeedchoiceTextNone },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

const struct OptionChoiceConfig OptionChoiceConfigRaceGoal[MAX_CHOICES] = {
    { 110, gSpeedchoiceTextManual },
    { 150, gSpeedchoiceTextHoF    },
    { 180, gSpeedchoiceTextE4R2   },
    { -1, NULL },
    { -1, NULL },
    { -1, NULL }
};

/*
 * In order to use ProcessGeneralInput, a struct is needed for page, so, I opt to have a
 * dummy struct which only has the number of choices relevent to the calculation of the
 * selection.
 */
const struct OptionChoiceConfig OptionChoiceConfigPage[MAX_CHOICES] =
    {
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

// player name needs its own config. Leave NULL there.
const struct OptionChoiceConfig OptionChoiceConfigPlayerName[MAX_CHOICES] =
    {
        { 120, NULL }, // we still have a coord so it's handled properly.
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

// not a normal config struct, but used for the arrows for multi choice.
const struct OptionChoiceConfig Arrows[MAX_CHOICES] =
    {
        { 120, gSpeedchoiceOptionLeftArrow },
        { 195, gSpeedchoiceOptionRightArrow },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL },
        { -1, NULL }
    };

// ---------------------------------------
// Speedchoice menu Option data
// ---------------------------------------
const struct SpeedchoiceOption SpeedchoiceOptions[CURRENT_OPTIONS_NUM + 1] = // plus one for page
    {
        // ----------------------------------
        // PRESET OPTION
        // ----------------------------------
        [PRESET] = {
            .optionCount = 4,
            .optionType = ARROW,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionPreset,
            .options = Arrows,
            .tooltip = gSpeedchoiceTooltipPreset,
        },
        // ----------------------------------
        // PLAYER NAME
        // ----------------------------------
        [PLAYER_NAME_SET] = {
            .optionCount = 1,
            .optionType = PLAYER_NAME,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionName,
            .options = OptionChoiceConfigPlayerName,
            .tooltip = gSpeedchoiceTooltipName,
        },
        // ----------------------------------
        // EXP OPTION
        // ----------------------------------
        [EXPMATH] = {
            .optionCount = 3,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionEXP,
            .options = OptionChoiceConfigKeepNone,
            .tooltip = gSpeedchoiceTooltipEXP,
        },
        // ----------------------------------
        // PLOTLESS OPTION
        // ----------------------------------
        [PLOTLESS] = {
            .optionCount = 3,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionPlotless,
            .options = OptionChoiceConfigSemiFull,
            .tooltip = gSpeedchoiceTooltipPlotless,
        },
        // ----------------------------------
        // PLOTLESS OPTION
        // ----------------------------------
        [EARLY_SAFFRON] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionEarlySaffron,
            .options = OptionChoiceConfigYesNo,
            .tooltip = gSpeedchoiceTooltipEarlySaffron,
        },
        // ----------------------------------
        // RACE GOAL OPTION
        // ----------------------------------
        [RACE_GOAL] = {
            .optionCount = 3,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionRaceGoal,
            .options = OptionChoiceConfigRaceGoal,
            .tooltip = gSpeedchoiceTooltipRaceGoal,
        },
        // ----------------------------------
        // SPINNERS OPTION
        // ----------------------------------
        [SPINNERS] = {
            .optionCount = 4,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionSpinners,
            .options = OptionChoiceConfigNerfKeep,
            .tooltip = gSpeedchoiceTooltipSpinners,
        },
        // ----------------------------------
        // EARLY SURF OPTION
        // ----------------------------------
        [EARLYSURF] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionEarlySurf,
            .options = OptionChoiceConfigOnOff,
            .tooltip = gSpeedchoiceTooltipEarlySurf,
        },
        // ----------------------------------
        // MAX VISION OPTION
        // ----------------------------------
        [MAXVISION] = {
            .optionCount = 3,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionMaxVision,
            .options = OptionChoiceConfigSaneHell,
            .tooltip = gSpeedchoiceTooltipMaxVision,
        },
        // ----------------------------------
        // GOOD EARLY WILDS OPTION
        // ----------------------------------
        [GOOD_EARLY_WILDS] = {
            .optionCount = 3,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionGoodEarlyWilds,
            .options = OptionChoiceConfigOffRand,
            .tooltip = gSpeedchoiceTooltipGoodEarlyWilds,
        },
        // ----------------------------------
        // EASY FALSE SWIPE OPTION
        // ----------------------------------
        [EASY_FALSE_SWIPE] = {
            .optionCount = 3,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionEasyFalseSwipe,
            .options = OptionChoiceConfigOffTutorHM,
            .tooltip = gSpeedchoiceTooltipEasyFalseSwipe,
        },
        // ----------------------------------
        // EASY FALSE SWIPE OPTION
        // ----------------------------------
        [EASY_DEX_REWARDS] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchocieOptionEasyDexRewards,
            .options = OptionChoiceConfigOnOff,
            .tooltip = gSpeedchoiceTooltipEasyDexRewards,
        },
        // ----------------------------------
        // FAST CATCH OPTION
        // ----------------------------------
        [FAST_CATCH] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionFastCatch,
            .options = OptionChoiceConfigOnOff,
            .tooltip = gSpeedchoiceTooltipFastCatch,
        },
        // ----------------------------------
        // EARLY BIKE OPTION
        // ----------------------------------
        [EARLY_BIKE] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionEarlyBike,
            .options = OptionChoiceConfigYesNo,
            .tooltip = gSpeedchoiceTooltipEarlyBike,
        },
        // ----------------------------------
        // GEN 7 X ITEMS OPTION
        // ----------------------------------
        [GEN_7_X_ITEMS] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionGen7XItems,
            .options = OptionChoiceConfigOnOff,
            .tooltip = gSpeedchoiceTooltipGen7XItems,
        },
        // ----------------------------------
        // EVO EVERY LEVEL OPTION
        // ----------------------------------
        [EVO_EVERY_LEVEL] = {
            .optionCount = 3,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionEvoEveryLv,
            .options = OptionChoiceConfigOffRand,
            .tooltip = gSpeedchoiceTooltipEvoEveryLv,
        },
        // ----------------------------------
        // HM BADGE CHECKS OPTION
        // ----------------------------------
        [HM_BADGE_CHECKS] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionHmBadgeChk,
            .options = OptionChoiceConfigNerfKeep,
            .tooltip = gSpeedchoiceTooltipHmBadgeChk,
        },
        // ----------------------------------
        // EASY SURGE CANS OPTION
        // ----------------------------------
        [EASY_SURGE_CANS] = {
            .optionCount = 4,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionEasySurgeCans,
            .options = OptionChoiceConfigNerfKeep,
            .tooltip = gSpeedchoiceTooltipEasySurgeCans,
        },
        // ----------------------------------
        // NERF BROCK OPTION
        // ----------------------------------
        [NERF_BROCK] = {
            .optionCount = 2,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionNerfBrock,
            .options = OptionChoiceConfigNerfKeep,
            .tooltip = gSpeedchoiceTooltipNerfBrock,
        },
        // ----------------------------------
        // PAGE STATIC OPTION
        // ----------------------------------
        [PAGE] = {
            .optionCount = MAX_PAGES,
            .optionType = NORMAL,
            .enabled = TRUE,
            .string = gSpeedchoiceOptionPage,
            .options = OptionChoiceConfigPage,
            .tooltip = NULL,
        }
    };

// To avoid redrawing the page info every frame, we store the active page number
// and compare it to the task's page number. We only redraw if they are different
// and update accordingly.
static EWRAM_DATA u8 sStoredPageNum = 0;

// See SpeedchoiceConfigStruct documentation in speedchoice.h.
static EWRAM_DATA struct SpeedchoiceConfigStruct sLocalSpeedchoiceConfig = {0};

// See MapObjectTimerBackup documentation in speedchoice.h.
EWRAM_DATA struct MapObjectTimerBackup * gMapObjectTimerBackup = NULL;

// Used to signal that the page must redraw in cases where it is needed such as
// updating the Preset.
static EWRAM_DATA bool8 sForceUpdate = FALSE;

// Stores the written player name in the options menu until it is flushed to the
// Save Block.
static EWRAM_DATA u8 sTempPlayerName[PLAYER_NAME_LENGTH + 1] = {0};

// -------------------------------------
// PROTOTYPES
// -------------------------------------
u8 GetPageDrawCount(u8 page);
void SetPageIndexFromTrueIndex(u8 taskId, s16 index);

/*
 * When initializing the menu for the first time or setting a preset, this function
 * is invoked to set the local config and save block data from the preset or default
 * preset.
 */

void SetByteArrayToSaveOptions(const u8 * options_arr)
{
    gSaveBlock2Ptr->speedchoiceConfig.expsystem = options_arr[EXPMATH];
    gSaveBlock2Ptr->speedchoiceConfig.plotless = options_arr[PLOTLESS];
    gSaveBlock2Ptr->speedchoiceConfig.earlySaffron = options_arr[EARLY_SAFFRON];
    gSaveBlock2Ptr->speedchoiceConfig.raceGoal = options_arr[RACE_GOAL];
    gSaveBlock2Ptr->speedchoiceConfig.spinners = options_arr[SPINNERS];
    gSaveBlock2Ptr->speedchoiceConfig.earlySurf = options_arr[EARLYSURF];
    gSaveBlock2Ptr->speedchoiceConfig.maxVision = options_arr[MAXVISION];
    gSaveBlock2Ptr->speedchoiceConfig.goodEarlyWilds = options_arr[GOOD_EARLY_WILDS];
    gSaveBlock2Ptr->speedchoiceConfig.easyFalseSwipe = options_arr[EASY_FALSE_SWIPE];
    gSaveBlock2Ptr->speedchoiceConfig.easyDexRewards = options_arr[EASY_DEX_REWARDS];
    gSaveBlock2Ptr->speedchoiceConfig.fastCatch = options_arr[FAST_CATCH];
    gSaveBlock2Ptr->speedchoiceConfig.earlyBike = options_arr[EARLY_BIKE];
    gSaveBlock2Ptr->speedchoiceConfig.gen7XItems = options_arr[GEN_7_X_ITEMS];
    gSaveBlock2Ptr->speedchoiceConfig.evoEveryLevel = options_arr[EVO_EVERY_LEVEL];
    gSaveBlock2Ptr->speedchoiceConfig.hmBadgeChecks = options_arr[HM_BADGE_CHECKS];
    gSaveBlock2Ptr->speedchoiceConfig.easySurgeCans = options_arr[EASY_SURGE_CANS];
    gSaveBlock2Ptr->speedchoiceConfig.nerfBrock = options_arr[NERF_BROCK];
}

void SetOptionChoicesAndConfigFromPreset(const u8 *preset)
{
    u8 i;

    // set the local config for the current menu. Do NOT overwrite the preset!
    memcpy(sLocalSpeedchoiceConfig.optionConfig + 1, preset + 1, CURRENT_OPTIONS_NUM - 1);

    // this would be a for loop, but i want to use the fewest bits possible to
    // avoid shifting RAM too much: hence the ugly per-option saving.
    SetByteArrayToSaveOptions(preset);
}

/*
 * This option is used to check the currently set option data and is used when doing
 * speedchoice checks. (Is the EXP option set to BWEXP, etc)
 *
 * See the speedchoice.h enums for the values to pass here.
 */
u8 CheckSpeedchoiceOption(u8 option)
{
    switch(option)
    {
    case EXPMATH:
        return gSaveBlock2Ptr->speedchoiceConfig.expsystem;
    case RACE_GOAL:
        return gSaveBlock2Ptr->speedchoiceConfig.raceGoal;
    case EARLY_SAFFRON:
        return gSaveBlock2Ptr->speedchoiceConfig.earlySaffron;
    case PLOTLESS:
        return gSaveBlock2Ptr->speedchoiceConfig.plotless;
    case EARLYSURF:
        return gSaveBlock2Ptr->speedchoiceConfig.earlySurf;
    case SPINNERS:
        return gSaveBlock2Ptr->speedchoiceConfig.spinners;
    case MAXVISION:
        return gSaveBlock2Ptr->speedchoiceConfig.maxVision;
    case GOOD_EARLY_WILDS:
        return gSaveBlock2Ptr->speedchoiceConfig.goodEarlyWilds;
    case EASY_FALSE_SWIPE:
        return gSaveBlock2Ptr->speedchoiceConfig.easyFalseSwipe;
    case EASY_DEX_REWARDS:
        return gSaveBlock2Ptr->speedchoiceConfig.easyDexRewards;
    case FAST_CATCH:
        return gSaveBlock2Ptr->speedchoiceConfig.fastCatch;
    case EARLY_BIKE:
        return gSaveBlock2Ptr->speedchoiceConfig.earlyBike;
    case GEN_7_X_ITEMS:
        return gSaveBlock2Ptr->speedchoiceConfig.gen7XItems;
    case EVO_EVERY_LEVEL:
        return gSaveBlock2Ptr->speedchoiceConfig.evoEveryLevel;
    case HM_BADGE_CHECKS:
        return gSaveBlock2Ptr->speedchoiceConfig.hmBadgeChecks;
    case EASY_SURGE_CANS:
        return gSaveBlock2Ptr->speedchoiceConfig.easySurgeCans;
    case NERF_BROCK:
        return gSaveBlock2Ptr->speedchoiceConfig.nerfBrock;
    default:
        return FALSE;
    }
}

/*
 * Invoked to locally format the passed text and add it to the text printer to render it.
 */
static void DrawOptionMenuChoice(const u8 *text, u8 x, u8 y, u8 style)
{
    AddTextPrinterParameterized3(SPD_WIN_OPTIONS, 2, x, y, sTextColors[style], TEXT_SPEED_FF, text);
}

void DrawPageOptions(u8);

/*
 * This is used to handle the inputs per option, not the Speedchoice menu inputs overall.
 * See Task_SpeedchoiceMenuProcessInput for that.
 */
static u8 ProcessGeneralInput(struct SpeedchoiceOption *option, u8 selection, bool8 indexedToOne) // if indexedToOne is true (1), i can conveniently use it as the selection anchor.
{
    if(gMain.newKeys & DPAD_RIGHT)
    {
        if(option->enabled == FALSE)
        {
            PlaySE(SE_FAILURE);
            return selection;
        }
        if(selection == (option->optionCount - (!indexedToOne))) // i invert the option because when indexedToOne is set to true (or 1) it means i do not subtract 1 since it's not 0 indexed, so invert the boolean in both cases to get the correct modifier.
            selection = indexedToOne;
        else
            selection++;
        if(option->string != gSpeedchoiceOptionPage)
            PlaySE(SE_SELECT);
        else
            PlaySE(SE_WIN_OPEN); // page scrolling.
    }
    // i dont return immediately because emulators could hold both right and left down.
    if(gMain.newKeys & DPAD_LEFT)
    {
        if(option->enabled == FALSE)
        {
            PlaySE(SE_FAILURE);
            return selection;
        }
        if(selection == indexedToOne)
            selection = (option->optionCount - (!indexedToOne));
        else
            selection--;
        if(option->string != gSpeedchoiceOptionPage)
            PlaySE(SE_SELECT);
        else
            PlaySE(SE_WIN_OPEN); // page scrolling.
    }
    return selection;
}

// --------------------------------------------
// Standard callbacks for Main and VBlankCB.
// --------------------------------------------
static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// We dim the whole menu except the header and currently selected option. This is invoked when the menu
// is initialized or re-initialized.
void HighlightHeaderBox(void)
{
    SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(17, 223));
    SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE(1, 31));
}

//This version uses addition '+' instead of OR '|'.
#define WIN_RANGE_(a, b) (((a) << 8) + (b))

// Same as above, but for the option instead.
static void HighlightOptionMenuItem(u8 index)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, 224));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE_(index * 16 + 40, index * 16 + 56));
}

// The Options menu relies on a CB2 init while New Game relied on a task switchover. As a hack, I
// used this to transition it over. It's a bit messy but whatever.

static void DrawHeaderWindow(void);
static void Task_SpeedchoiceMenuFadeIn(u8 taskId);
static void sub_80BB154(void);

extern const u8 *const gFemalePresetNames[19];

/*
 * Initialize the default player name similar to how Birch intro does it.
 */
void FormatInitialTempName(u8 nameId)
{
    StringCopy7(sTempPlayerName, gFemalePresetNames[nameId]);
}

// Used to signal to avoid redrawing specific stuff. This is used for the naming screen switchover
// and the first time tooltip.
EWRAM_DATA bool32 gAlreadyLoaded = FALSE;

extern const u16 sHelpDocsPalette[0x80];

/*
 * Initialize the CB2 Speedchoice Menu.
 */
void CB2_InitSpeedchoice(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gSpeedchoiceTaskId = -1;
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x100);
        DmaClear32(3, (void *)OAM, OAM_SIZE);
        DmaClear16(3, (void *)PLTT, PLTT_SIZE);
        gMain.state++;
        break;
    case 2:
        FillPalette(RGB_BLACK, 0, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sSpeedchoiceMenuBgTemplates, NELEMS(sSpeedchoiceMenuBgTemplates));
        ChangeBgX(0, 0, 0);
        ChangeBgY(0, 0, 0);
        ChangeBgX(1, 0, 0);
        ChangeBgY(1, 0, 0);
        ChangeBgX(2, 0, 0);
        ChangeBgY(2, 0, 0);
        ChangeBgX(3, 0, 0);
        ChangeBgY(3, 0, 0);
        InitWindows(sSpeedchoiceMenuWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, 5);
        SetGpuReg(REG_OFFSET_WINOUT, 39);
        SetGpuReg(REG_OFFSET_BLDCNT, 193);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        gMain.state++;
        break;
    case 3:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 4:
        LoadBgTiles(1, GetUserFrameGraphicsInfo(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sHelpDocsPalette, 0x000, 0x080);
//        LoadPalette(sOptionMenuPalette, 0, sizeof(sOptionMenuPalette));
        LoadPalette(GetUserFrameGraphicsInfo(gSaveBlock2Ptr->optionsWindowFrameType)->palette, 0x70, 0x20);
        LoadPalette(GetUserFrameGraphicsInfo(gSaveBlock2Ptr->optionsWindowFrameType)->palette, 0x20, 0x20); // again for the hardcoded border
//        gPlttBufferUnfaded[0] = RGB_BLACK;
        gMain.state++;
        break;
    case 6:
        LoadPalette(stdpal_get(2), 0x10, 0x20);
        LoadPalette(sMainMenuTextPal, 0xF0, sizeof(sMainMenuTextPal));
        gMain.state++;
        break;
    case 7:
        PutWindowTilemap(0);
        DrawHeaderWindow();
        gMain.state++;
        break;
    case 8:
        gMain.state++;
        break;
    case 9:
        PutWindowTilemap(1);
        //DrawOptionMenuTexts();
        gMain.state++;
    case 10:
        sub_80BB154();
        gMain.state++;
        break;
    case 11:
    {
        gSpeedchoiceTaskId = CreateTask(Task_SpeedchoiceMenuFadeIn, 0);

        sStoredPageNum = 1;

        if(!gAlreadyLoaded)
        {
            sLocalSpeedchoiceConfig.trueIndex = 0;
            sLocalSpeedchoiceConfig.pageIndex = 0;
            sLocalSpeedchoiceConfig.pageNum = 1;

            SetOptionChoicesAndConfigFromPreset(gPresets[PRESET_VANILLA]);

            FormatInitialTempName(Random() % NELEMS(gFemalePresetNames));
        }
        DrawHeaderWindow();
        DrawPageOptions(sLocalSpeedchoiceConfig.pageNum);

        /*TextSpeed_DrawChoices(gTasks[taskId].data[TD_TEXTSPEED]);
        BattleScene_DrawChoices(gTasks[taskId].data[TD_BATTLESCENE]);
        BattleStyle_DrawChoices(gTasks[taskId].data[TD_BATTLESTYLE]);
        Sound_DrawChoices(gTasks[taskId].data[TD_SOUND]);
        ButtonMode_DrawChoices(gTasks[taskId].data[TD_BUTTONMODE]);
        FrameType_DrawChoices(gTasks[taskId].data[TD_FRAMETYPE]);
        HighlightOptionMenuItem(gTasks[taskId].data[TD_MENUSELECTION]);*/

        HighlightHeaderBox();
        HighlightOptionMenuItem(sLocalSpeedchoiceConfig.pageIndex);
        if(!gAlreadyLoaded)
            PlayBGM(MUS_NEW_GAME_INSTRUCT);
        gMain.state++;
        break;
    }
    case 12:
        BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

// from charmap.txt
#define CHAR_0 0xA1
#define CHAR_PERCENT 0x5B

// Replaces MENUOPTIONCOORDS in speedchoice.h.
#define NEWMENUOPTIONCOORDS(i)  (((i) * 16) + 2)

// Render the page number.
static void DrawPageChoice(u8 selection)
{
    u8 text[2] = {selection + CHAR_0, EOS};

    AddTextPrinterParameterized3(SPD_WIN_OPTIONS, 2, 40, NEWMENUOPTIONCOORDS(5), sTextColors[SPC_COLOR_RED], TEXT_SPEED_FF, text);
}

// Render the text for the choices for each option.
static void DrawGeneralChoices(struct SpeedchoiceOption *option, u8 selection, u8 row)
{
    u8 styles[MAX_CHOICES];
    u8 numChoices = option->optionCount;
    u8 i;

    // Format the array.
    if(numChoices < MAX_CHOICES)
    {
        for(i = 0; i < numChoices; i++)
            styles[i] = 0;

        styles[selection] = 1;
    }

    // Arrows needs special handling for positioning the arrows and text.
    if(option->optionType == ARROW)
    {
        u8 text[8];
        s16 x_left = Arrows[0].x;
        s16 x_right = Arrows[1].x;
        s16 y = NEWMENUOPTIONCOORDS(row);
        // perform centering, add 4 pixels for the 8x8 arrow centering
        s16 x_preset = 4 + x_left + (x_right - x_left - GetStringWidth(1, gPresetNames[sLocalSpeedchoiceConfig.optionConfig[0]], 0)) / 2;

        DrawOptionMenuChoice(Arrows[0].string, x_left, y, SPC_COLOR_RED); // left arrow
        DrawOptionMenuChoice(Arrows[1].string, x_right, y, SPC_COLOR_RED); // right arrow
        DrawOptionMenuChoice(gPresetNames[sLocalSpeedchoiceConfig.optionConfig[0]], x_preset, y, SPC_COLOR_BLUE);
    }
        // Player name needs special handling as well.
    else if(option->optionType == PLAYER_NAME)
    {
        s16 y = NEWMENUOPTIONCOORDS(row);
        s16 x_left = OptionChoiceConfigPlayerName[0].x;
        s16 x_right = 195; // from Arrows[1].x dont mind me just borrowing
        s16 length = GetStringWidth(1, sTempPlayerName, 0);
        s16 x_preset = 4 + x_left + (x_right - x_left - length) / 2;

        DrawOptionMenuChoice(sTempPlayerName, x_preset, y, SPC_COLOR_RED);
    }
        // Assume everything else is a normal option render.
    else
    {
        for(i = 0; i < numChoices; i++)
        {
            s16 x = option->options[i].x;
            s16 y = NEWMENUOPTIONCOORDS(row);
            const u8 *string = option->options[i].string;

            DrawOptionMenuChoice(string, x, y, styles[i]);
        }
    }
}

static void Task_SpeedchoiceMenuProcessInput(u8);
static void DrawTooltip(u8 taskId, const u8 *str, int speed, bool32 isYesNo);

/*
 * Fade in until it's time to start processing inputs.
 */
static void Task_SpeedchoiceMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        gTasks[taskId].func = Task_SpeedchoiceMenuProcessInput;
    }
}

/*
 * Get the true index of the first or last page option depending on the enum passed.
 */
u8 GetPageOptionTrueIndex(bool8 lastOrFirst, u8 page)
{
    if(lastOrFirst == LAST)
        return (OPTIONS_PER_PAGE * (page - 1)) + GetPageDrawCount(page) - 1;
    else
        return (OPTIONS_PER_PAGE * (page - 1));
}

/*
 * Same as above, but return the page index.
 */
u8 GetPageOptionPageIndex(bool8 lastOrFirst, u8 page)
{
    return (lastOrFirst) ? GetPageDrawCount(page) : 1;
}

extern void MainMenu_EraseWindow(const struct WindowTemplate *template);

/*
 * Finish rendering the tooltip by holding until it has completed rendering.
 */
static void Task_WaitForTooltip(u8 taskId)
{
    RunTextPrinters();

    if(!IsTextPrinterActive(SPD_WIN_TOOLTIP))
    {
        if(gMain.newKeys & A_BUTTON)
        {
            ClearWindowTilemap(SPD_WIN_TOOLTIP);
            MainMenu_EraseWindow((struct WindowTemplate *)&sSpeedchoiceMenuWinTemplates[SPD_WIN_TOOLTIP]);
            DrawPageOptions(sLocalSpeedchoiceConfig.pageNum);
            gTasks[taskId].func = Task_SpeedchoiceMenuProcessInput;
        }
    }
}

extern void MainMenu_DrawWindow(const struct WindowTemplate*, u16);

/*
 * Initiate the rendering for the tooltip.
 */
static void DrawTooltip(u8 taskId, const u8 *str, int speed, bool32 isYesNo)
{
    FillWindowPixelBuffer(SPD_WIN_TOOLTIP, PIXEL_FILL(1));
    AddTextPrinterParameterized3(SPD_WIN_TOOLTIP, 2, 0, 1, sTextColors[SPC_COLOR_GREY], speed, str);
    //sub_8098858(SPD_WIN_TOOLTIP, 0x1D5, 0);
    MainMenu_DrawWindow((struct WindowTemplate *)&sSpeedchoiceMenuWinTemplates[SPD_WIN_TOOLTIP], 418);
    PutWindowTilemap(SPD_WIN_TOOLTIP);
    CopyWindowToVram(SPD_WIN_TOOLTIP, 3);
    if(isYesNo)
    {
        FillWindowPixelBuffer(3, PIXEL_FILL(1));
        MainMenu_DrawWindow((struct WindowTemplate *)&sSpeedchoiceMenuWinTemplates[SPD_WIN_YESNO], 418);
        PutWindowTilemap(3);
        CopyWindowToVram(3, 3);
    }
    SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(1, 241));
    SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE_(114, 160));
    if(!isYesNo)
        gTasks[taskId].func = Task_WaitForTooltip;
}

// Calculate the Check Value given a given option configuration. Used for verifying ROM
// check value + option config before starting a run/race.
u32 CalculateCheckValue(void)
{
    u32 checkValue;
    // Promoted to s32 because the actual width is not important
    s32 i; // current option
    s32 totalBitsUsed;
    u32 curBitsUsed;

    // do checkvalue increment for 32-bit value.
    for (checkValue = 0, i = 0, totalBitsUsed = 0; i < CURRENT_OPTIONS_NUM; i++)
    {
        checkValue += sLocalSpeedchoiceConfig.optionConfig[i] << totalBitsUsed;
        // Because MAX_CHOICES == 6, this is valid.
        // Otherwise we'd need to do some actual work.
        curBitsUsed = (SpeedchoiceOptions[i].optionCount + 1u) / 2u;
        totalBitsUsed += curBitsUsed;

        // Prevent useless shifts.
        // This sizeof check is to control whether the compiler actually
        // builds this statement.
        if (sizeof(struct SpeedchoiceSaveOptions) > 4)
        {
            if (totalBitsUsed >= 32)
            {
                totalBitsUsed -= 32;
                if (totalBitsUsed)
                    checkValue += sLocalSpeedchoiceConfig.optionConfig[i] >> (curBitsUsed - totalBitsUsed);
            }
        }
    }

    // seed RNG with checkValue for more hash-like number.
    // xor with randomizer value, if one is present.
    return LC_RNG(checkValue) ^ gRandomizerCheckValue;
}

// Flush the settings to the Save Block.
static void SaveSpeedchoiceOptions()
{
    SetByteArrayToSaveOptions(sLocalSpeedchoiceConfig.optionConfig);

    // write the playername.
    StringCopy7(gSaveBlock2Ptr->playerName, sTempPlayerName);
}

extern const struct BgTemplate sMainMenuBgTemplates[];
extern void Task_OaksSpeech1(u8 taskId);
extern void CB2_NewGameOaksSpeech(void);

/*
 * Complete the fade out of the speedchoice menu and then clear the menu data and
 * jump to Birch Intro.
 */
static void Task_SpeedchoiceMenuFadeOut(u8 taskId)
{
    if(!gPaletteFade.active && --gTasks[taskId].data[15] == 0)
    {
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, (void *)OAM, OAM_SIZE);
        DmaClear16(3, (void *)PLTT, PLTT_SIZE);
        gPlttBufferUnfaded[0] = 0;
        gPlttBufferFaded[0] = 0;
        /*ClearWindowTilemap(SPD_WIN_TEXT_OPTION);
        ClearWindowTilemap(SPD_WIN_OPTIONS);
        ClearWindowTilemap(SPD_WIN_TOOLTIP);
        ClearWindowTilemap(3);
        sub_8032250(&sSpeedchoiceMenuWinTemplates[SPD_WIN_TEXT_OPTION]);
        sub_8032250(&sSpeedchoiceMenuWinTemplates[SPD_WIN_OPTIONS]);
        sub_8032250(&sSpeedchoiceMenuWinTemplates[SPD_WIN_TOOLTIP]);
        sub_8032250(&sSpeedchoiceMenuWinTemplates[SPD_WIN_YESNO]);*/
        FreeAllWindowBuffers();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetGpuReg(REG_OFFSET_BG2CNT, 0);
        SetGpuReg(REG_OFFSET_BG1CNT, 0);
        SetGpuReg(REG_OFFSET_BG0CNT, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG1VOFS, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        ResetBgs();
        InitBgsFromTemplates(0, sMainMenuBgTemplates, 2);
        sInIntro = TRUE;
        sInSubMenu = FALSE;
        sInBattle = FALSE;
        sInField = FALSE;
        SetMainCallback2(CB2_NewGameOaksSpeech);
        gTasks[taskId].func = Task_OaksSpeech1;
    }
}

/*
 * Prompt the Yes/No menu choice to determine if the run/race is started.
 */
static void Task_AskToStartGame(u8 taskId)
{
    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0:  // YES
        PlayBGM(MUS_NEW_GAME_EXIT);
        PlaySE(SE_SELECT);
        SaveSpeedchoiceOptions();
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 0x10, 0);
        gTasks[taskId].func = Task_SpeedchoiceMenuFadeOut;
        gTasks[taskId].data[15] = 24;
        break;
    case 1:  // NO
    case -1: // B button
        PlayBGM(MUS_NEW_GAME_INSTRUCT);
        PlaySE(SE_SELECT);
        ClearWindowTilemap(SPD_WIN_TOOLTIP);
        ClearWindowTilemap(3);
        MainMenu_EraseWindow((struct WindowTemplate *)&sSpeedchoiceMenuWinTemplates[SPD_WIN_TOOLTIP]);
        MainMenu_EraseWindow((struct WindowTemplate *)&sSpeedchoiceMenuWinTemplates[SPD_WIN_YESNO]);
        DrawPageOptions(sLocalSpeedchoiceConfig.pageNum);
        gTasks[taskId].func = Task_SpeedchoiceMenuProcessInput;
        break;
    }
}

/*
 * This might be incorrectly named, but this is the function to initialize the Yes/No for start game.
 * The flushing of the options to the Save Block may have been done here originally.
 */

static u8 * SC_PrintHex32(u8 * dest, u32 num)
{
    s32 i;
    for (i = 0; i < 8; i++)
    {
        s32 nybble = (num >> 28) & 0xF;
        if (nybble < 10)
            *dest++ = nybble + CHAR_0;
        else
            *dest++ = nybble - 10 + CHAR_A;
        num <<= 4;
    }
    *dest = EOS;
    return dest;
}

static void Task_SpeedchoiceMenuSave(u8 taskId)
{
    SC_PrintHex32(gStringVar1, CalculateCheckValue());
    StringExpandPlaceholders(gStringVar4, gSpeedchoiceStartGameText);
    DrawTooltip(taskId, gStringVar4, TEXT_SPEED_FF, TRUE); // a bit of a hack, but whatever.
    CreateYesNoMenu(&sSpeedchoiceMenuWinTemplates[SPD_WIN_YESNO], 3, 0, 2, 418, 2, 0);

    gTasks[taskId].func = Task_AskToStartGame;
}

/*
 * Fade out Task function to initialize naming screen.
 */
void Task_SpeedchoiceMenuFadeOutToNamingScreen(u8 taskId)
{
    if(!gPaletteFade.active)
    {
        FreeAllWindowBuffers();
        DestroyTask(gSpeedchoiceTaskId);
        DoNamingScreen(NAMING_SCREEN_PLAYER, sTempPlayerName, 1, 0, 0, CB2_InitSpeedchoice);
    }
}

/*
 * General processor for the input on the speedchoice menu.
 */
static void Task_SpeedchoiceMenuProcessInput(u8 taskId)
{
    if(!gAlreadyLoaded)
    {
        DrawTooltip(taskId, gSpeedchoiceTooltipExplanation, GetTextSpeedSetting(), FALSE);
        gAlreadyLoaded = TRUE;
    }
    else if (gMain.newKeys & A_BUTTON)
    {
        if (sLocalSpeedchoiceConfig.trueIndex == START_GAME)
        {
            PlayBGM(MUS_NEW_GAME_INTRO);
            gTasks[taskId].func = Task_SpeedchoiceMenuSave;
        }
        else if (sLocalSpeedchoiceConfig.trueIndex == PRESET) {
            SetOptionChoicesAndConfigFromPreset(GetPresetPtr(sLocalSpeedchoiceConfig.optionConfig[PRESET]));
            PlaySE(SE_SELECT); // page scrolling.
            sForceUpdate = TRUE;
        }
        else if (sLocalSpeedchoiceConfig.trueIndex == PLAYER_NAME_SET) {
            BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
            gTasks[taskId].func = Task_SpeedchoiceMenuFadeOutToNamingScreen;
            PlaySE(SE_SELECT); // page scrolling.
        }
    }
    else if (gMain.newKeys & SELECT_BUTTON) // do tooltip.
    {
        if(sLocalSpeedchoiceConfig.trueIndex <= CURRENT_OPTIONS_NUM && SpeedchoiceOptions[sLocalSpeedchoiceConfig.trueIndex].tooltip != NULL)
            DrawTooltip(taskId, SpeedchoiceOptions[sLocalSpeedchoiceConfig.trueIndex].tooltip, GetTextSpeedSetting(), FALSE);
    }
    else if (gMain.newKeys & DPAD_UP)
    {
        if(sLocalSpeedchoiceConfig.trueIndex == PAGE)
            sLocalSpeedchoiceConfig.trueIndex = GetPageOptionTrueIndex(LAST, sLocalSpeedchoiceConfig.pageNum); // set the entry to the last available option.
        else if(sLocalSpeedchoiceConfig.trueIndex > GetPageOptionTrueIndex(FIRST, sLocalSpeedchoiceConfig.pageNum))
            sLocalSpeedchoiceConfig.trueIndex--;
        else
            sLocalSpeedchoiceConfig.trueIndex = START_GAME;

        SetPageIndexFromTrueIndex(taskId, sLocalSpeedchoiceConfig.trueIndex);
        HighlightOptionMenuItem(sLocalSpeedchoiceConfig.pageIndex);
    }
    else if (gMain.newKeys & DPAD_DOWN)
    {
        if(sLocalSpeedchoiceConfig.trueIndex == GetPageOptionTrueIndex(LAST, sLocalSpeedchoiceConfig.pageNum))
            sLocalSpeedchoiceConfig.trueIndex = PAGE; // you are at the last option when you press down, go to page index.
        else if(sLocalSpeedchoiceConfig.trueIndex == START_GAME)
            sLocalSpeedchoiceConfig.trueIndex = GetPageOptionTrueIndex(FIRST, sLocalSpeedchoiceConfig.pageNum);
        else
            sLocalSpeedchoiceConfig.trueIndex++;

        SetPageIndexFromTrueIndex(taskId, sLocalSpeedchoiceConfig.trueIndex);
        HighlightOptionMenuItem(sLocalSpeedchoiceConfig.pageIndex);
    }
    else
    {
        u8 trueIndex = sLocalSpeedchoiceConfig.trueIndex;
        u8 selection = sLocalSpeedchoiceConfig.optionConfig[trueIndex];
        switch (trueIndex)
        {
        default:
            if(trueIndex < CURRENT_OPTIONS_NUM)
            {
                // lol. I don't know why I hardcoded this in Sapphire.
                //if(trueIndex == NERFROXANNE)
                //    sLocalSpeedchoiceConfig.optionConfig[trueIndex] = ProcessGeneralInput((struct SpeedchoiceOption *)&SpeedchoiceOptions[trueIndex], selection, TRUE);
                //else
                u8 oldSelection = sLocalSpeedchoiceConfig.optionConfig[trueIndex];
                sLocalSpeedchoiceConfig.optionConfig[trueIndex] = ProcessGeneralInput((struct SpeedchoiceOption *)&SpeedchoiceOptions[trueIndex], selection, FALSE);
                DrawGeneralChoices((struct SpeedchoiceOption *)&SpeedchoiceOptions[trueIndex], sLocalSpeedchoiceConfig.optionConfig[trueIndex], sLocalSpeedchoiceConfig.pageIndex);
                if(oldSelection != sLocalSpeedchoiceConfig.optionConfig[trueIndex] || sForceUpdate) {
                    DrawPageOptions(sLocalSpeedchoiceConfig.pageNum); // HACK!!! The page has to redraw. But only redraw it if the selection changed, otherwise it lags.
                    sForceUpdate = FALSE;
                }
            }
            break;
        case PAGE:
            sLocalSpeedchoiceConfig.pageNum = ProcessGeneralInput((struct SpeedchoiceOption *)&SpeedchoiceOptions[CURRENT_OPTIONS_NUM], sLocalSpeedchoiceConfig.pageNum, TRUE);
            //DrawPageChoice(sLocalSpeedchoiceConfig.pageNum); Deprecated.
            if(sLocalSpeedchoiceConfig.pageNum != sStoredPageNum) // only redraw if the page updates!
            {
                DrawPageOptions(sLocalSpeedchoiceConfig.pageNum);
                sStoredPageNum = sLocalSpeedchoiceConfig.pageNum; // update the page.
            }
            break;
        case START_GAME:
            break;
        }
    }
    gTasks[taskId].data[0]++; // arrow timer
}

// Self-explanatory. Draws the header window.
static void DrawHeaderWindow(void)
{
    s32 width;
    FillWindowPixelBuffer(SPD_WIN_TEXT_OPTION, PIXEL_FILL(1));
    AddTextPrinterParameterized3(SPD_WIN_TEXT_OPTION, 2, 4, 1, sTextColors[SPC_COLOR_GREY], TEXT_SPEED_FF, gSpeedchoiceTextHeader);
    width = GetStringWidth(2, gSpeedchoiceCurrentVersion, GetFontAttribute(2, FONTATTR_LETTER_SPACING));
    AddTextPrinterParameterized3(SPD_WIN_TEXT_OPTION, 2, 204 - width, 1, sTextColors[SPC_COLOR_GREY], TEXT_SPEED_FF, gSpeedchoiceCurrentVersion);
    CopyWindowToVram(SPD_WIN_TEXT_OPTION, 3);
}

// Renders the frame for the options and choices window.
void DrawOptionsAndChoicesWindow(void)
{
    FillWindowPixelBuffer(SPD_WIN_OPTIONS, PIXEL_FILL(1));
    CopyWindowToVram(SPD_WIN_OPTIONS, 3);
}

// Self-explanatory.
u8 GetPageDrawCount(u8 page)
{
    if ((page * OPTIONS_PER_PAGE) > CURRENT_OPTIONS_NUM)
        return CURRENT_OPTIONS_NUM % OPTIONS_PER_PAGE;

    return OPTIONS_PER_PAGE;
}

/*
 * Given a page number, renders the page options.
 */
void DrawPageOptions(u8 page) // Page is 1-indexed
{
    u8 i;
    u8 drawCount = GetPageDrawCount(page);

    FillWindowPixelBuffer(SPD_WIN_OPTIONS, PIXEL_FILL(1));

    // print page options.
    for(i = 0; i < drawCount; i++)
    {
        struct SpeedchoiceOption *option = (struct SpeedchoiceOption *)&SpeedchoiceOptions[i + (OPTIONS_PER_PAGE * (page - 1))];
        const u8 *string = option->string;

        AddTextPrinterParameterized3(SPD_WIN_OPTIONS, 2, 4, NEWMENUOPTIONCOORDS(i), sTextColors[SPC_COLOR_GREY], TEXT_SPEED_FF, string);
        // TODO: Draw on SPD_WIN_OPTIONS, if it's broken
        DrawGeneralChoices(option, sLocalSpeedchoiceConfig.optionConfig[i + ((page - 1) * 5)], i);
    }

    AddTextPrinterParameterized3(SPD_WIN_OPTIONS, 2,4, NEWMENUOPTIONCOORDS(5), sTextColors[SPC_COLOR_GREY], TEXT_SPEED_FF,  gSpeedchoiceOptionPage);
    AddTextPrinterParameterized3(SPD_WIN_OPTIONS, 2,4, NEWMENUOPTIONCOORDS(6), sTextColors[SPC_COLOR_GREY], TEXT_SPEED_FF,  gSpeedchoiceOptionStartGame);
    DrawPageChoice(sLocalSpeedchoiceConfig.pageNum);
    CopyWindowToVram(SPD_WIN_OPTIONS, 3);
}

/*
 * Given a true index, convert it to the page index so that the general processor function can
 * set the correctly newly selected option.
 */
void SetPageIndexFromTrueIndex(u8 taskId, s16 index) // data is s16.
{
    if(index == PAGE)
        sLocalSpeedchoiceConfig.pageIndex = 5;
    else if(index == START_GAME)
        sLocalSpeedchoiceConfig.pageIndex = 6;
    else
        sLocalSpeedchoiceConfig.pageIndex = (oldmin((index % OPTIONS_PER_PAGE), OPTIONS_PER_PAGE));
}

// Copied from option menu. Fills the window frames.
static void sub_80BB154(void)
{
    //                   bg, tileNum, x,    y,    width, height,  pal
    FillBgTilemapBufferRect(1, 0x1A2, 1,    0,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A3, 2,    0,      0x1B,   1,      7);
    FillBgTilemapBufferRect(1, 0x1A4, 28,   0,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A5, 1,    1,      1,      2,      7);
    FillBgTilemapBufferRect(1, 0x1A7, 28,   1,      1,      2,      7);
    FillBgTilemapBufferRect(1, 0x1A8, 1,    3,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A9, 2,    3,      0x1B,   1,      7);
    FillBgTilemapBufferRect(1, 0x1AA, 28,   3,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A2, 1,    4,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A3, 2,    4,      0x1A,   1,      7);
    FillBgTilemapBufferRect(1, 0x1A4, 28,   4,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A5, 1,    5,      1,      0x12,   7);
    FillBgTilemapBufferRect(1, 0x1A7, 28,   5,      1,      0x12,   7);
    FillBgTilemapBufferRect(1, 0x1A8, 1,    19,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A9, 2,    19,     0x1A,   1,      7);
    FillBgTilemapBufferRect(1, 0x1AA, 28,   19,     1,      1,      7);

    CopyBgTilemapBufferToVram(1);
}
