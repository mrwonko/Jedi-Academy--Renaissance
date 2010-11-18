g_addonDir = "addons/"

require("STLHelpers.lua")
require("Mod.lua")


ModManager =
{
	mods = {}, -- string -> Mod dictionary for easy checks whether a mod is available
	activeMods = {}, -- array so it can be sorted by load order
	inactiveMods = {},
}

function ModManager:New()
	obj = {}
	setmetatable(obj, self)
	self.__index = self
	return obj
end

-- basically a private method of ModManager
local function AddAvailableMods(self)
	--mods are in ./addons/, either directories or .pk3 or .zip (some people can't/won't rename)
	
	--get files in ./addons/
	local filenames = jar.GetFilesInDirectory(g_addonDir)
	for filename in StringVectorIterator(filenames) do
		assert(filename ~= nil)
		--only zip/pk3 files are interesting
		if string.sub(filename, -4) == ".zip" or string.sub(filename, -4) == ".pk3" then
			--get their info & insert them
			local mod, err = Mod:New(filename)
			if not mod then
				jar.Logger.GetDefaultLogger():Warning(err)
			else
				if self.mods[mod.name] then
					jar.Logger.GetDefaultLogger():Warning("Multiple mods named \"" .. mod.name .."\" exist, only one will be loaded!")
				end
				self.mods[mod.name] = mod
			end
		end
	end
	
	--get directories in ./addons/
	local filenames = jar.GetDirectoriesInDirectory(g_addonDir)
	for filename in StringVectorIterator(filenames) do
		--get their info & insert them
		local mod, err = Mod:New(filename .. "/")
		if not mod then
			jar.Logger.GetDefaultLogger():Warning(err)
		else
			if self.mods[mod.name] then
				jar.Logger.GetDefaultLogger():Warning("Multiple mods named \"" .. mod.name .."\" exist, only one will be loaded!")
			end
			self.mods[mod.name] = mod
		end
	end
	return true
end

-- reads mods that are to be inactive from config/InactiveMods.lua
-- basically a private method of ModManager
local function LoadAndApplyConfigFile(self)
	
	-- get inactive mods

	local file, err = loadfile("config/InactiveMods.lua")
	
	-- if I can't open that file I assume it doesn't exist (which is ok) although it could contain an error
	if not file then
		jar.Logger.GetDefaultLogger():Info("Could not load config/InactiveMods.lua: "..err, 2)
		return true
	end
	local fileEnv = {}
	setfenv(file, fileEnv)
	local status, err = pcall(file)
	if not status then
		jar.Logger.GetDefaultLogger():Warning("Ignoring config/InactiveMods.lua (i.e. inactive mods settings) due to error: " .. err)
		-- it's not really bad, except that the user's settings are not used.
		return true
	end
	if not fileEnv.inactiveMods then
		jar.Logger.GetDefaultLogger():Warning("config/InactiveMods.lua doesn't contain info about inactive mods!")
		-- well, that's no reason to stop booting...
		return true
	end
	if type(fileEnv.inactiveMods) ~= "table" then
		jar.Logger.GetDefaultLogger():Warning("inactiveMods in config/InactiveMods.lua is no table, thus inactive mods cannot be loaded.")
		-- dito
		return true
	end
	for _, modname in ipairs(fileEnv.inactiveMods) do
		if type(modname) ~= "string" then
			jar.Logger.GetDefaultLogger():Warning("Invalid entry in config/InactiveMods.lua: No string!")
		else
			if self.mods[modname] then
				jar.Logger.GetDefaultLogger():Info("Disabling mod \"" .. modname .. "\" as specified in config/InactiveMods.lua.", 2)
				self.inactiveMods[modname] = self.mods[modname]
			else
				jar.Logger.GetDefaultLogger():Info("\"Forgetting\" disabled mod \"" .. modname .. "\" since it no longer exists.", 2)
			end
		end
	end
	
	return true
end

-- basically a private method of ModManager
local function ActivateMods(self)

	-- copy self.mods for the algorithm since we want to keep it the way it is
	local mods = {}
	for key, val in pairs(self.mods) do
		mods[key] = val
	end
	
	-- as long as another mod can be added I need to run this again.
	local continue = true
	while continue do
		continue = false
		-- try adding a mod, starting from the beginning
		for modname, mod in pairs(mods) do
			-- may have been put on inactive list by config
			if self.inactiveMods[modname] then
				mods[modname] = nil
				-- I don't think iterators still work (correctly) then the table is changed so let's start again
				continue = true
				break
			-- if there is an incompatibility, this mod cannot be added. ever.
			elseif mod:IsIncompatible(self.activeMods) then
				mods[modname] = nil
				self.inactiveMods[modname] = mod
				-- I don't think iterators still work (correctly) then the table is changed so let's start again
				continue = true
				break
			elseif mod:DependenciesSatisfied(self.activeMods) then
				-- this mod can be loaded. I'm not doing it here though since the order of pairs() is undefined and there should be at least *some* order.
				mods[modname] = nil
				-- array since it can be sorted
				table.insert(self.activeMods, mod)
				-- we need to run again since mods tested earlier may now have their dependencies satisfied.
				-- I don't think iterators still work (correctly) then the table is changed so let's start again
				continue = true
				break
			end
			--else: this mod can't be added yet, the dependencies aren't satisfied.
		end
	end
	-- anything still left doesn't have its dependencies satisfied. Put them into inactive list and report them.
	for name, mod in pairs(mods) do
		self.inactiveMods[name] = mod
		jar.Logger.GetDefaultLogger():Warning("The mod \"" .. name .. "\" has unsatisfied dependencies!")
	end
	
	-- sort the to-be-active mods by filename (for now) - I <3 anonymous functions
	table.sort(self.activeMods, function(first, second)
		--this functions should return true when first is "less" than second.
		--I order the loadorder by filename, just as in base jka.
		--TODO: change loadorder to be adjustable by the user (necessary? can't they just rename?)
		return string.lower(first.filename) < string.lower(second.filename)
		end)
	
	-- mods are now ordered correctly, mount them.
	for _, mod in ipairs(self.activeMods) do
		if not jar.fs.Mount(g_addonDir .. mod.filename, false) then
			-- if it couldn't be mounted then some dependencies may in fact not be satisfied.
			-- note: this should actually not happen since the file was already successfully opened durin modinfo.lua reading.
			jar.Logger.GetDefaultLogger():Error("Error mounting " .. g_addonDir .. mod.filename .. ": " .. jar.fs.GetLastError())
			return false
		end
	end
	
	return true
end

function ModManager:Init()
	--Get mods
	if not AddAvailableMods(self) then return false end
	
	--Deactivate those deactivated by the user
	if not LoadAndApplyConfigFile(self) then return false end

	--Activate mods
	if not ActivateMods(self) then return false end
	
	--save config (since maybe some mods could not be activated)
	self:SaveConfig()
	
	return true
end

-- saves currently inactive mods to config/InactiveMods.lua
function ModManager:SaveConfig()
	local file = jar.fs.OpenWrite("config/InactiveMods.lua")
	if not file then
		return false
	end
	assert(file:WriteString("inactiveMods =\n{\n"))
	for modname, _ in pairs(self.inactiveMods) do
		assert(file:WriteString("	\""..modname.."\",\n"))
	end
	assert(file:WriteString("}"))
	assert(file:Close())
	return true;
end