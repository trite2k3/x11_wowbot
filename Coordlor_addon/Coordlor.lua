-- Main frame setup
local frame = CreateFrame("Frame", "CoordColorFrame", UIParent)
frame:SetWidth(300)
frame:SetHeight(60)
frame:SetPoint("TOPLEFT", UIParent, "TOPLEFT", 10, -10)

-- Font text
local text = frame:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
text:SetAllPoints()
text:SetJustifyH("LEFT")
text:SetJustifyV("TOP")
text:SetTextColor(0, 1, 0)
text:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE")

-- Function to create pixel boxes
local function CreateColorBox(parent, xOffset, yOffset)
    local box = parent:CreateTexture(nil, "OVERLAY")
    box:SetWidth(5)
    box:SetHeight(5)
    box:SetPoint("TOPLEFT", parent, "BOTTOMLEFT", xOffset, yOffset)
    box:SetTexture(0, 0, 0)
    return box
end

-- Check if player is in Cat Form
local function IsInCatForm()
    for i = 1, 16 do
        local texture = GetPlayerBuffTexture(i)
        if texture and string.find(texture, "Ability_Druid_CatForm") then
            return true
        end
    end
    return false
end

local function GetFacingAngle()
  if pfQuestCompat and pfQuestCompat.GetPlayerFacing then
    return pfQuestCompat.GetPlayerFacing()        -- pfQuest (vanilla & TBC)
  elseif GetPlayerFacing then
    return GetPlayerFacing()                      -- WotLK or newer
  end
  return nil                                      -- not available
end

-- Positioning error
local lastSeenError = nil

local function GetLastUIErrorMessage()
    local regions = { UIErrorsFrame:GetRegions() }
    for i = 1, table.getn(regions) do
        local region = regions[i]
        if type(region) == "table" and region.GetText then
            local text = region:GetText()
            if text and text ~= "" then
                return text
            end
        end
    end
    return nil
end


-- Create pixel boxes
local boxX       = CreateColorBox(frame, 0, -2)
local boxY       = CreateColorBox(frame, 10, -2)
local boxCombat  = CreateColorBox(frame, 20, -2)
local boxHealth  = CreateColorBox(frame, 30, -2)
local boxTarget  = CreateColorBox(frame, 40, -2)
local boxCatForm = CreateColorBox(frame, 50, -2)
local boxMana = CreateColorBox(frame, 60, -2)
local boxFacing = CreateColorBox(frame, 70, -2)
local boxError = CreateColorBox(frame, 80, -2)
local boxAggro = CreateColorBox(frame, 90, -2)   -- red = aggro on you, green = no target
--

-- Time accumulator
local timeSinceLastUpdate = 0
local recentError = nil
local errorExpireTime = 0

-- Mappings to match
local errorMappings = {
    ["of range"] = "too far",
    ["in front"] = "not facing",
    ["the wrong"] = "not facing"
}

-- Update logic
frame:SetScript("OnUpdate", function()
    local elapsed = arg1 or 0.1
    timeSinceLastUpdate = timeSinceLastUpdate + elapsed
    if timeSinceLastUpdate >= 0.2 then
        -- Coordinates
        local x, y = GetPlayerMapPosition("player")
        local coordText = "Coords: ??,??"
        if x and y and x > 0 and y > 0 then
            x = x * 100
            y = y * 100
            coordText = string.format("Coords: %.2f,%.2f", x, y)
            boxX:SetTexture(x / 100, 0, 0)
            boxY:SetTexture(0, y / 100, 0)
        end

    -- ────────────────────────────────────────────────────────────────────
    --  (2)  INSIDE YOUR OnUpdate HANDLER  (just after you read coords)
    -- ────────────────────────────────────────────────────────────────────
        -- Player facing
        local facingRad  = GetFacingAngle()
        local facingText = "Facing: n/a"

        if facingRad then
            -- 0 … 2π  →  0 … 360
            local facingDeg = math.mod(facingRad, 2 * math.pi) * 180 / math.pi
            facingText       = string.format("Facing: %.0f°", facingDeg)

            -- encode 0‑360° → 0‑1 → 0‑255 (red channel)
            local encoded = facingDeg / 360            -- 0‑1
            boxFacing:SetTexture(encoded, 0, 0)        -- R=angle, G/B left at 0
        else
            boxFacing:SetTexture(0, 0, 0)              -- API not available
        end

        -- Health
        local hp = UnitHealth("player")
        local hpMax = UnitHealthMax("player")
        local hpPct = hpMax > 0 and (hp / hpMax) or 0
        local healthText = string.format("Health: %d / %d", hp, hpMax)
        boxHealth:SetTexture(0, 0, hpPct)

        -- Combat
        local inCombat = UnitAffectingCombat("player")
        boxCombat:SetTexture(inCombat and 1 or 0, inCombat and 0 or 1, 0)
        local combatText = inCombat and "Combat: YES" or "Combat: NO"

        -- Target status with alive/dead check
        local hasTarget = UnitExists("target")
        local isDead = hasTarget and UnitIsDead("target")

        if not hasTarget then
            boxTarget:SetTexture(0.2, 0.2, 0.2)  -- Dark grey = no target
        elseif isDead then
            boxTarget:SetTexture(1, 0, 0)        -- Red = dead target
        else
            boxTarget:SetTexture(0, 1, 0)        -- Green = alive target
        end

        local targetText
        if not hasTarget then
            targetText = "No target"
        elseif isDead then
            targetText = "Target is dead"
        else
            targetText = "Target is alive"
        end

        -- Cat Form detection via buff icon
        local inCatForm = IsInCatForm()
        boxCatForm:SetTexture(inCatForm and 0 or 0.2, inCatForm and 0.5 or 0.2, inCatForm and 1 or 0.2)
        local formText = inCatForm and "In Cat Form" or "Not in Cat Form"

        -- Mana
        local mana = UnitMana("player")
        local manaMax = UnitManaMax("player")
        local manaPct = manaMax > 0 and (mana / manaMax) or 0
        boxMana:SetTexture(0, 0, manaPct)
        local manaText = string.format("Mana: %d / %d", mana, manaMax)

        local currentError = GetLastUIErrorMessage()
        if currentError and currentError ~= lastSeenError then
            lastSeenError = currentError
            for pattern, message in pairs(errorMappings) do
                if string.find(currentError, pattern) then
                    recentError = message
                    errorExpireTime = GetTime() + 2
                    break
                end
            end
        end

        -- Set or clear error box color
        if GetTime() < errorExpireTime then
            if recentError == "too far" then
                boxError:SetTexture(1, 0.5, 0) -- orange
            elseif recentError == "not facing" then
                boxError:SetTexture(1, 0, 1)   -- magenta
            else
                boxError:SetTexture(0.3, 0.3, 0.3)
            end
            else
                boxError:SetTexture(0, 0, 0)
                recentError = nil
                -- DO NOT reset lastSeenError unless the message is gone
                local stillVisible = GetLastUIErrorMessage()
                if not stillVisible then
                    lastSeenError = nil
                end
            end

        if hasTarget then
            if not UnitExists("targettarget") then            -- nothing selected
                boxAggro:SetTexture(0, 1, 0)                  -- GREEN
            elseif UnitIsUnit("targettarget", "player") then  -- attacking YOU
                boxAggro:SetTexture(1, 0, 0)                  -- RED
            else                                              -- attacking something else
                boxAggro:SetTexture(0.3, 0.3, 0.3)            -- dark grey (optional)
            end
        else
            boxAggro:SetTexture(0, 0, 0)                      -- you have no target
        end

        -- Final onscreen text
        text:SetText(coordText .. "\n" .. healthText .. "\n" .. combatText .. "\n" .. targetText .. "\n" .. formText)

        timeSinceLastUpdate = 0
    end
end)
