--[[
CoordColorFrame_Optimized_Classic.lua
Compatible with WoW 1.12 / Turtle WoW (Lua 5.0)

Key differences from the retail/WotLK version:
• **No SetSize** (not available in 1.12) – uses SetWidth / SetHeight
• **No UnitPower / UnitPowerMax** – falls back to UnitMana / UnitManaMax
• **No UnitAffectingCombat** in 1.12 – combat state is tracked via PLAYER_REGEN_* events
• **No “%” operator in Lua 5.0** – uses math.mod instead
• **Cat‑form detection** falls back to GetPlayerBuffTexture

Everything else is identical, so performance stays flat.
--]]

---------------------------------------------------------------------
--  Constants & fallbacks
---------------------------------------------------------------------
local UPDATE_INTERVAL = 0.2          -- seconds between UI refreshes
local FLASH_TIME      = 1            -- seconds the UI‑error colour stays on

-- Map partial error strings ➜ short codes
local errorMappings = {
    ["too far away"] = "too far",
    ["in front"] = "not facing",
    ["facing the"]   = "not facing",
    ["of range"]   = "too far",
}

-- Lua 5.0 doesn’t ship “%” – helper wrapper
local function mod(a, b) return math.mod and math.mod(a, b) or a - math.floor(a/b)*b end

---------------------------------------------------------------------
--  Classic‑safe combat detection
---------------------------------------------------------------------
local has_UnitAffectingCombat = type(UnitAffectingCombat) == "function"
local playerInCombat          = false    -- updated by events in 1.12

if not has_UnitAffectingCombat then
    local combatWatch = CreateFrame("Frame")
    combatWatch:RegisterEvent("PLAYER_REGEN_DISABLED")
    combatWatch:RegisterEvent("PLAYER_REGEN_ENABLED")
    combatWatch:SetScript("OnEvent", function(_, event)
        playerInCombat = (event == "PLAYER_REGEN_DISABLED")
    end)
end

---------------------------------------------------------------------
--  Helper: get facing angle (pfQuest for vanilla, API later)
---------------------------------------------------------------------
local function GetFacingAngle()
    if pfQuestCompat and pfQuestCompat.GetPlayerFacing then
        return pfQuestCompat.GetPlayerFacing()           -- pfQuest (Vanilla/TBC)
    elseif GetPlayerFacing then
        return GetPlayerFacing()                         -- Burning Crusade+
    end
end

---------------------------------------------------------------------
--  Helper: Cat‑form check for 1.12 (buff slots 1‑16)
---------------------------------------------------------------------
local CAT_FORM_ICON = "Ability_Druid_CatForm"
local function IsInCatForm()
    for slot = 1, 16 do
        local tex = GetPlayerBuffTexture(slot)
        if tex and string.find(tex, CAT_FORM_ICON, 1, true) then
            return true
        end
    end
    return false
end

---------------------------------------------------------------------
--  Main frame & UI elements (created ONCE)
---------------------------------------------------------------------
local frame = CreateFrame("Frame", "CoordColorFrame", UIParent)
frame:SetWidth(300)
frame:SetHeight(60)
frame:SetPoint("TOPLEFT", UIParent, "TOPLEFT", 10, -10)

local text = frame:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
text:SetAllPoints()
text:SetJustifyH("LEFT")
text:SetJustifyV("TOP")
text:SetTextColor(0, 1, 0)
text:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE")

local function CreateColorBox(parent, xOffset)
    local tex = parent:CreateTexture(nil, "OVERLAY")
    tex:SetWidth(5)
    tex:SetHeight(5)
    tex:SetPoint("TOPLEFT", parent, "BOTTOMLEFT", xOffset, -2)
    tex:SetTexture(0, 0, 0)
    return tex
end

-- Row of indicator boxes
local boxX, boxY    = CreateColorBox(frame, 0),  CreateColorBox(frame, 10)
local boxCombat     = CreateColorBox(frame, 20)
local boxHealth     = CreateColorBox(frame, 30)
local boxTarget     = CreateColorBox(frame, 40)
local boxCatForm    = CreateColorBox(frame, 50)
local boxMana       = CreateColorBox(frame, 60)
local boxFacing     = CreateColorBox(frame, 70)
local boxError      = CreateColorBox(frame, 80)
local boxAggro      = CreateColorBox(frame, 90)
local boxPet        = CreateColorBox(frame, 100)

---------------------------------------------------------------------
--  UI‑error watcher (created once, never inside OnUpdate!)
---------------------------------------------------------------------
local recentError, errorExpireTime = nil, 0
local errFrame = CreateFrame("Frame")
errFrame:RegisterEvent("UI_ERROR_MESSAGE")
errFrame:SetScript("OnEvent", function(_, _, msg)
    msg = msg or arg1          -- arg1 in 1.12
    if type(msg) ~= "string" then return end
    local lower = string.lower(msg)
    for pat, code in pairs(errorMappings) do
        if string.find(lower, pat, 1, true) then
            recentError     = code
            errorExpireTime = GetTime() + FLASH_TIME
            break
        end
    end
end)

---------------------------------------------------------------------
--  Main update loop (throttled)
---------------------------------------------------------------------
local elapsedSince = 0
frame:SetScript("OnUpdate", function(_, elapsed)
    elapsedSince = elapsedSince + (elapsed or 0.05)
    if elapsedSince < UPDATE_INTERVAL then return end
    elapsedSince = 0

    ------------------------------------------------------------------
    -- Position (coords 0‑100)
    local x, y = GetPlayerMapPosition("player")
    local coordText = "Coords: ??,??"
    if x and x > 0 and y and y > 0 then
        x, y = x * 100, y * 100
        coordText = string.format("Coords: %.2f,%.2f", x, y)
        boxX:SetTexture(x / 100, 0, 0)
        boxY:SetTexture(0, y / 100, 0)
    else
        boxX:SetTexture(0, 0, 0)
        boxY:SetTexture(0, 0, 0)
    end

    ------------------------------------------------------------------
    -- Facing angle encoded in red channel
    local facing = GetFacingAngle()
    local facingText = "Facing: n/a"
    if facing then
        local deg = mod(facing, 2 * math.pi) * 180 / math.pi
        facingText = string.format("Facing: %.0f°", deg)
        boxFacing:SetTexture(deg / 360, 0, 0)
    else
        boxFacing:SetTexture(0, 0, 0)
    end

    ------------------------------------------------------------------
    -- Health ➜ blue channel
    local hp, hpMax = UnitHealth("player"), UnitHealthMax("player")
    local hpPct     = hpMax > 0 and hp / hpMax or 0
    boxHealth:SetTexture(0, 0, hpPct)
    local healthText = string.format("Health: %d / %d", hp, hpMax)

    ------------------------------------------------------------------
    -- Combat
    local inCombat = has_UnitAffectingCombat and UnitAffectingCombat("player") or playerInCombat
    boxCombat:SetTexture(inCombat and 1 or 0, inCombat and 0 or 1, 0)
    local combatText = inCombat and "Combat: YES" or "Combat: NO"

    ------------------------------------------------------------------
    -- Target status & aggro box
    local hasTarget = UnitExists("target")
    local isDead    = hasTarget and UnitIsDead("target")

    if not hasTarget then
        boxTarget:SetTexture(0.2, 0.2, 0.2)
    elseif isDead then
        boxTarget:SetTexture(1, 0, 0)
    else
        boxTarget:SetTexture(0, 1, 0)
    end

    local targetText = not hasTarget and "No target"
                    or isDead       and "Target is dead" or "Target is alive"

    if hasTarget then
        if not UnitExists("targettarget") then
            boxAggro:SetTexture(0, 1, 0)
        elseif UnitIsUnit("targettarget", "player") or UnitIsUnit("targettarget", "pet") then
            boxAggro:SetTexture(1, 0, 0)
        else
            boxAggro:SetTexture(0.3, 0.3, 0.3)
        end
    else
        boxAggro:SetTexture(0, 0, 0)
    end

    ------------------------------------------------------------------
    -- Druid cat form
    local inCatForm = IsInCatForm()
    boxCatForm:SetTexture(inCatForm and 0 or 0.2,
                          inCatForm and 0.5 or 0.2,
                          inCatForm and 1 or 0.2)
    local formText = inCatForm and "In Cat Form" or "Not in Cat Form"

    ------------------------------------------------------------------
    -- Mana (blue channel again – choose whichever you need more)
    local mana, manaMax = UnitMana("player"), UnitManaMax("player")
    local manaPct       = manaMax > 0 and mana / manaMax or 0
    boxMana:SetTexture(0, 0, manaPct)
    local manaText = string.format("Mana: %d / %d", mana, manaMax)

    ------------------------------------------------------------------
    -- UI error flash
    if recentError and GetTime() < errorExpireTime then
        if recentError == "too far" then
            boxError:SetTexture(1, 0.5, 0)
        elseif recentError == "not facing" then
            boxError:SetTexture(1, 0, 1)
        else
            boxError:SetTexture(0.3, 0.3, 0.3)
        end
    else
        boxError:SetTexture(0, 0, 0)
        recentError = nil
    end

    ------------------------------------------------------------------
    -- Pet status & HP (green = alive, red = low HP, gray = none)
    local hasPet = UnitExists("pet")
    if hasPet then
        local petHP, petHPMax = UnitHealth("pet"), UnitHealthMax("pet")
        local petHPPct = (petHPMax > 0) and (petHP / petHPMax) or 0

        -- Green when healthy, red when low, blue if full HP
        local r = (1 - petHPPct)
        local g = petHPPct
        boxPet:SetTexture(r, g, 0)
    else
        -- Black if no pet
        boxPet:SetTexture(0.0, 0.0, 0.0)
    end


    ------------------------------------------------------------------
    -- Combine & display multiline read‑out (only strings; no heavy ops)
    text:SetText(coordText .. "\n" .. healthText .. "\n" .. combatText ..
                 "\n" .. targetText .. "\n" .. formText .. "\n" .. facingText ..
                 "\n" .. manaText)
end)
