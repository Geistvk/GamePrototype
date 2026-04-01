#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <unordered_set>
#include <tuple>
#include <math.h>
#include <cmath>
#include <ctime>

#include <sstream>
#include <algorithm>

#include <chrono>
#include <format>

#include <string>
#include <iostream>

using namespace std;

struct Item;

struct ItemStack {
    const Item* item;
    int count;

    ItemStack() : item(nullptr), count(0) {}
    ItemStack(const Item* i, int c) : item(i), count(c) {}
};

struct Item {
    int id;
    bool placable;
    const char* name;
    const char* tooltip;
    bool hasInv;
    Color color;
};

enum class InventoryTab
{
    PLAYER,
    CONTAINER,
    CRAFTING, 
    PROFILE,
    SKILLS,
    CREATIVE
};

// ================= CRAFTING SYSTEM =================

struct RecipeIngredient
{
    const Item* item;
    int count;
};

struct Recipe
{
    const Item* result;
    int resultCount;
    std::vector<RecipeIngredient> ingredients;
};

std::vector<Recipe> allRecipes;

int selectedRecipeIndex = -1;
int craftingScroll = 0;


InventoryTab currentTab = InventoryTab::PLAYER;

std::vector<Item*> allItems; // enthält ALLE existierenden Items

// Globale Items
//const Item ItemId     = { id, placable,    "Name",    "Describtion",                             hasInv,   {RGBa Color}      };
const Item ITEM_NONE    = { 0,  true,        "None",    "Empty slot",                              false,    {0,0,0,0}         };
const Item ITEM_DIRT    = { 1,  true,        "Dirt",    "Basic soil block, good for farming",      false,    {160,120,80,255}  };
const Item ITEM_STONE   = { 2,  true,        "Stone",   "Hard block, used for crafting tools",     false,    {130,130,130,255} };
const Item ITEM_GRANITE = { 3,  true,        "Granite", "Hard block, can be found in the Caves.",  false,    {194,126,106,255} };
const Item ITEM_SAND    = { 4,  true,        "Sand",    "Loose block, falls under gravity",        false,    {194,178,128,255} };
const Item ITEM_GRASS   = { 5,  true,        "Grass",   "Soft block, grows on dirt",               false,    {124,252,0,255}   };
const Item ITEM_WOOD    = { 6,  true,        "Wood",    "Chopped from trees, useful for crafting", false,    {139,69,19,255}   };
const Item ITEM_PLANK   = { 7,  true,        "Planks",  "Can be used to craft various things",     false,    {181,101,29,255}  };
const Item ITEM_LEAVES  = { 8,  true,        "Leaves",  "Foliage from trees, can decay",           false,    {34,139,34,255}   };
const Item ITEM_Debug   = { 9,  true,        "Debug",   "A Block only to Debug",                   false,    {0,0,0,0}         };
const Item ITEM_CHEST   = { 10, true,        "Chest",   "Used to store Items",                     true,     {139,69,19,255}   };

// Globale DraggedItem
ItemStack draggedItem;
std::string hotbarItemName = ""; 
std::string lastHotbarItem = "None";

bool inventoryOpen = false;


void InitCreativeInventory()
{
    allItems.clear();

    allItems.push_back((Item*)&ITEM_DIRT);
    allItems.push_back((Item*)&ITEM_STONE);
    allItems.push_back((Item*)&ITEM_GRANITE);
    allItems.push_back((Item*)&ITEM_SAND);
    allItems.push_back((Item*)&ITEM_GRASS);
    allItems.push_back((Item*)&ITEM_WOOD);
    allItems.push_back((Item*)&ITEM_PLANK);
    allItems.push_back((Item*)&ITEM_LEAVES);
    allItems.push_back((Item*)&ITEM_Debug);
    allItems.push_back((Item*)&ITEM_CHEST);
}

void InitRecipes()
{
    allRecipes.clear();

    allRecipes.push_back({
        &ITEM_PLANK,
        1,
        {
            { &ITEM_WOOD, 4 }
        }
    });

    allRecipes.push_back({
        &ITEM_CHEST,
        1,
        {
            { &ITEM_WOOD, 8 }
        }
    });

    allRecipes.push_back({
        &ITEM_GRANITE,
        1,
        {
            { &ITEM_STONE, 1 }
        }
    });
}




#define CHEST_ROWS 3
#define CHEST_COLS 9

struct ContainerInventory
{
    Vector3 worldPos;
    ItemStack slots[CHEST_ROWS][CHEST_COLS];
};

std::vector<ContainerInventory> worldContainers;

ContainerInventory* currentContainer = nullptr;
const char* containerTitel = "";
bool containerOpen = false;


// Hotbar
const int HOTBAR_SIZE = 9;
ItemStack hotbar[HOTBAR_SIZE];

// Inventory
const int STACK_SIZE = 128;
const int INV_CELL = 60;
const int INV_SPACING = 8;
const int INV_ROWS = 3;
const int INV_COLS = 9;
ItemStack inventory[INV_ROWS][INV_COLS];

Color GetItemColor(const Item* item)
{
    if (item && item != nullptr) return item->color;
    return {0,0,0,0};
}




struct Block
{
    Vector3 pos;
    BoundingBox box;
    const Item* item;
};

// Hash-Funktion für Vector3 / integer positions
struct Vec3Hash {
    std::size_t operator()(const std::tuple<int,int,int>& key) const {
        int hx = std::get<0>(key);
        int hy = std::get<1>(key);
        int hz = std::get<2>(key);
        return std::hash<int>()(hx) ^ (std::hash<int>()(hy) << 1) ^ (std::hash<int>()(hz) << 2);
    }
};












// ----------------------------
// Skill Struktur
// ----------------------------
struct Skill
{
    std::string name;
    int level;
    int maxLevel;
    int cost;
    bool unlocked;
    Rectangle bounds; // wird dynamisch gesetzt
};


// ----------------------------
// Beispiel Initialisierung
// ----------------------------
std::vector<Skill> CreateSkills(int startX, int startY)
{
    std::vector<Skill> skills;

    int width = 220;
    int height = 70;
    int spacing = 20;

    skills.push_back({"Strength", 0, 5, 1, true,
        {(float)startX, (float)startY, (float)width, (float)height}});

    skills.push_back({"Agility", 0, 5, 1, false,
        {(float)startX, (float)startY + height + spacing, (float)width, (float)height}});

    skills.push_back({"Magic", 0, 5, 2, false,
        {(float)startX, (float)startY + (height + spacing) * 2, (float)width, (float)height}});

    return skills;
}

// ----------------------------
// Skill GUI
// ----------------------------
void DrawSkillGUI(std::vector<Skill>& skills,
                  int& skillPoints,
                  int bgX, int bgY,
                  int invWidth, int invHeight)
{
    DrawText("SKILLS", bgX + 20, bgY + 10, 20, RAYWHITE);

    // Skillpoints Anzeige
    Color pointsColor = skillPoints > 0 ? GOLD : RED;

    DrawText(TextFormat("Skillpoints: %d", skillPoints),
             bgX + invWidth - 220,
             bgY + 10,
             20,
             pointsColor);

    int contentX = bgX + 40;
    int contentY = bgY + 50;
    int contentWidth = invWidth - 80;
    int contentHeight = invHeight - 160;

    int cardWidth = 230;
    int cardHeight = 90;
    int spacing = 20;

    int columns = contentWidth / (cardWidth + spacing);
    if (columns < 1) columns = 1;

    int rows = (skills.size() + columns - 1) / columns;
    int totalContentHeight = rows * (cardHeight + spacing);

    static float scrollOffset = 0;

    bool mouseInside =
        CheckCollisionPointRec(GetMousePosition(),
            {(float)contentX, (float)contentY,
             (float)contentWidth, (float)contentHeight});

    if (mouseInside)
        scrollOffset -= GetMouseWheelMove() * 30;

    if (scrollOffset < 0) scrollOffset = 0;
    if (scrollOffset > totalContentHeight - contentHeight)
        scrollOffset = totalContentHeight - contentHeight;

    if (totalContentHeight < contentHeight)
        scrollOffset = 0;

    BeginScissorMode(contentX, contentY, contentWidth, contentHeight);

    for (int i = 0; (int) i < (int)skills.size(); i++)
    {
        Skill& s = skills[i];

        int row = i / columns;
        int col = i % columns;

        float x = contentX + col * (cardWidth + spacing);
        float y = contentY + row * (cardHeight + spacing) - scrollOffset;

        s.bounds = {x, y, (float)cardWidth, (float)cardHeight};

        bool hovered = CheckCollisionPointRec(GetMousePosition(), s.bounds);

        bool canAffordUnlock = skillPoints >= 1;
        bool canAffordLevel  = skillPoints >= s.cost;
        bool isMaxed = s.level >= s.maxLevel;

        // Hintergrundfarbe
        Color bgColor = s.unlocked ? Color{70, 70, 85, 255}
                                   : Color{50, 50, 50, 255};

        if (hovered) bgColor = Color{100, 100, 120, 255};
        if (isMaxed) bgColor = Color{40, 90, 40, 255};

        DrawRectangleRec(s.bounds, bgColor);
        DrawRectangleLinesEx(s.bounds, 2, BLACK);

        // Name
        DrawText(s.name.c_str(), x + 10, y + 8, 20, WHITE);

        // Level
        DrawText(TextFormat("Level: %d/%d", s.level, s.maxLevel),
                 x + 10, y + 35, 18, YELLOW);

        // Progressbar
        float progress = (float)s.level / s.maxLevel;

        DrawRectangle(x + 10, y + 60, 160 * progress, 12, GREEN);
        DrawRectangleLines(x + 10, y + 60, 160, 12, RAYWHITE);

        // ----------------------------
        // Kostenanzeige
        // ----------------------------
        std::string costText;
        Color costColor = WHITE;

        if (!s.unlocked)
        {
            costText = "Unlock: 1 SP";
            costColor = canAffordUnlock ? GREEN : RED;
        }
        else if (!isMaxed)
        {
            costText = TextFormat("Cost: %d SP", s.cost);
            costColor = canAffordLevel ? GREEN : RED;
        }
        else
        {
            costText = "MAX LEVEL";
            costColor = DARKGREEN;
        }

        DrawText(costText.c_str(),
                 x + cardWidth - 120,
                 y + 35,
                 18,
                 costColor);

        // ----------------------------
        // Klick Logik
        // ----------------------------
        if (hovered && mouseInside && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (!s.unlocked)
            {
                if (canAffordUnlock)
                {
                    s.unlocked = true;
                    skillPoints--;
                }
            }
            else if (!isMaxed)
            {
                if (canAffordLevel)
                {
                    s.level++;
                    skillPoints -= s.cost;
                }
            }
        }
    }

    EndScissorMode();

    // ----------------------------
    // Scrollbar
    // ----------------------------
    if (totalContentHeight > contentHeight)
    {
        float scrollbarHeight =
            (float)contentHeight / totalContentHeight * contentHeight;

        float scrollbarY =
            contentY + (scrollOffset / totalContentHeight) * contentHeight;

        DrawRectangle(contentX + contentWidth - 6,
                      contentY,
                      6,
                      contentHeight,
                      DARKGRAY);

        DrawRectangle(contentX + contentWidth - 6,
                      scrollbarY,
                      6,
                      scrollbarHeight,
                      LIGHTGRAY);
    }
}














void AddItemToInventory(const Item* item, int count)
{
    // 1️⃣ Hotbar bestehende Stacks füllen
    for (int i = 0; i < HOTBAR_SIZE; i++)
    {
        if (hotbar[i].item == item && hotbar[i].count < STACK_SIZE)
        {
            int space = STACK_SIZE - hotbar[i].count;
            int toAdd = (count < space) ? count : space;
            hotbar[i].count += toAdd;
            count -= toAdd;
            if (count <= 0) return;
        }
    }

    // 2️⃣ Inventory bestehende Stacks füllen
    for (int r = 0; r < INV_ROWS; r++)
    {
        for (int c = 0; c < INV_COLS; c++)
        {
            if (inventory[r][c].item == item && inventory[r][c].count < STACK_SIZE)
            {
                int space = STACK_SIZE - inventory[r][c].count;
                int toAdd = (count < space) ? count : space;
                inventory[r][c].count += toAdd;
                count -= toAdd;
                if (count <= 0) return;
            }
        }
    }

    // 3️⃣ Leere Hotbar-Slots
    for (int i = 0; i < HOTBAR_SIZE; i++)
    {
        if (hotbar[i].item == nullptr)
        {
            hotbar[i].item = item;
            hotbar[i].count = count;
            return;
        }
    }

    // 4️⃣ Leere Inventory-Slots
    for (int r = 0; r < INV_ROWS; r++)
    {
        for (int c = 0; c < INV_COLS; c++)
        {
            if (inventory[r][c].item == nullptr)
            {
                inventory[r][c].item = item;
                inventory[r][c].count = count;
                return;
            }
        }
    }
}


// Hilfsfunktionen
void TryAddToInventory(ItemStack& temp) {
    int cols = 0;
    int rows = 0;
    if (currentTab == InventoryTab::PLAYER) {
        cols = INV_COLS;
        rows = INV_ROWS;
    } else if (currentTab == InventoryTab::CONTAINER) {
        cols = CHEST_COLS;
        rows = CHEST_ROWS;
    }
    
    // 1️⃣ Fülle bestehende Stapel
    for (int r = 0; r < rows && temp.count > 0; r++)
    {
        for (int c = 0; c < cols && temp.count > 0; c++)
        {
            ItemStack& slot = inventory[r][c];
            if (slot.item == temp.item && slot.count < STACK_SIZE)
            {
                int space = STACK_SIZE - slot.count;
                int toAdd = (temp.count < space) ? temp.count : space;
                slot.count += toAdd;
                temp.count -= toAdd;
            }
        }
    }

    // 2️⃣ Leere Slots
    for (int r = 0; r < rows && temp.count > 0; r++)
    {
        for (int c = 0; c < cols && temp.count > 0; c++)
        {
            ItemStack& slot = inventory[r][c];
            if (slot.item == nullptr)
            {
                slot.item = temp.item;
                slot.count = temp.count;
                temp.count = 0;
                break;
            }
        }
    }

    // 3️⃣ Fallback
    if (temp.count > 0)
        AddItemToInventory(temp.item, temp.count);
}

void TryAddToHotbar(ItemStack& temp) {
    // 1️⃣ Fülle bestehende Stapel
    for (int i = 0; i < HOTBAR_SIZE && temp.count > 0; i++)
    {
        ItemStack& slot = hotbar[i];
        if (slot.item == temp.item && slot.count < STACK_SIZE)
        {
            int space = STACK_SIZE - slot.count;
            int toAdd = (temp.count < space) ? temp.count : space;
            slot.count += toAdd;
            temp.count -= toAdd;
        }
    }

    // 2️⃣ Leere Slots
    for (int i = 0; i < HOTBAR_SIZE && temp.count > 0; i++)
    {
        ItemStack& slot = hotbar[i];
        if (slot.item == nullptr)
        {
            slot.item = temp.item;
            slot.count = temp.count;
            temp.count = 0;
            break;
        }
    }

    // 3️⃣ Fallback
    if (temp.count > 0)
        AddItemToInventory(temp.item, temp.count);
}

void TryAddToContainer(ItemStack& temp) {
    // 1️⃣ Fülle bestehende Stapel
    for (int r = 0; r < INV_ROWS && temp.count > 0; r++)
    {
        for (int c = 0; c < INV_COLS && temp.count > 0; c++)
        {
            ItemStack& slot = currentContainer->slots[r][c];
            if (slot.item == temp.item && slot.count < STACK_SIZE)
            {
                int space = STACK_SIZE - slot.count;
                int toAdd = (temp.count < space) ? temp.count : space;
                slot.count += toAdd;
                temp.count -= toAdd;
            }
        }
    }

    // 2️⃣ Leere Slots
    for (int r = 0; r < INV_ROWS && temp.count > 0; r++)
    {
        for (int c = 0; c < INV_COLS && temp.count > 0; c++)
        {
            ItemStack& slot = currentContainer->slots[r][c];
            if (slot.item == nullptr)
            {
                slot.item = temp.item;
                slot.count = temp.count;
                temp.count = 0;
                break;
            }
        }
    }

    // 3️⃣ Fallback
    if (temp.count > 0)
        TryAddToHotbar(temp);
}


int CountItemInInventory(const Item* item)
{
    int total = 0;

    for (int i = 0; i < HOTBAR_SIZE; i++)
        if (hotbar[i].item == item)
            total += hotbar[i].count;

    for (int r = 0; r < INV_ROWS; r++)
        for (int c = 0; c < INV_COLS; c++)
            if (inventory[r][c].item == item)
                total += inventory[r][c].count;

    return total;
}

int GetMaxCraftAmount(const Recipe& recipe)
{
    int maxCraft = INT_MAX;

    for (auto& ing : recipe.ingredients)
    {
        int available = CountItemInInventory(ing.item);
        int possible = available / ing.count;

        if (possible < maxCraft)
            maxCraft = possible;
    }

    return maxCraft;
}

void RemoveItems(const Item* item, int count)
{
    // Hotbar
    for (int i = 0; i < HOTBAR_SIZE && count > 0; i++)
    {
        if (hotbar[i].item == item)
        {
            int take = std::min(count, hotbar[i].count);
            hotbar[i].count -= take;
            count -= take;

            if (hotbar[i].count <= 0)
                hotbar[i] = ItemStack(nullptr, 0);
        }
    }

    // Inventory
    for (int r = 0; r < INV_ROWS && count > 0; r++)
    {
        for (int c = 0; c < INV_COLS && count > 0; c++)
        {
            if (inventory[r][c].item == item)
            {
                int take = std::min(count, inventory[r][c].count);
                inventory[r][c].count -= take;
                count -= take;

                if (inventory[r][c].count <= 0)
                    inventory[r][c] = ItemStack(nullptr, 0);
            }
        }
    }
}

void CraftRecipe(const Recipe& recipe, int amount)
{
    for (auto& ing : recipe.ingredients)
        RemoveItems(ing.item, ing.count * amount);

    AddItemToInventory(recipe.result, recipe.resultCount * amount);
}














void openInv(bool hasInv, char* title, Vector3 pos)
{
    if (!hasInv)
        return;

    // =========================================
    // FALL 1: Container ist bereits offen → schließen
    // =========================================
    if (containerOpen)
    {
        containerOpen   = false;
        inventoryOpen   = false;
        currentTab      = InventoryTab::PLAYER;
        currentContainer = nullptr;

        DisableCursor();
        return;
    }

    // =========================================
    // FALL 2: Container öffnen
    // =========================================

    containerTitel = title;

    // Prüfen ob Container schon existiert
    for (auto& c : worldContainers)
    {
        if (c.worldPos.x == pos.x &&
            c.worldPos.y == pos.y &&
            c.worldPos.z == pos.z)
        {
            currentContainer = &c;

            containerOpen = true;
            inventoryOpen = true;
            currentTab    = InventoryTab::CONTAINER;

            EnableCursor();
            return;
        }
    }

    // Falls nicht existiert → neu erstellen
    ContainerInventory newContainer = {};
    newContainer.worldPos = pos;

    worldContainers.push_back(newContainer);
    currentContainer = &worldContainers.back();

    containerOpen = true;
    inventoryOpen = true;
    currentTab    = InventoryTab::CONTAINER;

    EnableCursor();
}

void eraseContainer(Vector3 pos)
{
    Vector3 containerPos = pos;

    cout << "Erasing Container" << endl;

    // 1️⃣ Container suchen
    auto it = std::find_if(
        worldContainers.begin(),
        worldContainers.end(),
        [&](const ContainerInventory& c)
        {
            return c.worldPos.x == containerPos.x &&
                   c.worldPos.y == containerPos.y &&
                   c.worldPos.z == containerPos.z;
        }
    );

    if (it == worldContainers.end())
        return;

    // 2️⃣ Alle Items ins Player-Inventar verschieben
    for (int r = 0; r < CHEST_ROWS; r++)
    {
        for (int c = 0; c < CHEST_COLS; c++)
        {
            ItemStack& stack = it->slots[r][c];

            if (stack.item != nullptr && stack.count > 0)
            {
                cout << "Adding item to inventory: " << stack.item->name << " x" << stack.count << endl;

                AddItemToInventory(stack.item, stack.count);
            }
        }
    }

    // 3️⃣ UI schließen, falls offen
    if (currentContainer == &(*it))
    {
        currentContainer = nullptr;
        containerOpen = false;
        inventoryOpen = false;
        currentTab = InventoryTab::PLAYER;
        DisableCursor();
    }

    // 4️⃣ Container aus der Welt löschen
    worldContainers.erase(it);
}




std::unordered_set<std::tuple<int,int,int>, Vec3Hash> blockPositions;

std::vector<Block> blocks;



BoundingBox MakeBox(Vector3 p)
{
    return {
        { p.x - 0.5f, p.y - 0.5f, p.z - 0.5f },
        { p.x + 0.5f, p.y + 0.5f, p.z + 0.5f }
    };
}

struct Vec3Int {
    int x, y, z;
    bool operator==(const Vec3Int& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Vec3IntHash {
    std::size_t operator()(const Vec3Int& v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) << 2);
    }
};

inline bool BlockExists(Vector3 pos) {
    return blockPositions.find({(int) pos.x, (int) pos.y, (int) pos.z}) != blockPositions.end();
}



void DrawBlockShaded(Block b, bool isHit)
{
    Color top, side, bottom;

    top = side = bottom = GetItemColor(b.item);

    if (lastHotbarItem == "Debug" && b.item->name == (char*) "Debug") top = side = bottom = {255,3,62,125};

    Vector3 pos = b.pos;
    float half = 0.5f;

    // TOP
    if (!BlockExists(Vector3Add(pos, {0, 1, 0}))) {
        Vector3 topPos = Vector3Add(pos, {0, half, 0});
        DrawCube(topPos, 1.0f, 0.01f, 1.0f, top);
    }

    // BOTTOM
    if (!BlockExists(Vector3Add(pos, {0, -1, 0}))) {
        Vector3 bottomPos = Vector3Add(pos, {0, -half, 0});
        DrawCube(bottomPos, 1.0f, 0.01f, 1.0f, bottom);
    }

    // FRONT
    if (!BlockExists(Vector3Add(pos, {0, 0, 1}))) {
        Vector3 frontPos = Vector3Add(pos, {0, 0, half});
        DrawCube(frontPos, 1.0f, 1.0f, 0.01f, side);
    }

    // BACK
    if (!BlockExists(Vector3Add(pos, {0, 0, -1}))) {
        Vector3 backPos = Vector3Add(pos, {0, 0, -half});
        DrawCube(backPos, 1.0f, 1.0f, 0.01f, side);
    }

    // RIGHT
    if (!BlockExists(Vector3Add(pos, {1, 0, 0}))) {
        Vector3 rightPos = Vector3Add(pos, {half, 0, 0});
        DrawCube(rightPos, 0.01f, 1.0f, 1.0f, side);
    }

    // LEFT
    if (!BlockExists(Vector3Add(pos, {-1, 0, 0}))) {
        Vector3 leftPos = Vector3Add(pos, {-half, 0, 0});
        DrawCube(leftPos, 0.01f, 1.0f, 1.0f, side);
    }


    float wireScale = 1.01f;
    if (isHit) DrawCubeWires(pos, wireScale, wireScale, wireScale, DARKBROWN);
}


bool HasBlockBelow(Vector3 pos, float playerRadius, float playerHeight, const std::vector<Block>& blocks)
{
    float checkY = pos.y - playerHeight - 0.05f;

    for (const auto& b : blocks)
    {
        if (pos.x + playerRadius > b.box.min.x &&
            pos.x - playerRadius < b.box.max.x &&
            pos.z + playerRadius > b.box.min.z &&
            pos.z - playerRadius < b.box.max.z &&
            checkY <= b.box.max.y &&
            checkY >= b.box.max.y - 0.3f)
        {
            return true;
        }
    }
    return false;
}

bool IsBlockInView(const Vector3& blockPos, const Vector3& playerPos, const Vector3& viewDir, float maxViewDistance, float fovAngle)
{
    // Richtung vom Spieler zum Block
    Vector3 toBlock = { 
        blockPos.x - playerPos.x,
        blockPos.y - playerPos.y,
        blockPos.z - playerPos.z
    };

    // Entfernung berechnen
    float distance = sqrt(toBlock.x*toBlock.x + toBlock.y*toBlock.y + toBlock.z*toBlock.z);
    if (distance > maxViewDistance)
        return false;

    // Normalisieren
    toBlock.x /= distance;
    toBlock.y /= distance;
    toBlock.z /= distance;

    // Skalarprodukt (Dot) zwischen Blickrichtung und Block
    float dot = viewDir.x*toBlock.x + viewDir.y*toBlock.y + viewDir.z*toBlock.z;

    float angle = acos(dot) * (180.0f / 3.14159265f); // Radiant → Grad

    return angle <= fovAngle * 0.5f;
}


// ---------------- Hilfsfunktionen ----------------
void MergeStacks(ItemStack& target, ItemStack& source)
{
    if (target.item != source.item || target.item == nullptr) return;

    int space = STACK_SIZE - target.count;
    int toAdd = (source.count < space) ? source.count : space;

    target.count += toAdd;
    source.count -= toAdd;

    if (source.count <= 0)
    {
        source.count = 0;
        source.item = nullptr;
    }
}














#define LINE_HEIGHT 25
#define TITLE_HEIGHT 30
#define PADDING 10
#define PANEL_WIDTH 200
#define PANEL_SPACING 20

enum class PanelAlignment { 
    LEFT, 
    LEFTMIDDLE,
    CENTER, 
    RIGHTMIDDLE,
    RIGHT 
};

struct HUDPanel {
    std::string title;
    std::vector<std::string> lines;
    Color color;
    PanelAlignment alignment;
};

// Funktion zum Zeichnen eines einzelnen Panels
void DrawHUDPanel(const HUDPanel& panel, float xPos, float yPos) {
    float rectRoundness = 0.2f;
    int rectSegments = 8;

    float panelHeight = TITLE_HEIGHT + panel.lines.size() * LINE_HEIGHT + 2 * PADDING;

    // Schatten
    DrawRectangleRounded(
        Rectangle{xPos + 3, yPos + 3, PANEL_WIDTH, panelHeight},
        rectRoundness, rectSegments, Fade(BLACK, 0.4f)
    );

    // Rechteck
    DrawRectangleRounded(
        Rectangle{xPos, yPos, PANEL_WIDTH, panelHeight},
        rectRoundness, rectSegments, Fade(panel.color, 0.8f)
    );

    // Titel
    DrawText(panel.title.c_str(), xPos + PADDING, yPos + PADDING, 20, WHITE);

    // Textzeilen
    for (size_t i = 0; i < panel.lines.size(); i++) {
        DrawText(panel.lines[i].c_str(), xPos + PADDING, yPos + PADDING + TITLE_HEIGHT + i * LINE_HEIGHT, 18, WHITE);
    }
}

// Funktion zum Zeichnen aller Panels (stapelweise)
void DrawHUD(const std::vector<HUDPanel>& panels) {
    float leftY = PANEL_SPACING;
    float leftMiddleY = PANEL_SPACING;
    float centerY = PANEL_SPACING;
    float rightMiddleY = PANEL_SPACING;
    float rightY = PANEL_SPACING;
    int screenWidth = GetScreenWidth();

    for (const auto& panel : panels) {
        float panelHeight = TITLE_HEIGHT + panel.lines.size() * LINE_HEIGHT + 2 * PADDING;
        float xPos = 0;

        switch(panel.alignment) {
            case PanelAlignment::LEFT:
                xPos = PANEL_SPACING;
                DrawHUDPanel(panel, xPos, leftY);
                leftY += panelHeight + PANEL_SPACING;
                break;
            case PanelAlignment::LEFTMIDDLE: 
                xPos = PANEL_SPACING * 2 + PANEL_WIDTH;
                DrawHUDPanel(panel, xPos, leftMiddleY);
                leftMiddleY += panelHeight + PANEL_SPACING;
                break;
            case PanelAlignment::CENTER:
                xPos = screenWidth / 2 - PANEL_WIDTH / 2;
                DrawHUDPanel(panel, xPos, centerY);
                centerY += panelHeight + PANEL_SPACING;
                break;
            case PanelAlignment::RIGHTMIDDLE:
                xPos = screenWidth - PANEL_WIDTH - PANEL_SPACING - PANEL_WIDTH - PANEL_SPACING;
                DrawHUDPanel(panel, xPos, rightMiddleY);
                rightMiddleY += panelHeight + PANEL_SPACING;
                break;
            case PanelAlignment::RIGHT:
                xPos = screenWidth - PANEL_WIDTH - PANEL_SPACING;
                DrawHUDPanel(panel, xPos, rightY);
                rightY += panelHeight + PANEL_SPACING;
                break;
        }
    }
}






















// Globale Variable, um das Split-Fenster zu steuern
bool showSplitWindow = false;
int splitAmount = 1;
ItemStack* splitTarget = nullptr;
std::string inputText = "";


// Funktion zum Zeichnen des Split-Fensters
void DrawSplitWindow(int screenWidth, int screenHeight) {
    int winWidth = 300;
    int winHeight = 150;
    Rectangle winRect = { (float)(screenWidth - winWidth)/2, (float)(screenHeight - winHeight)/2, (float)winWidth, (float)winHeight };

    int btnWidth = 80;
    int btnHeight = 30;

    DrawRectangleRec(winRect, LIGHTGRAY);
    DrawRectangleLinesEx(winRect, 2, DARKGRAY);

    DrawText("Enter Amount to split:", winRect.x + 10, winRect.y + 10, 20, BLACK);

    // Eingabefeld
    Rectangle inputRect = { winRect.x + 10, winRect.y + 50, (float) winWidth - 20, 30 };
    DrawRectangleRec(inputRect, WHITE);
    DrawRectangleLinesEx(inputRect, 2, DARKGRAY);
    DrawText(inputText.c_str(), inputRect.x + 5, inputRect.y + 5, 20, BLACK);

    // Buttons
    Rectangle splitBtn = { winRect.x + 60, winRect.y + 90, (float)btnWidth, (float)btnHeight };
    Rectangle cancelBtn = { winRect.x + 150, winRect.y + 90, (float)btnWidth, (float)btnHeight };
    DrawRectangleRec(splitBtn, SKYBLUE);
    DrawRectangleRec(cancelBtn, LIGHTGRAY);
    DrawRectangleLinesEx(splitBtn, 2, DARKBLUE);
    DrawRectangleLinesEx(cancelBtn, 2, DARKGRAY);
    DrawText("Split", splitBtn.x + 10, splitBtn.y + 5, 20, BLACK);
    DrawText("Cancel", cancelBtn.x + 5, cancelBtn.y + 5, 20, BLACK);

    // Mausinteraktionen
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse, splitBtn)) {
            if (splitTarget && !inputText.empty()) {
                const Item* tmp = splitTarget->item;
                int amount = std::stoi(inputText);

                // nur gültige Menge splitten
                if (amount > 0 && amount <= splitTarget->count) {
                    // draggedItem global setzen
                    draggedItem = ItemStack(tmp, amount);

                    // Menge vom Slot abziehen
                    splitTarget->count -= amount;

                    // Wenn Slot leer, auf nullptr setzen
                    if (splitTarget->count <= 0) {
                        *splitTarget = ItemStack(nullptr, 0);
                    }
                }
            }
            // Fenster schließen
            showSplitWindow = false;
            inputText = "";
            splitTarget = nullptr;
        } else if (CheckCollisionPointRec(mouse, cancelBtn)) {
            // Fenster schließen
            showSplitWindow = false;
            inputText = "";
            splitTarget = nullptr;
        }
    }

    // Eingabe für Zahl
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= '0' && key <= '9') && inputText.length() < 5) {
            inputText.push_back((char)key);
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty()) {
        inputText.pop_back();
    }
}













int main()
{
    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);

    InitWindow(screenWidth, screenHeight, "Minecraft Clone");
    ToggleFullscreen();

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    DisableCursor();



    // ---------------- PLAYER ----------------
    Vector3 playerPos = { 0.0f, 20.0f, 4.0f };
    Vector3 velocity = { 0 };
    const float playerHeight = 1.8f;
    const float playerRadius = 0.3f;
    const int playerReach = 4;
    const float speed = 5.0f;
    const float gravity = -25.0f;
    const float jumpForce = 9.0f;
    bool onGround = false;
    bool isSneaking = false;
    bool hitSomething = false;
    bool isDraggingCraftScroll = false;

    bool playerCreative = false;
    bool canCraft = true;
    bool playerInv = true;
    bool playerProfile = true;
    bool hasSkills = true;

    int blocksDestroyed = 0;
    int blocksPlaced = 0;
    int totalBlocks = 0;

    int creativeScrollRow = 0;

    const char* cardinalDirection;
    float renderDistance = 16.0f; 
    const float maxViewDistance = renderDistance; // z. B. 16 Blöcke
    const float fovAngle = 120.0f;

    float cameraSneakOffset = 0.0f;
    const float sneakCamTarget = 0.35f;
    const float sneakCamSpeed  = 8.0f;

    int selectedHotbarSlot = 0;

    bool paused = false;
    int hitIndex = -1;
    int hitDist = 0;
    Block hitBlock;

    bool showDebugOverlay = false;
    bool hWasDown = false;

    // Globale Variablen (außerhalb des Loops)
    float hotbarItemTimer = 0.0f;          // Zeit seit letztem Scroll
    float hotbarItemShowTime = 1.0f;       // Dauer der Anzeige beim Scrollen
    float hotbarItemHideDelay = 0.0f;      // Dauer bis Tooltip verschwindet, wenn nicht scrollt
    bool hotbarItemJustScrolled = false;   // Flag für sofortiges Anzeigen beim Scrollen


    const int previewTexSize = 400; // direkt 5× größer
    RenderTexture2D previewTex = LoadRenderTexture(previewTexSize, previewTexSize);
    float rotation = 0.0f;

    float yaw = 0, pitch = 0;

    InitCreativeInventory();
    InitRecipes();

    int skillPoints = 5;

    std::vector<Skill> skills =
    {
        {"Strength", 0, 5, 1, true},
        {"Agility", 0, 5, 1, false},
        {"Magic", 0, 5, 2, false},
        {"Defense", 0, 3, 2, false},
        {"Defense", 0, 3, 2, false},
        {"Defense", 0, 3, 2, false},
        {"Defense", 0, 3, 2, false}
    };



    // ---------------- CAMERA ----------------
    Camera3D camera = { 0 };
    camera.position = playerPos;
    camera.up = { 0, 1.5f, 0 };
    camera.fovy = 60;
    camera.projection = CAMERA_PERSPECTIVE;

    // ---------------- BLOCKS ----------------
    const int floorSize = 30;

    // Bodenblöcke
    for (int x = -floorSize; x <= floorSize; x++)
        for (int y = 0; y <= 5; y++)
            for (int z = -floorSize; z <= floorSize; z++)
            {
                Block b;
                b.item = &ITEM_STONE;
                b.pos = { (float)x, (float)y, (float)z };
                b.box = MakeBox(b.pos);
                blocks.push_back(b);
                blockPositions.emplace(
                    (int)b.pos.x, 
                    (int)b.pos.y, 
                    (int)b.pos.z
                );
                totalBlocks++;
            }
    
    
    for(int i = 0; i < HOTBAR_SIZE; i++) hotbar[i] = ItemStack();
    for(int r = 0; r < INV_ROWS; r++)
        for(int c = 0; c < INV_COLS; c++)
            inventory[r][c] = ItemStack();

    // ---- HOTBAR ----
    hotbar[0] = ItemStack(&ITEM_DIRT, STACK_SIZE/2);
    hotbar[1] = ItemStack(&ITEM_STONE,STACK_SIZE/2);
    hotbar[2] = ItemStack(&ITEM_SAND,STACK_SIZE/2);
    hotbar[3] = ItemStack(&ITEM_GRASS, STACK_SIZE/2);
    hotbar[4] = ItemStack(&ITEM_WOOD, STACK_SIZE/2);
    hotbar[5] = ItemStack(&ITEM_LEAVES, STACK_SIZE/2);
    hotbar[6] = ItemStack(&ITEM_Debug, STACK_SIZE);

    lastHotbarItem = hotbar[0].item->name;



    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // ---------------- ESC / PAUSE ----------------
        if (IsKeyPressed(KEY_ESCAPE))
        {
            paused = !paused;
            paused ? EnableCursor() : DisableCursor();
        }

        if (!paused)
        {
            //tick Rotation 
            rotation += 90 * GetFrameTime(); 

            // -------- Mouse Look --------
            if (!inventoryOpen)
            {
                Vector2 mouse = GetMouseDelta();
                yaw   -= mouse.x * 0.003f;
                pitch -= mouse.y * 0.003f;
                pitch = Clamp(pitch, -1.5f, 1.5f);
            }

            Vector3 forward = {
                cosf(pitch) * sinf(yaw),
                sinf(pitch),
                cosf(pitch) * cosf(yaw)
            };

            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, { 0,1,0 }));

            // --- Himmelsrichtung berechnen ---

            // Yaw von Radiant in Grad umwandeln
            float yawDegrees = yaw * (180.0f / PI);

            // sauber auf 0–360 normalisieren
            yawDegrees = fmodf(yawDegrees, 360.0f);
            if (yawDegrees < 0) yawDegrees += 360.0f;


            // 8 Himmelsrichtungen
            const char* directions[8] = {
                "N", "NO", "O", "SO",
                "S", "SW", "W", "NW"
            };

            // Index berechnen (360° / 8 = 45° pro Richtung)
            int index = (int)((yawDegrees + 22.5f) / 45.0f) % 8;

            cardinalDirection = directions[index];


            // Hotbar Update
            for (int i = 0; i < HOTBAR_SIZE; i++) {
                if (IsKeyPressed(KEY_ONE + i) && !inventoryOpen)
                    selectedHotbarSlot = i;
            }


            // -------- Gravity & Ground Check --------
            // Normale Gravity
            velocity.y += gravity * dt;

            // --- Position aktualisieren ---
            playerPos.y += velocity.y * dt;








            // Ground-Check
            onGround = false;
            float highestTop = -1000.0f;
            for (auto& b : blocks) {
                float top = b.box.max.y;
                if (playerPos.x + playerRadius > b.box.min.x &&
                    playerPos.x - playerRadius < b.box.max.x &&
                    playerPos.z + playerRadius > b.box.min.z &&
                    playerPos.z - playerRadius < b.box.max.z &&
                    playerPos.y - playerHeight >= top - 0.25f &&
                    playerPos.y - playerHeight <= top + 0.1f)
                {
                    if (top > highestTop) highestTop = top;
                }
            }

            if (highestTop > -1000.0f) {
                playerPos.y = highestTop + playerHeight;
                velocity.y = 0;
                onGround = true;
            }

            // -------- Horizontal Movement Input --------
            Vector3 move = {0};
            if (IsKeyDown(KEY_W)) move = Vector3Add(move, forward);
            if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, forward);
            if (IsKeyDown(KEY_A)) move = Vector3Subtract(move, right);
            if (IsKeyDown(KEY_D)) move = Vector3Add(move, right);
            move.y = 0;

            float currentSpeed = speed;
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                currentSpeed *= 1.5f; // Shift läuft 50% schneller
            }

            isSneaking = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
            if (isSneaking && !inventoryOpen && !containerOpen) {
                currentSpeed *= 0.3f;
            }

            float targetOffset = isSneaking ? sneakCamTarget : 0.0f;
            cameraSneakOffset += (targetOffset - cameraSneakOffset) * sneakCamSpeed * dt;


            if (Vector3Length(move) > 0)
                move = Vector3Scale(Vector3Normalize(move), currentSpeed * dt);

            // -------- Horizontal Collision & Sliding (Minecraft-Style) --------
            Vector3 nextX = playerPos; nextX.x += move.x;
            Vector3 nextZ = playerPos; nextZ.z += move.z;
            bool blockedX = false, blockedZ = false;

            float epsilon = 0.01f; // kleine Höhe über Boden, um Block darunter zu ignorieren

            for (auto& b : blocks) {
                BoundingBox bx = {
                    {nextX.x - playerRadius, playerPos.y - playerHeight + epsilon, nextX.z - playerRadius},
                    {nextX.x + playerRadius, playerPos.y, nextX.z + playerRadius}
                };
                BoundingBox bz = {
                    {nextZ.x - playerRadius, playerPos.y - playerHeight + epsilon, nextZ.z - playerRadius},
                    {nextZ.x + playerRadius, playerPos.y, nextZ.z + playerRadius}
                };

                if (CheckCollisionBoxes(bx, b.box)) blockedX = true;
                if (CheckCollisionBoxes(bz, b.box)) blockedZ = true;
            }

            // Sliding
            Vector3 testX = playerPos; testX.x += move.x;
            Vector3 testZ = playerPos; testZ.z += move.z;

            bool allowX = !blockedX;
            bool allowZ = !blockedZ;

            if (isSneaking && onGround)
            {
                if (!HasBlockBelow(testX, playerRadius, playerHeight, blocks))
                    allowX = false;

                if (!HasBlockBelow(testZ, playerRadius, playerHeight, blocks))
                    allowZ = false;
            }

            if (allowX) playerPos.x += move.x;
            if (allowZ) playerPos.z += move.z;





            // -------- Jump --------
            if (IsKeyPressed(KEY_SPACE) && onGround)
            {
                velocity.y = jumpForce;
                onGround = false;
            }

            // -------- Camera Sync --------
            Vector3 camPos = playerPos;
            camPos.y -= cameraSneakOffset;

            camera.position = camPos;
            camera.target   = Vector3Add(camPos, forward);


            // -------- Raycast Block Interaction --------
            Ray ray = { camera.position, Vector3Normalize(Vector3Subtract(camera.target, camera.position)) };
            float closest = 1000.0f;
            RayCollision hitInfo = { 0 };
            hitSomething = false;

            float reach = playerReach;
            for (auto& b : blocks) {
                if (fabs(b.pos.x - playerPos.x) > reach ||
                    fabs(b.pos.y - playerPos.y) > reach ||
                    fabs(b.pos.z - playerPos.z) > reach) continue;

                RayCollision hit = GetRayCollisionBox(ray, b.box);
                if (hit.hit && hit.distance < closest)
                {
                    hitDist = hit.distance;
                    hitSomething = true;

                    closest = hit.distance;
                    hitIndex = &b - &blocks[0]; // Index
                    hitInfo = hit;
                    hitBlock = b;
                }
            }


            //Break Block
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hitIndex != -1 && !inventoryOpen) {
                if (hitDist <= playerReach)
                {
                    const Item* brokenType = blocks[hitIndex].item;
                    blocks.erase(blocks.begin() + hitIndex);

                    blockPositions.erase(std::make_tuple(
                        (int)blocks[hitIndex].pos.x, 
                        (int)blocks[hitIndex].pos.y, 
                        (int)blocks[hitIndex].pos.z
                    ));

                    eraseContainer(blocks[hitIndex].pos);


                    if (!playerCreative) AddItemToInventory(brokenType, 1);
                    blocksDestroyed++;
                    totalBlocks--;
                }
            }


            //Place Block
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && hitIndex != -1 && !inventoryOpen) {
                if (hitDist <= playerReach)
                {
                    ItemStack& slot = hotbar[selectedHotbarSlot];
                    if (slot.item != nullptr && slot.count > 0)
                    {
                        if (slot.item->placable) {
                            Vector3 p = Vector3Add(blocks[hitIndex].pos, Vector3Scale(hitInfo.normal, 1.0f));

                            Block b;
                            b.pos = p;
                            b.item = slot.item;
                            b.box = MakeBox(p);
                            blocks.push_back(b);

                            blockPositions.emplace(
                                (int)b.pos.x, 
                                (int)b.pos.y, 
                                (int)b.pos.z
                            );

                            if (!playerCreative) slot.count--;
                            if (slot.count <= 0) slot.item = nullptr;
                            blocksPlaced++;
                            totalBlocks++;
                        }
                    }

                }
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && !inventoryOpen) {
                for (int i = 0; i < HOTBAR_SIZE; i++) {
                    if (hotbar[i].item == hitBlock.item) {
                        selectedHotbarSlot = i;

                        if (hotbar[selectedHotbarSlot].item != nullptr) {
                            hotbarItemName = hotbar[selectedHotbarSlot].item->name;
                            lastHotbarItem = hotbarItemName;
                        } else {
                            hotbarItemName = "Empty";
                            lastHotbarItem = "None";
                        }
                        break;
                    }
                }

                for (int r = 0; r < INV_ROWS; r++) {
                    for (int c = 0; c < INV_COLS; c++) { 
                        if (inventory[r][c].item == hitBlock.item) {
                            TryAddToInventory(hotbar[selectedHotbarSlot]);

                            ItemStack tmp = inventory[r][c];
                            inventory[r][c] = ItemStack(nullptr, 0);
                            
                            hotbarItemName = hitBlock.item->name;
                            lastHotbarItem = hotbarItemName;

                            TryAddToHotbar(tmp);
                            break;
                        }
                    }
                }
            }


            // Scroll-Event
            float wheel = GetMouseWheelMove();
            if (wheel != 0 && !inventoryOpen) {
                selectedHotbarSlot -= (int)wheel;
                if (selectedHotbarSlot < 0) selectedHotbarSlot = HOTBAR_SIZE - 1;
                if (selectedHotbarSlot >= HOTBAR_SIZE) selectedHotbarSlot = 0;

                // Name des neuen Items merken
                if (hotbar[selectedHotbarSlot].item != nullptr) {
                    hotbarItemName = hotbar[selectedHotbarSlot].item->name;
                    lastHotbarItem = hotbarItemName;
                } else {
                    hotbarItemName = "Empty";
                    lastHotbarItem = "None";
                }

                hotbarItemTimer = 0.0f;            // Timer zurücksetzen
                hotbarItemJustScrolled = true;     // Flag aktiv
            }



        }

        bool hIsDown = IsKeyDown(KEY_H);
        if (hIsDown && !hWasDown) showDebugOverlay = !showDebugOverlay;
        hWasDown = hIsDown;

        if (IsKeyPressed(KEY_E))
        {
            // =====================================================
            // 1️⃣ Container schließen, falls offen
            // =====================================================
            if (containerOpen)
            {
                containerOpen    = false;
                inventoryOpen    = false;
                currentContainer = nullptr;
                currentTab       = InventoryTab::PLAYER;

                DisableCursor();
            }
            // =====================================================
            // 2️⃣ Nur öffnen wenn WIRKLICH gerade getroffen
            // =====================================================
            else if (hitSomething &&
                    hitDist <= playerReach &&
                    hitBlock.item != nullptr &&
                    hitBlock.item->hasInv)
            {
                openInv(
                    hitBlock.item->hasInv,
                    (char*)hitBlock.item->name,
                    hitBlock.pos
                );
            }
            // =====================================================
            // 3️⃣ Normales Player Inventory
            // =====================================================
            else
            {
                inventoryOpen = !inventoryOpen;
                currentTab = InventoryTab::PLAYER;

                if (inventoryOpen)
                    EnableCursor();
                else
                    DisableCursor();
            }

            // Dragged Item zurücklegen
            if (draggedItem.item != nullptr)
            {
                TryAddToHotbar(draggedItem);
                draggedItem = ItemStack(nullptr, 0);
            }
        }



        if (IsKeyPressed(KEY_C)) {
            if (currentTab == InventoryTab::CREATIVE) currentTab = InventoryTab::PLAYER;
            playerCreative = !playerCreative;
        };











        // -------- DRAW --------
        BeginDrawing();
        ClearBackground(SKYBLUE);
        BeginMode3D(camera);
        
        /*for (auto& b : blocks) {
            if (fabs(b.pos.x - playerPos.x) <= renderDistance &&
                fabs(b.pos.y - playerPos.y) <= renderDistance &&
                fabs(b.pos.z - playerPos.z) <= renderDistance)
            {
                DrawBlockShaded(b, &b == &blocks[hitIndex]);
            }
        }*/
        /*for (size_t i = 0; i < blocks.size(); i++)
        {
            DrawBlockShaded(blocks[i], i == (size_t) hitIndex && hitDist <= playerReach);
        }*/


        // Berechne Blickrichtung (falls du eine Kamera benutzt)
        Vector3 viewDir = Vector3Subtract(camera.target, camera.position);
        float len = sqrt(viewDir.x*viewDir.x + viewDir.y*viewDir.y + viewDir.z*viewDir.z);
        viewDir.x /= len; 
        viewDir.y /= len; 
        viewDir.z /= len;

        for (size_t i = 0; i < blocks.size(); i++)
        {
            if (IsBlockInView(blocks[i].pos, playerPos, viewDir, maxViewDistance, fovAngle))
            {
                DrawBlockShaded(blocks[i], i == (size_t)hitIndex && hitDist <= playerReach);
            }
        }

        //Draw Player Modell
        //DrawCylinder((Vector3){ playerPos.x, playerPos.y - 0.75f, playerPos.z }, 0.25f, 0.25f, 0.25f, 16, BLUE);




        EndMode3D();

        int cx = GetScreenWidth()/2;
        int cy = GetScreenHeight()/2;
        DrawLine(cx-8,cy,cx+8,cy,BLACK);
        DrawLine(cx,cy-8,cx,cy+8,BLACK);

        if (showDebugOverlay) {
            std::vector<HUDPanel> panels;

            panels.push_back(HUDPanel{
                "Position",
                {
                    TextFormat("X: %d", (int)round(playerPos.x)),
                    TextFormat("Y: %d", (int)round(playerPos.y) - 1),
                    TextFormat("Z: %d", (int)round(playerPos.z))
                },
                BLUE,
                PanelAlignment::LEFT
            });

            panels.push_back(HUDPanel{
                "Target Block",
                {
                    TextFormat("Name: %s", hitBlock.item->name),
                    TextFormat("X: %d", (int)round(hitBlock.pos.x)),
                    TextFormat("Y: %d", (int)round(hitBlock.pos.y)),
                    TextFormat("Z: %d", (int)round(hitBlock.pos.z))
                },
                GREEN,
                PanelAlignment::LEFT
            });

            panels.push_back(HUDPanel{
                "Blocks",
                {
                    TextFormat("Blocks Placed: %d", (int)round(blocksPlaced)),
                    TextFormat("Blocks Destroyed: %d", (int)round(blocksDestroyed)),
                    TextFormat("Total Blocks: %d", (int)round(totalBlocks))
                },
                VIOLET,
                PanelAlignment::LEFT
            });

            panels.push_back(HUDPanel{
                "Time",
                {
                    TextFormat("Running: %.2f s", GetTime())
                },
                PURPLE,
                PanelAlignment::LEFT
            });

            panels.push_back(HUDPanel{
                "Rotation",
                {
                    TextFormat("Yaw: %.2f", (float)yaw),
                    TextFormat("Pitch: %.2f", (float)pitch), 
                    TextFormat("Facing: %s", cardinalDirection)
                },
                ORANGE,
                PanelAlignment::RIGHT
            });

            DrawHUD(panels);

            DrawText("Press H to disable the Debug Overlay", 15, (float)GetScreenHeight() - 30, 15, RAYWHITE);
        } else {
            DrawText("Press H to enable the Debug Overlay", 15, (float)GetScreenHeight() - 30, 15, RAYWHITE);
        }


        // ================== DRAW PLAYER HOTBAR UND HAND ==================
        int slotSize = 50;
        int spacing = 6;
        int totalWidth = HOTBAR_SIZE * slotSize + (HOTBAR_SIZE - 1) * spacing;
        int startX = GetScreenWidth()/2 - totalWidth/2;
        int y = GetScreenHeight() - 70;

        // Zeichne die Hotbar unten am Bildschirmrand
        for (int i = 0; i < HOTBAR_SIZE; i++)
        {
            Rectangle r = { (float)(startX + i*(slotSize + spacing)), (float)y, (float)slotSize, (float)slotSize };

            DrawRectangleRounded(r, 0.1f, 4, Fade(LIGHTGRAY, 0.5f));
            DrawRectangleLinesEx(r, 2, BLACK);

            if (hotbar[i].item != nullptr)
            {
                Color col = GetItemColor(hotbar[i].item);

                if (hotbar[i].item->name == (char*) "Debug") col = {255,3,62,125};

                float previewSize = r.width * 0.7f;
                DrawRectangle(
                    r.x + (r.width - previewSize)/2,
                    r.y + (r.height - previewSize)/2,
                    previewSize,
                    previewSize,
                    col
                );

                DrawText(
                    TextFormat("%d", hotbar[i].count),
                    r.x + r.width - 18,
                    r.y + r.height - 18,
                    14,
                    WHITE
                );
            }

            // Hervorhebung des ausgewählten Slots
            if (i == selectedHotbarSlot)
                DrawRectangleLinesEx(r, 3, YELLOW);
        }










        // --- Block Preview unten rechts ---
        ItemStack current = hotbar[selectedHotbarSlot];
        if (current.item != nullptr) {
            float cubeSize = 1.0f;

            // RenderTexture vorbereiten
            BeginTextureMode(previewTex);
                ClearBackground(BLANK);

                Camera3D previewCamera = {};
                previewCamera.position = { 2.0f * sinf(rotation * DEG2RAD), -1.8f, 2.0f * cosf(rotation * DEG2RAD) };
                previewCamera.target = { 0.0f, 0.0f, 0.0f };
                previewCamera.up = { 0.0f, 1.0f, 0.0f };
                previewCamera.fovy = 45.0f;
                previewCamera.projection = CAMERA_PERSPECTIVE;

                Color col = GetItemColor(current.item);

                if (current.item->name == (char*) "Debug") col = {255,3,62,125};

                BeginMode3D(previewCamera);
                    DrawCube({0.0f, 0.0f, 0.0f}, cubeSize, cubeSize, cubeSize, col);
                    DrawCubeWires({0.0f, 0.0f, 0.0f}, cubeSize, cubeSize, cubeSize, BLACK);
                EndMode3D();

            EndTextureMode();

            int px = GetScreenWidth() - previewTex.texture.width - 20;
            int py = GetScreenHeight() - previewTex.texture.height - 20;

            DrawTexture(previewTex.texture, px, py, WHITE);

        }






        // Timer hochzählen
        hotbarItemTimer += GetFrameTime();

        // Tooltip unten über der Hotbar anzeigen
        if (hotbarItemName != "") {
            float screenW = (float)GetScreenWidth();
            float screenH = (float)GetScreenHeight();
            float boxW = MeasureText(hotbarItemName.c_str(), 20) + 16; // Padding
            float boxH = 32;
            float x = screenW / 2 - boxW / 2;
            float y = screenH - 130; // Über der Hotbar

            // Hintergrund
            DrawRectangleRounded({x, y, boxW, boxH}, 0.2f, 4, Fade(BLACK, 0.7f));
            // Rahmen
            DrawRectangleRoundedLines({x, y, boxW, boxH}, 0.2f, 4, WHITE);
            // Text
            DrawText(hotbarItemName.c_str(), x + 8, y + 6, 20, WHITE);

            // Anzeige-Logik
            if (hotbarItemJustScrolled) {
                // Zeige sofort nach Scrollen für hotbarItemShowTime Sekunden
                if (hotbarItemTimer >= hotbarItemShowTime) {
                    hotbarItemJustScrolled = false;  // Timer endet, gehe in normale Hide-Phase
                    hotbarItemTimer = 0.0f;          // Reset für HideDelay
                }
            } else {
                // Wenn nicht gescrollt, nach hotbarItemHideDelay Sekunden verschwinden
                if (hotbarItemTimer >= hotbarItemHideDelay) {
                    hotbarItemName = "";
                    hotbarItemTimer = 0.0f;
                }
            }
        }


        if (playerCreative) {
            Rectangle creativeOverlayRec = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
            DrawRectangleLinesEx(creativeOverlayRec, 10.0f, RED); 
        }










        

        // ================== INVENTORY GUI ==================
        if (inventoryOpen)
        {
            bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

            // ---------- Hintergrund ----------
            int invWidth  = INV_COLS * INV_CELL + (INV_COLS-1)*INV_SPACING + 40;
            int invHeight = INV_ROWS * INV_CELL + (INV_ROWS-1)*INV_SPACING + 200;
            int bgX = GetScreenWidth()/2 - invWidth/2;
            int bgY = GetScreenHeight()/2 - invHeight/2;

            DrawRectangleRounded(
                { (float)bgX, (float)bgY, (float)invWidth, (float)invHeight },
                0.05f, 4, Fade(BLACK, 0.6f)
            );

            Vector2 mouse = GetMousePosition();

            // ---------- TABS ----------
            int yOffset = 10;
            

            if (playerProfile) {
                Rectangle tabProfile = { (float)bgX - 110,  (float)bgY + yOffset, 100, 30 };
                DrawRectangleRec(tabProfile, currentTab == InventoryTab::PROFILE ? DARKGRAY : Fade(GRAY, 0.5f));
                DrawText("Profile", tabProfile.x + 10, tabProfile.y + 7, 16, WHITE);

                yOffset += 40;

                if (CheckCollisionPointRec(mouse, tabProfile) &&
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    currentTab = InventoryTab::PROFILE;
            }
            
            if (playerInv) {
                Rectangle tabPlayer = { (float)bgX - 110,  (float)bgY + yOffset, 100, 30 };
                DrawRectangleRec(tabPlayer, currentTab == InventoryTab::PLAYER ? DARKGRAY : Fade(GRAY, 0.5f));
                DrawText("Inventory", tabPlayer.x + 10, tabPlayer.y + 7, 16, WHITE);

                yOffset += 40;

                if (CheckCollisionPointRec(mouse, tabPlayer) &&
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    currentTab = InventoryTab::PLAYER;
            } 

            if (canCraft) {
                Rectangle tabCrafting = { (float)bgX - 110,  (float)bgY + yOffset, 100, 30 };
                DrawRectangleRec(tabCrafting, currentTab == InventoryTab::CRAFTING ? DARKGRAY : Fade(GRAY, 0.5f));
                DrawText("Crafting",  tabCrafting.x + 10, tabCrafting.y + 7, 16, WHITE);

                yOffset += 40;

                if (CheckCollisionPointRec(mouse, tabCrafting) &&
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    currentTab = InventoryTab::CRAFTING;
            }

            if (hasSkills) {
                Rectangle tabSkills = { (float)bgX - 110,  (float)bgY + yOffset, 100, 30 };
                DrawRectangleRec(tabSkills, currentTab == InventoryTab::SKILLS ? DARKGRAY : Fade(GRAY, 0.5f));
                DrawText("Skills",  tabSkills.x + 10, tabSkills.y + 7, 16, WHITE);

                yOffset += 40;

                if (CheckCollisionPointRec(mouse, tabSkills) &&
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    currentTab = InventoryTab::SKILLS;
            }

            if (playerCreative) {
                Rectangle tabCreative = { (float)bgX - 110,  (float)bgY + yOffset, 100, 30 };
                DrawRectangleRec(tabCreative, currentTab == InventoryTab::CREATIVE ? DARKGRAY : Fade(GRAY, 0.5f));
                DrawText("Creative",  tabCreative.x + 10, tabCreative.y + 7, 16, WHITE);

                yOffset += 40;

                if (CheckCollisionPointRec(mouse, tabCreative) &&
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    currentTab = InventoryTab::CREATIVE;
            } 

            if (containerOpen) {
                Rectangle tabContainer = { (float)bgX - 110,  (float)bgY + yOffset, 100, 30 };
                DrawRectangleRec(tabContainer, currentTab == InventoryTab::CONTAINER ? DARKGRAY : Fade(GRAY, 0.5f));
                DrawText(containerTitel,  tabContainer.x + 10, tabContainer.y + 7, 16, WHITE);

                yOffset += 40;

                if (CheckCollisionPointRec(mouse, tabContainer) &&
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    currentTab = InventoryTab::CONTAINER;
            }

            int startX = bgX + 20;
            int startY = bgY + 40;

            // =========================================================
            // ================== PLAYER INVENTORY =====================
            // =========================================================
            if (currentTab == InventoryTab::PLAYER)
            {
                DrawText("Inventory", bgX + 20, bgY + 10, 20, RAYWHITE);

                // -------- INVENTORY SLOTS --------
                for (int r = 0; r < INV_ROWS; r++)
                {
                    for (int c = 0; c < INV_COLS; c++)
                    {
                        Rectangle slot = {
                            (float)(startX + c*(INV_CELL + INV_SPACING)),
                            (float)(startY + r*(INV_CELL + INV_SPACING)),
                            (float)INV_CELL,
                            (float)INV_CELL
                        };

                        DrawRectangleRounded(slot, 0.1f, 4, Fade(LIGHTGRAY, 0.2f));
                        DrawRectangleLinesEx(slot, 1, Fade(WHITE, 0.3f));

                        ItemStack& s = inventory[r][c];
                        if (s.item && s.count > 0)
                        {
                            Color col = GetItemColor(s.item);
                            if (s.item->name == (char*) "Debug") col = {255,3,62,125};
                            float previewSize = slot.width * 0.7f;

                            DrawRectangle(
                                slot.x + (slot.width-previewSize)/2,
                                slot.y + (slot.height-previewSize)/2,
                                previewSize, previewSize, col
                            );

                            DrawText(TextFormat("%d", s.count),
                                slot.x + slot.width - 18,
                                slot.y + slot.height - 18,
                                14, WHITE);
                        }
                    }
                }
            }

            //Chest inventory
            if (currentTab == InventoryTab::CONTAINER) 
            {
                DrawText(containerTitel, bgX + 20, bgY + 10, 20, RAYWHITE);

                for (int r = 0; r < CHEST_ROWS; r++)
                {
                    for (int c = 0; c < CHEST_COLS; c++)
                    {
                        Rectangle slot = {
                            (float)(startX + c*(INV_CELL + INV_SPACING)),
                            (float)(startY + r*(INV_CELL + INV_SPACING)),
                            (float)INV_CELL,
                            (float)INV_CELL
                        };

                        DrawRectangleRounded(slot, 0.1f, 4, Fade(LIGHTGRAY, 0.2f));
                        DrawRectangleLinesEx(slot, 1, Fade(WHITE, 0.3f));

                        ItemStack& s = currentContainer->slots[r][c];

                        if (s.item && s.count > 0)
                        {
                            Color col = GetItemColor(s.item);
                            if (s.item->name == (char*) "Debug") col = {255,3,62,125};
                            float previewSize = slot.width * 0.7f;

                            DrawRectangle(
                                slot.x + (slot.width-previewSize)/2,
                                slot.y + (slot.height-previewSize)/2,
                                previewSize,
                                previewSize,
                                col
                            );

                            DrawText(TextFormat("%d", s.count),
                                slot.x + slot.width - 18,
                                slot.y + slot.height - 18,
                                14,
                                WHITE);
                        }

                        // ================= CLICK LOGIC =================
                        if (CheckCollisionPointRec(mouse, slot))
                        {
                            bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
                            bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

                            // ================= LEFT CLICK =================
                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                            {
                                // 🔥 SHIFT CLICK (Container → Player)
                                if (ctrlDown && s.item != nullptr) {
                                    // Ctrl + Linksklick öffnet das Split-Fenster
                                    showSplitWindow = true;
                                    splitTarget = &s;
                                    inputText = "";
                                } 
                                else if (shiftDown && s.item != nullptr)
                                {
                                    ItemStack temp = s;
                                    TryAddToHotbar(temp);

                                    if (temp.count <= 0)
                                        s = ItemStack(nullptr, 0);
                                    else
                                        s.count = temp.count;

                                    break;
                                }
                                // Normal Click
                                else { 
                                    if (draggedItem.item == nullptr && s.item != nullptr)
                                    {
                                        draggedItem = s;
                                        s = ItemStack(nullptr, 0);
                                    }
                                    else if (draggedItem.item != nullptr)
                                    {
                                        if (s.item == draggedItem.item)
                                            MergeStacks(s, draggedItem);
                                        else
                                            std::swap(s, draggedItem);
                                    }
                                }
                            }

                            // ================= RIGHT CLICK =================
                            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                            {
                                if (draggedItem.item == nullptr && s.item != nullptr)
                                {
                                    int half = s.count / 2;
                                    if (half == 0) half = 1;

                                    draggedItem = ItemStack(s.item, half);
                                    s.count -= half;

                                    if (s.count <= 0)
                                        s = ItemStack(nullptr, 0);
                                }
                                else if (draggedItem.item != nullptr)
                                {
                                    if (s.item == nullptr)
                                    {
                                        s = ItemStack(draggedItem.item, 1);
                                        draggedItem.count--;
                                    }
                                    else if (s.item == draggedItem.item)
                                    {
                                        s.count++;
                                        draggedItem.count--;
                                    }

                                    if (draggedItem.count <= 0)
                                        draggedItem = ItemStack(nullptr, 0);
                                }
                            }
                        }

                    }
                }
            }










            //Crafing inventory
            if (currentTab == InventoryTab::CRAFTING)
            {
                DrawText("Crafting", bgX + 20, bgY + 10, 20, RAYWHITE);

                Vector2 mouse = GetMousePosition();
                int panelPadding = 20;

                // =========================================================
                // ================= LEFT PANEL (Recipe List) ==============
                // =========================================================

                int leftWidth = 250;

                Rectangle leftPanel = {
                    (float)bgX + panelPadding,
                    (float)bgY + 40,
                    (float)leftWidth,
                    (float)invHeight - 150
                };

                DrawRectangleRec(leftPanel, Fade(DARKGRAY, 0.6f));

                int entryHeight = 40;
                int visibleEntries = (int)(leftPanel.height / entryHeight);
                int totalRecipes = (int)allRecipes.size();

                // ================= Mouse Wheel Scroll =================
                if (CheckCollisionPointRec(mouse, leftPanel))
                {
                    float wheel = GetMouseWheelMove();
                    if (wheel != 0)
                        craftingScroll -= (int)wheel;
                }

                int maxScroll = totalRecipes - visibleEntries;
                if (maxScroll < 0) maxScroll = 0;

                if (craftingScroll < 0) craftingScroll = 0;
                if (craftingScroll > maxScroll) craftingScroll = maxScroll;

                // ================= Draw Recipe Entries =================
                for (int i = 0; i < visibleEntries; i++)
                {
                    int recipeIndex = i + craftingScroll;
                    if (recipeIndex >= totalRecipes)
                        break;

                    Rectangle entry = {
                        leftPanel.x + 5,
                        leftPanel.y + i * entryHeight,
                        leftPanel.width - 15,
                        (float)entryHeight - 5
                    };

                    bool selected = recipeIndex == selectedRecipeIndex;

                    DrawRectangleRec(entry, selected ? Fade(GRAY, 0.8f) : Fade(GRAY, 0.4f));

                    DrawText(
                        allRecipes[recipeIndex].result->name,
                        entry.x + 10,
                        entry.y + 15,
                        18,
                        WHITE
                    );

                    if (CheckCollisionPointRec(mouse, entry) &&
                        IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        selectedRecipeIndex = recipeIndex;
                    }
                }

                // ================= Scrollbar =================
                if (totalRecipes > visibleEntries)
                {
                    float scrollbarWidth = 8;

                    Rectangle scrollTrack = {
                        leftPanel.x + leftPanel.width - scrollbarWidth - 2,
                        leftPanel.y,
                        scrollbarWidth,
                        leftPanel.height
                    };

                    DrawRectangleRec(scrollTrack, Fade(BLACK, 0.4f));

                    float scrollRatio = (float)visibleEntries / totalRecipes;
                    float thumbHeight = scrollTrack.height * scrollRatio;

                    float scrollPercent = (maxScroll > 0)
                        ? (float)craftingScroll / maxScroll
                        : 0.0f;

                    float thumbY = scrollTrack.y +
                        (scrollTrack.height - thumbHeight) * scrollPercent;

                    Rectangle scrollThumb = {
                        scrollTrack.x,
                        thumbY,
                        scrollbarWidth,
                        thumbHeight
                    };

                    DrawRectangleRec(scrollThumb, Fade(LIGHTGRAY, 0.8f));

                    // ===== Drag Start =====
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                        CheckCollisionPointRec(mouse, scrollThumb))
                    {
                        isDraggingCraftScroll = true;
                    }

                    // ===== Drag End =====
                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                    {
                        isDraggingCraftScroll = false;
                    }

                    // ===== Drag Update =====
                    if (isDraggingCraftScroll)
                    {
                        float mouseRelative = mouse.y - scrollTrack.y;
                        float percent = mouseRelative / scrollTrack.height;

                        if (percent < 0) percent = 0;
                        if (percent > 1) percent = 1;

                        craftingScroll = (int)(percent * maxScroll);
                    }
                }

                // =========================================================
                // ================= RIGHT PANEL (Preview) =================
                // =========================================================

                if (selectedRecipeIndex >= 0 &&
                    selectedRecipeIndex < totalRecipes)
                {
                    const Recipe& recipe = allRecipes[selectedRecipeIndex];

                    int rightX = leftPanel.x + leftPanel.width + 20;

                    DrawText(recipe.result->name,
                        rightX,
                        leftPanel.y,
                        24,
                        YELLOW);

                    // Item Preview
                    DrawRectangle(
                        rightX,
                        leftPanel.y + 40,
                        70,
                        70,
                        recipe.result->color
                    );

                    DrawText(
                        recipe.result->tooltip,
                        rightX,
                        leftPanel.y + 120,
                        18,
                        RAYWHITE
                    );

                    // ================= Ingredients =================
                    int ingY = bgY + panelPadding * 2;
                    int btnY = leftPanel.y + 150;
                    int ingX = bgX + leftPanel.width + 200;

                    DrawText("Requires:", ingX, ingY, 20, WHITE);
                    ingY += 30;

                    for (auto& ing : recipe.ingredients)
                    {
                        int available = CountItemInInventory(ing.item);
                        Color col = available >= ing.count ? GREEN : RED;

                        DrawText(
                            TextFormat("%s x%d (%d)",
                                ing.item->name,
                                ing.count,
                                available),
                            ingX,
                            ingY,
                            18,
                            col
                        );

                        ingY += 25;
                    }

                    int maxCraft = GetMaxCraftAmount(recipe);

                    // ================= Buttons =================
                    Rectangle btn1   = { (float)rightX, (float)(btnY + 10), 100, 35 };
                    Rectangle btn10  = { (float)(rightX + 110), (float)(btnY + 10), 100, 35 };
                    Rectangle btnMax = { (float)(rightX + 220), (float)(btnY + 10), 120, 35 };

                    DrawRectangleRec(btn1,   maxCraft >= 1  ? SKYBLUE : DARKGRAY);
                    DrawRectangleRec(btn10,  maxCraft >= 10 ? SKYBLUE : DARKGRAY);
                    DrawRectangleRec(btnMax, maxCraft > 0   ? ORANGE  : DARKGRAY);

                    DrawText("Craft 1",  btn1.x + 10,  btn1.y + 8, 18, BLACK);
                    DrawText("Craft 10", btn10.x + 10, btn10.y + 8, 18, BLACK);
                    DrawText("Craft Max",btnMax.x + 10,btnMax.y + 8, 18, BLACK);

                    if (maxCraft >= 1 &&
                        CheckCollisionPointRec(mouse, btn1) &&
                        IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        CraftRecipe(recipe, 1);
                    }

                    if (maxCraft >= 10 &&
                        CheckCollisionPointRec(mouse, btn10) &&
                        IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        CraftRecipe(recipe, 10);
                    }

                    if (maxCraft > 0 &&
                        CheckCollisionPointRec(mouse, btnMax) &&
                        IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        CraftRecipe(recipe, maxCraft);
                    }
                }
            }


















            //Profile GUI
            if (currentTab == InventoryTab::PROFILE) 
            {
                DrawText("Profile", bgX + 20, bgY + 10, 20, RAYWHITE);

                DrawText("Comming Soon", bgX + (invWidth/2) - 65, bgY + (invHeight/2) - 25, 20, RAYWHITE);
            }










            //Skills GUI
            if (currentTab == InventoryTab::SKILLS)
            {
                DrawSkillGUI(skills, skillPoints, bgX, bgY, invWidth, invHeight);
            }











            // Creative Inventory
            if (currentTab == InventoryTab::CREATIVE)
            {
                DrawText("Creative Inventory", bgX + 20, bgY + 10, 20, RAYWHITE);

                const int CREATIVE_COLS = 9;
                const int CREATIVE_VISIBLE_ROWS = 3;

                int totalItems = (int)allItems.size();
                int totalRows  = (totalItems + CREATIVE_COLS - 1) / CREATIVE_COLS + 5;

                // ---------- MAUSRAD SCROLL ----------
                float wheel = GetMouseWheelMove();
                if (wheel != 0 && totalRows > CREATIVE_VISIBLE_ROWS)
                {
                    creativeScrollRow -= (int)wheel;
                    creativeScrollRow = Clamp(
                        creativeScrollRow,
                        0,
                        totalRows - CREATIVE_VISIBLE_ROWS
                    );
                }

                // ---------- ITEMS RENDERN (ALLE SLOTS) ----------
                for (int r = 0; r < CREATIVE_VISIBLE_ROWS; r++)
                {
                    for (int c = 0; c < CREATIVE_COLS; c++)
                    {
                        int itemIndex = (r + creativeScrollRow) * CREATIVE_COLS + c;

                        Rectangle slot = {
                            (float)(startX + c*(INV_CELL + INV_SPACING)),
                            (float)(startY + r*(INV_CELL + INV_SPACING)),
                            (float)INV_CELL,
                            (float)INV_CELL
                        };

                        // Immer Slot zeichnen (auch leer)
                        DrawRectangleRounded(slot, 0.1f, 4, Fade(LIGHTGRAY, 0.2f));
                        DrawRectangleLinesEx(slot, 1, Fade(WHITE, 0.3f));

                        bool slotHasItem = itemIndex < totalItems;
                        Item* item = slotHasItem ? allItems[itemIndex] : nullptr;

                        // Item zeichnen, falls vorhanden
                        if (slotHasItem)
                        {
                            Color col = GetItemColor(item);
                            if (item->name == (char*)"Debug") col = {255,3,62,125};

                            float previewSize = slot.width * 0.7f;
                            DrawRectangle(
                                slot.x + (slot.width - previewSize)/2,
                                slot.y + (slot.height - previewSize)/2,
                                previewSize,
                                previewSize,
                                col
                            );
                        }

                        //Mouse Clicks
                        if (CheckCollisionPointRec(GetMousePosition(), slot))
                        {
                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                            {
                                if (slotHasItem)
                                {
                                    if (draggedItem.item == nullptr) {
                                        if (!shiftDown) {
                                            draggedItem = ItemStack(item, 1);
                                        } else { 
                                            ItemStack temp = ItemStack(item, 1);

                                            if (!containerOpen) TryAddToHotbar(temp);
                                            else if (containerOpen) TryAddToContainer(temp);
                                        }
                                    } else if (draggedItem.item == item) {
                                        int count = draggedItem.count;
                                        count++;
                                        if (count <= STACK_SIZE) draggedItem = ItemStack(item, count);
                                    } else {
                                        draggedItem = ItemStack(nullptr, 0);
                                    }
                                } else {
                                    draggedItem = ItemStack(nullptr, 0);
                                }
                            }

                            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                            {
                                if (draggedItem.item == nullptr) {
                                    if (!shiftDown) {
                                        draggedItem = ItemStack(item, STACK_SIZE / 2);
                                    } else { 
                                        ItemStack temp = ItemStack(item, STACK_SIZE / 2);
                                        
                                        if (!containerOpen) TryAddToHotbar(temp);
                                        else if (containerOpen) TryAddToContainer(temp);
                                    }
                                } else {
                                    int count = draggedItem.count;
                                    count--;
                                    if (count > 0) draggedItem = ItemStack(draggedItem.item, count);
                                    else draggedItem = ItemStack(nullptr, 0);
                                }
                            }

                            if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && slotHasItem)
                            {
                                if (!shiftDown) {
                                    draggedItem = ItemStack(item, STACK_SIZE);
                                } else { 
                                    ItemStack temp = ItemStack(item, STACK_SIZE);
                                    
                                    if (!containerOpen) TryAddToHotbar(temp);
                                    else if (containerOpen) TryAddToContainer(temp);
                                }
                            }
                        }
                    }
                }


                // ================= SCROLLBAR =================
                if (totalRows > CREATIVE_VISIBLE_ROWS)
                {
                    float scrollbarHeight =
                        CREATIVE_VISIBLE_ROWS * (INV_CELL + INV_SPACING) - INV_SPACING;

                    Rectangle scrollbarTrack = {
                        (float)(startX + CREATIVE_COLS*(INV_CELL + INV_SPACING) + 6),
                        (float)startY,
                        12,
                        scrollbarHeight
                    };

                    DrawRectangleRec(scrollbarTrack, Fade(DARKGRAY, 0.6f));

                    float ratio = (float)CREATIVE_VISIBLE_ROWS / (float)totalRows;
                    float thumbHeight = scrollbarTrack.height * ratio;

                    float scrollPercent =
                        (float)creativeScrollRow / (float)(totalRows - CREATIVE_VISIBLE_ROWS);

                    float thumbY = scrollbarTrack.y + scrollPercent * (scrollbarTrack.height - thumbHeight);

                    Rectangle thumb = {scrollbarTrack.x, thumbY, scrollbarTrack.width, thumbHeight};
                    DrawRectangleRec(thumb, LIGHTGRAY);

                    // Scrollbar ziehen
                    if (CheckCollisionPointRec(GetMousePosition(), thumb) &&
                        IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                    {
                        float relative = (GetMouseY() - scrollbarTrack.y) / (scrollbarTrack.height - thumbHeight);
                        relative = Clamp(relative, 0.0f, 1.0f);
                        creativeScrollRow = (int)(relative * (totalRows - CREATIVE_VISIBLE_ROWS));
                    }
                }
            }






            // -------- HOTBAR --------
            int hotbarY = bgY + invHeight - 70;
            int hotbarWidth = HOTBAR_SIZE * INV_CELL + (HOTBAR_SIZE-1)*INV_SPACING;
            int hotbarX = GetScreenWidth()/2 - hotbarWidth/2;

            DrawText("Hotbar", hotbarX, hotbarY - 30, 20, RAYWHITE);

            for (int i = 0; i < HOTBAR_SIZE; i++)
            {
                Rectangle r = {
                    (float)(hotbarX + i*(INV_CELL + INV_SPACING)),
                    (float)hotbarY,
                    (float)INV_CELL,
                    (float)INV_CELL
                };

                DrawRectangleRounded(r, 0.1f, 4, Fade(LIGHTGRAY, 0.2f));
                DrawRectangleLinesEx(r, 1, Fade(WHITE, 0.3f));

                if (hotbar[i].item)
                {
                    Color col = GetItemColor(hotbar[i].item);
                    if (hotbar[i].item->name == (char*) "Debug") col = {255,3,62,125};
                    float previewSize = r.width * 0.7f;

                    DrawRectangle(
                        r.x + (r.width-previewSize)/2,
                        r.y + (r.height-previewSize)/2,
                        previewSize, previewSize, col
                    );

                    DrawText(TextFormat("%d", hotbar[i].count),
                        r.x + r.width - 18,
                        r.y + r.height - 18,
                        14, WHITE);
                }

                if (i == selectedHotbarSlot)
                    DrawRectangleLinesEx(r, 3, YELLOW);
            }










            // ================= DRAG & DROP LOGIC =================
            int invWidthLogic  = INV_COLS * INV_CELL + (INV_COLS - 1) * INV_SPACING;
            int invHeightLogic = INV_ROWS * INV_CELL + (INV_ROWS - 1) * INV_SPACING + INV_CELL * 2;

            int invStartX = GetScreenWidth() / 2 - invWidthLogic / 2;
            int invStartY = GetScreenHeight() / 2 - invHeightLogic / 2;

            int slotSize = 60;
            int spacing = 8;
            int startXHotBar = GetScreenWidth()/2 - totalWidth/2 - slotSize + 7;
            int yHotbar = bgY + invHeight - INV_CELL - 10;

            // ================= INVENTORY SLOTS =================
            for (int r = 0; r < INV_ROWS; r++)
            {
                for (int c = 0; c < INV_COLS; c++)
                {
                    Rectangle slot = {
                        (float)(invStartX + c * (INV_CELL + INV_SPACING)),
                        (float)(invStartY + r * (INV_CELL + INV_SPACING)),
                        (float)INV_CELL,
                        (float)INV_CELL
                    };

                    ItemStack& slotItem = inventory[r][c];

                    if (!CheckCollisionPointRec(mouse, slot))
                        continue;

                    // ---------- LEFT CLICK ----------
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

                        if (currentTab == InventoryTab::PLAYER)
                        {
                            if (ctrlDown && slotItem.item != nullptr) {
                                // Ctrl + Linksklick öffnet das Split-Fenster
                                showSplitWindow = true;
                                splitTarget = &slotItem;
                                inputText = "";
                            } 
                            else if (shiftDown && slotItem.item != nullptr)
                            {
                                ItemStack temp = slotItem;
                                if (!containerOpen) TryAddToHotbar(temp);
                                else if (containerOpen) TryAddToContainer(temp);
                                slotItem = ItemStack(nullptr, 0);
                            }
                            else
                            {
                                if (draggedItem.item == nullptr && slotItem.item != nullptr)
                                {
                                    draggedItem = slotItem;
                                    slotItem = ItemStack(nullptr, 0);
                                }
                                else if (draggedItem.item != nullptr)
                                {
                                    if (slotItem.item == draggedItem.item)
                                        MergeStacks(slotItem, draggedItem);
                                    else
                                        std::swap(slotItem, draggedItem);
                                }
                            }
                        }
                    }

                    // ---------- RIGHT CLICK ----------
                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                    {
                        if (currentTab == InventoryTab::PLAYER)
                        {
                            if (draggedItem.item == nullptr && slotItem.item != nullptr)
                            {
                                int half = slotItem.count / 2;
                                if (half == 0) half = 1;

                                draggedItem = ItemStack(slotItem.item, half);
                                slotItem.count -= half;

                                if (slotItem.count <= 0)
                                    slotItem = ItemStack(nullptr, 0);
                            }
                            else if (draggedItem.item != nullptr)
                            {
                                if (slotItem.item == nullptr)
                                {
                                    slotItem = ItemStack(draggedItem.item, 1);
                                    draggedItem.count--;
                                }
                                else if (slotItem.item == draggedItem.item)
                                {
                                    slotItem.count++;
                                    draggedItem.count--;
                                }

                                if (draggedItem.count <= 0)
                                    draggedItem = ItemStack(nullptr, 0);
                            }
                        }
                    }
                }
            }

            if (showSplitWindow) {
                DrawSplitWindow(GetScreenWidth(), GetScreenHeight());
            }

            // ================= HOTBAR SLOTS =================
            for (int i = 0; i < HOTBAR_SIZE; i++)
            {
                Rectangle slot = {
                    (float)(startXHotBar + i * (slotSize + spacing)),
                    (float)yHotbar,
                    (float)slotSize,
                    (float)slotSize
                };

                ItemStack& slotItem = hotbar[i];

                if (!CheckCollisionPointRec(mouse, slot))
                    continue;

                // ---------- LEFT CLICK ----------
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

                    if (ctrlDown && slotItem.item != nullptr) {
                        // Ctrl + Linksklick öffnet das Split-Fenster
                        showSplitWindow = true;
                        splitTarget = &slotItem;
                        inputText = "";
                    } 
                    else if (shiftDown && slotItem.item != nullptr)
                    {
                        ItemStack temp = slotItem;
                        if (currentTab == InventoryTab::PLAYER) TryAddToInventory(temp);
                        else if (currentTab == InventoryTab::CONTAINER) TryAddToContainer(temp);
                        slotItem = ItemStack(nullptr, 0);
                    }
                    else
                    {
                        if (draggedItem.item == nullptr && slotItem.item != nullptr)
                        {
                            draggedItem = slotItem;
                            slotItem = ItemStack(nullptr, 0);
                        }
                        else if (draggedItem.item != nullptr)
                        {
                            if (slotItem.item == draggedItem.item)
                                MergeStacks(slotItem, draggedItem);
                            else
                                std::swap(slotItem, draggedItem);
                        }
                    }
                }

                // ---------- RIGHT CLICK ----------
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                {
                    if (draggedItem.item == nullptr && slotItem.item != nullptr)
                    {
                        int half = slotItem.count / 2;
                        if (half == 0) half = 1;

                        draggedItem = ItemStack(slotItem.item, half);
                        slotItem.count -= half;

                        if (slotItem.count <= 0)
                            slotItem = ItemStack(nullptr, 0);
                    }
                    else if (draggedItem.item != nullptr)
                    {
                        if (slotItem.item == nullptr)
                        {
                            slotItem = ItemStack(draggedItem.item, 1);
                            draggedItem.count--;
                        }
                        else if (slotItem.item == draggedItem.item)
                        {
                            slotItem.count++;
                            draggedItem.count--;
                        }

                        if (draggedItem.count <= 0)
                            draggedItem = ItemStack(nullptr, 0);
                    }
                }
            }













            // ---- ITEM TOOLTIP (hover) ----
            ItemStack* hoveredStack = nullptr;

            // ---- PLAYER INVENTORY ----
            if (currentTab == InventoryTab::PLAYER)
            {
                for (int r = 0; r < INV_ROWS; r++)
                {
                    for (int c = 0; c < INV_COLS; c++)
                    {
                        Rectangle slot = {
                            (float)(startX + c * (INV_CELL + INV_SPACING)),
                            (float)(startY + r * (INV_CELL + INV_SPACING)),
                            (float)INV_CELL,
                            (float)INV_CELL
                        };

                        if (CheckCollisionPointRec(mouse, slot) &&
                            inventory[r][c].item != nullptr)
                        {
                            hoveredStack = &inventory[r][c];
                        }
                    }
                }

                // ---- HOTBAR ----
                int totalWidth = HOTBAR_SIZE * slotSize + (HOTBAR_SIZE - 1) * spacing;
                int startXHotBarInInventory = GetScreenWidth()/2 - totalWidth/2;
                int yHotBarInInventory = bgY + invHeight - 70;

                for (int i = 0; i < HOTBAR_SIZE; i++)
                {
                    Rectangle slot = {
                        (float)(startXHotBarInInventory + i * (slotSize + spacing)),
                        (float)yHotBarInInventory,
                        (float)slotSize,
                        (float)slotSize
                    };

                    if (CheckCollisionPointRec(mouse, slot) &&
                        hotbar[i].item != nullptr)
                    {
                        hoveredStack = &hotbar[i];
                    }
                }
            }

            Item* hoveredCreativeItem = nullptr;

            if (currentTab == InventoryTab::CREATIVE)
            {
                const int CREATIVE_COLS = 9;
                const int CREATIVE_VISIBLE_ROWS = 3;

                for (int r = 0; r < CREATIVE_VISIBLE_ROWS; r++)
                {
                    for (int c = 0; c < CREATIVE_COLS; c++)
                    {
                        int itemIndex =
                            (r + creativeScrollRow) * CREATIVE_COLS + c;

                        if (itemIndex >= (int)allItems.size())
                            continue;

                        Rectangle slot = {
                            (float)(startX + c*(INV_CELL + INV_SPACING)),
                            (float)(startY + r*(INV_CELL + INV_SPACING)),
                            (float)INV_CELL,
                            (float)INV_CELL
                        };

                        if (CheckCollisionPointRec(mouse, slot))
                        {
                            hoveredCreativeItem = allItems[itemIndex];
                        }
                    }
                }
            }

            Item* hoveredContainerItem = nullptr;

            if (currentTab == InventoryTab::CONTAINER && currentContainer != nullptr)
            {
                for (int r = 0; r < CHEST_ROWS; r++)
                {
                    for (int c = 0; c < CHEST_COLS; c++)
                    {
                        ItemStack& stack = currentContainer->slots[r][c];

                        Rectangle slot = {
                            (float)(startX + c * (INV_CELL + INV_SPACING)),
                            (float)(startY + r * (INV_CELL + INV_SPACING)),
                            (float)INV_CELL,
                            (float)INV_CELL
                        };

                        if (CheckCollisionPointRec(mouse, slot) && stack.item != nullptr)
                        {
                            hoveredContainerItem = (Item*) stack.item;
                        }
                    }
                }
            }




            // ---- HOTBAR SLOTS (innerhalb des Inventar-Screens) ----
            int totalWidth = HOTBAR_SIZE * slotSize + (HOTBAR_SIZE - 1) * spacing;
            int startXHotBarInInventory = GetScreenWidth()/2 - totalWidth/2;
            //int yHotBarInInventory = invStartY + INV_ROWS * (INV_CELL + INV_SPACING) + 20; // etwas Abstand zum Inventar
            int yHotBarInInventory = bgY + invHeight - 70;

            for (int i = 0; i < HOTBAR_SIZE; i++)
            {
                Rectangle slot = {
                    (float)(startXHotBarInInventory + i * (slotSize + spacing)),
                    (float)yHotBarInInventory,
                    (float)slotSize,
                    (float)slotSize
                };

                if (CheckCollisionPointRec(mouse, slot) && hotbar[i].item != nullptr)
                {
                    hoveredStack = &hotbar[i];
                }
            }

            const Item* tooltipItem = nullptr;

            if (hoveredStack && hoveredStack->item)
                tooltipItem = hoveredStack->item;
            else if (hoveredCreativeItem)
                tooltipItem = hoveredCreativeItem;
            else if (hoveredContainerItem)
                tooltipItem = hoveredContainerItem;

            if (tooltipItem)
            {
                const int padding = 6;
                const int fontSizeTitle = 20;
                const int fontSizeTooltip = 16;

                std::string title   = tooltipItem->name;
                std::string tooltip = tooltipItem->tooltip;

                Vector2 titleSize =
                    MeasureTextEx(GetFontDefault(), title.c_str(), fontSizeTitle, 1);
                Vector2 tooltipSize =
                    MeasureTextEx(GetFontDefault(), tooltip.c_str(), fontSizeTooltip, 1);

                float width  = std::max(titleSize.x, tooltipSize.x) + padding * 2;
                float height = titleSize.y + tooltipSize.y + padding * 3;

                Rectangle tooltipRect = {
                    (float)GetMouseX() + 16,
                    (float)GetMouseY() + 16,
                    width,
                    height
                };

                if (tooltipRect.x + tooltipRect.width > GetScreenWidth())
                    tooltipRect.x = GetScreenWidth() - tooltipRect.width - 4;

                if (tooltipRect.y + tooltipRect.height > GetScreenHeight())
                    tooltipRect.y = GetScreenHeight() - tooltipRect.height - 4;

                DrawRectangleRec(tooltipRect, Fade(BLACK, 0.85f));
                DrawRectangleLinesEx(tooltipRect, 2, WHITE);

                DrawTextEx(
                    GetFontDefault(),
                    title.c_str(),
                    {tooltipRect.x + padding, tooltipRect.y + padding},
                    fontSizeTitle,
                    1,
                    YELLOW
                );

                DrawTextEx(
                    GetFontDefault(),
                    tooltip.c_str(),
                    {
                        tooltipRect.x + padding,
                        tooltipRect.y + padding + titleSize.y + padding
                    },
                    fontSizeTooltip,
                    1,
                    WHITE
                );
            }


            // -------- DRAGGED ITEM AM MAUSZEIGER --------
            if (draggedItem.item)
            {
                Color col = GetItemColor(draggedItem.item);
                DrawRectangle(mouse.x - 20, mouse.y - 20, 40, 40, col);
                DrawText(TextFormat("%d", draggedItem.count),
                        mouse.x + 10, mouse.y + 10, 14, WHITE);
            }
        }






        //Draw DraggedItem
        if (draggedItem.item != nullptr)
        {
            Vector2 m = GetMousePosition();
            Color col = GetItemColor(draggedItem.item);

            if (draggedItem.item->name == (char*) "Debug") col = {255,3,62,125};

            DrawRectangle(m.x - 20, m.y - 20, 40, 40, col);
            DrawRectangleLines(m.x - 20, m.y - 20, 40, 40, BLACK);

            DrawText(
                TextFormat("%d", draggedItem.count),
                m.x + 10, m.y + 10, 14, WHITE
            );
        }








        if (paused)
        {
            DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(BLACK,0.6f));
            Rectangle resume = { (float)GetScreenWidth()/2-150.0f, (float)GetScreenHeight()/2-40.0f, 300.0f, 50.0f };
            Rectangle quit   = { (float)GetScreenWidth()/2-150.0f, (float)GetScreenHeight()/2+30.0f, 300.0f, 50.0f };
            DrawRectangleRec(resume,GRAY); 
            DrawRectangleRec(quit,GRAY);
            DrawText("Game Menu", resume.x+80,resume.y-40,30,WHITE);
            DrawText("Back to Game", resume.x+60,resume.y+15,20,BLACK);
            DrawText("Quit Game", quit.x+100,quit.y+15,20,BLACK);
            if (CheckCollisionPointRec(GetMousePosition(),resume) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { paused = false; DisableCursor(); }
            if (CheckCollisionPointRec(GetMousePosition(),quit) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) break;
        }

        DrawFPS((float)GetScreenWidth() - 100,20);
        EndDrawing();
    }

    UnloadRenderTexture(previewTex);

    CloseWindow();
    return 0;
}
