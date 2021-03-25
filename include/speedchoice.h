#ifndef POKEFIRERED_SPEEDCHOICE_H
#define POKEFIRERED_SPEEDCHOICE_H

#include "constants/speedchoice.h"

// =======================================
// Global Speedchoice Configuration
// =======================================

#define STR_(x) #x
#define STR(x) STR_(x)

#define SPEEDCHOICE_VERSION_MAJOR 1
#define SPEEDCHOICE_VERSION_MINOR 0
#define SPEEDCHOICE_VERSION_RELSTEP 0

#define SPEEDCHOICE_VERSION STR(SPEEDCHOICE_VERSION_MAJOR) "." STR(SPEEDCHOICE_VERSION_MINOR) "." STR(SPEEDCHOICE_VERSION_RELSTEP)

// Maximum number of possible selectable options per option.
#define MAX_CHOICES 6

// Maximum number of options to print per page besides Start Game and Page.
#define OPTIONS_PER_PAGE 5

// Maximum number of options including Page and Start Game.
#define ALLOPTIONS_PER_PAGE OPTIONS_PER_PAGE + 2

// Number of defined pages.
#define MAX_PAGES ((CURRENT_OPTIONS_NUM + OPTIONS_PER_PAGE - 1) / OPTIONS_PER_PAGE)

// Deprecated macro. May have been in use for Sapphire Speedchoice when the window system
// was different back then.
#define MENUOPTIONCOORDS(i) (5 + (2 * i)) // DEPRECATED

// Deprecated macro. Using Random() formula for spinner timer: this define isnt used to
// check Spinner timing anymore.
#define SPINNER_HELL_TIMER 4 // DEPRECATED

// Maximum number of tile vision trainers have when Max Vision is set.
#define MAX_VISION_RANGE 8

// Number of frames per spinner iteration in Purge setting for Spinners.
#define PURGE_SPINNER_TIMER 48

// Deprecated macro. I think battle speed is defined seperately now in the battle_main file
// somewhere.
#define BATTLE_SPEED_FRACTION 4 // DEPRECATED

// Reserved macros for a future version for a wild encounter queue system.
// Not implemented yet.
#define GRASS_QUEUE_SIZE 20
#define WATER_QUEUE_SIZE 15
#define ROCKSMASH_QUEUE_SIZE 15
#define FISHING_QUEUE_SIZE 5

// To Avoid breaking stuff, we used old min/max macros. I'm too lazy to correct this.
#define oldmin(a, b) (a < b ? a : b)
#define oldmax(a, b) (a > b ? a : b)

/*
 * Every Speedchoice option uses a configuration to determine how to render it's options.
 * For example (Yes/No), (On/Off), etc. These Configurations determine each option's coordinate
 * to use and the string to use. Ideally I could probably do lots of fancy math to determine
 * the X as well but I'd rather just hardcode it for detailing.
 */
struct OptionChoiceConfig
{
    s16 x; // do not store the Y coordinate. Y is automatically calculated depending on the row the option is placed and therefore is not necessary.
    u8 *string; // can be null
};

/*
 * To define an option, this global configuration struct is used to define every option that is
 * selectable or togglable.
 */
struct SpeedchoiceOption
{
    // Number of options. (2 = On/Off, etc)
    u8 optionCount;

    // see option type enumeration above.
    u8 optionType;

    // Pointer to the name of the option.
    const u8 *string;

    // Pointer to the array of Option configuration array. An array of MAX_CHOICES since no
    // need to define more than that. Use NULL for static options.
    const struct OptionChoiceConfig *options;

    // Pointer to Tooltip string. Printed when SELECT is pressed on the option.
    const u8 *tooltip;

    // Determines if the option is enabled or not. This was used during Speedchoice development.
    bool8 enabled;
};

/*
 * Before writing to the Save Block, we store the configuration in RAM temporarily before
 * flushing it to the Save Block.
 */
struct SpeedchoiceConfigStruct
{
    // The currently stored selected options per Speedchoice option. These are written
    // to the save block at the end.
    u8 optionConfig[CURRENT_OPTIONS_NUM];

    // The true ID of the selected page. This takes into account number of pages, etc.
    // If you're on Page 2, the trueIndex of the first option is 8. (7+1)
    u8 trueIndex;

    // The page ID of the option on the current page. 2nd option on page 2 is 2, etc.
    u8 pageIndex;

    // Current page number.
    u8 pageNum;
};

/*
 * In Gen 3, there is an oversight by Game Freak where upon going to a sub menu and returning
 * the spinner timers are reset due to reloading the field. Since spinners dont start trying
 * to spin until later, this oversight can be used to run past spinners safely without
 * encountering them. Because this trivializes Spinner Hell, we fix this when Hell/Why is
 * toggled by backing up the NPC timers and restoring them upon returning to the field. We skip
 * the player object's timer to avoid sprite issues.
 */
struct MapObjectTimerBackup
{
    bool8 backedUp;
    s16 spriteId;
    s16 timer;
};

/*
 * we need to extern the randomizer check value. If the ROM could see the value assigned
 * here (which is 0 by default), then it would be optimized out, but we need this location
 * to be externally written to by the randomizer.
 */
extern const u32 gRandomizerCheckValue;

// ----------------------
// Prototypes
// ----------------------
void CB2_InitSpeedchoice(void);
bool8 CheckSpeedchoiceOption(u8, u8);
u32 CalculateCheckValue(void);

#endif // POKEFIRERED_SPEEDCHOICE_H