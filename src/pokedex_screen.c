#include "global.h"
#include "gflib.h"
#include "graphics.h"
#include "m4a.h"
#include "scanline_effect.h"
#include "task.h"
#include "new_menu_helpers.h"
#include "event_data.h"
#include "help_system.h"
#include "menu_indicators.h"
#include "overworld.h"
#include "strings.h"
#include "menu.h"
#include "pokedex_screen.h"
#include "data.h"
#include "pokedex.h"
#include "trainer_pokemon_sprites.h"
#include "decompress.h"
#include "constants/songs.h"
#include "pokedex_area_markers.h"
#include "field_specials.h"

enum TextMode {
    TEXT_LEFT,
    TEXT_CENTER,
    TEXT_RIGHT
};

struct PokedexScreenData
{
    u8 taskId;
    u8 state;
    u8 data[4];
    u32 unlockedCategories;
    u32 modeSelectInput;
    u16 modeSelectItemsAbove;
    u16 modeSelectCursorPos;
    u8 modeSelectWindowId;
    u8 selectionIconWindowId;
    u8 dexCountsWindowId;
    u8 modeSelectListMenuId;
    u16 pageSpecies[4];
    u8 categoryMonWindowIds[4];
    u8 categoryMonInfoWindowIds[4];
    u8 category;
    u8 firstPageInCategory;
    u8 lastPageInCategory;
    u8 pageNum;
    u8 numMonsOnPage;
    u8 categoryCursorPosInPage;
    u8 categoryPageSelectionCursorTimer;
    u8 parentOfCategoryMenu;
    u32 characteristicMenuInput;
    u16 kantoOrderMenuItemsAbove;
    u16 kantoOrderMenuCursorPos;
    u16 characteristicOrderMenuItemsAbove;
    u16 characteristicOrderMenuCursorPos;
    u16 nationalOrderMenuItemsAbove;
    u16 nationalOrderMenuCursorPos;
    u8 numericalOrderWindowId;
    u8 orderedListMenuTaskId;
    u8 dexOrderId;
    struct ListMenuItem * listItems;
    u16 orderedDexCount;
    u8 field_4A[0x10];
    u16 field_5A;
    u16 * field_5C;
    u8 scrollArrowsTaskId;
    u8 categoryPageCursorTaskId;
    u16 modeSelectCursorPosBak;
    u8 field_64;
    u16 numSeenKanto;
    u16 numOwnedKanto;
    u16 numSeenNational;
    u16 numOwnedNational;
};

struct PokedexScreenWindowGfx
{
    const u16 * map;
    const u16 * pal;
};

struct PokedexCategoryPage
{
    const u16 * species;
    u8 count;
};

EWRAM_DATA static struct PokedexScreenData * sPokedexScreenData = NULL;

static void Task_PokedexScreen(u8 taskId);
static void DexScreen_InitGfxForTopMenu(void);
static void Task_DexScreen_NumericalOrder(u8 taskId);
static void DexScreen_InitGfxForNumericalOrderList(void);
static void Task_DexScreen_CharacteristicOrder(u8 taskId);
static void DexScreen_CreateCharacteristicListMenu(void);
static u16 DexScreen_CountMonsInOrderedList(u8 orderIdx);
static void DexScreen_InitListMenuForOrderedList(const struct ListMenuTemplate * a0, u8 order);
static u8 DexScreen_CreateDexOrderScrollArrows(void);
static void DexScreen_DestroyDexOrderListMenu(u8 order);
static void Task_DexScreen_CategorySubmenu(u8 taskId);
static u8 sub_8104234(void);
static int sub_8104284(void);
static void Task_DexScreen_ShowMonPage(u8 taskId);
static bool32 sub_8104664(u8 a0);
static void DexScreen_RemoveWindow(u8 *windowId_p);
static void DexScreen_AddTextPrinterParameterized(u8 windowId, u8 fontId, const u8 *str, u8 x, u8 y, u8 colorIdx);
static void DexScreen_PrintNum3RightAlign(u8 windowId, u8 fontId, u16 num, u8 x, u8 y, u8 colorIdx);
static void DexScreen_PrintMonDexNo(u8 windowId, u8 fontId, u16 species, u8 x, u8 y);
static u16 DexScreen_GetDexCount(u8 caseId, bool8 whichDex);
static void DexScreen_PrintControlInfo(const u8 *src);
static void DexScreen_DestroyCategoryPageMonIconAndInfoWindows(void);
static bool8 DexScreen_CreateCategoryListGfx(bool8 justRegistered);
static void DexScreen_CreateCategoryPageSelectionCursor(u8 cursorPos);
static void DexScreen_UpdateCategoryPageCursorObject(u8 taskId, u8 cursorPos, u8 numMonsInPage);
static bool8 sub_81052D0(u8 a0);
void sub_8105594(u8 a0, u8 a1);
static u8 DexScreen_DrawMonDexPage(bool8 justRegistered);
u8 sub_8106014(void);
u8 sub_810603C(void);
static bool8 DexScreen_IsPageUnlocked(u8 category, u8 pageNum);
static bool8 DexScreen_IsCategoryUnlocked(u8 category);
static u8 DexScreen_GetPageLimitsForCategory(u8 category);
static bool8 DexScreen_LookUpCategoryBySpecies(u16 species);
u8 sub_81067C0(void);
void DexScreen_CreateCategoryPageSpeciesList(u8 category, u8 pageNum);
static u8 DexScreen_PageNumberToRenderablePages(u16 page);
void sub_8106B34(void);
void DexScreen_PrintStringWithAlignment(const u8 *str, enum TextMode mode);
static void MoveCursorFunc_DexModeSelect(s32 itemIndex, bool8 onInit, struct ListMenu *list);
static void ItemPrintFunc_DexModeSelect(u8 windowId, s32 itemId, u8 y);
static void ItemPrintFunc_OrderedListMenu(u8 windowId, s32 itemId, u8 y);
static void Task_DexScreen_RegisterNonKantoMonBeforeNationalDex(u8 taskId);
static void Task_DexScreen_RegisterMonToPokedex(u8 taskId);

#include "data/pokemon_graphics/footprint_table.h"

const u8 sCategoryMonInfoBgTiles[] = INCBIN_U8("graphics/pokedex/unk_8440124.bin.lz");
const u8 sKantoDexTiles[] = INCBIN_U8("graphics/pokedex/unk_8440274.4bpp.lz");
const u8 sNatDexTiles[] = INCBIN_U8("graphics/pokedex/unk_84403AC.4bpp.lz");
const u16 sKantoDexPalette[] = INCBIN_U16("graphics/pokedex/unk_84404C8.gbapal");

const u16 sDexScreen_CategoryCursorPals[] = {
    RGB(24, 22, 17), RGB(26, 24, 20),
    RGB(26, 20, 15), RGB(27, 23, 19),
    RGB(28, 18, 15), RGB(28, 22, 19),
    RGB(30, 16, 13), RGB(29, 21, 18),
    RGB(28, 18, 15), RGB(28, 22, 19),
    RGB(26, 20, 15), RGB(27, 23, 19)
};

const u16 sNationalDexPalette[] = INCBIN_U16("graphics/pokedex/unk_84406E0.gbapal");
const u16 gUnknown_84408E0[] = INCBIN_U16("graphics/pokedex/unk_84408E0.bin.lz");
const u16 gUnknown_8440BD8[] = INCBIN_U16("graphics/pokedex/unk_8440BD8.bin.lz");
const u32 sTopMenuSelectionIconTiles_Cancel[] = INCBIN_U32("graphics/pokedex/unk_8440EF0.bin.lz");
const u16 gUnknown_844112C[] = INCBIN_U16("graphics/pokedex/unk_844112C.bin.lz");
const u16 gUnknown_84414BC[] = INCBIN_U16("graphics/pokedex/unk_84414BC.bin.lz");
const u32 gUnknown_8441808[] = INCBIN_U32("graphics/pokedex/unk_8441808.bin.lz");
const u16 gUnknown_8441A40[] = INCBIN_U16("graphics/pokedex/unk_8441A40.bin.lz");
const u16 gUnknown_8441D54[] = INCBIN_U16("graphics/pokedex/unk_8441D54.bin.lz");
const u16 gUnknown_8442004[] = INCBIN_U16("graphics/pokedex/unk_8442004.bin.lz");
const u16 gUnknown_844223C[] = INCBIN_U16("graphics/pokedex/unk_844223C.bin.lz");
const u16 gUnknown_84424E4[] = INCBIN_U16("graphics/pokedex/unk_84424E4.bin.lz");
const u16 gUnknown_8442838[] = INCBIN_U16("graphics/pokedex/unk_8442838.bin.lz");
const u16 gUnknown_8442BC0[] = INCBIN_U16("graphics/pokedex/unk_8442BC0.bin.lz");
const u16 gUnknown_8442EF8[] = INCBIN_U16("graphics/pokedex/unk_8442EF8.bin.lz");
const u16 gUnknown_844318C[] = INCBIN_U16("graphics/pokedex/unk_844318C.bin.lz");
const u16 gUnknown_8443420[] = INCBIN_U16("graphics/pokedex/unk_8443420.gbapal");
const u16 gUnknown_8443440[] = INCBIN_U16("graphics/pokedex/unk_8443440.gbapal");
const u16 sTopMenuSelectionIconPals_Cancel[] = INCBIN_U16("graphics/pokedex/unk_8443460.gbapal");
const u16 gUnknown_8443480[] = INCBIN_U16("graphics/pokedex/unk_8443480.gbapal");
const u16 gUnknown_84434A0[] = INCBIN_U16("graphics/pokedex/unk_84434A0.gbapal");
const u16 gUnknown_84434C0[] = INCBIN_U16("graphics/pokedex/unk_84434C0.gbapal");
const u16 gUnknown_84434E0[] = INCBIN_U16("graphics/pokedex/unk_84434E0.gbapal");
const u16 gUnknown_8443500[] = INCBIN_U16("graphics/pokedex/unk_8443500.gbapal");
const u16 gUnknown_8443520[] = INCBIN_U16("graphics/pokedex/unk_8443520.gbapal");
const u16 gUnknown_8443540[] = INCBIN_U16("graphics/pokedex/unk_8443540.gbapal");
const u16 gUnknown_8443560[] = INCBIN_U16("graphics/pokedex/unk_8443560.gbapal");
const u16 gUnknown_8443580[] = INCBIN_U16("graphics/pokedex/unk_8443580.gbapal");
const u16 gUnknown_84435A0[] = INCBIN_U16("graphics/pokedex/unk_84435A0.gbapal");
const u16 gUnknown_84435C0[] = INCBIN_U16("graphics/pokedex/unk_84435C0.gbapal");
const u16 gUnknown_84435E0[] = INCBIN_U16("graphics/pokedex/unk_84435E0.gbapal");
const u8 gUnknown_8443600[] = INCBIN_U8("graphics/pokedex/unk_8443600.4bpp");
const u32 gUnknown_8443620[] = INCBIN_U32("graphics/pokedex/unk_8443620.bin.lz");
const u32 gUnknown_8443910[] = INCBIN_U32("graphics/pokedex/unk_8443910.bin.lz");
const u32 gUnknown_8443988[] = INCBIN_U32("graphics/pokedex/unk_8443988.bin.lz");
const u32 gUnknown_84439FC[] = INCBIN_U32("graphics/pokedex/unk_84439FC.bin.lz");
const u32 gUnknown_8443A78[] = INCBIN_U32("graphics/pokedex/unk_8443A78.bin.lz");
const u32 gUnknown_8443AF8[] = INCBIN_U32("graphics/pokedex/unk_8443AF8.bin.lz");
const u32 gUnknown_8443BB0[] = INCBIN_U32("graphics/pokedex/unk_8443BB0.bin.lz");
const u32 gUnknown_8443C54[] = INCBIN_U32("graphics/pokedex/unk_8443C54.bin.lz");
const u16 gUnknown_8443D00[] = INCBIN_U16("graphics/pokedex/unk_8443D00.4bpp");

#include "data/pokemon/pokedex_orders.h"

static const u8 gExpandedPlaceholder_PokedexDescription[] = _("");

#include "data/pokemon/pokedex_text.h"
#include "data/pokemon/pokedex_entries.h"

static const struct BgTemplate sBgTemplates[] = {
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 5,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0x0000
    },
    {
        .bg = 1,
        .charBaseIndex = 2,
        .mapBaseIndex = 4,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0x0000
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 6,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0x0000
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 7,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0x0000
    },
};

static const struct WindowTemplate sWindowTemplates[] = {
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x03c4
    },
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 18,
        .width = 30,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x0388
    },
    {
        .bg = 255,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 0x0000
    },
};

static const struct PokedexScreenData sDexScreenDataInitialState = {
    .modeSelectItemsAbove = 1,
    .modeSelectWindowId = -1,
    .selectionIconWindowId = -1,
    .dexCountsWindowId = -1,
    .pageSpecies = {-1, -1, -1, -1},
    .categoryMonWindowIds = {-1, -1, -1, -1},
    .categoryMonInfoWindowIds = {-1, -1, -1, -1},
    .numericalOrderWindowId = -1, 
    .field_4A = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    .scrollArrowsTaskId = -1, 
    .categoryPageCursorTaskId = -1,
};


static const struct WindowTemplate sWindowTemplate_ModeSelect = {
   .bg = 1,
   .tilemapLeft = 1,
   .tilemapTop = 2,
   .width = 20,
   .height = 16,
   .paletteNum = 0,
   .baseBlock = 0x0008
 };

static const struct WindowTemplate sWindowTemplate_SelectionIcon = {
   .bg = 1,
   .tilemapLeft = 21,
   .tilemapTop = 11,
   .width = 8,
   .height = 6,
   .paletteNum = 1,
   .baseBlock = 0x0148
 };

static const struct WindowTemplate sWindowTemplate_DexCounts = {
   .bg = 1,
   .tilemapLeft = 21,
   .tilemapTop = 2,
   .width = 9,
   .height = 9,
   .paletteNum = 0,
   .baseBlock = 0x0178
 };

static const struct ListMenuItem sListMenuItems_KantoDexModeSelect[] = {
    {gText_PokemonList,                  LIST_HEADER},
    {gText_NumericalMode,                DEX_MODE(NUMERICAL_KANTO)},
    {gText_PokemonHabitats,              LIST_HEADER},
    {gText_DexCategory_GrasslandPkmn,    DEX_CATEGORY_GRASSLAND},
    {gText_DexCategory_ForestPkmn,       DEX_CATEGORY_FOREST},
    {gText_DexCategory_WatersEdgePkmn,   DEX_CATEGORY_WATERS_EDGE},
    {gText_DexCategory_SeaPkmn,          DEX_CATEGORY_SEA},
    {gText_DexCategory_CavePkmn,         DEX_CATEGORY_CAVE},
    {gText_DexCategory_MountainPkmn,     DEX_CATEGORY_MOUNTAIN},
    {gText_DexCategory_RoughTerrainPkmn, DEX_CATEGORY_ROUGH_TERRAIN},
    {gText_DexCategory_UrbanPkmn,        DEX_CATEGORY_URBAN},
    {gText_DexCategory_RarePkmn,         DEX_CATEGORY_RARE},
    {gText_Search,                       LIST_HEADER},
    {gText_AToZMode,                     DEX_MODE(ATOZ)},
    {gText_TypeMode,                     DEX_MODE(TYPE)},
    {gText_LightestMode,                 DEX_MODE(LIGHTEST)},
    {gText_SmallestMode,                 DEX_MODE(SMALLEST)},
    {gText_PokedexOther,                 LIST_HEADER},
    {gText_ClosePokedex,                 LIST_CANCEL},
};

static const struct ListMenuTemplate sListMenuTemplate_KantoDexModeSelect = {
    .items = sListMenuItems_KantoDexModeSelect,
    .moveCursorFunc = MoveCursorFunc_DexModeSelect,
    .itemPrintFunc = ItemPrintFunc_DexModeSelect,
    .totalItems = NELEMS(sListMenuItems_KantoDexModeSelect), 
    .maxShowed = 9,
    .windowId = 0, 
    .header_X = 0, 
    .item_X = 12, 
    .cursor_X = 4,
    .upText_Y = 2,
    .cursorPal = 1,
    .fillValue = 0,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = 0,
    .fontId = 2,
    .cursorKind = 0,
};

static const struct ListMenuItem sListMenuItems_NatDexModeSelect[] = {
    {gText_PokemonList,                  LIST_HEADER},
    {gText_NumericalModeKanto,           DEX_MODE(NUMERICAL_KANTO)},
    {gText_NumericalModeNational,        DEX_MODE(NUMERICAL_NATIONAL)},
    {gText_PokemonHabitats,              LIST_HEADER},
    {gText_DexCategory_GrasslandPkmn,    DEX_CATEGORY_GRASSLAND},
    {gText_DexCategory_ForestPkmn,       DEX_CATEGORY_FOREST},
    {gText_DexCategory_WatersEdgePkmn,   DEX_CATEGORY_WATERS_EDGE},
    {gText_DexCategory_SeaPkmn,          DEX_CATEGORY_SEA},
    {gText_DexCategory_CavePkmn,         DEX_CATEGORY_CAVE},
    {gText_DexCategory_MountainPkmn,     DEX_CATEGORY_MOUNTAIN},
    {gText_DexCategory_RoughTerrainPkmn, DEX_CATEGORY_ROUGH_TERRAIN},
    {gText_DexCategory_UrbanPkmn,        DEX_CATEGORY_URBAN},
    {gText_DexCategory_RarePkmn,         DEX_CATEGORY_RARE},
    {gText_Search,                       LIST_HEADER},
    {gText_AToZMode,                     DEX_MODE(ATOZ)},
    {gText_TypeMode,                     DEX_MODE(TYPE)},
    {gText_LightestMode,                 DEX_MODE(LIGHTEST)},
    {gText_SmallestMode,                 DEX_MODE(SMALLEST)},
    {gText_PokedexOther,                 LIST_HEADER},
    {gText_ClosePokedex,                 LIST_CANCEL},
};

static const struct ListMenuTemplate sListMenuTemplate_NatDexModeSelect = {
    .items = sListMenuItems_NatDexModeSelect,
    .moveCursorFunc = MoveCursorFunc_DexModeSelect,
    .itemPrintFunc = ItemPrintFunc_DexModeSelect,
    .totalItems = NELEMS(sListMenuItems_NatDexModeSelect),
    .maxShowed = 9,
    .windowId = 0, 
    .header_X = 0, 
    .item_X = 12, 
    .cursor_X = 4,
    .upText_Y = 2,
    .cursorPal = 1,
    .fillValue = 0,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = 0,
    .fontId = 2,
    .cursorKind = 0,
};

static const struct ScrollArrowsTemplate sScrollArrowsTemplate_KantoDex = {
    .firstArrowType = 2, 
    .firstX = 200, 
    .firstY = 19, 
    .secondArrowType = 3, 
    .secondX = 200, 
    .secondY = 141,
    .fullyUpThreshold = 0, 
    .fullyDownThreshold = 10, 
    .tileTag = 2000, 
    .palTag = 0xFFFF,
    .palNum = 1
};

static const struct ScrollArrowsTemplate sScrollArrowsTemplate_NatDex = {
    .firstArrowType = 2, 
    .firstX = 200, 
    .firstY = 19, 
    .secondArrowType = 3, 
    .secondX = 200, 
    .secondY = 141,
    .fullyUpThreshold = 0, 
    .fullyDownThreshold = 11, 
    .tileTag = 2000, 
    .palTag = 0xFFFF,
    .palNum = 1
};


static const struct PokedexScreenWindowGfx sTopMenuSelectionIconGfxPtrs[] = {
    {.map = gUnknown_84414BC, .pal = gUnknown_84434A0},
    {.map = gUnknown_844112C, .pal = gUnknown_8443480},
    {.map = gUnknown_8442838, .pal = gUnknown_8443580},
    {.map = gUnknown_8442004, .pal = gUnknown_8443520},
    {.map = gUnknown_84408E0, .pal = gUnknown_8443420},
    {.map = gUnknown_8441A40, .pal = gUnknown_84434E0},
    {.map = gUnknown_84424E4, .pal = gUnknown_8443560},
    {.map = gUnknown_8440BD8, .pal = gUnknown_8443440},
    {.map = gUnknown_8441D54, .pal = gUnknown_8443500},
    {.map = gUnknown_844223C, .pal = gUnknown_8443540},
    {.map = gUnknown_8E9C16C, .pal = gUnknown_8E9C14C},
    {.map = gUnknown_8442BC0, .pal = gUnknown_84435A0},
    {.map = gUnknown_8442EF8, .pal = gUnknown_84435C0},
    {.map = gUnknown_844318C, .pal = gUnknown_84435E0},
    {.map = gUnknown_844223C, .pal = gUnknown_8443540},
};

static const struct WindowTemplate sWindowTemplate_OrderedListMenu = {
    .bg = 1,
    .tilemapLeft = 2,
    .tilemapTop = 2,
    .width = 23,
    .height = 16,
    .paletteNum = 0,
    .baseBlock = 0x0008
};

static const struct ListMenuTemplate sListMenuTemplate_OrderedListMenu = {
    .items = sListMenuItems_KantoDexModeSelect,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .itemPrintFunc = ItemPrintFunc_OrderedListMenu,
    .totalItems = 0, 
    .maxShowed = 9,
    .windowId = 0, 
    .header_X = 0, 
    .item_X = 56, 
    .cursor_X = 4,
    .upText_Y = 2,
    .cursorPal = 1,
    .fillValue = 0,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = 1,
    .fontId = 2,
    .cursorKind = 0,
};

static const struct ListMenuWindowRect sListMenuRect_OrderedList = {
    .x = 0, 
    .y = 0, 
    .width = 5, 
    .height = 16, 
    .palNum = 0,
};

// Unused
static const u8 gUnknown_8452194[] = {
    0x05, 0x00, 0x02, 0x10, 0x01, 0x00, 0x00, 0x00, 
    0x07, 0x00, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 
    0x0f, 0x00, 0x08, 0x10, 0x02, 0x00, 0x00, 0x00, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00
};

static const struct ScrollArrowsTemplate sDexOrderScrollArrowsTemplate = {
    .firstArrowType = 2, 
    .firstX = 200, 
    .firstY = 19, 
    .secondArrowType = 3, 
    .secondX = 200, 
    .secondY = 141,
    .fullyUpThreshold = 0, 
    .fullyDownThreshold = 0, 
    .tileTag = 2000, 
    .palTag = 0xFFFF,
    .palNum = 1,
};

static const struct WindowTemplate sWindowTemplate_CategoryMonIcon = {
    .bg = 2,
    .tilemapLeft = 0,
    .tilemapTop = 0,
    .width = 8,
    .height = 8,
    .paletteNum = 0,
    .baseBlock = 0x0000
};

static const struct WindowTemplate sWindowTemplate_CategoryMonInfo = {
    .bg = 1,
    .tilemapLeft = 0,
    .tilemapTop = 0,
    .width = 8,
    .height = 5,
    .paletteNum = 0,
    .baseBlock = 0x0000
};

const struct WindowTemplate gUnknown_84521D4 = {
    .bg = 1,
    .tilemapLeft = 19,
    .tilemapTop = 3,
    .width = 8,
    .height = 8,
    .paletteNum = 9,
    .baseBlock = 0x01a8
};

const struct WindowTemplate gUnknown_84521DC = {
    .bg = 1,
    .tilemapLeft = 2,
    .tilemapTop = 3,
    .width = 13,
    .height = 8,
    .paletteNum = 0,
    .baseBlock = 0x01e8
};

const struct WindowTemplate gUnknown_84521E4 = {
    .bg = 1,
    .tilemapLeft = 0,
    .tilemapTop = 11,
    .width = 30,
    .height = 7,
    .paletteNum = 0,
    .baseBlock = 0x0250
};

const struct WindowTemplate gUnknown_84521EC = {
    .bg = 2,
    .tilemapLeft = 1,
    .tilemapTop = 2,
    .width = 4,
    .height = 4,
    .paletteNum = 10,
    .baseBlock = 0x01a8
};

const struct WindowTemplate gUnknown_84521F4 = {
    .bg = 2,
    .tilemapLeft = 5,
    .tilemapTop = 2,
    .width = 8,
    .height = 3,
    .paletteNum = 0,
    .baseBlock = 0x01b8
};

const struct WindowTemplate gUnknown_84521FC = {
    .bg = 2,
    .tilemapLeft = 2,
    .tilemapTop = 7,
    .width = 10,
    .height = 2,
    .paletteNum = 0,
    .baseBlock = 0x01d0
};

const struct WindowTemplate gUnknown_8452204 = {
    .bg = 2,
    .tilemapLeft = 18,
    .tilemapTop = 2,
    .width = 10,
    .height = 2,
    .paletteNum = 0,
    .baseBlock = 0x01e4
};

const struct WindowTemplate gUnknown_845220C = {
    .bg = 2,
    .tilemapLeft = 5,
    .tilemapTop = 5,
    .width = 8,
    .height = 2,
    .paletteNum = 11,
    .baseBlock = 0x01f8
};

const struct WindowTemplate gUnknown_8452214 = {
    .bg = 2,
    .tilemapLeft = 17,
    .tilemapTop = 4,
    .width = 12,
    .height = 9,
    .paletteNum = 0,
    .baseBlock = 0x0208
};

static const struct WindowTemplate sUnknown_845221C = {
    .bg = 2,
    .tilemapLeft = 13,
    .tilemapTop = 4,
    .width = 4,
    .height = 3,
    .paletteNum = 0,
    .baseBlock = 0x0274
};

static const struct WindowTemplate sUnknown_8452224 = {
    .bg = 2,
    .tilemapLeft = 13,
    .tilemapTop = 7,
    .width = 4,
    .height = 3,
    .paletteNum = 0,
    .baseBlock = 0x0280
};

static const struct WindowTemplate sUnknown_845222C = {
    .bg = 2,
    .tilemapLeft = 13,
    .tilemapTop = 10,
    .width = 4,
    .height = 3,
    .paletteNum = 0,
    .baseBlock = 0x028c
};

static const struct WindowTemplate sUnknown_8452234 = {
    .bg = 2,
    .tilemapLeft = 13,
    .tilemapTop = 13,
    .width = 4,
    .height = 4,
    .paletteNum = 0,
    .baseBlock = 0x0298
};

static const struct WindowTemplate sUnknown_845223C = {
    .bg = 2,
    .tilemapLeft = 17,
    .tilemapTop = 13,
    .width = 4,
    .height = 4,
    .paletteNum = 0,
    .baseBlock = 0x02a8
};

static const struct WindowTemplate sUnknown_8452244 = {
    .bg = 2,
    .tilemapLeft = 21,
    .tilemapTop = 13,
    .width = 4,
    .height = 4,
    .paletteNum = 0,
    .baseBlock = 0x02b8
};

static const struct WindowTemplate sUnknown_845224C = {
    .bg = 2,
    .tilemapLeft = 25,
    .tilemapTop = 13,
    .width = 4,
    .height = 4,
    .paletteNum = 0,
    .baseBlock = 0x02c8
};

struct {
    const struct WindowTemplate * window;
    const u32 * tilemap;
} const gUnknown_8452254[] = {
    {&sUnknown_845221C, gUnknown_8443910},
    {&sUnknown_8452224, gUnknown_8443988},
    {&sUnknown_845222C, gUnknown_84439FC},
    {&sUnknown_8452234, gUnknown_8443A78},
    {&sUnknown_845223C, gUnknown_8443AF8},
    {&sUnknown_8452244, gUnknown_8443BB0},
    {&sUnknown_845224C, gUnknown_8443C54},
};

static const u16 sCategoryPageIconWindowBg[] = INCBIN_U16("graphics/pokedex/unk_845228C.bin");

static const u8 sUnknown_845230C[][4] = {
    {0x0b, 0x03, 0x0b, 0x0b},
};

static const u8 sUnknown_8452310[][4] = {
    {0x03, 0x03, 0x0b, 0x03},
    {0x12, 0x09, 0x0a, 0x0b},
};

static const u8 sUnknown_8452318[][4] = {
    {0x01, 0x02, 0x09, 0x02},
    {0x0b, 0x09, 0x03, 0x0b},
    {0x15, 0x03, 0x15, 0x0b}
};

static const u8 sUnknown_8452324[][4] = {
    {0x00, 0x02, 0x06, 0x03},
    {0x07, 0x0a, 0x00, 0x0c},
    {0x0f, 0x0a, 0x16, 0x0b},
    {0x16, 0x02, 0x0f, 0x04}
};

const u8 (*const sCategoryPageIconCoords[])[4] = {
    sUnknown_845230C,
    sUnknown_8452310,
    sUnknown_8452318,
    sUnknown_8452324,
};

static const u8 * const sDexCategoryNamePtrs[] = {
    gText_DexCategory_GrasslandPkmn,
    gText_DexCategory_ForestPkmn,
    gText_DexCategory_WatersEdgePkmn,
    gText_DexCategory_SeaPkmn,
    gText_DexCategory_CavePkmn,
    gText_DexCategory_MountainPkmn,
    gText_DexCategory_RoughTerrainPkmn,
    gText_DexCategory_UrbanPkmn,
    gText_DexCategory_RarePkmn,
};

const u16 gUnknown_8452368[] = INCBIN_U16("graphics/pokedex/unk_8452368.gbapal");

static const u8 sUnknown_8452388[][30] = {
    {
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e
    }, {
        0x05, 0x0b, 0x11, 0x17, 0x1d, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e
    }, {
        0x02, 0x05, 0x08, 0x0b, 0x0e, 0x11, 0x14, 0x17, 0x1a, 0x1d, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e
    }, {
        0x02, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f, 0x11, 0x13, 
        0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e
    }, {
        0x02, 0x04, 0x05, 0x07, 0x08, 0x0a, 0x0b, 0x0d, 0x0e, 0x10, 
        0x11, 0x13, 0x14, 0x16, 0x17, 0x19, 0x1a, 0x1c, 0x1d, 0x1e, 
        0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e
    }, {
        0x01, 0x02, 0x03, 0x04, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
        0x0d, 0x0f, 0x10, 0x11, 0x13, 0x14, 0x15, 0x17, 0x18, 0x19, 
        0x1b, 0x1c, 0x1d, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e
    }, {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0a, 0x0b, 
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 
        0x17, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1e, 0x1e, 0x1e
    }, {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0b, 
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x15, 0x16, 
        0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1e, 0x1e
    }, {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 
        0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 
        0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e
    }, {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 
        0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 
        0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
    },
};

static const struct ScrollArrowsTemplate sUnknown_84524B4 = {
    .firstArrowType = 0, 
    .firstX = 16, 
    .firstY = 80, 
    .secondArrowType = 1, 
    .secondX = 224, 
    .secondY = 80,
    .fullyUpThreshold = 0, 
    .fullyDownThreshold = 0, 
    .tileTag = 2000, 
    .palTag = 0xFFFF,
    .palNum = 1,
};

const struct CursorStruct gUnknown_84524C4 = {
    .left = 0, 
    .top = 160,
    .rowWidth = 64, 
    .rowHeight = 40, 
    .tileTag = 2002, 
    .palTag = 0xFFFF,
    .palNum = 4,
};

#include "data/pokemon/pokedex_categories.h"

void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_PokedexScreen(void)
{
    if (!gPaletteFade.active || IsDma3ManagerBusyWithBgCopy())
    {
        RunTasks();
        RunTextPrinters();
        AnimateSprites();
        BuildOamBuffer();
    }
    else
    {
        UpdatePaletteFade();
    }
}

void DexScreen_LoadResources(void)
{
    bool8 natDex;
    u8 taskId;

    natDex = IsNationalPokedexEnabled();
    m4aSoundVSyncOff();
    SetVBlankCallback(NULL);
    ResetPaletteFade();
    ResetSpriteData();
    ResetTasks();
    ScanlineEffect_Stop();
    ResetBgsAndClearDma3BusyFlags(TRUE);
    InitBgsFromTemplates(0, sBgTemplates, NELEMS(sBgTemplates));
    SetBgTilemapBuffer(3, (u16*)Alloc(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(2, (u16*)Alloc(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(1, (u16*)Alloc(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(0, (u16*)Alloc(BG_SCREEN_SIZE));
    if (natDex)
        DecompressAndLoadBgGfxUsingHeap(3, (void*)sNatDexTiles, BG_SCREEN_SIZE, 0, 0);
    else
        DecompressAndLoadBgGfxUsingHeap(3, (void*)sKantoDexTiles, BG_SCREEN_SIZE, 0, 0);
    InitWindows(sWindowTemplates);
    DeactivateAllTextPrinters();
    m4aSoundVSyncOn();
    SetVBlankCallback(VBlankCB);
    EnableInterrupts(INTR_FLAG_VBLANK);
    taskId = CreateTask(Task_PokedexScreen, 0);
    sPokedexScreenData = Alloc(sizeof(struct PokedexScreenData));
    *sPokedexScreenData = sDexScreenDataInitialState;
    sPokedexScreenData->taskId = taskId;
    sPokedexScreenData->listItems = Alloc(NATIONAL_DEX_COUNT * sizeof(struct ListMenuItem));
    sPokedexScreenData->numSeenNational = DexScreen_GetDexCount(FLAG_GET_SEEN, 1);
    sPokedexScreenData->numOwnedNational = DexScreen_GetDexCount(FLAG_GET_CAUGHT, 1);
    sPokedexScreenData->numSeenKanto = DexScreen_GetDexCount(FLAG_GET_SEEN, 0);
    sPokedexScreenData->numOwnedKanto = DexScreen_GetDexCount(FLAG_GET_CAUGHT, 0);
    SetBGMVolume_SuppressHelpSystemReduction(0x80);
    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    ChangeBgX(1, 0, 0);
    ChangeBgY(1, 0, 0);
    ChangeBgX(2, 0, 0);
    ChangeBgY(2, 0, 0);
    ChangeBgX(3, 0, 0);
    ChangeBgY(3, 0, 0);
    gPaletteFade.bufferTransferDisabled = TRUE;
    if (natDex)
        LoadPalette(sNationalDexPalette, 0, 0x200);
    else
        LoadPalette(sKantoDexPalette, 0, 0x200);
    FillBgTilemapBufferRect(3, 0x001, 0, 0, 32, 32, 0);
    FillBgTilemapBufferRect(2, 0x000, 0, 0, 32, 32, 0x11);
    FillBgTilemapBufferRect(1, 0x000, 0, 0, 32, 32, 0x11);
    FillBgTilemapBufferRect(0, 0x0003, 0, 0, 32, 2, 0xF);
    FillBgTilemapBufferRect(0, 0x0000, 0, 2, 32, 16, 0x11);
    FillBgTilemapBufferRect(0, 0x003, 0, 18, 32, 2, 0xF);
}

void CB2_OpenPokedexFromStartMenu(void)
{
    DexScreen_LoadResources();
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
    SetMainCallback2(CB2_PokedexScreen);
    SetHelpContext(HELPCONTEXT_POKEDEX);
}

#define FREE_IF_NOT_NULL(ptr0) ({ void * ptr = (ptr0); if (ptr) Free(ptr); })

bool8 DoClosePokedex(void)
{
    switch (gMain.state)
    {
    case 0:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gMain.state++;
        return FALSE;
    case 1:
        if (!gPaletteFade.active)
            gMain.state = 2;
        else
            UpdatePaletteFade();
        return FALSE;
    case 2:
        FREE_IF_NOT_NULL(sPokedexScreenData->listItems);
        FREE_IF_NOT_NULL(sPokedexScreenData);
        FreeAllWindowBuffers();
        FREE_IF_NOT_NULL(GetBgTilemapBuffer(0));
        FREE_IF_NOT_NULL(GetBgTilemapBuffer(1));
        FREE_IF_NOT_NULL(GetBgTilemapBuffer(2));
        FREE_IF_NOT_NULL(GetBgTilemapBuffer(3));
        BGMVolumeMax_EnableHelpSystemReduction();
        break;
    }
    return TRUE;
}

void CB2_ClosePokedex(void)
{
    if (DoClosePokedex())
    {
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON);
        SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
    }
}

static void Task_PokedexScreen(u8 taskId)
{
    int i;
    switch (sPokedexScreenData->state)
    {
    case 0:
        sPokedexScreenData->unlockedCategories = 0;
        for (i = 0; i < 9; i++)
            sPokedexScreenData->unlockedCategories |= (DexScreen_IsCategoryUnlocked(i) << i);
        sPokedexScreenData->state = 2;
        break;
    case 1:
        RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
        DexScreen_RemoveWindow(&sPokedexScreenData->modeSelectWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->selectionIconWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->dexCountsWindowId);
        SetMainCallback2(CB2_ClosePokedex);
        DestroyTask(taskId);
        break;
    case 2:
        DexScreen_InitGfxForTopMenu();
        sPokedexScreenData->state = 3;
        break;
    case 3:
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);
        sPokedexScreenData->state = 4;
        break;
    case 4:
        ShowBg(3);
        ShowBg(2);
        ShowBg(1);
        ShowBg(0);
        if (gPaletteFade.bufferTransferDisabled)
        {
            gPaletteFade.bufferTransferDisabled = FALSE;
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 0, RGB_WHITEALPHA);
        }
        else
            BeginNormalPaletteFade(0xFFFF7FFF, 0, 16, 0, RGB_WHITEALPHA);
        sPokedexScreenData->state = 5;
        break;
    case 5:
        ListMenuGetScrollAndRow(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPosBak, NULL);
        if (IsNationalPokedexEnabled())
            sPokedexScreenData->scrollArrowsTaskId = AddScrollIndicatorArrowPair(&sScrollArrowsTemplate_NatDex, &sPokedexScreenData->modeSelectCursorPosBak);
        else
            sPokedexScreenData->scrollArrowsTaskId = AddScrollIndicatorArrowPair(&sScrollArrowsTemplate_KantoDex, &sPokedexScreenData->modeSelectCursorPosBak);
        sPokedexScreenData->state = 6;
        break;
    case 6:
        sPokedexScreenData->modeSelectInput = ListMenu_ProcessInput(sPokedexScreenData->modeSelectListMenuId);
        ListMenuGetScrollAndRow(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPosBak, NULL);
        if (JOY_NEW(A_BUTTON))
        {
            switch (sPokedexScreenData->modeSelectInput)
            {
            case LIST_CANCEL:
                sPokedexScreenData->state = 1;
                break;
            case DEX_CATEGORY_GRASSLAND:
            case DEX_CATEGORY_FOREST:
            case DEX_CATEGORY_WATERS_EDGE:
            case DEX_CATEGORY_SEA:
            case DEX_CATEGORY_CAVE:
            case DEX_CATEGORY_MOUNTAIN:
            case DEX_CATEGORY_ROUGH_TERRAIN:
            case DEX_CATEGORY_URBAN:
            case DEX_CATEGORY_RARE:
                if (DexScreen_IsCategoryUnlocked(sPokedexScreenData->modeSelectInput))
                {
                    RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
                    sPokedexScreenData->category = sPokedexScreenData->modeSelectInput;
                    BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
                    sPokedexScreenData->state = 7;
                }
                break;
            case DEX_MODE(NUMERICAL_KANTO):
            case DEX_MODE(NUMERICAL_NATIONAL):
                RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
                sPokedexScreenData->dexOrderId = sPokedexScreenData->modeSelectInput - 9;
                BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
                sPokedexScreenData->state = 9;
                break;
            case DEX_MODE(ATOZ):
            case DEX_MODE(TYPE):
            case DEX_MODE(LIGHTEST):
            case DEX_MODE(SMALLEST):
                RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
                sPokedexScreenData->dexOrderId = sPokedexScreenData->modeSelectInput - 9;
                sPokedexScreenData->characteristicOrderMenuItemsAbove = sPokedexScreenData->characteristicOrderMenuCursorPos = 0;
                BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
                sPokedexScreenData->state = 8;
                break;
            }
            break;
        }
        if (JOY_NEW(B_BUTTON))
        {
            sPokedexScreenData->state = 1;
        }
        break;
    case 7:
        DestroyListMenuTask(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPos, &sPokedexScreenData->modeSelectItemsAbove);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 20);
        CopyBgTilemapBufferToVram(1);
        DexScreen_RemoveWindow(&sPokedexScreenData->modeSelectWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->selectionIconWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->dexCountsWindowId);
        sPokedexScreenData->pageNum = 0;
        sPokedexScreenData->categoryCursorPosInPage = 0;
        sPokedexScreenData->parentOfCategoryMenu = 0;
        gTasks[taskId].func = Task_DexScreen_CategorySubmenu;
        sPokedexScreenData->state = 0;
        break;
    case 8:
        DestroyListMenuTask(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPos, &sPokedexScreenData->modeSelectItemsAbove);
        HideBg(1);
        DexScreen_RemoveWindow(&sPokedexScreenData->modeSelectWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->selectionIconWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->dexCountsWindowId);
        gTasks[taskId].func = Task_DexScreen_CharacteristicOrder;
        sPokedexScreenData->state = 0;
        break;
    case 9:
        DestroyListMenuTask(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPos, &sPokedexScreenData->modeSelectItemsAbove);
        HideBg(1);
        DexScreen_RemoveWindow(&sPokedexScreenData->modeSelectWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->selectionIconWindowId);
        DexScreen_RemoveWindow(&sPokedexScreenData->dexCountsWindowId);
        gTasks[taskId].func = Task_DexScreen_NumericalOrder;
        sPokedexScreenData->state = 0;
        break;
    }
}

static void DexScreen_InitGfxForTopMenu(void)
{
    struct ListMenuTemplate listMenuTemplate;
    FillBgTilemapBufferRect(3, 0x00E, 0, 0, 30, 20, 0x00);
    FillBgTilemapBufferRect(2, 0x000, 0, 0, 30, 20, 0x11);
    FillBgTilemapBufferRect(1, 0x000, 0, 0, 30, 20, 0x11);
    sPokedexScreenData->modeSelectWindowId = AddWindow(&sWindowTemplate_ModeSelect);
    sPokedexScreenData->selectionIconWindowId = AddWindow(&sWindowTemplate_SelectionIcon);
    sPokedexScreenData->dexCountsWindowId = AddWindow(&sWindowTemplate_DexCounts);
    if (IsNationalPokedexEnabled())
    {
        listMenuTemplate = sListMenuTemplate_NatDexModeSelect;
        listMenuTemplate.windowId = sPokedexScreenData->modeSelectWindowId;
        sPokedexScreenData->modeSelectListMenuId = ListMenuInit(&listMenuTemplate, sPokedexScreenData->modeSelectCursorPos, sPokedexScreenData->modeSelectItemsAbove);
        FillWindowPixelBuffer(sPokedexScreenData->dexCountsWindowId, PIXEL_FILL(0));
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 0, gText_Seen, 0, 2, 0);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 0, gText_Kanto, 8, 13, 0);
        DexScreen_PrintNum3RightAlign(sPokedexScreenData->dexCountsWindowId, 0, sPokedexScreenData->numSeenKanto, 52, 13, 2);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 0, gText_National, 8, 24, 0);
        DexScreen_PrintNum3RightAlign(sPokedexScreenData->dexCountsWindowId, 0, sPokedexScreenData->numSeenNational, 52, 24, 2);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 0, gText_Owned, 0, 37, 0);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 0, gText_Kanto, 8, 48, 0);
        DexScreen_PrintNum3RightAlign(sPokedexScreenData->dexCountsWindowId, 0, sPokedexScreenData->numOwnedKanto, 52, 48, 2);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 0, gText_National, 8, 59, 0);
        DexScreen_PrintNum3RightAlign(sPokedexScreenData->dexCountsWindowId, 0, sPokedexScreenData->numOwnedNational, 52, 59, 2);
    }
    else
    {
        listMenuTemplate = sListMenuTemplate_KantoDexModeSelect;
        listMenuTemplate.windowId = sPokedexScreenData->modeSelectWindowId;
        sPokedexScreenData->modeSelectListMenuId = ListMenuInit(&listMenuTemplate, sPokedexScreenData->modeSelectCursorPos, sPokedexScreenData->modeSelectItemsAbove);
        FillWindowPixelBuffer(sPokedexScreenData->dexCountsWindowId, PIXEL_FILL(0));
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 1, gText_Seen, 0, 9, 0);
        DexScreen_PrintNum3RightAlign(sPokedexScreenData->dexCountsWindowId, 1, sPokedexScreenData->numSeenKanto, 32, 21, 2);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->dexCountsWindowId, 1, gText_Owned, 0, 37, 0);
        DexScreen_PrintNum3RightAlign(sPokedexScreenData->dexCountsWindowId, 1, sPokedexScreenData->numOwnedKanto, 32, 49, 2);
    }
    FillWindowPixelBuffer(0, PIXEL_FILL(15));
    DexScreen_PrintStringWithAlignment(gText_PokedexTableOfContents, TEXT_CENTER);
    FillWindowPixelBuffer(1, PIXEL_FILL(15));
    DexScreen_PrintControlInfo(gText_PickOK);
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_GFX);
    PutWindowTilemap(1);
    CopyWindowToVram(1, COPYWIN_GFX);
    PutWindowTilemap(sPokedexScreenData->dexCountsWindowId);
    CopyWindowToVram(sPokedexScreenData->dexCountsWindowId, COPYWIN_GFX);
}

static void MoveCursorFunc_DexModeSelect(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    if (!onInit)
        PlaySE(SE_SELECT);
    if (itemIndex == LIST_CANCEL)
    {
        CopyToWindowPixelBuffer(sPokedexScreenData->selectionIconWindowId, sTopMenuSelectionIconTiles_Cancel, 0x000, 0x000);
        LoadPalette(sTopMenuSelectionIconPals_Cancel, 0x10, 0x20);
    }
    else
    {
        CopyToWindowPixelBuffer(sPokedexScreenData->selectionIconWindowId, sTopMenuSelectionIconGfxPtrs[itemIndex].map, 0x000, 0x000);
        LoadPalette(sTopMenuSelectionIconGfxPtrs[itemIndex].pal, 0x10, 0x20);
    }
    PutWindowTilemap(sPokedexScreenData->selectionIconWindowId);
    CopyWindowToVram(sPokedexScreenData->selectionIconWindowId, COPYWIN_GFX);
}

static void ItemPrintFunc_DexModeSelect(u8 windowId, s32 itemId, u8 y)
{
    u32 itemId_ = itemId;
    if (itemId_ > 8 || sPokedexScreenData->unlockedCategories & (1 << itemId_))
        ListMenuOverrideSetColors(1, 0, 3);
    else
        ListMenuOverrideSetColors(10, 0, 11);
}

static void Task_DexScreen_NumericalOrder(u8 taskId)
{
    switch (sPokedexScreenData->state)
    {
    case 0:
        ListMenuLoadStdPalAt(0x10, 0);
        ListMenuLoadStdPalAt(0x20, 1);
        sPokedexScreenData->orderedDexCount = DexScreen_CountMonsInOrderedList(sPokedexScreenData->dexOrderId);
        sPokedexScreenData->state = 2;
        break;
    case 1:
        DexScreen_DestroyDexOrderListMenu(sPokedexScreenData->dexOrderId);
        HideBg(1);
        DexScreen_RemoveWindow(&sPokedexScreenData->numericalOrderWindowId);
        gTasks[taskId].func = Task_PokedexScreen;
        sPokedexScreenData->state = 0;
        break;
    case 2:
        DexScreen_InitGfxForNumericalOrderList();
        sPokedexScreenData->state = 3;
        break;
    case 3:
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(1);
        sPokedexScreenData->state = 4;
        break;
    case 4:
        ShowBg(1);
        BeginNormalPaletteFade(0xFFFF7FFF, 0, 16, 0, RGB_WHITEALPHA);
        sPokedexScreenData->state = 5;
        break;
    case 5:
        ListMenuGetScrollAndRow(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPosBak, NULL);
        sPokedexScreenData->scrollArrowsTaskId = DexScreen_CreateDexOrderScrollArrows();
        sPokedexScreenData->state = 6;
        break;
    case 6:
        sPokedexScreenData->characteristicMenuInput = ListMenu_ProcessInput(sPokedexScreenData->orderedListMenuTaskId);
        ListMenuGetScrollAndRow(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPosBak, NULL);
        if (JOY_NEW(A_BUTTON))
        {
            if ((sPokedexScreenData->characteristicMenuInput >> 16) & 1)
            {
                sPokedexScreenData->field_5A = sPokedexScreenData->characteristicMenuInput;
                RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
                BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
                sPokedexScreenData->state = 7;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
            BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
            sPokedexScreenData->state = 1;
        }
        break;
    case 7:
        DexScreen_DestroyDexOrderListMenu(sPokedexScreenData->dexOrderId);
        FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 0, 32, 20);
        CopyBgTilemapBufferToVram(1);
        DexScreen_RemoveWindow(&sPokedexScreenData->numericalOrderWindowId);
        gTasks[taskId].func = Task_DexScreen_ShowMonPage;
        sPokedexScreenData->state = 0;
        break;
    }
}

static void DexScreen_InitGfxForNumericalOrderList(void)
{
    struct ListMenuTemplate template;
    FillBgTilemapBufferRect(3, 0x00E, 0, 0, 30, 20, 0x00);
    FillBgTilemapBufferRect(1, 0x000, 0, 0, 32, 32, 0x11);
    sPokedexScreenData->numericalOrderWindowId = AddWindow(&sWindowTemplate_OrderedListMenu);
    template = sListMenuTemplate_OrderedListMenu;
    template.items = sPokedexScreenData->listItems;
    template.windowId = sPokedexScreenData->numericalOrderWindowId;
    template.totalItems = sPokedexScreenData->orderedDexCount;
    DexScreen_InitListMenuForOrderedList(&template, sPokedexScreenData->dexOrderId);
    FillWindowPixelBuffer(0, PIXEL_FILL(15));
    DexScreen_PrintStringWithAlignment(gText_PokemonListNoColor, TEXT_CENTER);
    FillWindowPixelBuffer(1, PIXEL_FILL(15));
    DexScreen_PrintControlInfo(gText_PickOKExit);
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyWindowToVram(1, COPYWIN_GFX);
}

static void Task_DexScreen_CharacteristicOrder(u8 taskId)
{
    switch (sPokedexScreenData->state)
    {
    case 0:
        ListMenuLoadStdPalAt(0x10, 0);
        ListMenuLoadStdPalAt(0x20, 1);
        sPokedexScreenData->orderedDexCount = DexScreen_CountMonsInOrderedList(sPokedexScreenData->dexOrderId);
        sPokedexScreenData->state = 2;
        break;
    case 1:
        DexScreen_DestroyDexOrderListMenu(sPokedexScreenData->dexOrderId);
        HideBg(1);
        DexScreen_RemoveWindow(&sPokedexScreenData->numericalOrderWindowId);
        gTasks[taskId].func = Task_PokedexScreen;
        sPokedexScreenData->state = 0;
        break;
    case 2:
        DexScreen_CreateCharacteristicListMenu();
        sPokedexScreenData->state = 3;
        break;
    case 3:
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(1);
        sPokedexScreenData->state = 4;
        break;
    case 4:
        ShowBg(1);
        BeginNormalPaletteFade(0xFFFF7FFF, 0, 16, 0, RGB_WHITEALPHA);
        sPokedexScreenData->state = 5;
        break;
    case 5:
        ListMenuGetScrollAndRow(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPosBak, NULL);
        sPokedexScreenData->scrollArrowsTaskId = DexScreen_CreateDexOrderScrollArrows();
        sPokedexScreenData->state = 6;
        break;
    case 6:
        sPokedexScreenData->characteristicMenuInput = ListMenu_ProcessInput(sPokedexScreenData->orderedListMenuTaskId);
        ListMenuGetScrollAndRow(sPokedexScreenData->modeSelectListMenuId, &sPokedexScreenData->modeSelectCursorPosBak, NULL);
        if (JOY_NEW(A_BUTTON))
        {
            if (((sPokedexScreenData->characteristicMenuInput >> 16) & 1) && !DexScreen_LookUpCategoryBySpecies(sPokedexScreenData->characteristicMenuInput))
            {
                RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
                BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
                sPokedexScreenData->state = 7;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
            BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
            sPokedexScreenData->state = 1;
        }
        break;
    case 7:
        DexScreen_DestroyDexOrderListMenu(sPokedexScreenData->dexOrderId);
        FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 0, 32, 20);
        CopyBgTilemapBufferToVram(1);
        DexScreen_RemoveWindow(&sPokedexScreenData->numericalOrderWindowId);
        sPokedexScreenData->parentOfCategoryMenu = 1;
        gTasks[taskId].func = Task_DexScreen_CategorySubmenu;
        sPokedexScreenData->state = 0;
        break;
    }
}

static void DexScreen_CreateCharacteristicListMenu(void)
{
    struct ListMenuTemplate template;
    FillBgTilemapBufferRect(3, 0x00E, 0, 0, 30, 20, 0x00);
    FillBgTilemapBufferRect(1, 0x000, 0, 0, 32, 32, 0x11);
    sPokedexScreenData->numericalOrderWindowId = AddWindow(&sWindowTemplate_OrderedListMenu);
    template = sListMenuTemplate_OrderedListMenu;
    template.items = sPokedexScreenData->listItems;
    template.windowId = sPokedexScreenData->numericalOrderWindowId;
    template.totalItems = sPokedexScreenData->orderedDexCount;
    DexScreen_InitListMenuForOrderedList(&template, sPokedexScreenData->dexOrderId);
    FillWindowPixelBuffer(0, PIXEL_FILL(15));
    DexScreen_PrintStringWithAlignment(gText_SearchNoColor, TEXT_CENTER);
    FillWindowPixelBuffer(1, PIXEL_FILL(15));
    DexScreen_PrintControlInfo(gText_PickOKExit);
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyWindowToVram(1, COPYWIN_GFX);
}

static u16 DexScreen_CountMonsInOrderedList(u8 orderIdx)
{
    s32 max_n = IsNationalPokedexEnabled() ? NATIONAL_DEX_COUNT : KANTO_DEX_COUNT;
    u16 ndex_num;
    u16 ret = NATIONAL_DEX_NONE;
    s32 i;
    bool8 caught;
    bool8 seen;

    switch (orderIdx)
    {
    default:
    case DEX_ORDER_NUMERICAL_KANTO:
        for (i = 0; i < KANTO_DEX_COUNT; i++)
        {
            ndex_num = i + 1;
            seen = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_SEEN, FALSE);
            caught = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_CAUGHT, FALSE);
            if (seen)
            {
                sPokedexScreenData->listItems[i].label = gSpeciesNames[NationalPokedexNumToSpecies(ndex_num)];
                ret = ndex_num;
            }
            else
            {
                sPokedexScreenData->listItems[i].label = gText_5Dashes;
            }
            sPokedexScreenData->listItems[i].index = (caught << 17) + (seen << 16) + NationalPokedexNumToSpecies(ndex_num);
        }
        break;
    case DEX_ORDER_ATOZ:
        for (i = 0; i < NUM_SPECIES - 1; i++)
        {
            ndex_num = gPokedexOrder_Alphabetical[i];
            if (ndex_num <= max_n)
            {
                seen = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_SEEN, FALSE);
                caught = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_CAUGHT, FALSE);
                if (seen)
                {
                    sPokedexScreenData->listItems[ret].label = gSpeciesNames[NationalPokedexNumToSpecies(ndex_num)];
                    sPokedexScreenData->listItems[ret].index = (caught << 17) + (seen << 16) + NationalPokedexNumToSpecies(ndex_num);
                    ret++;
                }
            }
        }
        break;
    case DEX_ORDER_TYPE:
        for (i = 0; i < NUM_SPECIES - 1; i++)
        {
            ndex_num = SpeciesToNationalPokedexNum(gPokedexOrder_Type[i]);
            if (ndex_num <= max_n)
            {
                seen = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_SEEN, FALSE);
                caught = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_CAUGHT, FALSE);
                if (caught)
                {
                    sPokedexScreenData->listItems[ret].label = gSpeciesNames[NationalPokedexNumToSpecies(ndex_num)];
                    sPokedexScreenData->listItems[ret].index = (caught << 17) + (seen << 16) + NationalPokedexNumToSpecies(ndex_num);
                    ret++;
                }
            }
        }
        break;
    case DEX_ORDER_LIGHTEST:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            ndex_num = gPokedexOrder_Weight[i];
            if (ndex_num <= max_n)
            {
                seen = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_SEEN, FALSE);
                caught = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_CAUGHT, FALSE);
                if (caught)
                {
                    sPokedexScreenData->listItems[ret].label = gSpeciesNames[NationalPokedexNumToSpecies(ndex_num)];
                    sPokedexScreenData->listItems[ret].index = (caught << 17) + (seen << 16) + NationalPokedexNumToSpecies(ndex_num);
                    ret++;
                }
            }
        }
        break;
    case DEX_ORDER_SMALLEST:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            ndex_num = gPokedexOrder_Height[i];
            if (ndex_num <= max_n)
            {
                seen = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_SEEN, FALSE);
                caught = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_CAUGHT, FALSE);
                if (caught)
                {
                    sPokedexScreenData->listItems[ret].label = gSpeciesNames[NationalPokedexNumToSpecies(ndex_num)];
                    sPokedexScreenData->listItems[ret].index = (caught << 17) + (seen << 16) + NationalPokedexNumToSpecies(ndex_num);
                    ret++;
                }
            }
        }
        break;
    case DEX_ORDER_NUMERICAL_NATIONAL:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            ndex_num = i + 1;
            seen = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_SEEN, FALSE);
            caught = DexScreen_GetSetPokedexFlag(ndex_num, FLAG_GET_CAUGHT, FALSE);
            if (seen)
            {
                sPokedexScreenData->listItems[i].label = gSpeciesNames[NationalPokedexNumToSpecies(ndex_num)];
                ret = ndex_num;
            }
            else
            {
                sPokedexScreenData->listItems[i].label = gText_5Dashes;
            }
            sPokedexScreenData->listItems[i].index = (caught << 17) + (seen << 16) + NationalPokedexNumToSpecies(ndex_num);
        }
        break;
    }
    return ret;
}

static void DexScreen_InitListMenuForOrderedList(const struct ListMenuTemplate * template, u8 order)
{
    switch (order)
    {
    default:
    case DEX_ORDER_NUMERICAL_KANTO:
        sPokedexScreenData->orderedListMenuTaskId = ListMenuInitInRect(template, &sListMenuRect_OrderedList, sPokedexScreenData->kantoOrderMenuCursorPos, sPokedexScreenData->kantoOrderMenuItemsAbove);
        break;
    case DEX_ORDER_ATOZ:
    case DEX_ORDER_TYPE:
    case DEX_ORDER_LIGHTEST:
    case DEX_ORDER_SMALLEST:
        sPokedexScreenData->orderedListMenuTaskId = ListMenuInitInRect(template, &sListMenuRect_OrderedList, sPokedexScreenData->characteristicOrderMenuCursorPos, sPokedexScreenData->characteristicOrderMenuItemsAbove);
        break;
    case DEX_ORDER_NUMERICAL_NATIONAL:
        sPokedexScreenData->orderedListMenuTaskId = ListMenuInitInRect(template, &sListMenuRect_OrderedList, sPokedexScreenData->nationalOrderMenuCursorPos, sPokedexScreenData->nationalOrderMenuItemsAbove);
        break;
    }
}

static void DexScreen_DestroyDexOrderListMenu(u8 order)
{
    switch (order)
    {
    default:
    case DEX_ORDER_NUMERICAL_KANTO:
        DestroyListMenuTask(sPokedexScreenData->orderedListMenuTaskId, &sPokedexScreenData->kantoOrderMenuCursorPos, &sPokedexScreenData->kantoOrderMenuItemsAbove);
        break;
    case DEX_ORDER_ATOZ:
    case DEX_ORDER_TYPE:
    case DEX_ORDER_LIGHTEST:
    case DEX_ORDER_SMALLEST:
        DestroyListMenuTask(sPokedexScreenData->orderedListMenuTaskId, &sPokedexScreenData->characteristicOrderMenuCursorPos, &sPokedexScreenData->characteristicOrderMenuItemsAbove);
        break;
    case DEX_ORDER_NUMERICAL_NATIONAL:
        DestroyListMenuTask(sPokedexScreenData->orderedListMenuTaskId, &sPokedexScreenData->nationalOrderMenuCursorPos, &sPokedexScreenData->nationalOrderMenuItemsAbove);
        break;
    }
}

static u8 DexScreen_CreateDexOrderScrollArrows(void)
{
    struct ScrollArrowsTemplate template = sDexOrderScrollArrowsTemplate;
    if (sPokedexScreenData->orderedDexCount > sListMenuTemplate_OrderedListMenu.maxShowed)
        template.fullyDownThreshold = sPokedexScreenData->orderedDexCount - sListMenuTemplate_OrderedListMenu.maxShowed;
    else
        template.fullyDownThreshold = 0;
    return AddScrollIndicatorArrowPair(&template, &sPokedexScreenData->modeSelectCursorPosBak);
}

struct PokedexListItem
{
    u16 species;
    bool8 seen:1;
    bool8 caught:1;
};

static void ItemPrintFunc_OrderedListMenu(u8 windowId, s32 itemId, u8 y)
{
    u32 itemId_ = itemId;
    u16 species = itemId_;
    bool8 seen = (itemId_ >> 16) & 1;  // not used but required to match
    bool8 caught = (itemId_ >> 17) & 1;
    u8 type1;
    DexScreen_PrintMonDexNo(sPokedexScreenData->numericalOrderWindowId, 0, species, 12, y);
    if (caught)
    {
        BlitMoveInfoIcon(sPokedexScreenData->numericalOrderWindowId, 0, 0x28, y);
        type1 = gBaseStats[species].type1;
        BlitMoveInfoIcon(sPokedexScreenData->numericalOrderWindowId, type1 + 1, 0x78, y);
        if (type1 != gBaseStats[species].type2)
            BlitMoveInfoIcon(sPokedexScreenData->numericalOrderWindowId, gBaseStats[species].type2 + 1, 0x98, y);
    }
}

static void Task_DexScreen_CategorySubmenu(u8 taskId)
{
    int r4;
    u8 *ptr;
    switch (sPokedexScreenData->state)
    {
    case 0:
        HideBg(3);
        HideBg(2);
        HideBg(1);
        DexScreen_GetPageLimitsForCategory(sPokedexScreenData->category);
        if (sPokedexScreenData->pageNum < sPokedexScreenData->firstPageInCategory)
            sPokedexScreenData->pageNum = sPokedexScreenData->firstPageInCategory;
        sPokedexScreenData->state = 2;
        break;
    case 1:
        DexScreen_DestroyCategoryPageMonIconAndInfoWindows();
        HideBg(2);
        HideBg(1);
        switch (sPokedexScreenData->parentOfCategoryMenu)
        {
        case 0:
        default:
            gTasks[taskId].func = Task_PokedexScreen;
            break;
        case 1:
            gTasks[taskId].func = Task_DexScreen_CharacteristicOrder;
            break;
        }
        sPokedexScreenData->state = 0;
        break;
    case 2:
        DexScreen_CreateCategoryListGfx(FALSE);
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        DexScreen_CreateCategoryPageSelectionCursor(0xFF);
        sPokedexScreenData->state = 3;
        break;
    case 3:
        BeginNormalPaletteFade(0xFFFF7FFF, 0, 16, 0, RGB_WHITEALPHA);
        ShowBg(3);
        ShowBg(2);
        ShowBg(1);
        sPokedexScreenData->state = 4;
        break;
    case 4:
        sPokedexScreenData->scrollArrowsTaskId = sub_8104234();
        sPokedexScreenData->categoryPageCursorTaskId = ListMenuAddCursorObjectInternal(&gUnknown_84524C4, 0);
        sPokedexScreenData->state = 5;
        break;
    case 5:
        DexScreen_CreateCategoryPageSelectionCursor(sPokedexScreenData->categoryCursorPosInPage);
        DexScreen_UpdateCategoryPageCursorObject(sPokedexScreenData->categoryPageCursorTaskId, sPokedexScreenData->categoryCursorPosInPage, sPokedexScreenData->numMonsOnPage);
        sPokedexScreenData->modeSelectCursorPosBak = sPokedexScreenData->pageNum;
        r4 = 0;
        if (JOY_NEW(A_BUTTON) && DexScreen_GetSetPokedexFlag(sPokedexScreenData->pageSpecies[sPokedexScreenData->categoryCursorPosInPage], FLAG_GET_SEEN, TRUE))
        {
            RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
            ListMenuRemoveCursorObject(sPokedexScreenData->categoryPageCursorTaskId, 0);
            sPokedexScreenData->state = 12;
            break;
        }
        if (!JOY_HELD(R_BUTTON) && JOY_REPT(DPAD_LEFT))
        {
            if (sPokedexScreenData->categoryCursorPosInPage != 0)
            {
                sPokedexScreenData->categoryCursorPosInPage--;
                PlaySE(SE_SELECT);
                break;
            }
            else
                r4 = 1;
        }
        if (!JOY_HELD(R_BUTTON) && JOY_REPT(DPAD_RIGHT))
        {
            if (sPokedexScreenData->categoryCursorPosInPage < sPokedexScreenData->numMonsOnPage - 1)
            {
                sPokedexScreenData->categoryCursorPosInPage++;
                PlaySE(SE_SELECT);
                break;
            }
            else
                r4 = 2;
        }
        if (r4 == 0)
            r4 = sub_8104284();
        switch (r4)
        {
        case 0:
            break;
        case 1:
            while (sPokedexScreenData->pageNum > sPokedexScreenData->firstPageInCategory)
            {
                sPokedexScreenData->pageNum--;
                if (DexScreen_IsPageUnlocked(sPokedexScreenData->category, sPokedexScreenData->pageNum))
                {
                    sPokedexScreenData->state = 8;
                    break;
                }
            }
            if (sPokedexScreenData->state != 8)
                sPokedexScreenData->state = 6;
            break;
        case 2:
            while (sPokedexScreenData->pageNum < sPokedexScreenData->lastPageInCategory - 1)
            {
                sPokedexScreenData->pageNum++;
                if (DexScreen_IsPageUnlocked(sPokedexScreenData->category, sPokedexScreenData->pageNum))
                {
                    sPokedexScreenData->state = 10;
                    break;
                }
            }
            if (sPokedexScreenData->state != 10)
                sPokedexScreenData->state = 6;
            break;
        }
        if (JOY_NEW(B_BUTTON))
        {
            sPokedexScreenData->state = 6;
        }
        break;
    case 6:
    case 7:
        RemoveScrollIndicatorArrowPair(sPokedexScreenData->scrollArrowsTaskId);
        ListMenuRemoveCursorObject(sPokedexScreenData->categoryPageCursorTaskId, 0);
        BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
        sPokedexScreenData->state = 1;
        break;
    case 8:
    case 10:
        DexScreen_DestroyCategoryPageMonIconAndInfoWindows();
        DexScreen_CreateCategoryPageSelectionCursor(0xFF);
        ListMenuUpdateCursorObject(sPokedexScreenData->categoryPageCursorTaskId, 0, 0xA0, 0);
        sPokedexScreenData->categoryPageSelectionCursorTimer = 0;
        sPokedexScreenData->data[0] = 0;
        sPokedexScreenData->state++;
        break;
    case 9:
        if (sub_81052D0(0))
        {
            sPokedexScreenData->categoryCursorPosInPage = sPokedexScreenData->numMonsOnPage - 1;
            sPokedexScreenData->state = 5;
        }
        break;
    case 11:
        if (sub_81052D0(1))
        {
            sPokedexScreenData->categoryCursorPosInPage = 0;
            sPokedexScreenData->state = 5;
        }
        break;
    case 12:
        sPokedexScreenData->field_5A = sPokedexScreenData->pageSpecies[sPokedexScreenData->categoryCursorPosInPage];
        PlaySE(SE_SELECT);
        sPokedexScreenData->state = 14;
        break;
    case 13:
        sub_8106014();
        sPokedexScreenData->state = 4;
        break;
    case 14:
        DexScreen_DrawMonDexPage(FALSE);
        sPokedexScreenData->state = 15;
        break;
    case 15:
        sPokedexScreenData->data[0] = 0;
        sPokedexScreenData->data[1] = 0;
        sPokedexScreenData->state++;
        // fallthrough
    case 16:
        if (sPokedexScreenData->data[1] < 6)
        {
            if (sPokedexScreenData->data[0])
            {
                sub_8105594(0, sPokedexScreenData->data[1]);
                CopyBgTilemapBufferToVram(0);
                sPokedexScreenData->data[0] = 4;
                sPokedexScreenData->data[1]++;
            }
            else
            {
                sPokedexScreenData->data[0]--;
            }
        }
        else
        {
            FillBgTilemapBufferRect_Palette0(0, 0x000, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(3);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(0);
            PlayCry2(sPokedexScreenData->field_5A, 0, 125, 10);
            sPokedexScreenData->data[0] = 0;
            sPokedexScreenData->state = 17;
        }
        break;
    case 17:
        if (JOY_NEW(A_BUTTON))
        {
            sub_8106014();
            FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(1);
            sPokedexScreenData->state = 21;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            sPokedexScreenData->state = 18;
        }
        else
        {
            sub_8106B34();
        }
        break;
    case 18:
        DexScreen_CreateCategoryListGfx(FALSE);
        sub_8105594(0, 6);
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);
        sPokedexScreenData->state = 19;
        break;
    case 19:
        sPokedexScreenData->data[0] = 0;
        sPokedexScreenData->data[1] = 6;
        sPokedexScreenData->state++;
        // fallthrough
    case 20:
        if (sPokedexScreenData->data[1])
        {
            if (sPokedexScreenData->data[0])
            {
                sPokedexScreenData->data[1]--;
                FillBgTilemapBufferRect_Palette0(0, 0x000, 0, 2, 30, 16);
                sub_8105594(0, sPokedexScreenData->data[1]);
                CopyBgTilemapBufferToVram(0);
                sPokedexScreenData->data[0] = 1;
            }
            else
                sPokedexScreenData->data[0]--;
        }
        else
        {
            FillBgTilemapBufferRect_Palette0(0, 0x000, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(0);
            sPokedexScreenData->state = 13;
        }
        break;
    case 21:
        sub_810603C();
        sPokedexScreenData->state = 22;
        break;
    case 22:
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);
        sPokedexScreenData->state = 23;
        break;
    case 23:
        if (JOY_NEW(A_BUTTON))
        {
            FillBgTilemapBufferRect_Palette0(2, 0x000, 0, 2, 30, 16);
            FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 2, 30, 16);
            FillBgTilemapBufferRect_Palette0(0, 0x000, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(0);
            sPokedexScreenData->state = 26;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            FillBgTilemapBufferRect_Palette0(2, 0x000, 0, 2, 30, 16);
            FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 2, 30, 16);
            FillBgTilemapBufferRect_Palette0(0, 0x000, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(0);
            sPokedexScreenData->state = 24;
        }
        else
        {
            sub_8106B34();
        }
        break;
    case 24:
        sub_81067C0();
        sPokedexScreenData->state = 25;
        break;
    case 25:
        DexScreen_DrawMonDexPage(FALSE);
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);
        sPokedexScreenData->state = 17;
        break;
    case 26:
        sub_81067C0();
        sPokedexScreenData->state = 18;
        break;
    }
}

static u8 sub_8104234(void)
{
    struct ScrollArrowsTemplate template = sUnknown_84524B4;
    template.fullyUpThreshold = sPokedexScreenData->firstPageInCategory;
    template.fullyDownThreshold = sPokedexScreenData->lastPageInCategory - 1;
    sPokedexScreenData->modeSelectCursorPosBak = sPokedexScreenData->pageNum;
    return AddScrollIndicatorArrowPair(&template, &sPokedexScreenData->modeSelectCursorPosBak);
}

static int sub_8104284(void)
{
    switch (gSaveBlock2Ptr->optionsButtonMode)
    {
    case OPTIONS_BUTTON_MODE_L_EQUALS_A:
        // Using the JOY_HELD and JOY_NEW macros here does not match!
        if ((gMain.heldKeys & R_BUTTON) && (gMain.newKeys & DPAD_LEFT))
            return 1;
        else if ((gMain.heldKeys & R_BUTTON) && (gMain.newKeys & DPAD_RIGHT))
            return 2;
        else
            return 0;
    case OPTIONS_BUTTON_MODE_LR:
        if (gMain.newKeys & L_BUTTON)
            return 1;
        else if (gMain.newKeys & R_BUTTON)
            return 2;
        else
            return 0;
    case OPTIONS_BUTTON_MODE_HELP:
    default:
        return 0;
    }
}

static void Task_DexScreen_ShowMonPage(u8 taskId)
{
    switch (sPokedexScreenData->state)
    {
    case 0:
        HideBg(3);
        HideBg(2);
        HideBg(1);
        sPokedexScreenData->state = 2;
        break;
    case 1:
        HideBg(2);
        HideBg(1);
        gTasks[taskId].func = Task_DexScreen_NumericalOrder;
        sPokedexScreenData->state = 0;
        break;
    case 2:
        sPokedexScreenData->numMonsOnPage = 1;
        DexScreen_DrawMonDexPage(FALSE);
        sPokedexScreenData->state = 3;
        break;
    case 3:
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);
        PlayCry2(sPokedexScreenData->field_5A, 0, 125, 10);
        sPokedexScreenData->state = 4;
        break;
    case 4:
        BeginNormalPaletteFade(0xFFFF7FFF, 0, 16, 0, RGB_WHITEALPHA);
        ShowBg(3);
        ShowBg(2);
        ShowBg(1);
        sPokedexScreenData->state = 5;
        break;
    case 5:
        if (JOY_NEW(A_BUTTON))
        {
            sub_8106014();
            FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(1);
            sPokedexScreenData->state = 7;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            sub_8106014();
            BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
            sPokedexScreenData->state = 1;
        }
        else if (JOY_NEW(DPAD_UP) && sub_8104664(1))
        {
            sub_8106014();
            BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
            sPokedexScreenData->state = 6;
        }
        else if (JOY_NEW(DPAD_DOWN) && sub_8104664(0))
        {
            sub_8106014();
            BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
            sPokedexScreenData->state = 6;
        }
        else
        {
            sub_8106B34();
        }
        break;
    case 6:
        HideBg(2);
        HideBg(1);
        sPokedexScreenData->field_5A = sPokedexScreenData->characteristicMenuInput;
        sPokedexScreenData->state = 2;
        break;
    case 7:
        sub_810603C();
        sPokedexScreenData->state = 8;
        break;
    case 8:
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);
        sPokedexScreenData->state = 9;
        break;
    case 9:
        if (JOY_NEW(A_BUTTON))
        {
            BeginNormalPaletteFade(0xFFFF7FFF, 0, 0, 16, RGB_WHITEALPHA);
            sPokedexScreenData->state = 12;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            FillBgTilemapBufferRect_Palette0(2, 0x000, 0, 2, 30, 16);
            FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 2, 30, 16);
            FillBgTilemapBufferRect_Palette0(0, 0x000, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(0);
            sPokedexScreenData->state = 10;
        }
        else
        {
            sub_8106B34();
        }
        break;
    case 10:
        sub_81067C0();
        sPokedexScreenData->state = 11;
        break;
    case 11:
        DexScreen_DrawMonDexPage(FALSE);
        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);
        sPokedexScreenData->state = 5;
        break;
    case 12:
        sub_81067C0();
        FillBgTilemapBufferRect_Palette0(0, 0x000, 0, 2, 30, 16);
        CopyBgTilemapBufferToVram(0);
        sPokedexScreenData->state = 1;
        break;
    }
}

static bool32 sub_8104664(u8 a0)
{
    int r3;
    u16 *r6, *r12;

    switch (sPokedexScreenData->dexOrderId)
    {
    default:
    case 0:
        r12 = &sPokedexScreenData->kantoOrderMenuCursorPos;
        r6 = &sPokedexScreenData->kantoOrderMenuItemsAbove;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        r12 = &sPokedexScreenData->characteristicOrderMenuCursorPos;
        r6 = &sPokedexScreenData->characteristicOrderMenuItemsAbove;
        break;
    case 5:
        r12 = &sPokedexScreenData->nationalOrderMenuCursorPos;
        r6 = &sPokedexScreenData->nationalOrderMenuItemsAbove;
        break;
    }

    r3 = *r12 + *r6;
    if (a0)
    {
        if (r3 == 0)
            return FALSE;

        r3--;
        while (r3 >= 0) //Should be while (--r3 >= 0) without the r3-- in the body or before the while at all, but this is needed to match.
        {
            if ((sPokedexScreenData->listItems[r3].index >> 16) & 1)
            {
                break;
            }
            r3--;
        }

        if (r3 < 0)
        {
            return FALSE;
        }
    }
    else
    {
        if (r3 == sPokedexScreenData->orderedDexCount - 1)
        {
            return FALSE;
        }

        r3++;
        while (r3 < sPokedexScreenData->orderedDexCount) //Should be while (++r3 < sPokedexScreenData->orderedDexCount) without the r3++ in the body or before the while at all, but this is needed to match.
        {
            if ((sPokedexScreenData->listItems[r3].index >> 16) & 1)
                break;
            r3++;
        }
        if (r3 >= sPokedexScreenData->orderedDexCount)
        {
            return FALSE;
        }
    }
    sPokedexScreenData->characteristicMenuInput = sPokedexScreenData->listItems[r3].index;

    if (sPokedexScreenData->orderedDexCount > 9)
    {
        if (r3 < 4)
        {
            *r12 = 0;
            *r6 = r3;
        }
        else if (r3 >= (sPokedexScreenData->orderedDexCount - 4))
        {
            *r12 = (sPokedexScreenData->orderedDexCount - 9);
            *r6 = r3 + 9 - (sPokedexScreenData->orderedDexCount);
        }
        else
        {
            *r12 = r3 - 4;
            *r6 = 4;
        }
    }
    else
    {
        *r12 = 0;
        *r6 = r3;
    }
    return TRUE;
}

static void DexScreen_RemoveWindow(u8 *windowId_p)
{
    if (*windowId_p != 0xFF)
    {
        RemoveWindow(*windowId_p);
        *windowId_p = 0xFF;
    }
}

static void DexScreen_AddTextPrinterParameterized(u8 windowId, u8 fontId, const u8 *str, u8 x, u8 y, u8 colorIdx)
{
    u8 textColor[3];
    switch (colorIdx)
    {
    case 0:
        textColor[0] = 0;
        textColor[1] = 1;
        textColor[2] = 3;
        break;
    case 1:
        textColor[0] = 0;
        textColor[1] = 5;
        textColor[2] = 1;
        break;
    case 2:
        textColor[0] = 0;
        textColor[1] = 15;
        textColor[2] = 14;
        break;
    case 3:
        textColor[0] = 0;
        textColor[1] = 11;
        textColor[2] = 1;
        break;
    case 4:
        textColor[0] = 0;
        textColor[1] = 1;
        textColor[2] = 2;
        break;
    }
    AddTextPrinterParameterized4(windowId, fontId, x, y, fontId == 0 ? 0 : 1, 0, textColor, -1, str);
}

void DexScreen_PrintNum3LeadingZeroes(u8 windowId, u8 fontId, u16 num, u8 x, u8 y, u8 colorIdx)
{
    u8 buff[4];
    buff[0] = (num / 100) + CHAR_0;
    buff[1] = ((num %= 100) / 10) + CHAR_0;
    buff[2] = (num % 10) + CHAR_0;
    buff[3] = EOS;
    DexScreen_AddTextPrinterParameterized(windowId, fontId, buff, x, y, colorIdx);
}

static void DexScreen_PrintNum3RightAlign(u8 windowId, u8 fontId, u16 num, u8 x, u8 y, u8 colorIdx)
{
    u8 buff[4];
    int i;
    buff[0] = (num / 100) + CHAR_0;
    buff[1] = ((num %= 100) / 10) + CHAR_0;
    buff[2] = (num % 10) + CHAR_0;
    buff[3] = EOS;
    for (i = 0; i < 3; i++)
    {
        if (buff[i] != CHAR_0)
            break;
        buff[i] = CHAR_SPACE;
    }
    DexScreen_AddTextPrinterParameterized(windowId, fontId, buff, x, y, colorIdx);
}

static u32 DexScreen_GetDefaultPersonality(int species)
{
    switch (species)
    {
    case SPECIES_SPINDA:
        return gSaveBlock2Ptr->pokedex.spindaPersonality;
    case SPECIES_UNOWN:
        return gSaveBlock2Ptr->pokedex.unownPersonality;
    default:
        return 0;
    }
}

static void DexScreen_LoadMonPicInWindow(u8 windowId, u16 species, u16 paletteOffset)
{
    LoadMonPicInWindow(species, 8, DexScreen_GetDefaultPersonality(species), TRUE, paletteOffset >> 4, windowId);
}

static void DexScreen_PrintMonDexNo(u8 windowId, u8 fontId, u16 species, u8 x, u8 y)
{
    u16 dexNum = SpeciesToNationalPokedexNum(species);
    DexScreen_AddTextPrinterParameterized(windowId, fontId, gText_PokedexNo, x, y, 0);
    DexScreen_PrintNum3LeadingZeroes(windowId, fontId, dexNum, x + 9, y, 0);
}

s8 DexScreen_GetSetPokedexFlag(u16 nationalDexNo, u8 caseId, bool8 indexIsSpecies)
{
    u8 index;
    u8 bit;
    u8 mask;
    s8 retVal;

    if (indexIsSpecies)
        nationalDexNo = SpeciesToNationalPokedexNum(nationalDexNo);

    nationalDexNo--;
    index = nationalDexNo / 8;
    bit = nationalDexNo % 8;
    mask = 1 << bit;
    retVal = 0;
    switch (caseId)
    {
    case FLAG_GET_SEEN:
        if (gSaveBlock2Ptr->pokedex.seen[index] & mask)
        {
            if ((gSaveBlock2Ptr->pokedex.seen[index] & mask) == (gSaveBlock1Ptr->seen1[index] & mask)
                && (gSaveBlock2Ptr->pokedex.seen[index] & mask) == (gSaveBlock1Ptr->seen2[index] & mask))
                retVal = 1;
        }
        break;
    case FLAG_GET_CAUGHT:
        if (gSaveBlock2Ptr->pokedex.owned[index] & mask)
        {
            if ((gSaveBlock2Ptr->pokedex.owned[index] & mask) == (gSaveBlock2Ptr->pokedex.seen[index] & mask)
                && (gSaveBlock2Ptr->pokedex.owned[index] & mask) == (gSaveBlock1Ptr->seen1[index] & mask)
                && (gSaveBlock2Ptr->pokedex.owned[index] & mask) == (gSaveBlock1Ptr->seen2[index] & mask))
                retVal = 1;
        }
        break;
    case FLAG_SET_SEEN:
        gSaveBlock2Ptr->pokedex.seen[index] |= mask;
        gSaveBlock1Ptr->seen1[index] |= mask;
        gSaveBlock1Ptr->seen2[index] |= mask;
        break;
    case FLAG_SET_CAUGHT:
        gSaveBlock2Ptr->pokedex.owned[index] |= mask;
        break;
    }
    return retVal;
}

static u16 DexScreen_GetDexCount(u8 caseId, bool8 whichDex)
{
    u16 count = 0;
    u16 i;

    switch (whichDex)
    {
    case 0: // Kanto
        for (i = 0; i < KANTO_DEX_COUNT; i++)
        {
            if (DexScreen_GetSetPokedexFlag(i + 1, caseId, FALSE))
                count++;
        }
        break;
    case 1: // National
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            if (DexScreen_GetSetPokedexFlag(i + 1, caseId, FALSE))
                count++;

        }
        break;
    }
    return count;
}

static void DexScreen_PrintControlInfo(const u8 *src)
{
    DexScreen_AddTextPrinterParameterized(1, 0, src, 236 - GetStringWidth(0, src, 0), 2, 4);
}

bool8 DexScreen_DrawMonPicInCategoryPage(u16 species, u8 slot, u8 numSlots)
{
    struct WindowTemplate template;
    numSlots--;
    CopyToBgTilemapBufferRect_ChangePalette(3, sCategoryPageIconWindowBg, sCategoryPageIconCoords[numSlots][slot][0], sCategoryPageIconCoords[numSlots][slot][1], 8, 8, slot + 5);
    if (sPokedexScreenData->categoryMonWindowIds[slot] == 0xFF)
    {
        template = sWindowTemplate_CategoryMonIcon;
        template.tilemapLeft = sCategoryPageIconCoords[numSlots][slot][0];
        template.tilemapTop = sCategoryPageIconCoords[numSlots][slot][1];
        template.paletteNum = slot + 1;
        template.baseBlock = slot * 64 + 8;
        sPokedexScreenData->categoryMonWindowIds[slot] = AddWindow(&template);
        FillWindowPixelBuffer(sPokedexScreenData->categoryMonWindowIds[slot], PIXEL_FILL(0));
        DexScreen_LoadMonPicInWindow(sPokedexScreenData->categoryMonWindowIds[slot], species, slot * 16 + 16);
        PutWindowTilemap(sPokedexScreenData->categoryMonWindowIds[slot]);
        CopyWindowToVram(sPokedexScreenData->categoryMonWindowIds[slot], COPYWIN_GFX);
    }
    else
        PutWindowTilemap(sPokedexScreenData->categoryMonWindowIds[slot]);

    if (sPokedexScreenData->categoryMonInfoWindowIds[slot] == 0xFF)
    {
        if (species != SPECIES_NONE)
        {
            template = sWindowTemplate_CategoryMonInfo;
            template.tilemapLeft = sCategoryPageIconCoords[numSlots][slot][2];
            template.tilemapTop = sCategoryPageIconCoords[numSlots][slot][3];
            template.baseBlock = slot * 40 + 0x108;
            sPokedexScreenData->categoryMonInfoWindowIds[slot] = AddWindow(&template);
            CopyToWindowPixelBuffer(sPokedexScreenData->categoryMonInfoWindowIds[slot], sCategoryMonInfoBgTiles, 0, 0);
            DexScreen_PrintMonDexNo(sPokedexScreenData->categoryMonInfoWindowIds[slot], 0, species, 12, 0);
            DexScreen_AddTextPrinterParameterized(sPokedexScreenData->categoryMonInfoWindowIds[slot], 2, gSpeciesNames[species], 2, 13, 0);
            if (DexScreen_GetSetPokedexFlag(species, FLAG_GET_CAUGHT, TRUE))
                BlitBitmapRectToWindow(sPokedexScreenData->categoryMonInfoWindowIds[slot], gUnknown_8443600, 0, 0, 8, 8, 2, 3, 8, 8);
            PutWindowTilemap(sPokedexScreenData->categoryMonInfoWindowIds[slot]);
            CopyWindowToVram(sPokedexScreenData->categoryMonInfoWindowIds[slot], COPYWIN_GFX);
        }
    }
    else
        PutWindowTilemap(sPokedexScreenData->categoryMonInfoWindowIds[slot]);

    return TRUE;
}

static void DexScreen_DestroyCategoryPageMonIconAndInfoWindows(void)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        DexScreen_RemoveWindow(&sPokedexScreenData->categoryMonWindowIds[i]);
        DexScreen_RemoveWindow(&sPokedexScreenData->categoryMonInfoWindowIds[i]);
    }
}

static void DexScreen_PrintCategoryPageNumbers(u8 windowId, u16 currentPage, u16 totalPages, u16 x, u16 y)
{
    u8 buffer[30];
    u8 *ptr = StringCopy(buffer, gText_Page);
    ptr = ConvertIntToDecimalStringN(ptr, currentPage, STR_CONV_MODE_RIGHT_ALIGN, 2);
    *ptr++ = CHAR_SLASH;
    ptr = ConvertIntToDecimalStringN(ptr, totalPages, STR_CONV_MODE_RIGHT_ALIGN, 2);
    DexScreen_PrintStringWithAlignment(buffer, TEXT_RIGHT);
}

static bool8 DexScreen_CreateCategoryListGfx(bool8 justRegistered)
{
    FillBgTilemapBufferRect_Palette0(3, 2, 0, 0, 30, 20);
    FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 32, 20);
    FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 20);
    DexScreen_CreateCategoryPageSpeciesList(sPokedexScreenData->category, sPokedexScreenData->pageNum);
    FillWindowPixelBuffer(0, PIXEL_FILL(15));
    if (justRegistered)
    {
        DexScreen_PrintStringWithAlignment(sDexCategoryNamePtrs[sPokedexScreenData->category], TEXT_CENTER);
    }
    else
    {
        DexScreen_PrintStringWithAlignment(sDexCategoryNamePtrs[sPokedexScreenData->category], TEXT_LEFT);
        DexScreen_PrintCategoryPageNumbers(0, DexScreen_PageNumberToRenderablePages(sPokedexScreenData->pageNum), DexScreen_PageNumberToRenderablePages(sPokedexScreenData->lastPageInCategory - 1), 160, 2);
    }
    CopyWindowToVram(0, COPYWIN_GFX);
    FillWindowPixelBuffer(1, PIXEL_FILL(15));
    if (!justRegistered)
        DexScreen_PrintControlInfo(gText_PickFlipPageCheckCancel);
    CopyWindowToVram(1, COPYWIN_GFX);
    if (sPokedexScreenData->pageSpecies[0] != 0xFFFF)
        DexScreen_DrawMonPicInCategoryPage(sPokedexScreenData->pageSpecies[0], 0, sPokedexScreenData->numMonsOnPage);
    if (sPokedexScreenData->pageSpecies[1] != 0xFFFF)
        DexScreen_DrawMonPicInCategoryPage(sPokedexScreenData->pageSpecies[1], 1, sPokedexScreenData->numMonsOnPage);
    if (sPokedexScreenData->pageSpecies[2] != 0xFFFF)
        DexScreen_DrawMonPicInCategoryPage(sPokedexScreenData->pageSpecies[2], 2, sPokedexScreenData->numMonsOnPage);
    if (sPokedexScreenData->pageSpecies[3] != 0xFFFF)
        DexScreen_DrawMonPicInCategoryPage(sPokedexScreenData->pageSpecies[3], 3, sPokedexScreenData->numMonsOnPage);
    return FALSE;
}

static void DexScreen_CreateCategoryPageSelectionCursor(u8 cursorPos)
{
    int i;
    u32 palIdx;

    if (cursorPos == 0xFF)
    {
        for (i = 0; i < 4; i++)
        {
            LoadPalette(&sDexScreen_CategoryCursorPals[0], 0x52 + 0x10 * i, 2);
            LoadPalette(&sDexScreen_CategoryCursorPals[1], 0x58 + 0x10 * i, 2);
        }
        LoadPalette(&sDexScreen_CategoryCursorPals[0], 0x141, 2);
        sPokedexScreenData->categoryPageSelectionCursorTimer = 0;
    }
    else
    {
        sPokedexScreenData->categoryPageSelectionCursorTimer++;
        if (sPokedexScreenData->categoryPageSelectionCursorTimer == 16)
            sPokedexScreenData->categoryPageSelectionCursorTimer = 0;
        palIdx = sPokedexScreenData->categoryPageSelectionCursorTimer >> 2;
        for (i = 0; i < 4; i++)
        {
            if (i == cursorPos)
            {
                LoadPalette(&sDexScreen_CategoryCursorPals[2 * palIdx + 2], 0x52 + 0x10 * i, 2);
                LoadPalette(&sDexScreen_CategoryCursorPals[2 * palIdx + 3], 0x58 + 0x10 * i, 2);
            }
            else
            {
                LoadPalette(&sDexScreen_CategoryCursorPals[0], 0x52 + 0x10 * i, 2);
                LoadPalette(&sDexScreen_CategoryCursorPals[1], 0x58 + 0x10 * i, 2);
            }
        }
        LoadPalette(&sDexScreen_CategoryCursorPals[2 * palIdx + 2], 0x141, 2);
    }
}

static void DexScreen_UpdateCategoryPageCursorObject(u8 taskId, u8 cursorPos, u8 numMonsInPage)
{
    numMonsInPage--;
    ListMenuUpdateCursorObject(taskId, sCategoryPageIconCoords[numMonsInPage][cursorPos][2] * 8, sCategoryPageIconCoords[numMonsInPage][cursorPos][3] * 8, 0);
}

bool8 sub_81051AC(const u16 *a0, u8 a1, u16 *a2, u8 a3)
{
    int i;
    const u16 *src = &a0[a1];
    u16 *dst = &a2[a3];
    for (i = 0; i < 20; i++)
    {
        *dst = *src;
        dst += 32;
        src += 32;
    }
    return FALSE;
}

bool8 sub_81051D0(u16 a0, u16 *a1, u8 a2)
{
    int i;
    u16 *dst = &a1[a2];
    for (i = 0; i < 20; i++)
    {
        *dst = a0;
        dst += 32;
    }
    return FALSE;
}

bool8 sub_81051F0(u8 a0)
{
    int i;
    int r4;
    u16 *bg1buff = GetBgTilemapBuffer(1);
    u16 *bg2buff = GetBgTilemapBuffer(2);
    u16 *bg3buff = GetBgTilemapBuffer(3);
    u16 *sp04 = sPokedexScreenData->field_5C + 0x800;
    u16 *sp08 = sPokedexScreenData->field_5C + 0x400;
    u16 *sp0C = sPokedexScreenData->field_5C + 0x000;
    for (i = 0; i < 30; i++)
    {
        r4 = sUnknown_8452388[a0][i];
        if (r4 == 30)
        {
            sub_81051D0(0x000, bg1buff, i);
            sub_81051D0(0x000, bg2buff, i);
            sub_81051D0(0x00C, bg3buff, i);
        }
        else
        {
            sub_81051AC(sp04, r4, bg1buff, i);
            sub_81051AC(sp08, r4, bg2buff, i);
            sub_81051AC(sp0C, r4, bg3buff, i);
        }
    }
    CopyBgTilemapBufferToVram(1);
    CopyBgTilemapBufferToVram(2);
    CopyBgTilemapBufferToVram(3);
    return FALSE;
}

static bool8 sub_81052D0(u8 a0)
{
    u16 r4;
    if (IsNationalPokedexEnabled())
        r4 = sNationalDexPalette[7];
    else
        r4 = sKantoDexPalette[7];
    switch (sPokedexScreenData->data[0])
    {
    case 0:
        sPokedexScreenData->field_5C = Alloc(3 * BG_SCREEN_SIZE);
        if (a0)
            sPokedexScreenData->data[0] = 6;
        else
            sPokedexScreenData->data[0] = 2;
        break;
    case 1:
        Free(sPokedexScreenData->field_5C);
        return TRUE;
    case 2:
        BeginNormalPaletteFade(0x00007FFF, 0, 0, 16, r4);
        sPokedexScreenData->data[0]++;
        break;
    case 3:
        FillBgTilemapBufferRect_Palette0(3, 0x00C, 0, 0, 30, 20);
        FillBgTilemapBufferRect_Palette0(2, 0x000, 0, 0, 32, 20);
        FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 0, 32, 20);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        sPokedexScreenData->data[0]++;
        break;
    case 4:
        BeginNormalPaletteFade(0x00007FFF, 0, 0, 0, r4);
        DexScreen_CreateCategoryListGfx(FALSE);
        CpuFastCopy(GetBgTilemapBuffer(3), &sPokedexScreenData->field_5C[0 * BG_SCREEN_SIZE / 2], BG_SCREEN_SIZE);
        CpuFastCopy(GetBgTilemapBuffer(2), &sPokedexScreenData->field_5C[1 * BG_SCREEN_SIZE / 2], BG_SCREEN_SIZE);
        CpuFastCopy(GetBgTilemapBuffer(1), &sPokedexScreenData->field_5C[2 * BG_SCREEN_SIZE / 2], BG_SCREEN_SIZE);
        FillBgTilemapBufferRect_Palette0(3, 0x00C, 0, 0, 30, 20);
        FillBgTilemapBufferRect_Palette0(2, 0x000, 0, 0, 32, 20);
        FillBgTilemapBufferRect_Palette0(1, 0x000, 0, 0, 32, 20);
        
        sPokedexScreenData->data[1] = 0;
        sPokedexScreenData->data[0]++;
        PlaySE(SE_BALL_TRAY_ENTER);
        break;
    case 5:
        if (sPokedexScreenData->data[1] < 10)
        {
            sub_81051F0(sPokedexScreenData->data[1]);
            sPokedexScreenData->data[1]++;
        }
        else
        {
            sPokedexScreenData->data[0] = 1;
        }
        break;
    case 6:
        CpuFastCopy(GetBgTilemapBuffer(3), &sPokedexScreenData->field_5C[0 * BG_SCREEN_SIZE / 2], BG_SCREEN_SIZE);
        CpuFastCopy(GetBgTilemapBuffer(2), &sPokedexScreenData->field_5C[1 * BG_SCREEN_SIZE / 2], BG_SCREEN_SIZE);
        CpuFastCopy(GetBgTilemapBuffer(1), &sPokedexScreenData->field_5C[2 * BG_SCREEN_SIZE / 2], BG_SCREEN_SIZE);

        sPokedexScreenData->data[1] = 9;
        sPokedexScreenData->data[0]++;
        PlaySE(SE_BALL_TRAY_ENTER);
        break;
    case 7:
        if (sPokedexScreenData->data[1] != 0)
        {
            sub_81051F0(sPokedexScreenData->data[1]);
            sPokedexScreenData->data[1]--;
        }
        else
        {
            sub_81051F0(sPokedexScreenData->data[0]);
            BeginNormalPaletteFade(0x00007FFF, 0, 16, 16, r4);
            sPokedexScreenData->data[0]++;
        }
        break;
    case 8:
        gPaletteFade.bufferTransferDisabled = TRUE;
        DexScreen_CreateCategoryListGfx(FALSE);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        sPokedexScreenData->data[0]++;
        break;
    case 9:
        gPaletteFade.bufferTransferDisabled = FALSE;
        BeginNormalPaletteFade(0x00007FFF, 0, 16, 0, r4);
        sPokedexScreenData->data[0] = 1;
        break;
    }
    return FALSE;
}

void sub_8105594(u8 a0, u8 a1)
{
    u8 v0, v1, v2, v3;
    s16 v4, v5, v6;
 
    if (!sPokedexScreenData->numMonsOnPage)
    {
        v0 = sCategoryPageIconCoords[0][0][2];
        v1 = sCategoryPageIconCoords[0][0][3];
    }
    else
    {
        v0 = sCategoryPageIconCoords[sPokedexScreenData->numMonsOnPage - 1][sPokedexScreenData->categoryCursorPosInPage][2];
        v1 = sCategoryPageIconCoords[sPokedexScreenData->numMonsOnPage - 1][sPokedexScreenData->categoryCursorPosInPage][3];
    }
 
    v2 = 6 + (a1 * 4);
    v3 = 3 + (a1 * 2);
    if (v2 >= 28)
        v2 = 28;
    if (v3 >= 14)
        v3 = 14;
    v4 = v0 - ((a1 * 4) / 2);
    v5 = v1 - ((a1 * 2) / 2);
    if ((v4 + v2 + 2) >= 30)
        v4 -= ((v4 + v2 + 2) - 30);
    else if (v4 < 0)
        v4 = 0;
    if ((v5 + v3 + 2) >= 18)
        v5 -= ((v5 + v3 + 2) - 18);
    else if (v5 < 2)
        v5 = 2;
    v6 = (v5 + 1) + ((v3 / 2) + 1);
 
    FillBgTilemapBufferRect_Palette0(a0, 4, v4, v5, 1, 1);
    FillBgTilemapBufferRect_Palette0(a0, 5, v4 + 1, v5, v2, 1);
    FillBgTilemapBufferRect_Palette0(a0, 1028, v4 + 1 + v2, v5, 1, 1);
 
    FillBgTilemapBufferRect_Palette0(a0, 10, v4, v5 + 1 + v3, 1, 1);
    FillBgTilemapBufferRect_Palette0(a0, 11, v4 + 1, v5 + 1 + v3, v2, 1);
    FillBgTilemapBufferRect_Palette0(a0, 1034, v4 + 1 + v2, v5 + 1 + v3, 1, 1);
 
    FillBgTilemapBufferRect_Palette0(a0, 6, v4, v5 + 1, 1, v6 - v5 - 1);
    FillBgTilemapBufferRect_Palette0(a0, 7, v4, v6, 1, 1);
    FillBgTilemapBufferRect_Palette0(a0, 9, v4, v6 + 1, 1, v5 + v3 - v6);
 
    FillBgTilemapBufferRect_Palette0(a0, 1030, v4 + 1 + v2, v5 + 1, 1, v6 - v5 - 1);
    FillBgTilemapBufferRect_Palette0(a0, 1031, v4 + 1 + v2, v6, 1, 1);
    FillBgTilemapBufferRect_Palette0(a0, 1033, v4 + 1 + v2, v6 + 1, 1, v5 + v3 - v6);
 
    FillBgTilemapBufferRect_Palette0(a0, 1, v4 + 1, v5 + 1, v2, v6 - v5 - 1);
    FillBgTilemapBufferRect_Palette0(a0, 8, v4 + 1, v6, v2, 1);
    FillBgTilemapBufferRect_Palette0(a0, 2, v4 + 1, v6 + 1, v2, v5 + v3 - v6);
}

void sub_8105800(u8 a0, u16 species, u8 a2, u8 a3)
{
    u8 * categoryName;
    u8 index, categoryStr[12];

    species = SpeciesToNationalPokedexNum(species);

    categoryName = (u8 *)gPokedexEntries[species].categoryName;
    index = 0;
    if (DexScreen_GetSetPokedexFlag(species, FLAG_GET_CAUGHT, FALSE))
    {
#if REVISION == 0
        while ((categoryName[index] != CHAR_SPACE) && (index < 11))
#else
        while ((categoryName[index] != EOS) && (index < 11))
#endif
        {
            categoryStr[index] = categoryName[index];
            index++;
        }
    }
    else
    {
        while (index < 11)
        {
            categoryStr[index] = CHAR_QUESTION_MARK;
            index++;
        }
    }
    
    categoryStr[index] = EOS;

    DexScreen_AddTextPrinterParameterized(a0, 0, categoryStr, a2, a3, 0);
    a2 += GetStringWidth(0, categoryStr, 0);
    DexScreen_AddTextPrinterParameterized(a0, 0, gText_PokedexPokemon, a2, a3, 0);
}

void sub_81058C4(u8 windowId, u16 species, u8 x, u8 y)
{
    u16 height;
    u32 inches, feet;
    const u8 *labelText;
    u8 buffer[32];
    u8 i;

    species = SpeciesToNationalPokedexNum(species);
    height = gPokedexEntries[species].height;
    labelText = gText_HT;

    i = 0;
    buffer[i++] = EXT_CTRL_CODE_BEGIN;
    buffer[i++] = EXT_CTRL_CODE_MIN_LETTER_SPACING;
    buffer[i++] = 5;
    buffer[i++] = CHAR_SPACE;

    if (DexScreen_GetSetPokedexFlag(species, FLAG_GET_CAUGHT, FALSE))
    {
        inches = 10000 * height / 254; // actually tenths of inches here
        if (inches % 10 >= 5)
            inches += 10;
        feet = inches / 120;
        inches = (inches - (feet * 120)) / 10;
        if (feet / 10 == 0)
        {
            buffer[i++] = 0;
            buffer[i++] = feet + CHAR_0;
        }
        else
        {
            buffer[i++] = feet / 10 + CHAR_0;
            buffer[i++] = feet % 10 + CHAR_0;
        }
        buffer[i++] = CHAR_SGL_QUOT_RIGHT;
        buffer[i++] = inches / 10 + CHAR_0;
        buffer[i++] = inches % 10 + CHAR_0;
        buffer[i++] = CHAR_DBL_QUOT_RIGHT;
        buffer[i++] = EOS;
    }
    else
    {
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_SGL_QUOT_RIGHT;
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_DBL_QUOT_RIGHT;
    }

    buffer[i++] = EOS;
    DexScreen_AddTextPrinterParameterized(windowId, 0, labelText, x, y, 0);
    x += 30;
    DexScreen_AddTextPrinterParameterized(windowId, 0, buffer, x, y, 0);
}

void sub_8105A3C(u8 windowId, u16 species, u8 x, u8 y)
{
    u16 weight;
    u32 lbs;
    bool8 output;
    const u8 * labelText;
    const u8 * lbsText;
    u8 buffer[32];
    u8 i;
    u32 j;

    species = SpeciesToNationalPokedexNum(species);
    weight = gPokedexEntries[species].weight;
    labelText = gText_WT;
    lbsText = gText_Lbs;

    i = 0;
    buffer[i++] = EXT_CTRL_CODE_BEGIN;
    buffer[i++] = EXT_CTRL_CODE_MIN_LETTER_SPACING;
    buffer[i++] = 5;

    if (DexScreen_GetSetPokedexFlag(species, FLAG_GET_CAUGHT, FALSE))
    {
        lbs = (weight * 100000) / 4536;

        if (lbs % 10 >= 5)
            lbs += 10;

        output = FALSE;

        if ((buffer[i] = (lbs / 100000) + CHAR_0) == CHAR_0 && !output)
        {
            buffer[i++] = CHAR_SPACE;
        }
        else
        {
            output = TRUE;
            i++;
        }

        lbs %= 100000;
        if ((buffer[i] = (lbs / 10000) + CHAR_0) == CHAR_0 && !output)
        {
            buffer[i++] = CHAR_SPACE;
        }
        else
        {
            output = TRUE;
            i++;
        }
        
        lbs %= 10000;
        if ((buffer[i] = (lbs / 1000) + CHAR_0) == CHAR_0 && !output)
        {
            buffer[i++] = CHAR_SPACE;
        }
        else
        {
            output = TRUE;
            i++;
        }

        lbs %= 1000;
        buffer[i++] = (lbs / 100) + CHAR_0;
        lbs %= 100;
        buffer[i++] = CHAR_PERIOD;
        buffer[i++] = (lbs / 10) + CHAR_0;
    }
    else
    {
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_QUESTION_MARK;
        buffer[i++] = CHAR_PERIOD;
        buffer[i++] = CHAR_QUESTION_MARK;
    }
    buffer[i++] = CHAR_SPACE;
    buffer[i++] = EXT_CTRL_CODE_BEGIN;
    buffer[i++] = EXT_CTRL_CODE_MIN_LETTER_SPACING;
    buffer[i++] = 0;

    for (j = 0; j < 33 - i && lbsText[j] != EOS; j++)
        buffer[i + j] = lbsText[j];

    buffer[i + j] = EOS;
    DexScreen_AddTextPrinterParameterized(windowId, 0, labelText, x, y, 0);
    x += 30;
    DexScreen_AddTextPrinterParameterized(windowId, 0, buffer, x, y, 0);
}

void sub_8105CB0(u8 a0, u16 species, u8 x, u8 y)
{
    struct TextPrinterTemplate printerTemplate;
    u16 length;
    s32 v1;

    species = SpeciesToNationalPokedexNum(species);

    if (DexScreen_GetSetPokedexFlag(species, FLAG_GET_CAUGHT, FALSE))
    {
        printerTemplate.currentChar = gPokedexEntries[species].description;
        printerTemplate.windowId = a0;
        printerTemplate.fontId = 2;
        printerTemplate.letterSpacing = 1;
        printerTemplate.lineSpacing = 0;
        printerTemplate.unk = 0;
        printerTemplate.fgColor = 1;
        printerTemplate.bgColor = 0;
        printerTemplate.shadowColor = 2;

        length = GetStringWidth(2, gPokedexEntries[species].description, 0);
        v1 = x + (240 - length) / 2;

        if (v1 > 0)
            x = v1;
        else
            x = 0;

        printerTemplate.x = x;
        printerTemplate.y = y;
        printerTemplate.currentX = x;
        printerTemplate.currentY = y;

        AddTextPrinter(&printerTemplate, TEXT_SPEED_FF, NULL);
    }
}

void sub_8105D64(u8 a0, u16 species, u8 a2, u8 a3)
{
    u16 i, j, unused, v3;
    u8 v4, v5;
    u8 * buffer;
    u8 * footprint;

    if (!(DexScreen_GetSetPokedexFlag(species, FLAG_GET_CAUGHT, TRUE)))
        return;
    footprint = (u8 *)(gMonFootprintTable[species]);
    buffer = gDecompressionBuffer;
    unused = 0;
    v3 = 0;

    for (i = 0; i < 32; i++)
    {
        v4 = footprint[i];
        for (j = 0; j < 4; j++)
        {
            v5 = 0;
            if (v4 & (1 << (j * 2)))
                v5 |= 1;
            if (v4 & (2 << (j * 2)))
                v5 |= 16;
            buffer[v3] = v5;
            v3++;
        }
    }
    BlitBitmapRectToWindow(a0, buffer, 0, 0, 16, 16, a2, a3, 16, 16);
}

static u8 DexScreen_DrawMonDexPage(bool8 justRegistered)
{
    sub_8105594(3, 6);
    FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 30, 20);
    FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 2, 30, 16);

    sPokedexScreenData->field_4A[0] = AddWindow(&gUnknown_84521D4);
    sPokedexScreenData->field_4A[1] = AddWindow(&gUnknown_84521DC);
    sPokedexScreenData->field_4A[2] = AddWindow(&gUnknown_84521E4);

    FillWindowPixelBuffer(sPokedexScreenData->field_4A[0], 0);
    DexScreen_LoadMonPicInWindow(sPokedexScreenData->field_4A[0], sPokedexScreenData->field_5A, 144);
    PutWindowTilemap(sPokedexScreenData->field_4A[0]);
    CopyWindowToVram(sPokedexScreenData->field_4A[0], 2);
    FillWindowPixelBuffer(sPokedexScreenData->field_4A[1], 0);
    DexScreen_PrintMonDexNo(sPokedexScreenData->field_4A[1], 0, sPokedexScreenData->field_5A, 0, 8);
    DexScreen_AddTextPrinterParameterized(sPokedexScreenData->field_4A[1], 2, gSpeciesNames[sPokedexScreenData->field_5A], 28, 8, 0);
    sub_8105800(sPokedexScreenData->field_4A[1], sPokedexScreenData->field_5A, 0, 24);
    sub_81058C4(sPokedexScreenData->field_4A[1], sPokedexScreenData->field_5A, 0, 36);
    sub_8105A3C(sPokedexScreenData->field_4A[1], sPokedexScreenData->field_5A, 0, 48);
    sub_8105D64(sPokedexScreenData->field_4A[1], sPokedexScreenData->field_5A, 88, 40);

    PutWindowTilemap(sPokedexScreenData->field_4A[1]);
    CopyWindowToVram(sPokedexScreenData->field_4A[1], 2);
    FillWindowPixelBuffer(sPokedexScreenData->field_4A[2], 0);
    sub_8105CB0(sPokedexScreenData->field_4A[2], sPokedexScreenData->field_5A, 0, 8);
    PutWindowTilemap(sPokedexScreenData->field_4A[2]);
    CopyWindowToVram(sPokedexScreenData->field_4A[2], 2);
    FillWindowPixelBuffer(1, 255);
    if (justRegistered == FALSE)
    {
        DexScreen_AddTextPrinterParameterized(1, 0, gText_Cry, 8, 2, 4);
        DexScreen_PrintControlInfo(gText_NextDataCancel);
    }
    else
        // Just registered
        DexScreen_PrintControlInfo(gText_Next);
    PutWindowTilemap(1);
    CopyWindowToVram(1, 2);

    return 1;
}

u8 sub_8106014(void)
{
    DexScreen_RemoveWindow(&sPokedexScreenData->field_4A[0]);
    DexScreen_RemoveWindow(&sPokedexScreenData->field_4A[1]);
    DexScreen_RemoveWindow(&sPokedexScreenData->field_4A[2]);

    return 0;
}

u8 sub_810603C(void)
{
    int i;
    u8 v1, v2;
    bool8 v3;
    s16 v4, v5;
    u16 speciesId, species;
    u16 v8;

    species = sPokedexScreenData->field_5A;
    speciesId = SpeciesToNationalPokedexNum(species);
    v3 = DexScreen_GetSetPokedexFlag(species, FLAG_GET_CAUGHT, TRUE);
    v1 = 28;
    v2 = 14;
    v4 = 0;
    v5 = 2;

    FillBgTilemapBufferRect_Palette0(3, 4, v4, v5, 1, 1);
    FillBgTilemapBufferRect_Palette0(3, 1028, v4 + 1 + v1, v5, 1, 1);
    FillBgTilemapBufferRect_Palette0(3, 2052, v4, v5 + 1 + v2, 1, 1);
    FillBgTilemapBufferRect_Palette0(3, 3076, v4 + 1 + v1, v5 + 1 + v2, 1, 1);
    FillBgTilemapBufferRect_Palette0(3, 5, v4 + 1, v5, v1, 1);
    FillBgTilemapBufferRect_Palette0(3, 2053, v4 + 1, v5 + 1 + v2, v1, 1);
    FillBgTilemapBufferRect_Palette0(3, 6, v4, v5 + 1, 1, v2);
    FillBgTilemapBufferRect_Palette0(3, 1030, v4 + 1 + v1, v5 + 1, 1, v2);
    FillBgTilemapBufferRect_Palette0(3, 1, v4 + 1, v5 + 1, v1, v2);
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 2, 30, 16);

    v1 = 10;
    v2 = 6;
    v4 = 1;
    v5 = 9;

    FillBgTilemapBufferRect_Palette0(0, 29, v4, v5, 1, 1);
    FillBgTilemapBufferRect_Palette0(0, 1053, v4 + 1 + v1, v5, 1, 1);
    FillBgTilemapBufferRect_Palette0(0, 2077, v4, v5 + 1 + v2, 1, 1);
    FillBgTilemapBufferRect_Palette0(0, 3101, v4 + 1 + v1, v5 + 1 + v2, 1, 1);
    FillBgTilemapBufferRect_Palette0(0, 30, v4 + 1, v5, v1, 1);
    FillBgTilemapBufferRect_Palette0(0, 2078, v4 + 1, v5 + 1 + v2, v1, 1);
    FillBgTilemapBufferRect_Palette0(0, 31, v4, v5 + 1, 1, v2);
    FillBgTilemapBufferRect_Palette0(0, 1055, v4 + 1 + v1, v5 + 1, 1, v2);
    FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 30, 20);
    FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);

    sPokedexScreenData->field_64 = GetUnlockedSeviiAreas();
    v8 = 4;
    for (i = 3; i < 7; i++)
        if ((sPokedexScreenData->field_64 >> i) & 1)
            v8 = 0;

    sPokedexScreenData->field_4A[0] = AddWindow(&gUnknown_8452214);
    CopyToWindowPixelBuffer(sPokedexScreenData->field_4A[0], (void *)gUnknown_8443620, 0, 0);
    SetWindowAttribute(sPokedexScreenData->field_4A[0], 2,
                       GetWindowAttribute(sPokedexScreenData->field_4A[0], 2) + v8);
    PutWindowTilemap(sPokedexScreenData->field_4A[0]);
    for (i = 0; i < 7; i++)
        if ((sPokedexScreenData->field_64 >> i) & 1)
        {
            sPokedexScreenData->field_4A[i + 1] = AddWindow(gUnknown_8452254[i].window);
            CopyToWindowPixelBuffer(sPokedexScreenData->field_4A[i + 1], gUnknown_8452254[i].tilemap, 0, 0);
            SetWindowAttribute(sPokedexScreenData->field_4A[i + 1], 2, GetWindowAttribute(sPokedexScreenData->field_4A[i + 1], 2) + v8);
            PutWindowTilemap(sPokedexScreenData->field_4A[i + 1]);
            CopyWindowToVram(sPokedexScreenData->field_4A[i + 1], 2);
        }
    sPokedexScreenData->field_4A[8] = AddWindow(&gUnknown_84521F4);
    sPokedexScreenData->field_4A[9] = AddWindow(&gUnknown_84521FC);
    sPokedexScreenData->field_4A[10] = AddWindow(&gUnknown_8452204);
    sPokedexScreenData->field_4A[11] = AddWindow(&gUnknown_84521EC);
    sPokedexScreenData->field_4A[12] = AddWindow(&gUnknown_845220C);
    FillWindowPixelBuffer(sPokedexScreenData->field_4A[11], 0);
    sub_8107CD8(160, species);
    sub_8107CF8(sPokedexScreenData->field_4A[11], species, DexScreen_GetDefaultPersonality(species), 0, 0);
    PutWindowTilemap(sPokedexScreenData->field_4A[11]);
    CopyWindowToVram(sPokedexScreenData->field_4A[11], 2);
    FillWindowPixelBuffer(sPokedexScreenData->field_4A[9], 0);
    {
        s32 width = GetStringWidth(0, gText_Size, 0);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->field_4A[9], 0, gText_Size, (gUnknown_84521FC.width * 8 - width) / 2, 4, 0);
    }
    PutWindowTilemap(sPokedexScreenData->field_4A[9]);
    CopyWindowToVram(sPokedexScreenData->field_4A[9], 2);

    FillWindowPixelBuffer(sPokedexScreenData->field_4A[10], 0);
    {
        s32 width = GetStringWidth(0, gText_Area, 0);
        DexScreen_AddTextPrinterParameterized(sPokedexScreenData->field_4A[10], 0, gText_Area, (gUnknown_8452204.width * 8 - width) / 2, 4, 0);
    }
    SetWindowAttribute(sPokedexScreenData->field_4A[10], 2, GetWindowAttribute(sPokedexScreenData->field_4A[10], 2) + v8);
    PutWindowTilemap(sPokedexScreenData->field_4A[10]);
    CopyWindowToVram(sPokedexScreenData->field_4A[10], 2);
    FillWindowPixelBuffer(sPokedexScreenData->field_4A[8], 0);
    DexScreen_PrintMonDexNo(sPokedexScreenData->field_4A[8], 0, species, 0, 0);
    DexScreen_AddTextPrinterParameterized(sPokedexScreenData->field_4A[8], 2, gSpeciesNames[species], 3, 12, 0);
    PutWindowTilemap(sPokedexScreenData->field_4A[8]);
    CopyWindowToVram(sPokedexScreenData->field_4A[8], 2);
    FillWindowPixelBuffer(sPokedexScreenData->field_4A[12], 0);
    ListMenuLoadStdPalAt(176, 1);

    if (v3)
    {
        BlitMoveInfoIcon(sPokedexScreenData->field_4A[12], 1 + gBaseStats[species].type1, 0, 1);
        if (gBaseStats[species].type1 != gBaseStats[species].type2)
            BlitMoveInfoIcon(sPokedexScreenData->field_4A[12], 1 + gBaseStats[species].type2, 32, 1);
    }
    PutWindowTilemap(sPokedexScreenData->field_4A[12]);
    CopyWindowToVram(sPokedexScreenData->field_4A[12], 2);
    ResetAllPicSprites();
    LoadPalette(gUnknown_8452368, 288, 32);

    if (v3)
    {
        sPokedexScreenData->field_4A[14] = CreateMonPicSprite_HandleDeoxys(species, 8, DexScreen_GetDefaultPersonality(species), 1, 40, 104, 0, 65535);
        gSprites[sPokedexScreenData->field_4A[14]].oam.paletteNum = 2;
        gSprites[sPokedexScreenData->field_4A[14]].oam.affineMode = 1;
        gSprites[sPokedexScreenData->field_4A[14]].oam.matrixNum = 2;
        gSprites[sPokedexScreenData->field_4A[14]].oam.priority = 1;
        gSprites[sPokedexScreenData->field_4A[14]].pos2.y = gPokedexEntries[speciesId].pokemonOffset;
        SetOamMatrix(2, gPokedexEntries[speciesId].pokemonScale, 0, 0, gPokedexEntries[speciesId].pokemonScale);
        sPokedexScreenData->field_4A[15] = CreateTrainerPicSprite(PlayerGenderToFrontTrainerPicId_Debug(gSaveBlock2Ptr->playerGender, 1), 1, 80, 104, 0, 65535);
        gSprites[sPokedexScreenData->field_4A[15]].oam.paletteNum = 2;
        gSprites[sPokedexScreenData->field_4A[15]].oam.affineMode = 1;
        gSprites[sPokedexScreenData->field_4A[15]].oam.matrixNum = 1;
        gSprites[sPokedexScreenData->field_4A[15]].oam.priority = 1;
        gSprites[sPokedexScreenData->field_4A[15]].pos2.y = gPokedexEntries[speciesId].trainerOffset;
        SetOamMatrix(1, gPokedexEntries[speciesId].trainerScale, 0, 0, gPokedexEntries[speciesId].trainerScale);
    }
    else
    {
        sPokedexScreenData->field_4A[14] = 0xff;
        sPokedexScreenData->field_4A[15] = 0xff;
    }

    sPokedexScreenData->data[2] = sub_8134230(species, 2001, 3, v8 * 8);
    if (!(sub_81344E0(sPokedexScreenData->data[2])))
    {
        BlitBitmapRectToWindow(sPokedexScreenData->field_4A[0], (void *)gUnknown_8443D00, 0, 0, 88, 16, 4, 28, 88, 16);
        {
            s32 width = GetStringWidth(0, gText_AreaUnknown, 0);
            DexScreen_AddTextPrinterParameterized(sPokedexScreenData->field_4A[0], 0, gText_AreaUnknown, (96 - width) / 2, 29, 0);
        }
    }
    CopyWindowToVram(sPokedexScreenData->field_4A[0], 2);
    FillWindowPixelBuffer(1, 255);
    DexScreen_AddTextPrinterParameterized(1, 0, gText_Cry, 8, 2, 4);
    DexScreen_PrintControlInfo(gText_CancelPreviousData);
    PutWindowTilemap(1);
    CopyWindowToVram(1, 2);

    return 1;
}


u8 sub_81067C0(void)
{
    int i;

    sub_81343F4(sPokedexScreenData->data[2]);

    for (i = 0; i < 13; i++)
        DexScreen_RemoveWindow(&sPokedexScreenData->field_4A[i]);
    if (sPokedexScreenData->field_4A[15] != 0xff)
        FreeAndDestroyTrainerPicSprite(sPokedexScreenData->field_4A[15]);
    if (sPokedexScreenData->field_4A[14] != 0xff)
        FreeAndDestroyMonPicSprite(sPokedexScreenData->field_4A[14]);
    return 0;
}

static int DexScreen_CanShowMonInDex(u16 species)
{
    if (IsNationalPokedexEnabled() == TRUE)
        return TRUE;
    if (SpeciesToNationalPokedexNum(species) <= KANTO_DEX_COUNT)
        return TRUE;
    return FALSE;
}

static u8 DexScreen_IsPageUnlocked(u8 categoryNum, u8 pageNum)
{
    int i, count;
    u16 species;

    count = gDexCategories[categoryNum].page[pageNum].count;

    for (i = 0; i < 4; i++)
    {
        if (i < count)
        {
            species = gDexCategories[categoryNum].page[pageNum].species[i];
            if (DexScreen_CanShowMonInDex(species) == TRUE && DexScreen_GetSetPokedexFlag(species, FLAG_GET_SEEN, TRUE))
                return TRUE;
        }
    }
    return FALSE;
}

static bool8 DexScreen_IsCategoryUnlocked(u8 categoryNum)
{
    int i;
    u8 count;

    count = gDexCategories[categoryNum].count;

    for (i = 0; i < count; i++)
        if (DexScreen_IsPageUnlocked(categoryNum, i))
            return 1;

    return 0;
}

void DexScreen_CreateCategoryPageSpeciesList(u8 categoryNum, u8 pageNum)
{
    int i, count;
    u16 species;

    count = gDexCategories[categoryNum].page[pageNum].count;
    sPokedexScreenData->numMonsOnPage = 0;

    for (i = 0; i < 4; i++)
        sPokedexScreenData->pageSpecies[i] = 0xffff;
    for (i = 0; i < count; i++)
    {
        species = gDexCategories[categoryNum].page[pageNum].species[i];
        if (DexScreen_CanShowMonInDex(species) == TRUE && DexScreen_GetSetPokedexFlag(species, FLAG_GET_SEEN, TRUE))
        {
            sPokedexScreenData->pageSpecies[sPokedexScreenData->numMonsOnPage] = gDexCategories[categoryNum].page[pageNum].species[i];
            sPokedexScreenData->numMonsOnPage++;
        }
    }
}

static u8 DexScreen_GetPageLimitsForCategory(u8 category)
{
    int i;
    u8 count, firstPage, lastPage;

    count = gDexCategories[category].count;
    firstPage = 0xff;
    lastPage = 0xff;

    for (i = 0; i < count; i++)
        if (DexScreen_IsPageUnlocked(category, i))
        {
            if (firstPage == 0xff)
                firstPage = i;
            lastPage = i;
        }
    if (lastPage != 0xff)
    {
        sPokedexScreenData->firstPageInCategory = firstPage;
        sPokedexScreenData->lastPageInCategory = lastPage + 1;
        return FALSE;
    }
    else
    {
        sPokedexScreenData->firstPageInCategory = 0;
        sPokedexScreenData->lastPageInCategory = 0;
        return TRUE;
    }
}

static u8 DexScreen_LookUpCategoryBySpecies(u16 species)
{
    int i, j, k, categoryCount, categoryPageCount, posInPage;
    u16 dexSpecies;

    for (i = 0; i < NELEMS(gDexCategories); i++)
    {
        categoryCount = gDexCategories[i].count;
        for (j = 0; j < categoryCount; j++)
        {
            categoryPageCount = gDexCategories[i].page[j].count;
            for (k = 0, posInPage = 0; k < categoryPageCount; k++)
            {
                dexSpecies = gDexCategories[i].page[j].species[k];
                if (species == dexSpecies)
                {
                    sPokedexScreenData->category = i;
                    sPokedexScreenData->pageNum = j;
                    sPokedexScreenData->categoryCursorPosInPage = posInPage;
                    return FALSE;
                }
                if (DexScreen_CanShowMonInDex(dexSpecies) == TRUE && DexScreen_GetSetPokedexFlag(dexSpecies, FLAG_GET_SEEN, TRUE))
                    posInPage++;
            }
        }
    }
    return TRUE;
}

static u8 DexScreen_PageNumberToRenderablePages(u16 page)
{
    int i, count;

    for (i = 0, count = 0; i < page; i++)
        if (DexScreen_IsPageUnlocked(sPokedexScreenData->category, i))
            count++;

    return count + 1;
}

void sub_8106B34(void)
{
    if (JOY_NEW(START_BUTTON))
        PlayCry2(sPokedexScreenData->field_5A, 0, 125, 10);
}

u8 DexScreen_RegisterMonToPokedex(u16 species)
{
    DexScreen_GetSetPokedexFlag(species, FLAG_SET_SEEN, TRUE);
    DexScreen_GetSetPokedexFlag(species, FLAG_SET_CAUGHT, TRUE);

    if (!IsNationalPokedexEnabled() && SpeciesToNationalPokedexNum(species) > KANTO_DEX_COUNT)
        return CreateTask(Task_DexScreen_RegisterNonKantoMonBeforeNationalDex, 0);

    DexScreen_LoadResources();
    gTasks[sPokedexScreenData->taskId].func = Task_DexScreen_RegisterMonToPokedex;
    DexScreen_LookUpCategoryBySpecies(species);

    return sPokedexScreenData->taskId;
}

static void Task_DexScreen_RegisterNonKantoMonBeforeNationalDex(u8 taskId)
{
    DestroyTask(taskId);
}

static void Task_DexScreen_RegisterMonToPokedex(u8 taskId)
{
    switch (sPokedexScreenData->state)
    {
    case 0:
        DexScreen_GetPageLimitsForCategory(sPokedexScreenData->category);
        if (sPokedexScreenData->pageNum < sPokedexScreenData->firstPageInCategory)
            sPokedexScreenData->pageNum = sPokedexScreenData->firstPageInCategory;
        sPokedexScreenData->state = 3;
        break;
    case 1:
        sub_8106014();
        DexScreen_DestroyCategoryPageMonIconAndInfoWindows();

        gMain.state = 0;
        sPokedexScreenData->state = 2;
        break;
    case 2:
        if (DoClosePokedex())
            DestroyTask(taskId);
        break;
    case 3:
        DexScreen_CreateCategoryListGfx(TRUE);
        PutWindowTilemap(0);
        PutWindowTilemap(1);

        CopyBgTilemapBufferToVram(3);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(0);

        DexScreen_CreateCategoryPageSelectionCursor(0xff);

        sPokedexScreenData->state = 4;
        break;
    case 4:
        gPaletteFade.bufferTransferDisabled = 0;
        BeginNormalPaletteFade(0xffffffff, 0, 16, 0, 0xffff);
        ShowBg(3);
        ShowBg(2);
        ShowBg(1);
        ShowBg(0);

        sPokedexScreenData->state = 5;
        break;
    case 5:
        gTasks[taskId].data[0] = 30;
        sPokedexScreenData->categoryPageCursorTaskId = ListMenuAddCursorObjectInternal(&gUnknown_84524C4, 0);
        sPokedexScreenData->state = 6;
        break;
    case 6:
        DexScreen_CreateCategoryPageSelectionCursor(sPokedexScreenData->categoryCursorPosInPage);
        DexScreen_UpdateCategoryPageCursorObject(sPokedexScreenData->categoryPageCursorTaskId, sPokedexScreenData->categoryCursorPosInPage, sPokedexScreenData->numMonsOnPage);

        if (gTasks[taskId].data[0])
            gTasks[taskId].data[0]--;
        else
        {
            ListMenuRemoveCursorObject(sPokedexScreenData->categoryPageCursorTaskId, 0);
            sPokedexScreenData->state = 7;
        }
        break;
    case 7:
        sPokedexScreenData->field_5A = sPokedexScreenData->pageSpecies[sPokedexScreenData->categoryCursorPosInPage];
        sPokedexScreenData->state = 8;
        break;
    case 8:
        DexScreen_DrawMonDexPage(TRUE);
        sPokedexScreenData->state = 9;
        break;
    case 9:
        sPokedexScreenData->data[0] = 0;
        sPokedexScreenData->data[1] = 0;
        sPokedexScreenData->state++;
    case 10:
        if (sPokedexScreenData->data[1] < 6)
        {
            if (sPokedexScreenData->data[0])
            {
                sub_8105594(0, sPokedexScreenData->data[1]);
                CopyBgTilemapBufferToVram(0);
                sPokedexScreenData->data[0] = 4;
                sPokedexScreenData->data[1]++;
            }
            else
                sPokedexScreenData->data[0]--;
        }
        else
        {
            FillBgTilemapBufferRect_Palette0(0, 0, 0, 2, 30, 16);
            CopyBgTilemapBufferToVram(3);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(0);

            PlayCry2(sPokedexScreenData->field_5A, 0, 125, 10);
            sPokedexScreenData->data[0] = 0;
            sPokedexScreenData->state = 11;
        }
        break;
    case 11:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
            sPokedexScreenData->state = 2;
        break;
    }
}

void DexScreen_PrintStringWithAlignment(const u8 * str, enum TextMode mode)
{
    u32 x;

    switch (mode)
    {
    case 0:
        x = 8;
        break;
    case 1:
        x = (u32)(240 - GetStringWidth(2, str, 0)) / 2;
        break;
    case 2:
    default:
        x = 232 - GetStringWidth(2, str, 0);
        break;
    }

    DexScreen_AddTextPrinterParameterized(0, 2, str, x, 2, 4);
}
